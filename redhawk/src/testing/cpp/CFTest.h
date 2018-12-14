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

#ifndef CFTEST_H
#define CFTEST_H

#include <iomanip>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Message.h>
#include <cppunit/Asserter.h>

template <typename T>
inline void checkArraysEqual(const T* expected, const T* actual, size_t count,
                             CppUnit::SourceLine sourceLine, const std::string& message)
{
    for (size_t pos = 0; pos < count; ++pos) {
        if (expected[pos] != actual[pos]) {
            std::ostringstream description;
            description << "expected != actual at position " << pos;
            std::string expectedStr = CppUnit::assertion_traits<T>::toString(expected[pos]);
            std::string actualStr = CppUnit::assertion_traits<T>::toString(actual[pos]);
            CppUnit::Asserter::failNotEqual(expectedStr, actualStr, sourceLine, message, description.str());
        }
    }
}

#define CPPUNIT_ASSERT_ARRAYS_EQUAL(expected,actual,length)     \
    ( checkArraysEqual((expected), (actual), (length), CPPUNIT_SOURCELINE(), "") )

#define CPPUNIT_ASSERT_ARRAYS_EQUAL_MESSAGE(message, expected,actual,length) \
    ( checkArraysEqual((expected), (actual), (length), CPPUNIT_SOURCELINE(), (message) ) )

namespace CppUnit {
    // Specialize assertion traits for unsigned char to always display values
    // in 2-character hex.
    template <>
    struct assertion_traits<unsigned char>
    {
        static inline bool equal(unsigned char lhs, unsigned char rhs)
        {
            return lhs == rhs;
        }

        static inline std::string toString(unsigned char value)
        {
            std::ostringstream oss;
            oss << "0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)value;
            return oss.str();
        }
    };
}

#endif // CFTEST_H
