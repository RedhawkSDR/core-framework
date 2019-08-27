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
    _cpusPerPool(_getCpusPerPool())
{
}

int CPUHeapPolicy::getPoolCount()
{
    // If CPUs per pool is not an exact divisor of the total CPUs, ensure that
    // we round up to the next integral number
    return (getCpuCount() + _cpusPerPool - 1) / _cpusPerPool;
}

size_t CPUHeapPolicy::getPoolAssignment(ThreadState*)
{
    size_t cpuid = sched_getcpu();
    return cpuid / _cpusPerPool;
}

size_t CPUHeapPolicy::_getCpusPerPool()
{
    int cpus_per_pool = redhawk::env::getVariable("RH_SHMALLOC_CPUS_PER_POOL", 1);
    cpus_per_pool = std::max(cpus_per_pool, 1);
    int cpu_count = getCpuCount();
    cpus_per_pool = std::min(cpu_count, cpus_per_pool);

    int remainder = cpu_count % cpus_per_pool;
    if (remainder) {
        std::cerr << "SHM allocator: uneven distribution for CPUs ("
                  << cpu_count << " total, "
                  << cpus_per_pool << " per pool, "
                  << remainder << " in last pool)" << std::endl;
    }
    return cpus_per_pool;
}

ThreadHeapPolicy::ThreadHeapPolicy() :
    _numPools(_getNumPools())
{
}

int ThreadHeapPolicy::getPoolCount()
{
    return _numPools;
}

size_t ThreadHeapPolicy::getPoolAssignment(ThreadState* state)
{
    return state->poolId;
}

void ThreadHeapPolicy::initThreadState(ThreadState* state)
{
    pid_t tid = syscall(SYS_gettid);
    state->poolId = tid % _numPools;
}

size_t ThreadHeapPolicy::_getNumPools()
{
    int cpu_count = getCpuCount();
    // As a general rule of thumb, assume 1 thread per CPU, up to 8 total. This
    // gives enough pools to have a low probability of inter-thread contention
    // in the shared address space case, without splaying too much memory
    // across a large number of pools with a single-threaded component that
    // frequently stops and starts.
    int num_pools = std::min(cpu_count, 8);
    num_pools = redhawk::env::getVariable("RH_SHMALLOC_THREAD_NUM_POOLS", num_pools);
    return std::max(num_pools, 1);
}
