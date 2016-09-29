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

#include "AnyUtilsTest.h"

#include <limits>
#include <stdexcept>
#include <sstream>

#include <ossie/CorbaUtils.h>

CPPUNIT_TEST_SUITE_REGISTRATION(AnyUtilsTest);

namespace {

    template <typename T>
    struct range_test
    {
        typedef std::numeric_limits<T> limits;

        static T min()
        {
            return limits::min();
        }

        static T max()
        {
            return limits::max();
        }

        static double exceed_min()
        {
            return limits::min() - 1.0;
        }

        static double exceed_max()
        {
            return limits::max() + 1.0;
        }

        static double epsilon()
        {
            int bits = limits::digits - std::numeric_limits<double>::digits;
            if (bits < 0) {
                bits = 0;
            }
            return 1 << bits;
        }
    };

    template <>
    CORBA::LongLong range_test<CORBA::LongLong>::min()
    {
        return limits::min() + epsilon();
    }

    template <>
    CORBA::LongLong range_test<CORBA::LongLong>::max()
    {
        return limits::max() - (epsilon() - 1);
    }

    template <>
    CORBA::ULongLong range_test<CORBA::ULongLong>::max()
    {
        return limits::max() - (epsilon() - 1);
    }

    template <>
    float range_test<float>::min()
    {
        return -max();
    }

    template <>
    double range_test<float>::exceed_max()
    {
        return 2.0 * max();
    }

    template <>
    double range_test<float>::exceed_min()
    {
        return -exceed_max();
    }
}

void AnyUtilsTest::setUp()
{
}

void AnyUtilsTest::tearDown()
{
}

void AnyUtilsTest::testIsNull()
{
    // Default constructor, Any has no type information
    CORBA::Any any;
    CPPUNIT_ASSERT(ossie::any::isNull(any));

    // Insert a number, should no longer be null
    any <<= 1;
    CPPUNIT_ASSERT(!ossie::any::isNull(any));

    // Likewise, a more complex type should not be null
    any <<= CF::Properties();
    CPPUNIT_ASSERT(!ossie::any::isNull(any));
}

void AnyUtilsTest::testToBoolean()
{
    CORBA::Any any;
    bool result;

    // Case-insensitive string literal
    any <<= "true";
    CPPUNIT_ASSERT(ossie::any::toBoolean(any));
    CPPUNIT_ASSERT(ossie::any::toNumber(any, result));
    CPPUNIT_ASSERT_EQUAL(true, result);

    // Case-insensitive string literal
    any <<= "False";
    CPPUNIT_ASSERT(!ossie::any::toBoolean(any));
    CPPUNIT_ASSERT(ossie::any::toNumber(any, result));
    CPPUNIT_ASSERT_EQUAL(false, result);

    // Integer, converted by C++ rules (zero == false)
    any <<= 0;
    CPPUNIT_ASSERT(!ossie::any::toBoolean(any));
    CPPUNIT_ASSERT(ossie::any::toNumber(any, result));
    CPPUNIT_ASSERT_EQUAL(false, result);

    // Double, converted by C++ rules (non-zero == true)
    any <<= 100.5;
    CPPUNIT_ASSERT(ossie::any::toBoolean(any));
    CPPUNIT_ASSERT(ossie::any::toNumber(any, result));
    CPPUNIT_ASSERT_EQUAL(true, result);

    // String that can be converted via a number
    any <<= "5000";
    CPPUNIT_ASSERT(ossie::any::toBoolean(any));
    CPPUNIT_ASSERT(ossie::any::toNumber(any, result));
    CPPUNIT_ASSERT_EQUAL(true, result);

    // String that cannot be interpreted as a boolean at all
    any <<= "invalid";
    CPPUNIT_ASSERT_THROW(ossie::any::toBoolean(any), std::bad_cast);
    CPPUNIT_ASSERT(!ossie::any::toNumber(any, result));
}

void AnyUtilsTest::testOctetConversion()
{
    CORBA::Any any;
    any <<= "0";
    CPPUNIT_ASSERT_EQUAL((CORBA::Octet) 0, ossie::any::toOctet(any));

    any <<= "255";
    CPPUNIT_ASSERT_EQUAL((CORBA::Octet) 255, ossie::any::toOctet(any));

    any <<= "256";
    CPPUNIT_ASSERT_THROW(ossie::any::toOctet(any), std::range_error);

    testConversionImpl(&ossie::any::toOctet);
}

void AnyUtilsTest::testShortConversion()
{
    testConversionImpl(&ossie::any::toShort);
}

void AnyUtilsTest::testUShortConversion()
{
    testConversionImpl(&ossie::any::toUShort);
}

void AnyUtilsTest::testLongConversion()
{
    testConversionImpl(&ossie::any::toLong);
}

void AnyUtilsTest::testULongConversion()
{
    testConversionImpl(&ossie::any::toULong);
}

void AnyUtilsTest::testLongLongConversion()
{
    testConversionImpl(&ossie::any::toLongLong);
}

void AnyUtilsTest::testULongLongConversion()
{
    testConversionImpl(&ossie::any::toULongLong);
}

void AnyUtilsTest::testFloatConversion()
{
    testConversionImpl(&ossie::any::toFloat);
}

void AnyUtilsTest::testStringToNumber()
{
    CORBA::Any any;

    // Simple integer conversion
    any <<= "100000";
    CORBA::Long lval;
    CPPUNIT_ASSERT_NO_THROW(lval = ossie::any::toLong(any));
    CPPUNIT_ASSERT_EQUAL((CORBA::Long) 100000, lval);

    // Use a floating point value that is known to show a decimal point and
    // exponent
    double value = 1.125e7;
    std::ostringstream oss;
    oss << value;
    any <<= oss.str();

    // Conversion back to double should be simple
    double dval;
    CPPUNIT_ASSERT_NO_THROW(dval = ossie::any::toDouble(any));
    CPPUNIT_ASSERT_EQUAL(value, dval);

    // Conversion to an integer requires it to parse the number as a double
    // (because of the decimal point and exponent) before converting to the
    // final integer type
    lval = 0;
    CPPUNIT_ASSERT_NO_THROW(lval = ossie::any::toLong(any));
    CPPUNIT_ASSERT_EQUAL((CORBA::Long)value, lval);
}

template <typename T>
void AnyUtilsTest::testFromBoolean(T (*func)(const CORBA::Any&))
{
    CORBA::Any any;

    any <<= true;
    CPPUNIT_ASSERT_EQUAL((T) 1, func(any));

    any <<= false;
    CPPUNIT_ASSERT_EQUAL((T) 0, func(any));
}

template <typename T>
void AnyUtilsTest::testConversionRange(T (*func)(const CORBA::Any&))
{
    testFromBoolean(func);

    const T min = ::range_test<T>::min();
    const T max = ::range_test<T>::max();

    CORBA::Any any;
    any <<= (double) min;
    T result;
    CPPUNIT_ASSERT_NO_THROW(result = func(any));
    CPPUNIT_ASSERT_EQUAL(min, result);

    result = max;
    CPPUNIT_ASSERT(ossie::any::toNumber(any, result));
    CPPUNIT_ASSERT_EQUAL(min, result);

    any <<= (double) max;
    CPPUNIT_ASSERT_NO_THROW(result = func(any));
    CPPUNIT_ASSERT_EQUAL(max, result);

    result = min;
    CPPUNIT_ASSERT(ossie::any::toNumber(any, result));
    CPPUNIT_ASSERT_EQUAL(max, result);

    any <<= ::range_test<T>::exceed_min();
    CPPUNIT_ASSERT_THROW(result = func(any), std::range_error);
    CPPUNIT_ASSERT(!ossie::any::toNumber(any, result));

    any <<= ::range_test<T>::exceed_max();
    CPPUNIT_ASSERT_THROW(result = func(any), std::range_error);
    CPPUNIT_ASSERT(!ossie::any::toNumber(any, result));
}

template <typename T>
void AnyUtilsTest::testConversionImpl(T (*func)(const CORBA::Any&))
{
    testFromBoolean(func);
    testConversionRange(func);
}
