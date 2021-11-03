# Signal Related Information (SRI)

SRI is delivered with the data (when in-band) that describes the data payload. [SRI Keywords](#sri-keywords) provides guidance on how to manipulate keywords in SRI. The following table describes the SRI data structure fields.

##### SRI Data Structure Fields
| **Name** | **Type** | **Description**                                                                                                                            |
| :------- | :------- | :----------------------------------------------------------------------------------------------------------------------------------------- |
| hversion | `long`   | Version of the Stream SRI header. Set to 1.                       |
| xstart   | `double` | Specifies the start of the primary axis. (Refer to [SRI Fields for Contiguous Data](#sri-fields-for-contiguous-data) or [SRI Fields for Framed Data](#sri-fields-for-framed-data))                                                    |
| xdelta   | `double` | Specifies the interval along the primary axis. (Refer to [SRI Fields for Contiguous Data](#sri-fields-for-contiguous-data) or [SRI Fields for Framed Data](#sri-fields-for-framed-data)|
| xunits   | `short`  | Specifies the units associated with the xstart and xdelta values. Refer to the REDHAWK Interface Control Document (ICD) for definitions.|
| subsize  | `long`   | For contiguous data, 0. For framed data, specifies the number of data elements in each frame (i.e., the row length).  |
| ystart   | `double` | Specifies the start of the secondary axis. (Refer to [SRI Fields for Framed Data](#sri-fields-for-framed-data))|
| ydelta   | `double` | Specifies the interval along the secondary axis. Refer to ([SRI Fields for Framed Data](#sri-fields-for-framed-data)) |
| yunits   | `short`  | Specifies the units associated with the ystart and ydelta values. Refer to the REDHAWK ICD for definitions.  |
| mode     | `short`  | 0-Scalar, 1-Complex. Complex data is passed as interleaved I/Q values in the sequence. The type for the sequence remains the same for both real and complex data.  |
| streamID | `string` | Stream ID. Unique streams can be delivered over the same <abbr title="See Glossary.">port</abbr>, where each stream is identified by a unique string (generated or passed along by the provides side). The generation of this Stream ID is <abbr title="See Glossary.">Application</abbr>-specific and not controlled by the REDHAWK Core Framework (CF). |
| blocking | `boolean` | Flag to determine whether the receiving port exhibits back pressure. If this is false and the provides-side queue is full, the data is dumped. If this is true and the provides-side queue is full, the pushPacket() call blocks. |
| keywords | `sequence <CF::DataType>` | User defined keywords. This is a sequence of structures that contain an ID of type string and a value of type CORBA Any. The content of the CORBA Any can be any type. |

There are two modes of operation for Bulk Input/Output (BulkIO), contiguous or framed data, with subsize equal to zero or frame size, respectively.

### Contiguous Data

The most common use of BulkIO is to transfer contiguous data, typically digitized samples. The SRI subsize field must be set to 0. The primary axis is typically in units of time. The secondary axis is unused.  The following table describes the SRI fields for contiguous data.

##### SRI Fields for Contiguous Data
| **Name** | **Description**                                                                                                            |
| :------- | :------------------------------------------------------------------------------------------------------------------------- |
| xstart   | Specifies, in units identifed by xunits, the start time of the first sample, relative to the Unix epoch (January 1, 1970). |
| xdelta   | Specifies the interval between consecutive samples.                                                                        |
| xunits   | Specifies the units associated with the xstart and xdelta values.                                                          |
| subsize  | Set to 0.                                                                                                                  |
| ystart   | Not used.                                                                                                                  |
| ydelta   | Not used.                                                                                                                  |
| yunits   | Not used.                                                                                                                  |

### Framed Data

BulkIO supports framed data, such as the output of an Fast Fourier Transform (FFT), in which one dimension has a fixed size. The SRI subsize field is set to the frame length.  The following table describes the SRI fields for framed data.

##### SRI Fields for Framed Data
| **Name** | **Description**                                                                                                                                                                                                                                                                                                                                                                                                                         |
| :------- | :-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| xstart   | Specifies an abscissa-style value (i.e., relative to xunits) associated with the first element in each frame. For example, in streams containing a series of one-dimensional FFT results, each frame represents a frequency interval with xstart specifying the frequency associated with the lower end of the interval. For real-valued samples, xstart is typically zero, while for complex-valued samples, xstart is typically bw/2. |
| xdelta   | Specifies the interval between consecutive samples in a frame.                                                                                                                                                                                                                                                                                                                                                                          |
| xunits   | Specifies the units associated with the xstart and xdelta values.                                                                                                                                                                                                                                                                                                                                                                       |
| subsize  | Specifies the number of data elements in each frame (i.e., the row length).                                                                                                                                                                                                                                                                                                                                                             |
| ystart   | Interpreted the same way as the xstart field in contiguous data (Refer to [SRI Fields for Contiguous Data](#sri-fields-for-contiguous-data)), except that it refers to the start time of the first frame.                                                                                                                                                                                                                                                                                                |
| ydelta   | Specifies the interval between consecutive frames.                                                                                                                                                                                                                                                                                                                                                                                      |
| yunits   | Specifies the units associated with the ystart and ydelta values.                                                                                                                                                                                                                                                                                                                                                                       |

### SRI Transfer

SRI is transferred over a connection by the uses-side invoking the provides-side function `pushSRI()`. The `pushSRI()` function contains a single argument, an instance of an SRI object.

Each provides-side port that implements a BulkIO interface expects that SRI regarding data being received become available before any data is transferred. When using the code generators and base classes in the REDHAWK development tools, this behavior is hard-coded into the uses-side BulkIO ports. If the user code on the uses-side of a BulkIO connection does not explicitly invoke a `pushSRI()` call before any data is sent out, the auto-generated code creates a trivial SRI message with normalized values.

Part of the hard-coded behavior on the uses-side BulkIO port is to issue a `pushSRI()` when a new connection is made to the newly-connected object. For example, a system is created in which data is flowing between <abbr title="See Glossary.">components</abbr> A and B. As data is flowing between these components, a new connection is established between components A and C. When this connection is established, a `pushSRI()` method call is automatically made from component A to component C.

### SRI Keywords

SRI is metadata to describe the payload being pushed (for example, sampling period). While it is possible to describe some generic parameters, signal specific parameters are be stored in a generalized structure called SRI Keywords. SRI keywords are passed as a sequence of key/value pairs (`CF::DataType`) of type `CF::Properties`. In <abbr title="See Glossary.">properties</abbr>, the keys are strings, and the values are a CORBA type called `CORBA::Any`. `CORBA::Any` is a structure that can be used to marshal a wide variety of types. REDHAWK has developed helper APIs to interact with the keyword sequence.

### Adding SRI Keywords in C++, Python, and Java

Given a component with simple properties `chan_rf` and `col_rf` that are of type double and have an initial value of `-1`, and a BulkIO StreamSRI instance named `sri`, the following implementations in C++, Python and Java, push out those property values as the keywords `COL_RF` and `CHAN_RF`.

#### C++ Implementation
The `redhawk::PropertyMap` property map enables you to manipulate the sequence of keywords.

```cpp
include <ossie/PropertyMap.h>

redhawk::PropertyMap &tmp = redhawk::PropertyMap::cast(sri.keywords);
tmp["CHAN_RF"] = chan_rf;
tmp["COL_RF"] = col_rf;
```
#### Python Implementation
`omniORB` helpers `any.to_any` are used to convert a Python type to a `CORBA::Any`.

```python
from omniORB import any

self.sri.keywords.append(CF.DataType("CHAN_RF", any.to_any(self.chan_rf)))
self.sri.keywords.append(CF.DataType("COL_RF", any.to_any(self.col_rf)))
```
#### Java Implementation
The `AnyUtils` package is used to convert a Java type to a `CORBA::Any`.

```java
import org.ossie.properties.AnyUtils;

double chan_rf = this.chan_rf.getValue();
double col_rf = this.col_rf.getValue();
sri.keywords = new DataType[2];
sri.keywords[0] = new DataType("CHAN_RF", AnyUtils.toAny(chan_rf, TCKind.tk_double));
sri.keywords[1] = new DataType("COL_RF", AnyUtils.toAny(col_rf, TCKind.tk_double));
```

### Verifying SRI Keywords

It is possible to verify the keywords and values being pushed out by connecting a `DataSink()` component in the Python <abbr title="See Glossary.">sandbox</abbr>. This assumes there is at least one BulkIO output port for the test component, and a `pushSRI()` call is made on that port. The following code demonstrates this verification:

```python
from ossie.utils import sb
comp = sb.launch("<component name>")
sink = sb.DataSink()
comp.connect(sink)
sb.start()
print sink.sri().keywords
```

#### Retrieving SRI Keywords in C++

Because `redhawk::PropertyMap` contains `CORBA::Any` values, retrieving the contents requires the use of getters to convert to a native type. Assuming that the content of a particular keyword is a `double`:

```cpp
redhawk::PropertyMap &tmp = redhawk::PropertyMap::cast(sri.keywords);
chan_rf = tmp["CHAN_RF"].toDouble();
```
