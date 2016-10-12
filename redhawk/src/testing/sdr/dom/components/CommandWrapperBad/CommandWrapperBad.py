#!/usr/bin/python
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

#
from ossie.cf import CF, CF__POA
import os, time, signal, copy, sys
import omniORB.any
import logging
import signal

PROPERTIES = (
  ("DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e", "command"),
  ("DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53", "args"),
  ("DCE:95f19cb8-679e-48fb-bece-dc199ef45f20", "commandAlive"),
  ("DCE:85d133fd-1658-4e4d-b3ff-1443cd44c0e2", "execparams"),
)

_ID_MAP = dict([(prop[0], pos) for pos, prop in enumerate(PROPERTIES)])
_NAME_MAP = dict([(prop[1], pos) for pos, prop in enumerate(PROPERTIES)])

def getName(propid):
    return PROPERTIES[_ID_MAP[propid]][1]

def getId(name):
    return PROPERTIES[_NAME_MAP[name]][0]

class CommandWrapper_i(CF__POA.Resource):
    """This component simply ensures that all other components in the waveform get started and
    stopped together."""

    # The signal to send and the number of seconds (max) to wait
    # for the process to actually exit before trying the next signals.
    # None means wait until the process dies (which we have to do to
    # avoid creating zombie processes
    STOP_SIGNALS = ((signal.SIGINT, 1), (signal.SIGTERM, 5), (signal.SIGKILL, None))
    def __init__(self, parent_orb, execparams,  poa):
        CF._objref_Resource.__init__(self._this())
        # The CORBA name this object is registered under
        self.naming_service_name = execparams['NAME_BINDING']
        # The parent ORB for this object
        self.parent_orb = parent_orb
        # The CORBA portable object adapter
        self.poa = poa
        # The uuid assigned to this instance of the component
        self.uuid = execparams['COMPONENT_IDENTIFIER']
        # The storage of property values that don't have getters/setters
        self.propertySet = {}
        execparams_value = " ".join(["%s %s" % x for x in execparams.items()]) 
        self.propertySet[getId("execparams")] = CF.DataType(id=getId("execparams"), value=omniORB.any.to_any(execparams_value))

        # The PID of the child process
        self._pid = None
        self._log = logging.getLogger(self.naming_service_name)

    ######################################
    # Implement the Resource interface
    def start(self):
        self._log.debug("start()")
        if self.query_commandAlive() == False:
            command = self.propertySet[getId("command")].value.value()
            args = copy.copy(self.propertySet[getId("args")].value.value())
            args.insert(0, command)
            self._log.debug("start %s %r", command, args)
            self._pid = os.spawnv(os.P_NOWAIT, command, args)
            self._log.debug("spawned %s", self._pid)

    def stop(self):
        self._log.debug("%s.stop()", self.naming_service_name)
        if self.query_commandAlive() == True:
            for sig, timeout in self.STOP_SIGNALS:
                try:
                    os.kill(self._pid, sig)
                except OSError:
                    self._pid = None
                    return
                if timeout != None:
                    giveup_time = time.time() + timeout
                    while os.waitpid(self._pid, os.WNOHANG) == (0,0):
                        time.sleep(0.1)
                        if time.time() > giveup_time:
                            break
                else:
                    # Wait until there is a response
                    os.waitpid(self._pid, 0)
            self._pid = None

    def _get_identifier(self):
        return self.uuid

    ######################################
    # Implement the PropertySet interface
    def configure(self, props):
        self._log.debug("%s.configure(%s)", self.naming_service_name, props)
        raise CF.PropertySet.InvalidConfiguration("bad configuration", props)
        #raise CF.PropertySet.PartialConfiguration(props)
        #raise "None"

    def query(self, props):
        self._log.debug("%s.query()", self.naming_service_name)
        # If the properties list is empty we query all properties
        if len(props) == 0:
            for property in self.propertySet.values():
                props.append(property)
                
        for property in props:
            self._log.debug("query Id => %s", property.id)
            try:
                propertyName = getName(property.id)
                self._log.debug("query Name => %s", propertyName)
            except KeyError:
                pass
            else:
                try:
                    callback = getattr(self, "query_%s" % propertyName)
                except AttributeError:
                    property.value = self.propertySet[property.id].value
                else:
                    if callable(callback):
                        value = callback()
                        property.value = omniORB.any.to_any(value)
                self._log.debug("query Value => %r", property.value._v)
        # The OMG CORBA python mapping has inout, and out argument
        # return from the function.
        self._log.debug("query -> %s", props)
        return props 

    ######################################
    # Implement the PortSupplier interface
    def getPort(self, name):
        return None

    ######################################
    # Implement the LifeCycle interface
    def initialize(self):
        pass

    def releaseObject(self):
        pass

    ######################################
    # Implement specific property setters/getters 
    def query_commandAlive(self):
        if self._pid != None:
            try:
                os.kill(self._pid, 0)
                if os.waitpid(self._pid, os.WNOHANG) == (0,0):
                    return True
                else:
                    return False
            except OSError:
                pass
        return False
   
def exit_handler(signum, frame):
    raise SystemExit

if __name__ == '__main__':
    # Grab command line arguments and options, -d is decimation
    import sys, getopt
    # Import the CORBA stuff
    from omniORB import CORBA
    from omniORB import URI
    import CosNaming

    # TODO: Pass in a logging configuration file
    logging.basicConfig()
    logging.getLogger().setLevel(logging.DEBUG)

    logging.debug("CommandWrapper.py launched with %s", sys.argv)

    # Turn the args into a dictionary
    args = sys.argv[1:]
    execparams = {}
    while len(args) > 0:
        try:
            paramid = args.pop(0)
            paramvalue = args.pop(0)
            execparams[paramid] = paramvalue
        except IndexError:
            pass

    print execparams
    if not execparams.has_key('NAMING_CONTEXT_IOR'):
        print "Missing required execparam NAMING_CONTEXT_IOR"
        sys.exit(-1)
    if not execparams.has_key('COMPONENT_IDENTIFIER'):
        print "Missing required execparam COMPONENT_IDENTIFIER"
        sys.exit(-1)
    if not execparams.has_key('NAME_BINDING'):
        print "Missing required execparam NAME_BINDING"
        sys.exit(-1)
    
    orb = None
    signal.signal(signal.SIGINT, exit_handler)
    signal.signal(signal.SIGTERM, exit_handler)

    try:
        try:
            orb = CORBA.ORB_init()
           
            # get the device manager
            rootContext = orb.string_to_object(execparams['NAMING_CONTEXT_IOR'])
            if rootContext == None:
                print "Failed to lookup naming context"
                sys.exit(-1)
            rootContext = rootContext._narrow(CosNaming.NamingContext)

            # get the POA
            obj_poa = orb.resolve_initial_references("RootPOA")
            poaManager = obj_poa._get_the_POAManager()
            poaManager.activate()
            
            # create the main component object
            # TODO: Somehow we should create a CommandWrapperProperties.py file
            # based on the CommandWrapper.prf.xml as part of make
            component_Obj = CommandWrapper_i(orb, execparams, obj_poa)
            component_Var = component_Obj._this()
            
            name = URI.stringToName(execparams['NAME_BINDING'])
            rootContext.rebind(name, component_Var)
       
            orb.run()
        except SystemExit:
            pass
    finally:
        if orb:
            orb.destroy()
