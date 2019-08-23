# Shared Memory Metrics

The REDHAWK shared memory allocator policy controls provide a number of ways to tune shared memory usage for a system.
To assist developers in determining the best performance tradeoffs for a particular use case, the shared memory subsystem can collect runtime metrics on a per-process basis.

**Note:** Shared memory metrics are a new feature in REDHAWK 2.2.4. Setting the environment variable described below has no effect on earlier releases of REDHAWK.

## System Information

REDHAWK includes a `redhawk-shminfo` program that provides general information about the system's shared memory state, including REDHAWK heaps.
For more information about using the `redhawk-shminfo` program, refer to the Shared Memory Maintenance appendix of the REDHAWK manual.

## Enabling

Shared memory metrics are disabled by default.
Metrics may be enabled by setting the environment variable `RH_SHMALLOC_METRICS` to any value except `disable`.
A value of "1" is recommended for readability.

The following example enables metrics for all REDHAWK processes launched from the current shell:
```sh
export RH_SHMALLOC_METRICS=1
```

The performance impact of enabling shared memory metrics is typically negligible, but it requires atomic writes to global variables that may have an effect on highly-tasked multithreaded applications.

## Reports

If metrics are enabled, when a process exits, it dumps the collected metrics to the console.

The following example displays the output from a typical single-process component:
```
SHM metrics (24003):
  Executable: /var/redhawk/sdr/dom/components/rh/SigGen/cpp/SigGen
  Files created: 1
  Files opened: 0
  Files closed: 0
  Files total bytes: 2101248
  Heaps created: 1
  Pools created: 8
  Pools used: 1
  Pool allocations hot: 8090
  Pool allocations cold: 1
  Pool allocations failed: 0
  Superblocks created: 1
  Superblocks mapped: 0
  Superblocks reused: 0
  Superblocks unmapped: 0
  Heap clients created: 0
  Heap clients destroyed: 0
  Blocks created: 8091
  Blocks attached: 0
  Blocks released: 8091
  Blocks destroyed: 695
```

The process ID is included in the header, along with the path to the executable, to help distinguish which components are being reported.

## File Metrics

File metrics report information about the heap files created and used by a process.

### Files Created

Number of heap files created by this process.

### Files Opened

Number of heap files opened by this process.

### Files closed

Number of heap files closed by this process.

The heap file created by a process may not be included in this total depending on the order in which process cleanup executes.

### Files Total Bytes

Total number of bytes allocated in heap files created by this process.

Higher numbers mean that a process is using more shared memory.
If this is due to inefficient use of shared memory, tuning the allocator policy may reduce this number.

### Heaps Created

Number of heaps created by this process.

The number of heaps created should be equal to the number of heap files created.

## Pool Metrics

Pool metrics report the distribution of memory within a heap.
Memory assigned to one pool cannot be used from another pool.

### Pools Created

Number of pools created in this process.

Pool creation occurs at heap initialization time.
No shared memory is dedicated to a pool until it is used for allocation.

### Pools used

Number of pools in use by this process.

Indicates the actual usage pattern of pools.
More pools in use correlates with higher memory use.
With fewer threads, this may be inefficient; however, in a highly-threaded process, more pools reduce contention.

### Pool Allocations Hot

Number of pool allocations that were satisfied by an existing superblock.

A high number of "hot" allocations suggests that memory is being allocated efficiently.
Accordingly, the virtual address space and heap file tends to grow more slowly.

### Pool Allocations Cold

Number of pool allocations that required acquiring a new superblock.

A high number of "cold" allocations suggests either heavy memory use or inefficient allocation of memory.
The virtual address space and superblock file tends to grow more quickly.

### Pool Allocations Failed

Number of pool allocations that were not able to be fulfilled.

A failed allocation indicates that the shared memory filesystem is full.
Once this occurs, the system switches to using process-local memory and copy-in/copy-out IPC.

## Superblock Metrics

Superblock metrics reflect the usage of large, contiguous memory regions.
In a process that allocates shared memory, superblocks extend the size of the heap file.
In a process that attaches to shared memory, superblocks are mapped into the process virtual memory space.

### Superblocks Created

Number of superblocks created by this process.

A superblock is created by a heap owned by this process.
A higher number of superblocks created indicates more shared memory usage.

### Superblocks Mapped

Number of superblocks mapped into this process' memory space.

A mapped superblocks references another process' heap.
A higher number of superblocks mapped indicates more virtual address space usage.

### Superblocks Reused

Number of superblock mappings that were reused.

A superblock is reused whenever a mapping is requested that has already been fulfilled.
A high ratio of reused to mapped superblocks suggests efficient allocation.

### Superblocks Unmapped

Number of superblocks unmapped from this process' memory space.

Currently, superblocks are never unmapped.
This number is always zero.

## Heap Client Metrics

Heap clients manage access to heaps owned by other processes.

### Heap Clients Created

Number of heap clients created.

Each heap client maintains its own mapping of heaps from other processes.
No state is shared between heap clients, meaning that the same superblock may be mapped multiple times in a single process with more than one heap client.

### Heap Clients Destroyed

Number of heap clients destroyed.

In practice, at process exit, this should be equal to the number of heap clients created.

## Block Metrics

Block metrics give insight into the life cycle of an allocated chunk of memory.

### Blocks Created

Number of shared memory blocks allocated within this process.

### Blocks Attached

Number of shared memory blocks to which this process has attached.

Each attach call increments the block's reference count.
Across the system, the same block may have several clients attach to it.

### Blocks Released

Number of shared memory blocks from which this process has detached.

Each detach call decrements the block's reference count.
If the reference count dropped to zero when this process detached, it was also destroyed by this process.
For the entire system, the number of blocks released should equal the total of blocks created and attached.

### Blocks Destroyed

Number of shared memory blocks returned to their superblock's free list within this process.

Each block is created and destroyed exactly once across the system.
The process that destroys a block is not necessarily the same process that created it.
For the entire system, the number of blocks destroyed should equal the number of blocks created.
