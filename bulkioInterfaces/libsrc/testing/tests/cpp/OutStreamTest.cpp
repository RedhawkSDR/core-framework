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
#include "bulkio.h"

template <class Port>
void OutStreamTest<Port>::setUp()
{
    ossie::corba::CorbaInit(0,0);

    std::string name = "data" + getPortName() + "_out";
    port = new Port(name);

    stub = new InPortStub<PortTraits>();
    PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(stub);

    CORBA::Object_var objref = stub->_this();
    port->connectPort(objref, "test_connection");
}

template <class Port>
void OutStreamTest<Port>::tearDown()
{
    port->disconnectPort("test_connection");

    // The port has not been used as a CORBA object, so we can delete it directly
    delete port;

    try {
        PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->servant_to_id(stub);
        ossie::corba::RootPOA()->deactivate_object(oid);
    } catch (...) {
        // Ignore CORBA exceptions
    }
    stub->_remove_ref();
    stub = 0;
}

template <class Port>
void OutStreamTest<Port>::testStreamWriteCheck()
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
void OutStreamTest<Port>::testWriteTimestampsReal()
{
    StreamType stream = port->createStream("write_timestamps_real");
    _writeTimestampsImpl(stream, false);
}

template <class Port>
void OutStreamTest<Port>::testWriteTimestampsComplex()
{
    StreamType stream = port->createStream("write_timestamps_complex");
    stream.complex(true);
    _writeTimestampsImpl(stream, true);
}

template <class Port>
void OutStreamTest<Port>::testWriteTimestampsMixed()
{
   StreamType stream = port->createStream("write_timestamps_mixed");
   stream.complex(true);
   _writeTimestampsImpl(stream, false);
}

template <class Port>
void OutStreamTest<Port>::_writeTimestampsImpl(StreamType& stream, bool complexData)
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


#define CREATE_TEST(x)                                                  \
    class Out##x##StreamTest : public OutStreamTest<bulkio::Out##x##Port>  \
    {                                                                   \
        CPPUNIT_TEST_SUB_SUITE(Out##x##StreamTest, OutStreamTest<bulkio::Out##x##Port>); \
        CPPUNIT_TEST_SUITE_END();                                       \
        virtual std::string getPortName() const { return #x; };         \
    };                                                                  \
    CPPUNIT_TEST_SUITE_REGISTRATION(Out##x##StreamTest);

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
