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

#include "TestCppProps.h"
#include <ossie/prop_helpers.h>

PREPARE_LOGGING(TestCppProps);

static TestCppProps* servant = 0;

static void prop_changed (const std::string& id)
{
    if (servant) servant->propertyChanged(id);
}

TestCppProps::TestCppProps(const char *uuid, const char *label) :
    Resource_impl(uuid, label),
    testCallbackResults()
{
    addProperty(prop_long,
                0,
                "DCE:9d1e3621-27ca-4cd0-909d-90b7448b8f71",
                "prop_long",
                "readwrite",
                "null",
                "external",
                "configure");

    addProperty(prop_str,
                "default value",
                "DCE:4e7c1977-5f53-4061-bae7-cb8c1072f4b7",
                "prop_str",
                "readwrite",
                "null",
                "external",
                "configure");

    addProperty(simple_utctime,
                "2017:2:1::12:01:00.123",
                "simple_utctime",
                "",
                "readwrite",
                "",
                "external",
                "property");

    seq_utctime.push_back(redhawk::time::utils::convert("2010:2:1::12:01:00.123"));
    seq_utctime.push_back(redhawk::time::utils::convert("2011:2:1::12:01:00.123"));
    addProperty(seq_utctime,
                seq_utctime,
                "seq_utctime",
                "",
                "readwrite",
                "",
                "external",
                "property");
    
    addProperty(reset_utctime,
                "false",
                "reset_utctime",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(seq_oct,
                "DCE:f877b9ee-a682-43a6-ba21-5ea980167f55",
                "seq_oct",
                "readwrite",
                "null",
                "external",
                "configure");
    seq_oct.resize(4);

    addProperty(seq_foo,
                "DCE:e415de60-68cb-403e-812e-76f114731bb4",
                "prop_foo",
                "readwrite",
                "",
                "",
                "configure");
    seq_foo.resize(2);
    seq_foo[0].item_bool = false;
    seq_foo[0].item_string = "First";
    seq_foo[1].item_bool = true;
    seq_foo[1].item_string = "Second";

    addProperty(readOnly,
                "empty",
                "readOnly",
                "readOnly",
                "readonly",
                "",
                "external",
                "property");
    addPropertyListener(reset_utctime, this, &TestCppProps::resetUTCtime);
}

TestCppProps::~TestCppProps (void)
{
}

void TestCppProps::resetUTCtime(bool oldValue, bool newValue)
{
    if (newValue) {
        this->simple_utctime = redhawk::time::utils::create();
    }
}

void TestCppProps::runTest (CORBA::ULong testId, CF::Properties& testValues)
    throw (CF::UnknownProperties, CF::TestableObject::UnknownTest, CORBA::SystemException)
{
    switch (testId) {
    case TEST_MEMBER_CALLBACKS:
        testMemberCallbacks(testValues);
        break;
    case TEST_STATIC_CALLBACKS:
        testStaticCallbacks(testValues);
        break;
    case TEST_ENABLE_NIL_PROPERTY:
        testEnableNil(testValues);
        break;
    case TEST_SET_NIL_PROPERTY:
        testSetNil(testValues);
        break;
    default:
        throw CF::TestableObject::UnknownTest();
        break;
    }
}

void TestCppProps::propertyChanged (const std::string& id)
{
    // Mark the property as changed.
    PropertyInterface* property = getPropertyFromId(id);
    testCallbackResults.insert(id);
}

void TestCppProps::testMemberCallbacks (CF::Properties& testValues)
{
    // Set up notification to call `this->propertyChanged()` when any of the
    // test values are changed via `configure()`.
    LOG_DEBUG(TestCppProps, "Setting up " << testValues.length() << " member function callback(s)");
    for (CORBA::ULong ii = 0; ii < testValues.length(); ++ii) {
        const std::string id = static_cast<const char*>(testValues[ii].id);
        setPropertyChangeListener(id, this, &TestCppProps::propertyChanged);
    }
    
    testCallbacks(testValues);
}

void TestCppProps::testStaticCallbacks (CF::Properties& testValues)
{
    // Set up notification to call static function `prop_changed()` when any of the
    // test values are changed via `configure()`.
    LOG_DEBUG(TestCppProps, "Setting up " << testValues.length() << " static function callback(s)");
    servant = this;
    for (CORBA::ULong ii = 0; ii < testValues.length(); ++ii) {
        const std::string id = static_cast<const char*>(testValues[ii].id);
        setPropertyChangeListener(id, prop_changed);
    }

    testCallbacks(testValues);
}

void TestCppProps::testCallbacks (CF::Properties& testValues)
{
    // Check whether the property change callbacks are executed for the properties
    // given in 'testValues'. The values in 'testValues' are updated to a boolean
    // that indicates if a callback occurred.

    // Reset the results to prevent false positives.
    testCallbackResults.clear();
    LOG_DEBUG(TestCppProps, "Calling configure with test values");
    configure(testValues);
    for (CORBA::ULong ii = 0; ii < testValues.length(); ++ii) {
        const std::string id = static_cast<const char*>(testValues[ii].id);
        if (testCallbackResults.count(id)) {
            LOG_DEBUG(TestCppProps, "Callback was triggered for property " << id);
            testValues[ii].value <<= true;
        } else {
            LOG_DEBUG(TestCppProps, "Callback was NOT triggered for property " << id);
            testValues[ii].value <<= false;
        }
    }
}

void TestCppProps::testEnableNil (CF::Properties& testValues)
{
    for (CORBA::ULong ii = 0; ii < testValues.length(); ++ii) {
        const std::string id = static_cast<const char*>(testValues[ii].id);
        bool enable;
        if (testValues[ii].value >>= enable) {
            PropertyInterface* property = getPropertyFromId(id);
            if (property) {
                LOG_DEBUG(TestCppProps, "Property " << id << " nil values " << (enable?"enabled":"disabled"));
                property->enableNil(enable);
            } else {
                LOG_WARN(TestCppProps, "No property " << id << " found");
            }
        } else {
            LOG_WARN(TestCppProps, "Unable to get 'enabled' value for property " << id);
        }
    }
}

void TestCppProps::testSetNil (CF::Properties& testValues)
{
    for (CORBA::ULong ii = 0; ii < testValues.length(); ++ii) {
        const std::string id = static_cast<const char*>(testValues[ii].id);
        bool isNil;
        if (testValues[ii].value >>= isNil) {
            PropertyInterface* property = getPropertyFromId(id);
            if (property) {
                LOG_DEBUG(TestCppProps, "Property " << id << " set to " << (isNil?"nil":"non-nil"));
                property->isNil(isNil);
            } else {
                LOG_WARN(TestCppProps, "No property " << id << " found");
            }
        } else {
            LOG_WARN(TestCppProps, "Unable to get 'isNil' value for property " << id);
        }
    }
}
