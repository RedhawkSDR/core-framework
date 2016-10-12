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
import shutil
import os
import numpy

from ossie.utils import sb
from ossie.utils.bluefile import bluefile
from ossie.utils.bulkio import bulkio_helpers

class BluefileTest(unittest.TestCase):

    def test_formWritePath (self):
        """
        Ensure that bluefile.form_write_path does not throw an exception on a
        filename with no path.
        """
        try:
            bluefile.form_write_path('out')
        except AttributeError:
            self.fail('form_write_path raises an exception with no path')

class BlueFileHelpers(unittest.TestCase):

    TYPEMAP = { 'B' : ('char', int),
                'I' : ('short', int),
                'L' : ('long', int),
                'F' : ('float', float),
                'D' : ('double', float) }

    def setUp(self):
        try:
            import bulkio
        except ImportError:
            raise ImportError('BULKIO is required for this test')
        self._tempfiles = []

    def tearDown(self):
        for tempfile in self._tempfiles:
            try:
                os.unlink(tempfile)
            except:
                pass
        sb.domainless._getSandbox().shutdown()

    def test_FileSink(self):
        self._test_FileSink('SB')
        self._test_FileSink('SI')
        self._test_FileSink('SL')
        self._test_FileSink('SF')
        self._test_FileSink('SD')

        self._test_FileSink('CB')
        self._test_FileSink('CI')
        self._test_FileSink('CL')
        self._test_FileSink('CF')
        self._test_FileSink('CD')

    def _tempfileName(self, name):
        filename = 'test_bluefile_%s.tmp' % name
        self._tempfiles.append(filename)
        return filename

    def _flatten(self, data):
        for real, imag in zip(data.real, data.imag):
            yield real
            yield imag

    def _test_FileSink(self, format):
        filename = self._tempfileName('sink_%s' % format)

        complexData = format.startswith('C')
        typecode = format[1]
        dataFormat, dataType = self.TYPEMAP[typecode]
        indata = [dataType(x) for x in xrange(16)]

        source = sb.DataSource(dataFormat=dataFormat)
        sink = sb.FileSink(filename, midasFile=True)
        source.connect(sink)
        sb.start()
        source.push(indata, complexData=complexData, EOS=True)
        sink.waitForEOS()

        hdr, outdata = bluefile.read(filename)
        self.assertEqual(hdr['format'], format)
        if complexData:
            if dataFormat in ('double', 'float'):
                outdata = list(self._flatten(outdata))
            else:
                outdata = outdata.flatten()
        self.assertTrue(numpy.array_equal(indata, outdata), msg="Format '%s' %s != %s" % (format, indata, outdata))

    def _test_FileSource(self, format):
        filename = self._tempfileName('source_%s' % format)

        complexData = format.startswith('C')
        typecode = format[1]
        dataFormat, dataType = self.TYPEMAP[typecode]

        indata = [dataType(x) for x in xrange(16)]
        if complexData:
            if dataFormat in ('float', 'double'):
                indata = [complex(x) for x in indata]
            else:
                indata = numpy.reshape(indata, (8,2))
        hdr = bluefile.header(1000, format)
        bluefile.write(filename, hdr, indata)

        source = sb.FileSource(filename, midasFile=True, dataFormat=dataFormat)
        sink = sb.DataSink()
        source.connect(sink)
        sb.start()
        outdata = sink.getData(eos_block=True)
        if complexData:
            self.assertEqual(sink.sri().mode, 1)
            if dataFormat in ('float', 'double'):
                outdata = bulkio_helpers.bulkioComplexToPythonComplexList(outdata)
            else:
                outdata = numpy.reshape(outdata, (len(outdata)/2,2))
        else:
            self.assertEqual(sink.sri().mode, 0)
        self.assertTrue(numpy.array_equal(indata, outdata), msg='%s != %s' % (indata, outdata))

    def test_FileSource(self):
        self._test_FileSource('SB')
        self._test_FileSource('SI')
        self._test_FileSource('SL')
        self._test_FileSource('SF')
        self._test_FileSource('SD')

        self._test_FileSource('CB')
        self._test_FileSource('CI')
        self._test_FileSource('CL')
        self._test_FileSource('CF')
        self._test_FileSource('CD')
