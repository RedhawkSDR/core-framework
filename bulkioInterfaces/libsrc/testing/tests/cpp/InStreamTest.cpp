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
    typedef typename Port::PortSequenceType PortSequenceType;
    typedef typename Port::StreamType StreamType;
    typedef typename StreamType::DataBlockType DataBlockType;

    // Create a new stream and push some data to it
    BULKIO::StreamSRI sri = bulkio::sri::create("empty_eos");
    port->pushSRI(sri);
    PortSequenceType data;
    data.length(1024);
    port->pushPacket(data, bulkio::time::utils::now(), false, sri.streamID);

    // Get the input stream and read the first packet
    StreamType stream = port->getStream("empty_eos");
    CPPUNIT_ASSERT_EQUAL(!stream, false);
    DataBlockType block = stream.read();
    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());
    CPPUNIT_ASSERT(!stream.eos());

    // Push an end-of-stream packet with no data and get the stream again
    data.length(0);
    port->pushPacket(data, bulkio::time::utils::notSet(), true, sri.streamID);
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
    typedef typename Port::PortSequenceType PortSequenceType;
    typedef typename Port::StreamType StreamType;
    typedef typename StreamType::DataBlockType DataBlockType;

    // Create a new stream and push some data to it
    BULKIO::StreamSRI sri = bulkio::sri::create("empty_eos");
    port->pushSRI(sri);
    PortSequenceType data;
    data.length(1024);
    port->pushPacket(data, bulkio::time::utils::now(), false, sri.streamID);

    // Get the input stream and read the first packet
    StreamType stream = port->getStream("empty_eos");
    CPPUNIT_ASSERT_EQUAL(!stream, false);
    DataBlockType block = stream.read();
    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());
    CPPUNIT_ASSERT(!stream.eos());

    // Push an end-of-stream packet with data and get the stream again
    data.length(1024);
    port->pushPacket(data, bulkio::time::utils::now(), true, sri.streamID);
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
void InStreamTest<Port>::testSriModeChanges()
{
    typedef typename Port::PortSequenceType PortSequenceType;
    typedef typename Port::StreamType StreamType;
    typedef typename StreamType::DataBlockType DataBlockType;

    // Create a new stream and push some data to it
    BULKIO::StreamSRI sri = bulkio::sri::create("empty_eos");
    port->pushSRI(sri);
    PortSequenceType data;
    data.length(1024);
    port->pushPacket(data, bulkio::time::utils::now(), false, sri.streamID);

    // Get the input stream and read the first packet
    StreamType stream = port->getStream("empty_eos");
    CPPUNIT_ASSERT_EQUAL(!stream, false);
    DataBlockType block = stream.read();
    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());
    CPPUNIT_ASSERT(!stream.eos());
    CPPUNIT_ASSERT_EQUAL(block.complex(),false);

    // check mode....true
    sri.mode=1;
    port->pushSRI(sri);
    data.length(1024);
    port->pushPacket(data, bulkio::time::utils::now(), false, sri.streamID);
    block = stream.read();
    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());
    CPPUNIT_ASSERT(!stream.eos());
    CPPUNIT_ASSERT_EQUAL(block.complex(),true);

    // check mode....false
    sri.mode=0;
    port->pushSRI(sri);
    data.length(1024);
    port->pushPacket(data, bulkio::time::utils::now(), false, sri.streamID);
    block = stream.read();
    CPPUNIT_ASSERT(block);
    CPPUNIT_ASSERT_EQUAL((size_t) 1024, block.size());
    CPPUNIT_ASSERT(!stream.eos());
    CPPUNIT_ASSERT_EQUAL(block.complex(),false);
}



template <class Port>
void BufferedInStreamTest<Port>::testReadSizeEos()
{
    // Create a new stream and push an end-of-stream packet with no data
    BULKIO::StreamSRI sri = bulkio::sri::create("empty_eos");
    port->pushSRI(sri);
    PortSequenceType data;
    data.length(0);
    port->pushPacket(data, bulkio::time::utils::notSet(), true, sri.streamID);

    // Try to read a single element; this should return a null block
    StreamType stream = port->getStream("empty_eos");
    CPPUNIT_ASSERT(stream);
    DataBlockType block = stream.read(1);
    CPPUNIT_ASSERT(!block);
    CPPUNIT_ASSERT(stream.eos());
}


#define CREATE_TEST(x)                                                  \
    class In##x##StreamTest : public BufferedInStreamTest<bulkio::In##x##Port>  \
    {                                                                   \
        CPPUNIT_TEST_SUB_SUITE(In##x##StreamTest, BufferedInStreamTest<bulkio::In##x##Port>); \
        CPPUNIT_TEST_SUITE_END();                                       \
        virtual std::string getPortName() const { return #x; };         \
    };                                                                  \
    CPPUNIT_TEST_SUITE_REGISTRATION(In##x##StreamTest);

CREATE_TEST(Octet);
CREATE_TEST(Char);
CREATE_TEST(Short);
CREATE_TEST(UShort);
CREATE_TEST(Long);
CREATE_TEST(ULong);
CREATE_TEST(LongLong);
CREATE_TEST(ULongLong);
CREATE_TEST(Float);
CREATE_TEST(Double);
