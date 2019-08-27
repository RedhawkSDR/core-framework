# Shared Memory IPC
Instructions for taking advantage of zero-copy shared memory transfers in C++
components and devices.

Using Shared Memory
===================

BulkIO connections between C++ components or devices in different processes
on the same host automatically use shared memory to transfer data.
In order to make optimal use of shared memory, components should follow the
advice in the Shared Address Space Components HOWTO guide. In particular, the
section BulkIO Memory Sharing describes the preferred classes and API methods
for working with data buffers.

With no further modifications to user code, the `redhawk::buffer<T>` template
class acquires its memory from a process-shared heap, enabling shared memory-aware 
connections to transfer only a small amount of metadata to pass buffers
between processes.

Cleaning Up After Crashed Components
====================================
Each REDHAWK process that allocates shared memory creates its own per-process
heap using POSIX real-time shared memory. The file is automatically unlinked
when it is no longer in use; however, if components crash or are terminated
with a kill signal, the clean up does not occur.

Orphaned heaps are visible via the `/dev/shm` file system, with a filename of
`heap-<pid>`. If the process is no longer alive, these files can be removed
with the `rm` command.

Interprocess communication is done via FIFOs created on the local `/tmp` file
system. To avoid polluting the file system, these files are removed once the
communication channel is established, or the connection is canceled due to
error. In the unlikely event that a FIFO is not removed by REDHAWK, it may be
manually cleaned up by removing the file, which has a name with the following format:
`/tmp/fifo-<pid>-<hex number>`.
