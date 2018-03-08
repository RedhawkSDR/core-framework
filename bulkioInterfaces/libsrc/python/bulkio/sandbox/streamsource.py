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

import bulkio
from bulkio.output_ports import *
from bulkio.output_streams import OutputStream

_PORT_MAP = {
    'char' : (OutCharPort, 'IDL:BULKIO/dataChar:1.0'),
    'octet': (OutOctetPort, 'IDL:BULKIO/dataOctet:1.0'),
    'short' : (OutShortPort, 'IDL:BULKIO/dataShort:1.0'),
    'ushort' : (OutUShortPort, 'IDL:BULKIO/dataUshort:1.0'),
    'long' : (OutLongPort, 'IDL:BULKIO/dataLong:1.0'),
    'ulong' : (OutULongPort, 'IDL:BULKIO/dataUlong:1.0'),
    'longlong' : (OutLongLongPort, 'IDL:BULKIO/dataLongLong:1.0'),
    'ulonglong' : (OutULongLongPort, 'IDL:BULKIO/dataUlongLong:1.0'),
    'float' : (OutFloatPort, 'IDL:BULKIO/dataFloat:1.0'),
    'double' : (OutDoublePort, 'IDL:BULKIO/dataDouble:1.0'),
    'bit' : (OutBitPort, 'IDL:BULKIO/dataBit:1.0'),
    'file' : (OutFilePort, 'IDL:BULKIO/dataFile:1.0'),
    'XML' : (OutXMLPort, 'IDL:BULKIO/dataXML:1.0')
}

def proxy(attr):
    class property_proxy(object):
        __doc__ = attr.__doc__
        def __init__(self, attr):
            self.__attr = attr

        def __get__(self, obj, cls=None):
            if obj is None:
                return self
            if obj._stream is not None:
                return self.__attr.__get__(obj._stream)
            return self.__attr.__get__(obj)

        def __set__(self, obj, value):
            if obj._stream is not None:
                self.__attr.__set__(obj._stream, value)
            self.__attr.__set__(obj, value)

    return property_proxy(attr)

class StreamSource(SandboxHelper):
    def __init__(self, streamID=None, format=None):
        SandboxHelper.__init__(self)
        self._streamID = streamID
        self._stream = None

        if format:
            formats = [format]
        else:
            formats = _PORT_MAP.keys()
        for format in formats:
            clazz, repo_id = _PORT_MAP[format]
            self._addUsesPort(format+'Out', repo_id, clazz)

    def _initializeHelper(self):
        if not self._streamID:
            self._streamID = self._instanceName
        self._sri = bulkio.sri.create(self._streamID)

    def write(self, data, timestamp=None):
        if not self._port:
            # Not connected to anything
            # TODO: Is this an error condition or should we ignore it
            return

        if not self._stream:
            self.log.debug("Creating stream '%s'", self.streamID)
            self._stream = self._port.createStream(self._sri)

        args = [data]
        if not 'XML' in self._port.name:
            if timestamp is None:
                timestamp = bulkio.timestamp.now()
            args.append(timestamp)
        self._stream.write(*args)

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
    def setKeyword(self, name, value):
        if self._stream:
            self._stream.setKeyword(name, value)
        bulkio.sri.setKeyword(self._sri, name, value)

    @inherit_doc(bulkio.sri.eraseKeyword)
    def eraseKeyword(self, name):
        if self._stream:
            self._stream.eraseKeyword(name)
        bulkio.sri.eraseKeyword(self._sri, name)
