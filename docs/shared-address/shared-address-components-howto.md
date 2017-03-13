# Shared Address Space Components HOWTO
Instructions for getting up and running with shared address space
components.

Creating a Shared Library Component
===================================

In the REDHAWK IDE, create a component project as you normally would,
selecting C++ as the language. C++ components default to a shared
library implementation.

Converting a Component to a Shared Library
------------------------------------------

1.  Open up the component SPD in the editor and select the
    **Implementations** tab.
2.  In the right hand side of the pane, expand the **Code** section.
3.  Change **Type** to be "SharedLibrary"
4.  Add ".so" to the end of **Entry Point** and **File**
5.  Re-generate the component

Other than changing the code entry, everything else is the same. When
you generate the component, it will produce different
`Makefile.am, configure.ac` and `main.cpp` files. Make sure there are
overwritten.

Running a Shared Library Component
==================================

Shared library support is built in to both the Python Sandbox and IDE
Chalkboard. Launch the component the same way as always (by path or by
name); the Sandbox/Chalkboard recognizes that the component is a shared
library and, if necessary, launches the ComponentHost soft package. The
component is then deployed into the ComponentHost. All shared library
components in the Sandbox/Chalkboard run in the same ComponentHost.

Within the REDHAWK IDE, to terminate the ComponentHost instance, click
the "Release Waveform" button from the toolbar.

BulkIO Memory Sharing
=====================

This is where the bulk (no pun intended) of the difference comes in. To
avoid copies, you have to use the new shared buffer classes and API.

There are two STL-like template classes for working with shared data:

-   `redhawk::shared_buffer<T>` - read-only buffer
-   `redhawk::buffer<T>` - adds write operations to
    `redhawk::shared_buffer`

Buffer classes have reference semantics, which means that assignment
shares the underlying buffer instead of making a deep copy. Multiple
buffer classes can point to the same underlying memory; the memory is
freed only once there are no more references. For this reason, buffers
are memory leak-proof (unless you create them as pointers view the `new`
operator).

The intent is that new data starts out as a `redhawk::buffer<T>`, which
then gracefully degrades to a `redhawk::shared_buffer<T>` once it's
passed to the port. This allows the originator to modify the data as
needed, but prevents any downstream receivers from modifying it, which
is necessary to avoid copies. Both classes are part of the core
libraries (not BulkIO), in `<ossie/shared_buffer.h>`, and have complete
Doxygen documentation.

Unlike `std::vector`, the size is fixed at creation time.

BulkIO Output Ports
-------------------

Using the output stream API is strongly recommended, but all of the
numeric port classes have an overload of `pushPacket()` that takes a
shared buffer for the data argument.

To create a new writeable buffer, use the single-argument construtor
with the desired size (in number of elements) to allocate space:
```
redhawk::buffer<float> buffer(1024);
    for (size_t ii = 0; ii < buffer.size(); ++ii) {
        buffer[ii] = (float) ii;
    }
stream.write(buffer, bulkio::time::utils::now());
```

The buffer is shared with all local connections (which now have a
`redhawk::shared_buffer` that points to the same data), and transferred
over CORBA for remote connections (same IPC cost as before). Once a
`redhawk::buffer` has been shared with other contexts, in this case via
the `write()` method, **you must not modify it**.

If your algorithm requires history (besides overlap, which is supported
from the input stream), it's cheap to keep a read-only "copy" of your
output buffer:

```
redhawk::shared_buffer<float> history = buffer;
```

BulkIO Input Ports
------------------

To use this feature, you must use BulkIO input streams. The DataBlock
classes have been updated to use a `redhawk::shared_buffer` internally,
and provide access to the internal buffer in both scalar and complex
forms:

```
bulkio::InFloatStream stream = dataFloat_in->getCurrentStream();
    if (!stream) {
        return NOOP;
    }

    bulkio::FloatDataBlock block = stream.read();
    if (!block) {
        return NOOP;
    }

    if (block.complex()) {
        redhawk::shared_buffer<std::complex<float> > buffer = block.cxbuffer();
        std::complex<float> sum = std::accumulate(buffer.begin(), buffer.end(), std::complex<float>(0.0, 0.0));
    } else {
        redhawk::shared_buffer<float> buffer = block.buffer();
        float sum = std::accumulate(buffer.begin(), buffer.end(), 0.0);
    }

    return NORMAL;
```

You can pass along the input buffer to an output stream, and the same
sharing rules apply as above (though you don't have write access).

Advanced Usage
--------------

Beyond being a better array, buffers include some additional features
for low-level and signal processing use.

#### Externally Acquired Memory

You can wrap existing memory in a `redhawk::shared_buffer` or
`redhawk::buffer` (depending on whether you plan to modify it via the
buffer's API):
```
float* data = new float[1024];
redhawk::shared_buffer<float> buffer(data, 1024);
```

The buffer takes ownership of the memory, calling `delete[]` when the
last reference goes away.

#### Custom Deleter

You can customize the delete behavior by passing your own deleter as the
third argument:

```
float* data = (float*) malloc(sizeof(float) * 1024));
redhawk::shared_buffer<float> buffer(data, 1024, &std::free);
```

The deleter can be any callable object or function pointer that takes a
single argument, a pointer to the memory being deleted. The `std::free`
example is somewhat contrived, but it's easy to imagine a scenario like
SourceNic, where you may want to push portions of a larger slab of
memory to downstream components, and then be notified when the chunk
becomes available for DMA.

#### Slicing

Buffer classes provide a `slice()` method, which allows you to get a
subset of the data without making a copy. The returned buffer still
shares the underlying memory, but only provides access within the bounds
provided.

```
// NB: The arguments are the start and end indices.
redhawk::shared_buffer<float> part = buffer.slice(8, 24);
// part.size() == 16
```

#### Recast

The internal data is strongly typed, but you can reinterpret a buffer as
another type with the static method `recast()`, similar to C++'s
`reinterpret_cast`. The size of the elements are taken into account in
the returned buffer, with remainder always truncated.

```
redhawk::buffer<std::complex<short> > source(20); // 80 bytes
redhawk::shared_buffer<short> shorty = redhawk::shared_buffer<short>::recast(source);                                                        |
// shorty.size() == 40
```
