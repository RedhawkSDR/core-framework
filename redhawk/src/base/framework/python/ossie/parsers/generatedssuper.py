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
class GeneratedsSuper(object):
    def gds_format_string(self, input_data, input_name=''):
        return input_data
    def gds_validate_string(self, input_data, node, input_name=''):
        if input_data is None:
            # ElementTree parsers return None for empty text nodes
            return ''
        return input_data
    def gds_format_integer(self, input_data, input_name=''):
        return '%d' % input_data
    def gds_validate_integer(self, input_data, node, input_name=''):
        return input_data
