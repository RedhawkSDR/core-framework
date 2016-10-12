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


from ossie.cf import CF, CF__POA
from ossie.device import ExecutableDevice, AggregateDevice, start_device
import os, sys, stat
from omniORB import URI, any
import commands, copy, time, signal, pprint, subprocess
import logging
import signal
import shutil
from BasicTestDeviceProps import PROPERTIES

class BasicTestDevice(CF__POA.AggregateExecutableDevice, ExecutableDevice, AggregateDevice):

  def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
    ExecutableDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams, PROPERTIES)
    AggregateDevice.__init__(self)
    self._props["memCapacity"] = 100000000
    self._props["BogoMipsCapacity"] = 100000000

  # See BasicChildDevice for an example of manually dealing with allocate/deallocate
  def allocate_memCapacity(self, value):
    self._props["memCapacity"] = self._props["memCapacity"] - value
    if self._props["memCapacity"] < 0:
        return False
    return True

  def allocate_BogoMipsCapacity(self, value):
    self._props["BogoMipsCapacity"] = self._props["BogoMipsCapacity"] - value
    if self._props["BogoMipsCapacity"] < 0:
        return False
    return True

  def deallocate_memCapacity(self, value):
    self._props["memCapacity"] = self._props["memCapacity"] + value

  def deallocate_BogoMipsCapacity(self, value):
    self._props["BogoMipsCapacity"] = self._props["BogoMipsCapacity"] + value

  def updateUsageState(self):
    # Update usage state
    if self._props["memCapacity"] == 0 and self._props["BogoMipsCapacity"] == 0:
        self._usageState = CF.Device.BUSY
    elif self._props["memCapacity"] == 100000000 and self._props["BogoMipsCapacity"] == 100000000:
        self._usageState = CF.Device.IDLE
    else:
        self._usageState = CF.Device.ACTIVE

if __name__ == "__main__":
    logging.getLogger().setLevel(logging.DEBUG)
    logging.debug("Starting Device")
    start_device(BasicTestDevice)
