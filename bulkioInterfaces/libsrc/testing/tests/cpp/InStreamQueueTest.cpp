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
    double not_rtos_timing_error = 50e-6;
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
    if (not(right_now-time_start+not_rtos_timing_error >= offset-window)) {
        std::cout<<"fail (1b) "<<right_now-time_start<<" "<<offset<<" "<<window<<std::endl;
        return false;
    }
    // check that the time different between the test start and when the packet is less than the offset
    if (not(right_now-time_start < offset)) {
        std::cout<<"fail (2b) "<<right_now-time_start<<" "<<right_now<<" "<<time_start<<" "<<offset<<std::endl;
        return false;
    }
    // check the that time between receiving the packet and the packet's nominal time is within the error window
    if (not(packet_time-right_now < window+not_rtos_timing_error)) {
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
    bulkio::StreamQueue<Port> queue;
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
void InStreamQueueTest<Port>::testRetune()
{
    bulkio::StreamQueue<Port> queue;
    queue.update_port(port);
    std::vector<bulkio::StreamStatus> error_status;

    // push a packet with a future timestamp
    BULKIO::StreamSRI sri = bulkio::sri::create("stream_1");
    double f1 = 1e6;
    double xdelta = 1e-6;
    sri.xdelta = xdelta;
    redhawk::PropertyMap& new_keywords = redhawk::PropertyMap::cast(sri.keywords);
    new_keywords["CHAN_RF"] = f1;
    port->pushSRI(sri);
    BULKIO::PrecisionUTCTime ts_now = bulkio::time::utils::now();
    size_t length = 16;
    double time_offset = 0.065;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);
    usleep(xdelta*1e6*length*100);  // prevent timestamp overlap error

    // push a packet with a future timestamp beyond the timeout
    BULKIO::PrecisionUTCTime second_ts_now = bulkio::time::utils::now();
    time_offset = 0.065;
    double f2 = 2e6;
    new_keywords["CHAN_RF"] = f2;
    port->pushSRI(sri);
    this->_pushTestPacket(length, second_ts_now+time_offset, false, sri.streamID);

    double timeout = 0.1;
    double time_window = 200e-6;

    // inaccuracies in the OS require a wider window
    double measure_time_window = time_window*2;
    DataBlockType block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL(length, block.size());
    CPPUNIT_ASSERT_EQUAL(f1, queue.getCenterFrequency(block));
    CPPUNIT_ASSERT(error_status.size()==0);

    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL(length, block.size());
    CPPUNIT_ASSERT_EQUAL(f2, queue.getCenterFrequency(block));
    CPPUNIT_ASSERT(error_status.size()==0);
}

template <class Port>
void InStreamQueueTest<Port>::testShortWindowQueue()
{
    bulkio::StreamQueue<Port> queue;
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
void InStreamQueueTest<Port>::testZeroSend()
{
    bulkio::StreamQueue<Port> queue;
    queue.update_port(port);
    std::vector<bulkio::StreamStatus> error_status;

    // push a packet with a zero timestamp
    BULKIO::StreamSRI sri = bulkio::sri::create("stream_1");
    port->pushSRI(sri);
    BULKIO::PrecisionUTCTime ts_now = bulkio::time::utils::now();
    ts_now.twsec = 0;
    ts_now.tfsec = 0;
    size_t length = 16;
    this->_pushTestPacket(length, ts_now, false, sri.streamID);

    double timeout = 0.1;
    double time_window = 200e-6;

    DataBlockType block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL(length, block.size());
    CPPUNIT_ASSERT(error_status.size()==0);

    timeout = 0.05;
    ts_now = bulkio::time::utils::now();
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);
    BULKIO::PrecisionUTCTime time_check = bulkio::time::utils::now();

    // make sure the queue times out without returning a block
    CPPUNIT_ASSERT(time_check-ts_now >= timeout);
    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==0);

    ts_now.twsec = 0;
    ts_now.tfsec = 0;
    // push a packet with a zero timestamp
    this->_pushTestPacket(length, ts_now, false, sri.streamID);

    ts_now = bulkio::time::utils::now();
    timeout = 0.05;
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);
    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL(length, block.size());
    CPPUNIT_ASSERT(error_status.size()==0);
}

template <class Port>
void InStreamQueueTest<Port>::testQueueExpired()
{
    bulkio::StreamQueue<Port> queue;
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

    // push a packet with good timestamp beyond the timeout without reset
    ts_now = bulkio::time::utils::now();
    time_offset = 0.065;
    BULKIO::PrecisionUTCTime fourth_time = ts_now+time_offset;
    this->_pushTestPacket(length, fourth_time, false, sri.streamID);

    timeout = 0.1;
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==1);
    CPPUNIT_ASSERT(error_status[0].code==CF::DEV_MISSED_TRANSMIT_WINDOW);

    std::ostringstream fourth_time_str;
    fourth_time_str<<fourth_time;
    CPPUNIT_ASSERT(error_status[0].message.find(fourth_time_str.str()) == std::string::npos);
    CPPUNIT_ASSERT(error_status[0].message.find(second_time_str.str()) != std::string::npos);

    queue.reset();

    // push a packet with good timestamp beyond the timeout with reset
    ts_now = bulkio::time::utils::now();
    time_offset = 0.065;
    BULKIO::PrecisionUTCTime fifth_time = ts_now+time_offset;
    this->_pushTestPacket(length, fifth_time, false, sri.streamID);

    timeout = 0.1;
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT(error_status.size()==0);
}

template <class Port>
void InStreamQueueTest<Port>::testMultiStreamError()
{
    bulkio::StreamQueue<Port> queue;
    queue.update_port(port);
    std::vector<bulkio::StreamStatus> error_status;

    // push a packet with a future timestamp
    std::string stream_id_1 = "stream_1";
    BULKIO::StreamSRI sri_1 = bulkio::sri::create(stream_id_1);
    port->pushSRI(sri_1);
    BULKIO::PrecisionUTCTime ts_now = bulkio::time::utils::now();
    size_t length = 16;
    double time_offset = 0.065;
    BULKIO::PrecisionUTCTime first_time = ts_now+time_offset;
    this->_pushTestPacket(length, first_time, false, sri_1.streamID);

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

    // send another bad stream
    std::string stream_id_2 = "stream_2";
    BULKIO::StreamSRI sri_2 = bulkio::sri::create(stream_id_2);
    sri_2.xdelta = 0.001;
    port->pushSRI(sri_2);
    ts_now = bulkio::time::utils::now();
    size_t old_length = 1600;
    time_offset = 0.065;

    BULKIO::PrecisionUTCTime second_time = ts_now+time_offset;
    BULKIO::PrecisionUTCTime third_time = ts_now+time_offset+(old_length*sri_2.xdelta)/2.0;

    this->_pushTestPacket(old_length, second_time, false, sri_2.streamID);
    size_t new_length = 1000;
    this->_pushTestPacket(new_length, third_time, false, sri_2.streamID);

    std::ostringstream second_time_str;
    std::ostringstream third_time_str;
    second_time_str<<second_time;
    third_time_str<<third_time;

    timeout = 0.1;
    time_window = -1;

    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==2);
    CPPUNIT_ASSERT(error_status[0].stream_id == stream_id_1);
    CPPUNIT_ASSERT(error_status[0].message.find(first_time_str.str()) != std::string::npos);
    CPPUNIT_ASSERT(error_status[0].code==CF::DEV_MISSED_TRANSMIT_WINDOW);

    CPPUNIT_ASSERT(error_status[1].stream_id == stream_id_2);
    CPPUNIT_ASSERT(error_status[1].code==CF::DEV_INVALID_TRANSMIT_TIME_OVERLAP);
    size_t second_location = error_status[1].message.find(second_time_str.str());
    size_t third_location = error_status[1].message.find(third_time_str.str());
    CPPUNIT_ASSERT(second_location<third_location);

    std::string invalid_stream_id = "invalid stream id";
    queue.reset(invalid_stream_id);

    // send a good stream
    std::string stream_id_3 = "stream_3";
    BULKIO::StreamSRI sri_3 = bulkio::sri::create(stream_id_3);
    port->pushSRI(sri_3);
    ts_now = bulkio::time::utils::now();
    length = 16;
    time_offset = 0.065;
    BULKIO::PrecisionUTCTime fourth_time = ts_now+time_offset;
    this->_pushTestPacket(length, fourth_time, false, sri_3.streamID);

    timeout = 0.1;
    time_window = 200e-6;

    // inaccuracies in the OS require a wider window
    double measure_time_window = time_window*2;
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL(length, block.size());
    BULKIO::PrecisionUTCTime right_now = bulkio::time::utils::now();
    CPPUNIT_ASSERT(checkTimeWindow(block.getStartTime(), ts_now, right_now, time_offset, measure_time_window));

    std::ostringstream fourth_time_str;
    fourth_time_str<<fourth_time;

    CPPUNIT_ASSERT(error_status.size()==2);
    CPPUNIT_ASSERT(error_status[0].stream_id == stream_id_1);
    CPPUNIT_ASSERT(error_status[0].message.find(first_time_str.str()) != std::string::npos);
    CPPUNIT_ASSERT(error_status[0].message.find(second_time_str.str()) == std::string::npos);
    CPPUNIT_ASSERT(error_status[0].message.find(third_time_str.str()) == std::string::npos);
    CPPUNIT_ASSERT(error_status[0].message.find(fourth_time_str.str()) == std::string::npos);
    CPPUNIT_ASSERT(error_status[0].code==CF::DEV_MISSED_TRANSMIT_WINDOW);

    CPPUNIT_ASSERT(error_status[1].stream_id == stream_id_2);
    CPPUNIT_ASSERT(error_status[1].code==CF::DEV_INVALID_TRANSMIT_TIME_OVERLAP);
    second_location = error_status[1].message.find(second_time_str.str());
    third_location = error_status[1].message.find(third_time_str.str());
    CPPUNIT_ASSERT(second_location<third_location);
    CPPUNIT_ASSERT(error_status[1].message.find(first_time_str.str()) == std::string::npos);
    CPPUNIT_ASSERT(error_status[1].message.find(fourth_time_str.str()) == std::string::npos);

    queue.reset(stream_id_1);

    // push a packet with good timestamp beyond the timeout with reset on another stream id
    ts_now = bulkio::time::utils::now();
    time_offset = 0.065;
    BULKIO::PrecisionUTCTime fifth_time = ts_now+time_offset;
    this->_pushTestPacket(length, fifth_time, false, sri_1.streamID);

    timeout = 0.1;
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(block);

    std::ostringstream fifth_time_str;
    fifth_time_str<<fifth_time;

    CPPUNIT_ASSERT(error_status.size()==1);
    CPPUNIT_ASSERT(error_status[0].stream_id == stream_id_2);
    CPPUNIT_ASSERT(error_status[0].code==CF::DEV_INVALID_TRANSMIT_TIME_OVERLAP);
    second_location = error_status[0].message.find(second_time_str.str());
    third_location = error_status[0].message.find(third_time_str.str());
    CPPUNIT_ASSERT(second_location<third_location);
    CPPUNIT_ASSERT(error_status[0].message.find(first_time_str.str()) == std::string::npos);
    CPPUNIT_ASSERT(error_status[0].message.find(fourth_time_str.str()) == std::string::npos);
    CPPUNIT_ASSERT(error_status[0].message.find(fifth_time_str.str()) == std::string::npos);

    queue.reset();

    // push a packet with good timestamp beyond the timeout with reset on all stream ids
    ts_now = bulkio::time::utils::now();
    time_offset = 0.065;
    BULKIO::PrecisionUTCTime sixth_time = ts_now+time_offset;
    this->_pushTestPacket(length, sixth_time, false, sri_1.streamID);

    timeout = 0.1;
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT(error_status.size()==0);
}

template <class Port>
void InStreamQueueTest<Port>::testImmediateQueue()
{
    bulkio::StreamQueue<Port> queue;
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
void InStreamQueueTest<Port>::testQuickTimeout()
{
    bulkio::StreamQueue<Port> queue;
    queue.update_port(port);
    std::vector<bulkio::StreamStatus> error_status;

    std::string stream_id = "stream_1";
    // push a packet with a future timestamp
    BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
    port->pushSRI(sri);
    BULKIO::PrecisionUTCTime ts_now = bulkio::time::utils::now();
    size_t length = 16;
    double time_offset = 0.065;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    double timeout = 0.1;
    double time_window = -1;

    queue.hold(stream_id);

    // inaccuracies in the OS require a wider window
    double measure_time_window = time_window*2;
    DataBlockType block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==0);

    // push a packet with a future timestamp beyond the timeout
    ts_now = bulkio::time::utils::now();
    time_offset = 0.065;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==0);

    CPPUNIT_ASSERT(queue.held().size()==1);
    CPPUNIT_ASSERT(queue.held()[0]==stream_id);

    queue.allow(stream_id);

    ts_now = bulkio::time::utils::now();
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);
    CPPUNIT_ASSERT(error_status.size()==1);
    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status[0].code==CF::DEV_MISSED_TRANSMIT_WINDOW);
}

template <class Port>
void InStreamQueueTest<Port>::testQuickIgnoreError()
{
    bulkio::StreamQueue<Port> queue;
    queue.update_port(port);
    std::vector<bulkio::StreamStatus> error_status;

    std::string stream_id = "stream_1";
    // push a packet with a future timestamp
    BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
    port->pushSRI(sri);
    BULKIO::PrecisionUTCTime ts_now = bulkio::time::utils::now();
    size_t length = 16;
    double time_offset = 0.065;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    double timeout = 0.1;
    double time_window = -1;

    queue.hold(stream_id);

    // inaccuracies in the OS require a wider window
    double measure_time_window = time_window*2;
    DataBlockType block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==0);

    // push a packet with a future timestamp beyond the timeout
    ts_now = bulkio::time::utils::now();
    time_offset = 0.065;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==0);

    CPPUNIT_ASSERT(queue.held().size()==1);
    CPPUNIT_ASSERT(queue.held()[0]==stream_id);

    queue.update_ignore_error(true);
    queue.allow(stream_id);

    ts_now = bulkio::time::utils::now();
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);
    CPPUNIT_ASSERT(error_status.size()==1);
    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status[0].code==CF::DEV_INVALID_TRANSMIT_TIME_OVERLAP);
}

template <class Port>
void InStreamQueueTest<Port>::testQuickIgnoreErrorAndTimestamp()
{
    bulkio::StreamQueue<Port> queue;
    queue.update_port(port);
    std::vector<bulkio::StreamStatus> error_status;

    std::string stream_id = "stream_1";
    // push a packet with a future timestamp
    BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
    port->pushSRI(sri);
    BULKIO::PrecisionUTCTime ts_now = bulkio::time::utils::now();
    size_t length = 16;
    double time_offset = 0.065;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    double timeout = 0.1;
    double time_window = -1;

    queue.hold(stream_id);

    // inaccuracies in the OS require a wider window
    double measure_time_window = time_window*2;
    DataBlockType block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==0);

    // push a packet with a future timestamp beyond the timeout
    ts_now = bulkio::time::utils::now();
    time_offset = 0.065;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==0);

    CPPUNIT_ASSERT(queue.held().size()==1);
    CPPUNIT_ASSERT(queue.held()[0]==stream_id);

    queue.update_ignore_error(true);
    queue.update_ignore_timestamp(true);
    queue.allow(stream_id);

    ts_now = bulkio::time::utils::now();
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);
    CPPUNIT_ASSERT(error_status.size()==0);
    CPPUNIT_ASSERT(block);
    ts_now = bulkio::time::utils::now();
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);
    CPPUNIT_ASSERT(error_status.size()==0);
    CPPUNIT_ASSERT(block);
    ts_now = bulkio::time::utils::now();
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);
    CPPUNIT_ASSERT(error_status.size()==0);
    CPPUNIT_ASSERT(!block);
}

template <class Port>
void InStreamQueueTest<Port>::testQuick()
{
    bulkio::StreamQueue<Port> queue;
    queue.update_port(port);
    std::vector<bulkio::StreamStatus> error_status;

    std::string stream_id = "stream_1";
    // push a packet with a future timestamp
    BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
    port->pushSRI(sri);
    BULKIO::PrecisionUTCTime ts_now = bulkio::time::utils::now();
    size_t length = 16;
    double time_offset = 0.065;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    double timeout = 0.1;
    double time_window = -1;

    queue.hold(stream_id);

    // inaccuracies in the OS require a wider window
    double measure_time_window = time_window*2;
    DataBlockType block = queue.getNextBlock(ts_now, error_status, timeout, time_window);

    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==0);

    // push a packet with a future timestamp beyond the timeout
    ts_now = bulkio::time::utils::now();
    time_offset = 0.065;
    this->_pushTestPacket(length, ts_now+time_offset, false, sri.streamID);

    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status.size()==0);

    CPPUNIT_ASSERT(queue.held().size()==1);
    CPPUNIT_ASSERT(queue.held()[0]==stream_id);

    queue.allow(stream_id);

    ts_now = bulkio::time::utils::now();
    block = queue.getNextBlock(ts_now, error_status, timeout, time_window);
    CPPUNIT_ASSERT(error_status.size()==1);
    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(error_status[0].code==CF::DEV_MISSED_TRANSMIT_WINDOW);
}

template <class Port>
void InStreamQueueTest<Port>::testTimeOverlap()
{
    bulkio::StreamQueue<Port> queue;
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
