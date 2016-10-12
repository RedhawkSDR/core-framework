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

# vim: sw=2: et:
"""
This module intends to provide a log4j/log4cxx compatibility wrapper
for python.  The primary goal is to provide the ability to configure
the built-in python "logging" module using log4j/log4cxx configuration
file.

At this time, there is no intention to fully replicate the log4j/log4cxx
API in python nor provide a wrapper of the log4cxx libraries.

To use a log4j/log4cxx configuration file:

  >> import logging
  >> import ossie.utils.log4py.config
  >> ossie.utils.log4py.config.fileConfig("log4cxx.properties")

"""
__version__ = "0.1"

import logging

# Define a TRACE logging level.
logging.TRACE = 5
logging.addLevelName(logging.TRACE, "TRACE")

# Add a free-standing trace method.
def _trace(msg, *args, **kw):
  logging.log(logging.TRACE, msg, *args, **kw)

logging.trace = _trace
del _trace

# Extend logging class to add a "trace" method, and "getChild" if necessary.
LoggerBase = logging.getLoggerClass()
class RedhawkLogger(LoggerBase):
  def trace(self, msg, *args, **kw):
    self.log(logging.TRACE, msg, *args, **kw)

  if not hasattr(LoggerBase, 'getChild'):
    def getChild(self, suffix):
      return logging.getLogger(self.name + '.' + suffix)

del LoggerBase

logging.setLoggerClass(RedhawkLogger)
