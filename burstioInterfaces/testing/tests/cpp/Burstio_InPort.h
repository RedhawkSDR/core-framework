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

class Burstio_InPort : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( Burstio_InPort );
  CPPUNIT_TEST( test_create_int8 );
  CPPUNIT_TEST( test_int8 );
  CPPUNIT_TEST( test_create_int16 );
  CPPUNIT_TEST( test_int16 );
  CPPUNIT_TEST( test_create_int32);
  CPPUNIT_TEST( test_int32 );
  CPPUNIT_TEST( test_create_int64);
  CPPUNIT_TEST( test_int64 );
  CPPUNIT_TEST( test_create_uint8 );
  CPPUNIT_TEST( test_uint8 );
  CPPUNIT_TEST( test_create_uint16 );
  CPPUNIT_TEST( test_uint16 );
  CPPUNIT_TEST( test_create_uint32);
  CPPUNIT_TEST( test_uint32 );
  CPPUNIT_TEST( test_create_uint64);
  CPPUNIT_TEST( test_uint64 );
  CPPUNIT_TEST( test_create_float );
  CPPUNIT_TEST( test_float );
  CPPUNIT_TEST( test_create_double );
  CPPUNIT_TEST( test_double );
  CPPUNIT_TEST( test_subclass );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void test_create_int8();
  void test_int8();
  void test_create_int16();
  void test_int16();
  void test_create_int32();
  void test_int32();
  void test_create_int64();
  void test_int64();
  void test_create_uint8();
  void test_uint8();
  void test_create_uint16();
  void test_uint16();
  void test_create_uint32();
  void test_uint32();
  void test_create_uint64();
  void test_uint64();
  void test_create_float();
  void test_float();
  void test_create_double();
  void test_double();
  void test_subclass();

  template < typename T > void test_port_api( T *port );
  template < typename T > void test_push_flush_sequence( T *port );
  
  BURSTIO::BurstSRI make_sri_test(const  std::string &sid, const std::string &id );
  BURSTIO::BurstSRI make_sri_pkt1();
  BURSTIO::BurstSRI make_sri_pkt2();

  rh_logger::LoggerPtr logger;
};

#endif  // BURSTIO_InPort_FIXTURE_H
