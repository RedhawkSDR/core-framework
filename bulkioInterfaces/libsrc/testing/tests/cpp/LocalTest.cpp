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

#include "LocalTest.h"
#include <bulkio/bulkio.h>

namespace {
    template <class T>
    bool overlaps(const T start1, const T end1, const T start2, const T end2)
    {
        return (start2 <= end1) && (start1 <= end2);
    }

    template <class T>
    bool contains(const T start1, const T end1, const T start2, const T end2)
    {
        return (start2 >= start1) && (end2 <= end1);
    }

    template <class T>
    bool overlaps(const redhawk::shared_buffer<T>& lhs, const redhawk::shared_buffer<T>& rhs)
    {
        return overlaps(lhs.data(), lhs.data() + lhs.size(), rhs.data(), rhs.data() + rhs.size());
    }

    template <class T>
    bool contains(const redhawk::shared_buffer<T>& outer, const redhawk::shared_buffer<T>& inner)
    {
        return contains(outer.data(), outer.data() + outer.size(), inner.data(), inner.data() + inner.size());
    }

    bool overlaps(const redhawk::shared_bitbuffer& lhs, const redhawk::shared_bitbuffer& rhs)
    {
        // Normalize the starts and ends to be relative to the lower of the two
        // base addresses, and in terms of bits
        const unsigned char* base = std::min(lhs.data(), rhs.data());
        size_t lstart = (lhs.data() - base) * 8 + lhs.offset();
        size_t rstart = (rhs.data() - base) * 8 + rhs.offset();
        return overlaps(lstart, lstart + lhs.size(), rstart, rstart + rhs.size());
    }

    bool contains(const redhawk::shared_bitbuffer& outer, const redhawk::shared_bitbuffer& inner)
    {
        // Normalize the starts and ends to be relative to the lower of the two
        // base addresses, and in terms of bits
        const unsigned char* base = std::min(outer.data(), inner.data());
        size_t ostart = (outer.data() - base) * 8 + outer.offset();
        size_t rstart = (inner.data() - base) * 8 + inner.offset();
        return contains(ostart, ostart + outer.size(), rstart, rstart + inner.size());
    }
}

template <class OutPort, class InPort>
void LocalTest<OutPort,InPort>::setUp()
{
    std::string name = bulkio::CorbaTraits<CorbaType>::name();
    outPort = new OutPort(name + "_out");
    inPort = new InPort(name + "_in");

    PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(inPort);

    CORBA::Object_var objref = inPort->_this();
    outPort->connectPort(objref, "local_connection");
}

template <class OutPort, class InPort>
void LocalTest<OutPort,InPort>::tearDown()
{
    outPort->disconnectPort("local_connection");

    try {
        PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->servant_to_id(inPort);
        ossie::corba::RootPOA()->deactivate_object(oid);
    } catch (...) {
        // Ignore CORBA exceptions
    }
    inPort->_remove_ref();

    delete outPort;
}

template <class OutPort, class InPort>
void LocalTest<OutPort,InPort>::testBasicWrite()
{
    // Create an output stream and write a buffer to it
    OutStreamType out_stream = outPort->createStream("test_stream");
    MutableBufferType data(1024);
    out_stream.write(data, bulkio::time::utils::now());

    // The corresponding input stream should exist and have data
    InStreamType in_stream = inPort->getStream("test_stream");
    CPPUNIT_ASSERT(in_stream);
    DataBlockType block = in_stream.tryread();
    CPPUNIT_ASSERT(block);

    // Check that the input stream is sharing the underlying memory
    BufferType result = block.buffer();
    CPPUNIT_ASSERT_MESSAGE("Input stream received a copy of data", data.data() == result.data());
    CPPUNIT_ASSERT_EQUAL(data.size(), result.size());
}

template <class OutPort, class InPort>
void LocalTest<OutPort,InPort>::testLargeWrite()
{
    // Create an output stream and write a buffer that is too large for a
    // single CORBA transfer
    OutStreamType out_stream = outPort->createStream("test_stream");
    size_t count = (16 * bulkio::Const::MaxTransferBytes()) / bulkio::NativeTraits<CorbaType>::bits;
    MutableBufferType data(count);
    out_stream.write(data, bulkio::time::utils::now());

    // The corresponding input stream should exist and have data
    InStreamType in_stream = inPort->getStream("test_stream");
    CPPUNIT_ASSERT(in_stream);
    DataBlockType block = in_stream.tryread();
    CPPUNIT_ASSERT(block);

    // Make sure that the original buffer was preserved as a single transfer
    BufferType result = block.buffer();
    CPPUNIT_ASSERT_MESSAGE("Input stream received a copy of data", data.data() == result.data());
    CPPUNIT_ASSERT_EQUAL(data.size(), result.size());
}

template <class OutPort, class InPort>
void LocalTest<OutPort,InPort>::testReadSlice()
{
    // Create an output stream and write a buffer to it
    OutStreamType out_stream = outPort->createStream("test_stream");
    MutableBufferType data(1024);
    out_stream.write(data, bulkio::time::utils::now());

    // The corresponding input stream should exist and have data
    const size_t READ_SIZE = 500;
    InStreamType in_stream = inPort->getStream("test_stream");
    CPPUNIT_ASSERT(in_stream);
    DataBlockType block = in_stream.tryread(READ_SIZE);
    CPPUNIT_ASSERT(block);

    // Check that the read buffer is a subset of the original buffer
    BufferType result = block.buffer();
    CPPUNIT_ASSERT_EQUAL(READ_SIZE, result.size());
    CPPUNIT_ASSERT_MESSAGE("Input stream received a copy of data", data.data() == result.data());

    // The next read buffer should point to an offset into the original buffer
    block = in_stream.tryread(READ_SIZE);
    CPPUNIT_ASSERT(block);
    result = block.buffer();
    CPPUNIT_ASSERT_EQUAL(READ_SIZE, result.size());
    CPPUNIT_ASSERT_MESSAGE("Input stream received a copy of data", contains(data, result));
    CPPUNIT_ASSERT_MESSAGE("Input buffer did not advance", data.data() < result.data());

    // Write a new buffer (copy allocates a new memory block)
    BufferType data2 = data.copy();
    out_stream.write(data2, bulkio::time::utils::now());

    // Read a buffer that we know spans two input buffers; it should still be
    // able to read the full amount, but it'll have to make a copy
    block = in_stream.tryread(READ_SIZE);
    CPPUNIT_ASSERT(block);
    result = block.buffer();
    CPPUNIT_ASSERT_EQUAL(READ_SIZE, result.size());
    CPPUNIT_ASSERT(!overlaps(data, result));
    CPPUNIT_ASSERT(!overlaps(data2, result));
}

#define CREATE_TEST(x)                                                  \
    class Local##x##Test : public LocalTest<bulkio::Out##x##Port,bulkio::In##x##Port> \
    {                                                                   \
        typedef LocalTest<bulkio::Out##x##Port,bulkio::In##x##Port> TestBase; \
        CPPUNIT_TEST_SUB_SUITE(Local##x##Test, TestBase);               \
        CPPUNIT_TEST_SUITE_END();                                       \
    };                                                                  \
    CPPUNIT_TEST_SUITE_REGISTRATION(Local##x##Test);

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
