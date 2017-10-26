#include <ossie/shm/ProcessHeap.h>
#include <ossie/shm/Heap.h>

#include <boost/thread.hpp>

using namespace redhawk::shm;

static boost::once_flag heapInit = BOOST_ONCE_INIT;
boost::scoped_ptr<Heap> ProcessHeap::_instance(0);

Heap& ProcessHeap::Instance()
{
    boost::call_once(heapInit, &ProcessHeap::_initialize);
    return *_instance;
}

void ProcessHeap::_initialize()
{
    std::ostringstream oss;
    oss << "heap-" << getpid();
    _instance.reset(new Heap(oss.str()));
}
