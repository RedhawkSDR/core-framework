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

template <class Port>
void OutStreamTest<Port>::testOperators()
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

template <class Port>
void OutStreamTest<Port>::testBasicWrite()
{
    StreamType stream = port->createStream("test_basic_write");
    CPPUNIT_ASSERT(stub->packets.empty());

    const BULKIO::PrecisionUTCTime time = bulkio::time::utils::now();
    _writeSinglePacket(stream, 256, time);
    CPPUNIT_ASSERT(stub->packets.size() == 1);
    CPPUNIT_ASSERT_EQUAL((size_t) 256, stub->packets[0].size());
    CPPUNIT_ASSERT(!stub->packets[0].EOS);
    CPPUNIT_ASSERT_MESSAGE("Received incorrect time stamp", _checkLastTimestamp(time));
    CPPUNIT_ASSERT_EQUAL(stream.streamID(), stub->packets[0].streamID);
}

template <class Port>
void OutStreamTest<Port>::testSriFields()
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

/* Test that the SRI is not pushed when not needed.
 * Calls to set attributes of the SRI, or set or erase SRI keywords should
 * only cause the SRI to be pushed if a change was made.
 */
template <class Port>
void OutStreamTest<Port>::testSriNoChange()
{
    int expected_H_size = 0;

    StreamType stream = port->createStream("test_sri_no_change");
    BULKIO::StreamSRI sri = stream.sri();

    CPPUNIT_ASSERT_MESSAGE("SRI update occurred without call to write data.",
                           stub->H.size() == expected_H_size);

    _writeSinglePacket(stream, 10);
    expected_H_size++;
    CPPUNIT_ASSERT_MESSAGE("No SRI update before first data write.",
                           stub->H.size() == expected_H_size);

    sri.streamID = "changed_sri";
    stream.sri(sri);
    _writeSinglePacket(stream, 10);
    CPPUNIT_ASSERT_MESSAGE("Calling sri() with no sri change (except streamID) caused SRI update.",
                           stub->H.size() == expected_H_size);

    sri.xdelta = 2.0;
    stream.sri(sri);
    expected_H_size++;
    _writeSinglePacket(stream, 10);
    CPPUNIT_ASSERT_MESSAGE("Calling sri() with changed sri (other than streamID) failed to cause SRI update.",
                           stub->H.size() == expected_H_size);

    stream.xdelta(3.0);
    expected_H_size++;
    _writeSinglePacket(stream, 10);
    CPPUNIT_ASSERT_MESSAGE("Calling stream.xdelta() with changed value failed to cause SRI update.",
                           stub->H.size() == expected_H_size);

    stream.xdelta(3.0);
    _writeSinglePacket(stream, 10);
    CPPUNIT_ASSERT_MESSAGE("Calling stream.xdelta() with unchanged value caused SRI update.",
                           stub->H.size() == expected_H_size);

    redhawk::PropertyMap props;
    props["foo"] = "word1";
    props["bar"] = "word2";
    stream.keywords(props);
    expected_H_size++;
    _writeSinglePacket(stream, 10);
    CPPUNIT_ASSERT_MESSAGE("Calling stream.keywords() with new value failed to cause SRI update.",
                           stub->H.size() == expected_H_size);

    stream.keywords(props);
    _writeSinglePacket(stream, 10);
    CPPUNIT_ASSERT_MESSAGE("Calling stream.keywords() with unchanged value caused SRI update.",
                           stub->H.size() == expected_H_size);

    stream.setKeyword("foo", "word8");
    expected_H_size++;
    _writeSinglePacket(stream, 10);
    CPPUNIT_ASSERT_MESSAGE("Calling stream.setKeyword() with new value failed to cause SRI update.",
                           stub->H.size() == expected_H_size);

    stream.setKeyword("foo", "word8");
    _writeSinglePacket(stream, 10);
    CPPUNIT_ASSERT_MESSAGE("Calling stream.setKeyword() with unchanged value caused SRI update.",
                           stub->H.size() == expected_H_size);

    stream.eraseKeyword("bar");
    expected_H_size++;
    _writeSinglePacket(stream, 10);
    CPPUNIT_ASSERT_MESSAGE("Calling stream.eraseKeyword(<existing key>) failed to cause SRI update.",
                           stub->H.size() == expected_H_size);

    stream.eraseKeyword("bar");
    _writeSinglePacket(stream, 10);
    CPPUNIT_ASSERT_MESSAGE("Calling stream.eraseKeyword(<no such key>) caused SRI update.",
                           stub->H.size() == expected_H_size);
}

template <class Port>
void OutStreamTest<Port>::testSriUpdate()
{
    // Create initial stream; all changes should be queued up for the first
    // write
    StreamType stream = port->createStream("test_sri_update");
    double xdelta = 1.0 / 1.25e6;
    stream.xdelta(xdelta);
    stream.blocking(true);
    CPPUNIT_ASSERT(stub->H.empty());

    // Write data to trigger initial SRI update
    _writeSinglePacket(stream, 10);
    CPPUNIT_ASSERT(stub->H.size() == 1);
    CPPUNIT_ASSERT(stub->H.back().blocking);
    CPPUNIT_ASSERT_EQUAL(xdelta, stub->H.back().xdelta);

    // Update xdelta; no SRI update should occur
    double new_xdelta = 1.0/2.5e6;
    stream.xdelta(new_xdelta);
    CPPUNIT_ASSERT(stub->H.size() == 1);
    CPPUNIT_ASSERT_EQUAL(xdelta, stub->H.back().xdelta);

    // Write data to trigger SRI update
    _writeSinglePacket(stream, 25);
    CPPUNIT_ASSERT(stub->H.size() == 2);
    CPPUNIT_ASSERT_EQUAL(new_xdelta, stub->H.back().xdelta);

    // Change blocking flag, then trigger an SRI update
    stream.blocking(false);
    CPPUNIT_ASSERT(stub->H.size() == 2);
    CPPUNIT_ASSERT(stub->H.back().blocking);
    _writeSinglePacket(stream, 25);
    CPPUNIT_ASSERT(stub->H.size() == 3);
    CPPUNIT_ASSERT(!stub->H.back().blocking);

    // Change multiple fields, but only one SRI update should occur (after the
    // next write)
    stream.complex(true);
    stream.subsize(16);
    stream.xstart(-M_PI);
    stream.xdelta(2.0 * M_PI / 1024.0);
    stream.xunits(BULKIO::UNITS_FREQUENCY);
    stream.ydelta(1024.0 / 1.25e6);
    stream.yunits(BULKIO::UNITS_TIME);
    CPPUNIT_ASSERT(stub->H.size() == 3);

    // Trigger SRI update and verify that it matches
    _writeSinglePacket(stream, 1024);
    CPPUNIT_ASSERT(stub->H.size() == 4);
    CPPUNIT_ASSERT(bulkio::sri::DefaultComparator(stream.sri(), stub->H.back()));
}

template <class Port>
void OutStreamTest<Port>::testKeywords()
{
    StreamType stream = port->createStream("test_keywords");
    _writeSinglePacket(stream, 1);
    CPPUNIT_ASSERT(stub->H.size() == 1);

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
    CPPUNIT_ASSERT(stub->H.size() == 1);
    _writeSinglePacket(stream, 1);
    CPPUNIT_ASSERT(stub->H.size() == 2);
    {
        const redhawk::PropertyMap& keywords = redhawk::PropertyMap::cast(stub->H.back().keywords);
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
    CPPUNIT_ASSERT(stub->H.size() == 2);
    _writeSinglePacket(stream, 1);
    CPPUNIT_ASSERT(stub->H.size() == 3);
    {
        const redhawk::PropertyMap& keywords = redhawk::PropertyMap::cast(stub->H.back().keywords);
        CPPUNIT_ASSERT_EQUAL(stream.keywords().size(), keywords.size());
        CPPUNIT_ASSERT_EQUAL(stream.getKeyword("COL_RF").toDouble(), keywords.get("COL_RF").toDouble());
        CPPUNIT_ASSERT_EQUAL(stream.getKeyword("COL_RF").toDouble(), keywords.get("COL_RF").toDouble());
    }
}

template <class Port>
void OutStreamTest<Port>::testSendEosOnClose()
{
    StreamType stream = port->createStream("close_eos");

    CPPUNIT_ASSERT(stub->H.size() == 0);
    CPPUNIT_ASSERT(stub->packets.size() == 0);

    _writeSinglePacket(stream, 16);

    CPPUNIT_ASSERT(stub->H.size() == 1);
    CPPUNIT_ASSERT(stub->packets.size() == 1);
    CPPUNIT_ASSERT(!stub->packets.back().EOS);

    stream.close();
    CPPUNIT_ASSERT(stub->packets.size() == 2);
    CPPUNIT_ASSERT(stub->packets.back().EOS);
}

template <class Port>
void OutStreamTest<Port>::_writeSinglePacket(StreamType& stream, size_t size,
                                             const BULKIO::PrecisionUTCTime& time)
{
    typedef typename StreamType::ScalarType ScalarType;
    std::vector<ScalarType> buffer;
    buffer.resize(size);
    stream.write(buffer, time);
}

template <class Port>
bool OutStreamTest<Port>::_checkLastTimestamp(const BULKIO::PrecisionUTCTime& time)
{
    return (time == stub->packets.back().T);
}

// Specialization for dataBit, which uses redhawk::bitbuffer
template <>
void OutStreamTest<bulkio::OutBitPort>::_writeSinglePacket(StreamType& stream, size_t size,
                                                           const BULKIO::PrecisionUTCTime& time)
{
    redhawk::bitbuffer buffer(size);
    buffer.fill(0, buffer.size(), 0);
    stream.write(buffer, time);
}

// Specialization for dataFile, which uses std::string
template <>
void OutStreamTest<bulkio::OutFilePort>::_writeSinglePacket(StreamType& stream, size_t size,
                                                            const BULKIO::PrecisionUTCTime& time)
{
    std::string url(size, 'F');
    stream.write(url, time);
}

// Specializations for dataXML, which uses std::string and does not include a
// timestamp
template <>
void OutStreamTest<bulkio::OutXMLPort>::_writeSinglePacket(StreamType& stream, size_t size,
                                                           const BULKIO::PrecisionUTCTime& /*unused*/)
{
    std::string xml(size, 'X');
    stream.write(xml);
}

template <>
bool OutStreamTest<bulkio::OutXMLPort>::_checkLastTimestamp(const BULKIO::PrecisionUTCTime& /*unused*/)
{
    // dataXML has no time stamp, so the check should always succeed
    return true;
}


template <class Port>
void BufferedOutStreamTest<Port>::testBufferedWrite()
{
    // Initial state is unbuffered; turn on buffering
    StreamType stream = port->createStream("test_buffered_write");
    CPPUNIT_ASSERT_EQUAL((size_t) 0, stream.bufferSize());
    stream.setBufferSize(128);
    CPPUNIT_ASSERT_EQUAL((size_t) 128, stream.bufferSize());
    CPPUNIT_ASSERT(stub->packets.size() == 0);

    // First write is below the buffer size
    BufferType buffer;
    buffer.resize(48);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packets.size() == 0);

    // The second write is still below the buffer size
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packets.size() == 0);

    // The third write goes beyond the buffer size and should trigger a push,
    // but only up to the buffer size (48*3 == 144)
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packets.size() == 1);
    CPPUNIT_ASSERT_EQUAL(stream.bufferSize(), stub->packets.back().size());

    // There should now be 16 samples in the queue; writing another 48 should
    // not trigger a push
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packets.size() == 1);

    // Flush the stream and make sure we get as many samples as expected
    stream.flush();
    CPPUNIT_ASSERT(stub->packets.size() == 2);
    CPPUNIT_ASSERT_EQUAL((size_t) 64, stub->packets.back().size());

    // Disable buffering; push should happen immediately
    stream.setBufferSize(0);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packets.size() == 3);
}

template <class Port>
void BufferedOutStreamTest<Port>::testWriteSkipBuffer()
{
    // Turn on buffering
    StreamType stream = port->createStream("test_skip_buffer");
    stream.setBufferSize(100);

    // With an empty queue, large write should go right through
    BufferType buffer;
    buffer.resize(256);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packets.size() == 1);
    CPPUNIT_ASSERT_EQUAL(buffer.size(), stub->packets.back().size());

    // Queue up a bit of data
    buffer.resize(16);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packets.size() == 1);

    // With queued data, the large write should get broken up into a buffer-
    // sized packet
    buffer.resize(128);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packets.size() == 2);
    CPPUNIT_ASSERT_EQUAL(stream.bufferSize(), stub->packets.back().size());
}

template <class Port>
void BufferedOutStreamTest<Port>::testFlush()
{
    // Turn on buffering
    StreamType stream = port->createStream("test_flush");
    stream.setBufferSize(64);

    // Queue data (should not flush)
    BufferType buffer;
    buffer.resize(48);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->H.size() == 0);
    CPPUNIT_ASSERT(stub->packets.size() == 0);

    // Make sure flush sends a packet
    stream.flush();
    CPPUNIT_ASSERT(stub->H.size() == 1);
    CPPUNIT_ASSERT(stub->packets.size() == 1);
    CPPUNIT_ASSERT_EQUAL(buffer.size(), stub->packets.back().size());
}

template <class Port>
void BufferedOutStreamTest<Port>::testFlushOnClose()
{
    StreamType stream = port->createStream("test_flush_close");
    stream.setBufferSize(64);

    // Queue data (should not flush)
    BufferType buffer;
    buffer.resize(48);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->H.size() == 0);
    CPPUNIT_ASSERT(stub->packets.size() == 0);

    // Close the stream; should cause a flush
    stream.close();
    CPPUNIT_ASSERT(stub->H.size() == 1);
    CPPUNIT_ASSERT(stub->packets.size() == 1);
}

template <class Port>
void BufferedOutStreamTest<Port>::testFlushOnSriChange()
{
    // Start with known values for important stream metadata
    StreamType stream = port->createStream("test_flush_sri");
    stream.setBufferSize(64);
    stream.xdelta(0.125);
    stream.complex(false);
    stream.blocking(false);
    stream.subsize(0);

    // Queue data (should not flush)
    BufferType buffer;
    buffer.resize(48);
    stream.write(buffer, bulkio::time::utils::now());

    // Change the xdelta to cause a flush; the received data should be using
    // the old xdelta
    CPPUNIT_ASSERT(stub->packets.size() == 0);
    stream.xdelta(0.25);
    CPPUNIT_ASSERT_MESSAGE("xdelta change did not flush stream", stub->packets.size() == 1);
    CPPUNIT_ASSERT_EQUAL(0.125, stub->H.back().xdelta);

    // Queue more data (should not flush)
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->H.size() == 1);
    CPPUNIT_ASSERT(stub->packets.size() == 1);

    // Change the mode to complex to cause a flush; the mode shouldn't change
    // yet, but xdelta should be up-to-date now
    stream.complex(true);
    CPPUNIT_ASSERT_MESSAGE("Complex mode change did not flush stream", stub->packets.size() == 2);
    CPPUNIT_ASSERT(stub->H.back().mode == 0);
    CPPUNIT_ASSERT_EQUAL(stream.xdelta(), stub->H.back().xdelta);

    // Queue more data (should not flush)
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->H.size() == 2);
    CPPUNIT_ASSERT(stub->packets.size() == 2);

    // Change the blocking mode to cause a flush; the blocking flag shouldn't
    // change yet, but mode should be up-to-date now
    stream.blocking(true);
    CPPUNIT_ASSERT_MESSAGE("Blocking change did not flush stream", stub->packets.size() == 3);
    CPPUNIT_ASSERT(stub->H.back().blocking == 0);
    CPPUNIT_ASSERT(stub->H.back().mode != 0);

    // Queue more data (should not flush)
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->H.size() == 3);
    CPPUNIT_ASSERT(stub->packets.size() == 3);

    // Change the subsize to cause a flush; the subsize shouldn't change yet,
    // but blocking should be up-to-date now
    stream.subsize(16);
    CPPUNIT_ASSERT_MESSAGE("Subsize change did not flush stream", stub->packets.size() == 4);
    CPPUNIT_ASSERT(stub->H.back().subsize == 0);
    CPPUNIT_ASSERT(stub->H.back().blocking);
}

template <class Port>
void BufferedOutStreamTest<Port>::testFlushOnBufferSizeChange()
{
    StreamType stream = port->createStream("test_flush_buffer_size");
    stream.setBufferSize(64);

    // Queue data (should not flush)
    BufferType buffer;
    buffer.resize(48);
    stream.write(buffer, bulkio::time::utils::now());
    CPPUNIT_ASSERT(stub->packets.size() == 0);

    // Reduce the buffer size smaller than the current queue, should trigger a
    // flush
    stream.setBufferSize(32);
    CPPUNIT_ASSERT_MESSAGE("Reducing buffer size below queue size did not flush", stub->packets.size() == 1);

    // Reduce the buffer size again, but not down to the queue size, should not
    // trigger a flush
    buffer.resize(16);
    stream.write(buffer, bulkio::time::utils::now());
    stream.setBufferSize(24);
    CPPUNIT_ASSERT_MESSAGE("Reducing buffer size above queue size flushed", stub->packets.size() == 1);

    // Boundary condition: exact size
    stream.setBufferSize(16);
    CPPUNIT_ASSERT_MESSAGE("Reducing buffer size to exact size did not flush", stub->packets.size() == 2);

    // Increasing the buffer size should not trigger a flush
    buffer.resize(8);
    stream.write(buffer, bulkio::time::utils::now());
    stream.setBufferSize(128);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Increasing buffer size flushed", (size_t) 2, stub->packets.size());

    // Disabling buffering must flush
    stream.setBufferSize(0);
    CPPUNIT_ASSERT_MESSAGE("Disabling buffering did not flush", stub->packets.size() == 3);
}

template <class Port>
void NumericOutStreamTest<Port>::testStreamWriteCheck()
{
    StreamType stream = port->createStream("compound_push");
    // Generate a ramp using the scalar type; if the data needs to be pushed as
    // complex, it will be reintepreted there
    std::vector<ScalarType> scalars;
    size_t number_tstamps = 10;
    size_t payload_tstamps = 9;
    size_t single_push_size = 100;
    scalars.resize(single_push_size*number_tstamps);
    for (size_t ii = 0; ii < scalars.size(); ++ii) {
        scalars[ii] = ii;
    }
    size_t sample_count = scalars.size();

    // Create 9 timestamps
    BULKIO::PrecisionUTCTime start = bulkio::time::utils::now();
    std::list<bulkio::SampleTimestamp> tstamps;
    for (size_t tstamp_idx=0; tstamp_idx<number_tstamps; tstamp_idx++) {
        tstamps.push_back(bulkio::SampleTimestamp(start+tstamp_idx, single_push_size*tstamp_idx));
    }

    size_t push_size = 801;
    stream.write(&scalars[0], push_size, tstamps);

    // Data should have been broken into one chunk per timestamp
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of packets does not match timestamps",
                                 payload_tstamps, stub->packets.size());

    // Check that the packets are at the right offsets (implicitly checking
    // that the prior packet was the right size) and have the correct time
    size_t scalars_received = 0;
    std::list<bulkio::SampleTimestamp>::iterator ts = tstamps.begin();
    for (size_t ii = 0; ii < payload_tstamps; ++ii, ++ts) {
        // Take complex data into account for the expected timestamp offset
        size_t expected_offset = ts->offset;
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Packet timestamp is incorrect", ts->time, stub->packets[ii].T);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Packet offset does not match timestamp offset", expected_offset, scalars_received);
        scalars_received += stub->packets[ii].data.length();
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Final packet size is incorrect", scalars_received, push_size);
}

template <class Port>
void NumericOutStreamTest<Port>::testWriteTimestampsReal()
{
    StreamType stream = port->createStream("write_timestamps_real");
    _writeTimestampsImpl(stream, false);
}

template <class Port>
void NumericOutStreamTest<Port>::testWriteTimestampsComplex()
{
    StreamType stream = port->createStream("write_timestamps_complex");
    stream.complex(true);
    _writeTimestampsImpl(stream, true);
}

template <class Port>
void NumericOutStreamTest<Port>::testWriteTimestampsMixed()
{
   StreamType stream = port->createStream("write_timestamps_mixed");
   stream.complex(true);
   _writeTimestampsImpl(stream, false);
}

template <class Port>
void NumericOutStreamTest<Port>::_writeTimestampsImpl(StreamType& stream, bool complexData)
{
    // Generate a ramp using the scalar type; if the data needs to be pushed as
    // complex, it will be reintepreted there
    std::vector<ScalarType> scalars;
    scalars.resize(200);
    for (size_t ii = 0; ii < scalars.size(); ++ii) {
        scalars[ii] = ii;
    }
    size_t sample_count = scalars.size();
    if (stream.complex()) {
        sample_count /= 2;
    }

    // Create 3 timestamps, breaking the input data at the 1/4 and 5/8 points
    // (taking real/complex mode of the stream into account)
    BULKIO::PrecisionUTCTime start = bulkio::time::utils::now();
    std::list<bulkio::SampleTimestamp> tstamps;
    tstamps.push_back(bulkio::SampleTimestamp(start, 0));
    size_t offset = sample_count / 4;
    tstamps.push_back(bulkio::SampleTimestamp(start + offset, offset));
    offset = sample_count * 5 / 8;
    tstamps.push_back(bulkio::SampleTimestamp(start + offset, offset));

    if (complexData) {
        ComplexType* data = reinterpret_cast<ComplexType*>(&scalars[0]);
        stream.write(data, sample_count, tstamps);
    } else {
        stream.write(&scalars[0], scalars.size(), tstamps);
    }

    // Data should have been broken into one chunk per timestamp
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of packets does not match timestamps",
                                 tstamps.size(), stub->packets.size());

    // Check that the packets are at the right offsets (implicitly checking
    // that the prior packet was the right size) and have the correct time
    size_t scalars_received = 0;
    std::list<bulkio::SampleTimestamp>::iterator ts = tstamps.begin();
    for (size_t ii = 0; ii < stub->packets.size(); ++ii, ++ts) {
        // Take complex data into account for the expected timestamp offset
        size_t expected_offset = ts->offset;
        if (stream.complex()) {
            expected_offset *= 2;
        }
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Packet timestamp is incorrect", ts->time, stub->packets[ii].T);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Packet offset does not match timestamp offset", expected_offset, scalars_received);
        scalars_received += stub->packets[ii].data.length();
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Final packet size is incorrect", scalars_received, scalars.size());
}

#define CREATE_TEST_IMPL(TESTCLASS,PORT,NAME,BASE)                      \
    class TESTCLASS : public BASE<PORT>                                 \
    {                                                                   \
        typedef BASE<PORT> TestBase;                                    \
        CPPUNIT_TEST_SUB_SUITE(TESTCLASS, TestBase);                    \
        CPPUNIT_TEST_SUITE_END();                                       \
    };                                                                  \
    CPPUNIT_TEST_SUITE_REGISTRATION(TESTCLASS);

#define CREATE_TEST(x,BASE) CREATE_TEST_IMPL(Out##x##StreamTest,bulkio::Out##x##Port,x,BASE)
#define CREATE_BASIC_TEST(x) CREATE_TEST(x,OutStreamTest)
#define CREATE_NUMERIC_TEST(x) CREATE_TEST(x,NumericOutStreamTest)

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

CREATE_TEST(Bit,BufferedOutStreamTest);
CREATE_BASIC_TEST(XML);
CREATE_BASIC_TEST(File);
