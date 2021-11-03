# Examples

These two examples illustrate high-speed data exchange between two C++ <abbr title="See Glossary.">components</abbr> and basic data manipulation through the <abbr title="See Glossary.">sandbox</abbr>.

### High-speed data

In this example, two C++ components are created: a source and a sink. We will then deploy these components through the sandbox and evaluate the statistics of the data transfer between them.

1.  Create a C++ component called *source* with a uses <abbr title="See Glossary.">port</abbr> called *output* of type *dataShort*. Add a simple <abbr title="See Glossary.">property</abbr> with ID *xfer\_length*, type *ulong*, and default value of *100000*. Generate the component code.

2.  Open the file `source.h` and add the following members to the `source_i` class:

    ```cpp
    std::vector<short> data;
    bulkio::OutShortStream stream;
    ```

3.  Open the file `source.cpp` and edit it in the following ways:
      - In the `source_i` constructor:

        ```cpp
        data.resize(0);
        ```

      - In `serviceFunction()` comment-out the `LOG_DEBUG` statement and add the following lines:

        ```cpp
        if (data.size() != this->xfer_length) {
           data.resize(xfer_length);
        }
        if (!stream) {
           stream = output->createStream("sample");
        }
        BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();
        stream.write(data,tstamp);
        return NORMAL;
        ```
4.  Compile the component `source` and install it on <abbr title="See Glossary.">Target SDR</abbr>.

5.  Create a C++ component called *sink* with a provides port called *input* of type `dataShort`. Generate the component code.

6.  Open the file `sink.cpp` and edit it in the following ways:

    1.  In `serviceFunction()`, comment-out the `LOG_DEBUG` statement

    2.  Add the following lines:

        ```cpp
        bulkio::InShortStream stream = input->getCurrentStream();
        if (!stream) {
           return NOOP;
        }

        bulkio::ShortDataBlock block = stream.read();
        if (!block) {
           return NOOP;
        }

        return NORMAL;
        ```

7. Compile the component `sink` and install it on Target SDR.

8. Start a Python session in a command line terminal and run the following commands:

    ```python
    from ossie.utils import sb
    source = sb.launch("source")
    sink = sb.launch("sink")
    source.connect(sink)
    sb.start()
    print source._ports[0]._get_statistics()[0].statistics
    ```

The output of the print statement is an instance of the `PortStatistics` structure in Bulk Input/Output (BulkIO). This structure contains the statistics gathered from this connection. A measure of data rate is bits per second.

To display the number of Gigabits per second, run the following command:

```python
print source._ports[0]._get_statistics()[0].statistics.bitsPerSecond/1e9
```

The resulting value is the measured data transfer rate between the two components. The current `xfer_length` property can be viewed by typing the following:

```python
source.xfer_length
```

The default value is `100000`. Update the property to `200000` by running the following command:

```python
source.xfer_length = 200000
```

Check the new data rate by repeating the call to `_get_statistics()`. The resulting data rate is now different.

### Octet Ports

Octets are unsigned 8-bit units of data. In Java and C++, these map easily. However, that is not the case in Python, which treats a sequence of characters as a string. The following is an example of pushing Octet data out of a `dataOctet` port:

```python
import numpy
outputData = [1,2,3,4,5]
outputDataAsString = numpy.array(outputData, numpy.uint8).tostring()
self.port_output.pushPacket(outputDataAsString, T, EOS, streamID)
```

### Data manipulation

In this example, a Python component is created that takes vectors of floats as inputs, multiplies the vector by some arbitrary number, and then outputs the resulting vector. This example demonstrates some basic data manipulation as well as the interaction between the Python environment and the running component.

1.  Create a Python component called *mult* with a provides port called *input* of type *dataFloat* and a uses port called *output* of type *dataFloat*. Add a simple property with ID *factor*, type *float*, and default value of *1*. Generate the component code.

2.  Open the file `mult.py` and add the following lines:

    ```python
    data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed = self.port_input.getPacket()
    if data == None:
        return NOOP

    outData = []
    for value in data:
        outData.append(value*self.factor)
    if sriChanged:
        self.port_output.pushSRI(sri)

    self.port_output.pushPacket(outData, T, EOS, streamID)
    return NORMAL
    ```

3.  Save the project and drag the `mult` project to <abbr title="See Glossary.">**REDHAWK Explorer**</abbr>> **Target SDR**.

4.  Start a Python session in a command line terminal and run the following commands:

    ```python
    from ossie.utils import sb
    mult = sb.launch("mult")
    source = sb.DataSource(dataFormat="float")
    sink = sb.DataSink()
    source.connect(mult)
    mult.connect(sink)
    sb.start()
    source.push([1,2,3,4,5])
    sink.getData()
    ```
    Output:  
    ```python
    [1.0, 2.0, 3.0, 4.0, 5.0]
    ```

5.  The multiplication factor can be changed while the sandbox is up.

    ```python
    mult.factor = 2
    source.push([1,2,3,4,5])
    sink.getData()
    ```
    Output:
    ```python
    [2.0, 4.0, 6.0, 8.0, 10.0]
    ```
