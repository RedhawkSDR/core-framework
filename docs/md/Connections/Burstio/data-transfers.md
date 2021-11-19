# Data Transfers

Burst Input/Output (BurstIO) data transfers happen through the `pushBurst()` and `pushBursts()` method calls of a REDHAWK resource's (<abbr title="See Glossary.">component</abbr> or <abbr title="See Glossary.">device</abbr>) BurstIO <abbr title="See Glossary.">port</abbr> object. A resource can use these push methods to transfer bursts and their associated meta data from one resource to another within the resource's service function. Similar to Bulk Input/Output (BulkIO), BurstIO interfaces provide the same `BULKIO::PrecisionUTCTime` time stamp for each data vector of the burst. BurstIO defines a new `BurstSRI` Signal Related Information (SRI) object that enables developers to further describe the signaling environment and the data transformations. These fields are further described in [Burst Signal Related Information (SRI)](../burstio/sri.html).

### Input

A resource with a provides-port (input), grabs data from the port using the `getBurst()` method. This method returns a `PacketType` object (described in [Burst Packet Accessors](#burst-packet-accessors)) from the input port's data queue or a null/None value if the queue is empty.

The following code snippet is an example of the `getBurst()` method.

```cpp
/**
   Grab data from the port's getBurst method
 */
burstio::BurstShortIn::PacketType *pkt;
pkt = inShortPort->getBurst( bulkio::Const::NON_BLOCKING );

// check if a valid packet was returned
if ( pkt == NULL ) {
  return NOOP;
}

// check for EOS
if ( pkt->getEOS() ) {
  outShortPort->pushBurst(pkt->getSequence(), pkt->getSRI(), pkt->getEOS());
}

...  perform algorithm on the data: pkt->getData() ... or pkt->getSequence()
```

##### Burst Packet Accessors
| **Name**         | **Type**              | **Description**                                                                                 |
| :--------------- | :-------------------- | :---------------------------------------------------------------------------------------------- |
| `getSize`        | size                  | The number of samples in the burst. For complex data, the number of samples is half this value. |
| `getData`        | pointer to \<TYPE\>   | Pointer to the data samples.                                                                    |
| `getSequence`    | container to \<TYPE\> | Returns an iterable sequence to the data samples.                                               |
| `getEOS`         | boolean               | Returns `TRUE` if this is the last burst in a stream.                                           |
| `getSRI`         | `BULKIO::StreamSRI`   | The SRI (metadata) associated with the sample data.                                             |
| `getTime`        | PrecisionUTCTime      | The epoch birth date of the first sample of the sequence.                                       |
| `blockOccurred`  | boolean               | Returns `TRUE` if an incoming burst was blocked.                                                |
| `isComplex`      | boolean               | Returns `TRUE` if data is complex.                                                              |
| `getComplexData` | pointer to \<TYPE\>   | Pointer to a vector of complex pairs.                                                           |

### Output

Due to the asynchronous nature of BurstIO data, the interface enables the developer to control the output (egress) of bursts from a resource. The 2 main method calls to push burst data downstream from one resource to another are: `pushBursts()` and `pushBurst()`. `pushBursts()` enables multiple bursts to be sent directly downstream as a sequence of `BurstType` objects, whereas, `pushBurst()` provides an interface to queue a single burst to be pushed but follows policy directives based on the number of bursts, total queue size, and send intervals. Both methods route burst data using the specified routing constraints and connection filter which are controlled using the following interface:

```cpp
// this route streams with Stream ID == "data-stream-one" to a connection
// identified as "connection-one"
shortBurstPort->addConnectionFilter("data-stream-one", "connection-one");
```
or
```cpp
// update connection filter using the Component's connection property
// "myConnectionTable"
shortBurstPort->updateConnectionFilter(myConnectionTable);

// this sets the stream filter to only route streams to specific connections
shortBurstPort->setRoutingMode(burstio::ROUTE_CONNECTION_STREAMS);
```

##### Routing Control Directives
| **Routine Directive**      | **Description**                                                                        |
| :------------------------- | :------------------------------------------------------------------------------------- |
| `ROUTE_ALL_INTERLEAVED`    | All connections receive all streams; streams are interleaved in one buffer.            |
| `ROUTE_ALL_STREAMS`        | All connections receive all streams; streams are buffered independently.               |
| `ROUTE_CONNECTION_STREAMS` | Each connection may subscribe to a set of streams; streams are buffered independently. |


The major difference between the `pushBurst()` and `pushBursts()` methods is the ability to manage how and when the data is transferred. Only burst traffic that is queued using `pushBurst()` is controlled by the policy constraints, whereas, calls to `pushBursts()` are directly sent downstream to the connected resource.

```cpp
// this method will limit the maximum number of bursts that
// can be queued  before they are sent
shortBurstPort->setMaxBursts(size_t count);

// this method will enable threshold monitoring for the amount of sample
// data that exceeds this limit before sending data downstream
shortBurstPort->setByteThreshold(size_t bytes);

// this method will enable the latency time between the sending of
// available data downstream
shortBurstPort->setLatencyThreshold( long usec );
```

The following code snippet is an example of the `pushBurst()` method call for a vector data sample that is queued to the port.

```cpp
std::vector< BurstShortOut::NativeType > data;
my_transform(data);

BURSTIO::BurstSRI  sri;
burstio::BurstShortOut::BurstType burst;
burst.SRI = sri;
burst.EOS = false;
burst.T = burstio::utils::now();
burst.data.length(data.size());
for(int i=0; i< data.size(); i++ ) burst.data[i] = data[i];

// this queues a single burst
shortBurstPort->pushBurst( burst );

// or

std::vector< BurstShortOut::NativeType > data;
my_transform(data);

// this queues a single burst
shortBurstPort->pushBurst( data, sri, burstio::utils::now() );
```

The following code snippet is an example of the `pushBursts()` method call for a vector data sample. The bursts from this call are directly passed downstream to the connected resource.

```cpp
std::vector< BurstShortOut::NativeType > data;
my_transform(data);

BurstShortOut::BurstSequenceType bursts;
bursts.length(1);
burstio::BurstShortOut::BurstType burst;
burst.SRI = sri;
burst.EOS = false;
burst.T = burstio::utils::now();
burst.data.length(data.size());
for(int i=0; i< data.size(); i++ ) burst.data[i] = data[i];
bursts[0] = burst;

// this pushes the burst directly downstream because
// it is a sequence of bursts
shortBurstPort->pushBursts(bursts);
```
