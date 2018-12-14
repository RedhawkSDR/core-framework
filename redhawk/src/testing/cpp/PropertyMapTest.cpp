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

#include "PropertyMapTest.h"

#include <sstream>

#include <ossie/PropertyMap.h>

CPPUNIT_TEST_SUITE_REGISTRATION(PropertyMapTest);

namespace {
    redhawk::PropertyMap generate_test_data()
    {
        redhawk::PropertyMap propmap;
        propmap["first"] = (short)123;
        propmap["second"] = "abc";
        propmap["third"] = 5.25;
        return propmap;
    }

    redhawk::PropertyMap generate_test_sequence(size_t count)
    {
        redhawk::PropertyMap propmap;
        for (size_t index = 0; index < count; ++index) {
            std::ostringstream key;
            key << "prop_" << index;
            propmap[key.str()] = (CORBA::Long) index;
        }
        return propmap;
    }
}

void PropertyMapTest::setUp()
{
}

void PropertyMapTest::tearDown()
{
}

void PropertyMapTest::testDefaultConstructor()
{
    // Default constructor should create an empty PropertyMap
    redhawk::PropertyMap propmap;
    CPPUNIT_ASSERT(propmap.empty());
    CPPUNIT_ASSERT_EQUAL((size_t) 0, propmap.size());
}

void PropertyMapTest::testPropertiesConstructor()
{
    // Copy constructor from CF::Properties
    CF::Properties properties;
    properties.length(2);
    redhawk::PropertyMap propmap(properties);
    CPPUNIT_ASSERT(!propmap.empty());
    CPPUNIT_ASSERT_EQUAL((size_t) properties.length(), propmap.size());
}

void PropertyMapTest::testPropertyTypeFromAny()
{
    // Due to the implicit conversion from the templatized constructor for
    // Value (the explicit keyword was removed for 2.1), it was necessary to
    // add a constructor to PropertyType that takes a CORBA::Any as the value
    // argument to prevent accidental nesting of Anys; this test simply ensures
    // that this works
    CORBA::Any any;
    any <<= CF::Properties();
    redhawk::PropertyType prop("test", any);
    CPPUNIT_ASSERT_EQUAL(redhawk::Value::TYPE_PROPERTIES, prop.getValue().getType());
}

void PropertyMapTest::testConstCast()
{
    // Create a known set of CF::Properties
    CF::Properties properties;
    properties.length(2);
    properties[0].id = "first";
    properties[0].value <<= "abc";
    properties[1].id = "second";
    properties[1].value <<= 1.0;

    // Create a const PropertyMap reference alias and check the values
    const CF::Properties& const_properties = properties;
    const redhawk::PropertyMap& propmap = redhawk::PropertyMap::cast(const_properties);
    CPPUNIT_ASSERT(!propmap.empty());
    CPPUNIT_ASSERT_EQUAL((size_t) properties.length(), propmap.size());
    CPPUNIT_ASSERT_EQUAL(std::string("first"), propmap[0].getId());
    CPPUNIT_ASSERT_EQUAL(std::string("abc"), propmap[0].getValue().toString());
    CPPUNIT_ASSERT_EQUAL(std::string("second"), propmap[1].getId());
    CPPUNIT_ASSERT_EQUAL(1.0, propmap[1].getValue().toDouble());

    // Append to the Properties and check that the change is reflected in the
    // PropertyMap
    properties.length(3);
    properties[2].id = "third";
    properties[2].value <<= false;
    CPPUNIT_ASSERT_EQUAL((size_t) properties.length(), propmap.size());
    CPPUNIT_ASSERT_EQUAL(std::string("third"), propmap[2].getId());
    CPPUNIT_ASSERT_EQUAL(false, propmap[2].getValue().toBoolean());
}

void PropertyMapTest::testCast()
{
    // Start with an empty CF::Properties
    CF::Properties properties;

    // Create a PropertyMap reference alias
    redhawk::PropertyMap& propmap = redhawk::PropertyMap::cast(properties);
    CPPUNIT_ASSERT(propmap.empty());

    // Add a boolean PropertyType to the end of the PropertyMap and check that
    // the change is reflected in the aliased Properties
    propmap["boolean"] = true;
    CPPUNIT_ASSERT_EQUAL((size_t) 1, propmap.size());
    CPPUNIT_ASSERT_EQUAL((size_t) properties.length(), propmap.size());
    CPPUNIT_ASSERT_EQUAL(std::string(properties[0].id), propmap[0].getId());
    bool result = false;
    CPPUNIT_ASSERT(properties[0].value >>= result);
    CPPUNIT_ASSERT_EQUAL(true, result);
}

void PropertyMapTest::testPushBack()
{
    redhawk::PropertyMap propmap;
    CPPUNIT_ASSERT(propmap.empty());

    // Push a raw CF::DataType; it should be the first property
    CF::DataType dt;
    dt.id = "dt";
    dt.value <<= "one";
    propmap.push_back(dt);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, propmap.size());
    CPPUNIT_ASSERT_EQUAL(std::string("dt"), propmap[0].getId());
    CPPUNIT_ASSERT_EQUAL(std::string("one"), propmap[0].getValue().toString());

    // Push a PropertyType; it should be the second property
    propmap.push_back(redhawk::PropertyType("test", (short)0));
    CPPUNIT_ASSERT_EQUAL((size_t) 2, propmap.size());
    CPPUNIT_ASSERT_EQUAL(std::string("test"), propmap[1].getId());
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) 0, propmap[1].getValue().toLong());
}

void PropertyMapTest::testContains()
{
    // Use a const PropertyMap with known test data; contains() is intended to
    // be const-friendly
    const redhawk::PropertyMap propmap = generate_test_data();
    CPPUNIT_ASSERT(propmap.contains("first"));
    CPPUNIT_ASSERT(propmap.contains("third"));
    CPPUNIT_ASSERT(!propmap.contains("fourth"));
}

void PropertyMapTest::testConstIndexing()
{
    // Generate sequential properties (propN=N) and make sure the indexed
    // values match up
    const redhawk::PropertyMap propmap = generate_test_sequence(8);
    CPPUNIT_ASSERT_EQUAL(std::string("prop_4"), propmap[4].getId());
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) 7, propmap[7].getValue().toLong());
}

void PropertyMapTest::testMutableIndexing()
{
    // Create Properties with sequential IDs/values and a PropertyMap reference
    // alias--this allows us to use the trusted CORBA sequence operator[] for
    // comparison
    CF::Properties properties = generate_test_sequence(8);
    redhawk::PropertyMap& propmap = redhawk::PropertyMap::cast(properties);

    // Basic indexing
    CPPUNIT_ASSERT_EQUAL(std::string("prop_4"), propmap[4].getId());
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) 7, propmap[7].getValue().toLong());

    // Overwrite a value by index
    propmap[3] = redhawk::PropertyType("overwrite", (CORBA::Long)-128);
    CPPUNIT_ASSERT_EQUAL(std::string("overwrite"), std::string(properties[3].id));
    CORBA::Long lval = 0;
    CPPUNIT_ASSERT(properties[3].value >>= lval);
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) -128, lval);

    // Modify ID of an indexed item
    propmap[5].setId("modified");
    CPPUNIT_ASSERT_EQUAL(std::string("modified"), std::string(properties[5].id));

    // Modiyf value of an indexed item
    propmap[7].setValue((CORBA::Double) 9.75);
    CORBA::Double dval = 0.0;
    CPPUNIT_ASSERT(properties[7].value >>= dval);
    CPPUNIT_ASSERT_EQUAL((CORBA::Double) 9.75, dval);
}

void PropertyMapTest::testConstMapping()
{
    const redhawk::PropertyMap propmap = generate_test_data();

    // Check for a known property value
    CPPUNIT_ASSERT_EQUAL(std::string("abc"), propmap["second"].toString());

    // IDs that are not found should throw an exception
    CPPUNIT_ASSERT_THROW(propmap["fourth"], std::invalid_argument);
}

void PropertyMapTest::testMutableMapping()
{
    // Use the standard test data, with the caveat that if for some reason its
    // size changes, or the property IDs are different than we expect, this
    // test will intentionally fail.
    redhawk::PropertyMap propmap = generate_test_data();
    CPPUNIT_ASSERT_EQUAL((size_t) 3, propmap.size());

    // Set a value for a new key, and check that it adds a new property to the
    // end of the map
    CPPUNIT_ASSERT(!propmap.contains("fourth"));
    propmap["fourth"] = true;
    CPPUNIT_ASSERT_EQUAL((size_t) 4, propmap.size());
    CPPUNIT_ASSERT_EQUAL(std::string("fourth"), propmap[3].getId());
    CPPUNIT_ASSERT_EQUAL(true, propmap[3].getValue().toBoolean());

    // Set a value for an existing key, and check that it overwrote the old
    // value
    CPPUNIT_ASSERT_EQUAL(std::string("second"), propmap[1].getId());
    propmap["second"] = (short)5000;
    CPPUNIT_ASSERT_EQUAL((size_t) 4, propmap.size());
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) 5000, propmap[1].getValue().toLong());

    // When the property isn't found, it should create one using the default
    // Value constructor
    CPPUNIT_ASSERT(!propmap.contains("nil"));
    CPPUNIT_ASSERT(propmap["nil"].isNil());
    CPPUNIT_ASSERT_EQUAL((size_t) 5, propmap.size());
}

void PropertyMapTest::testConstFind()
{
    const redhawk::PropertyMap propmap = generate_test_data();

    // If the key is not found, find() should return the end iterator
    CPPUNIT_ASSERT_EQUAL(propmap.end(), propmap.find("missing"));

    // With a known good key, make sure that find() returns a valid iterator,
    // to the correct property
    redhawk::PropertyMap::const_iterator prop = propmap.find("third");
    CPPUNIT_ASSERT(prop != propmap.end());
    CPPUNIT_ASSERT_EQUAL(std::string("third"), prop->getId());
    CPPUNIT_ASSERT_EQUAL((CORBA::Double) 5.25, prop->getValue().toDouble());
}

void PropertyMapTest::testMutableFind()
{
    redhawk::PropertyMap propmap = generate_test_data();

    // If the key is not found, find() should return the end iterator (this is
    // essentially identical to the const version)
    CPPUNIT_ASSERT_EQUAL(propmap.end(), propmap.find("bogus"));

    // Find a known good key, and use the iterator to overwrite the value
    redhawk::PropertyMap::iterator prop = propmap.find("second");
    CPPUNIT_ASSERT(prop != propmap.end());
    CPPUNIT_ASSERT_EQUAL(std::string("second"), prop->getId());
    prop->setValue("override");
    CPPUNIT_ASSERT_EQUAL(std::string("override"), propmap["second"].toString());
}

void PropertyMapTest::testConstIteration()
{
    const redhawk::PropertyMap empty;
    CPPUNIT_ASSERT_EQUAL(empty.begin(), empty.end());

    const redhawk::PropertyMap propmap = generate_test_sequence(8);
    CORBA::Long offset = 0;
    for (redhawk::PropertyMap::const_iterator iter = propmap.begin(); iter != propmap.end(); ++iter, ++offset) {
        CPPUNIT_ASSERT_EQUAL(offset, iter->getValue().toLong());
    }
    CPPUNIT_ASSERT_EQUAL((size_t) offset, propmap.size());
}

void PropertyMapTest::testMutableIteration()
{
    redhawk::PropertyMap empty;
    CPPUNIT_ASSERT_EQUAL(empty.begin(), empty.end());

    redhawk::PropertyMap propmap = generate_test_sequence(8);
    CORBA::Long offset = 0;
    for (redhawk::PropertyMap::iterator iter = propmap.begin(); iter != propmap.end(); ++iter, ++offset) {
        if (iter->getId() == "prop_6") {
            iter->setValue("override");
        }
    }
    CPPUNIT_ASSERT_EQUAL((size_t) offset, propmap.size());
    CPPUNIT_ASSERT_EQUAL(std::string("override"), propmap[6].getValue().toString());
}

void PropertyMapTest::testUpdate()
{
    // Start with a PropertyMap that partially intersects the standard test
    // properties ("second" is in both)
    redhawk::PropertyMap propmap;
    propmap["second"] = "default";
    propmap["fourth"] = true;

    // Update with the standard test properties
    const redhawk::PropertyMap overrides = generate_test_data();
    propmap.update(overrides);

    // Two properties should have been added ("first" and "third")
    CPPUNIT_ASSERT_EQUAL((size_t) 4, propmap.size());
    CPPUNIT_ASSERT(propmap.contains("first"));
    CPPUNIT_ASSERT_EQUAL(overrides["first"].toLong(), propmap["first"].toLong());
    CPPUNIT_ASSERT(propmap.contains("third"));

    // The common property "second" should be updated according to the standard
    // test properties
    CPPUNIT_ASSERT_EQUAL(overrides["second"].toString(), propmap["second"].toString());

    // The "fourth" property should remain unchanged
    CPPUNIT_ASSERT_EQUAL(true, propmap["fourth"].toBoolean());
}

void PropertyMapTest::testErase()
{
    // Use the sequential test data, because we're going to delete several
    // entries, and it's easier with predictable property names
    redhawk::PropertyMap propmap = generate_test_sequence(10);

    // Erase a single property by ID
    CPPUNIT_ASSERT(propmap.contains("prop_4"));
    propmap.erase("prop_4");
    CPPUNIT_ASSERT_EQUAL((size_t) 9, propmap.size());
    CPPUNIT_ASSERT(!propmap.contains("prop_4"));

    // Erase a single property by iterator
    redhawk::PropertyMap::iterator prop = propmap.find("prop_2");
    CPPUNIT_ASSERT(prop != propmap.end());
    propmap.erase(prop);
    CPPUNIT_ASSERT_EQUAL((size_t) 8, propmap.size());
    CPPUNIT_ASSERT(!propmap.contains("prop_2"));

    // Erase a range of properties; this should remove 3, 5 and 6
    redhawk::PropertyMap::iterator first = propmap.find("prop_3");
    redhawk::PropertyMap::iterator last = propmap.find("prop_7");
    propmap.erase(first, last);
    CPPUNIT_ASSERT_EQUAL((size_t) 5, propmap.size());

    // Check that the gap is where we expect it (from 1 to 7)
    prop = propmap.find("prop_1");
    prop++;
    CPPUNIT_ASSERT_EQUAL(std::string("prop_7"), prop->getId());
}

void PropertyMapTest::testGet()
{
    const redhawk::PropertyMap propmap = generate_test_data();

    // Property exists, default ignored
    CPPUNIT_ASSERT_EQUAL(std::string("abc"), propmap.get("second", "fail").toString());

    // Property exists, default returned
    CPPUNIT_ASSERT(!propmap.contains("missing"));
    CPPUNIT_ASSERT_EQUAL(std::string("pass"), propmap.get("missing", "pass").toString());
}

void PropertyMapTest::testToString()
{
    // Using the standard test properties, create a string representation; this
    // test shouldn't necessarily be considered canonical, but should at least
    // raise a red flag if the format changes inadvertently
    const redhawk::PropertyMap propmap = generate_test_data();
    const std::string stringval = propmap.toString();

    // The string representation should:
    // * be non-empty
    // * be enclosed in curly braces
    // * contain one "key=value" for each property
    CPPUNIT_ASSERT(!stringval.empty());
    CPPUNIT_ASSERT_EQUAL('{', *stringval.begin());
    CPPUNIT_ASSERT_EQUAL('}', *(stringval.end()-1));
    CPPUNIT_ASSERT(stringval.find("first") != std::string::npos);
    CPPUNIT_ASSERT(stringval.find("second") != std::string::npos);
    CPPUNIT_ASSERT(stringval.find("third") != std::string::npos);
    size_t item_count = std::count(stringval.begin(), stringval.end(), '=');
    CPPUNIT_ASSERT_EQUAL(propmap.size(), item_count);
}
