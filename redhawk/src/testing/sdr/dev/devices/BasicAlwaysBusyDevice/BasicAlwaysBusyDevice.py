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
from ossie.properties import simple_property, simpleseq_property, struct_property
import os, sys, stat
from omniORB import URI, any
import commands, copy, time, signal, pprint, subprocess
import logging
import signal
import shutil
from BasicAlwaysBusyDeviceProps import PROPERTIES
import threading

class BasicAlwaysBusyDevice(CF__POA.AggregateExecutableDevice, ExecutableDevice, AggregateDevice):

  def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
    ExecutableDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams, PROPERTIES)
    AggregateDevice.__init__(self)
    
    
  def initialize(self):
    ExecutableDevice.initialize(self)
    self._props["memCapacity"] = 100000000
    self._props["BogoMipsCapacity"] = 100000000
    self._props["nicCapacity"] = 100.0
    self._props["fakeCapacity"] = 3
    #self._props["execparams"] = " ".join(["%s %s" % x for x in execparams.items()])
    self._usageState = CF.Device.BUSY
    

  def execute(self, name, options, parameters):
    retval = ExecutableDevice.execute(self, name, options, parameters)
    for option in options:
        if option.id == 'STACK_SIZE':
            self._props['check_STACK_SIZE'] = option.value._v
        elif option.id == 'PRIORITY':
            self._props['check_PRIORITY'] = option.value._v
    return retval
  # See BasicChildDevice for an example of manually dealing with allocate/deallocate
  def allocate_fakeCapacity(self, value):
    self._props["fakeCapacity"] = self._props["fakeCapacity"] - value
    return True

  def allocate_nicCapacity(self, value):
    self._props["nicCapacity"] = self._props["nicCapacity"] - value
    return True

  def allocate_memCapacity(self, value):
    self._props["memCapacity"] = self._props["memCapacity"] - value
    return True

  def allocate_BogoMipsCapacity(self, value):
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

  def updateUsageState(self):
    # Update usage state
    self._usageState = CF.Device.BUSY
    

  class SomeStruct(object):
      field1 = simple_property(id_="item1",
                                type_="string",
                                defvalue="value1")
      field2 = simple_property(id_="item2",
                                type_="long",
                                defvalue=100)
      field3 = simple_property(id_="item3",
                                type_="double",
                                defvalue=3.14156)

  struct = struct_property(id_="DCE:ffe634c9-096d-425b-86cc-df1cce50612f", 
                           name="struct_test", 
                           structdef=SomeStruct)

if __name__ == "__main__":
    logging.getLogger().setLevel(logging.DEBUG)
    logging.debug("Starting Device")
    start_device(BasicAlwaysBusyDevice)
