#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK throughput.
#
# REDHAWK throughput is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK throughput is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import os

from benchmark.tests import TestMonitor

class CSVOutput(TestMonitor):
    def __init__(self):
        self.fields = []

    def add_field(self, key, header=None):
        if header is None:
            header = key
        self.fields.append((key, header))

    def test_started(self, name, **kw):
        filename = '%s-%d.csv' % (self.make_filename(name), os.getpid())
        self.file = open(filename, 'w')
        print >>self.file, ','.join(title for name, title in self.fields)

    def sample_added(self, **stats):
        print >>self.file, ','.join(str(stats[name]) for name, title in self.fields)

    def test_complete(self, **kw):
        self.file.close()

    def make_filename(self, name):
        name = name.lower()
        name = name.replace('(', '').replace(')', '')
        return '-'.join(name.split())
