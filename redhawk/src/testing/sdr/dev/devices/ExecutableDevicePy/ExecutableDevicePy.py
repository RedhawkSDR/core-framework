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

from ossie.cf import CF__POA
from ossie.device import ExecutableDevice, start_device
from ossie.properties import simple_property
import os
import logging

class ExecutableDevicePy(CF__POA.ExecutableDevice, ExecutableDevice):

  def get_LD_LIBRARY_PATH(self):
    return os.getenv('LD_LIBRARY_PATH', '')

  def get_PYTHONPATH(self):
    return os.getenv('PYTHONPATH', '')

  def get_CLASSPATH(self):
    return os.getenv('CLASSPATH', '')

  LD_LIBRARY_PATH = simple_property(id_="LD_LIBRARY_PATH",
                                    type_="string",
                                    mode="readonly",
                                    action="external",
                                    kinds=("configure",),
                                    fget=get_LD_LIBRARY_PATH)

  PYTHONPATH = simple_property(id_="PYTHONPATH",
                               type_="string",
                               mode="readonly",
                               action="external",
                               kinds=("configure",),
                               fget=get_PYTHONPATH)

  CLASSPATH = simple_property(id_="CLASSPATH",
                              type_="string",
                              mode="readonly",
                              action="external",
                              kinds=("configure",),
                              fget=get_CLASSPATH)

if __name__ == "__main__":
    logging.getLogger().setLevel(logging.DEBUG)
    logging.debug("Starting Device")
    start_device(ExecutableDevicePy)
