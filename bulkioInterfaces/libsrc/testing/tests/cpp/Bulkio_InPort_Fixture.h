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
#ifndef BULKIO_INPORT_FIXTURE_H
#define BULKIO_INPORT_FIXTURE_H

#include <cppunit/extensions/HelperMacros.h>
#include<ossie/debug.h>

class Bulkio_InPort_Fixture : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( Bulkio_InPort_Fixture );
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
  CPPUNIT_TEST( test_create_double );
  CPPUNIT_TEST( test_create_file );
  CPPUNIT_TEST( test_file );
  CPPUNIT_TEST( test_create_xml );
  CPPUNIT_TEST( test_xml );
  CPPUNIT_TEST( test_create_sdds );
  CPPUNIT_TEST( test_sdds );
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
  void test_create_double();
  void test_create_file();
  void test_file();
  void test_create_xml();
  void test_xml();
  void test_create_sdds();
  void test_sdds();
  void test_subclass();

  template < typename T > void test_port_api( T *port );

  rh_logger::LoggerPtr logger;
};

#endif  // BULKIO_InPort_FIXTURE_H
