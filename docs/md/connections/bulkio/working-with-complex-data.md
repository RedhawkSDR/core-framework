# Working with Complex Data

If the StreamSRI mode field of the incoming data is set to 1, the associated input data is complex (i.e., it is composed of real and imaginary parts). Complex data is sent as alternating real and imaginary values. A developer can work with this data in any fashion; however, this section provides common methods for converting the data into a more workable form.

### Converting Complex Data in C++


In C++, the incoming Bulk Input/Output (BulkIO) data block provides a `complex()` method to check whether the data is complex, and a `cxbuffer()` method to reinterpret the sample data as a `redhawk::shared_buffer` of `std::complex` values.
For example:

```cpp
bulkio::ShortDataBlock block = stream.read();
if (block.complex()) {
    redhawk::shared_buffer<std::complex<short> > data = block.cxbuffer();
    const size_t size = data.size();
}
```

### Converting Complex Data in Python

In Python, the incoming BulkIO data block provides a `complex` attribute to check whether the data is complex, and a `cxbuffer` attribute that gives the sample data as a list of Python `complex` values.
For example:

```python
block = stream.read()
if block.complex:
    data = block.cxbuffer
    size = len(data)
```

### Converting Complex Data in Java

Unlike with C++ and Python, Java does not have a ubiquitous means for representing complex numbers; therefore, when using Java, users are free to map the incoming BulkIO data to the complex data representation of their choosing.
