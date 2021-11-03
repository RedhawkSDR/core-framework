# pushPacket Data Flow


> **NOTE**  
> It is strongly recommended that for BulkIO data in C++ and Python, you use the Bulk Input/Output (BulkIO) stream API, which provides a high-level interface to sending and receiving data via BulkIO <abbr title="See Glossary.">ports</abbr>. For more information, refer to [Stream API](../bulkio/stream-api.html) and the Shared Address information in the [Technical Notes](<https://redhawksdr.github.io/technical-notes/>).  

Data transfers happen through the `pushPacket()` method call of a REDHAWK <abbr title="See Glossary.">component</abbr>'s <abbr title="See Glossary.">port</abbr> object. This method transfers the data from the uses-side port to the corresponding connected provides-side port. The data is marshaled by the middleware (omniORB) and placed on a queue for processing by the receiving component. The implementations of the `pushPacket()` methods are maximized for the efficiency of data throughput while providing network-accessible ingest/egress of data and minimizing the complexity of the implementation.
![BulkIO Data Flow via pushPacket()](../images/REDHAWK_bulkio_dataflow.png)

Each implementation maintains the required behavior of providing an Signal Related Information (SRI) object before receiving any data transfers. This is accomplished by calling the `pushSRI()` method of the port with an SRI object. In most cases, a component takes the ingest SRI object received from an input port, makes any required modifications as necessary, and passes this object down stream over its output port. If a component does not provide an SRI object before its first `pushPacket()`, the port creates a default SRI object with nominal values to pass out the port.

The following sections explain the different methods for transferring supported data types by a component.


> **WARNING**  
> For the current implementation of omniORB, the `/etc/omniORB.cfg` maintains the configurable maximum transfer size defined by the value for giopMaxMsgSize. The default maximum transfer size is set to `2097152` (2 MB). For every `pushPacket()`, the data+headers must be less than this value; otherwise, a `MARSHAL` exception is raised by the middleware. This maximum value can be found during run time by using the `omniORB::giopMaxMsgSize()` function call or the `bulkio::Const::MAX_TRANSFER_BYTES` value.  

### Vector Data
A component usually ingests and egresses data from its ports in the service function. A component with a provides-port (input), grabs data from the port using the `getPacket()` method. This method returns a `dataTransfer` object (described in [DataTransfer Member Descriptions](#datatransfer-member-descriptions)) from the input port's data queue or a null/None value if the queue is empty.

The following code snippet is an example of the `getPacket()` method.

```cpp
/**
   Grab data from the port's getPacket method
 */
bulkio::InFloatPort::dataTransfer *pkt;
pkt = inFloatPort->getPacket( bulkio::Const::NON_BLOCKING );

// check if a valid packet was returned
if ( pkt == NULL ) {
  return NOOP;
}

// check if any SRI changes occurred
if ( pkt->sriChanged ) {
  outFloatPort->pushSRI( pkt->SRI );
}

...   perform some algorithm on the data:  pkt->dataBuffer ...
```

The following table describes `DataTransfer` members.

##### DataTransfer Member Descriptions
| **Name**          | **Type**            | **Description**                                                                                                                                                                                                                  |
| :---------------- | :------------------ | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| dataBuffer        | std::vector\<TYPE\>   | The data transferred, where TYPE is some native CORBA type (e.g., float, short). The sequence may be zero-length.                                                                                                                |
| T                 | PrecisionUTCTime    | The epoch birth date of the first sample of the sequence.                                                                                                                                                                        |
| EOS               | boolean             | Flag describing whether this particular stream ends with this buffer of data.                                                                                                                                                    |
| streamID          | string              | Stream ID for this particular payload. This value is used to reconcile this sequence of data with a particular Stream SRI data structure.                                                                                        |
| SRI               | `BULKIO::StreamSRI` | The SRI (metadata) associated with the buffer of data.                                                                                                                                                    |
| sriChanged        | boolean             | Flag that describes if a new SRI object was received for this stream.                                                                                                                                                            |
| inputQueueFlushed | boolean             | Flag that signifies if the port's incoming data queue was flushed (purged) because the limit was reached. This happens when the consuming component does not keep up with the incoming rate at which the data is being received. |

A queue flush condition occurs when the number of packets in the queue exceeds the queue depth.
When a flush occurs, each stream in the queue is replaced with a single packet.
Each packet contains the last data payload and corresponding timestamp as well as any SRI changes, queue flush, and EOS indications that may have occurred for that stream.
If a stream contains only a single data packet when the flush occurs, that stream does not have its inputQueueFlushed flag set because no data was lost.
If a stream identifier occurs multiple times (Stream ID reuse), each of these streams contains a single packet with the proper data payload, time stamp, SRI changes, queue flush, and EOS indicators.

The following code snippet is an example of the `pushPacket()` method call for vector data with sample parameters.

```cpp
std::vector<short> data;

...  perform algorithm and save results to data vector ...

BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();
outShortPort->pushPacket( data, tstamp, false, "sample" );
```

The following table describes `pushPacket()` parameters for vector data.

##### `pushPacket()` Parameter Descriptions for Vector Data
| **Name** | **Type**         | **Description**                                                                                                                            |
| :------- | :--------------- | :----------------------------------------------------------------------------------------------------------------------------------------- |
| data     | sequence\<TYPE\> | A sequence of a particular type, where TYPE is some native CORBA type (e.g., float, short). The sequence may be zero-length.               |
| T        | PrecisionUTCTime | The epoch birth date of the first sample of the sequence passed in this call.                                                              |
| EOS      | boolean          | Flag describing whether this particular stream ends with this `pushPacket()` call.                                                         |
| streamID | string           | Stream ID for this particular payload. This string is used to reconcile this sequence of data with a particular Stream SRI data structure. |

### String Data/XML Document

The following code snippet is an example of the `pushPacket()` method call for string data with sample parameters.

```cpp
std::string data;

... generate some text data to transfer ...

outStringPort->pushPacket( data.c_str(), false, "sample" );
```

The following table describes `pushPacket()` parameters for string data.

##### `pushPacket()` Parameter Descriptions for String Data
| **Name** | **Type** | **Description**                                                                                                                            |
| :------- | :------- | :----------------------------------------------------------------------------------------------------------------------------------------- |
| data     | `char`     | A string of characters to pass between components. Also used for passing XML documents in-line between components.                         |
| EOS      | `boolean`  | Flag describing whether this particular stream ends with this `pushPacket()` call.                                                         |
| streamID | `string`   | Stream ID for this particular payload. This string is used to reconcile this sequence of data with a particular Stream SRI data structure. |

### URL/File Data

The following code snippet is an example of the `pushPacket()` method call for file transfers with sample parameters.

```cpp
std::string uri;

uri = "file:///data/samples.8t";

... open the file, fill with samples of data, close the file ...

BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();
outURLPort->pushPacket( uri.c_str(), tstamp, false, "sample" );
```

The following table describes `pushPacket()` parameters for file transfers.

##### `pushPacket()` Parameter Descriptions for File Transfers
| **Name** | **Type**         | **Description**                                                                                                                                                                              |
| :------- | :--------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| url      | `string`           | The URL describing the file. Appropriate schemes for the URLs are file:// and sca://. The file scheme applies to the local file system, while the sca scheme applies to the Software Communications Architecture (SCA) <abbr title="See Glossary.">File Manager</abbr>. |
| T        | `PrecisionUTCTime` | An appropriate time stamp for the data being passed.                                                                                                                                         |
| EOS      | `boolean`          | Flag describing whether this particular stream ends with this `pushPacket()` call.                                                                                                           |
| streamID | `string`           | Stream ID for this particular payload. This string is used to reconcile this sequence of data with a particular Stream SRI data structure.                                                   |

Data files may be sent via the Bulk Input/Output (BulkIO) `dataFile` type. When using the BulkIO `dataFile` type, a filename is passed to the `pushPacket()` method. The location of the file is specified by a URI that either points to the local file system or the Software-Defined Radio (SDR) file system. To support portability, use of the SDR file system is recommended.

#### URI Options

The following table describes the URI path options.

##### URI Options
| **Protocol** | **Format**                  | **Description**                                                                                                                                                                                                           |
| :----------- | :-------------------------- | :------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| file         | `file://[localhost]/<path>` | A host specific absolute path of the deployed component/device/<abbr title="See Glossary.">service</abbr>.                                                                                                                                                   |
| sdr          | `sdr://[ior]/<path>`        | A path on the <abbr title="See Glossary.">Domain Manager</abbr>'s file system. If the optional `ior` is provided, this path provides the reference to the Domain Manager. If not provided, the Domain Manager is the default used by the component/device/service |
