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

import redhawk.codegen.lang.cpp

from redhawk.codegen.jinja.template import TemplateFile

class CppTemplate(TemplateFile):
    COMMENT_START = '/*'
    COMMENT_LINE  = ' *'
    COMMENT_END   = ' */'

    def options(self):
        return {
            'trim_blocks':           True,
            'line_statement_prefix': '//%',
            'variable_start_string': '${',
            'variable_end_string':   '}',
            'block_start_string':    '/*{%',
            'block_end_string':      '%}*/',
            'comment_start_string':  '/*#',
            'comment_end_string':    '#*/'
        }

    def context(self):
        return {
            'cpp': redhawk.codegen.lang.cpp
        }
