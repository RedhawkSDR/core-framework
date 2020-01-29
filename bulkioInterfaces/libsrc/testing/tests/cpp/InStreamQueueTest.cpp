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

#include "InStreamQueueTest.h"
#include "bulkio.h"

template <class Port>
bool InStreamQueueTest<Port>::checkTimeWindow(const BULKIO::PrecisionUTCTime &packet_time, const BULKIO::PrecisionUTCTime &time_start, const BULKIO::PrecisionUTCTime &right_now, double offset, double window) {
    // check that the time different between the test start and when the packet is received is within the error limit of the offset
    if (not(right_now-time_start > offset-window)) {
        std::cout<<"fail (1) "<<right_now-time_start<<" "<<offset<<" "<<window<<std::endl;
        return false;
    }
    // check that the time different between the test start and when the packet is less than the offset
    if (not(right_now-time_start < offset)) {
        std::cout<<"fail (2) "<<right_now-time_start<<" "<<right_now<<" "<<time_start<<" "<<offset<<std::endl;
        return false;
    }
    // check the that time between receiving the packet and the packet's nominal time is within the error window
    if (not(packet_time-right_now < window)) {
        std::cout<<"fail (3) "<<packet_time-right_now<<" "<<window<<std::endl;
        return false;
    }
    // check the that time for receiving the packet is less than the packet's nominal time
    if (not(right_now < packet_time)) {
        std::cout<<"fail (4) "<<right_now<<" "<<packet_time<<std::endl;
        return false;
    }
    return true;
};

template <class Port>
void InStreamQueueTest<Port>::testBaselineQueueTest()
{
    bulkio::streamQueue<Port> queue;
    queue.update_port(port);

    // push a packet with a future timestamp
    BULKIO::StreamSRI sri = bulkio::sri::create("stream_1");
    port->pushSRI(sri);
    BULKIO::PrecisionUTCTime ts_now = bulkio::time::utils::now();
    size_t length = 16;
    double time_offset = 0.65;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    double timeout = 1;
    double time_window = 20000e-6;
    DataBlockType block = queue.getNextBlock(ts_now, timeout, time_window);

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL(length, block.size());
    BULKIO::PrecisionUTCTime right_now = bulkio::time::utils::now();
    CPPUNIT_ASSERT(checkTimeWindow(block.getStartTime(), ts_now, right_now, time_offset, time_window));

    // push a packet with a future timestamp beyond the timeout
    ts_now = bulkio::time::utils::now();
    time_offset = 0.65;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    timeout = 0.5;
    block = queue.getNextBlock(ts_now, timeout, time_window);
    BULKIO::PrecisionUTCTime time_check = bulkio::time::utils::now();

    // make sure the queue times out without returning a block
    CPPUNIT_ASSERT(time_check-ts_now > timeout);
    CPPUNIT_ASSERT(!block);

    block = queue.getNextBlock(time_check, timeout, time_window);
    time_check = bulkio::time::utils::now();

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL(length, block.size());
    right_now = bulkio::time::utils::now();
    CPPUNIT_ASSERT(checkTimeWindow(block.getStartTime(), ts_now, right_now, time_offset, time_window));
}

template <class Port>
void InStreamQueueTest<Port>::testShortWindowQueue()
{
    bulkio::streamQueue<Port> queue;
    queue.update_port(port);

    // push a packet with a future timestamp
    BULKIO::StreamSRI sri = bulkio::sri::create("stream_1");
    port->pushSRI(sri);
    BULKIO::PrecisionUTCTime ts_now = bulkio::time::utils::now();
    size_t length = 16;
    double time_offset = 0.65;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    double timeout = 1;
    double time_window = 200e-6;

    // inaccuracies in the OS require a wider window
    double measure_time_window = time_window*2;
    DataBlockType block = queue.getNextBlock(ts_now, timeout, time_window);

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL(length, block.size());
    BULKIO::PrecisionUTCTime right_now = bulkio::time::utils::now();
    CPPUNIT_ASSERT(checkTimeWindow(block.getStartTime(), ts_now, right_now, time_offset, measure_time_window));

    // push a packet with a future timestamp beyond the timeout
    ts_now = bulkio::time::utils::now();
    time_offset = 0.65;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    timeout = 0.5;
    block = queue.getNextBlock(ts_now, timeout, time_window);
    BULKIO::PrecisionUTCTime time_check = bulkio::time::utils::now();

    // make sure the queue times out without returning a block
    CPPUNIT_ASSERT(time_check-ts_now > timeout);
    CPPUNIT_ASSERT(!block);

    block = queue.getNextBlock(time_check, timeout, time_window);
    time_check = bulkio::time::utils::now();

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL(length, block.size());
    right_now = bulkio::time::utils::now();
    CPPUNIT_ASSERT(checkTimeWindow(block.getStartTime(), ts_now, right_now, time_offset, measure_time_window));
}

#define CREATE_QUEUE_TEST(x, BASE)                                            \
    class In##x##StreamQueueTest : public BASE<bulkio::In##x##Port>          \
    {                                                                   \
        CPPUNIT_TEST_SUB_SUITE(In##x##StreamQueueTest, BASE<bulkio::In##x##Port>); \
        CPPUNIT_TEST_SUITE_END();                                       \
    };                                                                  \
    CPPUNIT_TEST_SUITE_REGISTRATION(In##x##StreamQueueTest);

#define CREATE_NUMERIC_QUEUE_TEST(x) CREATE_QUEUE_TEST(x, InStreamQueueTest)

CREATE_NUMERIC_QUEUE_TEST(Octet);
CREATE_NUMERIC_QUEUE_TEST(Char);
CREATE_NUMERIC_QUEUE_TEST(Short);
CREATE_NUMERIC_QUEUE_TEST(UShort);
CREATE_NUMERIC_QUEUE_TEST(Long);
CREATE_NUMERIC_QUEUE_TEST(ULong);
CREATE_NUMERIC_QUEUE_TEST(LongLong);
CREATE_NUMERIC_QUEUE_TEST(ULongLong);
CREATE_NUMERIC_QUEUE_TEST(Float);
CREATE_NUMERIC_QUEUE_TEST(Double);
