/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class PrecisionUTCTimeTest
{
    private void failTime(String message, BULKIO.PrecisionUTCTime expected, BULKIO.PrecisionUTCTime actual) {
        Assert.fail(String.format("%1$s expected:<%2$s> but was:<%3$s>", message,
                                  bulkio.time.utils.toString(expected), bulkio.time.utils.toString(actual)));
    }

    private void assertTimeEquals(String message, BULKIO.PrecisionUTCTime expected, BULKIO.PrecisionUTCTime actual) {
        if (bulkio.time.utils.compare(expected, actual) != 0) {
            failTime(message, expected, actual);
        }
    }

    private void assertTimeNotEquals(String message, BULKIO.PrecisionUTCTime expected, BULKIO.PrecisionUTCTime actual) {
        if (bulkio.time.utils.compare(expected, actual) == 0) {
            failTime(message, expected, actual);
        }
    }

    @Test
    public void testCreate() {
        BULKIO.PrecisionUTCTime ts = bulkio.time.utils.now();
	Assert.assertEquals("tcmode mismatch", ts.tcmode, BULKIO.TCM_CPU.value);
        Assert.assertEquals("tcstatus mismatch", ts.tcstatus, BULKIO.TCS_VALID.value);
        Assert.assertEquals("tcoff mismatch", ts.toff, 0.00, 3);

        ts = bulkio.time.utils.create(100.0, 0.125);
	Assert.assertEquals("tcmode mismatch", ts.tcmode, BULKIO.TCM_CPU.value);
        Assert.assertEquals("tcstatus mismatch", ts.tcstatus, BULKIO.TCS_VALID.value);
        Assert.assertEquals("tcwsec mismatch", ts.twsec, 100.0, 3);
        Assert.assertEquals("tcwsec mismatch", ts.tfsec, 0.125, 3);

        ts = bulkio.time.utils.create(100.0, 0.125, BULKIO.TCM_SDDS.value);
	Assert.assertEquals("tcmode mismatch", ts.tcmode, BULKIO.TCM_SDDS.value);
        Assert.assertEquals("tcstatus mismatch", ts.tcstatus, BULKIO.TCS_VALID.value);
        Assert.assertEquals("tcwsec mismatch", ts.twsec, 100.0, 3);
        Assert.assertEquals("tcwsec mismatch", ts.tfsec, 0.125, 3);
    }

    @Test
    public void testNormalize() {
        // NOTE: All tests use fractional portions that are exact binary fractions to
        // avoid potential roundoff issues

        // Already normalized, no change
        BULKIO.PrecisionUTCTime time = new BULKIO.PrecisionUTCTime();
        time.twsec = 100.0;
        time.tfsec = 0.5;
        bulkio.time.utils.normalize(time);
        assertTimeEquals("Already normalized time", bulkio.time.utils.create(100.0, 0.5), time);

        // Whole seconds has fractional portion, should be moved to fractional seconds
        time.twsec = 100.25;
        time.tfsec = 0.25;
        bulkio.time.utils.normalize(time);
        assertTimeEquals("Normalizing whole", bulkio.time.utils.create(100.0, 0.5), time);

        // Whole seconds has fractional portion, should be moved to fractional seconds
        // leading to carry
        time.twsec = 100.75;
        time.tfsec = 0.75;
        bulkio.time.utils.normalize(time);
        assertTimeEquals("Normalizing whole with carry", bulkio.time.utils.create(101.0, 0.5), time);

        // Fractional seconds contains whole portion, should be moved to whole seconds
        time.twsec = 100.0;
        time.tfsec = 2.5;
        bulkio.time.utils.normalize(time);
        assertTimeEquals("Normalizing fractional", bulkio.time.utils.create(102.0, 0.5), time);

        // Both parts require normalization; fractional portion of whole seconds adds an
        // additional carry
        time.twsec = 100.75;
        time.tfsec = 2.75;
        bulkio.time.utils.normalize(time);
        assertTimeEquals("Normalizing both", bulkio.time.utils.create(103.0, 0.5), time);

        // Negative fractional value should borrow
        time.twsec = 100.0;
        time.tfsec = -0.25;
        bulkio.time.utils.normalize(time);
        assertTimeEquals("Normalizing negative fractional", bulkio.time.utils.create(99.0, 0.75), time);

        // Negative fractional value with magnitude greater than one
        time.twsec = 100.0;
        time.tfsec = -3.125;
        bulkio.time.utils.normalize(time);
        assertTimeEquals("Normalizing negative fractional > 1", bulkio.time.utils.create(96.0, 0.875), time);

        // Fractional portion of whole seconds greater than negative fractional seconds
        time.twsec = 100.5;
        time.tfsec = -.125;
        bulkio.time.utils.normalize(time);
        assertTimeEquals("Normalizing both with negative fractional", bulkio.time.utils.create(100.0, 0.375), time);

        // Negative fractional seconds greater than fractional portion of whole seconds
        time.twsec = 100.125;
        time.tfsec = -.5;
        bulkio.time.utils.normalize(time);
        assertTimeEquals("Normalizing both with borrow", bulkio.time.utils.create(99.0, 0.625), time);

        // Negative fractional seconds have whole portion, but seconds whole seconds have
        // fractional portion with larger magnitude than remaining fractional seconds 
        time.twsec = 100.75;
        time.tfsec = -2.5;
        bulkio.time.utils.normalize(time);
        assertTimeEquals("Normalizing both with negative fractional > 1", bulkio.time.utils.create(98.0, 0.25), time);
    }

    @Test
    public void testCompare() {
        BULKIO.PrecisionUTCTime t1 = bulkio.time.utils.create(100.0, 0.5);
        BULKIO.PrecisionUTCTime t2 = bulkio.time.utils.create(100.0, 0.5);
        Assert.assertTrue("Identical values did not compare as equal", bulkio.time.utils.compare(t1, t2) == 0);

        // Only fractional seconds differ
        t1 = bulkio.time.utils.create(100.0, 0.5);
        t2 = bulkio.time.utils.create(100.0, 0.25);
        Assert.assertTrue("Time with larger fractional did not compare greater", bulkio.time.utils.compare(t1, t2) > 0);
        Assert.assertTrue("Time with smaller fractional did not compare lesser", bulkio.time.utils.compare(t2, t1) < 0);

        // Only whole seconds differ
        t1 = bulkio.time.utils.create(100.0, 0.75);
        t2 = bulkio.time.utils.create(101.0, 0.75);
        Assert.assertTrue("Time with smaller whole did not compare lesser", bulkio.time.utils.compare(t1, t2) < 0);
        Assert.assertTrue("Time with larger whole did not compare greater", bulkio.time.utils.compare(t2, t1) > 0);

        // Whole seconds differ, but fractional seconds have the opposite ordering (which has no effect)
        t1 = bulkio.time.utils.create(100.0, 0.75);
        t2 = bulkio.time.utils.create(5000.0, 0.25);
        Assert.assertTrue("Time with smaller whole and larger fractional did not compare lesser", bulkio.time.utils.compare(t1, t2) < 0);
        Assert.assertTrue("Time with larger whole and smaller fractional did not compare greater", bulkio.time.utils.compare(t2, t1) > 0);
    }

    @Test
    public void testOperators() {
        // Test that copy works as expected
        final BULKIO.PrecisionUTCTime reference = bulkio.time.utils.create(100.0, 0.5);
        BULKIO.PrecisionUTCTime t1 = bulkio.time.utils.copy(reference);
        Assert.assertNotSame("Copy returned same object", reference, t1);
        assertTimeEquals("Copy returned different values", reference, t1);

        // Add a positive offset
        BULKIO.PrecisionUTCTime result = bulkio.time.utils.add(t1, 1.75);
        BULKIO.PrecisionUTCTime expected = bulkio.time.utils.create(102.0, 0.25);
        Assert.assertNotSame("Add returned same object", t1, result);
        assertTimeEquals("Original value modified", reference, t1);
        assertTimeEquals("Add positive offset", expected, result);

        // Add a negative offset (i.e., subtract)
        result = bulkio.time.utils.add(t1, -1.75);
        expected = bulkio.time.utils.create(98.0, 0.75);
        assertTimeEquals("Original value modified", reference, t1);
        assertTimeEquals("Add negative offset", expected, result);

        // Increment by positive offset
        result = bulkio.time.utils.increment(t1, 2.25);
        expected = bulkio.time.utils.create(102.0, 0.75);
        Assert.assertSame("Increment returned different object", t1, result);
        assertTimeEquals("Increment by positive offset", expected, t1);

        // Increment by negative offset (i.e., decrement)
        bulkio.time.utils.increment(t1, -3.875);
        expected = bulkio.time.utils.create(98.0, 0.875);
        assertTimeEquals("Increment by negative offset", expected, t1);

        // Reset to reference time and subtract a positive offset
        t1 = bulkio.time.utils.copy(reference);
        result = bulkio.time.utils.subtract(t1, 1.25);
        expected = bulkio.time.utils.create(99.0, 0.25);
        Assert.assertNotSame("Subtract returned same object", t1, result);
        assertTimeEquals("Original value modified", reference, t1);
        assertTimeEquals("Subtract positive offset", expected, result);

        // Subtract a negative offset (i.e., add)
        result = bulkio.time.utils.subtract(t1, -4.875);
        expected = bulkio.time.utils.create(105.0, 0.375);
        assertTimeEquals("Original value modified", reference, t1);
        assertTimeEquals("Subtract negative offset", expected, result);

        // Decrement by positive offset
        result = bulkio.time.utils.decrement(t1, 2.75);
        expected = bulkio.time.utils.create(97.0, 0.75);
        Assert.assertSame("Increment returned different object", t1, result);
        assertTimeEquals("Decrement by positive offset", expected, t1);

        // Decrement by negative offset (i.e., increment)
        bulkio.time.utils.decrement(t1, -3.375);
        expected = bulkio.time.utils.create(101.0, 0.125);
        assertTimeEquals("Decrement by negative offset", expected, t1);

        // Difference, both positive and negative (exact binary fractions used to allow
        // exact comparison)
        t1 = bulkio.time.utils.add(reference, 8.875);
        Assert.assertEquals("Positive time difference", bulkio.time.utils.difference(t1, reference), 8.875, 0.0);
        Assert.assertEquals("Negative time difference", bulkio.time.utils.difference(reference, t1), -8.875, 0.0);
    }

    @Test
    public void testString() {
        // Test the default epoch (Unix time)
        BULKIO.PrecisionUTCTime time = bulkio.time.utils.create(0.0, 0.0);
        Assert.assertEquals("Epoch", "1970:01:01::00:00:00.000000", bulkio.time.utils.toString(time));

        // Use a recent time with rounding at the microsecond level
        time = bulkio.time.utils.create(1451933967.0, 0.2893569);
        Assert.assertEquals("String representation", "2016:01:04::18:59:27.289357", bulkio.time.utils.toString(time));
    }
}
