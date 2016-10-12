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

import copy
from omniORB import any

from base_ports  import *
from bulkio import *
from ossie.cf import CF

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

    def assertTimeEqual(self, t1, t2, msg=''):
        self.assertEqual(t1, t2, msg='%s expected:<%s> but was:<%s>' % (msg, t1, t2))

    def test_timestamp_normalize(self):
        # NOTE: All tests use fractional portions that are exact binary fractions to
        # avoid potential roundoff issues

        # Already normalized, no change
        tstamp = bulkio.timestamp.create(100.0, 0.5)
        bulkio.timestamp.normalize(tstamp)
        self.assertTimeEqual(bulkio.timestamp.create(100.0, 0.5), tstamp, msg="Already normalized time")

        # Whole seconds has fractional portion, should be moved to fractional seconds
        tstamp.twsec = 100.25;
        tstamp.tfsec = 0.25;
        bulkio.timestamp.normalize(tstamp)
        self.assertTimeEqual(bulkio.timestamp.create(100.0, 0.5), tstamp, msg="Normalizing whole")

        # Whole seconds has fractional portion, should be moved to fractional seconds
        # leading to carry
        tstamp.twsec = 100.75;
        tstamp.tfsec = 0.75;
        bulkio.timestamp.normalize(tstamp)
        self.assertTimeEqual(bulkio.timestamp.create(101.0, 0.5), tstamp, msg="Normalizing whole with carry")

        # Fractional seconds contains whole portion, should be moved to whole seconds
        tstamp.twsec = 100.0;
        tstamp.tfsec = 2.5;
        bulkio.timestamp.normalize(tstamp)
        self.assertTimeEqual(bulkio.timestamp.create(102.0, 0.5), tstamp, msg="Normalizing fractional")

        # Both parts require normalization; fractional portion of whole seconds adds an
        # additional carry
        tstamp.twsec = 100.75;
        tstamp.tfsec = 2.75;
        bulkio.timestamp.normalize(tstamp)
        self.assertTimeEqual(bulkio.timestamp.create(103.0, 0.5), tstamp, msg="Normalizing both")

        # Negative fractional value should borrow
        tstamp.twsec = 100.0;
        tstamp.tfsec = -0.25;
        bulkio.timestamp.normalize(tstamp)
        self.assertTimeEqual(bulkio.timestamp.create(99.0, 0.75), tstamp, msg="Normalizing negative fractional")

        # Negative fractional value with magnitude greater than one
        tstamp.twsec = 100.0;
        tstamp.tfsec = -3.125;
        bulkio.timestamp.normalize(tstamp)
        self.assertTimeEqual(bulkio.timestamp.create(96.0, 0.875), tstamp, msg="Normalizing negative fractional > 1")

        # Fractional portion of whole seconds greater than negative fractional seconds
        tstamp.twsec = 100.5;
        tstamp.tfsec = -.125;
        bulkio.timestamp.normalize(tstamp)
        self.assertTimeEqual(bulkio.timestamp.create(100.0, 0.375), tstamp, msg="Normalizing both with negative fractional")

        # Negative fractional seconds greater than fractional portion of whole seconds
        tstamp.twsec = 100.125;
        tstamp.tfsec = -.5;
        bulkio.timestamp.normalize(tstamp)
        self.assertTimeEqual(bulkio.timestamp.create(99.0, 0.625), tstamp, msg="Normalizing both with borrow")

        # Negative fractional seconds have whole portion, but seconds whole seconds have
        # fractional portion with larger magnitude than remaining fractional seconds 
        tstamp.twsec = 100.75;
        tstamp.tfsec = -2.5;
        bulkio.timestamp.normalize(tstamp)
        self.assertTimeEqual(bulkio.timestamp.create(98.0, 0.25), tstamp, msg="Normalizing both with negative fractional > 1")
  
    def test_timestamp_compare(self):
        t1 = bulkio.timestamp.create(100.0, 0.5)
        t2 = bulkio.timestamp.create(100.0, 0.5)
        self.assertTimeEqual(t1, t2)

        # Only fractional seconds differ
        t1 = bulkio.timestamp.create(100.0, 0.5)
        t2 = bulkio.timestamp.create(100.0, 0.25)
        self.assertTrue(t1 > t2, msg="Time with larger fractional did not compare greater")
        self.assertTrue(t2 < t1, msg="Time with smaller fractional did not compare lesser")

        # Only whole seconds differ
        t1 = bulkio.timestamp.create(100.0, 0.75)
        t2 = bulkio.timestamp.create(101.0, 0.75)
        self.assertTrue(t1 < t2, msg="Time with smaller whole did not compare lesser")
        self.assertTrue(t2 > t1, msg="Time with larger whole did not compare greater")

        # Whole seconds differ, but fractional seconds have the opposite ordering (which has no effect)
        t1 = bulkio.timestamp.create(100.0, 0.75)
        t2 = bulkio.timestamp.create(5000.0, 0.25)
        self.assertTrue(t1 < t2, msg="Time with smaller whole and larger fractional did not compare lesser")
        self.assertTrue(t2 > t1, msg="Time with larger whole and smaller fractional did not compare greater")

    def test_timestamp_operators(self):
        # Test that copy works as expected
        reference = bulkio.timestamp.create(100.0, 0.5)
        t1 = copy.copy(reference)
        self.assertTimeEqual(reference, t1, msg="copy.copy() returned different values")

        # Add a positive offset
        result = t1 + 1.75
        expected = bulkio.timestamp.create(102.0, 0.25)
        self.assertTrue(result is not t1, msg="Add returned same object")
        self.assertTimeEqual(reference, t1, msg="Add modified original value")
        self.assertTimeEqual(expected, result, msg="Add positive offset")

        # Add a negative offset (i.e., subtract)
        result = t1 + -1.75
        expected = bulkio.timestamp.create(98.0, 0.75)
        self.assertTimeEqual(reference, t1, msg="Add modified original value")
        self.assertTimeEqual(expected, result, msg="Add negative offset")

        # Increment by positive offset
        t1 += 2.25
        expected = bulkio.timestamp.create(102.0, 0.75)
        self.assertTimeEqual(expected, t1, msg="Increment by positive offset")

        # Increment by negative offset (i.e., decrement)
        t1 += -3.875
        expected = bulkio.timestamp.create(98.0, 0.875)
        self.assertTimeEqual(expected, t1, msg="Increment by negative offset")

        # Reset to reference time and subtract a positive offset
        t1 = copy.copy(reference)
        result = t1 - 1.25
        expected = bulkio.timestamp.create(99.0, 0.25)
        self.assertTrue(result is not t1, msg="Subtract returned same object")
        self.assertTimeEqual(reference, t1, msg="Subtract modified original value")
        self.assertTimeEqual(expected, result, msg="Subtract positive offset")

        # Subtract a negative offset (i.e., add)
        result = t1 - -4.875
        expected = bulkio.timestamp.create(105.0, 0.375)
        self.assertTrue(result is not t1, msg="Subtract returned same object")
        self.assertTimeEqual(reference, t1, msg="Subtract modified original value")
        self.assertTimeEqual(expected, result, msg="Subtract negative offset")

        # Decrement by positive offset
        t1 -= 2.75
        expected = bulkio.timestamp.create(97.0, 0.75)
        self.assertTimeEqual(expected, t1, msg="Decrement by positive offset")

        # Decrement by negative offset (i.e., increment)
        t1 -= -3.375
        expected = bulkio.timestamp.create(101.0, 0.125)
        self.assertTimeEqual(expected, t1, msg="Decrement by negative offset")

        # Difference, both positive and negative (exact binary fractions used to allow
        # exact comparison)
        t1 = reference + 8.875
        self.assertEqual(t1 - reference, 8.875)
        self.assertEqual(reference - t1, -8.875)

    def test_timestamp_str(self):
        # Test the default epoch (Unix time)
        tstamp = bulkio.timestamp.create(0.0, 0.0)
        self.assertEqual("1970:01:01::00:00:00.000000", str(tstamp))

        # Use a recent time with rounding at the microsecond level
        tstamp = bulkio.timestamp.create(1451933967.0, 0.2893569)
        self.assertEqual("2016:01:04::18:59:27.289357", str(tstamp))

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

