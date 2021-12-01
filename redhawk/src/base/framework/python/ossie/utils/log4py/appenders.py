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
import logging, logging.handlers
import sys
import os
from ossie import events
from omniORB import CORBA
import traceback
from omniORB import any
from ossie.cf import CF
import CosEventChannelAdmin
import ossie.logger
import exceptions


nullHandler=None
dailyRollingFileAppender=None
try:
    from logging import NullHandler
    nullHander='NullHandler'
except:
    pass
try:
  import logging.handlers.TimedRotatingFileHandler
  dailyRollingFileAppender='DailyRollingFileAppender'
except ImportError:
  pass

__all__ = ('handleError',
           'ConsoleAppender',
           'FileAppender',
           'RollingFileAppender',
           'RH_LogEventAppender',
           'SyslogAppender')
if nullHandler : __all__ = __all__+ (nullHandler)
if dailyRollingFileAppender : __all__ = __all__+ (dailyRollingFileAppender)

# Override base logging.Handler handleError method to give better stack trace
#  if string passed to log message is malformed
def handleError(self, record):
    print(traceback.print_stack())
logging.Handler.handleError = handleError

# The NullHandler, for library use, is only available in Python 2.7 and up.
# If it is not available, create an equivalent class.
try:
    from logging import NullHandler
except:
    class NullHandler(logging.Handler):
        def __init__(self, *args, **kwargs):
            logging.Handler.__init__(self, *args, **kwargs)
        
        def emit(self, record):
            pass

        def handle(self, record):
            pass

        def createLock(self):
            pass

##############################################################################
# Map standard log4j appenders to logging handlers
##############################################################################
class ConsoleAppender(logging.StreamHandler,object):

  def __init__(self):
    self.stream = sys.stdout # in log4j stdout is default
    self.threshold = logging.NOTSET
    self.log4pyProps = {'strm':sys.stdout}

  def setTarget(self, target):
    if target == "System.out":
      self.stream = sys.stdout
    elif target == "System.err":
      self.stream = sys.stderr
    self.log4pyProps['strm'] = self.stream

  def getTarget(self):
    if self.stream == sys.stdout:
      return "System.out"
    elif self.stream == sys.stderr:
      return "System.err"

  # Supported log4j Options
  Target = property(fget=getTarget, fset=setTarget)
  def activateOptions(self):
    if sys.hexversion >= 0x020700F0:
      # strm was changed to stream
      if "strm" in self.log4pyProps:
        # Don't override stream if it already exists
        self.log4pyProps.setdefault("stream", self.log4pyProps["strm"])
        del self.log4pyProps["strm"]
    logging.StreamHandler.__init__(self, **self.log4pyProps)
    self.setLevel(self.threshold)

class FileAppender(logging.FileHandler,object):

  def __init__(self):
    self.threshold = logging.NOTSET
    self.log4pyProps = {}

  def setFile(self, filename):
    self.filename = filename
    if self.filename.count('/') > 0:
        aggregate = os.path.join('/')
        if self.filename.startswith('/') == False :
            aggregate = os.path.join(os.getcwd()+'/')
        dirs = self.filename.split('/')
        for _dir in dirs[:-1]:
            if _dir == '':
                continue
            aggregate = os.path.join(aggregate,_dir)
            try: 
                os.mkdir(aggregate)
            except Exception as e:
                if type(e) == exceptions.OSError and e.errno == 13:
                    print(e)
                pass
            
    self.baseFilename = os.path.abspath(filename)
    self.log4pyProps['filename'] = filename

  def getFile(self):
    return self.filename

  def setAppend(self, append):
    if bool(append):
      self.mode = "a"
    else:
      self.mode = "w"
    self.log4pyProps['mode'] = self.mode

  def getAppend(self):
    if self.mode == "a":
      return "true"
    else:
      return "false"

  # Supported log4j Options
  Append = property(fget=getAppend, fset=setAppend)
  File = property(fget=getFile, fset=setFile)

  def activateOptions(self):
    logging.FileHandler.__init__(self, **self.log4pyProps)
    self.setLevel(self.threshold)

class RollingFileAppender(logging.handlers.RotatingFileHandler, object):

  def __init__(self):
    self.threshold = logging.NOTSET
    self.log4pyProps = {}

  def setMaxFileSize(self, value):
      # In configuration files, the MaxFileSize option takes an long integer in
      # the range 0 - 2^63. You can specify the value with the suffixes "KB",
      # "MB" or "GB" so that the integer is interpreted being expressed
      # respectively in kilobytes, megabytes or gigabytes. For example, the
      # value "10KB" will be interpreted as 10240.
      value = value.strip()
      suffix = value[-2:]
      multiplier = 1
      if suffix == "KB":
        multiplier = 1024
        value = int(value[:-2]) * multiplier
      elif suffix == "MB":
        multiplier = 1024 * 1024
        value = int(value[:-2]) * multiplier
      elif suffix == "GB":
        multiplier = 1024 * 1024 * 1024
        value = int(value[:-2]) * multiplier
      else:
        value = int(value)
      self.maxBytes = value
      self.log4pyProps['maxBytes'] = value

  def getMaxFileSize(self):
    return str(self.maxBytes)
  
  def setMaxBackupIndex(self, value):
    self.backupCount = int(value)
    self.log4pyProps['backupCount'] = int(value)

  def getMaxBackupIndex(self):
    return str(self.backupCount)

  def setFile(self, filename):
    self.filename = filename
    self.baseFilename = os.path.abspath(filename)
    self.log4pyProps['filename'] = filename

  def getFile(self):
    return self.filename

  # Supported log4j Options
  MaxFileSize = property(fget=getMaxFileSize, fset=setMaxFileSize)
  MaxBackupIndex = property(fget=getMaxBackupIndex, fset=setMaxBackupIndex)
  File = property(fget=getFile, fset=setFile)
  
  def activateOptions(self):
    logging.handlers.RotatingFileHandler.__init__(self, **self.log4pyProps)
    self.setLevel(self.threshold)

# TimedRotatingFileHandler is only available for 2.4 and up
try:
  import logging.handlers.TimedRotatingFileHandler
except ImportError:
  pass
else:
  class DailyRollingFileAppender(logging.handlers.TimedRotatingFileHandler, object):
    def __init__(self):
      self.threshold = logging.NOTSET
      self.log4pyProps = {}

    def setDatePattern(self, pattern):
      self.pattern = pattern.strip()
      if self.pattern == ".yyyy-ww":
        self.when="W0"
      elif self.pattern == ".yyyy-MM-dd":
        self.when="midnight"
      elif self.pattern == ".yyyy-MM-dd-HH":
        self.when="H"
      elif self.pattern == ".yyyy-MM-dd-HH-mm":
        self.when="M"
      elif self.pattern == ".yyyy-MM-dd-HH-mm-ss":
        self.when="S"
      else:
        raise ValueError
      self.log4pyProps['when'] = self.when
   
    def getDatePattern(self):
      return self.pattern

    def setFile(self, filename):
      self.filename = filename
      self.baseFilename = os.path.abspath(filename)
      self.log4pyProps['filename'] = filename

    def getFile(self):
      return self.filename

    # Supported log4j Options
    File = property(fget=getFile, fset=setFile)
    DatePattern = property(fget=getDatePattern, fset=setDatePattern)
    
    def activateOptions(self):
      logging.handlers.TimedRotatingFileHandler.__init__(self, **self.log4pyProps)
      self.setLevel(self.threshold)

class RH_LogEventAppender(logging.Handler):
  ECM=None
  def __init__(self, *args, **kwds):
    logging.Handler.__init__(self, *args, **kwds )
    self.log4pyProps = {}
    self.channelName = None
    self.nameContext = None
    self.prodId = "RESOURCE.ID"
    self.prodName = "RESOURCE.NAME"
    self.prodFQN = "RESOURCE.FQN"
    self._channelName = None
    self._nameContext = None
    self._event_channel = None
    self._ecm = None
    self._pub = None
    self._enable = False
    self.threshold = logging.NOTSET

  def setOption(self, option, value):
    if option.lower() == "event_channel": 
      self.channelName = str(value)
    elif option.lower() == "name_context":
      self.nameContext = str(value)
    elif option.lower() == "producer_id":
      self.prodId = str(value)
    elif option.lower() == "producer_name":
      self.prodName = str(value)
    elif option.lower() == "producer_fqn":
      self.prodFQN = str(value)

  def activateOptions(self):
    self.setLevel(self.threshold)
    if self.channelName != self._channelName and self.channelName != "":
        if self._ecm or self.ECM:
            if self.ECM:
                self.connect( self.ECM )
            else:
                self.connect()

            
  def setEventChannelManager(self, ECM):
      if ECM:
          self.ECM = ECM
          self.connect(ECM)
      else:
          # if no event channel manager is provided then we need to disable the handling of log messages
          self._enable = False
          if self._pub : 
              self._pub.disconnect()

          self._ecm = None
   
  def connect(self, ECM=None ):
      retval=0
      if self._ecm != None or ECM != None :
          self._ecm = ECM
          try:
              self._pub = self._ecm.Publisher(self.channelName)
              self._channelName = self.channelName
              #print "RH_LogEventAppender::connect .... requesting connection to ECM::EVENT CHANNEL, " + str(self.channelName)
              self._enable = True
          except:
              logging.error( "RH_LogEventAppender::connect .... connection FAILED  EVENT CHANNEL(EventChannelManager), " + str(self.channelName) )
              retval=-1
              pass
      else:
          if self.nameContext != None :
              try:
                  if self.channelName and self.nameContext:
                      # try and get the configured orb
                      orb=None
                      try:
                          orb = __orb__
                      except:
                          orb = CORBA.ORB_init()
                          pass
                      
                      channelManager = events.ChannelManager(orb)
                      #print "Channel " + str(self.channelName) + " Context" + str(self.nameContext)
                      self._event_channel = self.channelManager.createEventChannel(self.channelName,self.nameContext)
                      self._pub = Publisher( self._event_channel )
                      self._enable = True
              except:
                  logging.error( "RH_LogEventAppender::connect .... connection FAILED  EVENT CHANNEL(library), " + str(self.channelName) )
                  retval=-1
                  pass

      return retval


  # This is equivalent to the log4j append method
  def handle(self, logRecord):
      if self._enable:
          if self._ecm and self._pub :
              #print "RH_LogEventAppender::handle  .... connection to ECM::Publisher rec: " + str(logRecord)
              eventLogLevel = ossie.logger.ConvertLog4ToCFLevel(self.level)

              logEvent = any.to_any(CF.LogEvent(self.prodId,self.prodName,self.prodFQN,int(logRecord.created),eventLogLevel,logRecord.getMessage())) 
              try:
                  self._pub.push(logEvent)
              except Exception as e:
                  print("RH_LogEventAppender::handle EVENT CHANNEL, PUSH OPERATION FAILED WITH EXCEPTION " + str(e))
          else:
                  print("RH_LogEventAppender::handle No EVENT CHANNEL to publish on.")

class SyslogAppender(logging.handlers.SysLogHandler, object):
  def __init__(self):
    self.threshold = logging.NOTSET
    self.log4pyProps = {}

  def setFacility(self, facility):
    self.facility = self.facility_names[facility.lower()]
    self.log4pyProps['facility'] = self.facility

  def getFacility(self):
    return self.facility

  def setFacilityPrinting(self, facilityprint):
    self.facilityprint = bool(facilityprint)

  def getFacilityPrinting(self):
    return str(self.facilityprint)

  def setSyslogHost(self, sysloghost):
    addr = sysloghost.split(":", 1)
    if len(addr) == 1:
      addr.append(logging.handlers.SYSLOG_UDP_PORT)
    self.address = tuple(addr)
    self.log4pyProps['address'] = self.address
    
  def getSyslogHost(self):
    if len(self.adddress) == 2:
      return "%s:%s" % self.address
    else:
      return self.address[0]

  # Support log4j properites
  Facility = property(fget=getFacility, fset=setFacility)
  SyslogHost = property(fget=getSyslogHost, fset=setSyslogHost)
  
  def activateOptions(self):
    logging.handlers.SysLogHandler.__init__(self, **self.log4pyProps)
    self.setLevel(self.threshold)
