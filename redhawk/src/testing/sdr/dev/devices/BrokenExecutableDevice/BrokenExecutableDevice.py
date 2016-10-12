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
from ossie.device import ExecutableDevice, start_device
from ossie.properties import simple_property
import logging


class ExceptionFunc(object):
  def __init__ (self, exception):
    self.exception = exception

  def __call__ (self, *args, **kwargs):
    raise self.exception


class BasicTestDevice(CF__POA.ExecutableDevice, ExecutableDevice):

  exceptionPoint = simple_property(id_="DCE:58f5720f-3a33-4056-8359-6b560613815d",
                                   type_="string",
                                   name="exceptionPoint",
                                   mode="readwrite",
                                   action="external",
                                   kinds=("configure",))

  exceptionType = simple_property(id_="DCE:7f4ca822-5497-43d0-b92d-fe97c561e450",
                                  type_="string",
                                  name="exceptionType",
                                  mode="readwrite",
                                  action="external",
                                  kinds=("configure",))

  BogoMipsCapacity = simple_property(id_="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8",
                                     type_="long",
                                     name="BogoMipsCapacity",
                                     defvalue=100000000,
                                     mode="readonly",
                                     kinds=("allocation",))

  def __init__ (self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
    ExecutableDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)

  def allocate_BogoMipsCapacity (self, value):
    if self.BogoMipsCapacity < value:
      return False
    self.BogoMipsCapacity = self.BogoMipsCapacity - value
    return True

  def deallocate_BogoMipsCapacity (self, value):
    self.BogoMipsCapacity = self.BogoMipsCapacity + value

  def updateUsageState (self):
    # Update usage state
    if self.BogoMipsCapacity == 0:
      self._usageState = CF.Device.BUSY
    elif self.BogoMipsCapacity == 100000000:
      self._usageState = CF.Device.IDLE
    else:
      self._usageState = CF.Device.ACTIVE

  def __getattribute__ (self, attrname):
    """
    Intercept lookup of attributes to allow any method to throw an
    error via the 'exceptionPoint' and 'exceptionType' properties.
    """
    # Use the base class __getattribute__ to avoid recursion.
    getattribute = lambda x: ExecutableDevice.__getattribute__(self, x)
    attribute = getattribute(attrname)
    if callable(attribute) and attrname == getattribute('exceptionPoint'):
      if getattribute('exceptionType') == 'CF.InvalidFileName':
        exception = CF.InvalidFileName(CF.CF_EINVAL, "Test exception")
      elif self.exceptionType == 'CF.Device.InvalidCapacity':
        exception = CF.Device.InvalidCapacity("Test exception", [])
      else:
        exception = Exception()
      return ExceptionFunc(exception)
    return attribute

if __name__ == "__main__":
    logging.getLogger().setLevel(logging.DEBUG)
    logging.debug("Starting Device")
    start_device(BasicTestDevice)
