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
            datefmt = []
            i += 1
            while pattern[i] != "}":
              datefmt.append(pattern[i])
              i += 1
            i += 1
            datefmt = "".join(datefmt)
            # TODO : don't ingore the datefmt

          fmt.append(_FORMATTER_TRANS[char][0])
          fmt.append(modifier)
          fmt.append(_FORMATTER_TRANS[char][1])
    self._fmt = "".join(fmt)

  def getConversionPattern(self):
    return pattern
  ConversionPattern = property(fget=getConversionPattern, fset=setConversionPattern)
