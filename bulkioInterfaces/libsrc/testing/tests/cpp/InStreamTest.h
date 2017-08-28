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
#ifndef BULKIO_INSTREAMTEST_H
#define BULKIO_INSTREAMTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <ossie/debug.h>

template <class Port>
class InStreamTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(InStreamTest);
    CPPUNIT_TEST(testGetCurrentStreamEmptyEos);
    CPPUNIT_TEST(testGetCurrentStreamDataEos);
    CPPUNIT_TEST(testSizedReadEmptyEos);
    CPPUNIT_TEST(testSizedTryreadEmptyEos);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testGetCurrentStreamEmptyEos();
    void testGetCurrentStreamDataEos();

    void testSizedReadEmptyEos();
    void testSizedTryreadEmptyEos();

private:
    virtual std::string getPortName() const = 0;

    Port* port;
};

#endif  // BULKIO_INSTREAMTEST_H
