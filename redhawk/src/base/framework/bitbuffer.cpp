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

#include <stdexcept>

#include <ossie/bitbuffer.h>
#include <ossie/bitops.h>

using redhawk::shared_bitbuffer;
using redhawk::bitbuffer;

//
// shared_bitbuffer implementation
//

// Declare npos here so that storage is allocated for it, even though it most
// cases it is used directly as a constant; otherwise, some uses may fail to
// link.
const size_t shared_bitbuffer::npos;

shared_bitbuffer::shared_bitbuffer() :
    _M_memory(),
    _M_base(0),
    _M_offset(0),
    _M_size(0)
{
}

shared_bitbuffer::shared_bitbuffer(data_type* ptr, size_t bits) :
    _M_memory(ptr, _M_bits_to_bytes(bits)),
    _M_base(ptr),
    _M_offset(0),
    _M_size(bits)
{
}

bool shared_bitbuffer::empty() const
{
    return (_M_size == 0);
}

size_t shared_bitbuffer::size() const
{
    return _M_size;
}

const shared_bitbuffer::data_type* shared_bitbuffer::data() const
{
    return _M_base;
}

size_t shared_bitbuffer::offset() const
{
    return _M_offset;
}

int shared_bitbuffer::operator[] (size_t pos) const
{
    return bitops::getbit(data(), offset() + pos);
}

uint64_t shared_bitbuffer::getint(size_t pos, size_t bits) const
{
    _M_check_pos(pos + bits, size(), "redhawk::shared_bitbuffer::getint()");
    return bitops::getint(data(), offset() + pos, bits);
}

void shared_bitbuffer::trim(size_t start, size_t end)
{
    // Check indices for range, which may update end if it was not given, or
    // larger than the source size.
    _M_check_range(start, end, size(), "redhawk::shared_bitbuffer::trim");
    _M_offset += start;
    _M_size = (end - start);

    // Normalize base pointer and offset, so that offset is always in the range
    // [0, 8)
    _M_base += (_M_offset / 8);
    _M_offset &= 7;
}

shared_bitbuffer shared_bitbuffer::slice(size_t start, size_t end) const
{
    shared_bitbuffer result(*this);
    result.trim(start, end);
    return result;
}

void shared_bitbuffer::swap(shared_bitbuffer& other)
{
    _M_memory.swap(other._M_memory);
    std::swap(_M_base, other._M_base);
    std::swap(_M_offset, other._M_offset);
    std::swap(_M_size, other._M_size);
}

int shared_bitbuffer::popcount() const
{
    return bitops::popcount(data(), offset(), size());
}

int shared_bitbuffer::distance(const shared_bitbuffer& other) const
{
    return bitops::hammingDistance(data(), offset(), other.data(), other.offset(), other.size());
}

size_t shared_bitbuffer::find(size_t start, const shared_bitbuffer& pattern, int maxDistance) const
{
    return bitops::find(data(), offset() + start, size(), pattern.data(), pattern.offset(), pattern.size(), maxDistance);
}

shared_bitbuffer shared_bitbuffer::make_transient(const data_type* data, size_t start, size_t bits)
{
    shared_bitbuffer result;
    result._M_base = const_cast<data_type*>(data);
    result._M_offset = start;
    result._M_size = bits;
    return result;
}

void shared_bitbuffer::_M_check_pos(size_t pos, size_t size, const char* name)
{
    if (pos > size) {
        throw std::out_of_range(name);
    }
}

void shared_bitbuffer::_M_check_range(size_t start, size_t& end, size_t size, const char* name)
{
    _M_check_pos(start, size, name);
    if (end < start) {
        throw std::invalid_argument(std::string(name) + " end is before start");
    }
    end = std::min(end, size);
}

size_t shared_bitbuffer::_M_takeskip_size(size_t size, size_t take, size_t skip)
{
    size_t pass = take + skip;
    size_t iterations = size / pass;
    size_t remain = std::min(take, size % pass);
    return (iterations * take) + remain;
}

//
// bitbuffer::reference implementation
//
bitbuffer::reference::reference(data_type* data, size_t pos) :
    _M_data(data),
    _M_pos(pos)
{
}

bitbuffer::reference::operator int () const
{
    return bitops::getbit(_M_data, _M_pos);
}

bitbuffer::reference& bitbuffer::reference::operator=(bool value)
{
    bitops::setbit(_M_data, _M_pos, value);
    return *this;
}

bitbuffer::reference& bitbuffer::reference::operator=(const bitbuffer::reference& other)
{
    return *this = int(other);
}

//
// bitbuffer implementation
//
bitbuffer::bitbuffer() :
    shared_bitbuffer()
{
}

bitbuffer::data_type* bitbuffer::data()
{
    return const_cast<data_type*>(shared_bitbuffer::data());
}

void bitbuffer::fill(size_t start, size_t end, bool value)
{
    size_t bits = end - start;
    bitops::fill(data(), offset() + start, bits, value);
}

bitbuffer::reference bitbuffer::operator[] (size_t pos)
{
    return reference(data(), offset() + pos);
}

void bitbuffer::setint(size_t pos, uint64_t value, size_t bits)
{
    _M_check_pos(pos + bits, size(), "redhawk::bitbuffer::setint()");
    bitops::setint(data(), offset() + pos, value, bits);
}

bitbuffer bitbuffer::slice(size_t start, size_t end)
{
    _M_check_pos(start, size(), "redhawk::bitbuffer::slice()");
    bitbuffer temp(*this);
    temp.trim(start, end);
    return temp;
}

void bitbuffer::replace(size_t pos, size_t bits, const shared_bitbuffer& src, size_t srcpos)
{
    redhawk::bitops::copy(data(), offset() + pos, src.data(), src.offset() + srcpos, bits);
}

size_t bitbuffer::takeskip(size_t pos, const shared_bitbuffer& src, size_t take, size_t skip, size_t start, size_t end)
{
    // Check indices for range, which may update end if it was not given, or
    // larger than the source size.
    _M_check_range(start, end, src.size(), "redhawk::bitbuffer::takeskip");
    // Check size of destination to ensure it can hold enough bits
    size_t count = end - start;
    size_t required = _M_takeskip_size(count, take, skip);
    if ((size() - pos) < required) {
        throw std::length_error("redhawk::bitbuffer::takeskip");
    }
    // Account for internal bit offsets
    pos += offset();
    start += src.offset();
    return bitops::takeskip(data(), pos, src.data(), start, count, take, skip);
}

void bitbuffer::swap(bitbuffer& other)
{
    // Use base class swap, with the caveat that we have to do a cast so that
    // it can complile (the base class explicitly disallows swapping with a
    // mutable buffer to prevent accidental end-runs around const protection)
    shared_bitbuffer::swap(static_cast<shared_bitbuffer&>(other));
}

void bitbuffer::_M_parse(const std::string& str)
{
    int count = bitops::parseString(data(), offset(), str.c_str(), str.size());
    if (count < (int) str.size()) {
        std::string message = "invalid character '";
        message += str[count];
        message += '\'';
        throw std::invalid_argument(message);
    }
}

void bitbuffer::_M_resize(bitbuffer& dest)
{
    size_t bits = std::min(size(), dest.size());
    redhawk::bitops::copy(dest.data(), dest.offset(), data(), offset(), bits);
    this->swap(dest);
}

//
// global operator implementations
//
bool redhawk::operator==(const shared_bitbuffer& lhs, const shared_bitbuffer& rhs)
{
    if (lhs.size() != rhs.size()) {
        // Different sizes always compare unequal
        return false;
    } else if ((lhs.data() == rhs.data()) && (lhs.offset() == rhs.offset())) {
        // If the data pointer and offsets are the same (the size is already
        // known to be the same), no further comparison is required
        return true;
    } else {
        // Perform element-wise comparison
        return bitops::compare(lhs.data(), lhs.offset(), rhs.data(), rhs.offset(), lhs.size()) == 0;
    }
}

bool redhawk::operator!=(const shared_bitbuffer& lhs, const shared_bitbuffer& rhs)
{
    return !(lhs == rhs);
}
