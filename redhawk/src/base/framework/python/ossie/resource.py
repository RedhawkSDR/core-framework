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
import traceback
import sys
from ossie.cf import CF
from omniORB import URI, any, CORBA
import ossie.utils.log4py.config
import tempfile
import os
import urlparse
urlparse.uses_netloc.append("sca")
urlparse.uses_query.append("sca")
import time
import threading
import logging

from ossie.properties import PropertyStorage
from ossie.utils import uuid
from ossie.threadedcomponent import ThreadedComponent
import ossie.logger
import containers

import sys
import CosNaming
import CosEventChannelAdmin, CosEventChannelAdmin__POA
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
    def __init__(self, name, repid, type_, fget=None, description=None):
        self.name = name
        self.repid = repid
        self.type_ = type_
        self.fget = fget
        self._attrname = "__port_%s__" % self.name
        if description != None:
            self.__doc__ = description
        elif fget != None:
            self.__doc__ = fget.__doc__
        else:
            self.__doc__ = ""

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



#
# Support Classes for handling PropertyChange Listeners
#
class  _PCL_Except(Exception):
    def __init__(self,msg=""):
        self.message = msg

# Monitoring thread...
class _PropertyChangeThread(ThreadedComponent):
    def __init__(self, parent, delay=.5):
        ThreadedComponent.__init__(self)
        self.setThreadDelay(delay)
        self.parent = parent

    def process(self):
        if self.parent:
            return self.parent._propertyChangeServiceFunction()
        return ThreadedComponent.NOOP
        
# EventChannel Listener to notify
class _EC_PropertyChangeListener(object):
    def __init__(self, ec ):
        self.pub=None
        self.ec = None
        try:
            channel = ec._narrow(CosEventChannelAdmin.EventChannel)
            if channel:
                self.ec = channel
                self.pub = ossie.events.Publisher(self.ec)
            else:
                raise _PCL_Except("Cannot narrow to EventChannel")
        except:
            raise _PCL_Except("Cannot narrow to EventChannel")

    def notify( self, prec, props ):
        retval=0
        try:
            evt = CF.PropertyChangeListener.PropertyChangeEvent( str(uuid.uuid1()),
                                                                 prec.regId,
                                                                 prec.rscId,
                                                                 props)
            if self.pub:
                self.pub.push( evt )
        except:
            retval=-1

        return retval

# PropertyChange Listener to notify
class _INF_PropertyChangeListener(object):
    def __init__(self, obj ):
        self.obj = obj
        self.listener = None
        try:
            pcl = obj._narrow(CF.PropertyChangeListener)
            if pcl:
                self.listener = pcl
            else:
                raise _PCL_Except("Cannot narrow to PropertyChangeListener")

        except:
            raise _PCL_Except("Cannot narrow to PropertyChangeListener")

    def notify( self, prec, props ):
        retval=0
        try:
            evt = CF.PropertyChangeListener.PropertyChangeEvent( str(uuid.uuid1()),
                                                                 prec.regId,
                                                                 prec.rscId,
                                                                 props)
            if self.listener:
                self.listener.propertyChange( evt )
        except:
            retval=-1

        return retval

class _PCL_Monitor(object):
    def __init__(self):
        self.recorded_=False
        self.changed_=False

    def recordChanged(self):
        if self.recorded_ == False :
            self.recorded_=True
            self.changed_=True

    def isChanged(self):
        return self.changed_

    def isSet(self):
        return self.recorded_

    def reset(self):
        self.recorded_=False
        self.changed_=False


class _PCL_MonitorTrack(object):
    def __init__(self, rsc, prop ):
        self.rsc=rsc
        self.prop=prop
        self.old_ = None
        self.diff_ = False
        self.tested_ = False
        try:
            self.old_ = self.prop.get(self.rsc)
        except:
            pass

    def recordChanged(self):
        self.tested_=True
        self.diff_=True
        
    def isChanged(self):
        if self.tested_:
            return self.diff_
        try:
            curr = self.prop.get(self.rsc)
            if  self.old_ != curr:
                self.diff_= True
        except:
            pass
        self.tested_=True
        return self.diff_

    def reset(self):
        try:
            self.old_ = self.prop.get(self.rsc)
        except:
            pass
        self.tested_ = False
        self.diff_ = False

    def isSet(self):
        return self.tested_

class _PropertyChangeRec(object):
    def __init__(self, parent, listener, pcl, props, interval=0.500):
        self.regId = str(uuid.uuid1())
        self.rscId = "UNK_RSC_ID"
        if parent:
            self.rscId = parent._id
        self.listener = listener
        self.pcl = pcl
        self.reportInterval = interval
        self.expiration= time.time() + interval
        self.props = props
        self.parent = parent

    def callback(self, id_, oldvalue, newvalue ):
        if id_ in self.props:
            if self.props[id_]:
                self.props[id_].recordChanged()

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
        #print "resource __init__:"
        #print "id:" + str(identifier)
        #print "loggerName:" + str(loggerName)
        #print "execparams:" + str(execparams)
        self.propertySetAccess = threading.Lock()
        self._id = identifier
        self._started = False
        self._domMgr = None
        self._ecm = None

        ##
        ## logging context for the resource
        ##
        self.logLevel = None
        self._logLevel = None
        self.logConfig = ""
        self.loggingMacros = ossie.logger.GetDefaultMacros()
        ossie.logger.ResolveHostInfo( self.loggingMacros )
        self.loggingCtx = None
        self.loggingURL=None

        if loggerName == None:
            self._logid = execparams.get("NAME_BINDING", self._id )
            self._logid = self._logid.rsplit("_", 1)[0]
        else:
            self._logid = loggerName
        self._logid = self._logid.replace(":","_")
        self._log = logging.getLogger(self._logid)
        loglevel = self._log.getEffectiveLevel()
        if loglevel == logging.NOTSET:
            self.logLevel = logging.INFO
        else:
            self.logLevel = loglevel
        self._logLevel = ossie.logger.ConvertLog4ToCFLevel( self.logLevel )
        self.logListenerCallback=None
        self._name = execparams.get("NAME_BINDING", "")
        # The base resource class manages properties ...
        self._props = PropertyStorage(self, propertydefs, execparams)
        self._props.initialize()

        # property change listener registry and monitoring thread
        self._propChangeRegistry = {}
        self._propChangeThread = _PropertyChangeThread(self)
        self._propMonitors = {}

        # ... and also manages ports
        self.__loadPorts()

        logging.trace("Initial property storage %s", self._props)

        self.__initialized = False
        self.__propertiesInitialized = False

    def setAdditionalParameters(self, softwareProfile, application_registrar_ior, nic):
        self._softwareProfile = softwareProfile
        orb = __orb__
        try:
            obj = orb.string_to_object(application_registrar_ior)
            applicationRegistrar = obj._narrow(CF.ApplicationRegistrar)
            if applicationRegistrar != None:
                self._domMgr = containers.DomainManagerContainer(applicationRegistrar._get_domMgr())

                self._ecm = ossie.events.Manager.GetManager(self)
        except:
            self._domMgr = None
    
    def getDomainManager(self):
        return self._domMgr

    def __init_monitors(self):
        self._propMonitors = {}
        for k, p in self._props.items():
            self._propMonitors[k] = _PCL_MonitorTrack(self,p)
        
    #########################################
    # CF::Resource
    def start(self):
        self._log.trace("start()")
        # Check all ports for a startPort() method, and call it if one exists
        for portdef in self.__ports.itervalues():
            port = portdef.__get__(self)
            if hasattr(port, 'startPort'):
                port.startPort()
        self._started = True

    def stop(self):
        self._log.trace("stop()")
        # Check all ports for a stopPort() method, and call it if one exists
        for portdef in self.__ports.itervalues():
            port = portdef.__get__(self)
            if hasattr(port, 'stopPort'):
                port.stopPort()
        self._started = False

    def _get_identifier(self):
        return self._id

    def _get_started(self):
        return self._started

    def _get_softwareProfile(self):
        return self._softwareProfile

    #########################################
    # CF::LifeCycle
    def initialize(self):
        self._log.trace("initialize()")
        if not self.__initialized:
            self.__initialized = True
            try:
                self.constructor()
                self.__init_monitors()
            except Exception, exc:
                self._log.error("initialize(): %s", str(exc))
                raise CF.LifeCycle.InitializeError([str(exc)])

    def constructor(self):
        pass

    def releaseObject(self):
        self._log.trace("releaseObject()")
        self.stopPropertyChangeMonitor()
        # disable logging that uses EventChannels
        ossie.logger.SetEventChannelManager(None)
        # release all event channels
        if self._ecm: ossie.events.Manager.Terminate()
        objid = self._default_POA().servant_to_id(self)
        self._default_POA().deactivate_object(objid)
        __orb__.shutdown(False)

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

    def getPortSet(self):
       """Return list of ports for this Resource"""
       self._log.trace("getPortSet()")
       portList = []
       for name, portdef in self.__ports.iteritems():
           obj_ptr = self.getPort(name)
           repid = portdef.repid
           description = portdef.__doc__
           direction = ''
           if isinstance(portdef, usesport):
               direction = 'Uses'
           elif isinstance(portdef, providesport):
               if repid == 'IDL:ExtendedEvent/MessageEvent:1.0':
                   direction = 'Bidir'
               else:
                   direction = 'Provides'
           info = CF.PortSet.PortInfoType(obj_ptr, name, repid, description, direction)
           portList.append(info)

       return portList

    def __loadPorts(self):
        self.__ports = {}
        for name in dir(type(self)):
            attr = getattr(type(self), name)
            if isinstance(attr, _port):
                self.__ports[attr.name] = attr

    #########################################
    #  Common resource logging API

    # return logger assigned to resource
    def getLogger(self):
        return self._log

    # return a named logger
    def getLogger(self, logid, assignToResource=False):
        newLogger=logging.getLogger(logid)
        if assignToResource:
            self._logid = logid
            self._log = newLogger
        return newLogger

    # Apply a resource context to the set of logging Macros
    def setLoggingMacros(self, newTbl, applyCtx=False ):
        self.loggingMacros = newTbl
        if applyCtx and self.loggingCtx:
            self.loggingCtx.apply(self.loggingMacros )

    # Apply a resource context to the set of logging Macros
    def setResourceContext(self, rscCtx ):
        if rscCtx:
            rscCtx.apply(self.loggingMacros )
            self.loggingCtx = rscCtx

    def setLoggingContext(self, rscCtx ):
        # apply resource context to macro definitions
        if rscCtx:
            rscCtx.apply(self.loggingMacros )
            self.loggingCtx = rscCtx

        elif self.loggingCtx :
            self.loggingCtx.apply(self.loggingMacros )

        # load logging configuration url to resource
        self.setLogConfigURL( self.loggingURL )

        # apply logging level 
        self.setLogLevel( self._logid, self.loglevel )

    #
    # set the logging context for the resouce. Use the contents of 
    # of the url as the logging configuration data. If the 
    # the file contains any predefined macros then then 
    # apply the resource context object to the macro defintions.
    # After the configuration is established then set the
    # the appropriate level.  The contents of the level correspond
    # to the command line options when a program starts.
    #
    #
    def setLoggingContext(self, logcfg_url, oldstyle_loglevel, rscCtx ):

        # test we have a logging URI
        if logcfg_url==None or logcfg_url=="" :
            ossie.logger.ConfigureDefault()
        else:
            # apply resource context to macro definitions
            if rscCtx:
                rscCtx.apply(self.loggingMacros )
                self.loggingCtx = rscCtx

            # load logging configuration url to resource
            self.setLogConfigURL( logcfg_url )

        # apply logging level if explicitly stated
        if oldstyle_loglevel != None and oldstyle_loglevel > -1 :
            self.setLogLevel( "", ossie.logger.ConvertLogLevel(oldstyle_loglevel) )


    def saveLoggingContext(self, logcfg_url, oldstyle_loglevel, rscCtx ):

        # apply resource context to macro definitions
        if rscCtx:
            rscCtx.apply(self.loggingMacros )
            self.loggingCtx = rscCtx

        # test we have a logging URLx
        self.loggingURL = logcfg_url
        if logcfg_url==None or logcfg_url=="" :
            self.logConfig = ossie.logger.GetDefaultConfig()
        else:
            # try to process URL and grab contents
            try:
                cfg_data=ossie.logger.GetConfigFileContents( logcfg_url )
                if cfg_data and len(cfg_data) > 0 :
                    self.logConfig = ossie.logger.ExpandMacros( cfg_data, self.loggingMacros )
            except:
                pass

        # apply logging level if explicitly stated
        if oldstyle_loglevel != None and oldstyle_loglevel > -1 :
            loglevel = ossie.logger.ConvertLogLevel(oldstyle_loglevel)
            self.setLogLevel( "", loglevel )
            if self._log and self._log.getEffectiveLevel() == logging.NOTSET:
                self.setLevel( self._logid, loglevel )
        else:
            if self._log and self._log.getEffectiveLevel() == logging.NOTSET:
                loglevel = ossie.logger.ConvertLog4ToCFLevel( logging.getLogger(None).getEffectiveLevel() )
                self.setLogLevel( self._logid, loglevel )

        # assign an event channel manager to the logging library
        ossie.logger.SetEventChannelManager( self._ecm )


    def setLogListenerCallback(self, logListenerCB ):
        self.logListenerCallback=logListenerCB
        
    def addPropertyChangeListener(self, id, callback):
        self._props.addChangeListener(callback, id)

    #########################################
    # CF::LogConfiguration
    def _get_log_level(self):
        if self._log:
            lvl = ossie.logger.ConvertLog4ToCFLevel( self._log.getEffectiveLevel())
            if lvl != self._logLevel:
                self.logLevel = self._log.getEffectiveLevel()
                self._logLevel = lvl

        return self._logLevel

    def _set_log_level(self, newLogLevel ):
        self.log_level( newLogLevel )

    def log_level(self, newLogLevel=None ):
        if newLogLevel == None:
            return self._logLevel

        if ossie.logger.SupportedCFLevel(newLogLevel) == False:
           return

        if self.logListenerCallback and callable(self.logListenerCallback.logLevelChanged):
            self.logListenerCallback.logLevelChanged(self._logid, newLogLevel)
        else:
            ossie.logger.SetLogLevel( self._logid, newLogLevel )
            self._logLevel = newLogLevel
            self.logLevel = ossie.logger.ConvertToLog4Level( newLogLevel )

    def setLogLevel(self, logid, newLogLevel ):

        if ossie.logger.SupportedCFLevel(newLogLevel) == False:
            return

        if self.logListenerCallback and callable(self.logListenerCallback.logLevelChanged):
            #self._logLevel = newLogLevel
            #self.logLevel = ossie.logger.ConvertToLog4Level( newLogLevel )
            self.logListenerCallback.logLevelChanged(logid, newLogLevel)
        else:
            ossie.logger.SetLogLevel( logid, newLogLevel )
            if logid == self._logid:
                self._logLevel = newLogLevel
                self.logLevel = ossie.logger.ConvertToLog4Level( newLogLevel )

    def getLogConfig(self):
        return self.logConfig

    def setLogConfig(self, new_log_config):
        if self.logListenerCallback and callable(self.logListenerCallback.logConfigChanged):
            tcfg= ossie.logger.ExpandMacros( new_log_config, self.loggingMacros  )
            if tcfg:
               self.logConfig = tcfg
               self.logListenerCallback.logConfigChanged(tcfg)
        elif new_log_config:
            tcfg= ossie.logger.ConfigureWithContext( new_log_config, self.loggingMacros  )
            if tcfg:
                self.logConfig = tcfg
                lvl=self._get_log_level()
        else:
            pass

    def setLogConfigURL(self, log_config_url):
        if log_config_url:
            fc=ossie.logger.GetConfigFileContents( log_config_url )
            self.loggingURL = log_config_url
            if fc and len(fc) > 0 :
                self.setLogConfig( fc )

    #########################################
    # CF::LogEventConsumer
    def retrieve_records(self, howMany, startingRecord ):
        howMany=0
        return None

    def retrieve_records_by_date(self, howMany, to_timetStamp ):
        howMany=0
        return None

    def retrieve_records_from_date(self, howMany, from_timetStamp ):
        howMany=0
        return None



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
                        prp = self._props.getPropDef(propid)
                        if type(prp) == ossie.properties.struct_property:
                            newvalval = []
                            for v in value.value():
                                if prp.fields[v.id][1].optional == True:
                                    if isinstance(v.value.value(), list):
                                        if v.value.value() != []:
                                            newvalval.append(v)
                                    else:
                                        if v.value.value() != None:
                                            newvalval.append(v)
                                else:
                                    newvalval.append(v)
                            value = CORBA.Any(value.typecode(), newvalval)

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
                        prp = self._props.getPropDef(prop.id)
                        if type(prp) == ossie.properties.struct_property:
                            newvalval = []
                            for v in prop.value.value():
                                if prp.fields[v.id][1].optional == True:
                                    if isinstance(v.value.value(), list):
                                        if v.value.value() != []:
                                            newvalval.append(v)
                                    else:
                                        if v.value.value() != None:
                                            newvalval.append(v)
                                else:
                                    newvalval.append(v)
                            prop.value = CORBA.Any(prop.value.typecode(), newvalval)
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

    def initializeProperties(self, ctorProps):
        self._log.trace("initializeProperties(%s)", ctorProps)

        with self.propertySetAccess:
            # Disallow multiple calls
            if self.__propertiesInitialized:
                raise CF.PropertyEmitter.AlreadyInitialized()
            self.__propertiesInitialized = True

            notSet = []
            for prop in ctorProps:
                try:
                    if self._props.has_id(prop.id) and self._props.isProperty(prop.id):
                        try:
                            # run configure on property.. disable callback feature
                            self._props.construct(prop.id, prop.value)
                        except ValueError, e:
                            self._log.warning("Invalid value provided to construct for property %s %s", prop.id, e)
                            notSet.append(prop)
                    else:
                        self._log.warning("Tried to construct non-existent, readonly, or property with action not equal to external %s", prop.id)
                        notSet.append(prop)
                except Exception, e:
                    self._log.exception("Unexpected exception.")
                    notSet.append(prop)

            if notSet:
                if len(notSet) < len(ctorProps):
                    self._log.warning("Property initialization failed with partial configuration, %s", notSet)
                    raise CF.PropertySet.PartialConfiguration(notSet)
                else:
                    self._log.warning("Property initialization failed with invalid configuration, %s", notSet)
                    raise CF.PropertySet.InvalidConfiguration("Failure", notSet)

        self._log.trace("initializeProperties(%s)", ctorProps)


    def configure(self, configProperties):
        self._log.trace("configure(%s)", configProperties)
        self.propertySetAccess.acquire()
        notSet = []
        error_message = ''
        for prop in configProperties:
            try:
                if self._props.has_id(prop.id) and self._props.isConfigurable(prop.id):
                    try:
                        self._props.configure(prop.id, prop.value)
                    except Exception, e:
                        self._log.warning("Invalid value provided to configure for property %s: %s", prop.id, e)
                        notSet.append(prop)
                else:
                    self._log.warning("Tried to configure non-existent, readonly, or property with action not equal to external %s", prop.id)
                    notSet.append(prop)
            except Exception, e:
                error_message += str(e)
                self._log.exception("Unexpected exception.")
                notSet.append(prop)

        if len(notSet) > 0 and len(notSet) < len(configProperties):
            self.propertySetAccess.release()
            self._log.warning("Configure failed with partial configuration, %s", notSet)
            raise CF.PropertySet.PartialConfiguration(notSet)
        elif len(notSet) > 0 and len(notSet) >= len(configProperties):
            self.propertySetAccess.release()
            self._log.warning("Configure failed with invalid configuration, %s", notSet)
            raise CF.PropertySet.InvalidConfiguration("Failure: "+error_message, notSet)
        self.propertySetAccess.release()
        self._log.trace("configure(%s)", configProperties)

    def registerPropertyListener(self, listener, prop_ids, interval):
        self._log.trace("registerPropertyListener(%s)", prop_ids)
        self.propertySetAccess.acquire()

        
        unknownProperties = []   # list of properties with unknown ids
        props = {}  # maps of callbacks to property ids

        # If the list is empty, get all props
        if prop_ids == []:
            self._log.trace("registering all properties")
            for propid in self._props.keys():
                if self._props.has_id(propid) and self._props.isQueryable(propid):
                    props[propid] = _PCL_Monitor()
        else:
            self._log.trace("registering %s properties", len(prop_ids))
            for pid in prop_ids:
                if self._props.has_id(pid) and self._props.isQueryable(pid):
                    props[pid] = _PCL_Monitor()
                else:
                    value=any.to_any(None)
                    unknownProperties.append(CF.DataType(pid,value))

        if len(unknownProperties) > 0:
            self._log.warning("registerPropertyListener, called with invalid properties %s", unknownProperties)
            self.propertySetAccess.release()
            raise CF.UnknownProperties(unknownProperties)


        pcl = None
        is_ec = False
        try:
            self._log.debug("registerPropertyListener, Registering Event Channel...")
            pcl = _EC_PropertyChangeListener(listener)
            is_ec = True
        except:
            pcl = None
            self._log.debug("registerPropertyListener, Try for PropertyChangeListener next...")

        if is_ec == False:
            try:
                self._log.debug("registerPropertyListener, Trying PropertyChangeListener interface...")
                pcl = _INF_PropertyChangeListener(listener)
            except:
                #print traceback.format_exc()
                pcl = None
                self._log.warning("registerPropertyListener, Caller provided invalid registrant.")
                self.propertySetAccess.release()
                raise CF.InvalidObjectReference("registerPropertyListener, Caller provided invalid registrant.")
            

        # create/add registration record
        rec = _PropertyChangeRec(self, listener, pcl, props, interval )
        reg_id = rec.regId
        self._props.addChangeListener( rec.callback )
        self._propChangeRegistry[reg_id] = rec
        self._log.debug( "registerPropertyListener .. reg_id/interval: " + str(reg_id) + "/" + str(rec.reportInterval) + " callback: "+  str(self._propChangeRegistry[reg_id].callback))
        
        # start 
        if self._propChangeThread and self._propChangeThread.isRunning() == False :
            self._log.debug( "registerPropertyListener  Starting PROPERTY CHANGE THREAD ... resource/reg_id: " + str(self._name) + "/" + str(reg_id) )
            self._propChangeThread.startThread()        

        self.propertySetAccess.release()
        return reg_id

    def unregisterPropertyListener(self, reg_id ):
        self._log.debug("unregisterPropertyListener")
        self.propertySetAccess.acquire()
        if reg_id in self._propChangeRegistry:
            try:
                self._log.debug("unregisterPropertyListener - Remove registration " + str(reg_id) )             
                self._props.removeChangeListener( self._propChangeRegistry[reg_id].callback )
                del self._propChangeRegistry[reg_id]
                self._log.debug("unregisterPropertyListener - Removed registration " + str(reg_id) )

            except:
                 pass

            if len(self._propChangeRegistry) == 0 :
                self.stopPropertyChangeMonitor()
                
            self.propertySetAccess.release()
            self._log.trace("unregisterPropertyListener")
        else:
            self._log.trace("unregisterPropertyListener - end")
            self.propertySetAccess.release()
            raise CF.InvalidIdentifier()


    def startPropertyChangeMonitor(self):
        pass


    def stopPropertyChangeMonitor(self):
        if self._propChangeThread:
            self._log.debug( "Stopping PROPERTY CHANGE THREAD ... ")
            self._propChangeThread.stopThread()

    def _propertyChangeServiceFunction(self):
        self._log.debug("_propertyChangeServiceFunction - START")
        delay = 0.0
        now = time.time()
        self.propertySetAccess.acquire()
        try:
            self._log.debug("_propertyChangeServiceFunction - checking registry.... " + str(len(self._propChangeRegistry)))
            for regid,rec in self._propChangeRegistry.iteritems():

                # process changes for each property
                for k,p in rec.props.iteritems():
                    self._log.debug("_propertyChangeServiceFunction - prop/set. " + str(k) + "/" + str(p.isSet()))
                    try:
                        if self._propMonitors[k].isChanged():
                            p.recordChanged()
                        self._log.debug("_propertyChangeServiceFunction - prop/changed. " + str(k) + "/" + str(self._propMonitors[k].isChanged()))
                    except Exception, e:
                        pass


                # check if time expired
                dur = rec.expiration - now
                if dur <= 0.0:
                    rpt_props = []
                    idx=0
                    # process changes for each property
                    for k,p in rec.props.iteritems():
                        self._log.debug("_propertyChangeServiceFunction - prop/set. " + str(k) + "/" + str(p.isSet()))
                        if  p.isChanged() == True :
                            try:
                                value = self._props.query(k)
                                rpt_props.append( CF.DataType(k,value))
                            except:
                                pass
                        p.reset()

                    if len(rpt_props) > 0  and rec.pcl:
                        try:
                            self._log.debug("_propertyChangeServiceFunction - sending notification reg_id:" + str( regid ))
                            if rec.pcl.notify( rec, rpt_props ) != 0:
                                self._log.error("Publishing changes to PropertyChangeListener FAILED, reg_id:" + str( regid ))
                        except:
                            pass
                    
                    rec.expiration = time.time()+ rec.reportInterval
                    dur = rec.reportInterval

                if  delay == 0.0 : delay = dur
                if dur > 0 : delay = min( delay, dur )
                self._log.debug("_propertyChangeServiceFunction -  delay :" + str( delay ))
        except:
            pass

        # reset monitor's state
        if len( self._propChangeRegistry ) > 0:
           for k,mon in self._propMonitors.iteritems():
               mon.reset()

        self._log.debug("_propertyChangeServiceFunction -  adjust thread delay :" + str( delay ))
        if delay > 0 : self._propChangeThread.setThreadDelay(delay)
        self.propertySetAccess.release()
        
        self._log.debug("_propertyChangeServiceFunction - STOP")
        return -1


def __exit_handler(signum, frame):
    raise SystemExit


def configure_logging(orb, uri, debugLevel=3, binding=None):
    #print "resource configure_logging:"
    #print "orb:" + str(orb)
    #print "uri:" + str(uri)
    #print "debugLevel:" + str(debugLevel)
    #print "binding:" + str(binding)
    if uri:
        load_logging_config_uri(orb, uri, binding)
    else:
        ossie.logger.ConfigureDefault()


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
    #print "configureLogging:"
    #print "orb:" + str(orb)
    #print "loggerName:" + str(loggerName)

    # Configure logging (defaulting to INFO level).
    log_config_uri = execparams.get("LOGGING_CONFIG_URI", None)
    debug_level = execparams.get("DEBUG_LEVEL", 3)
    dom_path=execparams.get("DOM_PATH", "")
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
    name_binding="NOT SET"
    setupSignalHandlers()
    orb = None
    globals()['__orb__'] = orb

    try:
        try:
            orb = createOrb()
            globals()['__orb__'] = orb
            name_binding=""
            component_identifier=""
            
            componentPOA = getPOA(orb, thread_policy, "componentPOA")
            if not execparams.has_key("COMPONENT_IDENTIFIER"):
                if not interactive:
                    logging.warning("No 'COMPONENT_IDENTIFIER' argument provided")
                execparams["COMPONENT_IDENTIFIER"] = ""
            
            if not execparams.has_key("NAME_BINDING"):
                if not interactive:
                    logging.warning("No 'NAME_BINDING' argument provided")
                execparams["NAME_BINDING"] = ""
            
            if not execparams.has_key("PROFILE_NAME"):
                if not interactive:
                    logging.warning("No 'PROFILE_NAME' argument provided")
                execparams["PROFILE_NAME"] = ""

            # Configure logging (defaulting to INFO level).
            log_config_uri = execparams.get("LOGGING_CONFIG_URI", None)
            debug_level = execparams.get("DEBUG_LEVEL", None)
            if debug_level != None: debug_level = int(debug_level)
            dpath=execparams.get("DOM_PATH", "")
            component_identifier=execparams.get("COMPONENT_IDENTIFIER", "")
            name_binding=execparams.get("NAME_BINDING", "")
            category=loggerName
            try:
              if not category and name_binding != "": category=name_binding.rsplit("_", 1)[0]
            except:
                pass 

            ## sets up logging during component startup
            ctx = ossie.logger.ComponentCtx(
                name = name_binding,
                id = component_identifier,
                dpath = dpath )
            ossie.logger.Configure(
                logcfgUri = log_config_uri,
                logLevel = debug_level,
                ctx = ctx,
                category=category )

            # Create the component
            component_Obj = componentclass(execparams["COMPONENT_IDENTIFIER"], execparams)
            componentPOA.activate_object(component_Obj)
            component_Var = component_Obj._this()
            nic = ''
            if execparams.has_key('NIC'):
                nic = execparams['NIC']

            component_Obj.setAdditionalParameters(execparams["PROFILE_NAME"],execparams['NAMING_CONTEXT_IOR'], nic)

            ## sets up logging context for resource to support CF::Logging
            component_Obj.saveLoggingContext( log_config_uri, debug_level, ctx )

            # get the naming context and bind to it
            if execparams.has_key("NAMING_CONTEXT_IOR"):
                try:
                    binding_object = orb.string_to_object(execparams['NAMING_CONTEXT_IOR'])
                except:
                    binding_object = None
                if binding_object == None:
                    logging.error("Failed to lookup application registrar and naming context")
                    sys.exit(-1)

                applicationRegistrar = binding_object._narrow(CF.ApplicationRegistrar)
                if applicationRegistrar == None:
                    name = URI.stringToName(execparams['NAME_BINDING'])
                    rootContext = binding_object._narrow(CosNaming.NamingContext)
                    rootContext.rebind(name, component_Var)
                else:
                    applicationRegistrar.registerComponent(execparams['NAME_BINDING'], component_Var)
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


