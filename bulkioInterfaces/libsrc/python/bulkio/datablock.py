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

import itertools

from redhawk.bitbuffer import bitbuffer

import bulkio.sri
from bulkio.bulkioInterfaces import BULKIO

__all__ = ('SampleDataBlock', 'BitDataBlock', 'StringDataBlock')

def _get_drift(begin, end, xdelta):
    real = end.time - begin.time
    expected = (end.offset - begin.offset) * self._sri.xdelta
    return real - expected

def packed_to_complex(values):
    real = itertools.islice(values, 0, len(values), 2)
    imag = itertools.islice(values, 1, len(values), 2)
    return [complex(re,im) for re, im in zip(real, imag)]

class SampleTimestamp(object):
    def __init__(self, time, offset=0, synthetic=False):
        self.time = time
        self.offset = offset
        self.synthetic = synthetic

class DataBlock(object):
    def __init__(self, sri, data):
        self._sri = sri
        self._data = data
        self._timestamps = []
        self.sriChangeFlags = bulkio.sri.NONE
        self.inputQueueFlushed = False

    def sri(self):
        return self._sri

    def xdelta(self):
        return self._sri.xdelta

    def sriChanged(self):
        return self.sriChangeFlags != bulkio.sri.NONE

    def data(self):
        return self._data

    def size(self):
        return len(self._data)

    def getStartTime(self):
        return self._timestamps[0].time

    def addTimestamp(self, timestamp, offset=0, synthetic=False):
        self._timestamps.append(SampleTimestamp(timestamp, offset, synthetic))

    def getTimestamps(self):
        """
        Returns the time stamps for the sample data.
        """
        return self._timestamps

    def getNetTimeDrift(self):
        """
        Returns the difference between the expected and actual value of the
        last time stamp
        """
        self._validateTimestamps()
        return _get_drift(self._timestamps[0], self._timestamps[-1], self.xdelta())

    def getMaxTimeDrift(self):
        """
        Returns the largest difference between expected and actual time stamps
        in the block.
        """
        self._validateTimestamps()
        max_drift = 0.0
        for index in xrange(1, len(self._timestamps)):
            drift = _get_drift(self._timestamps[index-1], self._timestamps[index], self.xdelta())
            if abs(drift) > abs(max):
                max_drift = drift
        return max_drift

    def _validateTimestamps(self):
        if not self._timestamps:
            raise Exception('block contains no timestamps')
        elif self._timestamps[0].offset != 0:
            raise Exception('no timestamp at offset 0')


class SampleDataBlock(DataBlock):
    def complex(self):
        return self._sri.mode != 0

    def cxdata(self):
        return packed_to_complex(self._data)

    def cxsize(self):
        return self.size() / 2
