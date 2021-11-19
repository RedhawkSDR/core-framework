# Shared Memory

REDHAWK uses POSIX shared memory to provide optimized data transfer for Bulk Input/Output (BulkIO) connections between C++ <abbr title="See Glossary.">components</abbr> or <abbr title="See Glossary.">devices</abbr> on the same host.
On Linux systems, POSIX shared memory is visible as a filesystem of type `tmpfs` mounted at `/dev/shm`.
Files on this filesystem are backed by RAM and are not written to disk.

The shared memory used by REDHAWK is organized into heaps, one per process, that exist as files in `/dev/shm`.
The heap is created on demand the first time the process attempts to allocate shared memory.
In normal operation, the heap is removed by the process when it exits.
Additionally, the REDHAWK <abbr title="See Glossary.">`GPP`</abbr> monitors its child processes and removes their heap files when the process crashes or is terminated abnormally.
As such, heaps are typically only left behind when a component or device crashes in the Python <abbr title="See Glossary.">sandbox</abbr> or the IDE Chalkboard.
When this occurs, the heap is considered "orphaned."

If shared memory is fully utilized, performance may be degraded because C++ components and devices will not be able to allocate additional shared memory for BulkIO data transfers.
REDHAWK provides tools to help system maintainers view the state of their shared memory filesystem and remove unwanted files that are using shared memory.

## Inspecting Shared Memory State

The `redhawk-shminfo` program enables system maintainers to view the current state of their shared memory filesystem.

By default, it displays the total and current free shared memory amounts on the system, followed by a listing of REDHAWK heaps:
```bash
redhawk-shminfo
```

Example output:
```
/dev/shm:
  size: 7.7G
  free: 7.6G (98.7%)

heap-2286
  type:        REDHAWK heap
  file size:   60.1M
  heap size:   60.0M
  heap used:   32.0K (0.1%)
  creator:     2286
  orphaned:    false
  refcount:    2
  user:        redhawk
  group:       redhawk
  mode:        640
```


> **NOTE**  
> Viewing REDHAWK heaps owned by other users may require superuser privileges.  

If a heap is listed as orphaned, the process that created it is no longer alive.
Under normal circumstances, the heap is removed by the creating process or the REDHAWK GPP upon exit.

> **NOTE**  
> When a shared memory file is removed, other processes that have mapped the memory can still access it, but no new processes may attach to it.  
> The memory will not be returned to the free shared memory total until all attached processes have unmapped the memory or exited.

### Viewing All Shared Memory Files

Other programs on the system may use shared memory as well.
Although they are not listed when using `redhawk-shminfo` in its default mode, any memory that is in use by these files is counted against the free shared memory.

To view all shared memory files, use the `--all` or `-a` flag:
```bash
redhawk-shminfo -a
```

Example output:
```
/dev/shm
  size: 7.7G
  free: 7.6G (98.4%)

<...output elided...>

pulse-shm-2249902370
  type:        other
  file size:   64.0M
  allocated:   4.0K
  user:        gdm
  group:       gdm
  mode:        400
```

REDHAWK heaps that cannot be read by the current user are displayed as files of type "other" when using `--all`.

> **NOTE**  
> Only the allocated size of files is counted against free memory.
> Shared memory files are sparse, meaning that no physical memory is dedicated until it is used.
> The total of all file sizes may therefore exceed the total shared memory on the system.

Because the free shared memory takes into account files that were removed but are still mapped by active processes, it may be less than the total shared memory minus the sum of all REDHAWK heaps and other shared memory files.
This memory will be reclaimed when the processes exit.

## Cleaning Shared Memory With `redhawk-shmclean`

The `redhawk-shmclean` tool can remove orphaned heaps and other shared memory files.
With no arguments, it scans the entire shared memory filesystem and removes all orphaned heaps.

```bash
redhawk-shmclean
```

Example output:
```
unlinking heap-2286
```


> **NOTE**  
> Removing REDHAWK heaps and shared memory files owned by other users may require superuser privileges.  

### Removing Individual Heaps

It is possible to remove one or more individual heaps by giving the heap names as arguments to `redhawk-shmclean`:

```bash
redhawk-shmclean heap-2286
```

If the heap is not orphaned (that is, its creating process is still alive), `redhawk-shmclean` will refuse to remove it unless the `--force` or `-f` flag is given.

### Removing Non-REDHAWK Files

Because `/dev/shm` behaves like a regular UNIX filesystem, shared memory files can be removed with `rm`.
It is also possible to remove these files with `redhawk-shmclean` using the `--force` or `-f` flag.
Filenames must be relative to `/dev/shm`.
