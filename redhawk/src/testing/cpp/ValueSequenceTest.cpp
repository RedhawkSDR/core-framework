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

#include "ValueSequenceTest.h"

#include <string>
#include <vector>

#include <ossie/Value.h>

CPPUNIT_TEST_SUITE_REGISTRATION(ValueSequenceTest);

void ValueSequenceTest::setUp()
{
}

void ValueSequenceTest::tearDown()
{
}

void ValueSequenceTest::testDefaultConstructor()
{
    redhawk::ValueSequence values;
    CPPUNIT_ASSERT(values.empty());
    CPPUNIT_ASSERT_EQUAL((size_t) 0, values.size());
}

void ValueSequenceTest::testAnySeqConstructor()
{
    CORBA::AnySeq anys;
    anys.length(1);
    redhawk::ValueSequence values(anys);
    CPPUNIT_ASSERT(!values.empty());
    CPPUNIT_ASSERT_EQUAL((size_t) anys.length(), values.size());
}

void ValueSequenceTest::testConstCast()
{
    CORBA::AnySeq anys;
    anys.length(2);
    anys[0] <<= "abc";
    anys[1] <<= 1.0;

    // Create a const ValueSequence reference alias and check the values
    const CORBA::AnySeq& const_anys = anys;
    const redhawk::ValueSequence& values = redhawk::ValueSequence::cast(const_anys);
    CPPUNIT_ASSERT(!values.empty());
    CPPUNIT_ASSERT_EQUAL((size_t) anys.length(), values.size());
    CPPUNIT_ASSERT_EQUAL(std::string("abc"), values[0].toString());
    CPPUNIT_ASSERT_EQUAL(1.0, values[1].toDouble());

    // Modify the AnySeq and check that the change is reflected in the aliased
    // ValueSequence
    anys.length(3);
    anys[2] <<= (CORBA::Long) 20;
    CPPUNIT_ASSERT_EQUAL((size_t) anys.length(), values.size());
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) 20, values[2].toLong());
}

void ValueSequenceTest::testCast()
{
    CORBA::AnySeq anys;

    // Create a ValueSequence reference alias
    redhawk::ValueSequence& values = redhawk::ValueSequence::cast(anys);
    CPPUNIT_ASSERT(values.empty());

    // Append a boolean to the end of the ValueSequence and check that the
    // change is reflected in the aliased AnySeq
    values.push_back(true);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, values.size());
    CPPUNIT_ASSERT_EQUAL((size_t) anys.length(), values.size());
    bool result = false;
    CPPUNIT_ASSERT(anys[0] >>= result);
    CPPUNIT_ASSERT_EQUAL(true, result);
}

void ValueSequenceTest::testPushBack()
{
    redhawk::ValueSequence values;
    CPPUNIT_ASSERT(values.empty());

    values.push_back((short)0);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, values.size());
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) 0, values[0].toLong());

    values.push_back("one");
    CPPUNIT_ASSERT_EQUAL((size_t) 2, values.size());
    CPPUNIT_ASSERT_EQUAL(std::string("one"), values[1].toString());
}

void ValueSequenceTest::testConstIndexing()
{
    // Fill an AnySeq such that seq[x] = x
    CORBA::AnySeq anys;
    anys.length(8);
    for (CORBA::ULong index = 0; index < anys.length(); ++index) {
        anys[index] <<= index;
    }

    // Check that accessing returns the expected values
    const redhawk::ValueSequence values(anys);
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) 3, values[3].toLong());
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) 5, values[5].toLong());
}

void ValueSequenceTest::testIndexing()
{
    // Fill an AnySeq such that seq[x] = x
    CORBA::AnySeq anys;
    anys.length(8);
    for (CORBA::ULong index = 0; index < anys.length(); ++index) {
        anys[index] <<= index;
    }

    // Check that accessing returns the expected values
    redhawk::ValueSequence& values = redhawk::ValueSequence::cast(anys);
    CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 3, values[3].toULong());
    CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 5, values[5].toULong());

    // Modify values within the sequence
    values[4] = (CORBA::Long) -4;
    values[6] = (CORBA::Double) -6.0;

    // Check that the correct values were modified
    CORBA::Long lval = 0;
    CPPUNIT_ASSERT(anys[4] >>= lval);
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) -4, lval);
    CORBA::Double dval = 0.0;
    CPPUNIT_ASSERT(anys[6] >>= dval);
    CPPUNIT_ASSERT_EQUAL((CORBA::Double) -6.0, dval);
}

void ValueSequenceTest::testConstIteration()
{
    // Create a source AnySequence with predictable values
    CORBA::AnySeq anys;
    anys.length(16);
    for (CORBA::ULong index = 0; index < anys.length(); ++index) {
        anys[index] <<= (CORBA::Double) index;
    }

    // Use copy constructor to create a const ValueSequence
    const redhawk::ValueSequence values(anys);

    // The distance between the begin and end iterators must be the same as the
    // size, and iteration should yield the same result as sequential indexing
    size_t offset = 0;
    for (redhawk::ValueSequence::const_iterator iter = values.begin(); iter != values.end(); ++iter, ++offset) {
        CPPUNIT_ASSERT_EQUAL(values[offset].toDouble(), iter->toDouble());
    }
    CPPUNIT_ASSERT_EQUAL(values.size(), offset);
}

void ValueSequenceTest::testMutableIteration()
{
    // Start with an empty sequence
    redhawk::ValueSequence values;
    CPPUNIT_ASSERT_EQUAL(values.begin(), values.end());

    // Fill the sequence with predictable values
    for (size_t index = 0; index < 10; ++index) {
        values.push_back((CORBA::Double) index);
    }

    // Modify one value via an iterator
    for (redhawk::ValueSequence::iterator iter = values.begin(); iter != values.end(); ++iter) {
        if (iter->toDouble() == 5.0) {
            *iter = (short)-1000;
        }
    }

    // Check that the expected value was modified
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) -1000, values[5].toLong());
}

void ValueSequenceTest::testFromConstValue()
{
    // Create a ValueSequence with known values
    redhawk::ValueSequence original;
    original.push_back("name");
    original.push_back((short)1000);

    // Create a const Value with a copy of the original sequence
    const redhawk::Value rvalue(original);
    CPPUNIT_ASSERT_EQUAL(redhawk::Value::TYPE_VALUE_SEQUENCE, rvalue.getType());

    // Create a const ValueSequence alias to the Value and check that it
    // matches the original
    const redhawk::ValueSequence& values = rvalue.asSequence();
    CPPUNIT_ASSERT_EQUAL(original.size(), values.size());
    CPPUNIT_ASSERT_EQUAL(original[0].toString(), values[0].toString());
}

void ValueSequenceTest::testFromMutableValue()
{
    // Create a new Value from an empty ValueSequence
    redhawk::Value rvalue = redhawk::ValueSequence();
    CPPUNIT_ASSERT_EQUAL(redhawk::Value::TYPE_VALUE_SEQUENCE, rvalue.getType());

    // Create an alias and insert a value at the back, making sure it modifies
    // the Value by extracting the AnySeq
    redhawk::ValueSequence& values = rvalue.asSequence();
    values.push_back("test");
    CORBA::AnySeq* anys;
    CPPUNIT_ASSERT(rvalue >>= anys);
    CPPUNIT_ASSERT_EQUAL(values.size(), (size_t) anys->length());
    std::string result;
    CPPUNIT_ASSERT((*anys)[0] >>= result);
    CPPUNIT_ASSERT_EQUAL(std::string("test"), result);
}
