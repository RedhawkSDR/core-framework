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

#include <ossie/shm/Heap.h>
#include "Superblock.h"
#include "Block.h"
#include "ThreadState.h"
#include "HeapPolicy.h"
#include "Metrics.h"
#include "Environment.h"

#include <stdexcept>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <vector>

#include <unistd.h>

#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>

#define PAGE_ROUND_DOWN(x,p) ((x/p)*p)
#define PAGE_ROUND_UP(x,p) (((x+p-1)/p)*p)

namespace redhawk {
    namespace shm {
        namespace {
            static HeapPolicy* initializePolicy()
            {
                std::string policy_type = redhawk::env::getVariable("RH_SHMALLOC_POLICY", "thread");
                if (policy_type == "cpu") {
                    return new CPUHeapPolicy;
                } else {
                    if (policy_type != "thread") {
                        std::cerr << "Invalid SHM heap policy '" << policy_type << "'" << std::endl;
                    }
                    return new ThreadHeapPolicy;
                }
            }

            static boost::scoped_ptr<HeapPolicy> policy(initializePolicy());
        }
    }
}

using namespace redhawk::shm;

bool MemoryRef::operator! () const
{
    return heap.empty();
}

class Heap::Pool {
public:
    Pool(int id, Heap* heap) :
        _id(id),
        _heap(heap)
    {
        RECORD_SHM_METRIC(pools_created);
    }

    void* allocate(ThreadState* state, size_t bytes)
    {
        boost::mutex::scoped_lock lock(_mutex);
        for (SuperblockList::iterator superblock = _superblocks.begin(); superblock != _superblocks.end(); ++superblock) {
            void* ptr = (*superblock)->allocate(state, bytes);
            if (ptr) {
                // Move the successful superblock to the front of the list,
                // under the assumption that it is more likely to satisfy a
                // future request
                std::iter_swap(superblock, _superblocks.begin());
                RECORD_SHM_METRIC(pools_alloc_hot);
                return ptr;
            }
        }

        Superblock* superblock = _heap->_createSuperblock(bytes);
        if (superblock) {
            RECORD_SHM_METRIC_IF(_superblocks.empty(), pools_used);
            _superblocks.insert(_superblocks.begin(), superblock);
            RECORD_SHM_METRIC(pools_alloc_cold);
            return superblock->allocate(state, bytes);
        }

        RECORD_SHM_METRIC(pools_alloc_failed);
        return 0;
    }

private:
    int _id;
    Heap* _heap;

    boost::mutex _mutex;
    typedef std::vector<Superblock*> SuperblockList;
    SuperblockList _superblocks;
};

Heap::Heap(const std::string& name) :
    _file(name),
    _canGrow(true)
{
    _file.create();
    int num_pools = policy->getPoolCount();
    for (int id = 0; id < num_pools; ++id) {
        _allocs.push_back(new Pool(id, this));
    }

    RECORD_SHM_METRIC(heaps_created);
}

Heap::~Heap()
{
    // Remove the file when the owner exits; other processes connected to the
    // same superblock file will still be able to access everything, but no new
    // connections are possible
    try {
        _file.file().unlink();
    } catch (const std::exception&) {
        // It may have been removed from another context, nothing else to do
    }
}

void* Heap::allocate(size_t bytes)
{
    ThreadState* state = _getThreadState();
    Pool* pool = _getPool(state);
    return pool->allocate(state, bytes);
}

void Heap::deallocate(void* ptr)
{
    Superblock::deallocate(ptr);
}

MemoryRef Heap::getRef(const void* ptr)
{
    Block* block = Block::from_pointer(const_cast<void*>(ptr));
    MemoryRef ref;
    const Superblock* superblock = block->getSuperblock();
    if (superblock) {
        ref.heap = superblock->heap();
        ref.superblock = superblock->offset();
    } else {
        ref.heap = "";
        ref.superblock = 0;
    }
    ref.offset = block->offset();
    return ref;
}

const std::string& Heap::name() const
{
    return _file.name();
}

Heap::Pool* Heap::_getPool(ThreadState* state)
{
    size_t pool_id = policy->getPoolAssignment(state);
    assert(pool_id < _allocs.size());
    return _allocs[pool_id];
}

ThreadState* Heap::_getThreadState()
{
    ThreadState* state = _threadState.get();
    if (!state) {
        state = new ThreadState;
        _threadState.reset(state);
        policy->initThreadState(state);
    }
    return state;
}

Superblock* Heap::_createSuperblock(size_t minSize)
{
    boost::mutex::scoped_lock lock(_mutex);
    if (!_canGrow) {
        return 0;
    }

    // Ensure that the superblock is large enough for the request, accounting
    // for the overhead of the block metadata (roughly)
    size_t superblock_size = _superblockSize;
    minSize = (minSize + 64) * 2;
    if (minSize > superblock_size) {
        superblock_size = PAGE_ROUND_UP(minSize, MappedFile::PAGE_SIZE);
    }

    try {
        return _file.createSuperblock(superblock_size);
    } catch (const std::bad_alloc&) {
        _canGrow = false;
        return 0;
    }
}

size_t Heap::_initSuperblockSize()
{
    // We would prefer to use MappedFile::PAGE_SIZE here but the order of
    // initialization for C++ modules is undefined, meaning it may still be 0
    // when this function is called. Use the same system call instead.
    static size_t PAGE_SIZE = sysconf(_SC_PAGESIZE);
    size_t superblock_size = redhawk::env::getVariable("RH_SHMALLOC_SUPERBLOCK_SIZE", DEFAULT_SUPERBLOCK_SIZE);
    return PAGE_ROUND_UP(superblock_size, PAGE_SIZE);
}

size_t Heap::_superblockSize = Heap::_initSuperblockSize();
