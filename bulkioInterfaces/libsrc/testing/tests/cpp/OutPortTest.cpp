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

#include "OutPortTest.h"

// Suppress warnings for access to deprecated methods
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// Global connection/disconnection callbacks
static void port_connected(const char* connectionId)
{
}

static void port_disconnected(const char* connectionId)
{
}

template <class Port>
void OutPortTest<Port>::testBasicAPI()
{
    port->setNewConnectListener(&port_connected);
    port->setNewDisconnectListener(&port_disconnected);

    BULKIO::StreamSRI sri;
    port->pushSRI(sri);

    typename Port::ConnectionsList cl = port->_getConnections();
    std::string sid="none";
    CPPUNIT_ASSERT(port->getCurrentSRI().count(sid) == 0);

    port->enableStats(false);

    rh_logger::LoggerPtr logger = rh_logger::Logger::getLogger("BulkioOutPort");
    port->setLogger(logger);
}

template <class Port>
void OutPortTest<Port>::testConnections()
{
    // Should start with one connection, to the in port stub
    ExtendedCF::UsesConnectionSequence_var connections = port->connections();
    CPPUNIT_ASSERT(connections->length() == 1);
    CPPUNIT_ASSERT(port->state() == BULKIO::ACTIVE);

    // Should throw an invalid port on a nil
    CORBA::Object_var objref;
    CPPUNIT_ASSERT_THROW(port->connectPort(objref, "connection_nil"), CF::Port::InvalidPort);

    // Normal connection
    StubType* stub2 = this->_createStub();
    objref = stub2->_this();
    port->connectPort(objref, "connection_2");
    connections = port->connections();
    CPPUNIT_ASSERT(connections->length() == 2);

    // Cannot reuse connection ID
    CPPUNIT_ASSERT_THROW(port->connectPort(objref, "connection_2"), CF::Port::OccupiedPort);

    // Disconnect second connection
    port->disconnectPort("connection_2");
    connections = port->connections();
    CPPUNIT_ASSERT(connections->length() == 1);

    // Bad connection ID on disconnect
    CPPUNIT_ASSERT_THROW(port->disconnectPort("connection_bad"), CF::Port::InvalidPort);

    // Disconnect the default stub; port should go to idle
    port->disconnectPort("test_connection");
    connections = port->connections();
    CPPUNIT_ASSERT(connections->length() == 0);
    CPPUNIT_ASSERT(port->state() == BULKIO::IDLE);
}

template <class Port>
void OutPortTest<Port>::testStatistics()
{
    const char* stream_id = "port_stats";

    BULKIO::UsesPortStatisticsSequence_var uses_stats = port->statistics();
    CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 1, uses_stats->length());
    CPPUNIT_ASSERT_EQUAL(std::string("test_connection"), std::string(uses_stats[0].connectionId));

    BULKIO::StreamSRI sri = bulkio::sri::create();
    port->pushSRI(sri);

    this->_pushTestPacket(1024, BULKIO::PrecisionUTCTime(), false, stream_id);

    uses_stats = port->statistics();
    CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 1, uses_stats->length());
    const BULKIO::PortStatistics& stats = uses_stats[0].statistics;

    CPPUNIT_ASSERT(stats.elementsPerSecond > 0.0);
    size_t bits_per_element = round(stats.bitsPerSecond / stats.elementsPerSecond);
    //CPPUNIT_ASSERT_EQUAL(8 * sizeof(typename Port::NativeType), bits_per_element);
}

template <class Port>
class NumericOutPortTest : public OutPortTest<Port>
{
    typedef OutPortTest<Port> TestBase;

    CPPUNIT_TEST_SUB_SUITE(NumericOutPortTest, TestBase);
    CPPUNIT_TEST(testPushPointer);
    CPPUNIT_TEST_SUITE_END();

public:
    void testPushPointer()
    {
        const char* stream_id = "push_pointer";
        BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
        port->pushSRI(sri);

        NativeType buffer[128];
        port->pushPacket(buffer, 128, BULKIO::PrecisionUTCTime(), false, stream_id);
        CPPUNIT_ASSERT(stub->packets.size() == 1);
        CPPUNIT_ASSERT_EQUAL((size_t) 128, stub->packets.back().size());
    }

protected:
    typedef typename Port::NativeType NativeType;

    using TestBase::port;
    using TestBase::stub;
};

class OutCharPortTest : public NumericOutPortTest<bulkio::OutCharPort>
{
    typedef NumericOutPortTest<bulkio::OutCharPort> TestBase;

    CPPUNIT_TEST_SUB_SUITE(OutCharPortTest, TestBase);
    CPPUNIT_TEST(testPushChar);
    CPPUNIT_TEST_SUITE_END();

public:
    void testPushChar()
    {
        // Test for char* overloads of pushPacket
        const char* stream_id = "push_char";
        BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
        port->pushSRI(sri);

        std::vector<char> vec;
        port->pushPacket(vec, BULKIO::PrecisionUTCTime(), false, stream_id);
        CPPUNIT_ASSERT(stub->packets.size() == 1);
        CPPUNIT_ASSERT_EQUAL(vec.size(), stub->packets.back().size());

        char buffer[100];
        port->pushPacket(buffer, sizeof(buffer), BULKIO::PrecisionUTCTime(), false, stream_id);
        CPPUNIT_ASSERT(stub->packets.size() == 2);
        CPPUNIT_ASSERT_EQUAL(sizeof(buffer), stub->packets.back().size());
    }

protected:
    virtual std::string getPortName() const { return "Char"; };

    using TestBase::port;
    using TestBase::stub;
};

#define CREATE_TEST(x,BASE)                                             \
    class Out##x##PortTest : public BASE<bulkio::Out##x##Port>          \
    {                                                                   \
        CPPUNIT_TEST_SUB_SUITE(Out##x##PortTest, BASE<bulkio::Out##x##Port>); \
        CPPUNIT_TEST_SUITE_END();                                       \
        virtual std::string getPortName() const { return #x; };         \
    };                                                                  \
    CPPUNIT_TEST_SUITE_REGISTRATION(Out##x##PortTest);

#define CREATE_BASIC_TEST(x) CREATE_TEST(x,OutPortTest)
#define CREATE_NUMERIC_TEST(x) CREATE_TEST(x,NumericOutPortTest)

CREATE_NUMERIC_TEST(Octet);
CREATE_NUMERIC_TEST(Short);
CREATE_NUMERIC_TEST(UShort);
CREATE_NUMERIC_TEST(Long);
CREATE_NUMERIC_TEST(ULong);
CREATE_NUMERIC_TEST(LongLong);
CREATE_NUMERIC_TEST(ULongLong);
CREATE_NUMERIC_TEST(Float);
CREATE_NUMERIC_TEST(Double);
CREATE_BASIC_TEST(Bit);
CREATE_BASIC_TEST(XML);
CREATE_BASIC_TEST(File);
