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

def _bitmask(bits):
    return (1<<bits)-1

def _split_index(pos):
    byte = pos // 8
    bit = pos & 7
    return byte, bit

def _write_bits(dest, dbyte, dbit, value, bits):
    # Preserves leftmost and rightmost bits as necessary
    shift = 8 - (dbit + bits)
    mask = _bitmask(bits) << shift
    dest[dbyte] = (dest[dbyte] & ~mask) | ((value << shift) & mask)

def _read_split_byte(src, sbyte, offset):
    shift = 8 - offset
    value = ((src[sbyte] << 8) | src[sbyte+1]) >> shift
    return value & 0xFF

def _read_split_bits(src, sbyte, offset, bits):
    value = src[sbyte] << 8
    last = offset + bits - 1
    value |= src[sbyte + (last//8)]
    shift = 15 - last
    return (value >> shift) & ((1 << bits) - 1)

def _copy_bits(dest, dstart, src, sstart, count):
    dbyte, dbit = _split_index(dstart)
    sbyte, sbit = _split_index(sstart)

    # If the left hand side is not byte-aligned, copy a sub-byte number of bits
    # so that remaining iterations are aligned
    if dbit > 0:
        nbits = min(8 - dbit, count)
        value = _read_split_bits(src, sbyte, sbit, nbits)
        _write_bits(dest, dbyte, dbit, value, nbits)

        # Advance to the next byte for the left hand side, and adjust
        # the offset for the right hand side (which may advance to the
        # next byte as well)
        dbyte += 1
        sbit += nbits
        sbyte += sbit / 8
        sbit = sbit & 7
        count -= nbits

    # Left offset is now guaranteed to be 0; if the right offset is also 0,
    # then both sides are exactly byte-aligned
    bytes = count // 8
    if sbit == 0:
        dest[dbyte:dbyte+bytes] = src[sbyte:sbyte+bytes]
    else:
        # The two bit arrays are not exactly aligned; iterate through each
        # byte from the left-hand side
        for pos in xrange(bytes):
            dest[dbyte+pos] = _read_split_byte(src, sbyte+pos, sbit)
    dbyte += bytes
    sbyte += bytes

    # If less than a full byte remains, process it
    remain = count & 7
    if remain > 0:
        value = _read_split_bits(src, sbyte, sbit, remain)
        _write_bits(dest, dbyte, 0, value, remain)

def _unpack(src, start, count):
    byte, bit = _split_index(start)

    last_byte = byte + (bit + count + 7) // 8
    for pos in xrange(byte, last_byte):
        nbits = min(8 - bit, count)

        # Use the first (inclusive) and last (exclusive) bits to determine the
        # shift range
        first = 7 - bit
        last = first - nbits
        value = src[pos]
        for shift in range(first, last, -1):
            yield (value >> shift) & 1

        # Subsequent bytes should be aligned
        bit = 0
        count -= nbits

# Lookup table with each byte exploded into a list of 8 bit values
_exploded = [[(x>>shift)&1 for shift in range(7,-1,-1)] for x in range(256)]

def _unpack(src, start, count):
    byte, bit = _split_index(start)

    if bit > 0:
        nbits = min(8 - bit, count)
        items = _exploded[src[byte]]
        for item in items[bit:bit+nbits]:
            yield item
        count -= nbits
        byte += 1

    bytes = count // 8
    for value in src[byte:byte+bytes]:
        for item in _exploded[value]:
            yield item
    byte += bytes

    remain = count & 7
    if remain > 0:
        items = _exploded[src[byte]]
        for item in items[:remain]:
            yield item

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

    bitbuffer provides read/write bit-level access to a backing array of raw
    byte data, in addition to higher-level bit string operations. It supports
    common Python sequence operations, such as len(), element access, slicing
    and iteration.

    Creating a bitbuffer from a string:
    
        buf = bitbuffer('10101101010011')

    Creating a bitbuffer from an integer, such as a hexadecimal literal:

        buf = bitbuffer(0x1AC07B23, 29)

    Creating a bitbuffer from an iterable:

        buf = bitbuffer(x & 1 for x in xrange(64))

    Indexing:
    Individual bits are accessed by index. The value of a bit is treated as an
    int with a value of 0 or 1 (as opposed to a bool). When setting a bit, the
    value is first converted to an int, and any non-zero value is considered to
    be 1.

    Slicing:
    Standard Python slice syntax is supported. For example, to select 64 bits
    starting at offset 16:
        
        buf2 = buf[16:80]

    For efficiency, object slices share the backing byte array instead of
    creating a new copy. Modifications to the slice affect the original buffer,
    and vice-versa. To make a unique copy that does not share data, use the
    `copy` module:

        import copy
        buf2 = copy.copy(buf[16:80])

    Integer Conversion:
    A bitbuffer may be converted to an integer with the int() method, e.g.:

        val = int(buf)

    Use slicing to select a range of bits:

        val = int(buf[8:16])

    Integer conversion is performed in big-endian order, starting at the most
    significant bit. The returned value is right-justified (i.e., the least-
    significant N bits contain the value). If the value exceeds the range of
    int, it is automatically converted to a long.
    """
    def __init__(self, data=None, bits=None, start=None):
        """
        Create a bitbuffer.

        If `data` is not specified, enough space to hold `bits` bits is
        allocated. The bits are not initialized. If `bits` is not given, a 0-
        length bitbuffer is created.

        If `data` is given, the initial value depends on the type of `data`:
            * str:       Parse `data` as a string of 0's and 1's.
            * bitbuffer: Creates an alias to `data`.
            * bytearray: Use `data` as backing byte array.
            * integer:   Store `bits` bits from `data` starting at MSB.
            * iterable:  Convert each item in `data` to a bit value.

        When `bits` is not given, the resulting number of bits is determined
        from `data` if possible. If `data` is an integral type (int, long),
        `bits` must be specified.

        The optional `start` argument may be used to discard bits from the
        beginning of `data`.

        Args:
            data:  Object to convert to bitbuffer value.
            bits:  Number of bits.
            start: Index of first bit (default 0).

        Raises:
            TypeError:  If `data` is an int or long but `bits` is not given.
            ValueError: If `data` is a string and contains characters other
                        than '0' and '1'.
            ValueError: If `data` is a sequence or iterable and contains an
                        item that cannot be converted to int.
        """
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
            pos = self._check_index(pos)
            byte, bit = self._split_index(pos)
            return (self.__data[byte] >> (7-bit)) & 1

    def __setitem__(self, pos, value):
        if isinstance(pos, slice):
            # Set via a slice object; indices() takes care of start/stop range,
            # there is no need for an explicit check
            start, stop, step = pos.indices(len(self))

            # With normal stride and a bitbuffer as the source, use byte-wise
            # copy method (~1000x faster in some cases)
            if step == 1 and isinstance(value, bitbuffer):
                self._assign(start, stop, value)
                return

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
            pos = self._check_index(pos)
            byte, bit = self._split_index(pos)
            mask = 1 << (7 - bit)
            value = mask if int(value) else 0
            self.__data[byte] = (self.__data[byte] & ~mask) | value

    def __iter__(self):
        return _unpack(self.__data, self.__start, self.__bits)

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
        return bitbuffer(bytearray(self.__data), self.__bits, self.__start)

    def __add__(self, other):
        # Convert incoming value to a bitbuffer, because at the very least we
        # need to know its size
        if not isinstance(other, bitbuffer):
            other = bitbuffer(other)
        bits = len(self) + len(other)
        result = bitbuffer(bits=bits)
        result[:len(self)] = self
        result[len(self):] = other
        return result

    def bytes(self):
        """
        Returns a raw byte string containing all bits from this bitbuffer,
        left-aligned.
        """
        byte, bit = self._split_index(0)
        if bit == 0:
            # Bit data is byte aligned, convert the backing array to bytes
            return bytes(self.__data[byte:])
        else:
            # Unaligned, create a new bitbuffer with a copy of the data, which
            # will be aligned
            temp = bitbuffer(bits=len(self))
            temp[:] = self
            return temp.bytes()

    def unpack(self):
        """
        Unpacks the bits into a list of integers, one per bit.
        """
        return list(iter(self))

    def popcount(self):
        """
        Returns the population count (number of 1's).
        """
        return sum(self)

    def distance(self, other):
        """
        Determines the Hamming distance from a sequence or iterable.

        Args:
            other: Another bitbuffer or iterable to compare with.

        Returns:
            int: Number of bits that are different.
        """
        return sum(x^bool(y) for x,y in zip(self, other))

    def find(self, pattern, start=0, end=None, maxDistance=0):
        """
        Finds a pattern in this bitbuffer within a maximum Hamming distance.

        Starting from `start` and ending at `end` (or the end of the bitbuffer
        if `end` is not given), searches forward for a position at which the
        Hamming distance between this bitbuffer and `pattern` is less than or
        equal to `maxDistance`.

        Args:
            pattern:     Bit pattern to search for.
            start:       Starting bit index.
            end:         Ending bit index.
            maxDistance: Maximum allowable Hamming distance.

        Returns:
            int: Bit index of first occurrence of pattern, or -1 if pattern was
            not found.
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
        """
        Performs a take/skip operation to create a new bitbuffer.

        Alternately copies `take` bits and skips `skip` bits from the range
        [start, end) into a new bitbuffer.

        Args:
            take:  Number of bits to copy per iteration.
            skip:  Number of bits to skip per iteration.
            start: Index of first bit (default 0).
            end:   Index of last bit, exclusive (default end).

        Returns:
            New bitbuffer with requested bits.
        """
        start, end, step = self._indices(start, end)
        return bitbuffer(takeskip(self[start:end], take, skip))

    def _assign(self, start, end, other):
        bits = end - start
        if bits != len(other):
            raise ValueError('attempt to assign sequence of size %d to extended slice of size %d' % (len(other), bits))
        _copy_bits(self.__data, self.__start + start, other.__data, other.__start, bits)

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
        return _split_index(pos + self.__start)
