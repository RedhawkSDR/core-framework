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

import bulkio
from bulkio.output_ports import *

from .helper import SandboxPortHelper

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

class StreamSource(SandboxPortHelper):
    def __init__(self, streamID='StreamSource_stream', format=None):
        SandboxPortHelper.__init__(self)

        if format:
            formats = [format]
        else:
            formats = _PORT_MAP.keys()
        for format in formats:
            clazz, repo_id = _PORT_MAP[format]
            self._addUsesPort(format+'Out', repo_id, clazz)

        self._streamID = streamID

    def write(self, data, timestamp=None):
        if not self._port:
            # Not connected to anything
            # TODO: Is this an error condition or should we ignore it
            return
        stream = self._port.getStream(self._streamID)
        if not stream:
            stream = self._port.createStream(self._streamID)

        args = [data]
        if not 'XML' in self._port.name:
            if timestamp is None:
                timestamp = bulkio.timestamp.now()
            args.append(timestamp)
        stream.write(*args)
