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

# Define additional logging levels
logging.TRACE = 5
logging.ALL = 1
logging.OFF = logging.FATAL+1
logging.addLevelName(logging.TRACE, "TRACE")
logging.addLevelName(logging.ALL, "ALL")
logging.addLevelName(logging.OFF, "OFF")

USER_LOGS = 'user'
SYSTEM_LOGS = 'system'

# Add a free-standing trace method.
def _trace(msg, *args, **kw):
  logging.log(logging.TRACE, msg, *args, **kw)

logging.trace = _trace
del _trace

# Extend logging class to add a "trace" method, and "getChild" if necessary.
LoggerBase = logging.getLoggerClass()
class RedhawkLogger(LoggerBase):
  def __init__(self, name, level=0):
      LoggerBase.__init__(self, name, level)
      self._loggers = [name]
      self._rh_parent = None

  _ECM = None
  def trace(self, msg, *args, **kw):
    self.log(logging.TRACE, msg, *args, **kw)

  def getCurrentLoggers(self):
    return self._loggers

  def _setParent(self, parent):
    self._rh_parent = parent

  def _addChildLogger(self, logname):
    self._loggers.append(logname)
    if self._rh_parent:
        self._rh_parent._addChildLogger(logname)

  def isLoggerInHierarchy(self, search_name):
    for _name in self._loggers:
        _idx = _name.find(self.name);
        if (_idx == 0):
            if len(_name) > len(self.name):
                if _name[len(self.name)] != '.':
                    continue
            if _name.find(search_name) != 0:
                continue
            if len(_name) > len(search_name):
                if (len(search_name) != 0) and (_name[len(search_name)] != '.'):
                    continue
            return True
    return False

  def getChildLogger(self, logname, ns=USER_LOGS):
      _full_name = ''
      _ns = ns
      if _ns == 'user':
          if '.' in self.name:
              _ns = ''
      if _ns and ((_ns != USER_LOGS) or ((_ns == USER_LOGS) and (not ('.'+USER_LOGS+'.' in self.name)))):
          _full_name = self.name + '.' + _ns + '.' + logname
      else:
          _full_name = self.name + '.' + logname
      if not _full_name in self._loggers:
          self._loggers.append(_full_name)
      if self._rh_parent:
          self._rh_parent._addChildLogger(_full_name)
      retval = logging.getLogger(_full_name)
      try:
          retval._setParent(self)
      except:
          pass
      return logging.getLogger(_full_name)

  @staticmethod
  def SetEventChannelManager(ECM):
    from ossie.utils.log4py.appenders import RH_LogEventAppender
    RedhawkLogger._ECM = ECM
    RH_LogEventAppender.ECM = ECM
    for app in logging._handlerList:
      if isinstance(app, RH_LogEventAppender ):
        app.setEventChannelManager(ECM)

  if not hasattr(LoggerBase, 'getChild'):
    def getChild(self, suffix):
      return logging.getLogger(self.name + '.' + suffix)


#del LoggerBase

logging.setLoggerClass(RedhawkLogger)
