# Working with Complex Data

Each `BurstPacket` of the incoming data provides the `getComplex()` method to denote if the vector contains complex samples (It is comprised of real and imaginary parts.) Complex data is sent as alternating real and imaginary values. A developer can work with this data in any fashion; however, this section describes the common methods for converting the data into a more workable form.

### Converting Complex Data in C++

In C++, the incoming Burst Input/Output (BurstIO) data vector may be typecast into a std::vector of complex values. For example:

```cpp
BurstShortIn::BurstPacket *pkt = myShortPort->getPacket(bulkio::Const::BLOCKING);
if ( pkt->isComplex() ) {
  BurstShortIn::ComplexType cplx_data = pkt->getComplexData();

  // ... do some processing with cplx_data
}
```

### Converting Complex Data in Python

The helper functions `bulkioComplexToPythonComplexList` and `pythonComplexListToBulkioComplex`, defined in the module `ossie.utils.bulkio.bulkio_helpers`, provide an efficient translation to and from lists of Python complex numbers.

### Converting Complex Data in Java

Unlike with C++ and Python, Java does not have an ubiquitous means for representing complex numbers; therefore, when using Java, users are free to map the incoming BurstIO `getData()` method to the complex data representation of their choosing.
