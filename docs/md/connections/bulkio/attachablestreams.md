# Attachable Streams

Attachable streams provide a way to use out-of-band data transfer. The mechanics of the data transfer are defined by a separate network protocol, while REDHAWK provides connection management. The two supported attachable streams for Bulk Input/Output (BulkIO) are SDDS and VITA49.  The BulkIO uses port passes a StreamDefinition object to the connected ports using the attach method.  The connected input port provides the StreamDefinition object for the downstream resource to create a connection to the actual data source.  The StreamDefinition objects passed are mapped to the underlying BulkIO data type. BulkIO SDDS ports pass SDDSStreamDefintion objects and BulkIO VITA49 ports pass VITA49StreamDefinition objects.

### SDDS Stream Definition

The SDDS Stream Definition object defines a connection to a data source from a network interface. The methods for the SDDS Stream Definition Interface do not follow the normal BulkIO `pushPacket()` convention; instead, the interface defines `attach()` and `detach()` methods. The `attach()` and `detach()` methods are provided in the following code snippet.

```cpp
/**
 *  SDDS Stream Definition Interface
 */

/**
  *  attach : request to an attachment to a specified network data source
  */
char  *attach( BULKIO::SDDSStreamDefinition stream, const char * userid );

/**
  * detach: unlatch from a network data source
  */
void  detach( const char* attachId );
```

The following tables describe the `attach()` and `detach()` methods and the SDDS Stream Definition Member.

##### `char* attach()`
| **Name**     | **Type**             | **Description**                                             |
| :----------- | :------------------- | :---------------------------------------------------------- |
| return value | char\*               | Attachment identifier assigned to this request.             |
| stream       | SDDSStreamDefinition | Stream definition object describing a multicast IP address ([SDDS Stream Definition Member Descriptions](#sdds-stream-definition-member-descriptions)). |
| userid       | const char\*         | Identification for the request of the attach call.          |

##### `void detach()`
| **Name**     | **Type**             | **Description**                                             |
| :----------- | :------------------- | :---------------------------------------------------------- |
| attachId     | char\*               | Attachment identifier returned from attach request.         |

##### SDDS Stream Definition Member Descriptions
| **Name**         | **Type**        | **Description**                                             |
| :--------------- | :-------------- | :---------------------------------------------------------- |
| ID               | string          | Unique identifier for the source stream.                    |
| dataFormat       | SDDSDataDigraph | Data format of the data stream samples.                     |
| multicastAddress | string          | IPv4 network address in dot notation form.                  |
| vlan             | unsigned long   | Virtual lan identifier as defined by 802.11q.               |
| port             | unsigned long   | IP port number associated with the network connection.      |
| sampleRate       | unsigned long   | Expected sample rate for the data.                          |
| timeTagValid     | boolean         | Denotes if data stream can provide valid time stamp values. |
| privateInfo      | string          | Allows for user-defined values to be passed as a string.    |
