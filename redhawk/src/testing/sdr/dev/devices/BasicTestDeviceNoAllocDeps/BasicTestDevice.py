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

if __name__ == "__main__":
    logging.getLogger().setLevel(logging.DEBUG)
    logging.debug("Starting Device")
    start_device(BasicTestDevice)
