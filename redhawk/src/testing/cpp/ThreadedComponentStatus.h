/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

// This version uses a Comp class that descends from ThreadedComponent
//                   and a driver class to instantiate the Comp class

#ifndef THREADED_COMPONENT_STATUS_
#define THREADED_COMPONENT_STATUS_

#include "CFTest.h"
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include <boost/thread.hpp>

namespace RetVal {
    enum { FINISH, NOOP, NORMAL, DONT_RETURN };
}

class Comp : public Component, protected ThreadedComponent
{
    public:
        Comp(const char *uuid, const char *label);
        ~Comp();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();
        int serviceFunction();

        bool isFinished() { return ThreadedComponent::isFinished(); }
        bool isRunning() { return ThreadedComponent::isRunning(); }
        bool wasStopCalled() { return ThreadedComponent::wasStopCalled(); }
        CF::UTCTime getFinishedTime() { return ThreadedComponent::getFinishedTime(); }
        void setReturnVal(int val) { returnVal = val; }

        bool isInDontReturn;
        int returnVal;
};


class StatusTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(StatusTest);
    CPPUNIT_TEST(testStatusWithFinish);
    CPPUNIT_TEST(testStatusWithNormal);
    CPPUNIT_TEST(testStatusWithLongRunning);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testStatusWithFinish();
    void testStatusWithNormal();
    void testStatusWithLongRunning();
};

#endif // THREADED_COMPONENT_STATUS_
