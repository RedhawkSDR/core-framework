/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef ANYUTILSTEST_H
#define ANYUTILSTEST_H

#include <cppunit/extensions/HelperMacros.h>

#include <ossie/AnyUtils.h>

class AnyUtilsTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(AnyUtilsTest);
    CPPUNIT_TEST(testIsNull);
    CPPUNIT_TEST(testToBoolean);
    CPPUNIT_TEST(testOctetConversion);
    CPPUNIT_TEST(testShortConversion);
    CPPUNIT_TEST(testUShortConversion);
    CPPUNIT_TEST(testLongConversion);
    CPPUNIT_TEST(testULongConversion);
    CPPUNIT_TEST(testLongLongConversion);
    CPPUNIT_TEST(testULongLongConversion);
    CPPUNIT_TEST(testFloatConversion);
    CPPUNIT_TEST(testStringToNumber);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testIsNull();

    void testToBoolean();
    void testOctetConversion();
    void testShortConversion();
    void testUShortConversion();
    void testLongConversion();
    void testULongConversion();
    void testLongLongConversion();
    void testULongLongConversion();
    void testFloatConversion();

    void testStringToNumber();

private:
    template <typename T>
    void testConversionImpl(T (*func)(const CORBA::Any&));

    template <typename T>
    void testFromBoolean(T (*func)(const CORBA::Any&));

    template <typename T>
    void testConversionRange(T (*func)(const CORBA::Any&));
};

#endif // ANYUTILS_TEST_H
