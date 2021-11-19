# Port Statistics

All Bulk Input/Output (BulkIO) <abbr title="See Glossary.">ports</abbr> contain a read only attribute called statistics. The statistics attribute is of type `BULKIO::PortStatistics`, and it contains information regarding the performance of the port. The table below contains a description of a statistics structure:

##### Fields in Returned Port Statistics
| **Name**          | **Type**                  | **Description**                               |
| :---------------- | :------------------------ | :-------------------------------------------- |
| portName          | `string`                  | Name of this port                           |
| elementsPerSecond | `float`                   | A moving average describing the rate at which elements are arriving.   |
| bitsPerSecond     | `float`                   | This is the same as elementsPerSecond \* bits per elements. |
| callsPerSecond    | `float`                   | Number of `pushPacket()` calls per second. |
| streamIDs         | `CF::StringSequence`      | List of all Stream IDs where a `pushSRI()` has occurred but no End of Stream (EOS) has been received.  |
| averageQueueDepth | `float`                   | Moving average calculation of the percentage queue depth. |
| timeSinceLastCall | `float`                   | The elapsed time, in seconds, since the last packet was transferred via a `pushPacket()` call |
| keywords          | `sequence <CF::DataType>` | Additional statistics information provided by the port. |

The provides-side port contains a single `PortStatistics` structure. The uses-side port contains a sequence of `PortStatistics` structures; each one associated with a single connection.

An interesting exercise is to create <abbr title="See Glossary.">components</abbr> that generate and consume data in the three languages supported by REDHAWK. The data generator and consumer generate/consume data as fast as possible. The statistics data structure can provide metrics regarding data transfer rates, average latency, and other relevant data. Shifting the transfer length (by changing the size of the sequence in the `pushPacket()` call) and seeing its effects on the performance of the connection is also instructive.
