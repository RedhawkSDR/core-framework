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
import pprint
import re
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


def _parsePropertiesStream( cfg ):
  """Parse a Java Properties file into a python dictionary.

  NOTE: CURRENTLY THIS DOES NOT SUPPORT ESCAPE CHARACTERS
  NOR LINE CONTINUATIONS.
  """
  result = {}
  for line in cfg.split('\n'):
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
  if name.startswith("org.ossie.logging."):
    name = name[len("org.ossie.logging."):]
  return eval(name)

def _import_layout(name):
  if name.startswith("org.apache.log4j."):
    name = name[len("org.apache.log4j."):]
  return eval(name)

def strConfig(cfg, category=None):
  props=_parsePropertiesStream(cfg)
  return _config(props,category)

def fileConfig(f, category=None):
  props=_parsePropertiesFile(f)
  return _config(props,category)


def _config(props, category=None, disable_existing_loggers=1):
  logging.shutdown()

  # critical section
  # patterned after from logging.config... 
  logging._acquireLock()
  try:
     logging._handlers.clear()
     del logging._handlerList[:]
     # Handlers add themselves to logging._handlers
     handlers = _install_handlers(props)
     _install_loggers(props, handlers, category, disable_existing_loggers)
  finally:
    logging._releaseLock()

def _install_handlers(props):
  
  # First load up all handlers...
  handlers = {}

  hlist=[]
  tlist = filter(lambda x: x.startswith("log4j.appender."), props.keys())
  if not len(tlist) : return handlers

  # filter out just the appender names
  for h in tlist:
    # skip over layout options
    if re.search(r'log4j\.appender\.(.*?)\.', h): continue
    m=re.search(r'log4j\.appender\.(.*?)$', h)
    if m:
      hlist.append(m.group(1))

  for hname in hlist:
     # aka python logging formatter
     layout = None
     appenderKey = "log4j.appender."+str(hname.strip())
     appenderClass = props[appenderKey]
     klass = _import_handler(appenderClass)
     handler = klass()
     # Deal with appender options
     appenderOptions = filter(lambda x: x.startswith(appenderKey+"."), props.keys())
     for appenderOption in appenderOptions:
       opt = appenderOption[len(appenderKey+"."):]
       value = props[appenderOption].strip()
       if "RH_LogEventAppender" in handler.__class__.__name__:
         handler.setOption(opt,value)
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
     if layout:
        handler.setFormatter(layout)

     handlers[hname]=handler
  return handlers

def _encoded(s):
    return s if isinstance(s, str) else s.encode('utf-8')

def _install_loggers(props, handlers, filterCategory, disable_existing_loggers ):
  llist=[]

  # process root logger first
  log_cfg = props["log4j.rootLogger"].split(",")
  root = logging.root
  log = root
  
  try:
     repoWideThresh = props["log4j.threshold"].strip()
     log.setLevel(_LEVEL_TRANS[repoWideThresh.strip().upper()])
  except KeyError:
    pass

  if log_cfg[0].strip().upper() in _LEVEL_TRANS.keys():
    log.setLevel(_LEVEL_TRANS[log_cfg[0].strip().upper()])
    del log_cfg[0]

  for h in root.handlers[:]:
    root.removeHandler(h)
  
  hlist = [x.strip() for x in log_cfg]
  for h in hlist:
    root.addHandler(handlers[h])

  tmp = filter(lambda x: x.startswith("log4j.category."), props.keys() )
  clist = [ x[len("log4j.category."):] for x in tmp ]
  tmp = filter(lambda x: x.startswith("log4j.logger."), props.keys())
  llist = [ x[len("log4j.logger."):] for x in tmp ]

  # existing loggers
  existing = root.manager.loggerDict.keys()
  existing.sort(key=_encoded)
  child_loggers = []
  #now set up the new ones...

  # check additive tags to avoid additive logging to the root loggers
  additivities = filter(lambda x: x.startswith("log4j.additivity."), props.keys())

  _install_from_list( clist, props, filterCategory, additivities, handlers, existing, child_loggers )
  _install_from_list( llist, props, filterCategory, additivities, handlers, existing, child_loggers )

  #fixup existing loggers...
  for log in existing:
    logger = root.manager.loggerDict[log]
    if isinstance(logger, logging.PlaceHolder): continue
    root.manager._fixupParents(logger)

    if log in child_loggers:
      logger.level = logging.NOTSET
      logger.handlers = []
      logger.propagate = 1
    elif disable_existing_loggers:
      logger.disabled = 1


def _install_from_list( llist, props, filterCategory, additivities, handlers, existing, child_loggers):

  for log in llist:

    # filter out any loggers that are not listed, old behavior...
    if filterCategory:
      if type(filterCategory) == list:
        if log not in filterCategory: continue
      if type(filterCategory) == str:
        if log != filterCategory: continue

    # get a logger
    logger = logging.getLogger(log)

    aname = 'log4j.additivity.'+log
    propagate=1
    if aname in additivities:
      if ( (str(props[aname]).strip().upper()) == "TRUE" ):
          propagate=1
      else:
          propagate=0

    # get logging level and appenders
    hlist=[]
    has_level=False
    for x in [ "log4j.category."+log, "log4j.logger."+log ]:
      if x in props.keys():
        log_cfg = props[x].split(',')
        if log_cfg[0].strip().upper() in _LEVEL_TRANS.keys():
            level = _LEVEL_TRANS[log_cfg[0].strip().upper()]
            has_level = True
            del log_cfg[0]

        hlist = hlist + [x.strip() for x in log_cfg]

    if log in existing:
      i = existing.index(log)
      prefixed = log + "."
      pflen = len(prefixed)
      num_existing = len(existing)
      i = i + 1
      while (i < num_existing) and ( existing[i][:pflen] == prefixed ):
        child_loggers.append(existing[i])
        i = i + 1
      existing.remove(log)

    if has_level:
      logger.setLevel(level)
    for h in logger.handlers[:]:
      logger.removeHandler(h)
    logger.propagate = propagate
    logger.disabled = 0
    if len(hlist):
      for hand in hlist:
        logger.addHandler(handlers[hand])

