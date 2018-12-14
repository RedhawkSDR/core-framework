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

import os

class TemplateFile(object):
    # Most file types that we generate use hash for comment lines, but
    # subclasses can override if needed
    COMMENT_START = '#'
    COMMENT_LINE  = '#'
    COMMENT_END   = '#'

    def __init__(self, template, filename=None, executable=False, userfile=False):
        self.template = template
        if filename:
            self.filename = filename
        else:
            self.filename = os.path.basename(self.template)
        self.executable = executable
        self.userfile = userfile

    def options(self):
        return {}

    def filters(self):
        return {}

    def context(self):
        return {}

    def comment(self, text):
        """
        Generates a comment block from 'text' suitable for this template type.
        """
        def generate(t):
            yield self.COMMENT_START
            for line in t.split('\n'):
                # Add a space between the comment marker and the line, but only
                # if the line is non-empty
                if line:
                    line = ' ' + line
                yield self.COMMENT_LINE + line
            yield self.COMMENT_END
            yield ''
        return '\n'.join(generate(text))
