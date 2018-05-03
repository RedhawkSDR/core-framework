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

#include <stdexcept>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <vector>

#include <unistd.h>

#include <boost/thread.hpp>

using namespace redhawk::shm;

#define PAGE_ROUND_DOWN(x,p) ((x/p)*p)
#define PAGE_ROUND_UP(x,p) (((x+p-1)/p)*p)

bool MemoryRef::operator! () const
{
    return heap.empty();
}

class Heap::PrivateHeap {
public:
    PrivateHeap(int id, Heap* heap) :
        _id(id),
        _heap(heap)
    {
    }

    void* allocate(size_t bytes)
    {
        // NB: Thread-specific state may not be needed with per-CPU private
        //     heaps; it is maintained here to avoid modifying the superblock
        //     API (for now)
        ThreadState* state = _heap->_getThreadState();
        boost::mutex::scoped_lock lock(_mutex);
        for (SuperblockList::iterator superblock = _superblocks.begin(); superblock != _superblocks.end(); ++superblock) {
            void* ptr = (*superblock)->allocate(state, bytes);
            if (ptr) {
                // Move the successful superblock to the front of the list,
                // under the assumption that it is more likely to satisfy a
                // future request
                std::iter_swap(superblock, _superblocks.begin());
                return ptr;
            }
        }

        Superblock* superblock = _heap->_createSuperblock(bytes);
        if (superblock) {
            _superblocks.insert(_superblocks.begin(), superblock);
            return superblock->allocate(state, bytes);
        }

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
    int nprocs = sysconf(_SC_NPROCESSORS_CONF);
    for (int id = 0; id < nprocs; ++id) {
        _allocs.push_back(new PrivateHeap(id, this));
    }
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

#ifdef HEAP_DEBUG
    std::cout << _superblocks.size() << " superblocks" << std::endl;
    std::cout << _file.size() << " total bytes" << std::endl;
#endif
}

void* Heap::allocate(size_t bytes)
{
    PrivateHeap* heap = _getPrivateHeap();
    return heap->allocate(bytes);
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

Heap::PrivateHeap* Heap::_getPrivateHeap()
{
    size_t cpuid = sched_getcpu();
    assert(cpuid < _allocs.size());
    return _allocs[cpuid];
}

ThreadState* Heap::_getThreadState()
{
    ThreadState* state = _threadState.get();
    if (!state) {
        state = new ThreadState();
        _threadState.reset(state);
    }
    return state;
}

Superblock* Heap::_createSuperblock(size_t minSize)
{
    boost::mutex::scoped_lock lock(_mutex);
    if (!_canGrow) {
        return 0;
    }

    size_t superblock_size = DEFAULT_SUPERBLOCK_SIZE;
    const char* superblock_size_env = getenv("SUPERBLOCK_SIZE");
    if (superblock_size_env) {
        char* end;
        superblock_size = strtoll(superblock_size_env, &end, 10);
        if ((superblock_size == 0) || (*end != '\0')) {
            std::cerr << "Invalid superblock size, using default" << std::endl;
            superblock_size = DEFAULT_SUPERBLOCK_SIZE;
        } else {
            // Shared memory should be allocated along page boundaries
            superblock_size = PAGE_ROUND_UP(superblock_size, MappedFile::PAGE_SIZE);
            std::cout << "Using superblock size " << superblock_size << std::endl;
        }
    }

    // Ensure that the superblock is large enough for the request, accounting
    // for the overhead of the block metadata (roughly)
    // TODO: Should extra large requests be handled differently? In glibc,
    //       above a certain size it starts using mmap/munmap. As a quick "fix"
    //       use a minimum of 2 blocks plus overhead.
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
