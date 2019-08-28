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

from base_ports  import *
from  bulkio import *
from ossie.cf import CF
from omniORB import any
from ossie.utils import sb
import os, time

class Test_QueueFlush(unittest.TestCase):
    def __init__( self, methodName='runTest' ):
        unittest.TestCase.__init__(self, methodName)

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_cpp_flush(self):
        comp = sb.launch('../components/sri_changed_cpp/sri_changed_cpp.spd.xml')
        self.assertNotEqual(comp,None)
        source = sb.StreamSource()
        source.connect(comp)
        source.start()
        data = range(100)
        for _ in range(1000):
            source.write(data)
        comp.start()
        time.sleep(1)
        self.assertEquals(comp.verified,True)
        self.assertEquals(comp.changed,True)
        self.assertEquals(len(comp.ports[0].ref._get_statistics().keywords),1)
        self.assertEquals(comp.ports[0].ref._get_statistics().keywords[0].id,'timeSinceLastFlush')
        

if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ Test_QueueFlush ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    try:
        import xmlrunner
        runner = xmlrunner.XMLTestRunner(verbosity=2)
    except ImportError:
        runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)

