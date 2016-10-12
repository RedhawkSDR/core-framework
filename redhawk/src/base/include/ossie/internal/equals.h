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

#ifndef OSSIE_INTERNAL_EQUALS_H
#define OSSIE_INTERNAL_EQUALS_H

#include <boost/utility/enable_if.hpp>

/*
 * Prior to 1.8, the code generators did not define an operator== for struct
 * properties; although most user code should be 1.8.0 or newer, some of the
 * unit test components predate that change. For compatibility, the equals
 * function can be templatized to use == if it is available, but falls back on
 * something else (memcmp).
 *
 * Note that the types and methods for performing the boost::enable_if tests
 * are only declared, and never actually defined or instantiated. Everything is
 * hidden in an internal namespace to prevent polluting the global namespace
 * and introducing potential ambiguities in functions overloads.
 */
namespace ossie {
    namespace internal {
        // Types used for compile-time checking are guaranteed to have
        // different sizes
        typedef char no[1];
        typedef char yes[2];

        // Generic struct that can be implicity constructed from any input
        // type; by defining an operator== in terms of this type, any type that
        // does not otherwise have an operator== will fall back to this one
        // (but only within the ossie::internal namespace). The different
        // return type ("no" versus "bool") allows us to distinguish which
        // overload for operator== was selected.
        struct any_t {
            template <typename T> any_t(const T&);
        };
        no& operator== (const any_t&, const any_t&);

        // Overloaded function to take the return type of operator== ("bool" if
        // one exists, "no" otherwise) and return one of two distinguishable
        // types
        yes& test (bool);
        no& test (no);

        // Templatized struct for use with boost::enable_if and friends.
        // Contains a static value that is true if an operator== exists that
        // can be applied to the templatized type, or false otherwise; its
        // value is determined at compile time. If there is an operator==
        // defined for the type, the overload of test() taking a bool and
        // returning a "yes" object is selected. Otherwise, it uses the any_t
        // fallback operator==, which returns a "no" object; the two return
        // types have different sizes.
        template <typename T>
        struct has_equals
        {
            static const T& t;
            static bool const value = sizeof(test(t == t)) == sizeof(yes);
        };

        // Equality comparison overload for types that have operator== support
        template <typename T>
        inline typename boost::enable_if<has_equals<T>, bool>::type
        equals(const T& lhs, const T& rhs)
        {
            return (lhs == rhs);
        }

        // Equality comparison overload for types that do not have operator==
        // support
        template <typename T>
        inline typename boost::disable_if<has_equals<T>, bool>::type
        equals(const T& lhs, const T& rhs)
        {
            return std::memcmp(&lhs, &rhs, sizeof(T)) == 0;
        }

        // Equality comparison overload for vectors of types that do not have
        // operator== support (i.e., struct sequences). The vector operator==
        // only works if the contained type also supports operator==.
        template <typename T>
        inline typename boost::disable_if<has_equals<T>, bool>::type
        equals(const std::vector<T>& lhs, const std::vector<T>& rhs)
        {
            if (lhs.size() != rhs.size()) {
                return false;
            }
            for (size_t ii = 0; ii < lhs.size(); ++ii) {
                if (!equals(lhs[ii], rhs[ii])) {
                    return false;
                }
            }
            return true;
        }
    }
}

#endif // OSSIE_INTERNAL_EQUALS_H
