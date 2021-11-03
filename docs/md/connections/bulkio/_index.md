# BulkIO

Bulk Input/Output (BulkIO) is designed to provide a standardized methodology and to maximize efficiency for bulk data transfers between REDHAWK resources (<abbr title="See Glossary.">components</abbr> and <abbr title="See Glossary.">devices</abbr>). This interface supports the transfer of data vectors (float, double, char (int8), octet (uint8), short (int16), ushort (uint16), long (int32), ulong (uint32), longlong (int64), ulonglong(uint64)), character strings (char \*), and out-of-band connection descriptors for SDDS data streams.

These interfaces also allow for metadata, Signal Related Information (SRI), and a precision time stamp (described in detail in the following subsections), which describe the content being transferred and support content processing. Part of the required methodology for passing data between REDHAWK components is that all data transfers via `pushPacket()` are preceded by at least one call to `pushSRI()` with an appropriate SRI object. SRI data is passed out-of-band from the content data to reduce the overhead for transferring data between components. The precision time stamp represents the birth date for data and is part of the `pushPacket()` method call for those components that require this information.

The data flow implementation for a component's BulkIO <abbr title="See Glossary.">port</abbr> interface is provided by a shared `bulkio` base class library. The resulting component code instantiates a `bulkio` base class object and makes use of the shared library during deployment and execution.

