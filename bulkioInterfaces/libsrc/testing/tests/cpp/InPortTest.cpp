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

#include "InPortTest.h"

class SriListener {
public:
    SriListener() :
        sri_(),
        sriChanged_(false)
    {
    }

    void updateSRI(BULKIO::StreamSRI& sri)
    {
        sri_ = sri;
        sriChanged_ = true;
    }

    void reset()
    {
        sriChanged_ = false;
    }

    bool sriChanged()
    {
        return sriChanged_;
    }

private:
    BULKIO::StreamSRI sri_;
    bool sriChanged_;
};

template <class Port>
void InPortTest<Port>::testBasicAPI()
{
    const char* stream_id = "basic_api";

    BULKIO::PortStatistics* stats = port->statistics();
    CPPUNIT_ASSERT(stats);
    delete stats;

    CPPUNIT_ASSERT(port->state() == BULKIO::IDLE);

    BULKIO::StreamSRISequence* streams = port->activeSRIs();
    CPPUNIT_ASSERT(streams);
    CPPUNIT_ASSERT(streams->length() == 0);
    delete streams;

    CPPUNIT_ASSERT(port->getMaxQueueDepth() == 100);
    CPPUNIT_ASSERT(port->getCurrentQueueDepth() == 0);

    port->setMaxQueueDepth(22);
    CPPUNIT_ASSERT(port->getMaxQueueDepth() == 22);

    // check that port queue is empty
    PacketType* packet = port->getPacket(bulkio::Const::NON_BLOCKING);
    CPPUNIT_ASSERT(!packet);

    BULKIO::StreamSRI sri = bulkio::sri::create(stream_id);
    port->pushSRI(sri);

    streams = port->activeSRIs();
    CPPUNIT_ASSERT(streams != NULL);
    CPPUNIT_ASSERT(streams->length() == 1);
    delete streams;

    this->_pushTestPacket(0, BULKIO::PrecisionUTCTime(), false, stream_id);

    // grab off packet
    packet = port->getPacket(bulkio::Const::NON_BLOCKING);
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(!packet->EOS);
    CPPUNIT_ASSERT(packet->SRI.mode == 0);
    delete packet;

    sri.mode = 1;
    port->pushSRI(sri);

    streams = port->activeSRIs();
    CPPUNIT_ASSERT(streams != NULL);
    CPPUNIT_ASSERT(streams->length() == 1);
    delete streams;

    this->_pushTestPacket(0, BULKIO::PrecisionUTCTime(), false, stream_id);

    // grab off packet
    packet = port->getPacket(bulkio::Const::NON_BLOCKING);
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(!(packet->EOS));
    CPPUNIT_ASSERT(packet->SRI.mode == 1);
    delete packet;

    // test for EOS..
    this->_pushTestPacket(0, BULKIO::PrecisionUTCTime(), true, stream_id);

    // grab off packet
    packet = port->getPacket(bulkio::Const::NON_BLOCKING);
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(packet->EOS);
    CPPUNIT_ASSERT(packet->SRI.mode == 1);
    delete packet;

    port->enableStats(false);

    port->block();

    port->unblock();
}

template <class Port>
void InPortTest<Port>::testSriChange()
{
    const char* stream_id = "invalid_stream";

    // Push data without an SRI to check that the sriChanged flag is still set
    // and the SRI callback gets called
    boost::scoped_ptr<typename Port::dataTransfer> packet;
    SriListener listener;
    port->setNewStreamListener(&listener, &SriListener::updateSRI);
    this->_pushTestPacket(0, BULKIO::PrecisionUTCTime(), false, stream_id);
    packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(packet->sriChanged);
    CPPUNIT_ASSERT(listener.sriChanged());

    // Push again to the same stream ID; sriChanged should now be false and the
    // SRI callback should not get called
    listener.reset();
    this->_pushTestPacket(0, BULKIO::PrecisionUTCTime(), false, stream_id);
    packet.reset(port->getPacket(bulkio::Const::NON_BLOCKING));
    CPPUNIT_ASSERT(packet);
    CPPUNIT_ASSERT(!(packet->sriChanged));
    CPPUNIT_ASSERT(!listener.sriChanged());
}

#define CREATE_TEST(x)                                                  \
    class In##x##PortTest : public InPortTest<bulkio::In##x##Port>      \
    {                                                                   \
        CPPUNIT_TEST_SUB_SUITE(In##x##PortTest, InPortTest<bulkio::In##x##Port>); \
        CPPUNIT_TEST_SUITE_END();                                       \
    };                                                                  \
    CPPUNIT_TEST_SUITE_REGISTRATION(In##x##PortTest);

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
CREATE_TEST(Bit);
CREATE_TEST(XML);
CREATE_TEST(File);
