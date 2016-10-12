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

#
# vim: set sw=4:et:softtabstop=4
#
from ossie.cf import CF, CF__POA
from ossie.resource import PropertyAttributeMixIn 
from ossie.device import Device, start_device
import os, sys, stat
from omniORB import URI, any
import commands, copy, time, signal, pprint, subprocess
import logging
import signal
import shutil
from BasicChildDeviceProps import PROPERTIES

# Demonstrate the attribute MixIn
class BasicChildDevice(CF__POA.Device, Device, PropertyAttributeMixIn):

    def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
        Device.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams, PROPERTIES)
      
    def _allocateCapacity(self, propname, value):
        if propname == "someCapacity":
            self.prop_someCapacity = self.props_someCapacity - value
            if self.prop_someCapacity < 0:
                return False
            return True
        else:
            return False

    def _deallocateCapacity(self, propname, value):
        if propname == "someCapacity":
            self.prop_someCapacity = self.prop_someCapacity + value

    def updateUsageState(self):
        if self.prop_someCapacity == 0 and self.prop_someCapacity == 0:
            self._usageState = CF.Device.BUSY
        elif self.prop_someCapacity == 100000000 and self.prop_someCapacity == 100000000: 
            self._usageState = CF.Device.IDLE
        else:
            self._usageState = CF.Device.ACTIVE

if __name__ == "__main__":
    start_device(BasicChildDevice)
