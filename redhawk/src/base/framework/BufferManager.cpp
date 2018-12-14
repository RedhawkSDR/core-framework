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

#include <iostream>

#include <ossie/BufferManager.h>
#include "inplace_list.h"

using redhawk::BufferManager;

struct BufferManager::CacheBlock {
    CacheBlock(size_t size) :
        size(size),
        cache(0)
    {
    }

    static CacheBlock* from_pointer(void* ptr)
    {
        CacheBlock* block = reinterpret_cast<CacheBlock*>(ptr);
        return block - 1;
    }

    void* data()
    {
        return this + 1;
    }

    static size_t required_bytes(size_t bytes)
    {
        return bytes + sizeof(CacheBlock);
    }

    static size_t usable_bytes(size_t bytes)
    {
        return bytes - sizeof(CacheBlock);
    }

    const size_t size;
    BufferManager::BufferCache* cache;
};

class BufferManager::BufferCache {
public:
    // The CacheNode structure contains fields that are only required when a
    // memory block is being stored in the cache
    struct CacheNode : public CacheBlock {
        CacheNode* prev;
        CacheNode* next;
        size_t lastUsed;
    };

    BufferCache(BufferManager* manager) :
        _manager(manager),
        _enabled(true),
        _time(0),
        _maxBytes(-1),
        _maxBlocks(-1),
        _maxAge(-1),
        _hits(0),
        _misses(0),
        _currentBytes(0),
        _refcount(1)
    {
    }

    CacheBlock* fetch(size_t bytes)
    {
        boost::mutex::scoped_lock lock(_lock);
        CacheNode* node = _fetch(bytes);
        if (!node) {
            ++_misses;
            return 0;
        }

        ++_hits;
        _currentBytes -= node->size;
        _manager->_decreaseSize(node->size);
        return node;
    }

    void store(CacheBlock* block)
    {
        // Overlay the CacheNode struct on the block
        CacheNode* node = static_cast<CacheNode*>(block);
        boost::mutex::scoped_lock lock(_lock);
        node->lastUsed = ++_time;
        _cache.push_front(*node);
        _currentBytes += node->size;
        _manager->_increaseSize(node->size);
        _compact();
    }

    void enable(bool enabled)
    {
        boost::mutex::scoped_lock lock(_lock);
        _enabled = enabled;
        if (!_enabled) {
            _compact();
        }
    }

    void setMaxBytes(size_t bytes)
    {
        boost::mutex::scoped_lock lock(_lock);
        _maxBytes = bytes;
        _compact();
    }

    void setMaxBlocks(size_t blocks)
    {
        boost::mutex::scoped_lock lock(_lock);
        _maxBlocks = blocks;
        _compact();
    }

    void setMaxAge(size_t age)
    {
        boost::mutex::scoped_lock lock(_lock);
        _maxAge = age;
        _compact();
    }

    size_t hits()
    {
        return _hits;
    }

    size_t misses()
    {
        return _misses;
    }

    size_t size()
    {
        return _cache.size();
    }

    void incref()
    {
        __sync_fetch_and_add(&_refcount, 1);
    }

    bool decref()
    {
        size_t count = __sync_sub_and_fetch(&_refcount, 1);
        if (count == 0) {
            delete this;
            return false;
        }
        return true;
    }

    static void release(BufferCache* cache)
    {
        cache->decref();
    }

private:
    ~BufferCache()
    {
        _enabled = false;
        _compact();
        _manager->_removeCache(this);
    }

    inline CacheNode* _fetch(size_t bytes)
    {
        for (CacheList::iterator iter = _cache.begin(); iter != _cache.end(); ++iter) {
            if (iter->size == bytes) {
                CacheNode* node = iter.get_node();
                _cache.erase(iter);
                return node;
            }
        }
        return 0;
    }

    inline bool _overThreshold()
    {
        if (_cache.empty()) {
            return false;
        }
        if (!_enabled) {
            return true;
        }
        size_t age = _time - _cache.back().lastUsed;
        return (age > _maxAge) || (_currentBytes > _maxBytes) || (_cache.size() > _maxBlocks);
    }

    inline void _compact()
    {
        size_t previous = _currentBytes;
        while (_overThreshold()) {
            CacheNode* node = &_cache.back();
            _cache.pop_back();
            _currentBytes -= node->size;
            _manager->_deallocate(node);
        }
        size_t delta = previous - _currentBytes;
        if (delta > 0) {
            _manager->_decreaseSize(delta);
        }
    }

    BufferManager* _manager;
    boost::mutex _lock;
    typedef redhawk::inplace_list<CacheNode> CacheList;
    CacheList _cache;
    bool _enabled;
    size_t _time;

    size_t _maxBytes;
    size_t _maxBlocks;
    size_t _maxAge;
    size_t _hits;
    size_t _misses;
    size_t _currentBytes;
    volatile size_t _refcount;
};

BufferManager::BufferManager() :
    _threadCache(&BufferCache::release),
    _enabled(true),
    _maxThreadBytes(-1),
    _maxThreadBlocks(-1),
    _maxThreadAge(-1),
    _hits(0),
    _misses(0),
    _currentBytes(0),
    _highWaterBytes(0)
{
}

BufferManager::~BufferManager()
{
}

BufferManager& BufferManager::Instance()
{
    return _instance;
}

void* BufferManager::allocate(size_t bytes)
{
    bytes = _nearestSize(bytes);

    BufferCache* cache = 0;
    CacheBlock* block = 0;
    if (_enabled) {
        cache = _getCache();
        cache->incref();
        block = cache->fetch(bytes);
    }
    if (!block) {
        block = _allocate(bytes);
        block->cache = cache;
    }
    return block->data();
}

void BufferManager::deallocate(void* ptr)
{
    CacheBlock* block = CacheBlock::from_pointer(ptr);
    BufferCache* cache = block->cache;
    if (cache) {
        if (cache->decref() && _enabled) {
            cache->store(block);
            return;
        }
    }
    _deallocate(block);
}

size_t BufferManager::_nearestSize(size_t bytes)
{
    // Include cache overhead in rounding
    bytes = CacheBlock::required_bytes(bytes);
    if (bytes <= 128*1024) {
        // Up to 128K, round to nearest 1K
        bytes = (bytes + 1023) & ~1023;
    } else {
        // Round to nearest 4K
        bytes = (bytes + 4095) & ~4095;
    }
    // Remove cache overhead, leaving usable bytes
    return CacheBlock::usable_bytes(bytes);
}

BufferManager::CacheBlock* BufferManager::_allocate(size_t bytes)
{
    void* buffer = ::operator new(CacheBlock::required_bytes(bytes));
    return new (buffer) CacheBlock(bytes);
}

void BufferManager::_deallocate(CacheBlock* block)
{
    ::operator delete(block);
}

bool BufferManager::isEnabled() const
{
    return _enabled;
}

void BufferManager::enable(bool enabled)
{
    _enabled = enabled;
    boost::mutex::scoped_lock lock(_lock);
    for (CacheList::iterator ii = _caches.begin(); ii != _caches.end(); ++ii) {
        (*ii)->enable(enabled);
    }
}

size_t BufferManager::getMaxThreadBytes() const
{
    return _maxThreadBytes;
}

void BufferManager::setMaxThreadBytes(size_t bytes)
{
    boost::mutex::scoped_lock lock(_lock);
    _maxThreadBytes = bytes;
    for (CacheList::iterator ii = _caches.begin(); ii != _caches.end(); ++ii) {
        (*ii)->setMaxBytes(bytes);
    }
}

size_t BufferManager::getMaxThreadBlocks() const
{
    return _maxThreadBlocks;
}

void BufferManager::setMaxThreadBlocks(size_t blocks)
{
    boost::mutex::scoped_lock lock(_lock);
    _maxThreadBlocks = blocks;
    for (CacheList::iterator ii = _caches.begin(); ii != _caches.end(); ++ii) {
        (*ii)->setMaxBlocks(blocks);
    }
}

size_t BufferManager::getMaxThreadAge() const
{
    return _maxThreadAge;
}

void BufferManager::setMaxThreadAge(size_t age)
{
    boost::mutex::scoped_lock lock(_lock);
    _maxThreadAge = age;
    for (CacheList::iterator ii = _caches.begin(); ii != _caches.end(); ++ii) {
        (*ii)->setMaxAge(age);
    }
}

BufferManager::Statistics BufferManager::getStatistics()
{
    boost::mutex::scoped_lock lock(_lock);
    Statistics stats;
    stats.caches = _caches.size();
    stats.hits = _hits;
    stats.misses = _misses;
    stats.blocks = 0;
    stats.bytes = _currentBytes;
    stats.highBytes = _highWaterBytes;

    for (CacheList::iterator iter = _caches.begin(); iter != _caches.end(); ++iter) {
        BufferCache* cache = *iter;
        stats.hits += cache->hits();
        stats.misses += cache->misses();
        stats.blocks += cache->size();
    }
    return stats;
}

BufferManager::BufferCache* BufferManager::_getCache()
{
    BufferCache* cache = _threadCache.get();
    if (!cache) {
        cache = new BufferCache(this);
        cache->setMaxBytes(_maxThreadBytes);
        cache->setMaxBlocks(_maxThreadBlocks);
        cache->setMaxAge(_maxThreadAge);
        this->_addCache(cache);
        _threadCache.reset(cache);
    }
    return cache;
}

void BufferManager::_addCache(BufferCache* cache)
{
    boost::mutex::scoped_lock lock(_lock);
    _caches.insert(cache);
}

void BufferManager::_removeCache(BufferCache* cache)
{
    boost::mutex::scoped_lock lock(_lock);
    _hits += cache->hits();
    _misses += cache->misses();
    _caches.erase(cache);
}

void BufferManager::_increaseSize(size_t bytes)
{
    size_t current = __sync_add_and_fetch(&_currentBytes, bytes);
    size_t high = _highWaterBytes;
    while (current > high) {
        high = __sync_val_compare_and_swap(&_highWaterBytes, high, current);
    }
}

void BufferManager::_decreaseSize(size_t bytes)
{
    __sync_fetch_and_sub(&_currentBytes, bytes);
}

BufferManager BufferManager::_instance;
