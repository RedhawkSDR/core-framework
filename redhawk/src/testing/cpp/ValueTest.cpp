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

#include "ValueTest.h"

#include <string>
#include <vector>

#include <ossie/Value.h>

CPPUNIT_TEST_SUITE_REGISTRATION(ValueTest);

void ValueTest::setUp()
{
}

void ValueTest::tearDown()
{
}

void ValueTest::testConstructor()
{
    // This test is mostly to make sure that the constructors for all supported
    // types compile.

    // Start with an empty value
    {
        redhawk::Value value;
        CPPUNIT_ASSERT(value.isNil());
    }

    // NB: The CORBA C++ mapping does not allow directly setting an octet value,
    // nor does it have true 8-bit numeric types. Use the helper to insert an
    // octet for the purposes of testing.
    {
        redhawk::Value value(CORBA::Any::from_octet(0));
        CPPUNIT_ASSERT_MESSAGE("Octet constructor", !value.isNil());
    }

    // Numeric and common sequence types
#define ASSERT_CTOR(x, v)                                               \
    {                                                                   \
        redhawk::Value value((x) v);                                    \
        CPPUNIT_ASSERT_MESSAGE(#x " constructor", !value.isNil());      \
    }
    ASSERT_CTOR(CORBA::Short, 0);
    ASSERT_CTOR(CORBA::UShort, 0);
    ASSERT_CTOR(CORBA::Long, 0);
    ASSERT_CTOR(CORBA::ULong, 0);
    ASSERT_CTOR(CORBA::LongLong, 0);
    ASSERT_CTOR(CORBA::ULongLong, 0);
    ASSERT_CTOR(CORBA::Float, 0.0);
    ASSERT_CTOR(CORBA::Double, 0.0);
    ASSERT_CTOR(std::string, "abc");

    CORBA::AnySeq anys;
    ASSERT_CTOR(CORBA::AnySeq, anys);

    CF::Properties properties;
    ASSERT_CTOR(CF::Properties, properties);
}

void ValueTest::testCopyConstructor()
{
    CORBA::Any any;
    CF::Properties properties;
    any <<= properties;
    CORBA::TypeCode_ptr typecode = any.type();
    redhawk::Value::Type type = redhawk::Value::GetType(typecode);

    // Copy constructor from Any (mostly making sure the type is the same, as
    // opposed to nesting another level of Any)
    redhawk::Value value(any);
    CPPUNIT_ASSERT_EQUAL(type, value.getType());

    // Copy constructor (again, checking that no accidental nesting occurred)
    redhawk::Value copy(value);
    CPPUNIT_ASSERT_EQUAL(type, copy.getType());
}

void ValueTest::testType()
{
    redhawk::Value value;
    CPPUNIT_ASSERT(value.getType() == redhawk::Value::TYPE_NONE);
    CPPUNIT_ASSERT(!value.isNumeric());
    CPPUNIT_ASSERT(!value.isSequence());

    value = std::string("test");
    CPPUNIT_ASSERT_EQUAL(value.getType(), redhawk::Value::TYPE_STRING);
    CPPUNIT_ASSERT(!value.isNumeric());
    CPPUNIT_ASSERT(!value.isSequence());

#define ASSERT_BASIC_NUMERIC(v, t)              \
    CPPUNIT_ASSERT_EQUAL(v.getType(), t);       \
    CPPUNIT_ASSERT(v.isNumeric());              \
    CPPUNIT_ASSERT(!v.isSequence());

    value = true;
    ASSERT_BASIC_NUMERIC(value, redhawk::Value::TYPE_BOOLEAN);

    // See testConstructor() for note about octet.
    value = CORBA::Any::from_octet(1);
    ASSERT_BASIC_NUMERIC(value, redhawk::Value::TYPE_OCTET);

    value = (CORBA::Short) -2;
    ASSERT_BASIC_NUMERIC(value, redhawk::Value::TYPE_SHORT);

    value = (CORBA::UShort) 3;
    ASSERT_BASIC_NUMERIC(value, redhawk::Value::TYPE_USHORT);

    value = (CORBA::Long) -4;
    ASSERT_BASIC_NUMERIC(value, redhawk::Value::TYPE_LONG);

    value = (CORBA::ULong) 5;
    ASSERT_BASIC_NUMERIC(value, redhawk::Value::TYPE_ULONG);

    value = (CORBA::LongLong) -6;
    ASSERT_BASIC_NUMERIC(value, redhawk::Value::TYPE_LONGLONG);

    value = (CORBA::ULongLong) 7;
    ASSERT_BASIC_NUMERIC(value, redhawk::Value::TYPE_ULONGLONG);

    value = (CORBA::Float) -8.0;
    ASSERT_BASIC_NUMERIC(value, redhawk::Value::TYPE_FLOAT);

    value = (CORBA::Double) 1e100;
    ASSERT_BASIC_NUMERIC(value, redhawk::Value::TYPE_DOUBLE);

    value = CORBA::AnySeq();
    CPPUNIT_ASSERT_EQUAL(value.getType(), redhawk::Value::TYPE_VALUE_SEQUENCE);
    CPPUNIT_ASSERT(!value.isNumeric());
    CPPUNIT_ASSERT(value.isSequence());
    CPPUNIT_ASSERT_EQUAL(value.getElementType(), redhawk::Value::TYPE_VALUE);

    value = CF::Properties();
    CPPUNIT_ASSERT_EQUAL(value.getType(), redhawk::Value::TYPE_PROPERTIES);
    CPPUNIT_ASSERT(!value.isNumeric());
    CPPUNIT_ASSERT(value.isSequence());
    CPPUNIT_ASSERT_EQUAL(value.getElementType(), redhawk::Value::TYPE_DATATYPE);
}

void ValueTest::testNumericConversion()
{
    // Boolean conversion from case-insensitive string literals and numbers
    // (where zero is false and non-zero is true)
    CPPUNIT_ASSERT_EQUAL(true, redhawk::Value("True").toBoolean());
    CPPUNIT_ASSERT_EQUAL(false, redhawk::Value("false").toBoolean());
    CPPUNIT_ASSERT_EQUAL(true, redhawk::Value((short)-1).toBoolean());
    CPPUNIT_ASSERT_EQUAL(false, redhawk::Value((short)0).toBoolean());

    // Octet conversion from string, int and double; range test
    CPPUNIT_ASSERT_EQUAL((CORBA::Octet) 222, redhawk::Value("222").toOctet());
    CPPUNIT_ASSERT_EQUAL((CORBA::Octet) 1, redhawk::Value((short)1).toOctet());
    CPPUNIT_ASSERT_EQUAL((CORBA::Octet) 125, redhawk::Value(125.5).toOctet());
    CPPUNIT_ASSERT_THROW(redhawk::Value((short)-1).toOctet(), std::range_error);
    CPPUNIT_ASSERT_THROW(redhawk::Value((short)256).toOctet(), std::range_error);

    // Short conversion from string, int and double; range test
    CPPUNIT_ASSERT_EQUAL((CORBA::Short) -25000, redhawk::Value("-25000").toShort());
    CPPUNIT_ASSERT_EQUAL((CORBA::Short) 1, redhawk::Value((float)1).toShort());
    CPPUNIT_ASSERT_EQUAL((CORBA::Short) 16000, redhawk::Value(16000.1).toShort());
    CPPUNIT_ASSERT_THROW(redhawk::Value((float)65536).toShort(), std::range_error);

    // UShort conversion from string, int and double; range test
    CPPUNIT_ASSERT_EQUAL((CORBA::UShort) 60000, redhawk::Value("60000").toUShort());
    CPPUNIT_ASSERT_EQUAL((CORBA::UShort) 1, redhawk::Value((float)1).toUShort());
    CPPUNIT_ASSERT_EQUAL((CORBA::UShort) 50000, redhawk::Value(50000.999).toUShort());
    CPPUNIT_ASSERT_THROW(redhawk::Value((float)-1).toUShort(), std::range_error);
    CPPUNIT_ASSERT_THROW(redhawk::Value((float)65536).toUShort(), std::range_error);

    // Long conversion from string, short and double; range test
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) -262144, redhawk::Value("-262144").toLong());
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) 1, redhawk::Value((short) 1).toLong());
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) 100000000, redhawk::Value(1e8).toLong());
    CPPUNIT_ASSERT_THROW(redhawk::Value(1e10).toLong(), std::range_error);
    CPPUNIT_ASSERT_THROW(redhawk::Value(-1e10).toLong(), std::range_error);

    // ULong conversion from string, int and double; range test
    CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 4294967295, redhawk::Value("4294967295").toULong());
    CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 1, redhawk::Value((short)1).toULong());
    CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 3000000000, redhawk::Value(3e9).toULong());
    CPPUNIT_ASSERT_THROW(redhawk::Value((short)-1).toULong(), std::range_error);
    CPPUNIT_ASSERT_THROW(redhawk::Value(4294967296L).toULong(), std::range_error);

    // LongLong conversion from string, int and double; range test
    CPPUNIT_ASSERT_EQUAL((CORBA::LongLong) 1099511627776L, redhawk::Value("1099511627776").toLongLong());
    CPPUNIT_ASSERT_EQUAL((CORBA::LongLong) 1, redhawk::Value((short)1).toLongLong());
    CPPUNIT_ASSERT_EQUAL((CORBA::LongLong) 10000000000, redhawk::Value(1e10).toLongLong());
    CPPUNIT_ASSERT_THROW(redhawk::Value(1e19).toLongLong(), std::range_error);
    CPPUNIT_ASSERT_THROW(redhawk::Value(-1e19).toLongLong(), std::range_error);

    // ULongLong conversion from string, boolean and double; range test
    CPPUNIT_ASSERT_EQUAL((CORBA::ULongLong) 9223372036854775808UL, redhawk::Value("9223372036854775808").toULongLong());
    CPPUNIT_ASSERT_EQUAL((CORBA::ULongLong) 1, redhawk::Value(true).toULongLong());
    CPPUNIT_ASSERT_EQUAL((CORBA::ULongLong) 500000000000, redhawk::Value(5e11).toULongLong());
    CPPUNIT_ASSERT_THROW(redhawk::Value((short)-1).toULongLong(), std::range_error);
    CPPUNIT_ASSERT_THROW(redhawk::Value(1e20).toULongLong(), std::range_error);
}

void ValueTest::testStringConversion()
{
    // Not defined: TYPE_NONE, TYPE_VALUE_SEQUENCE
    // Not tested: TYPE_PROPERTIES

    // Boolean
    // NB: This would probably be more helpful if it were "true" and "false"
    CPPUNIT_ASSERT_EQUAL(std::string("1"), redhawk::Value(true).toString());
    CPPUNIT_ASSERT_EQUAL(std::string("0"), redhawk::Value(false).toString());

    // Octet (we have to use from_octet because of ambiguity in any insertion;
    // see above)
    CPPUNIT_ASSERT_EQUAL(std::string("255"), redhawk::Value(CORBA::Any::from_octet(255)).toString());

    // Integer numeric types
    CPPUNIT_ASSERT_EQUAL(std::string("-16000"), redhawk::Value((CORBA::Short) -16000).toString());
    CPPUNIT_ASSERT_EQUAL(std::string("65535"), redhawk::Value((CORBA::UShort) 65535).toString());
    CPPUNIT_ASSERT_EQUAL(std::string("-200000"), redhawk::Value((CORBA::Long) -200000).toString());
    CPPUNIT_ASSERT_EQUAL(std::string("4294967295"), redhawk::Value((CORBA::ULong) 4294967295).toString());
    CPPUNIT_ASSERT_EQUAL(std::string("-5000000000"), redhawk::Value((CORBA::LongLong) -5000000000L).toString());
    CPPUNIT_ASSERT_EQUAL(std::string("9223372036854775808"), redhawk::Value((CORBA::ULongLong) 9223372036854775808UL).toString());

    // Floating point numeric types--this is a little more fragile because it
    // depends on the specific output format
    CPPUNIT_ASSERT_EQUAL(std::string("5.125"), redhawk::Value((CORBA::Float) 5.125).toString());
    CPPUNIT_ASSERT_EQUAL(std::string("-2.25e+40"), redhawk::Value((CORBA::Double) -2.25e40).toString());
}

void ValueTest::testConstCast()
{
    CORBA::Any any;
    const double dval = 1.25;
    any <<= dval;

    // Create a const Value alias and check that it matches the Any
    const CORBA::Any& const_any = any;
    const redhawk::Value& value = redhawk::Value::cast(const_any);
    CPPUNIT_ASSERT_EQUAL(redhawk::Value::TYPE_DOUBLE, value.getType());
    CPPUNIT_ASSERT_EQUAL(dval, value.toDouble());

    // Modify the Any and check that the change is reflected in the Value
    const std::string stringval = "value";
    any <<= stringval;
    CPPUNIT_ASSERT_EQUAL(redhawk::Value::TYPE_STRING, value.getType());
    CPPUNIT_ASSERT_EQUAL(stringval, value.toString());
}

void ValueTest::testCast()
{
    CORBA::Any any;
    const double dval = 1.25;
    any <<= dval;

    // Create a Value alias and check that it matches the Any
    redhawk::Value& value = redhawk::Value::cast(any);
    CPPUNIT_ASSERT_EQUAL(redhawk::Value::TYPE_DOUBLE, value.getType());
    CPPUNIT_ASSERT_EQUAL(dval, value.toDouble());

    // Set the value to nil and check that the Any was modified (ergo, they
    // are both the same object)
    CPPUNIT_ASSERT(!ossie::any::isNull(any));
    CPPUNIT_ASSERT(!value.isNil());
    value = redhawk::Value();
    CPPUNIT_ASSERT(value.isNil());
    CPPUNIT_ASSERT(ossie::any::isNull(any));
}
