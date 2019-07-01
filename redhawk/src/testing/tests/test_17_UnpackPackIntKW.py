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

import unittest
import tempfile
import os
import numpy
import platform

from ossie.utils import sb
from ossie.utils.bluefile import bluefile, bluefile_helpers
from ossie.properties import *

#CF-1889

class BlueFileExtKeyword(unittest.TestCase):

    def setUp(self):

        try:
            import bulkio
            globals()['bulkio'] = bulkio
        except ImportError:
            raise ImportError('BULKIO is required for this test')

        self._tempfiles = []

    def tearDown(self):
        for tempfile in self._tempfiles:
            try:
                os.unlink(tempfile)
            except:
                pass
        sb.release()

    def _createtempfile(self, name):
        #Populate hdr (header) and data
        filename = 'test_bluefile_%s.tmp' % name
        self._tempfiles.append(filename)

        #filename = self._tempfileName(name)
        data = numpy.arange(1024)
        hdr = bluefile.header(1000, 'SI')
        bluefile.write(filename, hdr, data)

        #populate ext (extended header) 
        mkdata = 3
        mykw = props_from_dict({'mykeyword': mkdata})
        bf=bluefile_helpers.BlueFileWriter(filename,'dataLong')
        srate=5e6
        sri = bulkio.sri.create('test_stream',srate)
        sri.mode = False
        sri.blocking=False
        sri.keywords=mykw
        bf.start()
        bf.pushSRI(sri)
        tmpData = range(0, 1024)
        srid=ossie.properties.props_to_dict(sri.keywords)
        return filename

    def test_typeConsistency(self):
        filename = self._createtempfile('dataTypeConsistency')
        hdr = bluefile.readheader(filename,dict)

        # write back to disk, unmodified
        bluefile.writeheader(filename, hdr)
        hdr2 = bluefile.readheader(filename,dict)

        # read in same file and check keyword
        self.assertEqual(type(hdr['ext_header']['mykeyword']), \
                         type(hdr2['ext_header']['mykeyword']), \
                         msg="\nKeyword data type is not packed and unpacked consistently")

        #cast the integer to int()
        hdr['ext_header']['mykeyword'] = int(hdr['ext_header']['mykeyword'])
        bluefile.writeheader(filename, hdr)
        hdr = bluefile.readheader(filename,dict)
        self.assertEqual(type(hdr['ext_header']['mykeyword']), \
                         type(hdr2['ext_header']['mykeyword']), \
                         msg="\nKeyword is not packed and unpacked consistently")

    def _typeInteger(self, dt):
        # read in same file and check keyword
        cver=platform.linux_distribution()[1]
        datatype = str(type(dt))
        if cver.startswith('7'):
            self.assertIn('int',datatype, msg=("\nKeyword is not packed/unpacked as an integer (int, numpy.int32, or numpy.int16)"))
        else:
            self.assertTrue('int' in datatype, msg=("\nKeyword is not packed/unpacked as an integer (int, numpy.int32, or numpy.int16)"))


    def test_typeInteger(self):
    #confirm value types are integers (int, numpy.int32, numpy.int16)
        filename = self._createtempfile('dataValueTypeInt')
        hdr = bluefile.readheader(filename,dict)
        self._typeInteger(hdr['ext_header']['mykeyword'])

        # write back to disk, unmodified
        bluefile.writeheader(filename, hdr)
        hdr2 = bluefile.readheader(filename,dict)

        # read in same file and check keyword
        self._typeInteger(hdr2['ext_header']['mykeyword'])

        #cast the integer to 'int'
        hdr['ext_header']['mykeyword'] = int(hdr['ext_header']['mykeyword'])
        bluefile.writeheader(filename, hdr)
        hdr = bluefile.readheader(filename,dict)
        self._typeInteger(hdr['ext_header']['mykeyword'])
