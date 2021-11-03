# Port Statistics

All Burst Input/Output (BurstIO) <abbr title="See Glossary.">ports</abbr> support the Bulk Input/Output (BulkIO) statistics interface with additional keywords to track burst-specific metrics. Statistics are tracked over a window of 10 `pushBurst` calls. An input port contains a single `PortStatistics` structure, whereas, an output port contains a sequence of `PortStatistics` structures; one structure per connection. For more information on `BULKIO::PortStatistics`, see [Port Statistics](../bulkio/port-statistics.html). The additional BurstIO metrics for both input and output ports are described in the following tables:

### C++

The following example illustrates a <abbr title="See Glossary.">component</abbr> that performs a transform on the incoming burst data and pushes the results downstream.

```cpp
burstio::BurstShortIn::PacketType *pkt;
pkt = inShortPort->getPacket(bulkio::Const::NON_BLOCKING);

// check if a valid packet was returned
if ( pkt == NULL ) {
  return NOOP;
}

// check for EOS
if ( pkt->getEOS() ) {
  outShortPort->pushBurst(pkt->getSequence(), pkt->getSRI(), pkt->getEOS());
}

// do some processing.....to the burst contents
BurstShortOut::SequenceType  data =  do_some_magic(pkt->getSequence());

// we changed the data so calc new time stamp....
BULKIO::PrecisionUTCTime newTS = calc_timestamp(pkt->getTime());  

outShortPort->pushBurst(data, pkt->getSRI(), newTS, pkt->getEOS());
```

### Java

The following example illustrates a component that generates 10 bursts objects containing 100 samples of data and sends the array downstream.

```Java
/**
  This example demonstrates a Component that uses the pushBursts method
  to generate data
 */
int nbursts=10;
String sid = new String("stream-1-1");
BURSTIO.LongBurst [] bursts = new BURSTIO.LongBurst[nbursts];
// allocate space for 10 bursts to push downstream

// generate bursts with 100 samples of data for downstream
for ( int j=0; j < nbursts; j++ ) {
    BURSTIO.LongBurst burst  = new BURSTIO.LongBurst();
    burst.SRI = make_SRI( sid );
    // generate a Burst SRI object for use...
    burst.EOS = false;
    burst.T = burstio.Utils.now();
    burst.data = generate_samples( 100 );
    bursts[j] = burst;
}

longOutPort.pushBursts( bursts );
```

### Python

The following example illustrates a component that generates burst samples that will be filtered out by the port's routing table when at least 10 bursts are queued for delivery.

```python
from redhawk.burstioInterfaces import BURSTIO
from redhawk.burstio import *

def initialize(self):
  #
  # Send bursts to downstream resource using connection filtering
  #  **you will need to set the routing table in a property change event
  #     callback for the resource's connection table object**
  #
  self.outShortPort.setRoutingMode( ROUTE_CONNECTION_STREAMS )
  self.outShortPort.setMaxBursts(10);

def generate_burst_samples(self, nsamps=10 ):
  #
  # generate number of sample data..
  return range(nsamps)

def process(self):
  #
  data = self.generate_burst_samples(100)
  SRI = burstio.utils.createSRI("test_stream_id")
  SRI.xdelta = 1.0/1000.0
  SRI.mode = 0

  self.outShortPort.pushBurst( data, SRI, burstio.utils.now() )

  return NORMAL
```
