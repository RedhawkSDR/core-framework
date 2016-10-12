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
#include "Bulkio_Helper_Fixture.h"
#include "bulkio.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Bulkio_Helper_Fixture );


void 
Bulkio_Helper_Fixture::setUp()
{
}


void 
Bulkio_Helper_Fixture::tearDown()
{
}


void 
Bulkio_Helper_Fixture::test_sri_create()
{
  BULKIO::StreamSRI sri = bulkio::sri::create();
}


void 
Bulkio_Helper_Fixture::test_sri_compare()
{
  BULKIO::StreamSRI A = bulkio::sri::create();
  BULKIO::StreamSRI B = bulkio::sri::create();
  BULKIO::StreamSRI C = bulkio::sri::create();

  C.streamID = std::string("No Match").c_str();

  CPPUNIT_ASSERT( bulkio::sri::DefaultComparator(A , B) == true );
  CPPUNIT_ASSERT( bulkio::sri::DefaultComparator(A , C) == false );

}

void 
Bulkio_Helper_Fixture::test_time_now()
{
  BULKIO::PrecisionUTCTime  T = bulkio::time::utils::now();
}

void 
Bulkio_Helper_Fixture::test_time_create()
{
  const double wsec = 100.0;
  const double fsec = 0.125;
  BULKIO::PrecisionUTCTime  T = bulkio::time::utils::create(100.0, 0.125);
  
  CPPUNIT_ASSERT( T.twsec == wsec );
  CPPUNIT_ASSERT( T.tfsec == fsec );
}

void 
Bulkio_Helper_Fixture::test_time_compare()
{
  BULKIO::PrecisionUTCTime t1 = bulkio::time::utils::create(100.0, 0.5);
  BULKIO::PrecisionUTCTime t2 = bulkio::time::utils::create(100.0, 0.5);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Identical times did not compare equal", t1, t2);
  CPPUNIT_ASSERT_MESSAGE("Identical times did not compare as >=", t1 >= t2);
  CPPUNIT_ASSERT_MESSAGE("Identical times did not compare as <=", t2 <= t1);
  CPPUNIT_ASSERT_MESSAGE("Identical times compared as >", !(t1 > t2));
  CPPUNIT_ASSERT_MESSAGE("Identical times compared as <", !(t1 < t2));

  // Only fractional seconds differ
  t1 = bulkio::time::utils::create(100.0, 0.5);
  t2 = bulkio::time::utils::create(100.0, 0.25);
  CPPUNIT_ASSERT_MESSAGE("Different times did not compare !=", t1 != t2);
  CPPUNIT_ASSERT_MESSAGE("Time with larger fractional did not compare >", t1 > t2);
  CPPUNIT_ASSERT_MESSAGE("Time with smaller fractional did not compare <", t2 < t1);

  // Only whole seconds differ
  t1 = bulkio::time::utils::create(100.0, 0.75);
  t2 = bulkio::time::utils::create(101.0, 0.75);
  CPPUNIT_ASSERT_MESSAGE("Different times did not compare !=", t1 != t2);
  CPPUNIT_ASSERT_MESSAGE("Time with smaller whole did not compare <=", t1 <= t2);
  CPPUNIT_ASSERT_MESSAGE("Time with larger whole did not compare >=", t2 >= t1);

  // Whole seconds differ, but fractional seconds have the opposite ordering (which has no effect)
  t1 = bulkio::time::utils::create(100.0, 0.75);
  t2 = bulkio::time::utils::create(5000.0, 0.25);
  CPPUNIT_ASSERT_MESSAGE("Different times compared equal", !(t1 == t2));
  CPPUNIT_ASSERT_MESSAGE("Time with smaller whole and larger fractional did not compare >", t1 < t2);
  CPPUNIT_ASSERT_MESSAGE("Time with larger whole and smaller fractional did not compare <", t2 > t1);
}

void
Bulkio_Helper_Fixture::test_time_normalize()
{
  // NOTE: All tests use fractional portions that are exact binary fractions to
  // avoid potential roundoff issues

  // Already normalized, no change
  BULKIO::PrecisionUTCTime time = bulkio::time::utils::create(100.0, 0.5);
  bulkio::time::utils::normalize(time);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Already normalized time", bulkio::time::utils::create(100.0, 0.5), time);

  // Whole seconds has fractional portion, should be moved to fractional seconds
  time.twsec = 100.25;
  time.tfsec = 0.25;
  bulkio::time::utils::normalize(time);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Normalizing whole", bulkio::time::utils::create(100.0, 0.5), time);

  // Whole seconds has fractional portion, should be moved to fractional seconds
  // leading to carry
  time.twsec = 100.75;
  time.tfsec = 0.75;
  bulkio::time::utils::normalize(time);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Normalizing whole with carry", bulkio::time::utils::create(101.0, 0.5), time);

  // Fractional seconds contains whole portion, should be moved to whole seconds
  time.twsec = 100.0;
  time.tfsec = 2.5;
  bulkio::time::utils::normalize(time);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Normalizing fractional", bulkio::time::utils::create(102.0, 0.5), time);

  // Both parts require normalization; fractional portion of whole seconds adds an
  // additional carry
  time.twsec = 100.75;
  time.tfsec = 2.75;
  bulkio::time::utils::normalize(time);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Normalizing both", bulkio::time::utils::create(103.0, 0.5), time);

  // Negative fractional value should borrow
  time.twsec = 100.0;
  time.tfsec = -0.25;
  bulkio::time::utils::normalize(time);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Normalizing negative fractional", bulkio::time::utils::create(99.0, 0.75), time);

  // Negative fractional value with magnitude greater than one
  time.twsec = 100.0;
  time.tfsec = -3.125;
  bulkio::time::utils::normalize(time);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Normalizing negative fractional > 1", bulkio::time::utils::create(96.0, 0.875), time);

  // Fractional portion of whole seconds greater than negative fractional seconds
  time.twsec = 100.5;
  time.tfsec = -.125;
  bulkio::time::utils::normalize(time);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Normalizing both with negative fractional", bulkio::time::utils::create(100.0, 0.375), time);

  // Negative fractional seconds greater than fractional portion of whole seconds
  time.twsec = 100.125;
  time.tfsec = -.5;
  bulkio::time::utils::normalize(time);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Normalizing both with borrow", bulkio::time::utils::create(99.0, 0.625), time);

  // Negative fractional seconds have whole portion, but seconds whole seconds have
  // fractional portion with larger magnitude than remaining fractional seconds 
  time.twsec = 100.75;
  time.tfsec = -2.5;
  bulkio::time::utils::normalize(time);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Normalizing both with negative fractional > 1", bulkio::time::utils::create(98.0, 0.25), time);
}

void
Bulkio_Helper_Fixture::test_time_operators()
{
  // NOTE: All tests use fractional portions that are exact binary fractions to
  // avoid potential roundoff issues

  // Test that copy works as expected
  const BULKIO::PrecisionUTCTime reference = bulkio::time::utils::create(100.0, 0.5);
  BULKIO::PrecisionUTCTime t1 = reference;
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Copy returned different values", reference, t1);

  // Add a positive offset
  BULKIO::PrecisionUTCTime result = t1 + 1.75;
  BULKIO::PrecisionUTCTime expected = bulkio::time::utils::create(102.0, 0.25);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Original value modified", reference, t1);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Add positive offset", expected, result);

  // Add a negative offset (i.e., subtract)
  result = t1 + -1.75;
  expected = bulkio::time::utils::create(98.0, 0.75);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Original value modified", reference, t1);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Add negative offset", expected, result);

  // Increment by positive offset
  t1 += 2.25;
  expected = bulkio::time::utils::create(102.0, 0.75);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Increment by positive offset", expected, t1);

  // Increment by negative offset (i.e., decrement)
  t1 += -3.875;
  expected = bulkio::time::utils::create(98.0, 0.875);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Increment by negative offset", expected, t1);

  // Reset to reference time and subtract a positive offset
  t1 = reference;
  result = t1 - 1.25;
  expected = bulkio::time::utils::create(99.0, 0.25);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Original value modified", reference, t1);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Subtract positive offset", expected, result);

  // Subtract a negative offset (i.e., add)
  result = t1 - -4.875;
  expected = bulkio::time::utils::create(105.0, 0.375);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Original value modified", reference, t1);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Subtract negative offset", expected, result);

  // Decrement by positive offset
  t1 -= 2.75;
  expected = bulkio::time::utils::create(97.0, 0.75);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Decrement by positive offset", expected, t1);

  // Decrement by negative offset (i.e., increment)
  t1 -= -3.375;
  expected = bulkio::time::utils::create(101.0, 0.125);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Decrement by negative offset", expected, t1);

  // Difference, both positive and negative (exact binary fractions used to allow
  // exact comparison)
  t1 = reference + 8.875;
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Positive time difference", t1 - reference, 8.875);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Negative time difference", reference - t1, -8.875);
}

void
Bulkio_Helper_Fixture::test_time_string()
{
  // Test the default epoch (Unix time)
  BULKIO::PrecisionUTCTime time = bulkio::time::utils::create(0.0, 0.0);
  std::ostringstream oss;
  oss << time;
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Epoch", std::string("1970:01:01::00:00:00.000000"), oss.str());

  // Use a recent time with rounding at the microsecond level
  oss.str("");
  oss << bulkio::time::utils::create(1451933967.0, 0.2893569);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Reference", std::string("2016:01:04::18:59:27.289357"), oss.str());
}
