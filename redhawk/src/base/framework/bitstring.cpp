/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#include <ossie/bitstring.h>
#include <ossie/bitops.h>

using redhawk::bitstring;

bitstring::reference::reference(data_type* data, size_type pos) :
    _M_data(data),
    _M_pos(pos)
{
}

bitstring::reference::operator int () const
{
    return bitops::getbit(_M_data, _M_pos);
}

bitstring::reference& bitstring::reference::operator=(bool value)
{
    bitops::setbit(_M_data, _M_pos, value);
    return *this;
}

bitstring::bitstring() :
    _M_data(0),
    _M_size(0)
{
}

bitstring::bitstring(uint64_t value, size_type bits) :
    _M_data(_M_allocate(bits)),
    _M_size(bits)
{
    intval(0, value, bits);
}

bitstring::bitstring(const data_type* data, size_type bits) :
    _M_data(_M_allocate(bits)),
    _M_size(bits)
{
    bitops::copy(_M_data, 0, data, 0, bits);
}

bitstring::bitstring(const bitstring& other) :
    _M_data(_M_allocate(other._M_size)),
    _M_size(other._M_size)
{
    bitops::copy(_M_data, 0, other._M_data, 0, _M_size);
}

bitstring::~bitstring()
{
    if (_M_data) {
        delete[] _M_data;
    }
}

bitstring& bitstring::operator=(const bitstring& other)
{
    bitstring temp(other);
    swap(temp);
    return *this;
}

void bitstring::swap(bitstring& other)
{
    std::swap(_M_data, other._M_data);
    std::swap(_M_size, other._M_size);
}

bool bitstring::empty() const
{
    return (_M_size == 0);
}

bitstring::size_type bitstring::size() const
{
    return _M_size;
}

void bitstring::resize(size_type bits, bool val)
{
    bitstring temp;
    temp._M_data = _M_allocate(bits);
    temp._M_size = bits;
    bitops::copy(temp.data(), 0, data(), 0, std::min(bits, size()));
    if (temp.size() > size()) {
        temp.fill(size(), temp.size(), val);
    }
    swap(temp);
}

bitstring::data_type* bitstring::data()
{
    return _M_data;
}

const bitstring::data_type* bitstring::data() const
{
    return _M_data;
}

bitstring::reference bitstring::operator[] (size_type pos)
{
    return reference(data(), pos);
}

int bitstring::operator[] (size_type pos) const
{
     return bitops::getbit(data(), pos);
}

uint64_t bitstring::intval(size_type pos, size_type bits) const
{
    return bitops::getint(data(), pos, bits);
}

void bitstring::intval(size_type pos, uint64_t value, size_type bits)
{
    bitops::setint(data(), pos, value, bits);
}

bitstring::size_type bitstring::find(const bitstring& pattern, int maxErrors)
{
    return find(0, pattern, maxErrors);
}

bitstring::size_type bitstring::find(size_type start, const bitstring& pattern, int maxErrors)
{
    return bitops::find(data(), start, size(), pattern.data(), 0, pattern.size(), maxErrors);
}

void bitstring::fill(size_type start, size_type end, bool value)
{
    bitops::fill(data(), start, end - start, value);
}

bitstring bitstring::substr(size_type start, size_type end) const
{
    const size_t bits = end - start;
    bitstring result;
    result._M_data = _M_allocate(bits);
    result._M_size = bits;
    bitops::copy(result._M_data, 0, _M_data, start, bits);
    return result;
}

int bitstring::popcount() const
{
    return bitops::popcount(data(), 0, size());
}

bitstring::data_type* bitstring::_M_allocate(size_t bits)
{
    return new data_type[(bits + 7) / 8];
}

bool redhawk::operator==(const bitstring& lhs, const bitstring& rhs)
{
    if (lhs.size() != rhs.size()) {
        return false;
    }
    return (redhawk::bitops::compare(lhs.data(), 0, rhs.data(), 0, lhs.size()) == 0);
}

bool redhawk::operator!=(const bitstring& rhs, const bitstring& lhs)
{
    return !(rhs == lhs); 
}

std::ostream& redhawk::operator<<(std::ostream& oss, const bitstring& str)
{
    const bitstring::data_type* data = str.data();
    for (size_t ii = 0; ii < str.size(); ++ii) {
        size_t offset = ii / 8;
        size_t shift = 7 - (ii % 8);
        // Simple optimization: the value has to be 0 or 1, and the ASCII
        // characters for 0 and 1 are adjacent, so adding the bit value to '0'
        // gives the right value, as long as you cast back to a char (addition
        // promotes to int here).
        oss << (char) ('0' + ((data[offset] >> shift) & 1));
    }
    return oss;
}
