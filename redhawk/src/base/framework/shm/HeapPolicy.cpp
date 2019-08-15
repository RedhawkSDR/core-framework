#include "HeapPolicy.h"
#include "Environment.h"

#include <algorithm>
#include <iostream>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

namespace {
    static int getCpuCount()
    {
        static int count = sysconf(_SC_NPROCESSORS_CONF);
        return std::max(count, 1);
    }
}

using namespace redhawk::shm;

CPUHeapPolicy::CPUHeapPolicy() :
    _cpusPerHeap(_getCpusPerHeap())
{
}

int CPUHeapPolicy::getHeapCount()
{
    // If CPUs per heap is not an exact divisor of the total CPUs, ensure that
    // we round up to the next integral number
    return (getCpuCount() + _cpusPerHeap - 1) / _cpusPerHeap;
}

size_t CPUHeapPolicy::getHeapAssignment(ThreadState*)
{
    size_t cpuid = sched_getcpu();
    return cpuid / _cpusPerHeap;
}

size_t CPUHeapPolicy::_getCpusPerHeap()
{
    int cpus_per_heap = redhawk::env::getVariable("RH_SHMALLOC_CPUS_PER_HEAP", 1);
    cpus_per_heap = std::max(cpus_per_heap, 1);
    int cpu_count = getCpuCount();
    cpus_per_heap = std::min(cpu_count, cpus_per_heap);

    int remainder = cpu_count % cpus_per_heap;
    if (remainder) {
        std::cerr << "SHM allocator: uneven distribution for CPUs ("
                  << cpu_count << " total, "
                  << cpus_per_heap << " per heap, "
                  << remainder << " in last heap)" << std::endl;
    }
    return cpus_per_heap;
}

ThreadHeapPolicy::ThreadHeapPolicy() :
    _numHeaps(_getNumHeaps())
{
}

int ThreadHeapPolicy::getHeapCount()
{
    return _numHeaps;
}

size_t ThreadHeapPolicy::getHeapAssignment(ThreadState* state)
{
    return state->heapId;
}

void ThreadHeapPolicy::initThreadState(ThreadState* state)
{
    pid_t tid = syscall(SYS_gettid);
    state->heapId = tid % _numHeaps;
}

size_t ThreadHeapPolicy::_getNumHeaps()
{
    int cpu_count = getCpuCount();
    // As a general rule of thumb, assume 1 thread per CPU, up to 8 total. This
    // gives enough private heaps to have a low probability of inter-thread
    // contention in the shared address space case, without splaying too much
    // memory across a large number of private heaps with a single-threaded
    // component that frequently stops and starts.
    int num_heaps = std::min(cpu_count, 8);
    num_heaps = redhawk::env::getVariable("RH_SHMALLOC_THREAD_NUM_HEAPS", num_heaps);
    return std::max(num_heaps, 1);
}
