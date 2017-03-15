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
import re

__all__ = ('_FORMATTER_TRANS','_log4j_strftime', 'PatternLayout' )


##############################################################################
# Map standard log4j layouts to logging formatters
##############################################################################
_FORMATTER_TRANS = {"c": ("%(name)", "s"),            # Name of the logger (logging channel)
                    "p": ("%(levelname)", "s"),       # Text logging level for the message ("DEBUG", "INFO", "WARNING", "ERROR", "CRITICAL")
                    "F": ("%(filename)", "s"),        # Filename portion of pathname
                    "L": ("%(lineno)", "d"),          # Source line number where the logging call was issued (if available)
                    "M": ("%(funcName)", "s"),        # Function name
                    "r": ("%(relativeCreated)", "d"), # Time in milliseconds when the LogRecord was created, relative to start
                    "t": ("%(thread)", "d"),          # Thread ID (if available)
                    "m": ("%(message)", "s"),         # The result of record.getMessage(), computed just as the record is emitted
                    "C": ("", ""),
                    "d": ("%(asctime)", "s"),
                    "l": ("", ""),
                    "n": ("", ""),
                    "x": ("", ""), 
                    "X": ("", ""),
                   }

# not supported log4j formats..  with time.strftime
#  W - week in month
#  G - Era
#  S  - millisecond..
#
#  Warning order of tuples matters... so be carefull if you change...
#   for example. first entry (a)->%p,   day in weekname is a %a...
_log4j_strftime= [
                     ( '(a){1,}', '%p' ),                                               # AM or PM
                     ( '(y){4,}', '%Y' ),  ( '(y){2,}', '%y' ),                         # year
                     ( '(M){4,}', '%B' ),  ( '(M){3,}', '%b' ),                         # month names
                     ( '(M){2,}', '%Q' ),  ( '(M){1,}', '%Q' ),                         # month numbers (stage 1...)
                     ( '(d){2,}', '%X' ),  ( '(d){1,}', '%d' ),                         # day of month (stage 1)
                     ( '(E){4,}', '%A' ),  ( '(E){3,}', '%a' ),                         # day in week name
                     ( '(D){3,}', '%j' ),  ( '(D){1,}', '%j' ),                         # day in year
                     ( '(F){1,}', '%R' ),                                               # day in week number (stage 1)
                     ( '(w){1,}', '%U' ),                                               # week in year
                     ( '(H){2,}', '%V' ), ( '(H){1,}', '%H' ),                          # hour (24)
                     ( '(K){2,}', '%I' ), ( '(K){1,}', '%I' ),                          # hour (12)
                     ( '(m){3,}', '%M' ), ( '(m){2,}', '%M' ),                          # minute
                     ( '(s){2,}', '%S' ), ( '(2){1,}', '%@' ),                          # seconds
                     ( '(z){4,}', '%Z' ), ( '(z){3,}', '%Z' ), ( '(z){1,}', '%Z' ),     # timezone
                     ( '(%Q){1,}', '%m' ),                                              # month numbers (stage 2...)
                     ( '(%X){1,}', '%d' ),                                              # day of month (stage 2 )
                     ( '(%V){1,}', '%H' ),                                              # hour (24) (stage 2)
                     ( '(%R){1,}', '%w' ),                                              # day in week number (stage 2)
                     ]

class PatternLayout(logging.Formatter,object):
  def setConversionPattern(self, pattern):
    self.pattern = pattern
    fmt = []
    # Translate the pattern to python logging formats
    i = 0
    while i < len(pattern):
      if pattern[i] != "%":
        fmt.append(pattern[i])
        i += 1
      else:
        i += 1
        if pattern[i] == "%":
          fmt.append("%%")
        else:
          modifier = []
          while pattern[i] in ("-", ".", '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'):
            modifier.append(pattern[i])
            i += 1
          modifier = "".join(modifier)

          char = pattern[i]
          i += 1
          
          # If it's a date spec, see if there is a custom date format
          if char == "d" and pattern[i] == "{":
              dfmt=''
              # grab format string
              pat=re.compile(r'%d\{(.*?)\}')
              try:
                  m=pat.findall(pattern)
                  dfmt = m[0]
                  for x in _log4j_strftime:
                      zz=re.sub(r''+x[0], r''+x[1],dfmt)
                      dfmt=zz
                  i=i+len(m[0])+2
              except:
                  pass
              if dfmt.upper() == 'ISO8601' :
                self.datefmt = None
              else:
                self.datefmt = "".join(dfmt)

          fmt.append(_FORMATTER_TRANS[char][0])
          fmt.append(modifier)
          fmt.append(_FORMATTER_TRANS[char][1])
    self._fmt = "".join(fmt)

  def getConversionPattern(self):
    return pattern
  ConversionPattern = property(fget=getConversionPattern, fset=setConversionPattern)
