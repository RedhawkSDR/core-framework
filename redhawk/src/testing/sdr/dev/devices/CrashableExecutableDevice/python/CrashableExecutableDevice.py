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

import os
import signal

from ossie.cf import CF, CF__POA
from ossie.device import ExecutableDevice, start_device
from ossie.properties import simple_property
from ossie.utils.log4py import logging

class CrashableExecutableDevice(CF__POA.ExecutableDevice, ExecutableDevice):

  crashEnabled = simple_property(id_="crashEnabled",
                                 type_="boolean",
                                 defvalue=False,
                                 mode="readwrite",
                                 action="external",
                                 kinds=("configure",))

  BogoMipsCapacity = simple_property(id_="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8",
                                     type_="long",
                                     name="BogoMipsCapacity",
                                     defvalue=100000000,
                                     mode="readonly",
                                     kinds=("allocation",))

  def allocate_BogoMipsCapacity (self, value):
    if self.BogoMipsCapacity < value:
      return False
    self.BogoMipsCapacity -= value
    return True

  def deallocate_BogoMipsCapacity (self, value):
    self.BogoMipsCapacity += value

  def updateUsageState (self):
    # Update usage state
    if self.BogoMipsCapacity == 0:
      self._usageState = CF.Device.BUSY
    elif self.BogoMipsCapacity == 100000000:
      self._usageState = CF.Device.IDLE
    else:
      self._usageState = CF.Device.ACTIVE

  def execute (self, *args):
    if self.crashEnabled:
      os.kill(os.getpid(), signal.SIGKILL)
    return ExecutableDevice.execute(self, *args)

if __name__ == "__main__":
    logging.getLogger().setLevel(logging.DEBUG)
    logging.debug("Starting Device")
    start_device(CrashableExecutableDevice)
