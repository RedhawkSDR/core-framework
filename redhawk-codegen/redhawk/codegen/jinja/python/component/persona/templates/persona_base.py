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
#% set className = component.personaclass.name
#% set baseClass = component.baseclass.name
#% set artifactType = component.artifacttype

import os
import sys
from ossie.cf import CF
#{% if component is device %}
from ossie.device import start_device
#{% else %}
from ossie.resource import Resource, start_component
#{% endif %}
import logging

from ${baseClass} import *


class ${className}(${baseClass}):

    def initialize(self):
        ${baseClass}.initialize(self)
        self._parentDevice = None
        self._parentAllocated = False
        self._previousRequestProps = None
#{% if isExecutable == True %}
        self._resourceMap = {}
        self._processMap = {}
        self._processIdIncrement = 0
#{% endif %}

    def releaseObject(self):
#{% if isExecutable == True %}
        for pid in self._processMap:
            self.terminate(pid)

#{% endif %}
        self._baseLog.debug("releaseObject()")
        if self._adminState == CF.Device.UNLOCKED:
            self._adminState = CF.Device.SHUTTING_DOWN

        try:
            self._unregister()
        except Exception, e:
            raise CF.LifeCycle.ReleaseError(str(e))

        self._adminState = CF.Device.LOCKED
        try:
            objid = self._default_POA().servant_to_id(self)
            self._default_POA().deactivate_object(objid)
        except:
            self._baseLog.error("failed releaseObject()")

    def attemptToProgramParent(self):
        if (not self._parentDevice):
            self._baseLog.error("Unable to Program parent: No reference to parent device exists!")
            return False

        if (not self._parentAllocated):
            self._baseLog.debug("About to allocate parent device!")

            self.beforeHardwareProgrammed()
            requestProps   = self.hwLoadRequest()
            formattedProps = self._formatRequestProps(requestProps)

            if not formattedProps:
                self._baseLog.error("Failed to format hw_load_request props for parent device")
                return False

            self._parentAllocated = self._parentDevice.allocateCapacity(formattedProps)

            if (self._parentAllocated):
                self._previousRequestProps = formattedProps
                self.afterHardwareProgramSuccess()
            else:
                self.afterHardwareProgramFailure()

        return self._parentAllocated

    def attemptToUnprogramParent(self):
        if (not self._parentDevice):
            self._baseLog.error("Unable to Program parent: No reference to parent device exists!")
            return False

        if self._parentAllocated:
            self._baseLog.debug("About to deallocate parent device!")

            if (not self._previousRequestProps):
                self._baseLog.error("Previously requested hw_load props empty!")
                return False

            self._parentDevice.deallocateCapacity(self._previousRequestProps)
            self._parentAllocated = False
            self.afterHardwareUnprogrammed();

        return (not self._parentAllocated)

#{% if isExecutable == True %}
    def execute(self, name, options, parameters):
        
        for param in parameters:
            propId = param.id
            propVal = param.value.value()
            if type(propVal) == str:
                self._baseLog.debug("InstantiateResourceProp: ID['" + 
                    str(propId) + "'] = " + str(propVal))

        resource = instantiateResource(name, options, parameters)
        
        if not resource:
            msg = "Unable to dynamically instantiate resource!"
            self._baseLog.error(msg)
            raise CF.ExecutableDevice.ExecuteFail(CF.CF_NOTSET, msg)

        resourceId = resource._get_identifier()
        fakePid = self._processIdIncrement + 1
        self._resourceMap[resourceId] = resource
        self._processMap[fakePid] = resourceId
        self._processIdIncrement += 1

        return fakePid

    def terminate(self, processId):
        if not self._processMap.has_key(processId):
            raise CF.ExecutableDevice.InvalidProcess(CF.CF_ENOENT,
                "Cannot terminate.  Process %s does not reference a resource!." % str(processId))

        resourceId = self._processMap.pop(processId)

        if not self._resourceMap.has_key(resourceId):
            raise CF.ExecutableDevice.InvalidState(
                "Cannot terminate.  Unable to locate resource '%s'!" % str(resourceId))

        resource = self._resourceMap.pop(resourceId)
        del resource

    
    def instantiateResource(self, libraryName, options, parameters):
        resource = None
        # TODO fill this in
        return resource
#{% endif %}

    def allocateCapacity(self, capacities):
        allocationSuccess = ${baseClass}.allocateCapacity(self, capacities)
        if allocationSuccess: 
            allocationSuccess = self.attemptToProgramParent()
            # Undo current allocations if we can't program parent
            if not allocationSuccess:
                ${baseClass}.deallocateCapacity(self, capacities)
        return allocationSuccess

    def deallocateCapacity(self, capacities):
        ${baseClass}.deallocateCapacity(self, capacities)
#{% if isExecutable == True %}
        if not self.hasRunningResource():
            self.attemptToUnprogramParent()
#{% else %}
        self.attemptToUnprogramParent()
#{% endif %}

    def hwLoadRequest(self):
        return []

    def beforeHardwareProgrammed(self):
        # User may override this method
        return

    def beforeHardwareUnprogrammed(self): 
        # User may override this method
        return

    def afterHardwareProgramSuccess(self): 
        # User may override this method
        return

    def afterHardwareProgramFailure(self): 
        # User may override this method
        return

    def afterHardwareUnprogrammed(self): 
        # User may override this method
        return

    def _formatRequestProps(self, requestProps):
        # Sanity check... Can't format nothing!
        if not requestProps:
            self._baseLog.error("Unable to format hw_load_request_properties.  Properties are empty!")
            return None

        # Sanity check... Make sure the type has an id field!
        if not hasattr(requestProps[0], "id"):
            self._baseLog.error("Unable to format hw_load_request_properties.  Properties must be of list of 'CF.Datatype'")
            return None

        # Case 1 - Properties are already formatted properly
        if len(requestProps) == 1:
            if requestProps[0].id == "hw_load_requests":
                self._baseLog.debug("No formatting occurred - Assumed formatting is proper")
                return requestProps

        # Further inspection of properties
        allPropsAreHwLoadRequest = True
        foundRequestId = False
        for prop in requestProps:
            allPropsAreHwLoadRequest &= (prop.id == "hw_load_request")
            foundRequestId |= (prop.id == "request_id") or \
                              (prop.id == "hw_load_request::request_id")

        # Case 2 - Properties are list of hw_load_request structs
        if allPropsAreHwLoadRequest:
            self._baseLog.debug("Found hw_load_request list - Formatting to structseq")
            return [CF.DataType(id="hw_load_requests", value=requestProps)]

        # Case 3 - Properties represent the contents of a single hw_load_request
        if foundRequestId:
            self._baseLog.debug("Found hw_load_request contents - Formatting to structseq")
            structProp = CF.DataType(id="hw_load_request", value=requestProps)
            return [CF.DataType(id="hw_load_requests", value=structProp)]

        self._baseLog.error("Unable to format hw_load_request_properties.  Properties are empty!")
        return None


