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

class Test_PythonHelpers(unittest.TestCase):
    def __init__( self, methodName='runTest' ):
        unittest.TestCase.__init__(self, methodName)

    def setUp(self):
        pass

    def tearDown(self):
        pass

    def test_sri_create(self):
        ##
        ## test bulkio helper methods
        ##
        print "Bulkio Helpers:  SRI CREATE"
        sri = bulkio.sri.create()
        self.assertEqual(sri.streamID,"defStream","Stream ID mismatch.")
        self.assertEqual(sri.hversion,1,"Version mismatch.")
        self.assertEqual(sri.xunits,1,"XUnits mismatch.")
        self.assertAlmostEqual(sri.xstart,0.00,3, msg="XStart mismatch.")
        self.assertAlmostEqual(sri.xdelta,1.00,3, msg="XDelta mismatch.")
        self.assertEqual(sri.yunits,0,"YUnits mismatch.")
        self.assertAlmostEqual(sri.ystart,0.00,3, msg="YStart mismatch.")
        self.assertAlmostEqual(sri.ydelta,0.00,3, msg="YDelta mismatch.")
        self.assertEqual(sri.subsize,0,"Subsize mismatch.")
        self.assertEqual(sri.blocking,False,"Blocking mismatch.")
        self.assertEqual(sri.keywords,[],"Keywords mismatch.")

        print "Bulkio Helpers:  SRI CREATE - part Due"
        sri = bulkio.sri.create( "NEW-STREAM-ID" )
        self.assertEqual(sri.streamID,"NEW-STREAM-ID","Stream ID mismatch.")


    def test_sri_compare(self):

        ##
        ## test bulkio helper method
        ##
        print "Bulkio Helpers:  SRI CREATE"
        a_sri = bulkio.sri.create()
        b_sri = bulkio.sri.create()
        c_sri = bulkio.sri.create()
        c_sri.streamID = "THIS_DOES_NOT_MATCH"

        self.assertEqual( bulkio.sri.compare( a_sri, b_sri ), True, " bulkio.sri.compare method - same.")
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - StreamID .")

        c_sri = bulkio.sri.create()
        c_sri.hversion = 2
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - hversion ")

        c_sri = bulkio.sri.create()
        c_sri.xstart = 3
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - xstart ")

        c_sri = bulkio.sri.create()
        c_sri.xdelta = 100.0
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - xdelta ")

        c_sri = bulkio.sri.create()
        c_sri.xunits = 100.0
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - xunits ")

        c_sri = bulkio.sri.create()
        c_sri.subsize = 100
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - subsize ")

        c_sri = bulkio.sri.create()
        c_sri.ystart = 3
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - ystart ")

        c_sri = bulkio.sri.create()
        c_sri.ydelta = 100.0
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - ydelta ")

        c_sri = bulkio.sri.create()
        c_sri.yunits = 100.0
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - yunits ")

        c_sri = bulkio.sri.create()
        c_sri.mode = 100
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - mode ")

        kv = CF.DataType( id="key_one", value=any.to_any(1) )
        kv2 = CF.DataType( id="key_one", value=any.to_any(1) )
        a_sri.keywords = [kv]
        c_sri = bulkio.sri.create()
        c_sri.keywords = [kv2]
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), True, " bulkio.sri.compare method - same - keyword item ")

        kv2 = CF.DataType( id="key_one", value=any.to_any(100) )
        c_sri = bulkio.sri.create()
        c_sri.keywords = [kv2]
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - keywords value mismatch ")

        kv2 = CF.DataType( id="key_two", value=any.to_any(100) )
        c_sri = bulkio.sri.create()
        c_sri.keywords = [kv2]
        self.assertEqual( bulkio.sri.compare( a_sri, c_sri ), False, " bulkio.sri.compare method - different - keywords name mismatch  ")


    def test_timestamp_create(self):
        ##
        ## test bulkio helper methods
        ##
        print "Bulkio Helpers:  TimeStamp CREATE"
        ts = bulkio.timestamp.now()
        self.assertEqual(ts.tcmode,BULKIO.TCM_CPU, msg=" tcmode mismatch.")
        self.assertEqual(ts.tcstatus,BULKIO.TCS_VALID, msg=" tcstatus mismatch.")
        self.assertAlmostEqual(ts.toff,0.00,3, msg=" tcoff  mismatch.")

        print "Bulkio Helpers:  TimeStamp CPU TimeStamp"
        ts = bulkio.timestamp.cpuTimeStamp()
        self.assertEqual(ts.tcmode,BULKIO.TCM_CPU, msg=" tcmode mismatch.")
        self.assertEqual(ts.tcstatus,BULKIO.TCS_VALID, msg=" tcstatus mismatch.")
        self.assertAlmostEqual(ts.toff,0.00,3, msg=" tcoff  mismatch.")

        ##
        ## test bulkio helper methods
        ##
        print "Bulkio Helpers:  TimeStamp CREATE with time"
        ts = bulkio.timestamp.create( 100.0, 0.125 )
        self.assertEqual(ts.tcmode,BULKIO.TCM_CPU, msg=" tcmode mismatch.")
        self.assertEqual(ts.tcstatus,BULKIO.TCS_VALID, msg=" tcstatus mismatch.")
        self.assertAlmostEqual(ts.twsec,100.0, 3, msg=" tcwsec mismatch.")
        self.assertAlmostEqual(ts.tfsec,0.125, 3, msg=" tcfsec mismatch.")

        print "Bulkio Helpers:  TimeStamp CREATE with time"
        ts = bulkio.timestamp.create( 100.0, 0.125, BULKIO.TCM_SDDS)
        self.assertEqual(ts.tcmode,BULKIO.TCM_SDDS, msg=" tcmode mismatch.")
        self.assertEqual(ts.tcstatus,BULKIO.TCS_VALID, msg=" tcstatus mismatch.")
        self.assertAlmostEqual(ts.twsec,100.0, 3, msg=" tcwsec mismatch.")
        self.assertAlmostEqual(ts.tfsec,0.125, 3, msg=" tcfsec mismatch.")


if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ Test_PythonHelpers ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    try:
        import xmlrunner
        runner = xmlrunner.XMLTestRunner(verbosity=2)
    except ImportError:
        runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)

