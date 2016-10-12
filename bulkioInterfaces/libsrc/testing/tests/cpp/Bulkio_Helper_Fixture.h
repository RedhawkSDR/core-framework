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
#ifndef BULKIO_HELPER_FIXTURE_H
#define BULKIO_HELPER_FIXTURE_H

#include <cppunit/extensions/HelperMacros.h>

class Bulkio_Helper_Fixture : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( Bulkio_Helper_Fixture );
  CPPUNIT_TEST( test_sri_create );
  CPPUNIT_TEST( test_sri_compare );
  CPPUNIT_TEST( test_time_now );
  CPPUNIT_TEST( test_time_create );
  CPPUNIT_TEST( test_time_compare );
  CPPUNIT_TEST( test_time_normalize );
  CPPUNIT_TEST( test_time_operators );
  CPPUNIT_TEST( test_time_string );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void test_sri_create();
  void test_sri_compare();

  void test_time_now();
  void test_time_create();
  void test_time_compare();
  void test_time_normalize();
  void test_time_operators();
  void test_time_string();
};

#endif  // BULKIO_HELPER_FIXTURE_H
