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
#ifndef BULKIO_OUTPORTTEST_H
#define BULKIO_OUTPORTTEST_H

#include "OutPortTestFixture.h"

template <class Port>
class OutPortTest : public OutPortTestFixture<Port>
{
    typedef OutPortTestFixture<Port> TestBase;

    CPPUNIT_TEST_SUITE(OutPortTest);
    CPPUNIT_TEST(testLegacyAPI);
    CPPUNIT_TEST(testConnections);
    CPPUNIT_TEST(testStatistics);
    CPPUNIT_TEST(testMultiOut);
    CPPUNIT_TEST_SUITE_END();

public:
    void testLegacyAPI();
    void testConnections();
    void testStatistics();
    void testMultiOut();

protected:
    typedef typename TestBase::StubType StubType;
    typedef typename Port::CorbaType CorbaType;

    static const size_t BITS_PER_ELEMENT;

    void _addStreamFilter(const std::string& streamId, const std::string& connectionId);

    using TestBase::port;
    using TestBase::stub;

    std::vector<bulkio::connection_descriptor_struct> connectionTable;
};

template <class Port>
class ChunkingOutPortTest : public OutPortTest<Port>
{
    typedef OutPortTest<Port> TestBase;

    CPPUNIT_TEST_SUB_SUITE(ChunkingOutPortTest, TestBase);
    CPPUNIT_TEST(testPushChunking);
    CPPUNIT_TEST(testPushChunkingEOS);
    CPPUNIT_TEST(testPushChunkingSubsize);
    CPPUNIT_TEST_SUITE_END();

public:
    void testPushChunking();
    void testPushChunkingEOS();
    void testPushChunkingSubsize();

protected:
    typedef typename Port::CorbaType CorbaType;

    void _testPushOversizedPacket(const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamId);

    using TestBase::BITS_PER_ELEMENT;
    using TestBase::port;
    using TestBase::stub;
};

template <class Port>
class NumericOutPortTest : public ChunkingOutPortTest<Port>
{
    typedef ChunkingOutPortTest<Port> TestBase;

    CPPUNIT_TEST_SUB_SUITE(NumericOutPortTest, TestBase);
    CPPUNIT_TEST(testPushPacketData);
    CPPUNIT_TEST(testPushChunkingComplex);
    CPPUNIT_TEST(testPushChunkingSubsizeComplex);
    CPPUNIT_TEST_SUITE_END();

public:
    void testPushPacketData();
    void testPushChunkingComplex();
    void testPushChunkingSubsizeComplex();

protected:
    typedef typename Port::NativeType NativeType;
    typedef typename Port::VectorType VectorType;
    typedef typename Port::CorbaType CorbaType;

    template <typename T>
    void _testPushPacketDataImpl();

    using TestBase::port;
    using TestBase::stub;
};

#endif // BULKIO_OUTPORTTEST_H
