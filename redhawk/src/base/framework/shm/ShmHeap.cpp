#include <ossie/shm/ShmHeap.h>
#include "ShmArena.h"
#include "ShmBlock.h"
#include "ThreadState.h"

#include <stdexcept>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <vector>

#include <unistd.h>

#include <boost/thread.hpp>

using namespace redhawk;

#define PAGE_ROUND_DOWN(x,p) ((x/p)*p)
#define PAGE_ROUND_UP(x,p) (((x+p-1)/p)*p)

class ShmHeap::PrivateHeap {
public:
    PrivateHeap(int id, ShmHeap* heap) :
        _id(id),
        _heap(heap)
    {
    }

    void* allocate(size_t bytes)
    {
        // NB: Thread-specific state may not be needed with per- CPU private
        //     heaps; it is maintained here to avoid modifying the arena API
        //     (for now)
        ThreadState* state = _heap->_getThreadState();
        boost::mutex::scoped_lock lock(_mutex);
        for (ArenaList::iterator arena = _arenas.begin(); arena != _arenas.end(); ++arena) {
            void* ptr = (*arena)->allocate(state, bytes);
            if (ptr) {
                // Move the successful arena to the front of the list, under
                // the assumption that it is more likely to satisfy a future
                // request
                std::iter_swap(arena, _arenas.begin());
                return ptr;
            }
        }

        ShmArena* arena = _heap->_createArena(bytes);
        _arenas.insert(_arenas.begin(), arena);
        return arena->allocate(state, bytes);
    }

private:
    int _id;
    ShmHeap* _heap;

    boost::mutex _mutex;
    typedef std::vector<ShmArena*> ArenaList;
    ArenaList _arenas;
};

ShmHeap::ShmHeap(const std::string& name) :
    _shm(name)
{
    _shm.create();
    int nprocs = sysconf(_SC_NPROCESSORS_CONF);
    for (int id = 0; id < nprocs; ++id) {
        _allocs.push_back(new PrivateHeap(id, this));
    }
}

ShmHeap::~ShmHeap()
{
#ifdef HEAP_DEBUG
    std::cout << _arenas.size() << " arenas" << std::endl;
    std::cout << _shm.size() << " total bytes" << std::endl;
#endif
    unlink();
}

void* ShmHeap::allocate(size_t bytes)
{
    PrivateHeap* heap = _getPrivateHeap();
    return heap->allocate(bytes);
}

void ShmHeap::deallocate(void* ptr)
{
    ShmArena::deallocate(ptr);
}

void ShmHeap::unlink()
{
    _shm.unlink();
}

ShmHeap::ID ShmHeap::getID(void* ptr)
{
    ShmBlock* block = ShmBlock::from_pointer(ptr);
    ID id;
    id.arena = block->arena()->offset();
    id.offset = block->offset();
    return id;
}

const std::string& ShmHeap::name() const
{
    return _shm.name();
}

ShmHeap::PrivateHeap* ShmHeap::_getPrivateHeap()
{
    size_t cpuid = sched_getcpu();
    assert(cpuid < _allocs.size());
    return _allocs[cpuid];
}

ThreadState* ShmHeap::_getThreadState()
{
    ThreadState* state = _threadState.get();
    if (!state) {
        state = new ThreadState();
        _threadState.reset(state);
    }
    return state;
}

ShmArena* ShmHeap::_createArena(size_t minSize)
{
    boost::mutex::scoped_lock lock(_mutex);
    size_t arena_size = DEFAULT_ARENA_SIZE;
    const char* arena_size_env = getenv("ARENA_SIZE");
    if (arena_size_env) {
        char* end;
        arena_size = strtoll(arena_size_env, &end, 10);
        if ((arena_size == 0) || (*end != '\0')) {
            std::cerr << "Invalid arena size, using default" << std::endl;
            arena_size = DEFAULT_ARENA_SIZE;
        } else {
            // Shared memory should be allocated along page boundaries
            arena_size = PAGE_ROUND_UP(arena_size, ShmFile::PAGE_SIZE);
            std::cout << "Using arena size " << arena_size << std::endl;
        }
    }

    // Ensure that the arena is large enough for the request, accounting for
    // the overhead of the block metadata (roughly)
    // TODO: Should extra large requests be handled differently? In glibc,
    //       above a certain size it starts using mmap/munmap. As a quick "fix"
    //       use a minimum of 2 blocks plus overhead.
    minSize = (minSize + 64) * 2;
    if (minSize > arena_size) {
        arena_size = PAGE_ROUND_UP(minSize, ShmFile::PAGE_SIZE);
    }

    // Allocate 1 page for the index, plus the arena memory
    size_t current_offset = _shm.size();
    size_t total_size = ShmFile::PAGE_SIZE + arena_size;
    _shm.resize(current_offset + total_size);

    void* base = _shm.map(total_size, ShmFile::READWRITE, current_offset);
    ShmArena* arena = new (base) ShmArena(current_offset, arena_size);
    _arenas.push_back(arena);
    return arena;
}

ShmHeapClient::ShmHeapClient(const std::string& name) :
    _shm(name)
{
    _shm.open();
}

void* ShmHeapClient::fetch(ShmHeap::ID id)
{
    ShmArena* arena = 0;
    ArenaMap::iterator iter = _arenas.find(id.arena);
    if (iter != _arenas.end()) {
        arena = iter->second;
    } else {
        arena = _attach(id.arena);
    }
    return arena->attach(id.offset);
}

void ShmHeapClient::deallocate(void* ptr)
{
    ShmArena::deallocate(ptr);
}

ShmArena* ShmHeapClient::_attach(size_t offset)
{
    // Map just the arena's index to get the complete size
    void* base = _shm.map(ShmFile::PAGE_SIZE, ShmFile::READWRITE, offset);
    ShmArena* arena = reinterpret_cast<ShmArena*>(base);
    size_t arena_size = arena->size();

    // Remap to get the full arena size
    base = _shm.remap(base, ShmFile::PAGE_SIZE, ShmFile::PAGE_SIZE + arena_size);
    arena = reinterpret_cast<ShmArena*>(base);

    // Store mapping
    _arenas[arena->offset()] = arena;
    return arena;
}
