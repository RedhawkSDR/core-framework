#{#
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
#}
#% set className = component.progclass.name
#% set baseClass = component.baseclass.name
#% set artifactType = component.artifacttype
#% set executesHWComponents = component.executesHWComponents                    
#% set executesPersonaDevices = not executesHWComponents
#% set executeType = "resource" if executesHWComponents else "persona"
#% set executeClass = "Resource_impl" if executesHWComponents else "Device_impl"

import os
import sys
import threading
from ossie.cf import CF
#{% if component is device %}
from ossie.device import start_device
#{% else %}
from ossie.resource import Resource, start_component
#{% endif %}
import logging

from ${baseClass} import *

class HwLoadStates:
    INACTIVE = 0
    ACTIVE = 1
    PENDING = 2
    ERRORED = 3

class DefaultHWLoadRequestContainer():
    def __init__(self, request_id="",
                       requester_id="",
                       hardware_id="",
                       load_filepath=""):
        self.request_id = request_id
        self.requester_id = requester_id
        self.hardware_id = hardware_id
        self.load_filepath = load_filepath

class DefaultHWLoadStatusContainer():
    def __init__(self, request_id="",
                       requester_id="",
                       hardware_id="",
                       load_filepath="",
                       state=HwLoadStates.INACTIVE):
        self.request_id = request_id
        self.requester_id = requester_id
        self.hardware_id = hardware_id
        self.load_filepath = load_filepath
        self.state = state

#default_hw_load_request_struct = {}
#default_hw_load_request_struct["request_id"] = ""
#default_hw_load_request_struct["requester_id"] = ""
#default_hw_load_request_struct["hardware_id"] = ""
#default_hw_load_request_struct["load_filepath"] = ""

#default_hw_load_status_struct = {}
#default_hw_load_status_struct["request_id"] = ""
#default_hw_load_status_struct["requester_id"] = ""
#default_hw_load_status_struct["hardware_id"] = ""
#default_hw_load_status_struct["load_filepath"] = ""
#default_hw_load_status_struct["state"] = HwLoadStates.INACTIVE

HW_LOAD_REQUEST_PROP = "hw_load_requests"


class ${className}(${baseClass}):

    def initialize(self):
#{% if executesPersonaDevices %}
        self._usageState = CF.Device.BUSY
#{% endif %}
        self._${executeType}Map = {}
        self._processMap = {}
        self._processIdIncrement = 0
        self._hwLoadRequestClass = DefaultHWLoadRequestContainer
        self._hwLoadStatusClass = DefaultHWLoadStatusContainer
        self._hwLoadRequestsContainer = []
        self._hwLoadStatusesContainer = [DefaultHWLoadStatusContainer(), DefaultHWLoadStatusContainer()]
        self._allocationLock = threading.Lock()

#{% if component.hasHwLoadRequestProp %}
        self.setHwLoadRequestsClass(${baseClass}.HwLoadRequest)
        self.setHwLoadRequestsContainer(self.hw_load_requests)
#{% else %}
        #self.setHwLoadRequestsClass(${baseClass}.HwLoadRequest)
        #self.setHwLoadRequestsContainer(self.hw_load_requests)
#{% endif %}
#{% if component.hasHwLoadStatusProp %}
        self.setHwLoadStatusesClass(${baseClass}.HwLoadStatus)
        self.setHwLoadStatusesContainer(self.hw_load_statuses)
#{% else %}
        #self.setHwLoadStatusesClass(${baseClass}.HwLoadStatus)
        #self.setHwLoadStatusesContainer(self.hw_load_statuses)
#{% endif %}


    def load(self, fs, fileName, loadKind):
        isSharedLibrary = (loadKind == CF.LoadableDevice.SHARED_LIBRARY)
        existsOnDevFS   = self._devmgr._get_fileSys().exists(fileName)
        
        # Use the device fs if file exists on there
        if existsOnDevFS:
            fs = self._devmgr._get_fileSys()

        # Need to add base class to python path first
        if isSharedLibrary:
            ${baseClass}.load(self, fs, fileName[:-3]+"_base.py", loadKind)
            ${baseClass}.load(self, fs, fileName[:-3]+"_persona_base.py", loadKind)

        # Load the standard file using the normal mechanism
        ${baseClass}.load(self, fs, fileName, loadKind)


    def execute(self, name, options, parameters):

        ${executeType} = self.instantiate${executeType.capitalize()}(name, options, parameters)

        # Validate that the ${executeType} was instantiated properly
        if not ${executeType}:
            msg = "Unable to instantiate '%s'" % str(name)
            self._baseLog.error(msg)
            raise CF.ExecutableDevice.ExecuteFail(CF.CF_NOTSET, msg)
        
        # Validate that we can set the parentDevice reference on the persona
        if hasattr(persona, "_parentDevice"):
            persona._parentDevice = self
        else:
            msg = "Unable to set parent device on persona '%s'" % str(name)
            self._baseLog.warning(msg)
            raise CF.ExecutableDevice.ExecuteFail(CF.CF_NOTSET, msg)

        # Setup variables for storing the mappings
        ${executeType}Id = ${executeType}._get_identifier()
        fakePid = self._processIdIncrement

        # Update the mappings to include new ${executeType}
        self._${executeType}Map[${executeType}Id] = ${executeType}
        self._processMap[fakePid] = ${executeType}Id
        self._processIdIncrement += 1

#{% if executesPersonaDevices %}
        self._devmgr.registerDevice(${executeType}._this())
#{% endif %}

        return fakePid


    def instantiate${executeType.capitalize()}(self, name, options, parameters):
        mod_name, file_ext = os.path.splitext(os.path.split(name)[-1])
        expected_class_name = mod_name + "_i"
        expected_class = None
        localpath = os.path.join(os.environ['SDRROOT'], "dev/"+name)

        # Import the python module of the 'persona'
        py_module = __import__(mod_name, fromlist=[expected_class_name])

        # Setup system argv before launching 
        argv = [name]
        for param in parameters:
            argv.append(str(param.id))
            argv.append(str(param.value._v))
        sys.argv = argv

        if hasattr(py_module, expected_class_name):
            expected_class = getattr(py_module, expected_class_name)

        return start_device(expected_class, skip_run=True)

    def terminate(self, processId):
        if not self._processMap.has_key(processId):
            raise CF.ExecutableDevice.InvalidProcess(CF.CF_ENOENT,
                "Cannot terminate.  Process %s does not reference a resource!." % str(processId))

        resourceId = self._processMap.pop(processId)
        
        if not self._${executeType}Map.has_key(resourceId):
            raise CF.ExecutableDevice.InvalidState(
                "Cannot terminate.  Unable to locate resource '%s'!" % str(resourceId))

        resource = self._${executeType}Map[resourceId]
        
        # Let the device manager clean up child devices
        # resource.releaseObject()

#{% if executesPersonaDevices %}
        resource._set_adminState(CF.Device.UNLOCKED)
        sys.stdout.flush()
#{% endif %}

    def configure(self, properties):
        ${baseClass}.configure(self, properties)
#{% if component.hasHwLoadRequestProp %}
        self.setHwLoadRequestsContainer(self.hw_load_requests)
#{% endif %}
#{% if component.hasHwLoadStatusProp %}
        self.setHwLoadStatusesContainer(self.hw_load_statuses)
#{% endif %}


    def allocateCapacity(self, capacities):
        self._allocationLock.acquire()
        allocationSuccess = False        
        try:
            for capacity in capacities:
        
                # Only process capacitiy with matching ID
                if capacity.id != HW_LOAD_REQUEST_PROP:
                    continue;
            
                # TODO: Populate internal hardware load request struct
                hwLoadRequestsContainer = self.getHwLoadRequestsContainer()
                self._populateHwLoadRequest(hwLoadRequestsContainer, capacity)

                if not (self.hwLoadRequestsAreValid(hwLoadRequestsContainer)):
                    self._baseLog.warn("Received invalid hw_load_request - Not allocating hardware!")
                    continue;

                hwLoadStatusesContainer = self.getHwLoadStatusesContainer()
                allocationSuccess = self._applyHwLoadRequests(hwLoadRequestsContainer,
                                                              hwLoadStatusesContainer)
                if (allocationSuccess):
                    self._baseLog.warn("TODO: Figure out this callback in allocateCapacity")
                    # TODO: Figure out the callback
        finally:
            self.updateAdminStates()
            self._allocationLock.release()

        return allocationSuccess


    def deallocateCapacity(self, capacities):
        deallocationSuccess = False

        for capacity in capacities:
            if capacity.id != HW_LOAD_REQUEST_PROP:
                continue;

            self._baseLog.debug("Deallocating hw_load_requests...")

            # TODO: Populate hwLoadRequestsToRemove
            hwLoadStatusesContainer = self.getHwLoadStatusesContainer()
            hwLoadRequestsToRemove = self._locateHwLoadStatuses(hwLoadStatusesContainer, capacities)
            
            for hwLoadRequestToRemove in hwLoadRequestsToRemove:
                deallocationSuccess |= self._removeHwLoadRequestFromStatus(hwLoadRequestToRemove,
                                                                          hwLoadStatusesContainer)
        self.updateAdminStates()

        if not deallocationSuccess:
            msg = "Unable to deallocate hw_load_requests!"
            self._baseLog.error(msg)
            raise CF.Device.InvalidCapacity(msg, capacities)

    
    def releaseObject(self):
        self._baseLog.debug("Received release call")
        # Wrapped in list in order to use list copy
        for fakePid in list(self._processMap.keys()):
            self.terminate(fakePid)
        cg_py_programmable_base.releaseObject(self)

    def getHwLoadRequestsContainer(self):
        return self._hwLoadRequestsContainer

    def getHwLoadStatusesContainer(self):
        return self._hwLoadStatusesContainer

    def setHwLoadRequestsContainer(self, container):
        self._hwLoadRequestsContainer = container
    
    def setHwLoadStatusesContainer(self, container):
        self._hwLoadStatusesContainer = container

    def setHwLoadRequestsClass(self, clazz):
        self._hwLoadRequestClass = clazz
    
    def setHwLoadStatusesClass(self, clazz):
        self._hwLoadStatusClass = clazz

    def generate${executeType.capitalize()}(self, argc, argv, fn, libraryName):
        raise "Generate${executeType.capitalize()} method must be overriden!"

    def hwLoadRequestsAreValid(self, hwLoadRequests):
        isValid = True
        for hwLoadRequest in hwLoadRequests:
            isValid |= self.hwLoadRequestIsValid(hwLoadRequest)
        return isValid

    def hwLoadRequestIsValid(self, hwLoadRequest):
        # Override this method for fine-grain validation
        return True

    def _applyHwLoadRequests(self, loadRequestContainer, loadStatusContainer):
        success = False
        
        # Maintain which statuses were used incase of failures
        usedStatusIndices = []
        for loadRequest in loadRequestContainer:
            availableStatusIndex = self._findAvailableHwLoadStatusIndex(loadStatusContainer)

            # Attempt to fill in hw_status_struct from hw load request
            if availableStatusIndex > -1:
                availableStatusContainer = loadStatusContainer[availableStatusIndex]
                success |= self._applyHwLoadRequest(loadRequest, availableStatusContainer)
                usedStatusIndices.append(availableStatusIndex)
            else:
                self._baseLog.error("Device cannot be allocated against.  No more hw_load capacity available!")
                success = False

            # Rollback all statuses that we're previously valid
            if not success:
                for ind in usedStatusIndices:
                    self._resetHwLoadStatus(loadStatusContainer[ind])
                break;

        # Clear out the incoming loads
        #   DO NOT DO THE FOLLOWING TO CLEAR:
        #   loadRequestContainer = []
        #   
        #   The load request is mutable and updated unless we set it equal
        while loadRequestContainer:
            loadRequestContainer.pop()

        return success

    def _applyHwLoadRequest(self, incomingRequest, newStatus):
        newStatus.request_id = incomingRequest.request_id
        newStatus.requester_id = incomingRequest.requester_id
        newStatus.hardware_id = incomingRequest.hardware_id
        newStatus.load_filepath = incomingRequest.load_filepath
        newStatus.state = HwLoadStates.PENDING

        # Call user-defined 'loadHardware' and updated state based on result
        if self.loadHardware(newStatus):
            newStatus.state = HwLoadStates.ACTIVE
            return True
        else:
            newStatus.state = HwLoadStates.ERRORED
            return False

    def _resetHwLoadStatus(self, loadStatusStruct):
        # Call user-defined 'unloadHardware' method
        self.unloadHardware(loadStatusStruct)
        
        loadStatusStruct.request_id = ""
        loadStatusStruct.requester_id = ""
        loadStatusStruct.hardware_id = ""
        loadStatusStruct.load_filepath = ""
        loadStatusStruct.state = HwLoadStates.INACTIVE

    def loadHardware(self, newStatus):
        self._baseLog.debug("Method 'loadHardware' is not implemented!")
        return True

    def unloadHardware(self, loadStatus):
        self._baseLog.debug("Method 'unloadHardware' is not implemented")

    def _removeHwLoadRequestFromStatus(self, hwLoadRequest, hwLoadStatusContainer):
        for hwLoadStatus in hwLoadStatusContainer:
            if hwLoadStatus.request_id == hwLoadRequest.request_id:
                self._resetHwLoadStatus(hwLoadStatus)
                return True
        return False
    
    def _hasAnInactiveHwLoadStatus(self):
        hwLoadStatusesContainer = self.getHwLoadStatusesContainer()
        return (self._findAvailableHwLoadStatusIndex(hwLoadStatusesContainer) >= 0)

    def updateAdminStates(self):
        if self._hasAnInactiveHwLoadStatus():
#{% if executesPersonaDevices %}
            for resource in self._${executeType}Map.values():
                resource._set_adminState(CF.Device.UNLOCKED)
#{% endif %}
            self._set_adminState = CF.Device.UNLOCKED
        else:
#{% if executesPersonaDevices %}
            hwLoadStatusesContainer = self.getHwLoadStatusesContainer()

            # Keep track of which personas are running
            runningPersonas = []
            for hwLoadStatus in hwLoadStatusesContainer:
                runningPersonas.append(hwLoadStatus.requester_id)
            
            # Deactivate all non-allocated personas
            for resource in self._${executeType}Map.values():
                resourceId = resource._get_identifier()
                if runningPersonas.count(resourceId) < 1:
                    self._baseLog.debug("Locking device: '%s'" % str(resourceId)) 
                    resource._set_adminState(CF.Device.LOCKED)

#{% endif %}
            self._set_adminState = CF.Device.LOCKED

    def _populateHwLoadRequest(self, container, requests):      
        # Format lists to list
        requestVals = requests.value
        if not isinstance(requestVals, list):
            requestVals = [requestVals]
        
        # Iterate through each request in list
        for request in requestVals:
            if request.id != "hw_load_request":
                self._baseLog.warn("Unable to convert incoming request - PropId must be 'hw_load_request'")
                continue;
            
            request_id = ""
            requester_id = ""
            hardware_id = ""
            load_filepath = ""
            for prop in request.value:
                if self._propMatches(prop, "request_id"):
                    request_id = prop.value
                if self._propMatches(prop, "requester_id"):
                    requester_id = prop.value
                if self._propMatches(prop, "hardware_id"):
                    hardware_id = prop.value
                if self._propMatches(prop, "load_filepath"):
                    load_filepath = prop.value
            
            container.append(self._hwLoadRequestClass(request_id=request_id, 
                                                      requester_id=requester_id,
                                                      hardware_id=hardware_id,
                                                      load_filepath=load_filepath))

    def _locateHwLoadStatuses(self, container, capacities):
        retVal = []
        for capacity in capacities:
            if capacity.id == "hw_load_requests":
                # Format all into a list
                hwLoadRequests = capacity.value
                if not isinstance(hwLoadRequests, list):
                    hwLoadRequests = [hwLoadRequests]

                # Recurse this method with array of hw_load_request structs
                retVal.extend(self._locateHwLoadStatuses(container, hwLoadRequests))

            elif capacity.id == "hw_load_request":
                # Format props into a list
                props = capacity.value
                if not isinstance(capacity.value, list):
                    props = [props]
                
                # Recurse this method with array of hw_load_request structs
                retVal.extend(self._locateHwLoadStatuses(container, props))

            elif capacity.id == "hw_load_request::request_id":
                for status in container:
                    # Validate that our container has the 'request_id' field
                    if not hasattr(status, "request_id"):
                        self._baseLog.warn("Unable to locate status by request_id: \
                                        HwLoadStatuses does not have request_id field!")
                        return retVal

                    # Check if request_id fields match
                    if status.request_id == capacity.value:
                        retVal.append(status)
                        print "Found request " + str(status.request_id)
        return retVal

    def _propMatches(self, prop, propId):
        match = False
        match |= (prop.id == str(propId)) 
        match |= (prop.id == "hw_load_request::" + str(propId))
        return match

    def _findAvailableHwLoadStatusIndex(self, hwLoadStatusesContainer):
        for idx, hwLoadStatus in enumerate(hwLoadStatusesContainer):
            if hwLoadStatus.state == HwLoadStates.INACTIVE:
                return idx
        return -1
