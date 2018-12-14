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

# REDHAWK-specific version of generateDS base class
# Implements only the methods that are explicitly used by the parsers

import sys
import re as re_

ExternalEncoding = 'ascii'
if sys.version_info[0] == 2:
    BaseStrType_ = basestring
else:
    BaseStrType_ = str
CDATA_pattern_ = re_.compile(r"<!\[CDATA\[.*?\]\]>", re_.DOTALL)

def quote_xml(inStr):
    "Escape markup chars, but do not modify CDATA sections."
    if not inStr:
        return ''
    s1 = (isinstance(inStr, BaseStrType_) and inStr or '%s' % inStr)
    s2 = ''
    pos = 0
    matchobjects = CDATA_pattern_.finditer(s1)
    for mo in matchobjects:
        s3 = s1[pos:mo.start()]
        s2 += quote_xml_aux(s3)
        s2 += s1[mo.start():mo.end()]
        pos = mo.end()
    s3 = s1[pos:]
    s2 += quote_xml_aux(s3)
    return s2

def quote_xml_aux(inStr):
    s1 = inStr.replace('&', '&amp;')
    s1 = s1.replace('<', '&lt;')
    s1 = s1.replace('>', '&gt;')
    return s1

class GeneratedsSuper(object):
    def gds_format_string(self, input_data, input_name=''):
        return input_data
    @staticmethod
    def gds_encode(instring):
        if sys.version_info[0] == 2:
            return instring.encode(ExternalEncoding)
        else:
            return instring
    def gds_validate_string(self, input_data, node, input_name=''):
        if input_data is None:
            # ElementTree parsers return None for empty text nodes
            return ''
        return input_data
    def gds_format_integer(self, input_data, input_name=''):
        return '%d' % input_data
    def gds_validate_integer(self, input_data, node, input_name=''):
        return input_data
    def convert_unicode(self, instring):
        if isinstance(instring, str):
            result = quote_xml(instring)
        elif sys.version_info[0] == 2 and isinstance(instring, unicode):
            result = quote_xml(instring).encode('utf8')
        else:
            result = GeneratedsSuper.gds_encode(str(instring))
        return result
