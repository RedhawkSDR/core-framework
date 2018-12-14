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

#ifndef REDHAWK_BUFFERMANAGER_H
#define REDHAWK_BUFFERMANAGER_H

#include <set>

#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

namespace redhawk {
    /**
     * @brief  Singleton class providing buffer allocation management.
     *
     * The %BufferManager improves the performance of repetetive allocations by
     * caching allocated memory blocks on a per-thread basis. When a memory
     * block is deallocated, it is returned to the cache of the thread that
     * made the original allocation. Future allocations of the same (or nearly
     * the same) size on the originating thread can return cached memory blocks
     * rather than allocating a new memory block from the operating system. In
     * comparison to using the operating system's facilities, the memory is not
     * zeroed before it is returned to the caller; skipping this step provides
     * the most significant optimization in the allocation process.
     *
     * %BufferManager's API gives additional control over caching policy to
     * limit the size of per-thread caches.
     */
    class BufferManager {
    public:

        /**
         * @brief  STL-compliant allocator using BufferManager.
         *
         * %Allocator goes through the BufferManager to improve performance
         * with repetitive allocations.  In testing, allocations under 1K bytes
         * did not show any benefit from %BufferManager; as a result, they
         * defer to the basic std::allocator<T> implementation.
         */
        template <typename T>
        class Allocator : public std::allocator<T>
        {
        public:
            typedef std::allocator<T> base;
            typedef typename base::pointer pointer;
            typedef typename base::value_type value_type;
            typedef typename base::size_type size_type;

            static const size_type MIN_ELEMENTS = 1024 / sizeof(value_type);

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
                if (count >= MIN_ELEMENTS) {
                    return static_cast<pointer>(BufferManager::Allocate(count * sizeof(value_type)));
                } else {
                    return base::allocate(count);
                }
            }

            void deallocate(pointer ptr, size_type count)
            {
                if (count >= MIN_ELEMENTS) {
                    BufferManager::Deallocate(ptr);
                } else {
                    base::deallocate(ptr, count);
                }
            }
        };

        /**
         * @brief  Gets the %BufferManager singleton.
         * @return Reference to the singleton instace.
         */
        static BufferManager& Instance();

        /**
         * Static convenience function to allocate memory.
         * @see allocate(size_t)
         */
        static inline void* Allocate(size_t bytes)
        {
            return Instance().allocate(bytes);
        }

        /**
         * Static convenience function to deallocate memory.
         * @see deallocate(void*)
         */
        static inline void Deallocate(void* ptr)
        {
            return Instance().deallocate(ptr);
        }

        /**
         * @brief  Allocate memory.
         * @param bytes  Required number of bytes.
         * @return  A void* to a memory block of at least @a bytes.
         *
         * If the requested allocation can be satisfied by the current thread's
         * cache, a previously used memory block is returned. Otherwise, a new
         * memory block is allocated from the operating system.
         */
        void* allocate(size_t bytes);

        /**
         * @brief  Deallocate memory.
         * @param ptr  The memory block to deallocate.
         *
         * The memory block is returned to the cache of the thread that originally
         * allocated it.
         */
        void deallocate(void* ptr);

        /**
         * @brief Checks whether the %BufferManager is enabled.
         * @return true if the %BufferManager is enabled.
         *
         * If the %BufferManager is disabled, deallocated memory blocks are
         * immediately returned to the operating system.
         */
        bool isEnabled() const;

        /**
         * @brief Enable or disable the %BufferManager.
         * @param enabled  true to enable the %BufferManager, false to disable
         *
         * If the %BufferManager is changing from enabled to disabled, all
         * currently cached memory blocks are returned to the operating system.
         */
        void enable(bool enabled);

        /**
         * @returns  The current per-thread cache byte limit.
         */
        size_t getMaxThreadBytes() const;

        /**
         * @brief  Sets the per-thread cache byte limit.
         * @param bytes  Maximum cached bytes (per thread).
         */
        void setMaxThreadBytes(size_t bytes);

        /**
         * @returns  The current per-thread cached memory block limit.
         */
        size_t getMaxThreadBlocks() const;

        /**
         * @brief  Sets the per-thread cached memory block limit.
         * @param bytes  Maximum cached memory blocks (per thread).
         */
        void setMaxThreadBlocks(size_t blocks);

        /**
         * @returns  The current per-thread cache memory block age limit.
         */
        size_t getMaxThreadAge() const;

        /**
         * @brief  Sets the per-thread cache memory block age limit.
         * @param bytes  Maximum age of cached memory block (per thread).
         *
         * The age of a memory block is defined in terms of deallocate cycles.
         * When a block is returned to the cache, it starts with an age of 0.
         * Each time another block of memory is returned to the cache, the age
         * of all existing blocks in the cache increases by one. If a block's
         * age hits the age limit it is deallocated, allowing infrequently used
         * blocks to be returned to the operating system.
         */
        void setMaxThreadAge(size_t age);

        /**
         * @brief  Statistical information about buffer caches.
         *
         * The %Statistics struct describes the aggregate state of all thread
         * caches maintained by %BufferManager.
         */
        struct Statistics {
            /**
             * Number of currently active caches.
             */
            size_t caches;

            /**
             * Number of times an allocation was satisfied with a memory block
             * from the cache.
             */
            size_t hits;

            /**
             * Number of times the cache was not able to satisfy an allocation
             * and a new memory block had to be allocated from the system.
             */
            size_t misses;

            /**
             * Total number of memory blocks currently cached.
             */
            size_t blocks;

            /**
             * Total bytes currently cached.
             */
            size_t bytes;

            /**
             * High water mark for total cached bytes.
             */
            size_t highBytes;
        };

        /**
         * @brief  Returns the current statistical information for all caches.
         */
        Statistics getStatistics();

    private:
        /// @cond IMPL

        // BufferManager is a singleton object. This method is inaccessible to
        // user code.
        BufferManager();

        // BufferManager is a singleton whose lifetime is managed by the
        // library. This method is inaccessible to user code.
        ~BufferManager();

        // Non-copyable
        BufferManager(const BufferManager&);

        // Non-assignable
        BufferManager& operator=(const BufferManager&);

        class CacheBlock;
        friend class CacheBlock;

        class BufferCache;
        friend class BufferCache;

        // Round up the given allocation size to the nearest granularity,
        // taking the CacheBlock overhead into consideration
        size_t _nearestSize(size_t bytes);

        // Acquire a new memory block from the operating system
        CacheBlock* _allocate(size_t bytes);

        // Return an existing memory block to the operating system
        void _deallocate(CacheBlock* ptr);

        // Associate a new thread's buffer cache with the BufferManager
        void _addCache(BufferCache* cache);

        // Remove an existing buffer cache from the BufferManager
        void _removeCache(BufferCache* cache);

        // Returns the buffer cache for the current thread
        BufferCache* _getCache();

        // Report an increase in the total cached bytes (also updates high
        // water mark if necessary)
        void _increaseSize(size_t bytes);

        // Report a decrease in the total cached bytes 
        void _decreaseSize(size_t bytes);

        // Lock protecting the cache list
        boost::mutex _lock;
        typedef std::set<BufferCache*> CacheList;
        CacheList _caches;

        // Per-thread association of buffer cache
        boost::thread_specific_ptr<BufferCache> _threadCache;

        // Policy settings
        bool _enabled;
        size_t _maxThreadBytes;
        size_t _maxThreadBlocks;
        size_t _maxThreadAge;

        // Statistical data
        size_t _hits;
        size_t _misses;

        // Total size tracking; must be updated atomically
        volatile size_t _currentBytes;
        volatile size_t _highWaterBytes;

        // Singleton instance
        static BufferManager _instance;

        /// @endcond
    };


    template <typename T1, typename T2>
    inline bool operator==(const BufferManager::Allocator<T1>& a1,
                           const BufferManager::Allocator<T2>& a2)
    {
        return true;
    }

    template <typename T1, typename T2>
    inline bool operator!=(const BufferManager::Allocator<T1>& a1,
                           const BufferManager::Allocator<T2>& a2)
    {
        return false;
    }

} // namespace redhawk

#endif // OSSIE_THREADCACHE_H
