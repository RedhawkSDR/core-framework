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
from omniORB import any, CORBA
import omniORB
hasEvents = True
try:
    import CosEventComm__POA
    import CosEventChannelAdmin
    from ossie.cf import StandardEvent
except:
    hasEvents = False

import ossie.resource as resource
import ossie.utils
import ossie.logger
from  ossie.events import Publisher
from  ossie.events import  Manager
import sys
import logging
import signal
import os
import stat
import commands
import threading
import exceptions
from Queue import Queue
import time
import traceback
import zipfile
import containers


if hasEvents:
    # Map CF.Device states to StandardEvent states for sending state change messages.
    stateMap = { CF.Device.IDLE          : StandardEvent.IDLE,
                 CF.Device.ACTIVE        : StandardEvent.ACTIVE,
                 CF.Device.BUSY          : StandardEvent.BUSY,
                 CF.Device.LOCKED        : StandardEvent.LOCKED,
                 CF.Device.UNLOCKED      : StandardEvent.UNLOCKED,
                 CF.Device.SHUTTING_DOWN : StandardEvent.SHUTTING_DOWN }

    class Supplier_i(CosEventComm__POA.PushSupplier):
        def disconnect_push_supplier (self):
            print "Push Supplier: disconnected."

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

def _checkpg(pid):
    """
    Checks if any members of a process group are alive.
    """
    try:
        os.killpg(pid, 0)
        return True
    except OSError:
        return False

class sharedLibraryStorage():
    _filename = ''
    _modifications = []
    
    def __init__(self, filename):
        self._filename = filename

    def addModification(self, path_to_modify, path_modification):
        self._modifications.append((path_to_modify, path_modification))

class envStateContainer():
    ld_lib_path = ''
    pythonpath = ''
    classpath = ''
    octave_path = ''
    
    def __init__(self):
        if (os.getenv("LD_LIBRARY_PATH")):
            self.ld_lib_path = os.getenv("LD_LIBRARY_PATH")
        else:
            self.ld_lib_path = ''
        if (os.getenv("PYTHONPATH")):
            self.pythonpath = os.getenv("PYTHONPATH")
        else:
            self.pythonpath = ''
        if (os.getenv("CLASSPATH")):
            self.classpath = os.getenv("CLASSPATH")
        else:
            self.classpath = ''
        if (os.getenv("OCTAVE_PATH")):
            self.octave_path = os.getenv("OCTAVE_PATH")
        else:
            self.octave_path = ''
    
    def set(self):
        os.putenv("LD_LIBRARY_PATH", self.ld_lib_path)
        os.putenv("PYTHONPATH", self.pythonpath)
        os.putenv("CLASSPATH", self.classpath)
        os.putenv("OCTAVE_PATH", self.octave_path)
        os.environ["LD_LIBRARY_PATH"] = self.ld_lib_path
        os.environ["PYTHONPATH"] = self.pythonpath
        os.environ["CLASSPATH"] = self.classpath
        os.environ["OCTAVE_PATH"] = self.octave_path

class envState():
    ld_lib_path = ''
    pythonpath = ''
    classpath = ''
    octave_path = ''
    def __enter__(self):
        if (os.getenv("LD_LIBRARY_PATH")):
            self.ld_lib_path = os.getenv("LD_LIBRARY_PATH")
        else:
            self.ld_lib_path = ''
        if (os.getenv("PYTHONPATH")):
            self.pythonpath = os.getenv("PYTHONPATH")
        else:
            self.pythonpath = ''
        if (os.getenv("CLASSPATH")):
            self.classpath = os.getenv("CLASSPATH")
        else:
            self.classpath = ''
        if (os.getenv("OCTAVE_PATH")):
            self.octave_path = os.getenv("OCTAVE_PATH")
        else:
            self.octave_path = ''
            
    def __exit__(self,exc_type,exc_value,traceback):
        os.putenv("LD_LIBRARY_PATH", self.ld_lib_path)
        os.putenv("PYTHONPATH", self.pythonpath)
        os.putenv("CLASSPATH", self.classpath)
        os.putenv("OCTAVE_PATH", self.octave_path)
        os.environ["LD_LIBRARY_PATH"] = self.ld_lib_path
        os.environ["PYTHONPATH"] = self.pythonpath
        os.environ["CLASSPATH"] = self.classpath
        os.environ["OCTAVE_PATH"] = self.octave_path

class Device(resource.Resource):
    """A basic device implementation that deals with the core SCA requirements for a device.
    You are required to implement:

      query()
      configure()

    However, it is recommended that you use the appropriate MixIns.  The MixIns
    are kept separate to allow lightweight customization of those functions in
    your implementation when using the MixIn is undesirable.

    You may override:

      runTest()
      getPort()

    as the default implementations do nothing.

    If you override:

      initialize()
      releaseObject()

    you *must* call ensure you call the base-class functionality for proper operation of the device.
    """

    def __init__(self, devmgr, identifier, label, softwareProfile, compositeDevice, execparams, propertydefs=(),loggerName=None):
        if not loggerName and label: loggerName = label.rsplit("_", 1)[0]
        resource.Resource.__init__(self, identifier, execparams, propertydefs, loggerName=loggerName)
        self._log.debug("Initializing Device %s %s %s %s", identifier, execparams, propertydefs, loggerName)
        self._label = label
        self._name = label
        self._softwareProfile = softwareProfile
        self._devmgr = devmgr
        self._devMgr = containers.DeviceManagerContainer(devmgr)
        self._domMgr = containers.DomainManagerContainer(devmgr._get_domMgr())
        self._compositeDevice = compositeDevice
        self._capacityLock = threading.RLock()
        self._idm_publisher = None
        self._cmdLock = threading.RLock()
        self._allocationCallbacks = {}

        self.__initialize()
    
    class allocationCallbackStructDef():
        _allocate = None
        _deallocate = None
        def __init__(self, alloc, dealloc):
            self._allocate = alloc
            self._deallocate = dealloc
    
    def getDeviceManager(self):
        return self._devMgr
    
    def setAllocationImpl(self, prop_name, alloc_callback, dealloc_callback):
        self._allocationCallbacks[prop_name] = self.allocationCallbackStructDef(alloc_callback, dealloc_callback)

    def registerDevice(self):
        """This function registers the device with the device manager.
        This should be called by the process that instantiates the device.
        """
        if self._devmgr:
            self._log.info("Registering Device:" + str(self._label) )
            self._register()

        if self._compositeDevice:
            self._log.info("Adding Device:" + str(self._label)  + " to parent" )
            deviceAdded = False
            while not deviceAdded:
                try:
                    self._compositeDevice.addDevice(self._this())
                    deviceAdded = True
                except:
                    time.sleep(0.01)

    def postConstruction(self, registrar_ior=None, idm_ior=None ):
        # resolves Domain and Device Manger relationships
        # resolveDomainAwareness( registrar_ior ) -- done during CTOR 

        try:
            if self._domMgr:
                self._ecm = ossie.events.Manager.GetManager(self)
        except:
            pass

        # establish IDM Channel connectivity
        self.connectIDMChannel( idm_ior );

        # register ourself with my DeviceManager
        self.registerDevice()


    def connectIDMChannel(self, idm_ior=None ):
        self._log.debug("Connecting to IDM CHANNEL idm_ior:" + str(idm_ior) )

        if self._idm_publisher == None:
            if idm_ior != None and idm_ior != "":
                # Get DomainManager incoming event channel and connect the device to it,
                # where applicable.
                try:
                    
                    idm_channel_obj = resource.createOrb().string_to_object(idm_ior)
                    idm_channel = idm_channel_obj._narrow(CosEventChannelAdmin.EventChannel)
                    self._idm_publisher = Publisher( idm_channel )
                    self._log.info("Connected to IDM CHANNEL, (command line IOR).... DEV-ID:" + self._id )
                except Exception, err:
                    #traceback.print_exc()
                    self._log.warn("Unable to connect to IDM channel (command line IOR).... DEV-ID:" + self._id )
            else:
                try:
                    # make sure we have access to get to the EventChanneManager for the domain
                    if self._domMgr:
                        if self._ecm == None:
                            self._log.debug("Setting up EventManager .... DEV-ID:" + self._id )
                            evt_mgr= Manager.GetManager(self)
                            self._ecm = evt_mgr
                        else:
                            evt_mgr = self._ecm
                        self._log.debug("Requesting registration with IDM Channel .... DEV-ID:" + self._id )
                        self._idm_publisher = evt_mgr.Publisher( ossie.events.IDM_Channel_Spec )
                        self._log.info("Registered with IDM CHANNEL (Domain::EventChannelManager).... DEV-ID:" + self._id )
                except:
                    #traceback.print_exc()
                    self._log.warn("Unable to connect to IDM channel (Domain::EventChannelManager).... DEV-ID:" + self._id )




    #########################################
    # CF::LifeCycle
    def initialize(self):
        resource.Resource.initialize(self)
        self.__initialize()

    def releaseObject(self):
        self._log.debug("releaseObject()")
        if self._adminState == CF.Device.UNLOCKED:
            self._adminState = CF.Device.SHUTTING_DOWN

        try:
            # release all of the child devices
            # if they have included the AggregateDeviceMixIn
            try:
                childDevice = self._childDevices
            except AttributeError:
                pass
            else:
                while len(self._childDevices)>0:
                    child = self._childDevices.pop()
                    child.releaseObject()
            # remove device from parent and set compositeDevice to None
            if self._compositeDevice:
                self._compositeDevice.removeDevice(self._this())
                self._compositeDevice = None

            self._unregister()

        except Exception, e:
            raise CF.LifeCycle.ReleaseError(str(e))

        self._adminState = CF.Device.LOCKED
        try:
            self._cmdLock.release()
        except:
            pass
        try:
            resource.Resource.releaseObject(self)
        except:
            self._log.error("failed releaseObject()")

    ###########################################
    # CF::Device
    def _validateAllocProps(self, properties):
        self._log.debug("validating")
        # Validate before trying to consume
        for prop in properties:
            try:
                if not self._props.isAllocatable(prop.id):
                    raise exceptions.Exception()
            except:
                self._log.error("Property %s is not allocatable", prop.id)
                raise CF.Device.InvalidCapacity("Invalid capacity %s" % prop.id, [prop])

    def _allocateCapacities(self, propDict={}):
        """
        Allocates all the properties in the dictionary.  If the device has a
        allocateCapacities(propDict) method it gets invoked.  If no method is 
        provided then it invokes individual methods my calling the 
        _allocateCapacity which in turns calls the allocate_<prop name> method  
        
        Input:
            <properties>    A list of CF.DataType properties to allocate
            
        Output:
            Returns true if all the allocations were done successfully or false
            otherwise        
        """
        
        self._log.debug("_allocateCapacities(%s)" % str(propDict))
        if self.isEnabled() and self.isUnLocked():
            successfulAllocations = {}
            self._capacityLock.acquire()
            try:
                # determines if the allocateCapacities method exists 
                allocate = _getCallback(self, 'allocateCapacities')
                # if it does, then use it
                if allocate:
                    success = allocate(propDict)
                # if the device does not have allocateCapacities, then try
                # individually
                else:           
                    for key, val in propDict.iteritems():
                        propname = self._props.getPropName(key)
                        success = self._allocateCapacity(propname, val)
                        if success:
                            successfulAllocations[key] = val
                        else:
                            self._log.debug("property %s, could not be set to %s" % 
                                            (key, val))
                            break
                    # If we couldn't allocate enough capacity, add it back
                    success = len(successfulAllocations) == len(propDict)
                
                    # if the allocations were not successful, then deallocate 
                    if not success:
                        self._log.debug("failed")
                        self._deallocateCapacities(successfulAllocations)
                        
            finally:
                self._capacityLock.release()
                
            self._log.debug("update")
            # Update usage state
            self.updateUsageState()
            self._log.debug("allocateCapacity() --> %s", success)
            return success
        else:
            self._log.error("allocate capacity failed due to InvalidState")
            self._log.debug("%s %s %s", self._adminState, self._operationalState, self._usageState)
            raise CF.Device.InvalidState("System is not ENABLED and UNLOCKED")
        
    
    def _allocateCapacity(self, propname, value):
        """Override this if you want if you don't want magic dispatch"""
        self._log.debug("_allocateCapacity(%s, %s)", propname, value)
        if self._allocationCallbacks.has_key(propname):
            return self._allocationCallbacks[propname]._allocate(value)
        
        modified_propname = ''
        for ch in propname:
            if ch.isalnum():
                modified_propname += ch
            else:
                modified_propname += '_'
        allocate = _getCallback(self, "allocate_%s" % modified_propname)
        if allocate:
            self._log.debug("using callback for _allocateCapacity()", )
            return allocate(value)
        else:
            self._log.debug("no callback for _allocateCapacity()", )
            return False

    def updateUsageState(self):
        """
        This is called automatically after allocateCapacity or deallocateCapacity are called.
        Your implementation should determine the current state of the device:
           self._usageState = CF.Device.IDLE   # not in use
           self._usageState = CF.Device.ACTIVE # in use, with capacity remaining for allocation
           self._usageState = CF.Device.BUSY   # in use, with no capacity remaining for allocation
        """        
        raise NotImplementedError
            
            
    def allocateCapacity(self, properties):
        """
        Takes the list of properties and turns it into a dictionary.  If the 
        device has a allocateCapacities(propDict) method it is invoked.  The 
        method should return a boolean flag indicating whether all the 
        allocations were done successfully or not
        
        Input:
            <properties>    A list of CF.DataType properties to allocate
            
        Output:
            Returns true if all the allocations were done successfully or false
            otherwise
        """
        self._log.debug("allocateCapacity(%s)", properties)
        # Validate
        self._validateAllocProps(properties)
        # Consume
        propdict = {}
        for prop in properties:
            propdef = self._props.getPropDef(prop.id)
            propdict[prop.id] = propdef._fromAny(prop.value)
        try:
            retval = self._allocateCapacities(propdict)
            return retval
        except CF.Device.InvalidCapacity:
            raise # re-raise valid exceptions
        except CF.Device.InvalidState:
            raise  # re-raise valid exceptions
        except Exception, e:
            self._log.exception("Unexpected error in _allocateCapacities: %s", str(e))
            return False


    def _deallocateCapacities(self, propDict):
        """
        Deallocates all the properties in the dictionary.  If the device has a
        deallocateCapacities(propDict) method it gets invoked.  If no method is
        provided then it invokes individual methods my calling the
        _deallocateCapacity which in turns calls the deallocate_<prop name>
        method
        
        Input:
            <properties>    A list of CF.DataType properties to allocate
            
        Output:
            None
        """
        self._log.debug("_deallocateCapacities(%s)" % str(propDict))
        if self.isEnabled() and self.isUnLocked():
            # Determine if the deallocateCapacities method exists 
            deallocate = _getCallback(self, 'deallocateCapacities')
            if deallocate:
                # If it does, then use it
                deallocate(propDict)
            else:
                # If the device does not have deallocateCapacities, then try
                # individually
                for id, val in propDict.iteritems():
                    propname = self._props.getPropName(id)
                    self._deallocateCapacity(propname, val)
        else:
            self._log.error("deallocate capacity failed due to InvalidState")
            self._log.debug("%s %s %s", self._adminState, self._operationalState, self._usageState)
            raise CF.Device.InvalidState("Cannot deallocate capacity. System is not DISABLED and UNLOCKED")

    def _deallocateCapacity(self, propname, value):
        """Override this if you want if you don't want magic dispatch"""
        methodName = "deallocate_%s" % propname.replace(" ", "_")
        if self._allocationCallbacks.has_key(propname):
            return self._allocationCallbacks[propname]._deallocate(value)
        deallocate = _getCallback(self, methodName)
        if deallocate:
            deallocate(value)

    def deallocateCapacity(self, properties):
        """
        Takes the list of properties and turns it into a dictionary.  If the 
        device has a deallocateCapacities(propDict) method it is invoked.
        
        Input:
            <properties>    A list of CF.DataType properties to allocate
            
        Output:
            None
        """
        self._log.debug("deallocateCapacity(%s)", properties)
        # Validate
        self._validateAllocProps(properties)
        # Consume
        propdict = {}
        for prop in properties:
            propdef = self._props.getPropDef(prop.id)
            propdict[prop.id] = propdef._fromAny(prop.value)

        self._capacityLock.acquire()
        try:
            self._deallocateCapacities(propdict)
        finally:
            self._capacityLock.release()

        # Update usage state
        self.updateUsageState()

        self._log.debug("deallocateCapacity() -->")

    def _get_usageState(self):
        return self._usageState

    def _get_adminState(self):
        return self._adminState

    # Legal state transitions for _set_adminState
    _adminStateTransitions = (
        (CF.Device.LOCKED, CF.Device.UNLOCKED),
        (CF.Device.UNLOCKED, CF.Device.LOCKED)
    )

    def _set_adminState(self, state):
        if (self._adminState, state) not in Device._adminStateTransitions:
            self._log.debug("Ignoring invalid admin state transition %s->%s", self._adminState, state)
            return
        self._adminState = state

    def _get_operationalState(self):
        return self._operationalState

    def _get_compositeDevice(self):
        return self._compositeDevice

    def _get_softwareProfile(self):
        return self._softwareProfile

    def _get_label(self):
        return self._label

    ###########################################
    # Helper Methods
    def _unregister(self):
        """
        Unregister with the DeviceManager.  This has the potential to timeout as
        omniORB will sometimes hang on the unregisterDevice call if there is a 
        lack of available threads on the system.

        """
        def _logUnregisterFailure(msg = ""):
            self._log.error("Could not unregister from DeviceManager: %s", msg)
                
        def _unregisterThreadFunction():
            if self._devmgr:
                self._log.debug("Unregistering from DeviceManager")
                try:
                    self._devmgr.unregisterDevice(self._this())
                except CORBA.Exception, e:
                    _logUnregisterFailure(str(e))
                # put something on the queue to indicate that we either 
                # successfully unregistered, or that we have already 
                # logged an error.
                queue.put(True)

        queue = Queue(maxsize=1) 

        success = resource.callOmniorbpyWithTimeout(_unregisterThreadFunction, queue)

        if not success:
            _logUnregisterFailure("timeout while attempting to unregister")

    def _register(self):
        """
        iRegister with the DeviceManager.  This has the potential to timeout as
        omniORB will sometimes hang on the registerDevice call if there is a 
        lack of available threads on the system.

        """
        def _logRegisterFailure(msg = ""):
            self._log.error("Could not register with DeviceManager: %s", msg)

        def _logRegisterWarning(msg = ""):
            self._log.warn("May not have registered with DeviceManager: %s", msg)
                
        def _registerThreadFunction():
            if self._devmgr:
                self._log.debug("Registering with DeviceManager")
                try:
                    self._devmgr.registerDevice(self._this())
                except CORBA.Exception, e:
                    _logRegisterFailure(str(e))
                # put something on the queue to indicate that we either 
                # successfully registered, or that we have already 
                # logged an error.
                queue.put(True)

        success = False
        queue = Queue(maxsize=1) 

        success = resource.callOmniorbpyWithTimeout(_registerThreadFunction, queue, timeoutSeconds = 10)

        if not success:
            _logRegisterWarning("Threaded client call may have timed out while attempting to register")

    def __initialize(self):
        self._usageState = CF.Device.IDLE
        self._adminState = CF.Device.UNLOCKED
        self._operationalState = CF.Device.ENABLED


    def __sendStateChangeEvent(self, eventType, fromState, toState):
        if self._idm_publisher is None:
            # Gracefully handle an unconnected event channel
            return

        try:
            event = StandardEvent.StateChangeEventType(self._id, self._id, eventType, stateMap[fromState], stateMap[toState])

        except:
            self._log.warn("Error creating StateChangeEvent")

        try:
            if self._idm_publisher.push(any.to_any(event)) != 0 :
                self._log.debug("Sending state change event......" + str((self._id, self._id, eventType, stateMap[fromState], stateMap[toState])) )
        except:
            self._log.warn("Error sending event")

    def __setattr__ (self, attr, value):
        # Detect when the usage state and administrative state change, so that
        # the device can attempt to send a state change event.
        if attr == "_usageState":
            try:
                if value != self._usageState:
                    self.__sendStateChangeEvent(StandardEvent.USAGE_STATE_EVENT, self._usageState, value)
            except AttributeError:
                # self._usageState does not exist yet, no need to send an event.
                pass
        elif attr == "_adminState":
            try:
                if value != self._adminState:
                    self.__sendStateChangeEvent(StandardEvent.ADMINISTRATIVE_STATE_EVENT, self._adminState, value)
            except AttributeError:
                # self._adminState does not exist yet, no need to send an event.
                pass

        # Use the base class behavior to actually store the attribute value.
        resource.Resource.__setattr__(self, attr, value)

    def isLocked(self):
        if self._adminState == CF.Device.LOCKED: return True
        return False

    def isUnLocked(self):
        if self._adminState == CF.Device.UNLOCKED: return True
        return False

    def isEnabled(self):
        if self._operationalState == CF.Device.ENABLED: return True
        return False

    def isDisabled(self):
        if self._operationalState == CF.Device.DISABLED: return True
        return False

    def isBusy(self):
        if self._usageState == CF.Device.BUSY: return True
        return False

    def isIdle(self):
        if self._usageState == CF.Device.IDLE: return True
        return False

class LoadableDevice(Device):
    def __init__(self, devmgr, identifier, label, softwareProfile, compositeDevice, execparams, propertydefs=(),loggerName=None):
        Device.__init__(self, devmgr, identifier, label, softwareProfile, compositeDevice, execparams, propertydefs,loggerName=loggerName)
        self._loadedFiles = {}
        # Acquire this lock before modifying the device cache
        self.__cacheLock = threading.Lock()
        self._sharedPkgs = {}
        self.initialState = envStateContainer()

    def releaseObject(self):
        self._unloadAll()
        Device.releaseObject(self)

    ###########################################
    # CF::LoadableDevice
    def load(self, fileSystem, fileName, loadType):
        try:
            self._cmdLock.acquire()
            loadedPaths = []
            self._log.debug("load(%s, %s)", fileName, loadType)
            if not fileName.startswith("/"):
                raise CF.InvalidFileName(CF.CF_EINVAL, "Filename must be absolute, given '%s'"%fileName)

            # SR:429
            if self.isLocked():   raise CF.Device.InvalidState("System is locked down, can not load '%s'"%fileName)
            if self.isDisabled(): raise CF.Device.InvalidState("System is disabled, can not load '%s'"%fileName)


            # SR:430
            if loadType == CF.LoadableDevice.EXECUTABLE or loadType == CF.LoadableDevice.SHARED_LIBRARY:
                localpath = os.path.join(os.getcwd(), fileName)

                loadPoint = ""
                head, tail = os.path.split(os.path.normpath(fileName))
                dirs = head[1:].split("/")

                self.__cacheLock.acquire()
                try:
                    for dir in dirs:
                        if dir != "":
                            self._log.debug("Creating dir %s", dir)
                            loadPoint = os.path.join(loadPoint, dir)
                            if not os.path.exists(loadPoint):
                                os.mkdir(loadPoint)
                    try:
                        refCnt, loadedFiles = self._loadedFiles[fileName]
                    except KeyError:
                        refCnt = 0
                        loadedFiles = []

                    localFilePath = os.path.join(loadPoint, os.path.basename(fileName))
                    exist = os.path.exists(localFilePath)
                    # If this is a directory, make sure that all of the sub-directories
                    # and files exist
                    for loadedFile in loadedFiles:
                        if exist == False:
                            break
                        exist = exist & os.path.exists(loadedFile)
                    self._log.debug("File %s has reference count %s and local file existence is %s", fileName, refCnt, exist)

                    # Check if the remote file is newer than the local file, and if so, update the file
                    # in the cache. No consideration is given to clock sync differences between systems.
                    if exist and self._modTime(fileSystem, fileName) > os.path.getmtime(localFilePath):
                        self._log.debug("Remote file is newer than local file")
                        exist = False
                    if refCnt == 0 or not exist:
                        loadedFiles = self._loadTree(fileSystem, os.path.normpath(fileName), loadPoint)

                    # SR:428
                    self._loadedFiles[fileName] = (refCnt + 1, loadedFiles)
                finally:
                    self.__cacheLock.release()
            else:
                raise CF.LoadableDevice.InvalidLoadKind()

            # If we're loading a shared library, try to set up any language-specific environment vars.
            if loadType == CF.LoadableDevice.SHARED_LIBRARY:
                self._setEnvVars(localFilePath, fileName)

        except Exception, e:
            self._log.exception(e)
            raise CF.LoadableDevice.LoadFail(CF.CF_EINVAL, "Unknown Error loading '%s'"%fileName)
        finally:
            self._cmdLock.release()
            

    def _getEnvVarAsList(self, var):
        # Split the path up
        if os.environ.has_key(var):
            path = os.environ[var].split(os.path.pathsep)
        else:
            path = []
        return path

    def _prependToEnvVar(self, newVal, envVar):
        path = self._getEnvVarAsList(envVar)
        foundValue = False
        for entry in path:
            # Search to determine if the new value is already in the path
            try:
                if os.path.samefile(entry, newVal):
                    # The value is already in the path
                    foundValue = True
                    break
            except OSError:
                # If we can't find concrete files to compare, fall back to string compare
                if entry == newVal:
                    # The value is already in the path
                    foundValue = True
                    break

        if not foundValue:
            # The value does not already exist
            if os.environ.has_key(envVar):
                newpath = newVal+os.path.pathsep + os.getenv(envVar)+os.path.pathsep
            else:
                newpath = newVal+os.path.pathsep
            os.putenv(envVar, newpath)
            os.environ[envVar] = newpath

    def _update_selected_paths(self, selected_paths):
        for path in selected_paths:
            for mod in path._modifications:
                path_to_modify = mod[0]
                path_modification = mod[1]
                self._prependToEnvVar(path_modification, path_to_modify)
    
    def _setEnvVars(self, localFilePath, fileName):
        env_changes = sharedLibraryStorage(fileName)
        matchesPattern = False
        # check to see if it's a C shared library
        status, output = commands.getstatusoutput('nm '+localFilePath)
        if status == 0:
            # Assume this is a C library

            subdirs = os.path.abspath(localFilePath).split('/')

            # strip off .so filename
            subdirs = subdirs[:-1]

            # reconstruct the string from the list
            pathToAdd = ""
            for substring in subdirs:
                pathToAdd += substring + '/'

            env_changes.addModification('LD_LIBRARY_PATH', pathToAdd)

            matchesPattern = True

        # check to see if it's a python module
        try:
            currentdir=os.getcwd()
            subdirs = localFilePath.split('/')
            currentIdx = 0
            if len(subdirs) != 1:
                aggregateChange = ''
                while currentIdx != len(subdirs)-1:
                    aggregateChange += subdirs[currentIdx]+'/'
                    currentIdx += 1
                os.chdir(aggregateChange)
            foundRoot = False
            for entry in sys.path:
                if entry == '.':
                    foundRoot = True
                    break
            if not foundRoot:
                sys.path.append('.')
            importFile = subdirs[-1]

            path = self._getEnvVarAsList('PYTHONPATH')
            newFileValue = ''
            if importFile[-3:] == '.py':
                exec('import '+importFile[:-3])
                newFileValue = importFile[:-3]
            elif importFile[-4:] == '.pyc':
                exec('import '+importFile[:-4])
                newFileValue = importFile[:-4]
            else:
                exec('import '+importFile)
                newFileValue = importFile
            candidatePath = currentdir+'/'+aggregateChange
            env_changes.addModification('PYTHONPATH', candidatePath)
            self._log.debug("PYTHONPATH : ADDING:" + str(candidatePath) + " PATH:" + str(os.environ['PYTHONPATH']) )

            matchesPattern = True
        except:
            # This is not a python module
            pass

        os.chdir(currentdir)
        # check to see if it's a java package
        if localFilePath[-4:] == '.jar' and zipfile.is_zipfile(localFilePath):
            currentdir=os.getcwd()
            subdirs = localFilePath.split('/')
            candidatePath = currentdir+'/'+localFilePath
            env_changes.addModification('CLASSPATH', candidatePath)
            #self._prependToEnvVar(candidatePath, 'CLASSPATH')
            matchesPattern = True

        # it matches no patterns. Assume that it's a set of libraries
        if not matchesPattern:

            path = self._getEnvVarAsList('LD_LIBRARY_PATH')

            # Get an absolute path for localFilePath; look for a duplicate of
            # this path before appending
            candidatePath = os.path.abspath(localFilePath)

            env_changes.addModification('LD_LIBRARY_PATH', candidatePath)
            env_changes.addModification('OCTAVE_PATH', candidatePath)
            
        self._update_selected_paths([env_changes])
        self._sharedPkgs[env_changes._filename] = env_changes

    def _modTime(self, fileSystem, remotePath):
        try:
            fileInfo = fileSystem.list(remotePath)[0]
            for prop in fileInfo.fileProperties:
                if prop.id == 'MODIFIED_TIME':
                    return any.from_any(prop.value)
        except:
            pass
        return 0

    def _copyFile(self, fileSystem, remotePath, localPath):
        self._log.debug("Copy file %s -> %s", remotePath, os.path.abspath(localPath))
        modifiedName = None
        fileToLoad = fileSystem.open(remotePath, True)
        try:
            f = open(localPath, "w+")
        except Exception, e:
            if "Text file busy" in e:
                modifiedName = localPath+"_"+str(time.time()).split('.')[0]
                os.rename(localPath, modifiedName)
                f = open(localPath, "w+")
            else:
                fileToLoad.close();
                raise
        fileSize = fileToLoad.sizeOf()
        floorFileTransferSize=1024*1024
        while fileSize > 0:
            toRead = min(floorFileTransferSize, fileSize)
            buf = fileToLoad.read(toRead)
            if len(buf) == 0:
                break
            f.write(buf)
            fileSize = fileSize - len(buf)
        fileToLoad.close()
        f.close()
        return modifiedName

    def _loadTree(self, fileSystem, remotePath, localPath):
        # OSSIE CUSTOM BEHAVIOR
        # This is a breadth-first load
        loadedFiles = []
        fis = fileSystem.list(remotePath)
        self._log.debug("Loading Tree %s %s %s", remotePath, localPath, fis)
        if len(fis) == 0:
            # check to see if this is an empty directory
            if remotePath[-1] == '/':
                fis = fileSystem.list(remotePath[:-1])
                return loadedFiles
            # SR:431
            self._log.error("File %s could not be loaded", remotePath)
            raise CF.InvalidFileName(CF.CF_EINVAL, "File could not be found %s" % remotePath)

        for fileInformation in fis:
            if fileInformation.kind == CF.FileSystem.PLAIN:
                localFile = os.path.join(localPath, fileInformation.name)
                self._log.debug("Reading file %s -> %s", fileInformation.name, localFile)
                if remotePath.endswith("/"):
                    modified_file = self._copyFile(fileSystem, remotePath + fileInformation.name, localFile)
                else:
                    modified_file = self._copyFile(fileSystem, remotePath, localFile)
                loadedFiles.append(localFile)
                if modified_file != None:
                    loadedFiles.append(modified_file)
                for prop in fileInformation.fileProperties:
                    if prop.id == 'EXECUTABLE':
                        if prop.value._v == True:
                            command = localFile
                            os.chmod(command, os.stat(command)[0] | stat.S_IEXEC | stat.S_IREAD | stat.S_IWRITE)
            elif fileInformation.kind == CF.FileSystem.DIRECTORY:
                localDirectory = os.path.join(localPath, fileInformation.name)
                if not os.path.exists(localDirectory):
                    self._log.debug("Making directory %s", localDirectory)
                    os.mkdir(localDirectory)
                if remotePath.endswith("/"):
                    self._log.debug("From %s loading directory %s -> %s", remotePath, fileInformation.name, localPath)
                    loadedFiles.append(localDirectory)
                    loadedFiles.extend(self._loadTree(fileSystem, remotePath + "/" + fileInformation.name, localPath))
                else:
                    loadedFiles.extend(self._loadTree(fileSystem, remotePath + "/", localDirectory))

        return loadedFiles

    def _unloadAll(self):
        for fileName in self._loadedFiles.keys():
            try:
                self._log.debug("Forcing unload(%s)", fileName)
                self._unload(fileName, force=True)
            except Exception:
                self._log.exception("Failed to unload file %s", fileName)

    def _unload(self, fileName, force=False):
        self.__cacheLock.acquire()
        try:
            refCnt = None
            loadedFiles = []
            try:
                # SR:433
                refCnt, loadedFiles = self._loadedFiles[fileName]
                if force:
                    refCnt = 0
                else:
                    refCnt = refCnt - 1
            except KeyError:
                # SR:336
                self._log.error("File %s could not be unloaded", fileName)
                raise CF.InvalidFileName(CF.CF_EINVAL, "File %s could not be found" % fileName)

            if refCnt == 0:
                try:
                    # Iterate in reverse so that we emtpy the
                    # deepest directory, first and then remove it
                    # if it is empty
                    loadedFiles.reverse()
                    for f in loadedFiles:
                        # SR:434
                        if os.path.isdir(f) and len(os.listdir(f)) == 0:
                            try:
                                os.rmdir(f)
                            except:
                                pass
                        else:
                            # Give it a shot
                            try:
                                os.remove(f)
                            except:
                                pass
                            if f[-3:] == '.py':
                                try:
                                    os.remove(f+'c')
                                except:
                                    pass
                finally:
                    del self._loadedFiles[fileName]
            else:
                self._loadedFiles[fileName] = (refCnt, loadedFiles)
        finally:
            self.__cacheLock.release()

    def unload(self, fileName):
        try:
            self._cmdLock.acquire()
            self._log.debug("unload(%s)", fileName)
            # SR:435
            if self.isLocked(): raise CF.Device.InvalidState("System is locked down")
            if self.isDisabled(): raise CF.Device.InvalidState("System is disabled")

            self._unload(fileName)
        finally:
            self._cmdLock.release()

class ExecutableDevice(LoadableDevice):

    STOP_SIGNALS = ((signal.SIGINT, 2),
                    (signal.SIGQUIT, 2),
                    (signal.SIGTERM, 2),
                    (signal.SIGKILL, 0.5))

    def __init__(self, devmgr, identifier, label, softwareProfile, compositeDevice, execparams, propertydefs=(),loggerName=None):
        LoadableDevice.__init__(self, devmgr, identifier, label, softwareProfile, compositeDevice, execparams, propertydefs,loggerName=loggerName)
        self._applications = {}
        # Install our own SIGCHLD handler to allow reporting on abnormally terminated children,
        # keeping the old one around so that it can be chained (in case a subclass creates its
        # own children, for example).
        self._old_handler = signal.signal(signal.SIGCHLD, self._child_handler)
        self._devnull = open('/dev/null')

    def releaseObject(self):
        for pid in self._applications.keys():
            self.terminate(pid)
        LoadableDevice.releaseObject(self)

    ###########################################
    # CF::ExecutableDevice
    def execute(self, name, options, parameters):
        try:
            self._cmdLock.acquire()
            self._log.debug("execute(%s, %s, %s)", name, options, parameters)
            if not name.startswith("/"):
                raise CF.InvalidFileName(CF.CF_EINVAL, "Filename must be absolute")

            if self.isLocked(): raise CF.Device.InvalidState("System is locked down")
            if self.isDisabled(): raise CF.Device.InvalidState("System is disabled")

            priority = 0
            stack_size = 4096
            invalidOptions = []
            for option in options:
                val = option.value.value()
                if option.id == CF.ExecutableDevice.PRIORITY_ID:
                    if ((not isinstance(val, int)) and (not isinstance(val, long))):
                        invalidOptions.append(option)
                    else:
                        priority = val
                elif option.id == CF.ExecutableDevice.STACK_SIZE_ID:
                    if ((not isinstance(val, int)) and (not isinstance(val, long))):
                        invalidOptions.append(option)
                    else:
                        stack_size = val
            if len(invalidOptions) > 0:
                self._log.error("execute() received invalid options %s", invalidOptions)
                raise CF.ExecutableDevice.InvalidOptions(invalidOptions)

            command = name[1:] # This is relative to our CWD
            self._log.debug("Running %s %s", command, os.getcwd())

            if not os.path.isfile(command):
                raise CF.InvalidFileName(CF.CF_EINVAL, "File could not be found %s" % command)
            os.chmod(command, os.stat(command)[0] | stat.S_IEXEC | stat.S_IREAD | stat.S_IWRITE)
        finally:
            self._cmdLock.release()

        return self._execute(command, options, parameters)

    def executeLinked(self, name, options, parameters, deps):
        try:
            self._cmdLock.acquire()
            pid = 0
            with envState():
                self.initialState.set()
                selected_paths = []
                for dep in deps:
                    if self._sharedPkgs.has_key(dep):
                        selected_paths.append(self._sharedPkgs[dep])
                self._update_selected_paths(selected_paths)
                pid = self.execute(name, options, parameters)
        finally:
            self._cmdLock.release()
        return pid

    def terminate(self, pid):
        try:
            self._cmdLock.acquire()
            self._log.debug("terminate(%s)", pid)
            # SR:457
            if self.isLocked(): raise CF.Device.InvalidState("System is locked down")
            if self.isDisabled(): raise CF.Device.InvalidState("System is disabled")

            self._terminate(pid)
        finally:
            self._cmdLock.release()

    def _execute(self, command, options, parameters):
        """
        Launches the given command after SCA-specific processing has taken
        place in 'execute'. Override or extend this method in subclasses to
        have more control over the launching of components.

        Returns the pid of the new process.
        """
        args = [command]
        # SR:446, SR:447
        for param in parameters:
            if param.value.value() != None:
                args.append(str(param.id))
                # SR:453 indicates that an InvalidParameters exception should be
                # raised if the input parameter is not of a string type; however,
                # version 2.2.2 of the SCA spec is less strict in its wording. For
                # our part, as long as the value can be stringized, it is accepted,
                # to allow component developers to use more specific types.
                try:
                    args.append(str(param.value.value()))
                except:
                    raise CF.ExecutableDevice.InvalidParameters([param])
        self._log.debug("Popen %s %s", command, args)

        # SR:445
        try:
            sp = ossie.utils.Popen(args, executable=command, cwd=os.getcwd(), close_fds=True, stdin=self._devnull, preexec_fn=os.setpgrp)
        except OSError, e:
            # SR:455
            # CF error codes do not map directly to errno codes, so at present
            # we omit the enumerated value.
            self._log.error("subprocess.Popen: %s", e.strerror)
            raise CF.ExecutableDevice.ExecuteFail(CF.CF_NOTSET, e.strerror)

        pid = sp.pid
        self._applications[pid] = sp
        # SR:449
        self._log.debug("execute() --> %s", pid)
        self._log.debug("APPLICATIONS %s", self._applications)
        return pid

    def _terminate(self, pid):
        """
        Terminates the process given by pid after SCA-specifc processing has
        taken place in 'terminate'. Override or extend this method in
        subclasses to have more control over the termination of components.
        """
        # SR:458
        self._log.debug("%s", self._applications)
        if not self._applications.has_key(pid):
            raise CF.ExecutableDevice.InvalidProcess(CF.CF_ENOENT,
                "Cannot terminate.  Process %s does not exist." % str(pid))
        # SR:456
        try:
            sp = self._applications[pid]
            for sig, timeout in self.STOP_SIGNALS:
                try:
                    self._log.debug('Sending signal %d to process group %d', sig, pid)
                    # the group id is used to handle child processes (if they exist) of the component being cleaned up
                    os.killpg(pid, sig)
                except OSError:
                    pass

                # check if pid has finished
                status=None
                try:
                    status = self._applications[pid].poll()
                except KeyError:
                    time.sleep(0.01)                    
                    continue
            
                if status != None: 
                    self._log.debug('Process has stopped...pocess group ' + str(pid) + ' status' + str(status))
                    time.sleep(0.01)                    
                    continue
            
                if not _checkpg(pid): break
                giveup_time = time.time() + timeout
                while _checkpg(pid) and time.time() < giveup_time:
                    time.sleep(0.1)

                if not _checkpg(pid): break

            try:
                self._log.debug(' Delete APP (_terminate)  %d', pid)
                del self._applications[pid]
            except:
                pass
        except:
            pass

    def _child_handler(self, signal, frame):
        # Check the status of each child in sequence instead of using waitpid(-1,...) to find
        # out which child terminated so that we do not affect any children created via means
        # besides execute(). It may be an unlikely situation, but this should be safe and
        # relatively cheap.
        for pid in self._applications.keys()[:]:
            try:
                status = self._applications[pid].poll()
            except KeyError:
                # This particular component has been terminated in the interim; we don't
                # need to check the status anyway, because it's supposed to be dead.
                continue
            if status == None:
                continue
            if status < 0:
                self._log.error("Child process %d terminated with signal %s", pid, -status)
                try:
                    self._log.debug(' Delete APP (_child_handler)  %d', pid)
                    del self._applications[pid]
                except:
                    pass
        if self._old_handler:
            self._old_handler(signal, frame)

# Don't extend the CF__POA.AggregateDevice for the MixIn
# to avoid polluting the inheritance tree and causing
# problems with an object implementing two skeletons
# that are not in the inheritance tree.
class AggregateDevice:
    def __init__(self):
        self._childDevices = []

    ###########################################
    # CF::AggregateDevice
    def addDevice(self, associatedDevice):
        self._log.debug("addDevice(%s)", associatedDevice)
        self._childDevices.append(associatedDevice)

    def removeDevice(self, associatedDevice):
        self._log.debug("removeDevice(%s)", associatedDevice)

        for childdev in self._childDevices:
            if childdev._get_identifier() == associatedDevice._get_identifier():
                self._childDevices.remove(childdev)

    def _get_devices(self):
        return self._childDevices

def _checkForRequiredParameters(execparams):
    for reqparam in ("DEVICE_MGR_IOR", "PROFILE_NAME", "DEVICE_ID", "DEVICE_LABEL"):
        if not execparams.has_key(reqparam):
            if options["interactive"] == True:
                execparams[reqparam] = None
            else:
                logging.error("Missing SCA execparam %s", reqparam)
                sys.exit(-1)

def _getDevMgr(execparams, orb):
    devMgr = None
    if execparams["DEVICE_MGR_IOR"] not in (None, ""):
        devMgr = orb.string_to_object(execparams["DEVICE_MGR_IOR"])
        devMgr = devMgr._narrow(CF.DeviceManager)
    return devMgr

def _getParentAggregateDevice(execparams, orb):
    # get parent aggregate device if applicable
    if execparams.has_key("COMPOSITE_DEVICE_IOR"):
        parentdev = orb.string_to_object(execparams["COMPOSITE_DEVICE_IOR"])
        parentdev_ref = parentdev._narrow(CF.AggregateDevice)
    else:
        parentdev_ref = None

    return parentdev_ref


def start_device(deviceclass, interactive_callback=None, thread_policy=None,loggerName=None, skip_run=False):
    """Typically your device will use this to start the device in __main__.  It
    is recommended to use this function because it will ensure compliance with
    SCA argument parsing and execparams.

    Typical usage is

    start_device(MyDeviceImpl)
    """
    execparams, interactive = resource.parseCommandLineArgs(deviceclass)
    if not skip_run:
        resource.setupSignalHandlers()
        signal.signal(signal.SIGINT, signal.SIG_IGN)

    _checkForRequiredParameters(execparams)

    try:
        try:
            orb = resource.createOrb()
            resource.__orb__ = orb

            ## sets up backwards compat logging 
            resource.configureLogging(execparams, loggerName, orb)

            devicePOA = resource.getPOA(orb, thread_policy, "devicePOA")

            devMgr = _getDevMgr(execparams, orb)

            parentdev_ref = _getParentAggregateDevice(execparams, orb)

            # Configure logging (defaulting to INFO level).
            label = execparams.get("DEVICE_LABEL", "")
            id = execparams.get("DEVICE_ID", "")
            log_config_uri = execparams.get("LOGGING_CONFIG_URI", None)
            debug_level = execparams.get("DEBUG_LEVEL", None)
            if debug_level != None: debug_level = int(debug_level)
            dpath=execparams.get("DOM_PATH", "")
            category=loggerName
            try:
              if not category and label != "": category=label.rsplit("_", 1)[0]
            except:
                pass 
            ctx = ossie.logger.DeviceCtx( label, id, dpath )
            ossie.logger.Configure( log_config_uri, debug_level, ctx, category )

            # instantiate the provided device
            logging.debug("Instantiating Device")
            component_Obj = deviceclass(devMgr, 
                                        execparams["DEVICE_ID"], 
                                        execparams["DEVICE_LABEL"], 
                                        execparams["PROFILE_NAME"],
                                        parentdev_ref,
                                        execparams)
            devicePOA.activate_object(component_Obj)

            idm_channel_ior=execparams.get("IDM_CHANNEL_IOR",None)
            registrar_ior=execparams.get("DEVICE_MGR_IOR",None)
            component_Obj.postConstruction( registrar_ior, idm_channel_ior )

            # set logging context for resource to supoprt CF::Logging
            component_Obj.saveLoggingContext( log_config_uri, debug_level, ctx )

            if not interactive:
                logging.debug("Starting ORB event loop")
                objectActivated = True
                obj = devicePOA.servant_to_id(component_Obj)
                if skip_run:
                    return component_Obj

                # Establish a handler for CORBA::COMM_FAILURE exceptions that
                # can occur with persistence enabled; if the DomainManager dies
                # and comes back, there may be a cached connection that has to
                # fail once before establishing a new connection. More than one
                # failure indicates a more serious problem.
                def retry_on_failure(cookie, retries, exc):
                    return retries == 0
                omniORB.installCommFailureExceptionHandler(None, retry_on_failure)

                while objectActivated:
                    try:
                        obj = devicePOA.reference_to_servant(component_Obj._this())
                        time.sleep(0.5)
                    except:
                        objectActivated = False
            else:
                logging.debug("Entering interacive mode")
                if callable(interactive_callback):
                    # Pass only the Var to prevent anybody from calling non-CORBA functions
                    interactive_callback(component_Obj)
                else:
                    print orb.object_to_string(component_Obj._this())
                    objectActivated = True
                    obj = devicePOA.servant_to_id(component_Obj)
                    while objectActivated:
                        try:
                            obj = devicePOA.reference_to_servant(component_Obj._this())
                            time.sleep(0.5)
                        except:
                            objectActivated = False
        except SystemExit:
            pass
        except KeyboardInterrupt:
            pass
        except:
            traceback.print_exc()
            #logging.exception("Unexpected Error")

    finally:
        if orb and not skip_run:
            orb.destroy()
