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

#include "OutStreamTest.h"

#include <ossie/PropertyMap.h>
#include "bulkio.h"

template <class OutPort, class PortType>
void OutStreamTest<OutPort,PortType>::setUp()
{
    ossie::corba::CorbaInit(0,0);

    std::string name = "data" + getPortName() + "_out";
    port = new OutPort(name);

    stub = new InPortStub<PortType>();
    PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(stub);

    CORBA::Object_var objref = stub->_this();
    port->connectPort(objref, "test_connection");
}

template <class OutPort, class PortType>
void OutStreamTest<OutPort,PortType>::tearDown()
{
    port->disconnectPort("test_connection");

    delete port;

    try {
        PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->servant_to_id(stub);
        ossie::corba::RootPOA()->deactivate_object(oid);
    } catch (...) {
        // Ignore CORBA exceptions
    }
    stub->_remove_ref();
}

template <class OutPort, class PortType>
void OutStreamTest<OutPort,PortType>::testOperators()
{
    StreamType null_stream;
    CPPUNIT_ASSERT(!null_stream);
    if (null_stream) {
        // This check is structured as an if/else to avoid using operator!
        CPPUNIT_FAIL("Null stream evaluted to true");
    }
    CPPUNIT_ASSERT(null_stream == StreamType());

    // Create a new stream
    StreamType good_stream = port->createStream("test_operators");
    CPPUNIT_ASSERT_EQUAL(false, !good_stream);
    if (good_stream) {
        // This check is structured as an if/else because CppUnit's assert
        // macro implicitly uses operator!
    } else {
        CPPUNIT_FAIL("Valid stream evaluated to false");
    }
    CPPUNIT_ASSERT(good_stream != null_stream);

    // Get another handle to the same stream, should be equal
    StreamType same_stream = port->getStream("test_operators");
    CPPUNIT_ASSERT(same_stream == good_stream);

    // Create a new stream, should not be equal
    StreamType other_stream = port->createStream("test_operators_2");
    CPPUNIT_ASSERT(other_stream != good_stream);
}

template <class OutPort, class PortType>
void OutStreamTest<OutPort,PortType>::testBasicWrite()
{
    StreamType stream = port->createStream("test_basic_write");
    CPPUNIT_ASSERT(stub->packetCounter == 0);

    const BULKIO::PrecisionUTCTime time = bulkio::time::utils::now();
    _writeSinglePacket(stream, 256, time);
    CPPUNIT_ASSERT(stub->packetCounter == 1);
    CPPUNIT_ASSERT_EQUAL((size_t) 256, (size_t) stub->data.length());
    CPPUNIT_ASSERT(!stub->EOS);
    if (_hasTimestamp()) {
        CPPUNIT_ASSERT_EQUAL(time, stub->T);
    }
    CPPUNIT_ASSERT_EQUAL(stream.streamID(), stub->streamID);
}

template <class OutPort, class PortType>
void OutStreamTest<OutPort,PortType>::testSriFields()
{
    BULKIO::StreamSRI sri = bulkio::sri::create("test_sri");
    sri.xstart = -2.5;
    sri.xdelta = 0.125;
    sri.xunits = BULKIO::UNITS_FREQUENCY;
    sri.subsize = 1024;
    sri.ystart = 2.5;
    sri.ydelta = 1.0;
    sri.yunits = BULKIO::UNITS_TIME;
    sri.mode = 1;
    sri.blocking = 1;
    ossie::corba::push_back(sri.keywords, redhawk::PropertyType("string", "value"));
    ossie::corba::push_back(sri.keywords, redhawk::PropertyType("number", (CORBA::Long)100));
   
    // Create a stream from the SRI; assign to a const variable to ensure that
    // all accessors are const-safe
    const StreamType stream = port->createStream(sri);
    CPPUNIT_ASSERT(stream.streamID() == (const char*) sri.streamID);
    CPPUNIT_ASSERT(stream.xstart() == sri.xstart);
    CPPUNIT_ASSERT(stream.xdelta() == sri.xdelta);
    CPPUNIT_ASSERT(stream.xunits() == sri.xunits);
    CPPUNIT_ASSERT(stream.subsize() == sri.subsize);
    CPPUNIT_ASSERT(stream.ystart() == sri.ystart);
    CPPUNIT_ASSERT(stream.ydelta() == sri.ydelta);
    CPPUNIT_ASSERT(stream.yunits() == sri.yunits);
    CPPUNIT_ASSERT(stream.complex());
    CPPUNIT_ASSERT(stream.blocking());
    CPPUNIT_ASSERT(sri.keywords.length() == stream.keywords().size());
    CPPUNIT_ASSERT_EQUAL(std::string("value"), stream.getKeyword("string").toString());
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) 100, stream.getKeyword("number").toLong());
}

template <class OutPort, class PortType>
void OutStreamTest<OutPort,PortType>::testSriUpdate()
{
    // Create initial stream; all changes should be queued up for the first
    // write
    StreamType stream = port->createStream("test_sri_update");
    double xdelta = 1.0 / 1.25e6;
    stream.xdelta(xdelta);
    stream.blocking(true);
    CPPUNIT_ASSERT(stub->sriCounter == 0);

    // Write data to trigger initial SRI update
    _writeSinglePacket(stream, 10);
    CPPUNIT_ASSERT(stub->sriCounter == 1);
    CPPUNIT_ASSERT(stub->H.blocking);
    CPPUNIT_ASSERT_EQUAL(xdelta, stub->H.xdelta);

    // Update xdelta; no SRI update should occur
    double new_xdelta = 1.0/2.5e6;
    stream.xdelta(new_xdelta);
    CPPUNIT_ASSERT(stub->sriCounter == 1);
    CPPUNIT_ASSERT_EQUAL(xdelta, stub->H.xdelta);

    // Write data to trigger SRI update
    _writeSinglePacket(stream, 25);
    CPPUNIT_ASSERT(stub->sriCounter == 2);
    CPPUNIT_ASSERT_EQUAL(new_xdelta, stub->H.xdelta);

    // Change blocking flag, then trigger an SRI update
    stream.blocking(false);
    CPPUNIT_ASSERT(stub->sriCounter == 2);
    CPPUNIT_ASSERT(stub->H.blocking);
    _writeSinglePacket(stream, 25);
    CPPUNIT_ASSERT(stub->sriCounter == 3);
    CPPUNIT_ASSERT(!stub->H.blocking);

    // Change multiple fields, but only one SRI update should occur (after the
    // next write)
    stream.complex(true);
    stream.subsize(16);
    stream.xstart(-M_PI);
    stream.xdelta(2.0 * M_PI / 1024.0);
    stream.xunits(BULKIO::UNITS_FREQUENCY);
    stream.ydelta(1024.0 / 1.25e6);
    stream.yunits(BULKIO::UNITS_TIME);
    CPPUNIT_ASSERT(stub->sriCounter == 3);

    // Trigger SRI update and verify that it matches
    _writeSinglePacket(stream, 1024);
    CPPUNIT_ASSERT(stub->sriCounter == 4);
    CPPUNIT_ASSERT(bulkio::sri::DefaultComparator(stream.sri(), stub->H));
}

template <class OutPort, class PortType>
void OutStreamTest<OutPort,PortType>::testKeywords()
{
    StreamType stream = port->createStream("test_keywords");
    _writeSinglePacket(stream, 1);
    CPPUNIT_ASSERT(stub->sriCounter == 1);

    // Set/get keywords
    stream.setKeyword("integer", (CORBA::Long)250);
    stream.setKeyword("string", "value");
    stream.setKeyword("double", 101.1e6);
    stream.setKeyword("boolean", false);
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) 250, stream.getKeyword("integer").toLong());
    CPPUNIT_ASSERT_EQUAL(std::string("value"), stream.getKeyword("string").toString());
    CPPUNIT_ASSERT_EQUAL(101.1e6, stream.getKeyword("double").toDouble());
    CPPUNIT_ASSERT(!stream.getKeyword("boolean").toBoolean());

    // Erase and check for presence of keywords
    stream.eraseKeyword("string");
    CPPUNIT_ASSERT(stream.hasKeyword("integer"));
    CPPUNIT_ASSERT(!stream.hasKeyword("string"));
    CPPUNIT_ASSERT(stream.hasKeyword("double"));
    CPPUNIT_ASSERT(stream.hasKeyword("boolean"));

    // Write a packet to trigger an SRI update
    CPPUNIT_ASSERT(stub->sriCounter == 1);
    _writeSinglePacket(stream, 1);
    CPPUNIT_ASSERT(stub->sriCounter == 2);
    {
        const redhawk::PropertyMap& keywords = redhawk::PropertyMap::cast(stub->H.keywords);
        CPPUNIT_ASSERT_EQUAL(stream.keywords().size(), keywords.size());
        CPPUNIT_ASSERT_EQUAL(stream.getKeyword("integer").toLong(), keywords.get("integer").toLong());
        CPPUNIT_ASSERT_EQUAL(stream.getKeyword("double").toDouble(), keywords.get("double").toDouble());
        CPPUNIT_ASSERT_EQUAL(stream.getKeyword("boolean").toBoolean(), keywords.get("boolean").toBoolean());
    }

    // Replace keywords with a new set
    redhawk::PropertyMap new_keywords;
    new_keywords["COL_RF"] = 100.0e6;
    new_keywords["CHAN_RF"] = 101.1e6;
    stream.keywords(new_keywords);
    CPPUNIT_ASSERT_EQUAL((size_t) 2, stream.keywords().size());
    CPPUNIT_ASSERT_EQUAL(100.0e6, stream.getKeyword("COL_RF").toDouble());
    CPPUNIT_ASSERT_EQUAL(101.1e6, stream.getKeyword("CHAN_RF").toDouble());

    // Trigger another SRI update
    CPPUNIT_ASSERT(stub->sriCounter == 2);
    _writeSinglePacket(stream, 1);
    CPPUNIT_ASSERT(stub->sriCounter == 3);
    {
        const redhawk::PropertyMap& keywords = redhawk::PropertyMap::cast(stub->H.keywords);
        CPPUNIT_ASSERT_EQUAL(stream.keywords().size(), keywords.size());
        CPPUNIT_ASSERT_EQUAL(stream.getKeyword("COL_RF").toDouble(), keywords.get("COL_RF").toDouble());
        CPPUNIT_ASSERT_EQUAL(stream.getKeyword("COL_RF").toDouble(), keywords.get("COL_RF").toDouble());
    }
}

template <class OutPort, class PortType>
void OutStreamTest<OutPort,PortType>::testSendEosOnClose()
{
    StreamType stream = port->createStream("close_eos");

    CPPUNIT_ASSERT(stub->sriCounter == 0);
    CPPUNIT_ASSERT(stub->packetCounter == 0);

    _writeSinglePacket(stream, 16);

    CPPUNIT_ASSERT(stub->sriCounter == 1);
    CPPUNIT_ASSERT(stub->packetCounter == 1);
    CPPUNIT_ASSERT(!stub->EOS);

    stream.close();
    CPPUNIT_ASSERT(stub->packetCounter == 2);
    CPPUNIT_ASSERT(stub->EOS);
}

template <class OutPort, class PortType>
void OutStreamTest<OutPort,PortType>::_writeSinglePacket(StreamType& stream, size_t size,
                                                         const BULKIO::PrecisionUTCTime& time)
{
    typedef typename StreamType::ScalarType ScalarType;
    std::vector<ScalarType> buffer;
    buffer.resize(size);
    stream.write(buffer, time);
}

template <class OutPort, class PortType>
bool OutStreamTest<OutPort,PortType>::_hasTimestamp()
{
    return true;
}

// Specialization for dataFile, which uses std::string
template <>
void OutStreamTest<bulkio::OutFilePort,BULKIO::dataFile>::_writeSinglePacket(StreamType& stream, size_t size,
                                                                             const BULKIO::PrecisionUTCTime& time)
{
    std::string url(size, 'F');
    stream.write(url, time);
}

// Specializations for dataXML, which uses std::string and does not include a
// timestamp
template <>
void OutStreamTest<bulkio::OutXMLPort,BULKIO::dataXML>::_writeSinglePacket(StreamType& stream, size_t size,
                                                                           const BULKIO::PrecisionUTCTime& /*unused*/)
{
    std::string xml(size, 'X');
    stream.write(xml);
}

template <>
bool OutStreamTest<bulkio::OutXMLPort,BULKIO::dataXML>::_hasTimestamp()
{
    return false;
}


template <class OutPort, class PortType>
void BufferedOutStreamTest<OutPort,PortType>::testBufferedWrite()
{
    // Initial state is unbuffered; turn on buffering
    StreamType stream = port->createStream("test_buffered_write");
    CPPUNIT_ASSERT_EQUAL((size_t) 0, stream.bufferSize());
    stream.setBufferSize(128);
    CPPUNIT_ASSERT_EQUAL((size_t) 128, stream.bufferSize());
    CPPUNIT_ASSERT(stub->packetCounter == 0);

    // First write is below the buffer size
    std::vector<ScalarType> buffer;
    buffer.resize(48);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packetCounter == 0);

    // The second write is still below the buffer size
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packetCounter == 0);

    // The third write goes beyond the buffer size and should trigger a push,
    // but only up to the buffer size (48*3 == 144)
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packetCounter == 1);
    CPPUNIT_ASSERT_EQUAL(stream.bufferSize(), (size_t) stub->data.length());

    // There should now be 16 samples in the queue; writing another 48 should
    // not trigger a push
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packetCounter == 1);

    // Flush the stream and make sure we get as many samples as expected
    stream.flush();
    CPPUNIT_ASSERT(stub->packetCounter == 2);
    CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 64, stub->data.length());

    // Disable buffering; push should happen immediately
    stream.setBufferSize(0);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packetCounter == 3);
}

template <class OutPort, class PortType>
void BufferedOutStreamTest<OutPort,PortType>::testWriteSkipBuffer()
{
    // Turn on buffering
    StreamType stream = port->createStream("test_skip_buffer");
    stream.setBufferSize(100);

    // With an empty queue, large write should go right through
    std::vector<ScalarType> buffer;
    buffer.resize(256);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packetCounter == 1);
    CPPUNIT_ASSERT_EQUAL(buffer.size(), (size_t) stub->data.length());

    // Queue up a bit of data
    buffer.resize(16);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packetCounter == 1);

    // With queued data, the large write should get broken up into a buffer-
    // sized packet
    buffer.resize(128);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packetCounter == 2);
    CPPUNIT_ASSERT_EQUAL(stream.bufferSize(), (size_t) stub->data.length());
}

template <class OutPort, class PortType>
void BufferedOutStreamTest<OutPort,PortType>::testFlush()
{
    // Turn on buffering
    StreamType stream = port->createStream("test_flush");
    stream.setBufferSize(64);

    // Queue data (should not flush)
    std::vector<ScalarType> buffer;
    buffer.resize(48);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->sriCounter == 0);
    CPPUNIT_ASSERT(stub->packetCounter == 0);

    // Make sure flush sends a packet
    stream.flush();
    CPPUNIT_ASSERT(stub->sriCounter == 1);
    CPPUNIT_ASSERT(stub->packetCounter == 1);
    CPPUNIT_ASSERT_EQUAL(buffer.size(), (size_t) stub->data.length());
}

template <class OutPort, class PortType>
void BufferedOutStreamTest<OutPort,PortType>::testFlushOnClose()
{
    StreamType stream = port->createStream("test_flush_close");
    stream.setBufferSize(64);

    // Queue data (should not flush)
    std::vector<ScalarType> buffer;
    buffer.resize(48);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->sriCounter == 0);
    CPPUNIT_ASSERT(stub->packetCounter == 0);

    // Close the stream; should cause a flush
    stream.close();
    CPPUNIT_ASSERT(stub->sriCounter == 1);
    CPPUNIT_ASSERT(stub->packetCounter == 1);
}

template <class OutPort, class PortType>
void BufferedOutStreamTest<OutPort,PortType>::testFlushOnSriChange()
{
    // Start with known values for important stream metadata
    StreamType stream = port->createStream("test_flush_sri");
    stream.setBufferSize(64);
    stream.xdelta(0.125);
    stream.complex(false);
    stream.blocking(false);
    stream.subsize(0);

    // Queue data (should not flush)
    std::vector<ScalarType> buffer;
    buffer.resize(48);
    stream.write(buffer, bulkio::time::utils::now());

    // Change the xdelta to cause a flush; the received data should be using
    // the old xdelta
    CPPUNIT_ASSERT(stub->packetCounter == 0);
    stream.xdelta(0.25);
    CPPUNIT_ASSERT_MESSAGE("xdelta change did not flush stream", stub->packetCounter == 1);
    CPPUNIT_ASSERT_EQUAL(0.125, stub->H.xdelta);

    // Queue more data (should not flush)
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->sriCounter == 1);
    CPPUNIT_ASSERT(stub->packetCounter == 1);

    // Change the mode to complex to cause a flush; the mode shouldn't change
    // yet, but xdelta should be up-to-date now
    stream.complex(true);
    CPPUNIT_ASSERT_MESSAGE("Complex mode change did not flush stream", stub->packetCounter == 2);
    CPPUNIT_ASSERT(stub->H.mode == 0);
    CPPUNIT_ASSERT_EQUAL(stream.xdelta(), stub->H.xdelta);

    // Queue more data (should not flush)
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->sriCounter == 2);
    CPPUNIT_ASSERT(stub->packetCounter == 2);

    // Change the blocking mode to cause a flush; the blocking flag shouldn't
    // change yet, but mode should be up-to-date now
    stream.blocking(true);
    CPPUNIT_ASSERT_MESSAGE("Blocking change did not flush stream", stub->packetCounter == 3);
    CPPUNIT_ASSERT(stub->H.blocking == 0);
    CPPUNIT_ASSERT(stub->H.mode != 0);

    // Queue more data (should not flush)
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->sriCounter == 3);
    CPPUNIT_ASSERT(stub->packetCounter == 3);

    // Change the subsize to cause a flush; the subsize shouldn't change yet,
    // but blocking should be up-to-date now
    stream.subsize(16);
    CPPUNIT_ASSERT_MESSAGE("Subsize change did not flush stream", stub->packetCounter == 4);
    CPPUNIT_ASSERT(stub->H.subsize == 0);
    CPPUNIT_ASSERT(stub->H.blocking);
}

template <class OutPort, class PortType>
void BufferedOutStreamTest<OutPort,PortType>::testFlushOnBufferSizeChange()
{
    StreamType stream = port->createStream("test_flush_buffer_size");
    stream.setBufferSize(64);

    // Queue data (should not flush)
    std::vector<ScalarType> buffer;
    buffer.resize(48);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packetCounter == 0);

    // Reduce the buffer size smaller than the current queue, should trigger a
    // flush
    stream.setBufferSize(32);
    CPPUNIT_ASSERT_MESSAGE("Reducing buffer size below queue size did not flush", stub->packetCounter == 1);

    // Reduce the buffer size again, but not down to the queue size, should not
    // trigger a flush
    buffer.resize(16);
    stream.write(buffer, bulkio::time::utils::now());
    stream.setBufferSize(24);
    CPPUNIT_ASSERT_MESSAGE("Reducing buffer size above queue size flushed", stub->packetCounter == 1);

    // Boundary condition: exact size
    stream.setBufferSize(16);
    CPPUNIT_ASSERT_MESSAGE("Reducing buffer size to exact size did not flush", stub->packetCounter == 2);

    // Increasing the buffer size should not trigger a flush
    buffer.resize(8);
    stream.write(buffer, bulkio::time::utils::now());
    stream.setBufferSize(128);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Increasing buffer size flushed", 2, stub->packetCounter);

    // Disabling buffering must flush
    stream.setBufferSize(0);
    CPPUNIT_ASSERT_MESSAGE("Disabling buffering did not flush", stub->packetCounter == 3);
}

#define CREATE_TEST_IMPL(TESTCLASS,PORT,NAME,BASE,CORBA)                \
    class TESTCLASS : public BASE<PORT,CORBA>                           \
    {                                                                   \
        typedef BASE<PORT,CORBA> TestBase;                              \
        CPPUNIT_TEST_SUB_SUITE(TESTCLASS, TestBase);                    \
        CPPUNIT_TEST_SUITE_END();                                       \
        virtual std::string getPortName() const { return #NAME; };      \
    };                                                                  \
    CPPUNIT_TEST_SUITE_REGISTRATION(TESTCLASS);

#define CREATE_TEST(x,BASE,y) CREATE_TEST_IMPL(Out##x##StreamTest,bulkio::Out##x##Port,x,BASE,y)
#define CREATE_BASIC_TEST(x,y) CREATE_TEST(x,OutStreamTest,y)
#define CREATE_BUFFERED_TEST(x,y) CREATE_TEST(x,BufferedOutStreamTest,y)

CREATE_BUFFERED_TEST(Octet,BULKIO::dataOctet);
CREATE_BUFFERED_TEST(Char,BULKIO::dataChar);
CREATE_BUFFERED_TEST(Short,BULKIO::dataShort);
CREATE_BUFFERED_TEST(UShort,BULKIO::dataUshort);
CREATE_BUFFERED_TEST(Long,BULKIO::dataLong);
CREATE_BUFFERED_TEST(ULong,BULKIO::dataUlong);
CREATE_BUFFERED_TEST(LongLong,BULKIO::dataLongLong);
CREATE_BUFFERED_TEST(ULongLong,BULKIO::dataUlongLong);
CREATE_BUFFERED_TEST(Float,BULKIO::dataFloat);
CREATE_BUFFERED_TEST(Double,BULKIO::dataDouble);

CREATE_BASIC_TEST(XML,BULKIO::dataXML);
CREATE_BASIC_TEST(File,BULKIO::dataFile);
