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
#ifndef BULKIO_DATABLOCKTEST_H
#define BULKIO_DATABLOCKTEST_H

#include <cppunit/extensions/HelperMacros.h>

template <class Block>
class DataBlockTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(DataBlockTest);
    CPPUNIT_TEST(testCopy);
    CPPUNIT_TEST_SUITE_END();

public:
    void testCopy();

protected:
    Block _createBasicBlock(size_t size, const std::string& streamID);
};

template <class Block>
class SampleDataBlockTest : public DataBlockTest<Block>
{
    typedef DataBlockTest<Block> TestBase;
    CPPUNIT_TEST_SUB_SUITE(SampleDataBlockTest, TestBase);
    CPPUNIT_TEST_SUITE_END();
};

#endif // BULKIO_DATABLOCKTEST_H
