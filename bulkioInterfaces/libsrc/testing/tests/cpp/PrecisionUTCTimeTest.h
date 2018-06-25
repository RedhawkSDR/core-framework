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
#ifndef BULKIO_PRECISIONUTCTIMETEST_H
#define BULKIO_PRECISIONUTCTIMETEST_H

#include <cppunit/extensions/HelperMacros.h>

class PrecisionUTCTimeTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(PrecisionUTCTimeTest);
    CPPUNIT_TEST(testNow);
    CPPUNIT_TEST(testCreate);
    CPPUNIT_TEST(testCompare);
    CPPUNIT_TEST(testNormalize);
    CPPUNIT_TEST(testOperators);
    CPPUNIT_TEST(testString);
    CPPUNIT_TEST_SUITE_END();

public:
    void testNow();
    void testCreate();
    void testCompare();
    void testNormalize();
    void testOperators();
    void testString();
};

#endif  // BULKIO_PRECISIONUTCTIMETEST_H
