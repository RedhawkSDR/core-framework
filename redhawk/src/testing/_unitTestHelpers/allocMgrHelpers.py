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

from ossie import properties
from ossie.cf import CF

def compareAllocationStatus(lhs, rhs):
    if lhs.allocationID != rhs.allocationID:
        return False
    lhsProps = properties.props_to_dict(lhs.allocationProperties)
    rhsProps = properties.props_to_dict(rhs.allocationProperties)
    if lhsProps != rhsProps:
        return False
    if not lhs.allocatedDevice._is_equivalent(rhs.allocatedDevice):
        return False
    if not lhs.allocationDeviceManager._is_equivalent(rhs.allocationDeviceManager):
        return False
    return True

def compareAllocationStatusSequence(lhs, rhs):
    if len(lhs) != len(rhs):
        return False
    for lhs_item, rhs_item in zip(lhs, rhs):
        if not compareAllocationStatus(lhs_item, rhs_item):
            return False
    return True

def createRequest(requestId, props, pools=[], devices=[], sourceId=''):
    return CF.AllocationManager.AllocationRequestType(requestId, props, pools, devices, sourceId)

def parseDomainDevices(domMgr):
    devices = {}
    domainName = domMgr._get_name()
    for devMgr in domMgr._get_deviceManagers():
        nodeName = devMgr._get_identifier()
        for dev in devMgr._get_registeredDevices():
            devId = dev._get_identifier()
            devices[devId] = {'domain': domainName, 'node' : nodeName}
    return devices

def parseDeviceLocations(devLocs):
    devices = {}
    for loc in devLocs:
        devId = loc.dev._get_identifier()
        nodeName = loc.devMgr._get_identifier()
        devices[devId] = {'domain':loc.domainName, 'node':nodeName}
    return devices
