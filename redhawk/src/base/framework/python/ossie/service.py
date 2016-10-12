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

"""
"""
import logging
import getopt
import signal
from omniORB import CORBA
from ossie.resource import load_logging_config_uri
from ossie.cf import CF
import ossie.logger
import containers
import types

def __exit_handler(signum, frame):
    # Raise SystemExit - but only the first time we get a signal
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    signal.signal(signal.SIGQUIT, signal.SIG_IGN)
    signal.signal(signal.SIGTERM, signal.SIG_IGN)
    raise SystemExit

#
# Add required features to a python service instance
#
def patchService(target):
    def getDeviceManager(self):
        return self._devMgr
    def getDomainManager(self):
        return self._domMgr

    def initLogging(self, svc_name, loggerName=None):
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
            self._logid = svc_name
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

    # logging context 
    def saveLoggingContext(self, logcfg_url, oldstyle_loglevel, rscCtx ):

        if hasattr(self, 'name') and self.name :
            self.initLogging(self.name)
        elif rscCtx:
            self.initLogging(rscCtx.name)
        else:
           self.initLogging("SERVICE.NAME")


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
            log4level = ossie.logger.ConvertToLog4Level( ossie.logger.ConvertLogLevel(oldstyle_loglevel))
            logging.getLogger("").setLevel( log4level )
            if self._log and self._log.getEffectiveLevel() == logging.NOTSET:
                self._log.setLevel( self._logid, log4level )
        else:
            if self._log and self._log.getEffectiveLevel() == logging.NOTSET:
                self._log.setLevel( self._logid, logging.getLogger(None).getEffectiveLevel() )
                self._logLevel = ossie.logger.ConvertLog4ToCFLevel( logging.getLogger(None).getEffectiveLevel() )

        self._ecm = None
        try:
            self._ecm = ossie.events.Manager.GetManager(self)
        except:
            pass

        # assign an event channel manager to the logging library
        ossie.logger.SetEventChannelManager( self._ecm )


    def releaseObject(self):
        # release EventChannelManager from service use
        ossie.logger.SetEventChannelManager(None)
        if self._ecm: ossie.events.Manager.Terminate()
        self._ecm=None
        if self._releaseObject:
            self._releaseObject()

    def terminateService(self):
        # assign an event channel manager to the logging library
        ossie.logger.SetEventChannelManager(None)
        if self._ecm: ossie.events.Manager.Terminate()
        self._ecm=None
        if self._terminateService:
            self._terminateService()


    target._terminateService=None
    if callable(getattr(target, "terminateService", None)):
        target._terminateService = getattr(target, "terminateService", None)
    target._releaseObject=None
    if callable(getattr(target, "releaseObject", None)):
        target._releaseObject = getattr(target, "releaseObject", None)
    target._ecm = None
    target.getDeviceManager = types.MethodType(getDeviceManager,target)
    target.getDomainManager = types.MethodType(getDomainManager,target)
    target.saveLoggingContext = types.MethodType(saveLoggingContext,target)
    target.initLogging = types.MethodType(initLogging,target)
    target.releaseObject = types.MethodType(releaseObject,target)
    target.terminateService = types.MethodType(terminateService,target)


    
def start_service(serviceclass, thread_policy=None):
    import sys
    import CosNaming
    import copy
    import signal
    import getopt
    
    try:
        # IMPORTANT YOU CANNOT USE gnu_getopt OR OptionParser
        # because they will treat execparams with negative number
        # values as arguments.
        #
        # Since property ids *MUST* be valid XML names
        # they cannot start with -, therefore this is safe
        opts, args = getopt.getopt(sys.argv[1:], "", [""])
    except getopt.GetoptError:
        print "usage: %s [options] [execparams]" % sys.argv[0]
        print
        print serviceclass.__doc__
        sys.exit(2)

    options = {}
    for o, a in opts:
        pass

    # Turn the args into a dictionary
    execparams = {}
    while len(args) > 0:
        try:
            paramid = args.pop(0)
            paramvalue = args.pop(0)
            execparams[paramid] = paramvalue
        except IndexError:
            pass

    orb = None
    signal.signal(signal.SIGINT, __exit_handler)
    signal.signal(signal.SIGQUIT, __exit_handler)
    signal.signal(signal.SIGTERM, __exit_handler)
    try:
        devMgr = None
        component_Obj = None
        component_Var = None
        try:
            orb = CORBA.ORB_init()

            # get the POA
            obj_poa = orb.resolve_initial_references("RootPOA")
            poaManager = obj_poa._get_the_POAManager()

            if thread_policy != None:
                policyList = []
                policyList.append(obj_poa.create_thread_policy(thread_policy))
                servicePOA     = obj_poa.create_POA("servicePOA", poaManager, policyList)
            else:
                servicePOA = obj_poa
            poaManager.activate()
           
            # If provided, get the device manager
            if execparams.has_key("DEVICE_MGR_IOR"):
                devMgr = orb.string_to_object(execparams["DEVICE_MGR_IOR"])
                devMgr = devMgr._narrow(CF.DeviceManager)
           
            if not execparams.has_key("SERVICE_NAME"):
                logging.warning("No 'SERVICE_NAME' argument provided")
                execparams["SERVICE_NAME"] = ""


            # Configure logging context for the service
            name = execparams.get("SERVICE_NAME", "")
            log_config_uri = execparams.get("LOGGING_CONFIG_URI", None)
            debug_level = execparams.get("DEBUG_LEVEL", None)
            if debug_level != None: debug_level = int(debug_level)
            dpath=execparams.get("DOM_PATH", "")
            category=None
            try:
              if name != "": category=name.rsplit("_", 1)[0]
            except:
                pass 
            ctx = ossie.logger.ServiceCtx( name, dpath )
            ossie.logger.Configure( log_config_uri, debug_level, ctx, category )

            # Create the component
            component_Obj = serviceclass(execparams["SERVICE_NAME"], execparams)
            servicePOA.activate_object(component_Obj)
            component_Var = component_Obj._this()

            # add required methods
            patchService(component_Obj)
            
            if devMgr != None:
                logging.debug("Registering service with device manager")
                #patchService(component_Obj)
                component_Obj._devMgr = containers.DeviceManagerContainer(devMgr)
                component_Obj._domMgr = containers.DomainManagerContainer(devMgr._get_domMgr())
                devMgr.registerService(component_Var, execparams["SERVICE_NAME"])
            else:
                print orb.object_to_string(component_Var)

            ## sets up logging context for resource to support CF::Logging
            component_Obj.saveLoggingContext( log_config_uri, debug_level, ctx )

            # Run the blocking main loop
            logging.debug("Starting ORB event loop")
            orb.run()
        except SystemExit:
            pass
        except KeyboardInterrupt:
            pass
        except:
            logging.exception("Unexpected Error")
        try:
            if devMgr != None:
                devMgr.unregisterService(component_Var, execparams["SERVICE_NAME"])
        except:
            logging.warning("Error while unregistering service")
            
        if component_Obj != None and callable(getattr(component_Obj, "terminateService", None)):
            try:
                component_Obj.terminateService()
            except:
                logging.warning("Error releasing service object")
                
        # Call to a deprecated exit function.
        if component_Obj != None and callable(getattr(component_Obj, "releaseObject", None)):
            try:
                component_Obj.releaseObject()
            except:
                logging.warning("Error releasing service object")
            
    finally:
        if orb:
            orb.destroy()
