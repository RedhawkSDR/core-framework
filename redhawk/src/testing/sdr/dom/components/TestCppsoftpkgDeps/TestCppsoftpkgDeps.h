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

#ifndef TestCppsoftpkgDeps__H
#define TestCppsoftpkgDeps__H

#include <string>
#include <vector>
#include <set>

#include <ossie/Resource_impl.h>
#include <ossie/debug.h>
#include "struct_props.h"

class TestCppsoftpkgDeps : public Resource_impl
{
    ENABLE_LOGGING;

public:
    enum {
        TEST_MEMBER_CALLBACKS = 0,
        TEST_STATIC_CALLBACKS = 1
    };

    TestCppsoftpkgDeps (const char* uuid, const char* label);
    ~TestCppsoftpkgDeps (void);

    void runTest (CORBA::ULong testID, CF::Properties& testValues);

    void propertyChanged (const std::string&);

protected:
    void testMemberCallbacks (CF::Properties& values);
    void testStaticCallbacks (CF::Properties& values);
    void testCallbacks (CF::Properties& values);
 
    // Member variables exposed as properties
    CORBA::Long prop_long;
    std::string prop_str;
    std::vector<CORBA::Short> seq_oct;
    std::vector<prop_foo_struct> seq_foo;

    std::set<std::string> testCallbackResults;
};

#endif
