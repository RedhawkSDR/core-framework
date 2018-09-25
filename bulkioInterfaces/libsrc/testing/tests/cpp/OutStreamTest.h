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
    CPPUNIT_TEST(testStreamWriteCheck);
    CPPUNIT_TEST(testWriteTimestampsReal);
    CPPUNIT_TEST(testWriteTimestampsComplex);
    CPPUNIT_TEST(testWriteTimestampsMixed);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testStreamWriteCheck();
    void testWriteTimestampsReal();
    void testWriteTimestampsComplex();
    void testWriteTimestampsMixed();

private:
    typedef typename Port::StreamType StreamType;
    typedef typename StreamType::ScalarType ScalarType;
    typedef typename StreamType::ComplexType ComplexType;

    typedef typename Port::Traits PortTraits;

    void _writeTimestampsImpl(StreamType& stream, bool complexData);

    virtual std::string getPortName() const = 0;

    Port* port;
    InPortStub<PortTraits>* stub;
};

#endif  // BULKIO_OUTSTREAMTEST_H
