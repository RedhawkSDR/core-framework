#!/usr/bin/env python
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

from omniORB import any, CORBA
from ossie.cf import CF, CF__POA
from ossie.device import ExecutableDevice, start_device
from ossie.properties import simple_property
from ossie.resource import usesport, providesport
import commands, os, sys
import logging


class fromOther_i(CF__POA.Resource):
    def __init__(self, parent, name):
        self.parent = parent
        self.name = name

    def _get_identifier(self):
        return self.parent._get_identifier() + '/' + self.name


class testOut_i(CF__POA.Port):
    def __init__(self, parent, name):
        self.parent = parent
        self.name = name
        self.outPorts = {}
    
    def connectPort(self, connection, connectionId):
        port = connection._narrow(CF__POA.Resource)
        self.outPorts[str(connectionId)] = port
    
    def disconnectPort(self, connectionId):
        if self.outPorts.has_key(str(connectionId)):
            self.outPorts.pop(str(connectionId), None)   

    def getIdentifiers(self):
        props = []
        for id in self.outPorts:
            try:
                value = any.to_any(self.outPorts[id]._get_identifier())
            except CORBA.TRANSIENT:
                # If the other end is unreachable, just report the identifier
                # as "None" so it is clear that the connection still exists.
                value = any.to_any(None)
            props.append(CF.DataType(id=id, value=value))
        return props


class devicemanagerOut_i(CF__POA.Port):
    def __init__(self, parent, name):
        self.parent = parent
        self.name = name
        self.outPorts = {}
    
    def connectPort(self, connection, connectionId):
        port = connection._narrow(CF__POA.DeviceManager)
        self.outPorts[str(connectionId)] = port
    
    def disconnectPort(self, connectionId):
        if self.outPorts.has_key(str(connectionId)):
            self.outPorts.pop(str(connectionId), None)   

    def getIdentifiers(self):
        props = []
        for id in self.outPorts:
            value = any.to_any(self.outPorts[id]._get_identifier())
            props.append(CF.DataType(id=id, value=value))
        return props


class PortDevice_impl(CF__POA.ExecutableDevice, ExecutableDevice):

    toTest = usesport("resource_out", "IDL:CF/Resource:1.0", type_="test")
    toDevMgr = usesport("devicemanager_out", "IDL:CF/DeviceManager:1.0", type_="test")
    fromOther = providesport("resource_in", "IDL:CF/Resource:1.0", type_="test")

    os_name = simple_property(id_='DCE:4a23ad60-0b25-4121-a630-68803a498f75',
                              type_='string',
                              name='os_name',
                              defvalue='Linux',
                              mode='readonly',
                              action='eq',
                              kinds=('allocation',))

    processor_name = simple_property(id_='DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b',
                                     type_='string',
                                     name='processor_name',
                                     defvalue='i686',
                                     mode='readonly',
                                     action='eq',
                                     kinds=('allocation',))

    memCapacity = simple_property(id_='DCE:8dcef419-b440-4bcf-b893-cab79b6024fb',
                                  type_='long',
                                  name='memCapacity',
                                  defvalue=100000000,
                                  mode='readonly',
                                  action='external',
                                  kinds=('allocation',))

    BogoMipsCapacity = simple_property(id_='DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8',
                                       type_='long',
                                       name='BogoMipsCapacity',
                                       defvalue=100000000,
                                       mode='readonly',
                                       action='external',
                                       kinds=(u'allocation',))

    def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
        ExecutableDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)

    def initialize(self):
        ExecutableDevice.initialize(self)
        self.toTest = testOut_i(self, "resource_out")
        self.toDevMgr = devicemanagerOut_i(self, "devicemanager_out")
        self.fromOther = fromOther_i(self, "resource_in")

    def runTest(self, testid, properties):
        if testid == 0:
            return self.toTest.getIdentifiers()
        elif testid == 1:
            return self.toDevMgr.getIdentifiers()
        else:
            raise CF.TestableObject.UnknownTest('unknown test: ' + str(id))
        return []

    def allocate_memCapacity(self, value):
      if self.memCapacity < value:
        return False
      self.memCapacity = self.memCapacity - value
      return True

    def allocate_BogoMipsCapacity(self, value):
      if self.BogoMipsCapacity < value:
        return False
      self.BogoMipsCapacity = self.BogoMipsCapacity - value
      return True

    def deallocate_memCapacity(self, value):
      self.memCapacity = self.memCapacity + value

    def deallocate_BogoMipsCapacity(self, value):
      self.BogoMipsCapacity = self.BogoMipsCapacity + value

    def updateUsageState(self):
      # Update usage state
      if self.memCapacity == 0 and self.BogoMipsCapacity == 0:
        self._usageState = CF.Device.BUSY
      elif self.memCapacity == 100000000 and self.BogoMipsCapacity == 100000000:
        self._usageState = CF.Device.IDLE
      else:
        self._usageState = CF.Device.ACTIVE

if __name__ == "__main__":
    start_device(PortDevice_impl)
