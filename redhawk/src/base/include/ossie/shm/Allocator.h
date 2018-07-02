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

#ifndef REDHAWK_SHM_ALLOCATOR_H
#define REDHAWK_SHM_ALLOCATOR_H

#include <cstddef>
#include <string>

namespace redhawk {

    namespace shm {

        class Heap;

        std::string getProcessHeapName(pid_t pid);

        Heap* getProcessHeap();
        bool isEnabled();
        void* allocate(size_t bytes);
        void deallocate(void* ptr);

        void* allocateHybrid(size_t bytes);
        void deallocateHybrid(void* ptr);

        template <class T>
        struct Allocator : public std::allocator<T>
        {
        public:
            typedef std::allocator<T> base;
            typedef typename base::pointer pointer;
            typedef typename base::value_type value_type;
            typedef typename base::size_type size_type;

            template <typename U>
            struct rebind {
                typedef Allocator<U> other;
            };

            Allocator() throw() :
                base()
            {
            }

            Allocator(const Allocator& other) throw() :
                base(other)
            {
            }

            template <typename U>
            Allocator(const Allocator<U>& other) throw() :
                base(other)
            {
            }

            pointer allocate(size_type count)
            {
                size_type bytes = count * sizeof(value_type);
                void* ptr = redhawk::shm::allocate(bytes);
                if (!ptr) {
                    throw std::bad_alloc();
                }
                return static_cast<pointer>(ptr);
            }

            void deallocate(pointer ptr, size_type /*unused*/)
            {
                redhawk::shm::deallocate(ptr);
            }
        };

        template <class T>
        struct HybridAllocator : public std::allocator<T>
        {
        public:
            typedef std::allocator<T> base;
            typedef typename base::pointer pointer;
            typedef typename base::value_type value_type;
            typedef typename base::size_type size_type;

            template <typename U>
            struct rebind {
                typedef HybridAllocator<U> other;
            };

            HybridAllocator() throw() :
                base()
            {
            }

            HybridAllocator(const HybridAllocator& other) throw() :
                base(other)
            {
            }

            template <typename U>
            HybridAllocator(const HybridAllocator<U>& other) throw() :
                base(other)
            {
            }

            pointer allocate(size_type count)
            {
                size_type bytes = count * sizeof(value_type);
                void* ptr = redhawk::shm::allocateHybrid(bytes);
                if (!ptr) {
                    throw std::bad_alloc();
                }
                return static_cast<pointer>(ptr);
            }

            void deallocate(pointer ptr, size_type /*unused*/)
            {
                redhawk::shm::deallocateHybrid(ptr);
            }
        };
    }
}

#endif // REDHAWK_SHM_ALLOCATOR_H
