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
import os, time, tempfile

class Test_QueueFlush(unittest.TestCase):
    def __init__( self, methodName='runTest' ):
        unittest.TestCase.__init__(self, methodName)

    def setUp(self):
        if sb.domainless._sandbox:
            sb.domainless._sandbox.shutdown()
            sb.domainless._sandbox = None

    def tearDown(self):
        sb.release()
        try:
            os.remove('stream_test.log')
            pass
        except:
            pass

    def test_cpp_flush(self):
        src = sb.launch('../components/stream_src/stream_src.spd.xml')
        snk = sb.launch('../components/stream_snk/stream_snk.spd.xml', properties={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/stream_log.cfg'})
        self.assertNotEqual(src,None)
        self.assertNotEqual(snk,None)
        src.connect(snk)
        sb.start()
        time.sleep(2)
        sb.stop()
        fp=open('stream_test.log','r')
        log_contents=fp.read()
        fp.close()
        self.assertEquals(log_contents.find("received data for stream 'testStream' with no SRI"),-1)


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

