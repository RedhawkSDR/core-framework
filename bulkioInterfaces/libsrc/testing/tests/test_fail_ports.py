#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
#
#
#

#
# base_port
#  runs same set of tests for each type of port specified...
#

import os
import time
import unittest

from ossie.utils import sb
from base_ports import test_dir

class BaseFailPort(unittest.TestCase):        

    def __init__(self, component, *args, **kwargs):
        unittest.TestCase.__init__(self, *args, **kwargs)
        self.component = component

    def setUp(self):
        c_spd_xml = os.path.join(test_dir, 'components', self.component, self.component+'.spd.xml')
        execparams = { 'LOGGING_CONFIG_URI' : 'file://'+os.getcwd()+'/log4j.ex1' }

        self.comp1 = sb.launch(c_spd_xml, execparams=execparams)
        self.comp2 = sb.launch(c_spd_xml, execparams=execparams)

    def tearDown(self):
        sb.release()

    def _testConnectionFail(self, format, data):
        uses_name = 'data%sOut' % format
        provides_name = 'data%sIn' % format

        source = sb.StreamSource(streamID='test_connection_fail')
        source.connect(self.comp1, providesPortName=provides_name)
        self.comp1.connect(self.comp2, usesPortName=uses_name)

        sink = sb.StreamSink()
        self.comp1.connect(sink, usesPortName=uses_name)

        sb.start()

        os.kill(self.comp2._pid, 9)
        while self.comp2._process.isAlive():
            time.sleep(0.1)

        for _ in xrange(9):
            source.write(data)
        source.close()

        sink.read(eos=True)

    def testCharConnectionFail(self):
        self._testConnectionFail('Char', range(100))

    def testOctetConnectionFail(self):
        self._testConnectionFail('Octet', range(100))

    def testShortConnectionFail(self):
        self._testConnectionFail('Short', range(100))

    def testUShortConnectionFail(self):
        self._testConnectionFail('UShort', range(100))

    def testLongConnectionFail(self):
        self._testConnectionFail('Long', range(100))

    def testULongConnectionFail(self):
        self._testConnectionFail('ULong', range(100))

    def testLongLongConnectionFail(self):
        self._testConnectionFail('LongLong', range(100))

    def testULongLongConnectionFail(self):
        self._testConnectionFail('ULongLong', range(100))

    def testFloatConnectionFail(self):
        self._testConnectionFail('Float', range(100))

    def testDoubleConnectionFail(self):
        self._testConnectionFail('Double', range(100))

    def testFileConnectionFail(self):
        text = "The quick brown fox jumped over the lazy dog"
        self._testConnectionFail('File', text)


class CPPFailPortTest(BaseFailPort):
    def __init__(self, *args, **kwargs):
        BaseFailPort.__init__(self, 'CPP_Ports', *args, **kwargs)

class JavaFailPortTest(BaseFailPort):
    def __init__(self, *args, **kwargs):
        BaseFailPort.__init__(self, 'Java_Ports', *args, **kwargs)

class PythonFailPortTest(BaseFailPort):
    def __init__(self, *args, **kwargs):
        BaseFailPort.__init__(self, 'Python_Ports', *args, **kwargs)

if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ CPPFailPortTest, JavaFailPortTest, PythonFailPortTest ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    try:
        import xmlrunner
        runner = xmlrunner.XMLTestRunner(verbosity=2)
    except ImportError:
        runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)
