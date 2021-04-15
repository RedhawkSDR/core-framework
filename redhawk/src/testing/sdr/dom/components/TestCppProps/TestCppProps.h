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

#ifndef TESTCPPPROPS__H
#define TESTCPPPROPS__H

#include <string>
#include <vector>
#include <set>

#include <ossie/Resource_impl.h>
#include <ossie/debug.h>
#include "struct_props.h"

class TestCppProps : public Resource_impl
{
    ENABLE_LOGGING;

public:
    enum {
        TEST_MEMBER_CALLBACKS = 0,
        TEST_STATIC_CALLBACKS = 1,
        TEST_ENABLE_NIL_PROPERTY = 2,
        TEST_SET_NIL_PROPERTY = 3
    };

    TestCppProps (const char* uuid, const char* label);
    ~TestCppProps (void);

    void runTest (CORBA::ULong testID, CF::Properties& testValues);

    void propertyChanged (const std::string&);

protected:
    void testMemberCallbacks (CF::Properties& values);
    void testStaticCallbacks (CF::Properties& values);
    void testCallbacks (CF::Properties& values);
    void resetUTCtime(bool oldValue, bool newValue);

    void testEnableNil (CF::Properties& values);
    void testSetNil (CF::Properties& values);
 
    // Member variables exposed as properties
    CORBA::Long prop_long;
    std::string prop_str;
    CF::UTCTime simple_utctime;
    bool reset_utctime;
    std::vector<CF::UTCTime> seq_utctime;
    std::vector<CORBA::Short> seq_oct;
    std::vector<prop_foo_struct> seq_foo;

    std::set<std::string> testCallbackResults;

    /// Property: readOnly
    std::string readOnly;
};

#endif
