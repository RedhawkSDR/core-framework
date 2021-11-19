# Working with SDDS Data

This section describes how to work with SDDS data in the <abbr title="See Glossary.">sandbox</abbr>, including how to write, ingest, manipulate, and introspect the data.

### SDDS Data via REDHAWK Components

The sandbox along with the SourceSDDS and SinkSDDS REDHAWK <abbr title="See Glossary.">components</abbr> allow a user to ingest and emit SDDS data during a user's session. These two components provide a fully-compliant capability when processing SDDS network traffic. Consult the appropriate documentation for the SDDS specification and each of the components.

  - SourceSDDS - Ingests SDDS packets from the network and repackages the data for Bulk Input/Output (BulkIO) <abbr title="See Glossary.">port</abbr> transmission. The component creates the proper BulkIO Signal Related Information (SRI), time stamps, and data vectors from the SDDS packet headers and payloads.

  - SinkSDDS - Emits SDDS packets from a connected BulkIO port to the network. The component uses the BulkIO SRI, time stamps, and data vectors to produce valid SDDS packets.

Each of these components requires a network address specification to access the appropriate host interface. The following table describes the two different network addresses supported by these components.

##### SDDS Address Specification
| **Protocol** | **Address**                | **Port**     | **VLAN**          |
| :----------- | :------------------------- | :----------- | :---------------- |
| UDP          | IPv4 address               | 1024 - 65535 | number (optional) |
| MULTICAST    | 224.0.0.0 - 239.255.25.255 | 1024 - 65535 | number (optional) |

#### Writing SDDS Data to a Network Interface

To generate SDDS packet data, the `rh.SinkSDDS` component is used to send BulkIO data over the network interface as SDDS packets. This component accepts three different BulkIO data types (octet, short and float) and then formats the data, SRI, and time stamp information into valid SDDS packets.

The following example uses the sandbox's DataSource and the `rh.SinkSDDS` component to generate SDDS packets to be sent over interface `eth0`, the IP address `127.0.0.1`, and port `29000`.

```python
>>> from ossie.utils import sb
>>> sig=sb.DataSource()
>>> sdds_out = sb.launch("rh.SinkSDDS")
>>> sdds_out.network_settings.interface="eth0"
>>> sdds_out.network_settings.ip_address="127.0.0.1"
>>> sdds_out.network_settings.port=29000
>>> sig.connect(sdds_out, usesPortName="shortOut")
>>> data=range(1024)
>>> SRI=sig.SRI()
>>> SRI.xdelta = 1/1000.0
>>> sig.push(data, SRI=SRI, EOS=False, loop=False)
```

#### Reading SDDS Data from a Network Interface

To read SDDS packet data, the `rh.SourceSDDS` component is used to receive SDDS data from a network interface. This component transforms the SDDS packet data into BulkIO data, SRI, and time stamps for downstream connections.

The following example configures a `rh.SourceSDDS` component to read data from interface `eth0`, send it to IP address `127.0.0.1` and port `29495`, and forward the data to a sandbox DataSink over a `short` typed port.


```python
>>> from ossie.utils import sb
>>> dsink=sb.DataSink()
>>> sdds_in = sb.launch("rh.SourceSDDS")
>>> sdds_in.attachment_override.ip_address="127.0.0.1"
>>> sdds_in.attachment_override.port=29495
>>> sdds_in.attachment_override.enable=True
>>> sdds_in.interface="eth0"
>>> sdds_in.connect(dsink,usesPortName="dataShortOut")
```

In lieu of the `attachment_override` property, both components support the BulkIO dataSDDS interface and `BULKIO::SDDSStreamDefinition` structure. This interface defines an `attach` method, which is implemented by each component, and performs the necessary actions to connect to the SDDS source defined in the `BULKIO::SDDSStreamDefinition` structure.

The following table describes the members of the `BULKIO::SDDSStreamDefinition` structure.

##### `BULKIO::SDDSStreamDefinition`
| **Name**         | **Type**        | **Description**                                                |
| :--------------- | :-------------- | :------------------------------------------------------------- |
| id               | string          | Stream ID to identify the data or Allocation ID for attachment |
| dataFormat       | SDDSDataDigraph | Payload type of SDDS packet                                    |
| multicastAddress | string          | Multicast address                                              |
| vlan             | long            | Virtual LAN number                                             |
| port             | long            | Port number                                                    |
| sampleRate       | long            | Sampling frequence of SDDS payload data (egress only)          |
| timeTagValid     | boolean         | Marks packets with valid time stamp field (egress only)        |
| privateInfo      | string          | User-generated text                                            |

The following example configures the `rh.SourceSDDS` component to read data using the address specification defined by the `BULKIO::SDDSStreamDefinition` structure, and forward the data to a sandbox DataSink over it's `short` typed port.

```python

>>> from ossie.utils import sb
>>> from bulkio.bulkioInterfaces import BULKIO
>>> dsink=sb.DataSink()
>>> sdds_in = sb.launch("rh.SourceSDDS")
>>> sdds_in.connect(dsink,usesPortName="dataShortOut")
>>> sdds_port=sdds_in.getPort("dataSddsIn")
>>> sd = BULKIO.SDDSStreamDefinition("my_stream", BULKIO.SDDS_SI, "127.0.0.1",  0, 29495, 8000, True, "testing")
>>> attach_id = sdds_port.attach(sd, "username")
```

### Capturing SDDS Data and the Sandbox's `DataSourceSDDS`

This section describes how to capture SDDS Data packets, introspect their contents, and manipulate the data using the sandbox.

#### SDDS Data and the Sandbox's `DataSourceSDDS`

Independent of the components, the sandbox also provides a snapshot and introspection capability through the `DataSourceSDDS` class. The following commands create the `DataSourceSDDS` object.

```python
>>> from ossie.utils import sb
>>> ds=sb.DataSourceSDDS()
```

Using this object, the user can capture an arbitrary number of packets and then introspect their contents. The content analysis and packet decomposition is provided through the `SDDSAnalyzer` class. To initiate a data capture, call the DataSourceSDDS's `getData` or `getStreamDef`. By default, these methods return a `SDDSAnalyzer` object that contains all the raw packet data.

```python
getData( mgroup, hostip, port=29495, pkts=1000, pktlen=1080, block=True, returnSddsAnalyzer=True)

Parameters:
  mgroup = multicast address or IP address
  hostip = address of host interface to use
  port = port number to listen on
  pkts = number of packets to capture
  pktlen = length in bytes of a single packet
  block = will block until all packets are read
  returnSddsAnalyzer = returns SDDS analyzer object instead of raw data

Returns:
 SDDSAnalyzer:  provides SDDS packet introspection and tracking
   or
  tuple:  data - converted raw data to a list of numbers
          rawdata - actual data read from socket
          pktlen - packet length provided during capture
          pkts  - number of packets read
          totalRead - total number of bytes read
```

#### Using the `SDDSAnalyzer`

Using the `SDDSAnalyzer`, you can perform the following actions:

  - `dumpPackets` - Displays packet data as readable SDDS packets with header breakout.

    ```python
    dumpPackets(pkt_start=0, pkt_end=None, payload_start=0, payload_end=40, raw_payload=False, header_only=False, use_pager=True)

    Displays SDDS packet header and payload in human readable format.

    Parameters:
      pkt_start = first packet to dump
      pkt_end =  last packet to dump  (None == end of list)
      payload_start = starting payload sample to display
      payload_end = ending payload sample to display
      raw_payload = dump payload data as raw bytes
      header_only = only display header information for each packet
      use_pager  = display data using pager to limit number of packets that are displayed
    ```

  - `dumpRawPackets` - Displays packet data as bytes.

    ```python
    dumpRawPackets(pkt_start=0, pkt_end=None, row_width=80, bytes_per_group=2, pkt_len=None, use_pager=True)

    Displays SDDS packet data in hexidecimal format

    Parameters:
      pkt_start = first packet to dump
      pkt_end =  last packet to dump  (None == end of list)
      row_width = 80 the number of bytes to display per row
      bytes_per_group = the number of bytes to group when hexify-ing
      pkt_len = the number of bytes in a packet, None defaults to 1080 or length when getData method was called
      use_pager =  display data using pager to limit number of packets that are displayed
    ```

  - `getPacketIterator` - Returns a Python iterator that can be used in loops.

    ```python
    getPacketIterator(pkt_start=0, pkt_end=None)

    Returns a Python iterator that will traverse the set of packets managed by the SDDSAnalyzer object

    Parameters:
      pkt_start = first packet to dump
      pkt_end =  last packet to dump  (None == end of list)
    ```

  - `getPackets` - Returns a list of SDDS packet objects.

    ```python
    getPackets(pkt_start=0, pkt_end=None)

    Returns a list of sdds_packet objects

    Parameters:
      pkt_start = first packet to dump
      pkt_end =  last packet to dump  (None == end of list)
    ```

  - `trackChanges` - Tracks changes in header fields.

    ```python
    trackChanges(pkt_start=0, pkt_end=None, repeat_header=20, use_pager=True)

    Tracks changes to the following SDDS packet fields: sequence numbers, data mode, complex flags, bits per sample, frequency, rate, time tag valid, time slips

    No changes in the data field are displayed as:  -
    Changes in the data field are displayed as: ***
    Valid TimeStamps are denoted as: +

    Parameters:
       pkt_start = first packet to dump
       pkt_end =  last packet to dump  (None == end of list)
       repeat_header = displays column header every Nth packet displayed
       use_pager =  display data using pager to limit number of packets that are displayed
    ```

#### SDDS Packets

The resulting SDDS packet objects, provided by the SDDSAnalyzer, allow for inspection and manipulation of each packet's underlying data. The python help utility for `ossie.utils.sdds.sdds_packet` module describes these methods in more detail. The following sample code creates a SDDS packet object and sets the sample rate and payload contents of the packet.

```python
from ossie.utils.sdds import *
pkt=sdds_packet()
pkt.header.set_rate(10e6)
pkt.header.get_rate()
# OUT: 10000000.0
pkt.payload.sb.set_data(1024*[100])

# diplay the payload contents as list of numbers
pkt.payload.sb.get_data()
# OUT: [100, 100, 100, 100, 100,
  ....
100, 100, 100, 100, 100 ]

# print out the entire contents of the sdds packet as array of octets
pkt.asBuffer()
```

### `DataSinkSDDS` in the Sandbox

The `rh.SinkSDDS` component provides a fully compliant capability for ingesting BulkIO's data streams and publishing SDDS packets to a network. To support use cases that do not require this level of compliance, the sandbox provides the `DataSinkSDDS` object that can capture the stream definitions and SRI data from a BulkIO SDDS interface that publishes this information. Below is a sample code session showing how to connect to a `DataSinkSDDS` object and capture the stream definition.

```python
from ossie.utils import sb
from bulkio import BULKIO

def inStreamDef(sd,userid):
     print "stream def: ", sd
     print "user: ", userid

sink=sb.DataSinkSDDS()
src=sb.DataSourceSDDS()
src.connect(sink)

sd=BULKIO.SDDSStreamDefinition("data1", BULKIO.SDDS_SB,"239.1.1.0",0,29495,1000000,False,"")
#
# triggers default process to print out stream definition and user id
src.attach(sd,"stream1")

#
# setup callback for attachment
sink.registerAttachCallback(inStreamDef)
src.attach(sd,"stream2")
```
