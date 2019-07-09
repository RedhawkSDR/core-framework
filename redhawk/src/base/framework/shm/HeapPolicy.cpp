#include "HeapPolicy.h"

#include <algorithm>
#include <iostream>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include <boost/lexical_cast.hpp>

namespace {
    template <typename T>
    static T getEnvironment(const char* var, T defval)
    {
        const char* env_value = getenv(var);
        if (env_value && (*env_value != '\0')) {
            try {
                return boost::lexical_cast<T>(env_value);
            } catch (...) {
                std::cerr << "Invalid value for " << var << ": '" << env_value << "'" << std::endl;
            }
        }

        return defval;
    }

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
    int cpus_per_heap = getEnvironment("RH_SHMALLOC_CPUS_PER_HEAP", 1);
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
    int num_heaps = getEnvironment("RH_SHMALLOC_THREAD_NUM_HEAPS", cpu_count);
    return std::max(num_heaps, 1);
}
