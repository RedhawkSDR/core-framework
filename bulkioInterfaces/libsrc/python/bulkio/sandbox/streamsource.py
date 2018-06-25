#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

from ossie.utils.docstring import inherit_doc
from ossie.utils.sandbox.helper import SandboxHelper

from redhawk.bitbuffer import bitbuffer

import bulkio
from bulkio.bulkioInterfaces import BULKIO
from bulkio.output_ports import *
from bulkio.output_streams import OutputStream, OutXMLStream

_PORT_MAP = {
    'char' : (OutCharPort, BULKIO.dataChar),
    'octet': (OutOctetPort, BULKIO.dataOctet),
    'short' : (OutShortPort, BULKIO.dataShort),
    'ushort' : (OutUShortPort, BULKIO.dataUshort),
    'long' : (OutLongPort, BULKIO.dataLong),
    'ulong' : (OutULongPort, BULKIO.dataUlong),
    'longlong' : (OutLongLongPort, BULKIO.dataLongLong),
    'ulonglong' : (OutULongLongPort, BULKIO.dataUlongLong),
    'float' : (OutFloatPort, BULKIO.dataFloat),
    'double' : (OutDoublePort, BULKIO.dataDouble),
    'bit' : (OutBitPort, BULKIO.dataBit),
    'file' : (OutFilePort, BULKIO.dataFile),
    'xml' : (OutXMLPort, BULKIO.dataXML)
}

def proxy(attr):
    """
    Wrapper class to handle passthrough for getting and setting OutputStream
    attributes in StreamSource.
    """
    class property_proxy(object):
        __doc__ = attr.__doc__
        def __init__(self, attr):
            self.__attr = attr

        def __get__(self, obj, cls=None):
            if obj is None:
                return self
            if obj._stream is not None:
                return self.__attr.__get__(obj._stream, cls)
            return self.__attr.__get__(obj, cls)

        def __set__(self, obj, value):
            if obj._stream is not None:
                self.__attr.__set__(obj._stream, value)
            self.__attr.__set__(obj, value)

    return property_proxy(attr)

class StreamSource(SandboxHelper):
    """
    Sandbox helper for writing BulkIO stream data.

    StreamSource provides a simplified interface to write a single BulkIO
    stream to one or more components. It can support any BulkIO data type (with
    the exception of SDDS and VITA49), but only one type is supported per
    instance.

    If more than one stream is desired, create one StreamSource per stream ID,
    or use the `port` attribute to get direct access to the underlying BulkIO
    port. The port is an instance of the same class used in Python components,
    and supports the full stream API.

    See Also:
        StreamSink
    """
    def __init__(self, streamID=None, format=None):
        """
        Creates a new StreamSource.

        Args:
            streamID:  The unique identifier for this stream. If not given, the
                       helper's instance name is used (e.g., "StreamSource_1").
            format:    BulkIO port type to support (e.g., "float"). If not
                       given, all BulkIO port types except SDDS and VITA49 are
                       supported.
        """
        SandboxHelper.__init__(self)
        self._streamID = streamID
        self._stream = None

        if format:
            formats = [format]
        else:
            formats = _PORT_MAP.keys()
        for format in formats:
            clazz, helper = _PORT_MAP[format]
            self._addUsesPort(format+'Out', helper._NP_RepositoryId, clazz)

    def _initializeHelper(self):
        if not self._streamID:
            self._streamID = self._instanceName
        self._sri = bulkio.sri.create(self._streamID)

    def write(self, data, time=None, interleaved=False):
        """
        Writes data to the stream.

        Args:
            data:        Data to write.
            time:        Optional time stamp for first sample of `data`. If not
                         given, the current time is used. Ignored when the data
                         type is XML.
            interleaved: Indicates whether complex data is already interleaved.
        """
        if not self._port:
            # Not connected to anything
            return

        # Get the stream via the attribute, which will create it if it does not
        # already exist
        stream = self.stream

        # Turn framed input data into a 1-dimensional sequence, but only if the
        # stream is configured for it
        if stream.subsize > 0:
            data = self._unframeData(data)

        args = [data]
        kwargs = {}
        if not isinstance(stream, OutXMLStream):
            if time is None:
                time = bulkio.timestamp.now()
            kwargs['time'] = time
        if interleaved:
            kwargs['interleaved'] = True
        stream.write(*args, **kwargs)

    def close(self):
        """
        Closes the stream, sending an end-of-stream packet.
        """
        if self._stream:
            self._stream.close()
            self._stream = None

    @property
    def streamID(self):
        """
        The unique identifier of this stream.
        """
        return self._sri.streamID

    @property
    def port(self):
        """
        The BulkIO output port in use by this helper.

        The port is created when a connection is made from this helper to
        another object in the sandbox. If no connection exists, the port is
        None.
        """
        return self._port

    @property
    def stream(self):
        """
        The BulkIO stream managed by this helper.

        If no connection from this helper has been made yet, the stream is
        None.

        See Also:
            StreamSource.port
        """
        if self._port and not self._stream:
            # Port has to exist before creating the stream
            self.log.debug("Creating stream '%s'", self.streamID)
            self._stream = self._port.createStream(self._sri)
        return self._stream

    def _unframeData(self, data):
        if not data:
            # Assume empty sequence, nothing to do
            return data
        elif isinstance(data[0], bitbuffer):
            # Sequence of bitbuffers, compact down to a single bitbuffer
            return sum(data, bitbuffer())
        elif isinstance(data[0], (list, tuple)):
            # Sequence of sequences, probably numeric data, compact into a
            # single list
            return sum(data, [])
        else:
            # Something else (probably numbers), just pass through
            return data

    def _setStreamMetadata(self, attr, value):
        # This method is required by the proxied setters from the output stream
        # class; it just needs to update the local copy of the SRI
        setattr(self._sri, attr, value)
    
    def _modifyingStreamMetadata(self):
        # This method is required by the proxied setter for 'sri'; it doesn't
        # have to actually do anything
        pass

    # Stream properties are defined as proxies to provide consistent behavior.
    # If a stream is already created, it writes through to the stream, but also
    # updates its local SRI. If no stream has been created, the local SRI is
    # modified so that when a stream is created, it gets the latest SRI.
    sri = proxy(OutputStream.sri)
    xstart = proxy(OutputStream.xstart)
    xdelta = proxy(OutputStream.xdelta)
    xunits = proxy(OutputStream.xunits)
    subsize = proxy(OutputStream.subsize)
    ystart = proxy(OutputStream.ystart)
    ydelta = proxy(OutputStream.ydelta)
    yunits = proxy(OutputStream.yunits)
    blocking = proxy(OutputStream.blocking)
    complex = proxy(OutputStream.complex)
    keywords = proxy(OutputStream.keywords)

    @inherit_doc(bulkio.sri.hasKeyword)
    def hasKeyword(self, name):
        return bulkio.sri.hasKeyword(self._sri, name)

    @inherit_doc(bulkio.sri.getKeyword)
    def getKeyword(self, name):
        return bulkio.sri.getKeyword(self._sri, name)

    @inherit_doc(bulkio.sri.setKeyword)
    def setKeyword(self, name, value, format=None):
        if self._stream:
            self._stream.setKeyword(name, value, format)
        bulkio.sri.setKeyword(self._sri, name, value, format)

    @inherit_doc(bulkio.sri.eraseKeyword)
    def eraseKeyword(self, name):
        if self._stream:
            self._stream.eraseKeyword(name)
        bulkio.sri.eraseKeyword(self._sri, name)
