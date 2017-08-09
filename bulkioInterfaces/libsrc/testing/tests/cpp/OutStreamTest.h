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
#ifndef BULKIO_OUTSTREAMTEST_H
#define BULKIO_OUTSTREAMTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <ossie/debug.h>

#include "InPortStub.h"

template <class Port>
class OutStreamTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(OutStreamTest);
    CPPUNIT_TEST(testOperators);
    CPPUNIT_TEST(testBasicWrite);
    CPPUNIT_TEST(testSriFields);
    CPPUNIT_TEST(testSriUpdate);
    CPPUNIT_TEST(testKeywords);
    CPPUNIT_TEST(testSendEosOnClose);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testOperators();

    void testBasicWrite();

    void testSriFields();
    void testSriUpdate();

    void testKeywords();

    void testSendEosOnClose();

protected:
    typedef typename Port::StreamType StreamType;
    typedef typename Port::CorbaType CorbaType;

    virtual std::string getPortName() const = 0;
    void _writeSinglePacket(StreamType& stream, size_t size,
                            const BULKIO::PrecisionUTCTime& time=bulkio::time::utils::now());

    bool _checkLastTimestamp(const BULKIO::PrecisionUTCTime& time);

    Port* port;
    InPortStub<CorbaType>* stub;
};

template <class Port>
class BufferedOutStreamTest : public OutStreamTest<Port>
{
    typedef OutStreamTest<Port> TestBase;
    CPPUNIT_TEST_SUB_SUITE(BufferedOutStreamTest, TestBase);
    CPPUNIT_TEST(testBufferedWrite);
    CPPUNIT_TEST(testWriteSkipBuffer);
    CPPUNIT_TEST(testFlush);
    CPPUNIT_TEST(testFlushOnClose);
    CPPUNIT_TEST(testFlushOnSriChange);
    CPPUNIT_TEST(testFlushOnBufferSizeChange);
    CPPUNIT_TEST(testWriteTimestampsReal);
    CPPUNIT_TEST(testWriteTimestampsComplex);
    CPPUNIT_TEST(testWriteTimestampsMixed);
    CPPUNIT_TEST_SUITE_END();

public:
    void testBufferedWrite();
    void testWriteSkipBuffer();

    void testFlush();
    void testFlushOnClose();
    void testFlushOnSriChange();
    void testFlushOnBufferSizeChange();

    void testWriteTimestampsReal();
    void testWriteTimestampsComplex();
    void testWriteTimestampsMixed();

private:
    typedef typename Port::StreamType StreamType;
    typedef typename StreamType::ScalarType ScalarType;
    typedef typename StreamType::ComplexType ComplexType;

    void _writeTimestampsImpl(StreamType& stream, bool complexData);

    using TestBase::port;
    using TestBase::stub;
};

#endif  // BULKIO_OUTSTREAMTEST_H
