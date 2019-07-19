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

import copy

from ossie.cf import CF
from redhawk.bitbuffer import bitbuffer

import bulkio
from bulkio.bulkioInterfaces import BULKIO
from bulkio.stream_base import StreamBase

class OutputStream(StreamBase):
    """
    Basic BulkIO output stream class.

    OutputStream encapsulates a single BulkIO stream for writing. It is
    associated with the output port that created it, providing a file-like API
    on top of the classic BulkIO pushPacket model.

    Notionally, a BulkIO stream represents a contiguous data set and its
    associated signal-related information (SRI), uniquely identified by a
    stream ID, from creation until close. The SRI may vary over time, but the
    stream ID is immutable. Only one stream with a given stream ID can be
    active at a time.

    OutputStreams help manage the stream lifetime by tying that SRI with an
    output port and ensuring that all data is associated with a valid stream.
    When the stream is complete, it may be closed, notifying downstream
    receivers that no more data is expected.

    OutputStreams must be created via an output port. A stream cannot be
    associated with more than one port.

    SRI Changes:
    Updates to the stream that modify or replace its SRI are cached locally
    until the next write to minimize the number of updates that are published.
    When there are pending SRI changes, the OutputStream pushes the updated SRI
    first, followed by the data.

    See Also:
        OutPort.createStream
        OutPort.getStream
    """
    def __init__(self, sri, port, dtype=list):
        """
        Create an OutputStream.

        Warning:
             Output streams are created via an output port. This constructor
             should not be called directly.

        See Also:
            OutputPort.createStream
            OutputPort.getStream
        """
        StreamBase.__init__(self, sri)
        self._port = port
        self._dtype = dtype
        self.__sriModified = True

    @StreamBase.sri.setter
    def sri(self, sri):
        if not isinstance(sri, BULKIO.StreamSRI):
            raise TypeError('sri must be a BULKIO.StreamSRI') 
        # If the SRI is the same, or differs only by streamID, do nothing.
        if bulkio.sri.compareFields(self.sri, sri) in (bulkio.sri.NONE, bulkio.sri.STREAMID):
            return
        self._modifyingStreamMetadata()
        # Deep copy to avoid accidental updates to the SRI via the caller's
        # reference
        sri = copy.deepcopy(sri)
        # Preserve stream ID
        sri.streamID = self.streamID
        self._sri = sri

    @StreamBase.xstart.setter
    def xstart(self, xstart):
        self._setStreamMetadata('xstart', float(xstart))

    @StreamBase.xdelta.setter
    def xdelta(self, xdelta):
        self._setStreamMetadata('xdelta', float(xdelta))

    @StreamBase.xunits.setter
    def xunits(self, xunits):
        self._setStreamMetadata('xunits', int(xunits))

    @StreamBase.subsize.setter
    def subsize(self, subsize):
        self._setStreamMetadata('subsize', int(subsize))

    @StreamBase.ystart.setter
    def ystart(self, ystart):
        self._setStreamMetadata('ystart', float(ystart))

    @StreamBase.ydelta.setter
    def ydelta(self, ydelta):
        self._setStreamMetadata('ydelta', float(ydelta))

    @StreamBase.yunits.setter
    def yunits(self, yunits):
        self._setStreamMetadata('yunits', int(yunits))

    @StreamBase.complex.setter
    def complex(self, mode):
        self._setStreamMetadata('mode', 1 if mode else 0)

    @StreamBase.blocking.setter
    def blocking(self, blocking):
        self._setStreamMetadata('blocking', 1 if blocking else 0)

    @StreamBase.keywords.setter
    def keywords(self, keywords):
        if bulkio.sri._compareKeywords(self.keywords, keywords):
            return
        self._modifyingStreamMetadata()
        # Copy the sequence, but not the values
        self._sri.keywords = keywords[:]

    def setKeyword(self, name, value, format=None):
        """
        Sets the current value of a keyword in the SRI.

        If the keyword name does not exist, the new keyword is appended.
        If the keyword name exists, and the value argument differs from the
        keyword's value, the keyword is updated to the argument value.

        If the keyword name already exists, its value is updated to value. If
        the keyword name does not exist, the new keyword is appended.

        If the optional 'format' argument is given, it must be the name of the
        desired CORBA type. Otherwise, the CORBA type is determined based on
        the Python type of 'value'.

        If setting a keyword changes its key or value, it updates the SRI,
        which will be pushed on the next write.

        Args:
            name:   The name of the keyword.
            value:  The new value.
            format: Optional type name.
        """
        if bulkio.sri.hasKeyword(self._sri, name):
            if bulkio.sri.getKeyword(self._sri, name) == value:
                return
        self._modifyingStreamMetadata()
        bulkio.sri.setKeyword(self._sri, name, value, format)

    def eraseKeyword(self, name):
        """
        Removes a keyword from the SRI.

        Erases the keyword named 'name' from the SRI keywords. If no keyword
        'name' is found, the keywords are not modified.

        If the keywords are modified, the SRI is updated and will be pushed on
        the next write.

        Args:
            name: The name of the keyword.
        """
        if not bulkio.sri.hasKeyword(self._sri, name):
            return
        self._modifyingStreamMetadata()
        bulkio.sri.eraseKeyword(self._sri, name)

    def close(self):
        """
        Closes this stream.
        
        Sends an end-of-stream packet. No further operations should be made on
        the stream.
        """
        data = self._dtype()
        self._send(data, bulkio.timestamp.notSet(), True)

    def _setStreamMetadata(self, attr, value):
        field = getattr(self._sri, attr)
        if field != value:
            self._modifyingStreamMetadata()
            setattr(self._sri, attr, value)

    def _send(self, data, time, eos):
        if self.__sriModified:
            self._port.pushSRI(self._sri)
            self.__sriModified = False
        self._pushPacket(self._port, data, time, eos, self.streamID)

    def _pushPacket(self, port, data, time, eos, streamID):
        port.pushPacket(data, time, eos, streamID)

    def _modifyingStreamMetadata(self):
        self.__sriModified = True


class BufferedOutputStream(OutputStream):
    """
    BulkIO output stream class with data buffering.

    BufferedOutputStream can use an internal buffer to queue up multiple
    packets worth of data into a single push. By default, buffering is
    disabled.

    Data Buffering:
    BufferedOutputStreams can combine multiple small chunks of data into a
    single packet for reduced I/O overhead. Data buffering is enabled by
    setting a non-zero buffer size via the setBufferSize() method. The output
    stream creates an internal buffer of the requested size; the stream's
    complex mode is not taken into account.

    With buffering enabled, each write copies its data into the internal
    buffer, up to the maximum of the buffer size. When the internal buffer is
    full, a packet is sent via the output port, using the time stamp of the
    first buffered sample. After the packet is sent, the internal buffer is
    reset to its initial state. If there is any remaining data from the write,
    it is copied into a new buffer and a new starting time stamp is
    interpolated.

    Time Stamps:
    When buffering is enabled, the time stamps provided to the write() methods
    may be discarded. Furthermore, when write sizes do not align exactly with
    the buffer size, the output time stamp may be interpolated. If precise
    time stamps are required, buffering should not be used.

    See Also:
        OutputStream
    """
    def __init__(self, sri, port, dtype=list):
        """
        Create a BufferedOutputStream.

        Warning:
            Output streams are created via an output port. This constructor
            should not be called directly.

        See Also:
            OutputPort.createStream
            OutputPort.getStream
        """
        OutputStream.__init__(self, sri, port, dtype)
        self.__buffer = self._dtype()
        self.__bufferSize = 0
        self.__bufferTime = bulkio.timestamp.notSet()

    def write(self, data, time):
        """
        Writes data to the stream.

        Data is reformatted as necessary to match the port's requirements. For
        example, char or octet ports will pack numeric values into a binary
        string. In the case of bit data, string literals will be parsed into a
        bitbuffer.

        If buffering is disabled, `data` is sent as a single packet with the
        given time stamp.

        When buffering is enabled, `data` is copied into the internal buffer.
        If the internal buffer exceeds the configured buffer size, one or more
        packets will be sent.

        Args:
            data:      Sample data to write.
            time:      Time stamp of first sample.

        See Also:
            bufferSize()
            setBufferSize()
        """
        # Allow the port to reformat the data in its natural format
        data = self._port._reformat(data)

        # If buffering is disabled, or the buffer is empty and the input data
        # is large enough for a full buffer, send it immediately
        if self.__bufferSize == 0 or (not self.__buffer and (len(data) >= self.__bufferSize)):
            self._send(data, time, False)
        else:
            self._doBuffer(data, time)

    def bufferSize(self):
        """
        Gets the internal buffer size.

        The buffer size is in terms of real samples, ignoring the complex mode
        of the stream. Complex samples count as two real samples for the
        purposes of buffering.

        A buffer size of 0 indicates that buffering is disabled.

        Returns:
            int: Number of real samples to buffer per push.
        """
        return self.__bufferSize

    def setBufferSize(self, samples):
        """
        Sets the internal buffer size.

        The internal buffer is flushed if samples is less than the number of
        real samples currently buffered.

        A buffer size of 0 disables buffering, flushing any buffered data.

        Args:
            samples: Number of real samples to buffer per push.

        Raises:
            ValueError: If samples is negative.
        """
        size = int(samples)
        if size < 0:
            raise ValueError('buffer size cannot be negative')
        self.__bufferSize = size

        # If the new buffer size is less than (or exactly equal to) the
        # currently buffered data size, flush
        if self.__bufferSize <= len(self.__buffer):
            self.flush()

    def flush(self):
        """
        Flushes the internal buffer.

        Any data in the internal buffer is sent to the port to be pushed.
        """
        if not self.__buffer:
            return
        self._flush(False)

    def close(self):
        """
        Closes the stream.

        Sends an end-of-stream packet with any remaining buffered data. No
        further operations should be made on the stream.
        """
        if self.__buffer:
            # Add the end-of-stream marker to the buffered data and its
            # timestamp
            self._flush(True)
        else:
            OutputStream.close(self)

    def _modifyingStreamMetadata(self):
        # Flush any data queued with the old SRI
        self.flush()

        # Post-extend base class method
        OutputStream._modifyingStreamMetadata(self)

    def _flush(self, eos):
        self._send(self.__buffer, self.__bufferTime, eos)
        self.__buffer = self._dtype()

    def _doBuffer(self, data, time):
        # If this is the first data being queued, use its timestamp for the
        # start time of the buffered data
        if not self.__buffer:
            self.__bufferTime = copy.copy(time)

        # Only buffer up to the currently configured buffer size
        count = min(len(data), self.__bufferSize - len(self.__buffer));
        self.__buffer += data[:count]

        # Flush if the buffer is full
        if len(self.__buffer) >= self.__bufferSize:
            self._flush(False)

        # Handle remaining data
        if count < len(data):
            next = time + self.xdelta * count
            self._doBuffer(data[count:], next)


def _unpack_complex(data, dtype):
    # Yields alternating real and imaginary elements from a sequence of complex
    # values (or real values treated as complex values, where the imaginary
    # portion is always 0), converted to a desired data type
    for item in data:
        yield dtype(item.real)
        yield dtype(item.imag)
    
def _complex_to_interleaved(data, dtype):
    # Turns a sequence of complex values into a list with the real and
    # imaginary elements interleaved
    return list(_unpack_complex(data, dtype))

class NumericOutputStream(BufferedOutputStream):
    """
    BulkIO output stream class for numeric data types.

    NumericOutputStream extends BufferedOutputStream to add support for complex
    data and data reformatting.

    See Also:
        BufferedOutputStream
        OutputStream
    """
    def __init__(self, sri, port, dtype, elemType):
        """
        Create a NumericOutputStream.

        Warning:
            Output streams are created via an output port. This constructor
            should not be called directly.

        See Also:
            OutPort.createStream
            OutPort.getStream
        """
        BufferedOutputStream.__init__(self, sri, port, dtype)
        self._elemType = elemType

    def write(self, data, time, interleaved=False):
        """
        Writes sample data to the stream.

        If this stream is configured for complex data, `data` is treated as a
        list of complex values. The real and imaginary elements are interleaved
        into a list of real numbers.

        When `data` is already an interleaved list of real values, setting the
        optional `interleaved` keyword argument will skip the complex-to-real
        interleaving.

        For char or octet streams, real values are packed into a binary string
        after applying complex-to-real conversion (if required).

        Buffering behavior is inherited from BufferedOutputStream.write().

        Args:
            data:        Sample data to write.
            time:        Time stamp of first sample.
            interleaved: Indicates whether complex data is already interleaved.

        See Also:
            BufferedOutputStream.write()
            NumericOutputStream.complex
        """
        if not interleaved and self.complex:
            data = _complex_to_interleaved(data, self._elemType)
        BufferedOutputStream.write(self, data, time)


class OutFileStream(OutputStream):
    """
    File output stream class.

    See Also:
        OutputStream
    """
    def __init__(self, sri, port):
        """
        Create an OutFileStream.

        Warning:
            Output streams are created via an output port. This constructor
            should not be called directly.

        See Also:
            OutFilePort.createStream
            OutFilePort.getStream
        """
        OutputStream.__init__(self, sri, port, str)

    def write(self, data, time):
        """
        Writes a file URI to the stream.

        The URI is sent as a single packet with the given time stamp. File
        streams do not support buffering.

        Args:
            data: The file URI to write.
            time: Time stamp of file URI.
        """
        self._send(data, time, False)


class OutXMLStream(OutputStream):
    """
    XML output stream class.

    See Also:
        OutputStream
    """
    def __init__(self, sri, port):
        """
        Create an OutXMLStream.

        Warning:
            Output streams are created via an output port. This constructor
            should not be called directly.

        See Also:
            OutXMLPort.createStream
            OutXMLPort.getStream
        """
        OutputStream.__init__(self, sri, port, str)

    def write(self, data):
        """
        Writes XML data to the stream.

        The XML document `data` is sent as a single packet. XML streams do not
        support time stamps or buffering.

        Args:
            data: An XML string.
        """
        # Add a "null" timestamp to adapt to the base class method
        self._send(data, None, False)

    def _pushPacket(self, port, data, time, eos, streamID):
        # Drop the time stamp from the base class
        port.pushPacket(data, eos, streamID)
