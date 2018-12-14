#!/usr/bin/python
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

import unittest
import copy

import bulkio
from bulkio.bulkioInterfaces import BULKIO

class PrecisionUTCTimeTest(unittest.TestCase):
    def assertTimeEqual(self, t1, t2, msg=''):
        self.assertEqual(t1, t2, msg='%s expected:<%s> but was:<%s>' % (msg, t1, t2))

    def testNow(self):
        ts = bulkio.timestamp.now()
        self.assertEqual(ts.tcmode, BULKIO.TCM_CPU, msg="tcmode mismatch")
        self.assertEqual(ts.tcstatus, BULKIO.TCS_VALID, msg="tcstatus mismatch")
        self.assertEqual(ts.toff, 0, msg="toff mismatch")

        ts = bulkio.timestamp.cpuTimeStamp()
        self.assertEqual(ts.tcmode, BULKIO.TCM_CPU, msg="tcmode mismatch")
        self.assertEqual(ts.tcstatus, BULKIO.TCS_VALID, msg="tcstatus mismatch")
        self.assertEqual(ts.toff, 0, msg="toff mismatch")

    def testCreate(self):
        ts = bulkio.timestamp.create(100.0, 0.125)
        self.assertEqual(ts.tcmode, BULKIO.TCM_CPU, msg="tcmode mismatch")
        self.assertEqual(ts.tcstatus, BULKIO.TCS_VALID, msg="tcstatus mismatch")
        self.assertEqual(ts.twsec, 100.0, msg="tcwsec mismatch")
        self.assertEqual(ts.tfsec, 0.125, msg="tcfsec mismatch")

        ts = bulkio.timestamp.create(100.0, 0.125, BULKIO.TCM_SDDS)
        self.assertEqual(ts.tcmode, BULKIO.TCM_SDDS, msg="tcmode mismatch")
        self.assertEqual(ts.tcstatus, BULKIO.TCS_VALID, msg="tcstatus mismatch")
        self.assertEqual(ts.twsec, 100.0, msg="tcwsec mismatch")
        self.assertEqual(ts.tfsec, 0.125, msg="tcfsec mismatch")

    def testCompare(self):
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
    
    def testNormalize(self):
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
  
    def testOperators(self):
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

    def testString(self):
        # Test the default epoch (Unix time)
        tstamp = bulkio.timestamp.create(0.0, 0.0)
        self.assertEqual("1970:01:01::00:00:00.000000", str(tstamp))

        # Use a recent time with rounding at the microsecond level
        tstamp = bulkio.timestamp.create(1451933967.0, 0.2893569)
        self.assertEqual("2016:01:04::18:59:27.289357", str(tstamp))

if __name__ == '__main__':
    import runtests
    runtests.main()
