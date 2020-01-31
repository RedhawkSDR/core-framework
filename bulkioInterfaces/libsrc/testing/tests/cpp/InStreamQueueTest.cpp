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
    if (window < 0) { // do not queue
        double short_time = 1e-3;
        // check that the time different between the test start and when the packet is received is very short
        if (not(right_now-time_start < short_time)) {
            std::cout<<"fail (1a) "<<right_now-time_start<<" "<<short_time<<" "<<offset<<" "<<window<<std::endl;
            return false;
        }
        // check the that time between receiving the packet and the packet's nominal time is within the error window
        if (not(packet_time-right_now >= offset-short_time)) {
            std::cout<<"fail (3a) "<<packet_time-right_now<<" "<<offset<<" "<<short_time<<std::endl;
            return false;
        }
        // check the that time for receiving the packet is less than the packet's nominal time
        if (not(right_now < packet_time)) {
            std::cout<<"fail (4a) "<<right_now<<" "<<packet_time<<std::endl;
            return false;
        }
        return true;
    }
    // check that the time different between the test start and when the packet is received is within the error limit of the offset
    if (not(right_now-time_start >= offset-window)) {
        std::cout<<"fail (1b) "<<right_now-time_start<<" "<<offset<<" "<<window<<std::endl;
        return false;
    }
    // check that the time different between the test start and when the packet is less than the offset
    if (not(right_now-time_start < offset)) {
        std::cout<<"fail (2b) "<<right_now-time_start<<" "<<right_now<<" "<<time_start<<" "<<offset<<std::endl;
        return false;
    }
    // check the that time between receiving the packet and the packet's nominal time is within the error window
    if (not(packet_time-right_now < window)) {
        std::cout<<"fail (3b) "<<packet_time-right_now<<" "<<window<<std::endl;
        return false;
    }
    // check the that time for receiving the packet is less than the packet's nominal time
    if (not(right_now < packet_time)) {
        std::cout<<"fail (4b) "<<right_now<<" "<<packet_time<<std::endl;
        return false;
    }
    return true;
};

template <class Port>
void InStreamQueueTest<Port>::testBaselineQueueTest()
{
    bulkio::streamQueue<Port> queue;
    queue.update_port(port);
    std::vector<bulkio::StreamStatus> error_status;

    // push a packet with a future timestamp
    BULKIO::StreamSRI sri = bulkio::sri::create("stream_1");
    port->pushSRI(sri);
    BULKIO::PrecisionUTCTime ts_now = bulkio::time::utils::now();
    size_t length = 16;
    double time_offset = 0.13;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    double timeout = 0.2;
    double time_window = 4000e-6;
    DataBlockType block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL(length, block.size());
    BULKIO::PrecisionUTCTime right_now = bulkio::time::utils::now();
    CPPUNIT_ASSERT(checkTimeWindow(block.getStartTime(), ts_now, right_now, time_offset, time_window));
    CPPUNIT_ASSERT(error_status.size()==0);

    // push a packet with a future timestamp beyond the timeout
    ts_now = bulkio::time::utils::now();
    time_offset = 0.13;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    timeout = 0.1;
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);
    BULKIO::PrecisionUTCTime time_check = bulkio::time::utils::now();

    // make sure the queue times out without returning a block
    CPPUNIT_ASSERT(time_check-ts_now > timeout);
    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==0);

    block = queue.getNextBlock(time_check, error_status, timeout, time_window);
    time_check = bulkio::time::utils::now();

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL(length, block.size());
    right_now = bulkio::time::utils::now();
    CPPUNIT_ASSERT(checkTimeWindow(block.getStartTime(), ts_now, right_now, time_offset, time_window));
    CPPUNIT_ASSERT(error_status.size()==0);
}

template <class Port>
void InStreamQueueTest<Port>::testShortWindowQueue()
{
    bulkio::streamQueue<Port> queue;
    queue.update_port(port);
    std::vector<bulkio::StreamStatus> error_status;

    // push a packet with a future timestamp
    BULKIO::StreamSRI sri = bulkio::sri::create("stream_1");
    port->pushSRI(sri);
    BULKIO::PrecisionUTCTime ts_now = bulkio::time::utils::now();
    size_t length = 16;
    double time_offset = 0.065;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    double timeout = 0.1;
    double time_window = 200e-6;

    // inaccuracies in the OS require a wider window
    double measure_time_window = time_window*2;
    DataBlockType block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL(length, block.size());
    BULKIO::PrecisionUTCTime right_now = bulkio::time::utils::now();
    CPPUNIT_ASSERT(checkTimeWindow(block.getStartTime(), ts_now, right_now, time_offset, measure_time_window));
    CPPUNIT_ASSERT(error_status.size()==0);

    // push a packet with a future timestamp beyond the timeout
    ts_now = bulkio::time::utils::now();
    time_offset = 0.065;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    timeout = 0.05;
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);
    BULKIO::PrecisionUTCTime time_check = bulkio::time::utils::now();

    // make sure the queue times out without returning a block
    CPPUNIT_ASSERT(time_check-ts_now > timeout);
    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==0);

    block = queue.getNextBlock(time_check, error_status, timeout, time_window);
    time_check = bulkio::time::utils::now();

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL(length, block.size());
    right_now = bulkio::time::utils::now();
    CPPUNIT_ASSERT(checkTimeWindow(block.getStartTime(), ts_now, right_now, time_offset, measure_time_window));
    CPPUNIT_ASSERT(error_status.size()==0);
}

template <class Port>
void InStreamQueueTest<Port>::testQueueExpired()
{
    bulkio::streamQueue<Port> queue;
    queue.update_port(port);
    std::vector<bulkio::StreamStatus> error_status;

    // push a packet with a future timestamp
    BULKIO::StreamSRI sri = bulkio::sri::create("stream_1");
    port->pushSRI(sri);
    BULKIO::PrecisionUTCTime ts_now = bulkio::time::utils::now();
    size_t length = 16;
    double time_offset = 0.065;
    BULKIO::PrecisionUTCTime first_time = ts_now+time_offset;
    this->_pushTestPacket(length, first_time, false, sri.streamID);

    double timeout = 0.1;
    double time_window = -1;

    BULKIO::PrecisionUTCTime offset_time = ts_now+time_offset*2;
    DataBlockType block = queue.getNextBlock(offset_time, error_status, timeout, time_window);

    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==1);
    CPPUNIT_ASSERT(error_status[0].code==CF::DEV_MISSED_TRANSMIT_WINDOW);

    std::ostringstream first_time_str;
    first_time_str<<first_time;
    CPPUNIT_ASSERT(error_status[0].message.find(first_time_str.str()) != std::string::npos);

    queue.reset();

    // push a packet with a future timestamp beyond the timeout
    ts_now = bulkio::time::utils::now();
    time_offset = 0.065;
    BULKIO::PrecisionUTCTime second_time = ts_now+time_offset;
    this->_pushTestPacket(length, second_time, false, sri.streamID);

    timeout = 0.05;
    offset_time = ts_now+time_offset*2;
    block = queue.getNextBlock(offset_time, error_status, timeout, time_window);

    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==1);
    CPPUNIT_ASSERT(error_status[0].code==CF::DEV_MISSED_TRANSMIT_WINDOW);

    std::ostringstream second_time_str;
    second_time_str<<second_time;
    CPPUNIT_ASSERT(error_status[0].message.find(second_time_str.str()) != std::string::npos);

    // push a packet with a future timestamp beyond the timeout without reset
    ts_now = bulkio::time::utils::now();
    time_offset = 0.065;
    BULKIO::PrecisionUTCTime third_time = ts_now+time_offset;
    this->_pushTestPacket(length, third_time, false, sri.streamID);

    timeout = 0.05;
    offset_time = ts_now+time_offset*2;
    block = queue.getNextBlock(offset_time, error_status, timeout, time_window);

    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==1);
    CPPUNIT_ASSERT(error_status[0].code==CF::DEV_MISSED_TRANSMIT_WINDOW);

    std::ostringstream third_time_str;
    third_time_str<<third_time;
    CPPUNIT_ASSERT(error_status[0].message.find(third_time_str.str()) == std::string::npos);
    CPPUNIT_ASSERT(error_status[0].message.find(second_time_str.str()) != std::string::npos);
}

template <class Port>
void InStreamQueueTest<Port>::testImmediateQueue()
{
    bulkio::streamQueue<Port> queue;
    queue.update_port(port);
    std::vector<bulkio::StreamStatus> error_status;

    // push a packet with a future timestamp
    BULKIO::StreamSRI sri = bulkio::sri::create("stream_1");
    port->pushSRI(sri);
    BULKIO::PrecisionUTCTime ts_now = bulkio::time::utils::now();
    size_t length = 16;
    double time_offset = 0.065;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    double timeout = 0.1;
    double time_window = -1;

    // inaccuracies in the OS require a wider window
    double measure_time_window = time_window*2;
    DataBlockType block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL(length, block.size());
    BULKIO::PrecisionUTCTime right_now = bulkio::time::utils::now();
    CPPUNIT_ASSERT(checkTimeWindow(block.getStartTime(), ts_now, right_now, time_offset, measure_time_window));
    CPPUNIT_ASSERT(error_status.size()==0);

    // push a packet with a future timestamp beyond the timeout
    ts_now = bulkio::time::utils::now();
    time_offset = 0.065;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    timeout = 0.05;
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);
    BULKIO::PrecisionUTCTime time_check = bulkio::time::utils::now();

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL(length, block.size());
    right_now = bulkio::time::utils::now();
    CPPUNIT_ASSERT(checkTimeWindow(block.getStartTime(), ts_now, right_now, time_offset, measure_time_window));
    CPPUNIT_ASSERT(error_status.size()==0);

    BULKIO::PrecisionUTCTime pre_timeout = bulkio::time::utils::now();
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);
    BULKIO::PrecisionUTCTime post_timeout = bulkio::time::utils::now();
    CPPUNIT_ASSERT(post_timeout-pre_timeout > timeout * 0.9);
    CPPUNIT_ASSERT(post_timeout-pre_timeout < timeout * 1.1);
    CPPUNIT_ASSERT(error_status.size()==0);
}

template <class Port>
void InStreamQueueTest<Port>::testTimeOverlap()
{
    bulkio::streamQueue<Port> queue;
    queue.update_port(port);
    std::vector<bulkio::StreamStatus> error_status;

    // push a packet with a future timestamp
    BULKIO::StreamSRI sri = bulkio::sri::create("stream_1");
    sri.xdelta = 0.001;
    port->pushSRI(sri);
    BULKIO::PrecisionUTCTime ts_now = bulkio::time::utils::now();
    size_t old_length = 1600;
    double time_offset = 0.065;

    BULKIO::PrecisionUTCTime first_time = ts_now+time_offset;
    BULKIO::PrecisionUTCTime second_time = ts_now+time_offset+(old_length*sri.xdelta)/2.0;

    this->_pushTestPacket(old_length, first_time, false, sri.streamID);
    size_t length = 1000;
    this->_pushTestPacket(length, second_time, false, sri.streamID);

    std::ostringstream first_time_str;
    std::ostringstream second_time_str;
    first_time_str<<first_time;
    second_time_str<<second_time;

    double timeout = 0.1;
    double time_window = -1;

    DataBlockType block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==1);
    CPPUNIT_ASSERT(error_status[0].code==CF::DEV_INVALID_TRANSMIT_TIME_OVERLAP);
    size_t first_location = error_status[0].message.find(first_time_str.str());
    size_t second_location = error_status[0].message.find(second_time_str.str());
    CPPUNIT_ASSERT(first_location<second_location);
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
