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

#ifndef REDHAWK_INTERNAL_MESSAGE_TRAITS_H
#define REDHAWK_INTERNAL_MESSAGE_TRAITS_H

#include <boost/utility/enable_if.hpp>

namespace redhawk {

    namespace internal {

        // Templatized traits struct to distinguish between structs that have a
        // static format() method (REDHAWK 2.1+) and those that do not (2.0 and
        // prior.
        template <class T>
        struct has_format
        {
            typedef ::boost::type_traits::no_type no_type;
            typedef ::boost::type_traits::yes_type yes_type;
            template <typename U, U> struct type_check;

            template <typename U>
            static yes_type& check(type_check<const char* (*)(), &U::getFormat>*);

            template <typename>
            static no_type& check(...);

            static bool const value = (sizeof(check<T>(0)) == sizeof(yes_type));
        };

        // Base message traits for structs that do not include a static
        // getFormat() method. The format is always an empty string, to adapt
        // old structs to work with newer messaging code.
        template <class T, class Enable=void>
        struct message_traits_base
        {
            static const char* format()
            {
                return "";
            }
        };

        // Base message traits for structs that have a static getFormat()
        // method.
        template <class T>
        struct message_traits_base<T, typename boost::enable_if<has_format<T> >::type>
        {
            static const char* format()
            {
                return T::getFormat();
            }
        };

        // Traits class to adapt multiple levels of REDHAWK-generated struct
        // classes used in messaging. Provides a consistent interface for
        // getting a format string or message ID, and serialization.
        template <class T>
        struct message_traits : public message_traits_base<T> {
            static void serialize(CORBA::Any& any, const void* data)
            {
                any <<= *(reinterpret_cast<const T*>(data));
            }

            static inline std::string getId(const T& message)
            {
                // Workaround for older components whose structs have a non-const,
                // non-static member function getId(): const_cast the value
                return const_cast<T&>(message).getId();
            }
        };
    }

}

#endif // REDHAWK_INTERNAL_MESSAGE_TRAITS_H
