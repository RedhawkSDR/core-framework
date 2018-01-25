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

#include "CFTest.h"

#include <ossie/AnyUtils.h>

#define FOREACH_TEST(X, T)                      \
    X(T, FromBoolean)                           \
    X(T, FromNumber)                            \
    X(T, FromString)                            \
    X(T, Range)

#define FOREACH_TYPE_TEST(X)                    \
    FOREACH_TEST(X, Octet)                      \
    FOREACH_TEST(X, Short)                      \
    FOREACH_TEST(X, UShort)                     \
    FOREACH_TEST(X, Long)                       \
    FOREACH_TEST(X, ULong)                      \
    FOREACH_TEST(X, LongLong)                   \
    FOREACH_TEST(X, ULongLong)                  \
    FOREACH_TEST(X, Float)                      \
    FOREACH_TEST(X, Double)

class AnyUtilsTest : public CppUnit::TestFixture
{
#define REGISTER_TESTS(T,NAME) CPPUNIT_TEST(testTo##T##NAME);

    CPPUNIT_TEST_SUITE(AnyUtilsTest);
    CPPUNIT_TEST(testIsNull);
    CPPUNIT_TEST(testToBoolean);
    FOREACH_TYPE_TEST(REGISTER_TESTS);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testIsNull();

    void testToBoolean();

#define DECLARE_TESTS(T,NAME) void testTo##T##NAME();
    FOREACH_TYPE_TEST(DECLARE_TESTS);
};

#endif // ANYUTILS_TEST_H
