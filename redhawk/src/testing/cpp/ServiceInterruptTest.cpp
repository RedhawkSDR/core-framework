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

#include "ServiceInterruptTest.h"

#include <complex>
#include <cmath>

#include <boost/make_shared.hpp>

#include <ossie/callback.h>

CPPUNIT_TEST_SUITE_REGISTRATION(ServiceInterruptTest);

int svc_stuck_cpp_base::serviceFunction()
{
    boost::mutex dataBufferLock;
    boost::mutex::scoped_lock lock(dataBufferLock);
    boost::condition_variable my_wait;
    boost::system_time to_time  = boost::get_system_time() + boost::posix_time::seconds(1000);
    my_wait.timed_wait(lock, to_time);
    return NORMAL;
}

svc_stuck_cpp_base::svc_stuck_cpp_base(const char *uuid, const char *label) :
    Component(uuid, label),
    ThreadedComponent()
{
}

svc_stuck_cpp_base::~svc_stuck_cpp_base()
{
}
void svc_stuck_cpp_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Component::start();
    ThreadedComponent::startThread();
}
void svc_stuck_cpp_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Component::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}
void svc_stuck_cpp_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
    }

    Component::releaseObject();
}

void svc_stuck_cpp_base::loadProperties()
{
}

void ServiceInterruptTest::setUp()
{
    my_comp = new svc_stuck_cpp_base("hello", "hello");
}

void ServiceInterruptTest::tearDown()
{
    if (my_comp!=NULL) {
        delete my_comp;
    }
}

void ServiceInterruptTest::testInterruption()
{
    my_comp->start();
    CPPUNIT_ASSERT(my_comp->started());
    usleep(10000);
    my_comp->stop();
    CPPUNIT_ASSERT(!my_comp->started());
}
