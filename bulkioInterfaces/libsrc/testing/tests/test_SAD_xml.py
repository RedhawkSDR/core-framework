#!/usr/bin/env python
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

from base_ports import *
from ossie.utils import sb
import traceback


class Test_SADxml(unittest.TestCase):
    def __init__( self, methodName='runTest' ):
        unittest.TestCase.__init__(self, methodName)

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_iohelper(self):
        t1 = sb.launch('../components/Python_Ports/Python_Ports.spd.xml')
        t2 = sb.launch('../components/Python_Ports/Python_Ports.spd.xml')
        source = sb.StreamSource()
        destination = sb.StreamSink()
        source.connect(t1, usesPortName="shortOut")
        t1.connect(t2, usesPortName="dataShortOut")
        t2.connect(destination, providesPortName="shortIn")
        try:
            sad = sb.generateSADXML("testsadxml")
        except:
	    traceback.print_exc()
            self.fail("Generate SAD raised exception")
        self.assertEqual(sad.find("StreamSource"), -1, "Found StreamSource in SAD")
        self.assertEqual(sad.find("StreamSink"), -1, "Found StreamSink in SAD")

if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ Test_SADxml ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    try:
        import xmlrunner
        runner = xmlrunner.XMLTestRunner(verbosity=2)
    except ImportError:
        runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)
