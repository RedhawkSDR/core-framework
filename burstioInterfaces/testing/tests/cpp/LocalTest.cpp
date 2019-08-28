/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK burstioInterfaces.
 *
 * REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#include <boost/scoped_ptr.hpp>

#include "LocalTest.h"

#include <burstio/burstio.h>
#include <bulkio/bulkio_time_operators.h>

template <class OutPort,class InPort>
void LocalTest<OutPort,InPort>::setUp()
{
    rootLogger = rh_logger::Logger::getLogger("Local"+getPortName()+"Test");

    std::string name = "burst" + getPortName();
    outPort = new OutPort(name + "_out");
    outPort->setLogger(rh_logger::Logger::getLogger(rootLogger->getName() + "." + outPort->getName()));
    inPort = new InPort(name + "_in");
    inPort->setLogger(rh_logger::Logger::getLogger(rootLogger->getName() + "." + inPort->getName()));

    _activatePort(inPort);

    CORBA::Object_var objref = inPort->_this();
    outPort->connectPort(objref, "local_connection");

    inPort->start();
    outPort->start();
}

template <class OutPort,class InPort>
void LocalTest<OutPort,InPort>::tearDown()
{
    inPort->stop();
    outPort->stop();

    ExtendedCF::UsesConnectionSequence_var connections = outPort->connections();
    for (CORBA::ULong index = 0; index < connections->length(); ++index) {
        outPort->disconnectPort(connections[index].connectionId);
    }

    for (typename std::vector<InPort*>::iterator servant = servants.begin(); servant != servants.end(); ++servant) {
        _deactivatePort(*servant);
        (*servant)->_remove_ref();
    }

    delete outPort;
}

template <class OutPort,class InPort>
void LocalTest<OutPort,InPort>::_activatePort(InPort* port)
{
    PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(port);
    servants.push_back(port);
}

template <class OutPort,class InPort>
void LocalTest<OutPort,InPort>::_deactivatePort(InPort* port)
{
    try {
        PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->servant_to_id(port);
        ossie::corba::RootPOA()->deactivate_object(oid);
    } catch (...) {
        // Ignore CORBA exceptions
    }
}

template <class OutPort,class InPort>
void LocalTest<OutPort,InPort>::testPushBurst()
{
    // Queue up a bunch of bursts
    const size_t BURST_COUNT = 16;
    for (size_t ii = 0; ii < BURST_COUNT; ++ii) {
        BurstType burst;
        burst.SRI = burstio::utils::createSRI("test_stream");
        burst.data.length(50);
        std::fill(burst.data.get_buffer(), burst.data.get_buffer() + burst.data.length(), ii);
        burst.T = burstio::utils::now();
        burst.EOS = false;
        outPort->pushBurst(burst);
    }

    // Force the output port to send the bursts
    outPort->flush();

    // Read the bursts one at a time and check that they look reasonable
    for (size_t ii = 0; ii < BURST_COUNT; ++ii) {
        boost::scoped_ptr<PacketType> burst(inPort->getBurst(0.0));
        CPPUNIT_ASSERT(burst);
        CPPUNIT_ASSERT_EQUAL(std::string("test_stream"), burst->getStreamID());
        CPPUNIT_ASSERT_EQUAL((size_t) 50, burst->getSize());
        CPPUNIT_ASSERT(*(burst->getData()) == ii);
    }
}

template <class OutPort,class InPort>
void LocalTest<OutPort,InPort>::testPushBursts()
{
    // Build up a sequence of bursts
    BurstSequenceType bursts;
    for (size_t ii = 0; ii < 24; ++ii) {
        BurstType burst;
        burst.SRI = burstio::utils::createSRI("test_stream");
        burst.data.length(100);
        std::fill(burst.data.get_buffer(), burst.data.get_buffer() + burst.data.length(), ii);
        burst.T = burstio::utils::now();
        burst.EOS = false;
        ossie::corba::push_back(bursts, burst);
    }

    // Push the entire sequence (skips buffering) and make sure the other port
    // didn't steal the bursts
    outPort->pushBursts(bursts);
    CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 24, bursts.length());

    // Read the bursts (in bulk) and check them against the originals
    BurstSequenceVar results = inPort->getBursts(0.0);
    CPPUNIT_ASSERT_EQUAL(bursts.length(), results->length());
    for (CORBA::ULong ii = 0; ii < results->length(); ++ii) {
        CPPUNIT_ASSERT_EQUAL(std::string("test_stream"), std::string(results[ii].SRI.streamID));
        CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 100, results[ii].data.length());
        CPPUNIT_ASSERT(bursts[ii].data[0] == ii);
        CPPUNIT_ASSERT_EQUAL(bursts[ii].T, results[ii].T);
        CPPUNIT_ASSERT(!results[ii].EOS);
    }
}

template <class OutPort,class InPort>
void LocalTest<OutPort,InPort>::testFanOut()
{
    // Create a second input port to check 1:2 fan out
    InPort* inPort2 = new InPort("burst" + getPortName() + "_in_2");
    inPort2->setLogger(rh_logger::Logger::getLogger(rootLogger->getName() + "." + inPort2->getName()));
    _activatePort(inPort2);
    inPort2->start();

    CORBA::Object_var objref = inPort2->_this();
    outPort->connectPort(objref, "local_connection_2");

    // Build up a sequence of bursts
    BurstSequenceType bursts;
    for (size_t ii = 0; ii < 24; ++ii) {
        BurstType burst;
        burst.SRI = burstio::utils::createSRI("test_stream");
        burst.data.length(100);
        std::fill(burst.data.get_buffer(), burst.data.get_buffer() + burst.data.length(), ii);
        burst.T = burstio::utils::now();
        burst.EOS = false;
        ossie::corba::push_back(bursts, burst);
    }

    // Push the entire sequence (skips buffering) and make sure the other ports
    // didn't steal the bursts
    outPort->pushBursts(bursts);
    CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 24, bursts.length());

    // Read the bursts (in bulk) and check them against the originals
    BurstSequenceVar results = inPort->getBursts(0.0);
    CPPUNIT_ASSERT_EQUAL(bursts.length(), results->length());
    for (CORBA::ULong ii = 0; ii < results->length(); ++ii) {
        CPPUNIT_ASSERT_EQUAL(std::string("test_stream"), std::string(results[ii].SRI.streamID));
        CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 100, results[ii].data.length());
        CPPUNIT_ASSERT(bursts[ii].data[0] == ii);
        CPPUNIT_ASSERT_EQUAL(bursts[ii].T, results[ii].T);
        CPPUNIT_ASSERT(!results[ii].EOS);
    }

    // Repeat with the second port; the results should be the same
    BurstSequenceVar results2 = inPort2->getBursts(0.0);
    CPPUNIT_ASSERT_EQUAL(bursts.length(), results2->length());
    for (CORBA::ULong ii = 0; ii < results2->length(); ++ii) {
        CPPUNIT_ASSERT_EQUAL(std::string("test_stream"), std::string(results2[ii].SRI.streamID));
        CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 100, results2[ii].data.length());
        CPPUNIT_ASSERT(bursts[ii].data[0] == ii);
        CPPUNIT_ASSERT_EQUAL(bursts[ii].T, results2[ii].T);
        CPPUNIT_ASSERT(!results2[ii].EOS);
    }
}

#define CREATE_TEST(x)                                                  \
    class Local##x##Test : public LocalTest<burstio::Burst##x##Out,burstio::Burst##x##In> \
    {                                                                   \
        typedef LocalTest<burstio::Burst##x##Out,burstio::Burst##x##In> TestBase; \
        CPPUNIT_TEST_SUB_SUITE(Local##x##Test, TestBase);               \
        CPPUNIT_TEST_SUITE_END();                                       \
        virtual std::string getPortName() const { return #x; };         \
    };                                                                  \
    CPPUNIT_TEST_SUITE_REGISTRATION(Local##x##Test);

CREATE_TEST(Byte);
CREATE_TEST(Ubyte);
CREATE_TEST(Short);
CREATE_TEST(Ushort);
CREATE_TEST(Long);
CREATE_TEST(Ulong);
CREATE_TEST(LongLong);
CREATE_TEST(UlongLong);
CREATE_TEST(Float);
CREATE_TEST(Double);
