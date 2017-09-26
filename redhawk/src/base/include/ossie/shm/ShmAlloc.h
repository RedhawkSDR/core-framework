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

#ifndef REDHAWK_SHMALLOC_H
#define REDHAWK_SHMALLOC_H

#include <cstddef>
#include <string>

#include "ShmHeap.h"

namespace redhawk {

    class ShmAllocator {
    public:
        static void* allocate(size_t bytes);
        static void deallocate(void* ptr);
        static std::string name();

        static ShmHeap::ID getID(void* ptr);

        static ShmHeap& Heap();

    private:
        static void _initialize();

        static boost::scoped_ptr<ShmHeap> _heap;
    };

    template <class T>
    struct shm_allocator : public std::allocator<T>
    {
    public:
        typedef std::allocator<T> base;
        typedef typename base::pointer pointer;
        typedef typename base::value_type value_type;
        typedef typename base::size_type size_type;

        static const size_type MIN_ELEMENTS = 1024 / sizeof(value_type);

        template <typename U>
        struct rebind {
            typedef shm_allocator<U> other;
        };

        shm_allocator() throw() :
            base()
        {
        }

        shm_allocator(const shm_allocator& other) throw() :
            base(other)
        {
        }

        template <typename U>
        shm_allocator(const shm_allocator<U>& other) throw() :
            base(other)
        {
        }

        pointer allocate(size_type count)
        {
            if (count >= MIN_ELEMENTS) {
                return static_cast<pointer>(ShmAllocator::allocate(count * sizeof(value_type)));
            } else {
                return base::allocate(count);
            }
        }

        void deallocate(pointer ptr, size_type count)
        {
            if (count >= MIN_ELEMENTS) {
                ShmAllocator::deallocate(ptr);
            } else {
                base::deallocate(ptr, count);
            }
        }
    };
}

#endif // REDHAWK_SHMALLOC_H
