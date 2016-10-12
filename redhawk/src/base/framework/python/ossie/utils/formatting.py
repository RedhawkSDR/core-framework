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

import itertools
import sys

def _pad_infinite(iterable, padding):
    return itertools.chain(iterable, itertools.repeat(padding))

def _pad_finite(iterable, padding, limit):
    return tuple(itertools.islice(_pad_infinite(iterable, padding), limit))

class TablePrinter(object):
    def __init__(self, *headers):
        self._headers = headers
        self._lines = []
        self._limits = [-1] * self.columns
        self._enable_header = True

    @property
    def columns(self):
        return len(self._headers)

    def enable_header(self, state):
        self._enable_header = state

    def limit_column(self, index, length):
        self._limits[index] = length

    def append(self, *args):
        args = _pad_finite(args, '', self.columns)
        self._lines.append(args)

    def _limit_field(self, text, length):
        if len(text) > length:
            return text[:length-3] + '...'
        else:
            return text

    def _column_widths(self):
        widths = tuple(len(h) for h in self._headers)
        for fields in self._lines:
            widths = tuple(max(w, len(f)) for w, f in zip(widths, fields))
        return widths

    def _limit_widths(self):
        widths = self._column_widths()
        for width, limit in zip(widths, self._limits):
            if limit > 0:
                width = min(width, limit)
            yield width

    def write(self, f=sys.stdout):
        # Determine column widths based on current rows and fixed limits
        widths = tuple(self._limit_widths())

        # Create format string based on column width
        template = '\t'.join(['%%-%ds']*self.columns)
        template = template % tuple(widths)

        if self._enable_header:
            print >>f, template % self._headers
            print >>f, template % tuple('-'*len(h) for h in self._headers)

        for fields in self._lines:
            fields = tuple(self._limit_field(f, w) for f, w in zip(fields, widths))
            print >>f, template % fields
