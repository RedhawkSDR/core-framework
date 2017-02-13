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
    bool overlaps(const redhawk::shared_buffer<T>& lhs, const redhawk::shared_buffer<T>& rhs)
    {
        const T* end;
        const T* ptr;
        if (lhs.data() < rhs.data()) {
            end = lhs.data() + lhs.size();
            ptr = rhs.data();
        } else {
            end = rhs.data() + rhs.size();
            ptr = lhs.data();
        }
        return (ptr < end);
    }
}

template <class OutPort, class InPort>
void LocalTest<OutPort,InPort>::setUp()
{
    std::string name = getPortName();
    outPort = new OutPort("data" + name + "_out");
    inPort = new InPort("data" + name + "_in");

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
    redhawk::buffer<ScalarType> data(1024);
    out_stream.write(data, bulkio::time::utils::now());

    // The corresponding input stream should exist and have data
    InStreamType in_stream = inPort->getStream("test_stream");
    CPPUNIT_ASSERT(in_stream);
    DataBlockType block = in_stream.tryread();
    CPPUNIT_ASSERT(block);

    // Check that the input stream is sharing the underlying memory
    redhawk::shared_buffer<ScalarType> result = block.buffer();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Input stream received a copy of data", (const ScalarType*) data.data(), result.data());
    CPPUNIT_ASSERT_EQUAL(data.size(), result.size());
}

template <class OutPort, class InPort>
void LocalTest<OutPort,InPort>::testLargeWrite()
{
    // Create an output stream and write a buffer that is too large for a
    // single CORBA transfer
    OutStreamType out_stream = outPort->createStream("test_stream");
    size_t count = (2 * bulkio::Const::MaxTransferBytes()) / sizeof(ScalarType);
    redhawk::buffer<ScalarType> data(count);
    out_stream.write(data, bulkio::time::utils::now());

    // The corresponding input stream should exist and have data
    InStreamType in_stream = inPort->getStream("test_stream");
    CPPUNIT_ASSERT(in_stream);
    DataBlockType block = in_stream.tryread();
    CPPUNIT_ASSERT(block);

    // Make sure that the original buffer was preserved as a single transfer
    redhawk::shared_buffer<ScalarType> result = block.buffer();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Input stream received a copy of data", (const ScalarType*) data.data(), result.data());
    CPPUNIT_ASSERT_EQUAL(data.size(), result.size());
}

template <class OutPort, class InPort>
void LocalTest<OutPort,InPort>::testReadSlice()
{
    // Create an output stream and write a buffer to it
    OutStreamType out_stream = outPort->createStream("test_stream");
    redhawk::buffer<ScalarType> data(1024);
    out_stream.write(data, bulkio::time::utils::now());
    const ScalarType* start_pointer = data.data();

    // The corresponding input stream should exist and have data
    const size_t READ_SIZE = 500;
    InStreamType in_stream = inPort->getStream("test_stream");
    CPPUNIT_ASSERT(in_stream);
    DataBlockType block = in_stream.tryread(READ_SIZE);
    CPPUNIT_ASSERT(block);

    // Check that the read buffer is a subset of the original buffer
    redhawk::shared_buffer<ScalarType> result = block.buffer();
    CPPUNIT_ASSERT_EQUAL(READ_SIZE, result.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Input stream received a copy of data", start_pointer, result.data());

    // The next read buffer should point to an offset into the original buffer
    block = in_stream.tryread(READ_SIZE);
    CPPUNIT_ASSERT(block);
    result = block.buffer();
    CPPUNIT_ASSERT_EQUAL(READ_SIZE, result.size());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Input stream received a copy of data", start_pointer + READ_SIZE, result.data());

    // Write a new buffer (copy allocates a new memory block)
    redhawk::shared_buffer<ScalarType> data2 = data.copy();
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
        virtual std::string getPortName() const { return #x; };         \
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
