# Shared Memory Tuning

The REDHAWK shared memory allocator differs from traditional single-process allocators in the way that it manages memory.
Memory that is allocated by one process is often deallocated by another process at some indeterminite point in the future.
The allocator is designed to handle frequent allocations of large memory blocks while minimizing locking contention and inter-process coordination.
As this can lead to higher memory use, REDHAWK provides some controls that allow a system integrator to tune the shared memory allocator for their specific use case.

**Note:** Shared memory tuning options are a new feature in REDHAWK 2.2.4.
Setting the environment variables described below has no effect on earlier releases of REDHAWK.

## Heaps

Each REDHAWK process that allocates shared memory creates its own per-process heap using POSIX real-time shared memory.
To minimize contention between threads when allocating from shared memory, heaps are subdivided into pools.
Each allocation is serviced by one pool.

The overall shared memory usage of a process is related to the sharing of pools across allocations.
Less sharing reduces contention at the cost of more shared memory usage.
More sharing trades an increased potential for contention in order to limit shared memory usage.

### Superblocks

Shared memory is dedicated to pools in contiguous regions called superblocks.
The pool may then divide a superblock into smaller blocks to satisfy allocations.
Once a superblock has been assigned to a pool, it remains there for the lifetime of the heap.

The minimum size for superblocks can be configured with the `RH_SHMALLOC_SUPERBLOCK_SIZE` environment variable.
A superblock can be created beyond the minimum size to accomodate large allocations.
The default minimum superblock size is 2MB.
The value is rounded to the nearest page size for the system.

## Allocator Policy Control

To give system deployers maximum flexibility with shared memory, REDHAWK provides two policies for assigning memory to allocations:

* Thread-based
* CPU-based

The allocator policy can be set with the `RH_SHMALLOC_POLICY` environment variable.
If not set, it defaults to the thread-based policy.

The following example sets the allocator policy to CPU-based:
```sh
export RH_SHMALLOC_POLICY=cpu
```

**Note:** REDHAWK 2.2.0 through 2.2.3 used the CPU-based policy.

### Thread-Based Policy

In the thread-based heap policy, each thread is assigned to a pool based on its thread ID.
All allocations from a thread use the same pool.
This is the default policy, but it can be explicitly configured by setting `RH_SHMALLOC_POLICY` to `thread`.

By default, in the thread-based policy, the heap creates one pool per CPU, up to a maximum of 8.
The total number of pools can be controlled with the `RH_SHMALLOC_THREAD_NUM_POOLS` environment variable.

Using fewer pools may limit the total memory usage of a process by increasing the likelihood that different threads use the same pool.
For legacy components that run as a single processing thread in a standalone process, setting `RH_SHMALLOC_THREAD_NUM_POOLS` to `1` uses the least total shared memory.

```sh
export RH_SHMALLOC_THREAD_NUM_POOLS=1
```

### CPU-Based Policy

In the CPU-based heap policy, pools are assigned to one or more CPUs.
Each allocation is handled by the pool associated with its current CPU.
To use the CPU-based policy, set `RH_SHMALLOC_POLICY` to `cpu`.

By default, the heap creates one pool per CPU.
The ratio of CPUs to pools can be adjusted with the `RH_SHMALLOC_CPUS_PER_POOL` environment variable.
The following example assigns 4 CPUs to each pool:
```sh
export RH_SHMALLOC_CPUS_PER_POOL=4
```
In a system with 16 CPUs this creates 4 pools, with CPUs 0 through 3 assigned to the first pool, CPUs 4 through 7 assigned to the second, and so on.

Using the CPU policy can reduce contention with large numbers of threads and is more likely to use nearby memory on NUMA systems.
