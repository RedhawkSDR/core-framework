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

#include "DataBlockTest.h"
#include "bulkio.h"

using bulkio::StringDataBlock;
using bulkio::BitDataBlock;

template <class Block>
Block DataBlockTest<Block>::_createBasicBlock(size_t size, const std::string& streamID)
{
    // The default template implementation assumes that the block has a nested
    // ScalarType typedef, and expects a redhawk::shared_buffer of that type
    // (i.e., it's really a SampleDataBlock).
    BULKIO::StreamSRI sri = bulkio::sri::create(streamID);
    typedef typename Block::ScalarType ScalarType;
    redhawk::buffer<ScalarType> buffer(size, std::allocator<ScalarType>());
    return Block(sri, buffer);
}

// Specialization for creating a StringDataBlock
template <>
StringDataBlock DataBlockTest<StringDataBlock>::_createBasicBlock(size_t size, const std::string& streamID)
{
    BULKIO::StreamSRI sri = bulkio::sri::create(streamID);
    std::string buffer(' ', size);
    return StringDataBlock(sri, buffer);
}

// Specialization for creating a BitDataBlock
template <>
BitDataBlock DataBlockTest<BitDataBlock>::_createBasicBlock(size_t size, const std::string& streamID)
{
    BULKIO::StreamSRI sri = bulkio::sri::create(streamID);
    redhawk::bitbuffer buffer(size);
    return BitDataBlock(sri, buffer);
}

template <class Block>
void DataBlockTest<Block>::testCopy()
{
    Block block = _createBasicBlock(16, "test_copy");

    Block copy = block.copy();
    if (block.buffer() != copy.buffer()) {
        CPPUNIT_FAIL("Copy produced unequal buffers");
    }
}

#define CREATE_TEST(X, BASE)                                            \
    class X##Test : public BASE<bulkio::X>                              \
    {                                                                   \
        CPPUNIT_TEST_SUB_SUITE(X##Test, BASE<bulkio::X>);               \
        CPPUNIT_TEST_SUITE_END();                                       \
    };                                                                  \
    CPPUNIT_TEST_SUITE_REGISTRATION(X##Test);

#define CREATE_BASIC_TEST(x) CREATE_TEST(x##DataBlock, DataBlockTest)
#define CREATE_NUMERIC_TEST(x) CREATE_TEST(x##DataBlock, SampleDataBlockTest)

CREATE_BASIC_TEST(String);
CREATE_BASIC_TEST(Bit);
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
