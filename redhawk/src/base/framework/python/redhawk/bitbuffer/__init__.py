#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import copy
import itertools
import numbers

__all__ = ('bitbuffer', 'takeskip')

def _bits_to_bytes(bits):
    return int((bits + 7)/8)

def _char_to_bit(ch):
    bit = '01'.find(ch)
    if bit < 0:
        raise ValueError("invalid character '%s'" % ch)
    return bit

class counted_iterator(object):
    """
    Helper iterator wrapper that keeps track of the number of iterations that
    were performed.
    """
    def __init__(self, iterable):
        self.count = 0
        self.iter = iter(iterable)

    def __iter__(self):
        for item in self.iter:
            self.count += 1
            yield item

class biterator(object):
    """
    Helper binary iterator that yields one bit of an integer value per
    iteration, starting with the most significant bit.
    """
    def __init__(self, value, bits):
        self.value = value
        self.bits = bits

    def __iter__(self):
        for shift in xrange(self.bits-1, -1, -1):
            yield (self.value >> shift) & 1
        

def _iterable_to_bytes(iterable, func):
    value = 0
    shift = 7
    for item in iterable:
        bit = 1 if func(item) else 0
        value = value | (bit << shift)
        shift -= 1
        if shift < 0:
            yield value
            value = 0
            shift = 7

    # If the shift value is not 7, there are some bits stored in value that
    # must be returned
    if shift != 7:
        yield value


def takeskip(iterable, take, skip):
    """
    Generator function to perform a take/skip operation on any iterable.
    Returns 'take' elements from the iterable, then discards 'skip' elements,
    until the iterable is exhausted.
    """
    it = iter(iterable)
    while True:
        for _ in xrange(take):
            yield it.next()
        for _ in xrange(skip):
            it.next()

class bitbuffer(object):
    """
    A sequence container for bit data.
    """
    def __init__(self, data=None, bits=None, start=None):
        if data is None:
            # No data given, create a backing byte array
            if bits is None:
                bits = 0
            data = bytearray(_bits_to_bytes(bits))
        elif isinstance(data, bitbuffer):
            # Copy constructor, reference the same byte array, and if the size
            # and start were not given, get them from the other bitbuffer
            if bits is None:
                bits = len(data)
            if start is None:
                start = data.__start
            data = data.__data
        elif isinstance(data, bytearray):
            # Already a byte array, just determine the size if not given
            if bits is None:
                bits = len(data) * 8
        else:
            if isinstance(data, numbers.Integral):
                # Integer: copy bits starting at MSB, using iterator
                if bits is None:
                    raise TypeError('integer given with no bit count')
                data = biterator(data, bits)
                func = int
            elif isinstance(data, basestring):
                # String: parse as binary string
                func = _char_to_bit
            else:
                # Anything else (list, generator expression, etc.), convert via
                # integer
                func = int

            # Use a counted iterator to fill the byte array; this is helpful
            # when given a generator expression, where you can't know the
            # number of bits in advance and the backing bytearray is quantized
            # to bytes.
            it = counted_iterator(data)
            data = bytearray(_iterable_to_bytes(it, func))
            if bits is None:
                bits = it.count

        # If no start offset was given, start at 0
        if start is None:
            start = 0

        self.__data = data
        self.__bits = bits
        self.__start = start

    def __getitem__(self, pos):
        if isinstance(pos, slice):
            # Get via a slice object; indices() takes care of start/stop range,
            # there is no need for an explicit check
            start, stop, step = pos.indices(len(self))
            if step == 1:
                # Return a view using the same underlying array
                bits = stop - start
                start_bit = self.__start + start
                return bitbuffer(self, bits, start_bit)
            else:
                # Create a new bitbuffer by striding through this one
                return bitbuffer(self[pos] for pos in xrange(start, stop, step))
        else:
            # Get an individual bit
            byte, bit = self._split_index(pos)
            return (self.__data[byte] >> (7-bit)) & 1

    def __setitem__(self, pos, value):
        if isinstance(pos, slice):
            # Set via a slice object; indices() takes care of start/stop range,
            # there is no need for an explicit check
            start, stop, step = pos.indices(len(self))
            indices = xrange(start, stop, step)
            bits = len(indices)
            try:
                value_len = len(value)
            except TypeError:
                value = itertools.repeat(value)
                value_len = bits

            if value_len != bits:
                raise ValueError('attempt to assign sequence of size %d to extended slice of size %d' % (value_len, bits))

            for index, val in zip(indices, value):
                self[index] = val
        else:
            # Set an individual bit
            byte, bit = self._split_index(pos)
            mask = 1 << (7 - bit)
            value = mask if int(value) else 0
            self.__data[byte] = (self.__data[byte] & ~mask) | value

    def __len__(self):
        return self.__bits

    def __repr__(self):
        return "bitbuffer('%s')" % self

    def __str__(self):
        return ''.join(str(x) for x in self)

    def __eq__(self, other):
        if len(self) != len(other):
            return False
        if isinstance(other, basestring):
            func = _char_to_bit
        else:
            func = bool
        for lhs, rhs in zip(self, other):
            if lhs != func(rhs):
                return False
        return True

    def __int__(self):
        value = 0
        for bit in self:
            value = (value << 1) | bit
        return value

    def __hex__(self):
        return hex(int(self))

    def __copy__(self):
        # Make a copy of the data array so that modifications to the copy do
        # not affect this instance. This is consistent with bytearray itself
        # and numpy arrays.
        return bitbuffer(copy.copy(self.__data), self.__bits, self.__start)

    def bytes(self):
        """
        Returns a left-aligned byte array containing all bits from this
        bitbuffer.
        """
        byte, bit = self._split_index(0)
        if bit == 0:
            return self.__data[byte:]
        else:
            return bytearray(_iterable_to_bytes(self, int))

    def resize(self, bits):
        """
        Resizes to the specified number of bits.
        """
        # Create a new backing bytearray and copy existing values (up to a
        # maximum of 'bits') into it
        data = bytearray(_bits_to_bytes(bits))
        for pos, val in enumerate(_iterable_to_bytes(self[:bits], int)):
            data[pos] = val
        self.__data = data
        self.__bits = bits
        self.__start = 0

    def unpack(self):
        """
        Unpacks the bits into a list of integers, one per bit.
        """
        return [x for x in self]

    def popcount(self):
        """
        Returns the population count (number of 1's).
        """
        return sum(self)

    def distance(self, other):
        """
        Determines the Hamming distance from a sequence or iterable.
        """
        return sum(x^bool(y) for x,y in zip(self, other))

    def find(self, pattern, start=0, end=None, maxDistance=0):
        """
        Finds a pattern in this bitbuffer within a maximum Hamming distance.
        """
        # Explicitly convert the pattern to a bitbuffer to take care of string
        # parsing, generator functions, etc.
        pattern = bitbuffer(pattern)
        length = len(pattern)

        # Get bounded indices for start and end (ignoring step)
        start, end, step = self._indices(start, end)

        # Clamp end to the last position at which there is a full pattern
        # length to do the comparison
        end = min(end, len(self) - length)

        for pos in xrange(start, end):
            if pattern.distance(self[pos:pos+length]) <= maxDistance:
                return pos
        return -1

    def takeskip(self, take, skip, start=0, end=None):
        start, end, step = self._indices(start, end)
        return bitbuffer(takeskip(self[start:end], take, skip))

    def _indices(self, start, end, step=None):
        # Use slice to return properly bounded indices
        return slice(start, end, step).indices(len(self))

    def _check_index(self, pos):
        if pos < 0:
            pos = self.__bits + pos
        if pos >= self.__bits or pos < 0:
            raise IndexError('bit index out of range')
        return pos

    def _split_index(self, pos):
        # Given a bit index, returns the index of the byte that contains that
        # bit index, and its index relative to that byte.
        pos = self._check_index(pos)
        pos += self.__start
        return (pos / 8, pos % 8)
