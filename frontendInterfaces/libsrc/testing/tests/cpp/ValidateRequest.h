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
#ifndef FRONTEND_VALIDATEREQUESTTEST_H
#define FRONTEND_VALIDATEREQUESTTEST_H

#include <cppunit/extensions/HelperMacros.h>

class ValidateRequestTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(ValidateRequestTest);
    CPPUNIT_TEST(testSRI);
    CPPUNIT_TEST(testDeviceSRI);
    CPPUNIT_TEST(testRFInfo);
    CPPUNIT_TEST(testDeviceRFInfo);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testSRI();
    void testDeviceSRI();
    void testRFInfo();
    void testDeviceRFInfo();
};

#endif  // FRONTEND_VALIDATEREQUESTTEST_H
