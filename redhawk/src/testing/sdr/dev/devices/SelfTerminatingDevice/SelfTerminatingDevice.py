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
from SelfTerminatingDeviceProps import PROPERTIES
import threading



class SelfTerminatingDevice(CF__POA.AggregateExecutableDevice, ExecutableDevice, AggregateDevice):

  def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
    self.exit_lock = threading.Lock()
    self.exit_lock.acquire()
    self.process_thread = threading.Thread(target=self.TerminationThread)
    ExecutableDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams, PROPERTIES)
    AggregateDevice.__init__(self)
    self._props["memCapacity"] = 100000000
    self._props["BogoMipsCapacity"] = 100000000
    self._props["nicCapacity"] = 100.0
    self._props["fakeCapacity"] = 3
    self._props["execparams"] = " ".join(["%s %s" % x for x in execparams.items()])
    
  def configure(self, properties):
    ExecutableDevice.configure(self, properties)
    self.process_thread.start()

  def start(self):
    self.exit_lock.release()
    
  def allocate_fakeCapacity(self, value):
    if self._props["fakeCapacity"] < value:
        return False
    self._props["fakeCapacity"] = self._props["fakeCapacity"] - value
    return True

  def allocate_nicCapacity(self, value):
    if self._props["nicCapacity"] < value:
        return False
    self._props["nicCapacity"] = self._props["nicCapacity"] - value
    return True

  def allocate_memCapacity(self, value):
    if self._props["memCapacity"] < value:
        return False
    self._props["memCapacity"] = self._props["memCapacity"] - value
    return True

  def allocate_BogoMipsCapacity(self, value):
    if self._props["BogoMipsCapacity"] < value:
        return False
    self._props["BogoMipsCapacity"] = self._props["BogoMipsCapacity"] - value
    return True

  def deallocate_fakeCapacity(self, value):
    self._props["fakeCapacity"] = self._props["fakeCapacity"] + value

  def deallocate_nicCapacity(self, value):
    self._props["nicCapacity"] = self._props["nicCapacity"] + value

  def deallocate_memCapacity(self, value):
    self._props["memCapacity"] = self._props["memCapacity"] + value

  def deallocate_BogoMipsCapacity(self, value):
    self._props["BogoMipsCapacity"] = self._props["BogoMipsCapacity"] + value
  
  def TerminationThread(self):
    self.exit_lock.acquire()
    time.sleep(0.1)
    print "Self-terminating device ending process", os.getpid()
    os.kill(os.getpid(),9)

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
    start_device(SelfTerminatingDevice)
