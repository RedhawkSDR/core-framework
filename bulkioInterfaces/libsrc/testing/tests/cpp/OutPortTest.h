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
#ifndef BULKIO_OUTPORTTEST_H
#define BULKIO_OUTPORTTEST_H

#include "OutPortTestFixture.h"

template <class Port>
class OutPortTest : public OutPortTestFixture<Port>
{
    typedef OutPortTestFixture<Port> TestBase;

    CPPUNIT_TEST_SUITE(OutPortTest);
    CPPUNIT_TEST(testBasicAPI);
    CPPUNIT_TEST(testConnections);
    CPPUNIT_TEST(testStatistics);
    CPPUNIT_TEST_SUITE_END();

public:
    void testBasicAPI();
    void testConnections();
    void testStatistics();

protected:
    typedef typename TestBase::StubType StubType;

    using TestBase::port;
};

#endif // BULKIO_OUTPORTTEST_H
