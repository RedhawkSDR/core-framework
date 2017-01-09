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
     * Per-thread cache-based allocation.
     */
    class BufferManager {
    public:

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

        BufferManager();
        ~BufferManager();

        static BufferManager& Instance();

        static inline void* Allocate(size_t bytes)
        {
            return Instance().allocate(bytes);
        }

        static inline void Deallocate(void* ptr)
        {
            return Instance().deallocate(ptr);
        }

        void* allocate(size_t bytes);
        void deallocate(void* ptr);

        bool isEnabled() const;
        void enable(bool enabled);

        size_t getMaxThreadBytes() const;
        void setMaxThreadBytes(size_t bytes);

        size_t getMaxThreadBlocks() const;
        void setMaxThreadBlocks(size_t blocks);

        size_t getMaxThreadAge() const;
        void setMaxThreadAge(size_t age);

        // Statistical information
        struct Statistics {
            size_t caches;
            size_t hits;
            size_t misses;
            size_t blocks;
            size_t bytes;
            size_t highBytes;
        };

        Statistics getStatistics();

    private:
        BufferManager(const BufferManager&);
        BufferManager& operator=(const BufferManager&);

        class CacheBlock;
        friend class CacheBlock;

        class BufferCache;
        friend class BufferCache;

        size_t _nearestSize(size_t bytes);
        CacheBlock* _allocate(size_t bytes);
        void _deallocate(CacheBlock* ptr);

        void _addCache(BufferCache* cache);
        void _removeCache(BufferCache* cache);
        BufferCache* _getCache();

        void _increaseSize(size_t bytes);
        void _decreaseSize(size_t bytes);

        typedef std::set<BufferCache*> CacheList;
        boost::mutex _lock;
        CacheList _caches;
        boost::thread_specific_ptr<BufferCache> _threadCache;

        bool _enabled;
        size_t _maxThreadBytes;
        size_t _maxThreadBlocks;
        size_t _maxThreadAge;

        size_t _hits;
        size_t _misses;

        volatile size_t _currentBytes;
        volatile size_t _highWaterBytes;

        static BufferManager _instance;
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
