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

Comp::Comp(const char* uuid, const char* label) :
    Component(uuid, label),
    ThreadedComponent(),
    isInDontReturn(false),
    returnVal(RetVal::NORMAL)
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
    if (returnVal == RetVal::FINISH) {
        return FINISH;
    }
    if (returnVal == RetVal::NOOP) {
        return NOOP;
    }
    if (returnVal == RetVal::DONT_RETURN) {
        isInDontReturn = true;
        double val = 10.0;
        while (true) { // run forever, but don't thread.sleep()
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
    Comp comp("uuid", "label");
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(!comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());

    comp.start();
    // _running is set by a different thread, so accommodate a delay.
    boost::system_time end_time = boost::get_system_time() + boost::posix_time::milliseconds(100);
    while (boost::get_system_time() < end_time) {
        if (comp.isRunning()) {
            break;
        }
        usleep(10);
    }
    CPPUNIT_ASSERT(comp.started());
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());
    CF::UTCTime ts1 = comp.getFinishedTime();
    CPPUNIT_ASSERT(ts1.tcstatus == 0);
    CPPUNIT_ASSERT(ts1.twsec == 0);
    CPPUNIT_ASSERT(ts1.tfsec == 0);

    comp.setReturnVal(RetVal::FINISH);
    // Wait for FINISH to be processed.
    end_time = boost::get_system_time() + boost::posix_time::milliseconds(100);
    while (boost::get_system_time() < end_time) {
        if (comp.isFinished() && !comp.isRunning()) {
            break;
        }
        usleep(10);
    }
    CPPUNIT_ASSERT(comp.isFinished());
    CPPUNIT_ASSERT(!comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());
    CF::UTCTime ts2 = comp.getFinishedTime();
    CPPUNIT_ASSERT(ts2.tcstatus == 1);
    CPPUNIT_ASSERT(ts2.twsec > 0);

    comp.stop();
    CPPUNIT_ASSERT(comp.isFinished());
    CPPUNIT_ASSERT(!comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());

    comp.setReturnVal(RetVal::NOOP);
    comp.start();
    // _running is set by a different thread, so accommodate a delay.
    end_time = boost::get_system_time() + boost::posix_time::milliseconds(100);
    while (boost::get_system_time() < end_time) {
        if (comp.isRunning()) {
            break;
        }
        usleep(10);
    }
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());
    CF::UTCTime ts3 = comp.getFinishedTime();
    CPPUNIT_ASSERT(ts3.tcstatus == 0);
    CPPUNIT_ASSERT(ts3.twsec == 0);
    CPPUNIT_ASSERT(ts3.tfsec == 0);

    comp.releaseObject();
}

void StatusTest::testStatusWithNormal()
{
    Comp comp("uuid", "label");
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(!comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());

    comp.start();
    // _running is set by a different thread, so accommodate a delay.
    boost::system_time end_time = boost::get_system_time() + boost::posix_time::milliseconds(100);
    while (boost::get_system_time() < end_time) {
        if (comp.isRunning()) {
            break;
        }
        usleep(10);
    }
    CPPUNIT_ASSERT(comp.started());
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());
    CF::UTCTime tsBefore = comp.getFinishedTime();
    CPPUNIT_ASSERT(tsBefore.tcstatus == 0);
    CPPUNIT_ASSERT(tsBefore.twsec == 0);
    CPPUNIT_ASSERT(tsBefore.tfsec == 0);

    comp.stop();
    // Wait for stop() to be processed.
    end_time = boost::get_system_time() + boost::posix_time::milliseconds(100);
    while (boost::get_system_time() < end_time) {
        if (comp.wasStopCalled() && !comp.isRunning()) {
            break;
        }
        usleep(10);
    }
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(!comp.isRunning());
    CPPUNIT_ASSERT(comp.wasStopCalled());
    CF::UTCTime tsAfter = comp.getFinishedTime();
    CPPUNIT_ASSERT(tsAfter.tcstatus == 0);
    CPPUNIT_ASSERT(tsAfter.twsec == 0);

    comp.start();
    // _running is set by a different thread, so accommodate a delay.
    end_time = boost::get_system_time() + boost::posix_time::milliseconds(100);
    while (boost::get_system_time() < end_time) {
        if (comp.isRunning()) {
            break;
        }
        usleep(10);
    }
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());

    comp.releaseObject();
}

void StatusTest::testStatusWithLongRunning()
{
    Comp comp("uuid", "label");
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(!comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());

    comp.start();
    // _running is set by a different thread, so accommodate a delay.
    boost::system_time end_time = boost::get_system_time() + boost::posix_time::milliseconds(100);
    while (boost::get_system_time() < end_time) {
        if (comp.isRunning()) {
            break;
        }
        usleep(10);
    }
    CPPUNIT_ASSERT(comp.started());
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(comp.isRunning());
    CPPUNIT_ASSERT(!comp.wasStopCalled());
    CF::UTCTime tsBefore = comp.getFinishedTime();
    CPPUNIT_ASSERT(tsBefore.tcstatus == 0);
    CPPUNIT_ASSERT(tsBefore.twsec == 0);
    CPPUNIT_ASSERT(tsBefore.tfsec == 0);

    comp.setReturnVal(RetVal::DONT_RETURN);
    // Wait for serviceFunction() to enter the DONT_RETURN case.
    end_time = boost::get_system_time() + boost::posix_time::milliseconds(100);
    while (boost::get_system_time() < end_time) {
        if (comp.isInDontReturn) {
            break;
        }
        usleep(10);
    }
    comp.stop();
    // Wait for stop() to be processed.
    end_time = boost::get_system_time() + boost::posix_time::milliseconds(2100);
    while (boost::get_system_time() < end_time) {
        if (comp.wasStopCalled() && !comp.isRunning()) {
            break;
        }
        usleep(10);
    }
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(comp.isRunning());
    CPPUNIT_ASSERT(comp.wasStopCalled());

    comp.start();
    CPPUNIT_ASSERT(!comp.isFinished());
    CPPUNIT_ASSERT(comp.isRunning());
    CPPUNIT_ASSERT(comp.wasStopCalled());

    comp.releaseObject();
}
