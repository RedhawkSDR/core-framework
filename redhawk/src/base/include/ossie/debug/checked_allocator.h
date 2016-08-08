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

#ifndef REDHAWK_DEBUG_CHECKED_ALLOCATOR_H
#define REDHAWK_DEBUG_CHECKED_ALLOCATOR_H

#include <cstddef>
#include <stdint.h>

#include "check.h"

namespace redhawk {

    namespace debug {

        /**
         * @brief  Allocator that checks for buffer overruns and underruns.
         *
         * A custom allocator to perform additional checking for some classes
         * of memory error. Upon allocation, padding is added on the front and
         * back of the buffer and filled with a known pattern. When the memory
         * is deallocated, the front and back pads are checked to ensure that
         * they were not overwritten (or at least, are statistically unlikely
         * to have been). If the pattern has been disturbed, the allocator
         * aborts the program.
         *
         * This allocator is intended for development-time use only and should
         * never be used in production systems.
         */
        template <class T>
        struct checked_allocator
        {
            typedef std::size_t size_type;
            typedef std::ptrdiff_t difference_type;
            typedef T* pointer;
            typedef const T* const_pointer;
            typedef T& reference;
            typedef const T& const_reference;
            typedef T value_type;

            template <typename U>
            struct rebind {
                typedef checked_allocator<U> other;
            };

            pointer allocate(size_type count, const void* = 0)
            {
                // Allocate enough total bytes to accomodate the requested number
                // of elements plus the "magic number" pads on the front and back
                size_type bytes = count * sizeof(T);
                bytes += 2 * MAGIC_COUNT * sizeof(magic_type);
                void* data = ::operator new(bytes);

                // Mark the front and back pads with the known pattern and adjust
                // the returned pointer to just after the front pad
                magic_type* front = static_cast<magic_type*>(data);
                T* result = _M_mark_block(front);
                magic_type* back = _M_get_back(result, count);
                _M_mark_block(back);
                return result;
            }

            void deallocate(pointer ptr, size_type count)
            {
                // Adjust the pointer to the start of the front pad, which was the
                // beginning of the original allocation
                magic_type* front = _M_get_front(ptr);

                // Check that neither the front or the back pad has been written to
                _RH_DEBUG_CHECK(_M_is_unmodified(front));
                magic_type* back = _M_get_back(ptr, count);
                _RH_DEBUG_CHECK(_M_is_unmodified(back));

                ::operator delete(front);
            }

        private:
            // Use two 64-bit ints with a known pattern to mark the front and back
            // pads; this maintains the original alignment up to 16 bytes
            typedef int64_t magic_type;
            static const magic_type MAGIC_NUMBER = 0xfeedfacedeadbeef;
            static const size_t MAGIC_COUNT = 2;

            // Writes the known pattern to the given pad
            T* _M_mark_block(magic_type* ptr)
            {
                for (size_t ii = 0; ii < MAGIC_COUNT; ++ii) {
                    *ptr++ = MAGIC_NUMBER;
                }
                return reinterpret_cast<T*>(ptr);
            }

            // Recovers the front pad from a data pointer
            magic_type* _M_get_front(T* ptr)
            {
                magic_type* result = reinterpret_cast<magic_type*>(ptr);
                return (result - MAGIC_COUNT);
            }

            // Given data pointer and number of elements, returns the rear pad
            magic_type* _M_get_back(T* ptr, size_t count)
            {
                return reinterpret_cast<magic_type*>(ptr + count);
            }

            // Checks whether the known patter in a pad has been disturbed
            bool _M_is_unmodified(magic_type* ptr)
            {
                for (size_t ii = 0; ii < MAGIC_COUNT; ++ii) {
                    if (ptr[ii] != MAGIC_NUMBER) {
                        return false;
                    }
                }
                return true;
            }
        };

    } // namespace debug

} // namespace redhawk

#endif // _CHECKED_ALLOCATOR_HH_
