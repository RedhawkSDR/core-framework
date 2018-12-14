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
    class NumericTestImpl
    {
    public:
        typedef T (*conversion_func)(const CORBA::Any&);

        typedef std::numeric_limits<T> limits;

        NumericTestImpl(conversion_func func) :
            func(func)
        {
        }

        void testFromBoolean()
        {
            CORBA::Any any;

            any <<= true;
            CPPUNIT_ASSERT_EQUAL((T) 1, func(any));

            any <<= false;
            CPPUNIT_ASSERT_EQUAL((T) 0, func(any));
        }

        void testFromString()
        {
            const T mid = (limits::min() / 2) + (limits::max() / 2);
            std::ostringstream oss;
            oss << mid;

            CORBA::Any any;
            any <<= oss.str();
            CPPUNIT_ASSERT_EQUAL(mid, func(any));

            any <<= "1.27e2";
            CPPUNIT_ASSERT_EQUAL((T) 127, func(any));
        }

        void testFromNumber()
        {
            CORBA::Any any;

            any <<= CORBA::Any::from_octet(1);
            CPPUNIT_ASSERT_EQUAL((T) 1, func(any));

            any <<= (CORBA::Short) 2;
            CPPUNIT_ASSERT_EQUAL((T) 2, func(any));

            any <<= (CORBA::UShort) 3;
            CPPUNIT_ASSERT_EQUAL((T) 3, func(any));

            any <<= (CORBA::Long) 4;
            CPPUNIT_ASSERT_EQUAL((T) 4, func(any));

            any <<= (CORBA::ULong) 5;
            CPPUNIT_ASSERT_EQUAL((T) 5, func(any));

            any <<= (CORBA::LongLong) 6;
            CPPUNIT_ASSERT_EQUAL((T) 6, func(any));

            any <<= (CORBA::ULongLong) 7;
            CPPUNIT_ASSERT_EQUAL((T) 7, func(any));

            any <<= (CORBA::Float) 8;
            CPPUNIT_ASSERT_EQUAL((T) 8, func(any));

            any <<= (CORBA::Double) 9;
            CPPUNIT_ASSERT_EQUAL((T) 9, func(any));
        }

        void testRange()
        {
            T min = limits::min();
            T max = limits::max();
            testRangeImpl(min, max, min - 1.0, max + 1.0);
        }

    private:
        void testRangeImpl(T min, T max, double under, double over)
        {
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

            any <<= under;
            CPPUNIT_ASSERT_THROW(func(any), std::range_error);
            CPPUNIT_ASSERT(!ossie::any::toNumber(any, result));

            any <<= over;
            CPPUNIT_ASSERT_THROW(func(any), std::range_error);
            CPPUNIT_ASSERT(!ossie::any::toNumber(any, result));
        }

        conversion_func func;
    };

    template <>
    void NumericTestImpl<CORBA::LongLong>::testRange()
    {
        // Double has a wider range but less precision (52 bits) than 64-bit
        // integers, so it cannot precisely represent the maximum or minimum
        // values. To ensure that the minimum and maximum are within the legal
        // range of the converters (see boost::numeric::converter), adjust the
        // test values towards zero by the effective epsilon (the minimum
        // difference representible with a double at a given magnitude). In
        // other words, the double values are quantized, so explicitly pick the
        // nearest value that is less than the "real" value.
        int bits = limits::digits - std::numeric_limits<double>::digits;
        double epsilon = (1 << bits);
        CORBA::LongLong min = limits::min() + epsilon;
        // In essence, mask out the least significant bits of the maximum
        CORBA::LongLong max = limits::max() - (epsilon - 1);
        // On 32-bit x86, we need to ensure that the difference between the
        // double-precision "under" value and the 64-bit signed integer values
        // is enough to register as out of range; multiplying the epsilon value
        // by 4 (i.e., shift up by 2 bits) solves the problem.
        testRangeImpl(min, max, min - (epsilon*4), max + epsilon);
    }

    template <>
    void NumericTestImpl<CORBA::ULongLong>::testRange()
    {
        // See above, except only the maximum actually uses the full precision
        int bits = limits::digits - std::numeric_limits<double>::digits;
        double epsilon = (1 << bits);
        CORBA::ULongLong max = limits::max() - (epsilon - 1);
        testRangeImpl(0, max, -1.0, max + epsilon);
    }

    template <>
    void NumericTestImpl<CORBA::Octet>::testFromString()
    {
        CORBA::Any any;
        any <<= "0";
        CPPUNIT_ASSERT_EQUAL((CORBA::Octet) 0, ossie::any::toOctet(any));

        any <<= "255";
        CPPUNIT_ASSERT_EQUAL((CORBA::Octet) 255, ossie::any::toOctet(any));

        any <<= "256";
        CPPUNIT_ASSERT_THROW(ossie::any::toOctet(any), std::range_error);
    }

    template <>
    void NumericTestImpl<CORBA::Float>::testRange()
    {
        CORBA::Float max = limits::max();
        testRangeImpl(-max, max, -2.0 * max, 2.0 * max);
    }

    template <>
    void NumericTestImpl<CORBA::Float>::testFromString()
    {
        // Very large value
        CORBA::Any any;
        any <<= "1.125e+38";
        CPPUNIT_ASSERT_EQUAL(1.125e38f, ossie::any::toFloat(any));

        // Very small value
        any <<= "-1.0002441406250e-32";
        CPPUNIT_ASSERT_EQUAL(-1.0002441406250e-32f, ossie::any::toFloat(any));

        // Beyond the maximum range of float should throw an exception
        any <<= "7e40";
        CPPUNIT_ASSERT_THROW(ossie::any::toFloat(any), std::range_error);

        // Number too small to be represented as a float (but valid for double)
        // should get rounded to zero
        any <<= "5.03125e-46";
        CPPUNIT_ASSERT_EQUAL(0.0f, ossie::any::toFloat(any));
    }

    template <>
    void NumericTestImpl<CORBA::Double>::testRange()
    {
        // Double has the largest range of the common numeric primitive types,
        // so it's not possible to exceed its range with another primitive type
        // (this test is here so that double tests can be created using the
        // same macros as the other types)
    }

    template <>
    void NumericTestImpl<CORBA::Double>::testFromString()
    {
        CORBA::Any any;

        // Simple integer conversion
        any <<= "100000";
        CPPUNIT_ASSERT_EQUAL(100000.0, ossie::any::toDouble(any));

        // Use a floating point value that is known to show a decimal point and
        // exponent
        double value = 1.125e7;
        std::ostringstream oss;
        oss << value;
        any <<= oss.str();

        // Conversion back to double should be simple
        CPPUNIT_ASSERT_EQUAL(value, ossie::any::toDouble(any));
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
    any <<= (float)1;
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
    any <<= (short)0;
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

#define DEFINE_NUMERIC_TEST(T,NAME)                             \
    void AnyUtilsTest::testTo##T##NAME()                        \
    {                                                           \
        NumericTestImpl<CORBA::T> impl(ossie::any::to##T);      \
        impl.test##NAME();                                      \
    }

FOREACH_TYPE_TEST(DEFINE_NUMERIC_TEST);
