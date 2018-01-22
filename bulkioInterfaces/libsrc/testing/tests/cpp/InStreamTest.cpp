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

#include "InStreamTest.h"
#include "bulkio.h"

template <class Port>
void InStreamTest<Port>::setUp()
{
    std::string name = "data" + getPortName() + "_in";
    port = new Port(name);
}

template <class Port>
void InStreamTest<Port>::tearDown()
{
    delete port;
}

template <class Port>
void InStreamTest<Port>::testGetCurrentStreamEmptyEos()
{
    // Create a new stream and push some data to it
    BULKIO::StreamSRI sri = bulkio::sri::create("empty_eos");
    port->pushSRI(sri);
    _pushTestPacket(1024, bulkio::time::utils::now(), false, sri.streamID);

    // Get the input stream and read the first packet
    StreamType stream = port->getStream("empty_eos");
    CPPUNIT_ASSERT_EQUAL(!stream, false);
    DataBlockType block = stream.read();
    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());
    CPPUNIT_ASSERT(!stream.eos());

    // Push an end-of-stream packet with no data and get the stream again
    _pushTestPacket(0, bulkio::time::utils::notSet(), true, sri.streamID);
    stream = port->getCurrentStream(bulkio::Const::NON_BLOCKING);
    CPPUNIT_ASSERT(stream);
    block = stream.read();
    CPPUNIT_ASSERT(!block);

    // There should be no current stream, because the failed read should have
    // removed it
    StreamType next_stream = port->getCurrentStream(bulkio::Const::NON_BLOCKING);
    CPPUNIT_ASSERT(!next_stream);

    // The original stream should report end-of-stream
    CPPUNIT_ASSERT(stream.eos());
}

template <class Port>
void InStreamTest<Port>::testGetCurrentStreamDataEos()
{
    // Create a new stream and push some data to it
    BULKIO::StreamSRI sri = bulkio::sri::create("empty_eos");
    port->pushSRI(sri);
    _pushTestPacket(1024, bulkio::time::utils::now(), false, sri.streamID);

    // Get the input stream and read the first packet
    StreamType stream = port->getStream("empty_eos");
    CPPUNIT_ASSERT_EQUAL(!stream, false);
    DataBlockType block = stream.read();
    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());
    CPPUNIT_ASSERT(!stream.eos());

    // Push an end-of-stream packet with data and get the stream again
    _pushTestPacket(1024, bulkio::time::utils::now(), true, sri.streamID);
    stream = port->getCurrentStream(bulkio::Const::NON_BLOCKING);
    CPPUNIT_ASSERT(stream);
    block = stream.read();
    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());

    // Try to get the current stream again; since the end-of-stream has not been
    // checked yet, it should return the existing stream (as with above)
    stream = port->getCurrentStream(bulkio::Const::NON_BLOCKING);
    CPPUNIT_ASSERT(stream);
    block = stream.read();
    CPPUNIT_ASSERT(!block);

    // There should be no current stream, because the failed read should have
    // removed it
    StreamType next_stream = port->getCurrentStream(bulkio::Const::NON_BLOCKING);
    CPPUNIT_ASSERT(!next_stream);

    // The original stream should report end-of-stream
    CPPUNIT_ASSERT(stream.eos());
}

template <class Port>
void InStreamTest<Port>::testSriChanges()
{
    const char* stream_id = "sri_changes";

    // Create a new stream and push some data to it
    BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
    sri.xstart = 0.0;
    sri.xdelta = 1.0;
    port->pushSRI(sri);
    _pushTestPacket(1024, bulkio::time::utils::now(), false, sri.streamID);

    // Get the input stream and read the first packet
    StreamType stream = port->getStream(stream_id);
    CPPUNIT_ASSERT_EQUAL(!stream, false);
    DataBlockType block = stream.read();
    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());
    CPPUNIT_ASSERT(!stream.eos());
    CPPUNIT_ASSERT_EQUAL(sri.xdelta, block.sri().xdelta);

    // Change xdelta (based on sample rate of 2.5Msps)
    sri.xdelta = 1.0 / 2.5e6;
    port->pushSRI(sri);
    _pushTestPacket(1024, bulkio::time::utils::now(), false, sri.streamID);
    block = stream.read();
    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());
    CPPUNIT_ASSERT(!stream.eos());
    CPPUNIT_ASSERT(block.sriChanged());
    int flags = bulkio::sri::XDELTA;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SRI change flags incorrect", flags, block.sriChangeFlags());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SRI xdelta incorrect", sri.xdelta, block.sri().xdelta);

    // Add a keyword, change xdelta back and update xstart
    ossie::corba::push_back(sri.keywords, redhawk::PropertyType("COL_RF", 101.1e6));
    sri.xstart = 100.0;
    sri.xdelta = 1.0;
    port->pushSRI(sri);
    _pushTestPacket(1024, bulkio::time::utils::now(), false, sri.streamID);
    block = stream.read();
    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());
    CPPUNIT_ASSERT(!stream.eos());
    CPPUNIT_ASSERT(block.sriChanged());
    flags = bulkio::sri::XSTART | bulkio::sri::XDELTA | bulkio::sri::KEYWORDS;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SRI change flags incorrect", flags, block.sriChangeFlags());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SRI xstart incorrect", sri.xstart, block.sri().xstart);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("SRI xdelta incorrect", sri.xdelta, block.sri().xdelta);
}

template <class Port>
void InStreamTest<Port>::testDisable()
{
    const char* stream_id = "disable";

    // Create a new stream and push some data to it
    BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
    port->pushSRI(sri);
    _pushTestPacket(16, bulkio::time::utils::now(), false, sri.streamID);

    // Get the input stream and read the first packet
    StreamType stream = port->getStream(stream_id);
    CPPUNIT_ASSERT_EQUAL(false, !stream);

    DataBlockType block = stream.read();
    CPPUNIT_ASSERT_EQUAL(false, !block);

    // Push a couple more packets
    _pushTestPacket(16, bulkio::time::utils::now(), false, sri.streamID);
    _pushTestPacket(16, bulkio::time::utils::now(), false, sri.streamID);
    CPPUNIT_ASSERT_EQUAL(2, port->getCurrentQueueDepth());

    // Disable the stream; this should drop the existing packets
    stream.disable();
    CPPUNIT_ASSERT(!stream.enabled());
    CPPUNIT_ASSERT_EQUAL(0, port->getCurrentQueueDepth());

    // Push a couple more packets; they should get dropped
    _pushTestPacket(16, bulkio::time::utils::now(), false, sri.streamID);
    _pushTestPacket(16, bulkio::time::utils::now(), false, sri.streamID);
    CPPUNIT_ASSERT_EQUAL(0, port->getCurrentQueueDepth());

    // Push an end-of-stream packet
    _pushTestPacket(16, bulkio::time::utils::notSet(), true, sri.streamID);

    // Re-enable the stream and read; it should fail with end-of-stream set
    stream.enable();
    block = stream.read();
    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(stream.eos());
}

template <class Port>
void InStreamTest<Port>::_pushTestPacket(size_t length, const BULKIO::PrecisionUTCTime& time,
                                         bool eos, const char* streamID)
{
    typename Port::PortSequenceType data;
    data.length(length);
    port->pushPacket(data, time, eos, streamID);
}

template <>
void InStreamTest<bulkio::InFilePort>::_pushTestPacket(size_t length, const BULKIO::PrecisionUTCTime& time,
                                                       bool eos, const char* streamID)
{
    std::string data(length, ' ');
    port->pushPacket(data.c_str(), time, eos, streamID);
}

template <>
void InStreamTest<bulkio::InXMLPort>::_pushTestPacket(size_t length, const BULKIO::PrecisionUTCTime&,
                                                      bool eos, const char* streamID)
{
    std::string data(length, ' ');
    port->pushPacket(data.c_str(), eos, streamID);
}

template <>
void InStreamTest<bulkio::InBitPort>::_pushTestPacket(size_t length, const BULKIO::PrecisionUTCTime& time,
                                                      bool eos, const char* streamID)
{
    BULKIO::BitSequence data;
    data.data.length((length+7)/8);
    data.bits = length;
    port->pushPacket(data, time, eos, streamID);
}

template <class Port>
void BufferedInStreamTest<Port>::testSizedReadEmptyEos()
{
    const char* stream_id = "read_empty_eos";

    // Create a new stream and push an end-of-stream packet with no data
    BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
    port->pushSRI(sri);
    this->_pushTestPacket(0, bulkio::time::utils::notSet(), true, stream_id);

    // Try to read a single element; this should return a null block
    StreamType stream = port->getStream(stream_id);
    CPPUNIT_ASSERT(stream);
    DataBlockType block = stream.read(1);
    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(stream.eos());
}

template <class Port>
void BufferedInStreamTest<Port>::testSizedTryreadEmptyEos()
{
    const char* stream_id = "tryread_empty_eos";

    // Create a new stream and push an end-of-stream packet with no data
    BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
    port->pushSRI(sri);
    this->_pushTestPacket(0, bulkio::time::utils::notSet(), true, stream_id);

    // Try to read a single element; this should return a null block
    StreamType stream = port->getStream(stream_id);
    CPPUNIT_ASSERT(stream);
    DataBlockType block = stream.tryread(1);
    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(stream.eos());
}

template <class Port>
void BufferedInStreamTest<Port>::testTryreadPeek()
{
    const char* stream_id = "tryread_peek";

    // Create a new stream and push some data to it
    BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
    port->pushSRI(sri);
    this->_pushTestPacket(1024, bulkio::time::utils::now(), true, stream_id);

    // Get the input stream and read the first packet
    StreamType stream = port->getStream(stream_id);
    CPPUNIT_ASSERT_EQUAL(!stream, false);
    DataBlockType block = stream.tryread(10000,0);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());
    block = stream.read(10000);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());
    block = stream.read(10000);
    CPPUNIT_ASSERT(!block);
}

template <class Port>
void BufferedInStreamTest<Port>::testReadPeek()
{
    const char* stream_id = "read_peek";

    // Create a new stream and push some data to it
    BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
    port->pushSRI(sri);
    this->_pushTestPacket(1024, bulkio::time::utils::now(), true, stream_id);

    // Get the input stream and read the first packet
    StreamType stream = port->getStream(stream_id);
    CPPUNIT_ASSERT_EQUAL(!stream, false);
    DataBlockType block = stream.read(10000,0);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());
    block = stream.read(10000);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());
    block = stream.read(10000);
    CPPUNIT_ASSERT(!block);
}

template <class Port>
void BufferedInStreamTest<Port>::testReadPartial()
{
    const char* stream_id = "read_partial";

    // Create a new stream and push some data to it
    BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
    port->pushSRI(sri);
    this->_pushTestPacket(1024, bulkio::time::utils::now(), true, stream_id);

    // Get the input stream and read the first packet
    StreamType stream = port->getStream(stream_id);
    CPPUNIT_ASSERT_EQUAL(!stream, false);
    DataBlockType block = stream.read(10000,2000);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());
    block = stream.read(10000);
    CPPUNIT_ASSERT(!block);
}

template <class Port>
void BufferedInStreamTest<Port>::testDisableDiscard()
{
    const char* stream_id = "disable_discard";

    // Create a new stream and push a couple of packets to it
    BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
    port->pushSRI(sri);
    this->_pushTestPacket(1024, bulkio::time::utils::now(), false, sri.streamID);
    CPPUNIT_ASSERT_EQUAL(1, port->getCurrentQueueDepth());

    // Get the input stream and read half of the first packet
    StreamType stream = port->getStream(stream_id);
    CPPUNIT_ASSERT_EQUAL(false, !stream);
    DataBlockType block = stream.read(512);
    CPPUNIT_ASSERT_EQUAL(false, !block);

    // The stream should report samples available, but there should be no
    // packets in the port's queue
    CPPUNIT_ASSERT(stream.ready());
    CPPUNIT_ASSERT(stream.samplesAvailable() > 0);
    CPPUNIT_ASSERT_EQUAL(0, port->getCurrentQueueDepth());

    // Disable the stream; this should discard
    stream.disable();
    CPPUNIT_ASSERT(!stream.ready());
    CPPUNIT_ASSERT_EQUAL((size_t) 0, stream.samplesAvailable());
}

template <class Port>
void NumericInStreamTest<Port>::testSriModeChanges()
{
    const char* stream_id = "sri_mode_changes";

    // Create a new stream and push some data to it
    BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
    port->pushSRI(sri);
    this->_pushTestPacket(100, bulkio::time::utils::now(), false, sri.streamID);

    // Get the input stream and read the first packet
    StreamType stream = port->getStream(stream_id);
    CPPUNIT_ASSERT(stream);
    DataBlockType block = stream.read();
    CPPUNIT_ASSERT(block);

    // First block from a new stream reports SRI change
    CPPUNIT_ASSERT_EQUAL(true, block.sriChanged());

    // Change the mode to complex and push more data
    sri.mode = 1;
    port->pushSRI(sri);
    this->_pushTestPacket(200, bulkio::time::utils::now(), false, sri.streamID);
    block = stream.read();
    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT(block.complex());
    CPPUNIT_ASSERT(block.sriChanged());
    CPPUNIT_ASSERT(block.sriChangeFlags() & bulkio::sri::MODE);

    // Next push should report no SRI changes
    this->_pushTestPacket(200, bulkio::time::utils::now(), false, sri.streamID);
    block = stream.read();
    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT(block.complex());
    CPPUNIT_ASSERT(!block.sriChanged());

    // Change back to scalar
    sri.mode = 0;
    port->pushSRI(sri);
    this->_pushTestPacket(100, bulkio::time::utils::now(), false, sri.streamID);
    block = stream.read();
    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT(!block.complex());
    CPPUNIT_ASSERT(block.sriChanged());
    CPPUNIT_ASSERT(block.sriChangeFlags() & bulkio::sri::MODE);
}

#define CREATE_TEST(x, BASE)                                            \
    class In##x##StreamTest : public BASE<bulkio::In##x##Port>          \
    {                                                                   \
        CPPUNIT_TEST_SUB_SUITE(In##x##StreamTest, BASE<bulkio::In##x##Port>); \
        CPPUNIT_TEST_SUITE_END();                                       \
        virtual std::string getPortName() const { return #x; };         \
    };                                                                  \
    CPPUNIT_TEST_SUITE_REGISTRATION(In##x##StreamTest);

#define CREATE_BASIC_TEST(x) CREATE_TEST(x, InStreamTest)
#define CREATE_NUMERIC_TEST(x) CREATE_TEST(x, NumericInStreamTest)

CREATE_BASIC_TEST(XML);
CREATE_BASIC_TEST(File);
CREATE_TEST(Bit,BufferedInStreamTest);
CREATE_NUMERIC_TEST(Octet);
CREATE_NUMERIC_TEST(Char);
CREATE_NUMERIC_TEST(Short);
CREATE_NUMERIC_TEST(UShort);
CREATE_NUMERIC_TEST(Long);
CREATE_NUMERIC_TEST(ULong);
CREATE_NUMERIC_TEST(LongLong);
CREATE_NUMERIC_TEST(ULongLong);
CREATE_NUMERIC_TEST(Float);
CREATE_NUMERIC_TEST(Double);
