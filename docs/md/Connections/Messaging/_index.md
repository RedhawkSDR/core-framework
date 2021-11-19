# Messaging

Messaging relies on CORBA's event structure as a transport structure. In CORBA's event API, <abbr title="See Glossary.">messages</abbr>  are passed as an `Any` type using the function `push()`.

While CORBA manages marshaling and delivery of the data, it does not provide any mechanisms inherent to events to describe the contents of the `Any` type. REDHAWK decided to leverage an existing payload structure descriptor to describe the payload of messages, the <abbr title="See Glossary.">properties</abbr> Interface Description Language (IDL). The selection of this interface eliminates the need to create a new IDL describing messages. Furthermore, there is already an XML structure that is mapped to efficient binary data structures, allowing the use of XML to describe message contents while eliminating the need for introducing XML parsers in the message delivery mechanism.

To support this additional functionality, REDHAWK has expanded the properties descriptor to allow a property to have the kind **message**. The only property that can have a valid **message** kind is a structure.

