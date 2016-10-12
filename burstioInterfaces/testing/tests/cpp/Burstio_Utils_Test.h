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
#ifndef BURSTIO_INPORT_FIXTURE_H
#define BURSTIO_INPORT_FIXTURE_H

#include <cppunit/extensions/HelperMacros.h>
#include "ossie/debug.h"
#include "burstio.h"

class Burstio_Utils_Test : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( Burstio_Utils_Test );
  CPPUNIT_TEST( test_time_now );
  CPPUNIT_TEST( test_sri_create );
  CPPUNIT_TEST( test_keywords );
  CPPUNIT_TEST( test_time_elapsed );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void test_time_now();
  void test_sri_create( );
  void test_keywords ();
  void test_time_elapsed ();

  rh_logger::LoggerPtr logger;
};

#endif  // BURSTIO_InPort_FIXTURE_H
