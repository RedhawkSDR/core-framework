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
import codecs
import sys
import os
import string
from appenders import *
from layouts import *

# Map log4j levels to python logging levels
_LEVEL_TRANS = {"ALL": logging.NOTSET,
                "DEBUG": logging.DEBUG,
                "TRACE": logging.TRACE,
                "INFO": logging.INFO,
                "WARN": logging.WARNING,
                "ERROR": logging.ERROR,
                "FATAL": logging.FATAL,
                "OFF": logging.FATAL + 1
               }

def _parsePropertiesFile(f):
  """Parse a Java Properties file into a python dictionary.

  NOTE: CURRENTLY THIS DOES NOT SUPPORT ESCAPE CHARACTERS
  NOR LINE CONTINUATIONS.
  """
  if isinstance(f, str):
    try:
      f = codecs.open(f, "r", "iso-8859-1")
    except LookupError:
      f = open(f, "r")
 
  result = {}
  for line in f:
    line = line.lstrip()
    # A natural line that contains only white space characters is considered
    # blank and is ignored. A comment line has an ASCII '#' or '!' as its first
    # non-white space character; comment lines are also ignored
    if len(line) == 0 or line[0] in ('#', '!'):
      continue

    key = None
    value = None
    for i, char in enumerate(line):
      if char in ('\\'):
        # TODO properly deal with escape characters and line continuations
        continue

      # The key contains all of the characters in the line starting with the
      # first non-white space character and up to, but not including, the first
      # unescaped '=', ':', or white space character other than a line
      # terminator.
      if char in ('=', ':', ' ', '\t', '\f'):
        key = line[0:i]
        value = line[i+1:].lstrip()
        result[key] = value
        break
  return result

def _import_handler(name):
  if name.startswith("org.apache.log4j."):
    name = name[len("org.apache.log4j."):]
  return eval(name)

def _import_layout(name):
  if name.startswith("org.apache.log4j."):
    name = name[len("org.apache.log4j."):]
  return eval(name)

def fileConfig(f, category=None):
  logging.shutdown()
  logging.root = logging.RootLogger(logging.WARNING)
  logging.Logger.root = logging.root
  logging.Logger.manager = logging.Manager(logging.Logger.root)

  props = _parsePropertiesFile(f)
  try:
    repoWideThresh = props["log4j.threshold"].strip()
    logging.getLogger().setLevel(_LEVEL_TRANS[repoWideThresh.strip().upper()])
  except KeyError:
    logging.getLogger().setLevel(logging.NOTSET)

  # First load up all loggers and set their levels
  loggers = {}
  rootLoggerCfg = props["log4j.rootLogger"].split(",")
  if rootLoggerCfg[0].strip().upper() in _LEVEL_TRANS.keys():
    logging.getLogger().setLevel(_LEVEL_TRANS[rootLoggerCfg[0].strip().upper()])
    del rootLoggerCfg[0]
  else:
    logging.getLogger().setLevel(logging.NOTSET)

  loggers[logging.getLogger()] = [x.strip() for x in rootLoggerCfg]

  configuredLoggers = filter(lambda x: x.startswith("log4j.logger."), props.keys())
  for logger in configuredLoggers:
    loggerCfg = props[logger].split(",")
    pyname = logger[len("log4j.logger."):]
    if loggerCfg[0].strip() in _LEVEL_TRANS.keys():
      logging.getLogger(pyname).setLevel(_LEVEL_TRANS[loggerCfg[0].strip().upper()])
    else:
      logging.getLogger(pyname).setLevel(logging.NOTSET)
    loggers[logging.getLogger(pyname)] = [x.strip() for x in loggerCfg[1:]]

  # Process category tags for selected appending
  if (category != None):
    categories = filter(lambda x: x.startswith("log4j.category."), props.keys())
    for cat in categories:
      pyname = cat[len("log4j.category."):]
      if (pyname == category):
        categoryCfg = props[cat].split(",")
        for i in range(1,len(categoryCfg)):
          layout = None
          appenderKey = "log4j.appender."+str(categoryCfg[i].strip())
          appenderClass = props[appenderKey]
          klass = _import_handler(appenderClass)
          handler = klass()
          setattr(handler, "threshold", _LEVEL_TRANS[categoryCfg[0].strip()])
          # Deal with appender options
          appenderOptions = filter(lambda x: x.startswith(appenderKey+"."), props.keys())
          for appenderOption in appenderOptions:
            opt = appenderOption[len(appenderKey+"."):]
            value = props[appenderOption].strip()
            if opt.lower().endswith("layout"):
              layoutClass = value
              klass = _import_layout(layoutClass)
              layout = klass()
              layoutOptions = filter(lambda x: x.startswith(appenderKey+".layout."), props.keys())
              for layoutOption in layoutOptions:
                opt = layoutOption[len(appenderKey+".layout."):]
                value = props[layoutOption].strip()
                setattr(layout, opt, value)
            elif opt.lower().endswith("filter"):
              pass
            elif opt.lower().endswith("errorhandler"):
              pass
            elif opt.lower().endswith("threshold"):
              setattr(handler, opt.lower(), _LEVEL_TRANS[value])
            else:
              setattr(handler, opt, value)
          handler.activateOptions()
          logging.getLogger(str(pyname)).addHandler(handler)
          if layout:
            handler.setFormatter(layout)

    # check additive tags to avoid additive logging to the root loggers
    additivities = filter(lambda x: x.startswith("log4j.additivity."), props.keys())
  
    for additive in additivities:
      pyname = additive[len("log4j.additivity."):]  
      if (pyname == category):
        if ( (str(props[additive]).strip().upper()) == "FALSE" ):
          return
  
  # Now deal with root logging appenders
  for logger, appenders in loggers.items():
    for appender in appenders:
      layout = None
      appenderKey = "log4j.appender."+str(appender)
      appenderClass = props[appenderKey]
      klass = _import_handler(appenderClass)
      handler = klass()
      # Deal with appender options
      appenderOptions = filter(lambda x: x.startswith(appenderKey+"."), props.keys())
      for appenderOption in appenderOptions:
        opt = appenderOption[len(appenderKey+"."):]
        value = props[appenderOption].strip()
        if opt.lower().endswith("layout"):
          layoutClass = value
          klass = _import_layout(layoutClass)
          layout = klass()
          layoutOptions = filter(lambda x: x.startswith(appenderKey+".layout."), props.keys())
          for layoutOption in layoutOptions:
            opt = layoutOption[len(appenderKey+".layout."):]
            value = props[layoutOption].strip()
            setattr(layout, opt, value)
        elif opt.lower().endswith("filter"):
          pass
        elif opt.lower().endswith("errorhandler"):
          pass
        elif opt.lower().endswith("threshold"):
          setattr(handler, opt.lower(), _LEVEL_TRANS[value])
        else:
          setattr(handler, opt, value)
      handler.activateOptions()
      logger.addHandler(handler)
      if layout:
        handler.setFormatter(layout)
