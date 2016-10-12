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
  BULKIO::PrecisionUTCTime  A = bulkio::time::utils::create();
  BULKIO::PrecisionUTCTime  B = A;
  // Wait a brief time to ensure that "now" has changed
  usleep(100);
  BULKIO::PrecisionUTCTime  C = bulkio::time::utils::create();

  CPPUNIT_ASSERT( bulkio::time::DefaultComparator(A , B) == true );
  CPPUNIT_ASSERT( bulkio::time::DefaultComparator(A , C) == false );

}


