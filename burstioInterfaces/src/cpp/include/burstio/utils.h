/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK burstioInterfaces.
 *
 * REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef BURSTIO_UTILS_H
#define BURSTIO_UTILS_H

#include <algorithm>
#include <complex>

#include <omniORB4/CORBA.h>

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits.hpp>

#include <ossie/CorbaSequence.h>
#include <BURSTIO/burstioDataTypes.h>
#include <BULKIO/bulkioDataTypes.h>

namespace burstio {
    namespace utils {
        
        namespace detail {
            // Type traits class to support using scalar and complex types
            // generically
            template <class T>
            struct type_traits
            {
                static const bool is_complex = false;
                static const size_t scalars_per_element = 1;

                // Input type is assumed to be assignable from T, do not recast
                template <class U>
                static inline U* to_ptr(U* ptr)
                {
                    return ptr;
                }
            };

            // Partial specialization of type traits for complex types
            template <class T>
            struct type_traits<std::complex<T> >
            {
                static const bool is_complex = true;
                static const size_t scalars_per_element = 2;

                // Input type (assumed to be scalar) is not directly assignable
                // from std::complex<T>, recast to pointer to std::complex<U>.
                // This works because std::complex has no virtual methods and
                // no data members besides the real and imaginary components.
                template <class U>
                static inline std::complex<U>* to_ptr(U* ptr)
                {
                    return reinterpret_cast<std::complex<U>*>(ptr);
                }
            };

            // Implementation of iterator-based copy to a CORBA sequence, which
            // is assumed to be a scalar type (e.g., LongSequence). The value
            // of the 'element' argument is never used; it exists to allow
            // compile-time overloading of behavior based on the type of value
            // pointed to by an iterator.
            template <class Sequence, class Iterator, class Element>
            inline void copy (Sequence& dest, Iterator first, Iterator last, const Element&)
            {
                const size_t count = std::distance(first, last) * type_traits<Element>::scalars_per_element;
                dest.length(count);
                if (first != last) {
                    // Use type traits to convert destination to a pointer, to
                    // handle both scalar-to-scalar assignment (no type change)
                    // and complex-to-complex assignment (the resulting type is
                    // not necessarily the same as Element*)
                    std::copy(first, last, type_traits<Element>::to_ptr(&dest[0]));
                }
            }
        }

        // Provide backwards-compatibility with 1.8
        using ossie::corba::move;
        using ossie::corba::push_back;

        // Calls each functor in the range [begin,end) with the given argument
        template <class Iterator, class Arg>
        inline void call_each (const Iterator begin, const Iterator end, Arg arg)
        {
            for (Iterator ii = begin; ii != end; ++ii) {
                (*ii)(arg);
            }
        }

        // Removes a value from a container; the value type does not have to be
        // strictly that of the container, as long as operator== works
        template <typename Container, typename T>
        inline void remove (Container& container, const T& value)
        {
            container.erase(std::remove(container.begin(), container.end(), value), container.end());
        }

        // Returns whether a given type is complex (has real and imaginary
        // values), for the case where argument type deduction is the only way
        // to determine it
        template <typename T>
        inline bool is_complex (const T&)
        {
            return ::burstio::utils::detail::type_traits<T>::is_complex;
        }

        // Copies the values in the range [first, last) into the CORBA sequence
        // dest; handles both complex and scalar value types
        template <class Sequence, class Iterator>
        inline void copy (Sequence& dest, Iterator first, Iterator last)
        {
            // Dereference 'first' for the extra parameter to detail::copy to
            // distinguish between complex and scalar types
            ::burstio::utils::detail::copy(dest, first, last, *first);
        }

        template <typename T>
        inline void addKeyword (_CORBA_Sequence<CF::DataType>& keywords, const std::string& id, const T& value)
        {
            CF::DataType dt;
            dt.id = id.c_str();
            dt.value <<= value;
            push_back(keywords, dt);
        }

        // Borrowed from libbulkio
        BULKIO::PrecisionUTCTime now ();

        double elapsed (const BULKIO::PrecisionUTCTime& begin, const BULKIO::PrecisionUTCTime& end=now());

        BURSTIO::BurstSRI createSRI ( const std::string &streamID, double xdelta);

        BURSTIO::BurstSRI createSRI ( const std::string &streamID);

    }
}

#endif // BURSTIO_UTILS_H
