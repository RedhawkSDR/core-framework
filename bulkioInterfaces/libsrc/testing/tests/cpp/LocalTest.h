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
#ifndef BULKIO_LOCALTEST_H
#define BULKIO_LOCALTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <ossie/debug.h>
#include <bulkio/bulkio_typetraits.h>

template <class OutPort, class InPort>
class LocalTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(LocalTest);
    CPPUNIT_TEST(testBasicWrite);
    CPPUNIT_TEST(testLargeWrite);
    CPPUNIT_TEST(testReadSlice);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testBasicWrite();
    void testLargeWrite();

    void testReadSlice();

protected:
    typedef typename OutPort::StreamType OutStreamType;
    typedef typename InPort::StreamType InStreamType;
    typedef typename InStreamType::DataBlockType DataBlockType;
    typedef typename OutPort::CorbaType CorbaType;
    typedef typename bulkio::BufferTraits<CorbaType>::BufferType BufferType;
    typedef typename bulkio::BufferTraits<CorbaType>::MutableBufferType MutableBufferType;

    OutPort* outPort;
    InPort* inPort;
};

#endif // BULKIO_LOCALTEST_H
