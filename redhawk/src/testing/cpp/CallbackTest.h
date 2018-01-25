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

#ifndef CALLBACKTEST_H
#define CALLBACKTEST_H

#include "CFTest.h"

class CallbackTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CallbackTest);
    CPPUNIT_TEST(testEmpty);
    CPPUNIT_TEST(testEmptyCall);
    CPPUNIT_TEST(testBooleanOperators);
    CPPUNIT_TEST(testFunction);
    CPPUNIT_TEST(testFunctionEquals);
    CPPUNIT_TEST(testFunctor);
    CPPUNIT_TEST(testFunctorRef);
    CPPUNIT_TEST(testFunctorEquals);
    CPPUNIT_TEST(testMemberFunction);
    CPPUNIT_TEST(testMemberFunctionEquals);
    CPPUNIT_TEST(testMixedEquals);
    CPPUNIT_TEST(testReferenceArguments);
    CPPUNIT_TEST(testArgumentConversion);
    CPPUNIT_TEST(testVoidReturn);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testEmpty();
    void testEmptyCall();
    void testBooleanOperators();

    void testFunction();
    void testFunctionEquals();

    void testFunctor();
    void testFunctorRef();
    void testFunctorEquals();

    void testMemberFunction();
    void testMemberFunctionEquals();

    void testMixedEquals();

    void testReferenceArguments();
    void testArgumentConversion();
    void testVoidReturn();
};

#endif // CALLBACKTEST_H
