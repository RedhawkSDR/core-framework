# Multi-out Ports

Each output Burst Input/Output (BurstIO) <abbr title="See Glossary.">port</abbr> type provides the ability to filter burst data from the resource based on Stream ID and Connection ID. To use the multi-out capability of the ports, a resource must include code similar to the following:

```xml
<structsequence id="connectionTable">
    <struct id="connectionTable::connection_descriptor"
               name="connection_descriptor">
      <simple id="connectionTable::connection_id" name="connection_id"
                 type="string">
        <kind kindtype="configure"/>
      </simple>
      <simple id="connectionTable::stream_id" name="stream_id" type="string">
        <kind kindtype="configure"/>
      </simple>
      <simple id="connectionTable::port_name" name="port_name" type="string">
        <kind kindtype="configure"/>
      </simple>
    </struct>
    <configurationkind kindtype="configure"/>
</structsequence>
```

To steer a particular stream of data to a particular connection, pass the `connectionTable` object to the port's `updateConnectionFilter` method. With the routing mode set to `ROUTE_CONNECTION_STREAMS`, the port will then apply the filter state to any burst traffic as it is passed out the resource's BurstIO port. For the burst to be passed to an existing connection, there must exist a match in the port's filters table for the burst's Stream ID and Connection ID of the resource downstream.
