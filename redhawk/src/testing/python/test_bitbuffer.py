#!/usr/bin/python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK core.
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

import unittest
import copy

from redhawk.bitbuffer import bitbuffer

class BitBufferTest(unittest.TestCase):
    def testConstructor(self):
        # Empty
        buf = bitbuffer()
        self.assertEqual(0, len(buf), 'new empty bitbuffer should be zero-length')
        self.failIf(bool(buf), 'bitbuffer with zero length should evaluate to False')

        # Allocating
        NUM_BITS = 17
        buf = bitbuffer(bits=NUM_BITS)
        self.assertEqual(NUM_BITS, len(buf), 'new bitbuffer should have length 17')
        self.failUnless(bool(buf), 'bitbuffer with non-zero length should evaluate to True')

    def testFromInt(self):
        # Input value is right-aligned (i.e., take lowest 28 bits)
        buf = bitbuffer(0xBADC0DE, 28)
        self.assertEqual(28, len(buf))
        data = buf.bytes()
        self.assertEqual('\xBA\xDC\x0D', data[:3])
        self.assertEqual(0xE0, ord(data[3]) & 0xF0)

        # Ignore some of the most significant bits
        # 00(11 0100 0101 0110 0111 1000)
        #     3    4    5    6    7    8
        # =
        # 1101 0001 0101 1001 1110 00xx
        #    D    1    5    9    E
        buf = bitbuffer(0x12345678, 22)
        self.assertEqual(22, len(buf))
        data = buf.bytes()
        self.assertEqual('\xD1\x59', data[:2])
        self.assertEqual(0xE0, ord(data[2]) & 0xFC)

        # Since value is right-aligned, any higher bits are 0
        # 1111 0101 1010 1101
        #    F    5    A    D
        # =
        # 0000 0001 1110 1011 0101 101x
        #    0    1    E    B    5    A
        buf = bitbuffer(0xF5AD, 23)
        self.assertEqual(23, len(buf))
        data = buf.bytes()
        self.assertEqual('\x01\xEB', data[:2])
        self.assertEqual(0x5A, ord(data[2]) & 0xFE)

    def testFromArray(self):
        # Test with a large array, using the default behavior for number of
        # bits (8 bits for each byte) and start offset (0)
        array = bytearray('\x01\x23\x45\x67\x89\xAB\xCD\xEF\x11\x22')
        buf = bitbuffer(array)
        self.assertEqual(len(array)*8, len(buf))
        self.assertEqual(array, buf.bytes())

        # Test with a non-zero offset and non-integral number of bytes
        buf = bitbuffer(array, bits=18, start=4)
        self.assertEqual(18, len(buf))
        # Expected bytes are input array shifted left one nibble (easy to
        # calulate) with the last 6 bits masked off
        self.assertEqual(b'\x12\x34\x40', buf.bytes())

    def testFromGenerator(self):
        # Use a generator expresion to populate the bit data (0111, repeating)
        buf = bitbuffer(bool(x%4) for x in xrange(48))
        self.assertEqual(48, len(buf))
        expected = '\x77'*6
        self.assertEqual(expected, buf.bytes())

    def testFromList(self):
        # Integers
        int_vals = [1, 0, 1, 1, 0, 1, 0, 1, 0]
        buf = bitbuffer(int_vals)
        self.assertEqual(int_vals, buf)

        # Booleans
        bool_vals = [False, False, True, False, False, True, True, True, False, True]
        buf = bitbuffer(bool_vals)
        self.assertEqual(bool_vals, buf)

    def testFromString(self):
        # Basic binary string
        str_val = '101101001010101001010'
        buf = bitbuffer(str_val)
        self.assertEqual(len(str_val), len(buf))
        for index, (ch, bit) in enumerate(zip(str_val, buf)):
            self.assertEqual(int(ch), bit, 'wrong value at position %d' % index)

        # Invalid input string
        self.assertRaises(ValueError, bitbuffer, '01011002')

    def testBytes(self):
        # Empty bitbuffer should still support bytes()
        buf = bitbuffer()
        data = buf.bytes()
        self.assertEqual(0, len(data))

        # Start with a 32-bit value, with no offset and an exact byte length
        buf = bitbuffer(0xB552B4E1, 32)
        data = buf.bytes()
        self.assertEqual(4, len(data))
        self.assertEqual(b'\xB5\x52\xB4\xE1', data)

        # Offset, not byte-aligned slice; bytes() should create a new copy of
        # the byte array
        # 101(10101010 1001010)110100
        #     10101010|1001010x = 0xAA94
        data = buf[3:18].bytes()
        self.assertEqual(2, len(data))
        self.assertEqual('\xAA', data[0])
        self.assertEqual(0x94, ord(data[1]) & 0xFE)

    def testEquals(self):
        # Fill a bit buffer with a known pattern
        pattern = '11001010010101011011100001101001110000101'
        first = bitbuffer(pattern)

        # A bitbuffer should be rigorously equal to itself
        self.failUnless(first == first)

        # Another bitbuffer with different backing memory should still compare
        # equal
        second = bitbuffer(pattern)
        self.assertEqual(first, second)

        # Flip a bit, the comparison should now fail
        second[17] = not second[17]
        self.assertNotEqual(first, second)

        # Create a new buffer with a different size, but the same data (just
        # offset by few bits). It should compare unequal as-is; however, it
        # should compare equal if taking a slice of the original buffer to
        # re-align them.
        third = bitbuffer(pattern[3:])
        self.assertNotEqual(first, third)
        self.assertEqual(first[3:], third)

    def testEqualsPyObjects(self):
        pattern = '1011010101001011'
        int_vals = [int(ch) for ch in pattern]

        # Should be equal
        buf = bitbuffer(pattern)
        self.assertEquals(pattern, buf)
        self.assertEquals(int_vals, buf)

        # Different lengths
        self.assertNotEquals(pattern[:-2], buf, 'unequal size string compared equal')
        self.assertNotEquals(int_vals[:-2], buf, 'unequal size list compared equal')

        # Flipped bit
        buf[1] = 1
        self.assertNotEquals(pattern, buf, 'unequal string compared equal')
        self.assertNotEquals(int_vals, buf, 'unequal list compared equal')

        # Skip over flipped bit
        self.assertEquals(pattern[2:], buf[2:], 'substring compared not equal')
        self.assertEquals(int_vals[2:], buf[2:], 'list slice compared not equal')

    def testCopy(self):
        # Create a bitbuffer with known data: bit is set if index is odd
        original = bitbuffer(x&1 for x in xrange(127))

        # Make a copy and modify the original; the copy should be unaffected
        copied = copy.copy(original)
        self.assertEqual(original, copied)
        original[2] = 1
        self.assertEqual(0, copied[2])

    def testFill(self):
        # Create a new bitbuffer, with all bits initialized to one
        buf = bitbuffer([1] * 64)

        # Fill the entire bitbuffer with all zeros
        buf[:] = 0
        self.assertEqual([0] * len(buf), buf)

        # Fill a subset of the bitbuffer with ones
        buf[9:33] = 1
        data = buf.bytes()
        self.assertEqual(b'\x7F\xFF\xFF\x80', data[1:5])

        # Implicit offset and non-byte-aligned end
        buf = buf[42:47]
        buf[1:4] = 1
        self.assertEqual([0, 1, 1, 1, 0], buf)

    def testGetItem(self):
        buf = bitbuffer('001010100111100')
        self.assertEqual(0, buf[0])
        self.assertEqual(0, buf[1])
        self.assertEqual(1, buf[2])
        self.assertEqual(0, buf[3])
        self.assertEqual(1, buf[4])
        self.assertEqual(0, buf[8])
        self.assertEqual(1, buf[9])

        # Use slice to create a new bit buffer with a non-zero offset to test
        # that the offset is taken into account
        buf2 = buf[11:15]
        self.assertEqual(1, buf2[0])
        self.assertEqual(1, buf2[1])
        self.assertEqual(0, buf2[2])
        self.assertEqual(0, buf2[3])

        # Exceptions
        # Index past end
        self.assertRaises(IndexError, buf.__getitem__, len(buf))
        # Negative index past beginning
        self.assertRaises(IndexError, buf.__getitem__, -(len(buf) + 1))

    def testGetItemSlice(self):
        # Fill a new bit buffer with alternating 0's and 1's
        buf = bitbuffer(x & 1 for x in xrange(12))

        # Take a 4-bit slice from the middle and check that it has the expected
        # bits
        middle = buf[4:8]
        self.assertEqual(4, len(middle))
        self.assertEqual([0, 1, 0, 1], middle)

        # Take a slice from the midpoint to the end, and check that the bits
        # match
        end = buf[6:]
        self.assertEqual(6, len(end))
        self.assertEqual([0, 1, 0, 1, 0, 1], end)

        # Compare the overlap between the two slices by taking sub-slices
        self.assertEqual(middle[2:], end[0:2])

        # Starting at the end index should return an empty bit buffer
        empty = buf[len(buf):]
        self.assertEqual(0, len(empty))

        # Negative indices should be from end
        neg = buf[-6:-2]
        self.assertEqual(4, len(neg))
        self.assertEqual(buf[6:10], neg)

    def testSetItem(self):
        # Start with a zero-filled buffer
        buf = bitbuffer(bits=48)

        # Basic bit setting
        buf[3] = 1
        data = buf.bytes()
        self.assertEqual(0x10, ord(data[0]), 'Set bit')

        # Two bits in the same byte
        buf[8] = 1
        buf[13] = 1
        data = buf.bytes()
        self.assertEqual(0x84, ord(data[1]), 'Set two bits in same byte')

        # Any non-zero integer should be interpreted as a 1
        buf[18] = 2
        buf[22] = -5289
        data = buf.bytes()
        self.assertEqual(0x22, ord(data[2]), 'Set non-zero integer')

        # 0 should clear an existing bit
        buf[8] = 0
        data = buf.bytes()
        self.assertEqual(0x04, ord(data[1]), 'Clear bit')

        # Use a slice to test that offsets are accounted for (the slice shares
        # the same backing byte array)
        buf2 = buf[35:47]
        buf2[1] = 1
        self.assertEqual(1, buf[36], 'Slice with offset')

        # Exceptions
        # Index past end
        self.assertRaises(IndexError, buf.__setitem__, len(buf), 0)
        # Negative index past beginning
        self.assertRaises(IndexError, buf.__setitem__, -(len(buf) + 1), 0)

    def testSetItemSlice(self):
        # Destination is all 0's (allocating ctor zeros byte array)
        dest = bitbuffer(bits=36)
        
        # Set known pattern in source
        src = bitbuffer('10001100110001101101')

        # Replace 9 bits at offset 1
        #  (1000110 0|1)100
        # 0(1000110|0 1)000000 = 0x4640
        dest[1:10] = src[:9]
        data = dest.bytes()
        self.assertEqual(b'\x46\x40', data[:2])

        # Replace 13 bits at offset 22, starting with the 4th bit of the source
        #   1000(11 00|110001 10|1)101
        # 000000(11|00 110001|10 1)0xxxx = 0x0331A
        dest[22:35] = src[4:17]
        data = dest.bytes()
        self.assertEqual(b'\x03\x31', data[2:4])
        self.assertEqual(0xA0, ord(data[4]) & 0xF0)

        # Negative indices should be from end; invert first 3 of the last 4
        # bits (prior value above is 1010)
        dest[-4:-1] = bitbuffer('010')
        data = dest.bytes()
        self.assertEqual(0x40, ord(data[4]) & 0xF0)

    def testToInt(self):
        # Use a 96-bit long
        expected = 0x3545E6A9A11BAAE49A3F3B38
        buf = bitbuffer(expected, 96)

        # Small value
        self.assertEqual(3, int(buf[:4]))

        # Multi-byte with offset
        # 0x3545E6A9 = 001(10101|01000101|11100110|10101001)
        self.assertEqual(0x1545E6A9, int(buf[3:32]))

        # Implicit offset (slice)
        buf2 = buf[2:32]
        self.assertEqual(0x1545E6A9, int(buf2[1:]))

        # Since Python longs are arbitrarily large, try converting the whole
        # bitbuffer into an integer
        self.assertEqual(expected, int(buf))

    def testUnpack(self):
        # Larger, byte-aligned
        expected = [
            1, 0, 1, 0, 0, 1, 1, 0,
            1, 0, 0, 1, 0, 0, 1, 1,
            0, 1, 0, 1, 1, 0, 1, 1,
        ]
        buf = bitbuffer(expected)
        self.assertEqual(expected, buf.unpack())

        # Medium, partial byte at end
        buf2 = buf[:-3]
        self.assertEqual(expected[:-3], buf2.unpack())

        # Medium, unaligned
        buf2 = buf[3:]
        self.assertEqual(expected[3:], buf2.unpack())

        # Small, sub-byte, unaligned
        buf = bitbuffer('0101001', bits=6, start=1)
        self.assertEqual(6, len(buf))
        self.assertEqual([1, 0, 1, 0, 0, 1], buf.unpack())

    def testPopcount(self):
        buf = bitbuffer('10111001000100011101000110111100001')
        self.assertEqual(17, buf.popcount())

        buf = bitbuffer('100011101000110111100')
        self.assertEqual(11, buf.popcount())

        buf = bitbuffer('1010001')
        self.assertEqual(3, buf.popcount())

    def testDistance(self):
        first = bitbuffer('110100010011110100011001111100111')
        second = bitbuffer('000101011111111010011110100000011')

        # Distance from self should always be 0
        self.assertEqual(0, first.distance(first))

        # a         110100010011110100011001111100111
        # b         000101011111111010011110100000011
        # a XOR b = 110001001100001110000111011100100
        self.assertEqual(15, first.distance(second))

    def testFind(self):
        # Pick a oddly-sized pattern (22 bits)
        pattern = bitbuffer('1011000000011110111110')

        # Fill a bit buffer with 1's, then copy the pattern into it in a couple
        # of places
        buf = bitbuffer([1] * 300)
        buf[37:37+len(pattern)] = pattern
        buf[200:200+len(pattern)] = pattern

        # 1-argument find, looks for exact match from start
        self.assertEqual(37, buf.find(pattern))

        # Start after the first occurrence (using both positive and negative
        # indexing)
        self.assertEqual(200, buf.find(pattern, start=59))
        self.assertEqual(200, buf.find(pattern, start=-150))

        # The search should fail when started after both occurrences, or
        # bounded to a range where no occurences exit
        self.assertEqual(-1, buf.find(pattern, start=222))
        self.assertEqual(-1, buf.find(pattern, end=36))
        self.assertEqual(-1, buf.find(pattern, start=60, end=-101))

        # Test that end index is exclusive
        self.assertEqual(-1, buf.find(pattern, end=37))
        self.assertEqual(37, buf.find(pattern, end=38))

        # Introduce some bit errors
        buf[38] = not buf[38]
        buf[48] = not buf[48]
        buf[220] = not buf[220]

        # Try decreasing tolerances
        self.assertEqual(37, buf.find(pattern, maxDistance=2))
        self.assertEqual(200, buf.find(pattern, maxDistance=1))
        self.assertEqual(-1, buf.find(pattern))

        # Starting the search past the end of the bit buffer should always fail,
        # but without an exception
        self.assertEqual(-1, buf.find(pattern, start=len(buf)))

    def testTakeSkip(self):
        # Use a repeating pattern of an irregular length, where the discarded
        # part is disjoint
        taken = '10000100001'
        skipped = '1001'
        buf = bitbuffer((taken+skipped) * 5)
        expected = bitbuffer(taken * 5)
        self.assertEqual(expected, buf.takeskip(11, 4))

        # Use ASCII text, where the high bit is always zero; a start offset is
        # required
        msg = 'Here is some text'
        buf = bitbuffer(bytearray(msg))
        ascii = buf.takeskip(7, 1, start=1)

        # Reconstruct the input text by taking 7 bits at a time
        result = ''.join(chr(int(ascii[bit:bit+7])) for bit in xrange(0, len(ascii), 7))
        self.assertEqual(msg, result)

        # Repeat with a starting and ending offset
        ascii = buf.takeskip(7, 1, start=41, end=97)
        result = ''.join(chr(int(ascii[bit:bit+7])) for bit in xrange(0, len(ascii), 7))
        self.assertEqual(msg[5:12], result)

    def testAdd(self):
        buf1 = bitbuffer(0xADD, 13)
        buf2 = bitbuffer(0xC0DE5, 21)

        # 0xADD (13) = 0101011011101
        # 0xC0DE5 (21) = 011000000110111100101
        # =
        # 0101 0110 1110 1011 0000 0011 0111 1001 01xx
        #    5    6    E    B    0    3    7    9    4
        result = buf1 + buf2
        self.assertEqual(34, len(result))
        data = result.bytes()
        self.assertEqual('\x56\xEB\x03\x79', data[:4])
        self.assertEqual(0x40, ord(data[4]) & 0xC0)

    def testHex(self):
        # Using hex conversion should give same results as equivalent integer
        intval = 0x123456789ABCDEF
        buf = bitbuffer(intval, 60)
        # NB: comparing the result of two hex() expressions avoids potential
        # formatting differences in the output of hex()
        self.assertEqual(hex(intval), hex(buf))

if __name__ == '__main__':
    import runtests
    runtests.main()
