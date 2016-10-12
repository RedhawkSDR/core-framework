#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

from ossie.cf import CF
from omniORB import URI, any, CORBA
import ossie.utils.log4py.config
import tempfile
import os
import urlparse
urlparse.uses_netloc.append("sca")
urlparse.uses_query.append("sca")
import logging
import time
import threading
from ossie.properties import PropertyStorage

import sys
import CosNaming
import signal
import getopt
from Queue import Queue


def _getCallback(obj, methodName):
    try:
        callback = getattr(obj, methodName)
    except AttributeError:
        return None
    else:
        if callable(callback):
            return callback
        else:
            return None


class PropertyAttributeMixIn:
    """Include this MixIn with your Device if you want your properties to
    be accessible as attributes."""

    def __getattr__(self, name):
        if name.startswith("prop_"):
            propname = name.split("_", 1)[1]
            propid = self._props.getPropId(propname)
            return self._props[propid]
        else:
            try:
                return self.__dict__[name]
            except KeyError:
                raise AttributeError("%s object has no attribute %s" % (type(self), name))

    def __setattr__(self, name, value):
        if name.startswith("prop_"):
            propname = name.split("_", 1)[1]
            propid = self._props.getPropId(propname)
            self._props[propid] = value
        else:
            self.__dict__[name] = value

# Class 'static' objects for describing a port
class _port(object):
    def __init__(self, name, repid, type_, fget=None):
        self.name = name
        self.repid = repid
        self.type_ = type_
        self.fget = fget
        self._attrname = "__port_%s__" % self.name

    def __get__(self, obj, objtype=None):
        if obj is None:
            return self

        if self.fget != None:
            return self.fget(obj)
        else:
            try:
                retval = getattr(obj, self._attrname)
            except:
                retval = None
            return retval

    def __set__(self, obj, value):
        if self.fget != None:
            raise AttributeError, "can't set SCA port definition that uses fget"
        setattr(obj, self._attrname, value)

    def __delete__(self, obj):
        raise AttributeError, "can't delete SCA port definition"

    def isValid(self, portobj):
        raise NotImplementedError

class usesport(_port):
    CF_PORT_REPID = "IDL:CF/Port:1.0"

    def isValid(self, portobj):
        """Validate that portobj is the expected type to get returned by PortSupplier.getPort() based on the repid."""
        # Uses ports are always CF.Port objects
        return (portobj != None) and (portobj._is_a(self.CF_PORT_REPID))

class providesport(_port):
    def isValid(self, portobj):
        """Validate that portobj is the expected type to get returned by PortSupplier.getPort() based on the repid."""
        return (portobj != None) and (portobj._is_a(self.repid))

class Resource(object):
    def __init__(self, identifier, execparams, propertydefs=(), loggerName=None):
        """propertydefs is a iterable of tuples that contain

        (propid, propname, type, mode, defaultvalue, units, action, kinds)

        defaultvalue should be None if there is no default, otherwise:
            simple - a single str, int, float, bool
            simple sequence - an iterable of str, int, float, or bool
            struct - a dictionary there the key is the "id" and the value is a tuple
                     of (propname, type, mode)
            struct sequence - a iterable of dictionaries following the struct format
        """
        self.propertySetAccess = threading.Lock()
        self._id = identifier
        self._started = False
        if loggerName == None:
            self._log = logging.getLogger(self._id)
        else:
            self._log = logging.getLogger(loggerName)
        self._name = execparams.get("NAME_BINDING", "")
        # The base resource class manages properties ...
        self._props = PropertyStorage(self, propertydefs, execparams)
        self._props.initialize()
        # ... and also manages ports
        self.__loadPorts()

        logging.trace("Initial property storage %s", self._props)


    #########################################
    # CF::Resource
    def start(self):
        self._log.trace("start()")
        self._started = True

    def stop(self):
        self._log.trace("stop()")
        self._started = False

    def _get_identifier(self):
        return self._id

    def _get_started(self):
        return self._started

    #########################################
    # CF::LifeCycle
    def initialize(self):
        self._log.trace("initialize()")

    def releaseObject(self):
        self._log.trace("releaseObject()")
        objid = self._default_POA().servant_to_id(self)
        self._default_POA().deactivate_object(objid)

    #########################################
    # CF::PortSupplier
    def getPort(self, name):
        """The default behavior of getPort() will automatically
        return ports as defined by 'usesport' and 'providesport'
        static class attributes."""
        self._log.trace("getPort(%s)", name)
        try:
            portdef = self.__ports[name]
        except KeyError:
            self._log.warning("getPort() could not find port %s", name)
            raise CF.PortSupplier.UnknownPort()
        else:
            portobj = portdef.__get__(self)
            if portobj == None:
                self._log.warning("component did not implement port %s",name)
                raise CF.PortSupplier.UnknownPort()
            port = portobj._this()
            if not portdef.isValid(port):
                self._log.warning("getPort() for %s did match required repid", name)
            self._log.trace("getPort() --> %s", port)
            return port

    def __loadPorts(self):
        self.__ports = {}
        for name in dir(type(self)):
            attr = getattr(type(self), name)
            if isinstance(attr, _port):
                self.__ports[attr.name] = attr

    #########################################
    # CF::TestableObject
    def runTest(self, testid, properties):
        """Override this function to provide the desired behavior."""
        self._log.trace("runTest()")
        raise CF.Device.UnknownTest("unknown test: %s" % str(testid))

    #########################################
    # CF::PropertySet
    def query(self, configProperties):
        self.propertySetAccess.acquire()
        # If the list is empty, get all props
        if configProperties == []:
            self._log.trace("query all properties")
            try:
                rv = []
                for propid in self._props.keys():
                    if self._props.has_id(propid) and self._props.isQueryable(propid):
                        try:
                            value = self._props.query(propid)
                        except Exception, e:
                            self._log.error('Failed to query %s: %s', propid, e)
                            value = any.to_any(None)
                        rv.append(CF.DataType(propid, value))
            except:
                self.propertySetAccess.release()
                raise

        # otherwise get only the requested ones
        else:
            self._log.trace("query %s properties", len(configProperties))
            try:
                unknownProperties = []
                for prop in configProperties:
                    if self._props.has_id(prop.id) and self._props.isQueryable(prop.id):
                        try:
                            prop.value = self._props.query(prop.id)
                        except Exception, e:
                            self._log.error('Failed to query %s: %s', prop.id, e)
                    else:
                        self._log.warning("property %s cannot be queried.  valid Id: %s", 
                                        prop.id, self._props.has_id(prop.id))
                        unknownProperties.append(prop)
            except:
                self.propertySetAccess.release()
                raise

            if len(unknownProperties) > 0:
                self._log.warning("query called with invalid properties %s", unknownProperties)
                self.propertySetAccess.release()
                raise CF.UnknownProperties(unknownProperties)

            rv = configProperties
        self.propertySetAccess.release()
        self._log.trace("query -> %s properties", len(rv))
        return rv

    def configure(self, configProperties):
        self._log.trace("configure(%s)", configProperties)
        self.propertySetAccess.acquire()
        notSet = []
        for prop in configProperties:
            try:
                if self._props.has_id(prop.id) and self._props.isConfigurable(prop.id):
                    try:
                        self._props.configure(prop.id, prop.value)
                    except ValueError:
                        self._log.warning("Invalid value provided to configure for property %s", prop.id)
                        notSet.append(prop)
                else:
                    self._log.warning("Tried to configure non-existent, readonly, or property with action not equal to external %s", prop.id)
                    notSet.append(prop)
            except Exception, e:
                self._log.exception("Unexpected exception.")
                notSet.append(prop)

        if len(notSet) > 0 and len(notSet) < len(configProperties):
            self.propertySetAccess.release()
            self._log.warning("Configure failed with partial configuration, %s", notSet)
            raise CF.PropertySet.PartialConfiguration(notSet)
        elif len(notSet) > 0 and len(notSet) >= len(configProperties):
            self.propertySetAccess.release()
            self._log.warning("Configure failed with invalid configuration, %s", notSet)
            raise CF.PropertySet.InvalidConfiguration("Failure", notSet)
        self.propertySetAccess.release()
        self._log.trace("configure(%s)", configProperties)


def __exit_handler(signum, frame):
    raise SystemExit

def configure_logging(orb, uri, debugLevel=3, binding=None):
    if uri:
        load_logging_config_uri(orb, uri, binding)
    else:
        # Force the debug level to be an integer.
        try:
            debugLevel = int(debugLevel)
        except:
            debugLevel = 3

        DEBUG_LEVELS = [ logging.FATAL, logging.ERROR, logging.WARNING,
                         logging.INFO, logging.DEBUG, logging.NOTSET ]

        # Bound the debug level to the be within the lookup table.
        debugLevel = min(len(DEBUG_LEVELS)-1, max(0, debugLevel))
        level = DEBUG_LEVELS[debugLevel]

        try:
            logging.basicConfig(stream=sys.stdout, level=level)
        except:
            # The logging module in Python 2.3 does not allow arguments to basicConfig.
            logging.basicConfig()
            logging.getLogger().setLevel(level)

def load_logging_config_uri(orb, uri, binding=None):
    scheme, netloc, path, params, query, fragment = urlparse.urlparse(uri)
    if scheme == "file":
        ossie.utils.log4py.config.fileConfig(path, binding)
    elif scheme == "sca":
        q = dict([x.split("=") for x in query.split("&")])
        try:
            fileSys = orb.string_to_object(q["fs"])
        except KeyError:
            logging.warning("sca URI missing fs query parameter")
        else:
            if fileSys == None:
                logging.warning("Failed to lookup file system")
            else:
                try:
                    t = tempfile.mktemp()

                    tf = open(t, "w+")
                    scaFile = fileSys.open(path, True)
                    fileSize = scaFile.sizeOf()
                    buf = scaFile.read(fileSize)
                    tf.write(buf)
                    tf.close()
                    scaFile.close()

                    ossie.utils.log4py.config.fileConfig(t)
                finally:
                    os.remove(t)
    else:
        # Invalid scheme
        logging.warning("Invalid logging config URI scheme")

def _turnArgsIntoDictionary(args):
    execparams = {}
    while len(args) > 0:
        try:
            paramid = args.pop(0)
            paramvalue = args.pop(0)
            execparams[paramid] = paramvalue
        except IndexError:
            pass
    return execparams

def configureLogging(execparams, loggerName, orb):
    # Configure logging (defaulting to INFO level).
    log_config_uri = execparams.get("LOGGING_CONFIG_URI", None)
    debug_level = execparams.get("DEBUG_LEVEL", 3)
    if not loggerName:
        loggerName = execparams.get("NAME_BINDING", None)
        if loggerName:
            loggerName = loggerName.rsplit("_", 1)[0]
    configure_logging(orb, log_config_uri, debug_level, binding=loggerName)

def getPOA(orb, thread_policy, name):
    # get the POA
    obj_poa = orb.resolve_initial_references("RootPOA")
    poaManager = obj_poa._get_the_POAManager()
    if thread_policy != None:
        policyList = []
        policyList.append(obj_poa.create_thread_policy(thread_policy))
        POA = obj_poa.create_POA(name, poaManager, policyList)
    else:
        POA = obj_poa
    poaManager.activate()
    return POA

def _pollQueue(queue, pollPeriodSeconds = 0.001, timeoutSeconds = 1):
    '''
    Poll a Queue every pollPeriodSeconds for up to timeoutSeconds.
    Return the value on the queue once there is one.  Return None 
    if the polling times out.

    '''

    while timeoutSeconds > 0:
        if queue.full():
            # success
            return queue.get()
        time.sleep(pollPeriodSeconds)
        timeoutSeconds -= pollPeriodSeconds
    return None

def callOmniorbpyWithTimeout(method, queue, pollPeriodSeconds = 0.001, timeoutSeconds = 1):
    """
    Some omniorbpy methods have been found to hang if the system runs out of 
    threads.  Call method and wait for up to timeoutSeconds.  If the method
    returns within timeoutSeconds, return the value placed on the queue; 
    otherwise, return None.

    """

    thread = threading.Thread(target = method)
    try:
        thread.start()
    except:
        # If the system is out of threads, the thread.start() method can
        # potentially fail.
        return None

    return _pollQueue(queue, 
                      pollPeriodSeconds = pollPeriodSeconds, 
                      timeoutSeconds = timeoutSeconds)

def createOrb():
    '''
    Calls the omniorbpy CORBA.ORB_init() method in a thread.  Calling the method in a thread
    allows us to set a timeout for the ORB_init() call, as it will never return if the system
    has run out of threads.

    Return None on failure.
    '''
    # create a queue with one slot to hold the orb
    queue = Queue(maxsize=1) 

    def orbCreator():
        """
        A method to pass to callOmniorbpyWithTimeout.

        """

        orb = CORBA.ORB_init()
        queue.put(orb)

    orb = callOmniorbpyWithTimeout(orbCreator, queue)
    if orb == None:
        logging.error("omniorbpy failed to return from ORB_init.  This is often a result of an insufficient amount of threads available on the system.")
        sys.exit(-1)
    return orb

def _getOptions(classtype):
    try:
        # IMPORTANT YOU CANNOT USE gnu_getopt OR OptionParser
        # because they will treat execparams with negative number
        # values as arguments.
        #
        # Since property ids *MUST* be valid XML names
        # they cannot start with -, therefore this is safe
        opts, args = getopt.getopt(sys.argv[1:], "i", ["interactive"])
        if len(opts)==0 and len(args)==0:
            print "usage: %s [options] [execparams]" % sys.argv[0]
            print
            print "The set of execparams is defined in the .prf for the component"
            print "They are provided as arguments pairs ID VALUE, for example:"
            print "     %s INT_PARAM 5 STR_PARAM ABCDED" % sys.argv[0]
            print
            print "Options:"
            print "     -i,--interactive           Run the component in interactive test mode"
            print
            print classtype.__doc__
            sys.exit(2)
    except getopt.GetoptError:
        print "usage: %s [options] [execparams]" % sys.argv[0]
        print
        print "The set of execparams is defined in the .prf for the component"
        print "They are provided as arguments pairs ID VALUE, for example:"
        print "     %s INT_PARAM 5 STR_PARAM ABCDED" % sys.argv[0]
        print
        print "Options:"
        print "     -i,--interactive           Run the component in interactive test mode"
        print
        print classtype.__doc__
        sys.exit(2)
    return opts, args

def setupSignalHandlers():
    signal.signal(signal.SIGINT, __exit_handler)
    signal.signal(signal.SIGQUIT, __exit_handler)
    signal.signal(signal.SIGTERM, __exit_handler)

def _getInteractive(opts):
    """
    If opts contains '-1' or '--interactive', return True, otherwise 
    return False. 
    """

    interactive = False
    for opt, unused in opts:
        if opt == "-i" or opt == "--interactive":
            interactive = True
    return interactive

def parseCommandLineArgs(componentclass):
    opts, args = _getOptions(componentclass)

    interactive = _getInteractive(opts)

    execparams = _turnArgsIntoDictionary(args)

    return execparams, interactive

def start_component(componentclass, interactive_callback=None, thread_policy=None, loggerName=None):   
    execparams, interactive = parseCommandLineArgs(componentclass)
    setupSignalHandlers()
    orb = None

    try:
        try:
            orb = createOrb()

            configureLogging(execparams, loggerName, orb)

            componentPOA = getPOA(orb, thread_policy, "componentPOA")
          
            if not execparams.has_key("COMPONENT_IDENTIFIER"):
                if not interactive:
                    logging.warning("No 'COMPONENT_IDENTIFIER' argument provided")
                execparams["COMPONENT_IDENTIFIER"] = ""
            
            if not execparams.has_key("NAME_BINDING"):
                if not interactive:
                    logging.warning("No 'NAME_BINDING' argument provided")
                execparams["NAME_BINDING"] = ""

            # Create the component
            component_Obj = componentclass(execparams["COMPONENT_IDENTIFIER"], execparams)
            componentPOA.activate_object(component_Obj)
            component_Var = component_Obj._this()

            # get the naming context and bind to it
            if execparams.has_key("NAMING_CONTEXT_IOR"):
                rootContext = orb.string_to_object(execparams['NAMING_CONTEXT_IOR'])
                if rootContext == None:
                    logging.error("Failed to lookup naming context")
                    sys.exit(-1)

                rootContext = rootContext._narrow(CosNaming.NamingContext)
                name = URI.stringToName(execparams['NAME_BINDING'])
                rootContext.rebind(name, component_Var)
            else:
                if not interactive:
                    logging.warning("Skipping name-binding because required execparams 'NAMING_CONTEXT_IOR' is missing")

            if not interactive:
                logging.trace("Starting ORB event loop")
                orb.run()
            else:
                logging.trace("Entering interactive mode")
                if callable(interactive_callback):
                    # Pass only the Var to prevent anybody from calling non-CORBA functions
                    interactive_callback(component_Obj)
                else:
                    print orb.object_to_string(component_Obj._this())
                    orb.run()
            try:
                orb.shutdown(true)
            except:
                pass
            signal.signal(signal.SIGINT, signal.SIG_IGN)
        except SystemExit:
            pass
        except KeyboardInterrupt:
            pass
        except:
            logging.exception("Unexpected Error")
    finally:
        if orb:
            orb.destroy()


