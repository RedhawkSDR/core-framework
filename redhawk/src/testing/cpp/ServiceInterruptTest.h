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

#ifndef SERVICEINTERRUPTTEST_H
#define SERVICEINTERRUPTTEST_H

#include "CFTest.h"
#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

class svc_stuck_cpp_base : public Component, protected ThreadedComponent
{
    public:
        svc_stuck_cpp_base(const char *uuid, const char *label);
        ~svc_stuck_cpp_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();
        int serviceFunction();

    protected:

    private:
};

class ServiceInterruptTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(ServiceInterruptTest);
    CPPUNIT_TEST(testInterruption);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testInterruption();
    
    svc_stuck_cpp_base *my_comp;
};

#endif // SERVICEINTERRUPTTEST_H
