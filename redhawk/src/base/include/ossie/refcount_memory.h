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

#ifndef REDHAWK_REFCOUNT_MEMORY_H
#define REDHAWK_REFCOUNT_MEMORY_H

#include "shm/Allocator.h"

namespace redhawk {

    namespace detail {
        // Atomically increments a reference count. 
        // Abstracts away the platform-specific aspects of atomic operations
        // for use in reference counting.
        static inline void add_reference(int* counter)
        {
#ifdef __ATOMIC_RELAXED
            // Adding a reference can use the relaxed memory model because no
            // further action needs to be taken based on the reference count
            // (at least one reference exists already).
            __atomic_add_fetch(counter, 1, __ATOMIC_RELAXED);
#else
            // In GCC 4.4, the atomic built-ins are a full memory barrier.
            __sync_add_and_fetch(counter, 1);
#endif
        }

        // Atomically decrements a reference count, returning the new value.
        // Abstracts away the platform-specific aspects of atomic operations
        // for use in reference counting.
        static inline int remove_reference(int* counter)
        {
#ifdef __ATOMIC_ACQ_REL
            // Technically, the subtraction just requires a release barrier (to
            // ensure no prior operations can be moved after it), with an
            // acquire barrier only if the reference count is 0 (so that the
            // destructor code is not moved before it). In practice, using an
            // acquire-release barrier does not appear to make much difference
            // (on x86, at least) and is always safe (because it it stricter).
            return __atomic_sub_fetch(counter, 1, __ATOMIC_ACQ_REL);
#else
            // In GCC 4.4, the atomic built-ins are a full memory barrier.
            return __sync_sub_and_fetch(counter, 1);
#endif
        }

        // Special tag class for explicitly constructing a buffer that is known
        // to have been allocated by REDHAWK's shared memory allocator.
        struct process_shared_tag { };

        // Traits class for determining whether an allocator provides memory
        // that can be shared between processes (default: false).
        template <typename Allocator>
        struct is_process_shared {
            static const bool value = false;
        };

        // Traits class specialization for REDHAWK's shared memory allocator
        template <typename T>
        struct is_process_shared< ::redhawk::shm::Allocator<T> > {
            static const bool value = true;
        };

        // Traits class specialization for REDHAWK's hybrid memory allocator
        template <typename T>
        struct is_process_shared< ::redhawk::shm::HybridAllocator<T> > {
            static const bool value = true;
        };
    }

    /**
     * @brief  Reference-counted memory implementation similar to, but simpler
     * than, Boost/C++11's shared_ptr.
     *
     * %refcount_memory is a utility class that manages allocated memory with a
     * reference count. When the last reference to a block of allocated memory
     * expires, the memory is deallocated.
     *
     * This class is designed to be used as part of a more sophisticated data
     * structure like redhawk::shared_buffer to manage the reference counting
     * and customizable deallocation.
     */
    class refcount_memory
    {
    public:
        /**
         * @brief  Construct an empty %refcount_memory.
         */
        refcount_memory() :
            _M_impl(0)
        {
        }

        /**
         * @brief  Construct a %refcount_memory with an existing pointer.
         * @param ptr  Pointer to first element.
         * @param count  Number of elements.
         *
         * The newly-created %refcount_memory takes ownership of @a data. When
         * the last %refcount_memory pointing to @a data is destroyed, @a data
         * will be deleted with delete[].
         */
        template <typename T>
        refcount_memory(T* ptr, size_t count) :
            _M_impl(new impl(ptr, count))
        {
        }

        /**
         * @brief  Construct a %refcount_memory and allocate memory.
         * @param count  Number of elements to allocate.
         * @param alloc  STL-compliant allocator.

         * Creates an internal copy of @a alloc and allocates @a count elements
         * in an exception-safe manner. When the last %refcount_memory pointing
         * to the allocated memory is destroyed, the memory will be deallocated
         * via the internal copy of @a alloc.
         */
        template <class Alloc>
        refcount_memory(size_t count, const Alloc& alloc) :
            _M_impl(_M_allocate(count, alloc))
        {
        }

        /**
         * @brief  Construct a %refcount_memory with an existing pointer and a
         *         custom deleter.
         * @param ptr  Pointer to first element.
         * @param count  Number of elements.
         * @param deleter  Callable object.
         *
         * @a D must by copy-constructible. When the last %refcount_memory
         * pointing to @a ptr is destroyed, @a deleter will be called on
         * @a ptr. This can be used to define custom release behavior.
         */
        template <typename T, class D>
        refcount_memory(T* ptr, size_t count, D deleter) :
            _M_impl(new func_impl<D>(ptr, count, deleter, false))
        {
        }

        /**
         * @brief  Construct a %refcount_memory with an existing pointer known
         *         to be allocated from process-shared memory.
         * @param ptr  Pointer to first element.
         * @param count  Number of elements.
         * @param deleter  Callable object.
         * @param tag  Indicates that @a ptr is in process-shared memory.
         *
         * @warning This constructor is intended for internal use only.
         */
        template <typename T, class D>
        refcount_memory(T* ptr, size_t count, D deleter, detail::process_shared_tag) :
            _M_impl(new func_impl<D>(ptr, count, deleter, true))
        {
        }

        /**
         * @brief  %refcount_memory copy constructor.
         * @param other  Another %refcount_memory.
         *
         * %refcount_memory has reference semantics; after construction, this
         * instance shares the underlying memory, increasing its reference
         * count by one.
         */
        refcount_memory(const refcount_memory& other) :
            _M_impl(other._M_impl)
        {
            if (_M_impl) {
                detail::add_reference(&(_M_impl->refcount));
            }
        }

        /**
         * The dtor decrements reference count of the allocated memory. If no
         * other %refcount_memory points to the memory it is deallocated.
         */
        ~refcount_memory()
        {
            if (_M_impl) {
                if (detail::remove_reference(&(_M_impl->refcount)) == 0) {
                    delete _M_impl;
                }
            }
        }

        /**
         * @brief  %refcount_memory assignment operator.
         * @param other  Another %refcount_memory.
         *
         * %refcount_memory has reference semantics; after assignment, this
         * instance shares the underlying memory, increasing its reference
         * count by one. The prior memory is released; if this was the last
         * reference, the memory is deallocated.
         */
        refcount_memory& operator=(const refcount_memory& other)
        {
            // Use copy constructor and swap to handle reference count
            refcount_memory temp(other);
            this->swap(temp);
            return *this;
        }

        /**
         * Returns true if the allocated memory is from process-shared memory.
         */
        bool is_process_shared() const
        {
            if (_M_impl) {
                return _M_impl->shared;
            } else {
                return false;
            }
        }

        /**
         * Returns the base address of the allocated memory.
         */
        const void* address() const
        {
            if (_M_impl) {
                return _M_impl->addr;
            } else {
                return 0;
            }
        }

        /**
         * Returns the size of the allocated memory, in bytes.
         */
        size_t bytes() const
        {
            if (_M_impl) {
                return _M_impl->bytes;
            } else {
                return 0;
            }
        }

        /**
         * Validity check, returning true if this object contains no allocated
         * memory.
         */
        bool operator! () const
        {
            return !_M_impl;
        }

        /**
         * @brief  Swaps contents with another %refcount_memory.
         * @param other  %refcount_memory to swap with.
         */
        void swap(refcount_memory& other)
        {
            std::swap(_M_impl, other._M_impl);
        }

    private:
        /// @cond IMPL

        // Base implementation of the reference counted memory block, storing
        // the reference count and memory address along with a function pointer
        // to a deleter, which subclasses may provide. Compared with virtual
        // functions, this approach greatly reduces the number of symbols
        // created per type, and makes the in-memory layout more predicatable
        // should cache alignment be a concern.
        struct impl {
            typedef void (*release_func)(impl*);

            template <typename T>
            impl(T* ptr, size_t count, release_func func=&impl::delete_release<T>, bool shared=false) :
                refcount(1),
                addr(ptr),
                bytes(count * sizeof(T)),
                release(func),
                shared(shared)
            {
            }

            ~impl()
            {
                if (addr) {
                    release(this);
                }
            }

            template <typename T>
            static void delete_release(impl* imp)
            {
                T* ptr = reinterpret_cast<T*>(imp->addr);
                delete[] ptr;
            }

            int refcount;
            void* addr;
            size_t bytes;
            release_func release;
            bool shared;
        };

        // Data buffer implementation that uses an arbitrary function to
        // release the memory; may be a function pointer or functor.
        template <class Func>
        struct func_impl : public impl {
            template <typename T>
            func_impl(T* ptr, size_t count, Func func, bool shared) :
                impl(ptr, count, &func_impl::func_release<T>, shared),
                func(func)
            {
            }

            template <typename T>
            static void func_release(impl* imp)
            {
                T* ptr = reinterpret_cast<T*>(imp->addr);
                static_cast<func_impl*>(imp)->func(ptr);
            }

            Func func;
        };

        // Data buffer implementation that inherits from an STL-compliant
        // allocator class; used below for exception-safe allocation, as well
        // as deallocation when the memory is released.
        template <class Alloc>
        struct allocator_impl : public impl, public Alloc
        {
            typedef typename Alloc::value_type value_type;

            allocator_impl(value_type* ptr, size_t count, const Alloc& allocator) :
                impl(ptr, count, &allocator_impl::allocator_release, detail::is_process_shared<Alloc>::value),
                Alloc(allocator)
            {
            }

            static void allocator_release(impl* imp)
            {
                value_type* ptr = reinterpret_cast<value_type*>(imp->addr);
                size_t count = imp->bytes / sizeof(value_type);
                allocator_impl* alloc = static_cast<allocator_impl*>(imp);
                alloc->deallocate(ptr, count);
            }
        };

        // Implementation of allocating constructor; creates an allocator-based
        // data buffer implementation in an exception-safe way which can then
        // be passed to the base class constructor.
        template <class Alloc>
        static impl* _M_allocate(size_t count, const Alloc& allocator)
        {
            // Zero-length buffer requires no allocation, so don't bother with
            // an implementation in the first place.
            if (count == 0) {
                return 0;
            }

            // Create an empty allocator_impl instance first, then try to
            // allocate the memory, using the fact that it inherits from the
            // allocator.
            typedef allocator_impl<Alloc> impl_type;
            impl_type* imp = new impl_type(0, count, allocator);
            try {
                imp->addr = imp->allocate(count);
            } catch (...) {
                // If allocation throws an exception (most likely, std::bad_alloc),
                // delete the implementation to avoid a memory leak, and rethrow.
                delete imp;
                throw;
            }
            return imp;
        }

        impl* _M_impl;
        /// @endcond
    };
}

#endif // REDHAWK_REFCOUNT_MEMORY_H
