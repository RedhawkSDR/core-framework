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

import bulkio.sri

class StreamBase(object):
    """
    Base class for input and output streams.

    StreamBase encapsulates a single BulkIO stream. It implements the basic
    common API for input and output streams, providing attributes for StreamSRI
    fields.
    """
    def __init__(self, sri):
        self._sri = sri

    @property
    def streamID(self):
        """
        str: The stream's unique identifier.

        The stream ID is immutable and cannot be changed.
        """
        return self._sri.streamID

    @property
    def sri(self):
        """
        BULKIO.StreamSRI: The current stream metadata.
        """
        return self._sri

    @property
    def xstart(self):
        """
        float: Starting coordinate of the first sample in the X direction.

        For contiguous data, this is the start of the stream in terms of
        xunits. For framed data, this specifies the starting abscissa value, in
        terms of xunits, associated with the first element in
        """
        return self._sri.xstart

    @property
    def xdelta(self):
        """
        float: The distance between two adjacent samples in the X direction.

        Because the X-axis is commonly in terms of time (that is, sri.xunits is
        BULKIO.UNITS_TIME), this is typically the reciprocal of the sample
        rate.

        For framed data, this is the interval between consecutive samples in a
        frame.
        """
        return self._sri.xdelta

    @property
    def xunits(self):
        """
        int: The unit code for the xstart and xdelta values.

        Axis units are specified using constants in the BULKIO package.  For
        contiguous data, the X-axis is commonly in terms of time,
        BULKIO.UNITS_TIME. For framed data, the X-axis is often in terms of
        frequency, BULKIO.UNITS_FREQUENCY.
        """
        return self._sri.xunits

    @property
    def subsize(self):
        """
        int: The length of a row for framed data, or 0 if the data is
        contiguous.

        A subsize of 0 indicates that the data is contiguous; this is the
        default setting. For contiguous data, only the X-axis fields are
        applicable.

        A non-zero subsize indicates that the data is framed, with each row
        having a length of subsize. For framed data, both the X-axis and Y-axis
        fields are applicable.
        """
        return self._sri.subsize

    @property
    def ystart(self):
        """
        float: Starting coordinate of the first frame in the Y direction.

        This specifies the start of the stream in terms of yunits.

        Note:
            Y-axis fields are only applicable when subsize is non-zero.
        """
        return self._sri.ystart

    @property
    def ydelta(self):
        """
        float: The distance between two adjacent frames in the Y direction.

        This specifies the interval between frames in terms of yunits.

        Note:
            Y-axis fields are only applicable when subsize is non-zero.
        """
        return self._sri.ydelta

    @property
    def yunits(self):
        """
        int: The unit code for the ystart and ydelta values.

        Axis units are specified using constants in the BULKIO package.

        Note:
            Y-axis fields are only applicable when subsize is non-zero.
        """
        return self._sri.yunits

    @property
    def complex(self):
        """
        bool: The complex mode of this stream.

        A stream is considered complex if sri.mode is non-zero.
        """
        return self._sri.mode != 0

    @property
    def blocking(self):
        """
        bool: The blocking mode of this stream.
        """
        return self._sri.blocking

    @property
    def keywords(self):
        """
        list: User-defined keywords.

        See Also:
            hasKeyword()
            getKeyword()
        """
        return self._sri.keywords

    def hasKeyword(self, name):
        """
        Checks for the presence of a keyword in the SRI.

        Args:
            name: The name of the keyword.

        Returns:
            True:  If keyword `name` exists.
            False: If keyword `name` does not exist.

        See Also:
            bulkio.sri.hasKeyword()
        """
        return bulkio.sri.hasKeyword(self._sri, name)

    def getKeyword(self, name):
        """
        Gets the current value of a keyword in the SRI.

        Allows for easy lookup of keyword values in the SRI. To avoid
        exceptions on missing keywords, the presence of a keyword can be
        checked with hasKeyword().

        Returns:
            Value of keyword `name` as a Python object.

        Raises:
            KeyError: If keyword `name` does not exist.

        See Also:
            bulkio.sri.getKeyword()
        """
        return bulkio.sri.getKeyword(self._sri, name)
