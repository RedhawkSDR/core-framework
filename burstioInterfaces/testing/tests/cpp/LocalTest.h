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
#ifndef BURSTIO_LOCALTEST_H
#define BURSTIO_LOCALTEST_H

#include <vector>

#include <cppunit/extensions/HelperMacros.h>
#include <ossie/debug.h>

template <class OutPort, class InPort>
class LocalTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(LocalTest);
    CPPUNIT_TEST(testPushBurst);
    CPPUNIT_TEST(testPushBursts);
    CPPUNIT_TEST(testFanOut);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testPushBurst();
    void testPushBursts();
    void testFanOut();

protected:
    typedef typename OutPort::BurstType BurstType;
    typedef typename OutPort::BurstSequenceType BurstSequenceType;
    typedef typename InPort::PacketType PacketType;
    typedef typename InPort::BurstSequenceVar BurstSequenceVar;

    virtual std::string getPortName() const = 0;

    OutPort* outPort;
    InPort* inPort;

    void _activatePort(InPort* port);
    void _deactivatePort(InPort* port);

    std::vector<InPort*> servants;

    rh_logger::LoggerPtr rootLogger;
}; 

#endif  // BURSTIO_LOCALTEST_H
