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
#ifndef BULKIO_INPORTTEST_H
#define BULKIO_INPORTTEST_H

#include "InPortTestFixture.h"

template <class Port>
class InPortTest : public InPortTestFixture<Port>
{
    typedef InPortTestFixture<Port> TestBase;

    CPPUNIT_TEST_SUITE(InPortTest);
    CPPUNIT_TEST(testLegacyAPI);
    CPPUNIT_TEST(testGetPacket);
    CPPUNIT_TEST(testGetPacketStreamRemoved);
    CPPUNIT_TEST(testActiveSRIs);
    CPPUNIT_TEST(testStreamIds);
    CPPUNIT_TEST(testQueueDepth);
    CPPUNIT_TEST(testState);
    CPPUNIT_TEST(testSriChanged);
    CPPUNIT_TEST(testSriChangedFlush);
    CPPUNIT_TEST(testSriChangedInvalidStream);
    CPPUNIT_TEST(testStatistics);
    CPPUNIT_TEST(testDiscardEmptyPacket);
    CPPUNIT_TEST(testQueueFlushFlags);
    CPPUNIT_TEST(testQueueSize);
    CPPUNIT_TEST_SUITE_END();

public:
    void testLegacyAPI();
    void testGetPacket();
    void testGetPacketStreamRemoved();
    void testActiveSRIs();
    void testStreamIds();
    void testQueueDepth();
    void testState();
    void testSriChanged();
    void testSriChangedFlush();
    void testSriChangedInvalidStream();
    void testStatistics();
    void testDiscardEmptyPacket();
    void testQueueFlushFlags();
    void testQueueSize();

protected:
    typedef typename Port::dataTransfer PacketType;
    typedef typename Port::CorbaType CorbaType;

    static const size_t BITS_PER_ELEMENT;

    using TestBase::port;
};

#endif // BULKIO_INPORTTEST_H
