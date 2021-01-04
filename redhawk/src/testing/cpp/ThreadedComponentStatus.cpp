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

#include "ThreadedComponentStatus.h"

CPPUNIT_TEST_SUITE_REGISTRATION(StatusTest);

Comp::Comp(const char* uuid, const char* label, std::string behavior) :
    Component(uuid, label),
    ThreadedComponent(),
    mRanOnce(false),
    _behavior(behavior)
{
}

Comp::~Comp() { }

void Comp::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Component::start();
    ThreadedComponent::startThread();
}

void Comp::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Component::stop();
    if (!ThreadedComponent::stopThread()) {
        // User needs to add code here to deal with the case
        // in which the thread does not die.
        //throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }   
}

void Comp::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
    }   

    Component::releaseObject();
}

void Comp::loadProperties() {}

int Comp::serviceFunction()
{
    mRanOnce = true;
    if (_behavior.compare("return FINISH") == 0) {
        return FINISH;
    }
    if (_behavior.compare("return NOOP") == 0) {
        return NOOP;
    }
    if (_behavior.compare("long running") == 0) {
        double val = 10.0;
        while (true) {
            if (val < 999999.9) {
                val *= 11.0;
            } else {
                val /= 13.0;
            }
        }
    }
    return NORMAL;
}


// =============================================================================
//           TEST DRIVER
// =============================================================================
void StatusTest::setUp() {}

void StatusTest::tearDown() {}

void StatusTest::testStatusWithFinish()
{
    Comp comp("uuid", "label", "return FINISH");
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(!comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());

    comp.start();
    CPPUNIT_ASSERT(comp.started());
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());
    CF::UTCTime tsBefore = comp.getFinishedTime();
    CPPUNIT_ASSERT(tsBefore.tcstatus == 0);
    CPPUNIT_ASSERT(tsBefore.twsec == 0);
    CPPUNIT_ASSERT(tsBefore.tfsec == 0);

    // Wait for FINISH to be processed.
    usleep(10000);

    CPPUNIT_ASSERT(comp.isFinished());
    CPPUNIT_ASSERT(!comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());
    CF::UTCTime tsAfter = comp.getFinishedTime();
    CPPUNIT_ASSERT(tsAfter.tcstatus == 1);
    CPPUNIT_ASSERT(tsAfter.twsec > 0);

    comp.stop();
    CPPUNIT_ASSERT(comp.isFinished());
    CPPUNIT_ASSERT(!comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());

    comp.start();
    CPPUNIT_ASSERT(comp.isFinished());
    CPPUNIT_ASSERT(!comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());

    comp.releaseObject();
}

void StatusTest::testStatusWithNormal()
{
    Comp comp("uuid", "label", "return NORMAL");
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(!comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());

    comp.start();
    CPPUNIT_ASSERT(comp.started());
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());
    CF::UTCTime tsBefore = comp.getFinishedTime();
    CPPUNIT_ASSERT(tsBefore.tcstatus == 0);
    CPPUNIT_ASSERT(tsBefore.twsec == 0);
    CPPUNIT_ASSERT(tsBefore.tfsec == 0);

    while (!comp.mRanOnce) {
        usleep(10);
    }

    comp.stop();
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(!comp.isRunning());
    CPPUNIT_ASSERT(comp.wasStopCalled());
    CF::UTCTime tsAfter = comp.getFinishedTime();
    CPPUNIT_ASSERT(tsAfter.tcstatus == 0);
    CPPUNIT_ASSERT(tsAfter.twsec == 0);

    comp.start();
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());

    comp.releaseObject();
}

void StatusTest::testStatusWithLongRunning()
{
    Comp comp("uuid", "label", "long running");
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(!comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());

    comp.start();
    CPPUNIT_ASSERT(comp.started());
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());
    CF::UTCTime tsBefore = comp.getFinishedTime();
    CPPUNIT_ASSERT(tsBefore.tcstatus == 0);
    CPPUNIT_ASSERT(tsBefore.twsec == 0);
    CPPUNIT_ASSERT(tsBefore.tfsec == 0);

    while (!comp.mRanOnce) {
        usleep(10);
    }

    comp.stop();
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(comp.isRunning());
    CPPUNIT_ASSERT(comp.wasStopCalled());

    comp.start();
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(comp.isRunning());
    CPPUNIT_ASSERT(comp.wasStopCalled());

    comp.releaseObject();
}
