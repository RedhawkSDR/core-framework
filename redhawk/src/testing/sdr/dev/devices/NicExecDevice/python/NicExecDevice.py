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
# AUTO-GENERATED
#
# Source: NicExecDevice.spd.xml
from ossie.device import start_device
import logging

from NicExecDevice_base import *

class NicExecDevice_i(NicExecDevice_base):
    """<DESCRIPTION GOES HERE>"""
    def constructor(self):
        """
        This is called by the framework immediately after your device registers with the system.
        
        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()

        """
        self._allocatedNics = {}
        self.setAllocationImpl('nic_allocation', self.allocate_nic, self.deallocate_nic)

    def allocate_nic(self, value):
        if value.interface:
            nic = value.interface
            if not nic in self.nic_list:
                # Bad interface
                return False
            elif nic in self._allocatedNics:
                # Interface in use
                return False
        else:
            nic = self._findAvailableNic()
            if not nic:
                return False

        if value.identifier in self._allocatedNics:
            # Duplicate identifier
            return False

        self._allocatedNics[value.identifier] = nic
        return True

    def _findAvailableNic(self):
        all_nics = self.nic_list[:]
        for nic in self._allocatedNics.itervalues():
            all_nics.remove(nic)
        if not all_nics:
            return None
        return all_nics[0]

    def deallocate_nic(self, value):
        del self._allocatedNics[value.identifier]

    def get_nic_allocation_status(self):
        return [self.NicAllocationStatusStruct(k,v) for k, v in self._allocatedNics.iteritems()]

    nic_allocation_status = NicExecDevice_base.nic_allocation_status.rebind(fget=get_nic_allocation_status)

    def updateUsageState(self):
        """
        This is called automatically after allocateCapacity or deallocateCapacity are called.
        Your implementation should determine the current state of the device:
           self._usageState = CF.Device.IDLE   # not in use
           self._usageState = CF.Device.ACTIVE # in use, with capacity remaining for allocation
           self._usageState = CF.Device.BUSY   # in use, with no capacity remaining for allocation
        """
        if len(self._allocatedNics) == len(self.nic_list):
            self._usageState = CF.Device.BUSY
        elif len(self._allocatedNics) > 0:
            self._usageState = CF.Device.ACTIVE
        else:
            self._usageState = CF.Device.IDLE
        
    def process(self):
        return FINISH

  
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.INFO)
    logging.debug("Starting Device")
    start_device(NicExecDevice_i)

