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
import ossie.resource as resource

def __exit_handler(signum, frame):
    # Raise SystemExit - but only the first time we get a signal
    signal.signal(signal.SIGINT, signal.SIG_IGN)
    signal.signal(signal.SIGQUIT, signal.SIG_IGN)
    signal.signal(signal.SIGTERM, signal.SIG_IGN)
    raise SystemExit

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

            if execparams.has_key('LOGGING_CONFIG_URI'):
                load_logging_config_uri(orb, execparams["LOGGING_CONFIG_URI"])
            else:
                try:
                    resource.configureLogging(execparams, None, orb)
                except:
                    logging.basicConfig()
                    logging.getLogger().setLevel(logging.DEBUG)

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

            # Create the component
            component_Obj = serviceclass(execparams["SERVICE_NAME"], execparams)
            servicePOA.activate_object(component_Obj)
            component_Var = component_Obj._this()

            if devMgr != None:
                logging.debug("Registering service with device manager")
                devMgr.registerService(component_Var, execparams["SERVICE_NAME"])
            else:
                print orb.object_to_string(component_Var)

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
            logging.exception("Error while unregistering service")
            
        if component_Obj != None and callable(getattr(component_Obj, "releaseObject", None)):
            try:
                component_Obj.releaseObject()
            except:
                logging.exception("Error releasing service object")
            
    finally:
        if orb:
            orb.destroy()
