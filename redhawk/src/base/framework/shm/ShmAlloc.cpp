#include <ossie/shm/ShmAlloc.h>
#include <ossie/shm/ShmHeap.h>

#include <boost/thread.hpp>

#include "ShmArena.h"

using namespace redhawk;

static boost::once_flag heapInit = BOOST_ONCE_INIT;
boost::scoped_ptr<ShmHeap> ShmAllocator::_heap(0);

void* ShmAllocator::allocate(size_t bytes)
{
    return Heap().allocate(bytes);
}

void ShmAllocator::deallocate(void* ptr)
{
    Heap().deallocate(ptr);
}

std::string ShmAllocator::name()
{
    return Heap().name();
}

ShmHeap::ID ShmAllocator::getID(void* ptr)
{
    return Heap().getID(ptr);
}

ShmHeap& ShmAllocator::Heap()
{
    boost::call_once(heapInit, &ShmAllocator::_initialize);
    return *_heap;
}

void ShmAllocator::_initialize()
{
    std::ostringstream oss;
    oss << "heap-" << getpid();
    _heap.reset(new ShmHeap(oss.str()));
}
