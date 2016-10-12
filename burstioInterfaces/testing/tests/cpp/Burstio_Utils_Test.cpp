/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK burstioInterfaces.
 *
 * REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include "Burstio_Utils_Test.h"
#include "burstio.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Burstio_Utils_Test );

void 
Burstio_Utils_Test::setUp()
{
   logger = rh_logger::Logger::getLogger("Burstio-Utils");
   logger->setLevel( rh_logger::Level::getError());
}


void 
Burstio_Utils_Test::tearDown()
{
}



void  Burstio_Utils_Test::test_time_now() {
  BULKIO::PrecisionUTCTime ts = burstio::utils::now();
  CPPUNIT_ASSERT_MESSAGE( " tcmode mismatch.", ts.tcmode == BULKIO::TCM_CPU );
  CPPUNIT_ASSERT_MESSAGE( " tcstatus mismatch.", ts.tcstatus == BULKIO::TCS_VALID );
}


void  Burstio_Utils_Test::test_sri_create() {

  BURSTIO::BurstSRI sri = burstio::utils::createSRI("defaultSRI");
  CPPUNIT_ASSERT_MESSAGE("Stream ID mismatch.", strcmp( "defaultSRI", sri.streamID) == 0 );
  CPPUNIT_ASSERT_MESSAGE("Version mismatch.", 1==sri.hversion );
  CPPUNIT_ASSERT_MESSAGE("XDelta mismatch.", sri.xdelta==1.000);
  CPPUNIT_ASSERT_MESSAGE("Mode mismatch.", sri.mode==0);
  CPPUNIT_ASSERT_MESSAGE("Flags mismatch.", sri.flags==0);
  CPPUNIT_ASSERT_MESSAGE("Tau mismatch.", sri.tau==0.00);
  CPPUNIT_ASSERT_MESSAGE("Theta mismatch.", sri.theta==0.00);
  CPPUNIT_ASSERT_MESSAGE("UW Length mismatch.", sri.uwlength==0);
  CPPUNIT_ASSERT_MESSAGE("Burst Type mismatch.", sri.bursttype==0);
  CPPUNIT_ASSERT_MESSAGE("Burst Length mismatch.", sri.burstLength==0);
  CPPUNIT_ASSERT_MESSAGE("CHAN_RF mismatch.", sri.CHAN_RF==0.00);
  CPPUNIT_ASSERT_MESSAGE("Baud Estimate mismatch.", sri.baudestimate==0.00);
  CPPUNIT_ASSERT_MESSAGE("Baud Rate mismatch.", sri.baudrate==0.00);
  CPPUNIT_ASSERT_MESSAGE("Carrier Offset mismatch.", sri.carrieroffset==0.00);
  CPPUNIT_ASSERT_MESSAGE("SNR mismatch.", sri.SNR==0.00);
  CPPUNIT_ASSERT_MESSAGE("Modulation mismatch.", strcmp(sri.modulation,"") == 0 );
  CPPUNIT_ASSERT_MESSAGE("FEC mismatch.",  strcmp(sri.fec,"") == 0 );
  CPPUNIT_ASSERT_MESSAGE("FEC Rate mismatch.",  strcmp( sri.fecrate, "" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE("Randomizer mismatch.", strcmp( sri.randomizer, "") == 0 );
  CPPUNIT_ASSERT_MESSAGE("Overhead mismatch.", strcmp( sri.overhead, "" ) == 0 );
  CPPUNIT_ASSERT_MESSAGE("Keywords Length mismatch.", sri.keywords.length()== 0);
}



void  Burstio_Utils_Test::test_keywords() {
  BURSTIO::BurstSRI::_keywords_seq kwl;

  BURSTIO::BurstSRI sri = burstio::utils::createSRI("defaultSRI");
  burstio::utils::addKeyword( kwl, "test-kw-one", 22.5 );

  CPPUNIT_ASSERT_MESSAGE( "Keyword list length mismatch.", kwl.length() == 1 );

  burstio::utils::addKeyword( kwl, "test-kw-two", 115.5 );
  CPPUNIT_ASSERT_MESSAGE( "Keyword list length mismatch.", kwl.length() == 2 );

}

void  Burstio_Utils_Test::test_time_elapsed() {

  BULKIO::PrecisionUTCTime b = burstio::utils::now();
  BULKIO::PrecisionUTCTime e = b;
        
  double elapsed = burstio::utils::elapsed( b, e );
  CPPUNIT_ASSERT_MESSAGE("Elapse same begin/end mismatch.", elapsed==0.00);

  e=burstio::utils::now();
  b.twsec = 100;
  b.tfsec = 0;
  e.twsec = 200;
  e.tfsec = 0;
  elapsed = burstio::utils::elapsed( b, e );
  CPPUNIT_ASSERT_MESSAGE( "Elapsed calc mismatch.", elapsed==100.0);

  e=burstio::utils::now();
  b.twsec = 100;
  b.tfsec = 50;
  e.twsec = 200;
  e.tfsec = 100;
  elapsed = burstio::utils::elapsed( b, e );
  CPPUNIT_ASSERT_MESSAGE("Elapsed calc mismatch.", elapsed == 150.0 );

}
