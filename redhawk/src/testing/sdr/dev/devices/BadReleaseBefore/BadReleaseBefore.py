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
from ossie.properties import simple_property, simpleseq_property, struct_property, structseq_property
import os, sys, stat
from omniORB import URI, any
import commands, copy, time, signal, pprint, subprocess
import logging
import signal
import shutil
from BadReleaseBeforeProps import PROPERTIES
import threading

class BadReleaseBefore(CF__POA.AggregateExecutableDevice, ExecutableDevice, AggregateDevice):

  def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
    ExecutableDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams, PROPERTIES)
    AggregateDevice.__init__(self)
    self._props["memCapacity"] = 100000000
    self._props["BogoMipsCapacity"] = 100000000
    self._props["nicCapacity"] = 100.0
    self._props["fakeCapacity"] = 3
    self._props["execparams"] = " ".join(["%s %s" % x for x in execparams.items()])

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

  def allocate_allocStruct(self, value):
    if value > self.allocStruct:
      return False    
    
    self.allocStruct -= value
    return True

  def deallocate_allocStruct(self, value):
    self.allocStruct += value

  def updateUsageState(self):
    # Update usage state
    if self._props["memCapacity"] == 0 and self._props["BogoMipsCapacity"] == 0:
        self._usageState = CF.Device.BUSY
    elif self._props["memCapacity"] == 100000000 and self._props["BogoMipsCapacity"] == 100000000:
        self._usageState = CF.Device.IDLE
    else:
        self._usageState = CF.Device.ACTIVE
  
  def releaseObject(self):
      a = b
      ExecutableDevice.releaseObject(self)

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

  class AllocStruct(object):
    long_capacity = simple_property(id_="long_capacity",
                                    type_="long",
                                    defvalue=100)
    float_capacity = simple_property(id_="float_capacity",
                                     type_="float",
                                     defvalue=1.0)
    
    def __gt__(self, other):
      if self.long_capacity <= other.long_capacity:
        return False
      if self.float_capacity <= other.float_capacity:
        return False
      return True

    def __iadd__(self, other):
      self.long_capacity += other.long_capacity
      self.float_capacity += other.float_capacity
      return self

    def __isub__(self, other):
      self.long_capacity -= other.long_capacity
      self.float_capacity -= other.float_capacity
      return self

  allocStruct = struct_property(id_="DCE:001fad60-b4b3-4ed2-94cb-40e1d956bf4f",
                                 name="allocStruct",
                                 configurationkind=("configure", "allocation"),
                                 structdef=AllocStruct)

  structseq = structseq_property(id_="DCE:e836f007-5821-4671-b05a-e0ff5147fe86",
                                 name="structseq_test",
                                 structdef=SomeStruct,
                                 defvalue=[])

if __name__ == "__main__":
    logging.getLogger().setLevel(logging.DEBUG)
    logging.debug("Starting Device")
    start_device(BadReleaseBefore)
