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
#ifndef BULKIO_INSTREAMQUEUETEST_H
#define BULKIO_INSTREAMQUEUETEST_H

#include "InPortTestFixture.h"

template <class Port>
class InStreamQueueTest : public InPortTestFixture<Port>
{
    typedef InPortTestFixture<Port> TestBase;

    CPPUNIT_TEST_SUITE(InStreamQueueTest);

    CPPUNIT_TEST(testBaselineQueueTest);
    CPPUNIT_TEST(testShortWindowQueue);
    CPPUNIT_TEST(testImmediateQueue);
    CPPUNIT_TEST(testQueueExpired);
    CPPUNIT_TEST(testTimeOverlap);
    CPPUNIT_TEST(testMultiStreamError);
    CPPUNIT_TEST(testRetune);
    CPPUNIT_TEST(testQuick);
    CPPUNIT_TEST(testQuickTimeout);
    CPPUNIT_TEST(testQuickIgnoreError);
    CPPUNIT_TEST(testQuickIgnoreErrorAndTimestamp);

    CPPUNIT_TEST_SUITE_END();

public:
    void testBaselineQueueTest();
    void testShortWindowQueue();
    void testImmediateQueue();
    void testQueueExpired();
    void testTimeOverlap();
    void testMultiStreamError();
    void testRetune();
    void testQuick();
    void testQuickTimeout();
    void testQuickIgnoreError();
    void testQuickIgnoreErrorAndTimestamp();

    bool checkTimeWindow(const BULKIO::PrecisionUTCTime &packet_time, const BULKIO::PrecisionUTCTime &time_start, const BULKIO::PrecisionUTCTime &right_now, double offsets, double window);

protected:
    typedef typename Port::StreamType StreamType;
    typedef typename StreamType::DataBlockType DataBlockType;

    using TestBase::port;
};

#endif  // BULKIO_INSTREAMTEST_H
