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

#include <ossie/bitbuffer.h>
#include <ossie/bitops.h>

using redhawk::shared_bitbuffer;

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
    if ((pos + bits) > size()) {
        throw std::out_of_range("redhawk::shared_bitbuffer::getint()");
    }
    return bitops::getint(data(), offset() + pos, bits);
}

redhawk::bitbuffer shared_bitbuffer::copy() const
{
    bitbuffer result(this->size());
    this->_M_copy(result);
    return result;
}

void shared_bitbuffer::trim(size_t start, size_t end)
{
    _M_check_index(start, "redhawk::shared_bitbuffer::trim");
    // Limit end index to one past the last bit
    end = std::min(end, size());
    _M_offset += start;
    _M_size = (end - start);
    _M_normalize();
}

shared_bitbuffer shared_bitbuffer::slice(size_t start, size_t end) const
{
    _M_check_index(start, "redhawk::shared_bitbuffer::slice()");
    shared_bitbuffer result(*this);
    result.trim(start, end);
    return result;
}

void shared_bitbuffer::swap(shared_bitbuffer& other)
{
    this->_M_swap(other);
}

int shared_bitbuffer::popcount() const
{
    return bitops::popcount(data(), offset(), size());
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

void shared_bitbuffer::_M_copy(bitbuffer& dest) const
{
    redhawk::bitops::copy(dest.data(), dest.offset(), data(), offset(), size());
}

void shared_bitbuffer::_M_swap(shared_bitbuffer& other)
{
    _M_memory.swap(other._M_memory);
    std::swap(_M_base, other._M_base);
    std::swap(_M_offset, other._M_offset);
    std::swap(_M_size, other._M_size);
}

void shared_bitbuffer::_M_normalize()
{
    _M_base += (_M_offset / 8);
    _M_offset &= 7;
}

void shared_bitbuffer::_M_check_index(size_t pos, const char* name) const
{
    if (pos >= size()) {
        throw std::out_of_range(name);
    }
}

using redhawk::bitbuffer;

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

bitbuffer::bitbuffer() :
    shared_bitbuffer()
{
}

bitbuffer::data_type* bitbuffer::data()
{
    return _M_base;
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
    if ((pos + bits) > size()) {
        throw std::out_of_range("redhawk::bitbuffer::setint()");
    }
    bitops::setint(data(), offset() + pos, value, bits);
}

bitbuffer bitbuffer::slice(size_t start, size_t end)
{
    _M_check_index(start, "redhawk::bitbuffer::slice()");
    bitbuffer temp(*this);
    temp.trim(start, end);
    return temp;
}

void bitbuffer::replace(size_t pos, size_t bits, const shared_bitbuffer& src, size_t srcpos)
{
    redhawk::bitops::copy(data(), offset() + pos, src.data(), src.offset() + srcpos, bits);
}

void bitbuffer::swap(bitbuffer& other)
{
    this->_M_swap(other);
}

void bitbuffer::_M_resize(bitbuffer& dest)
{
    size_t bits = std::min(size(), dest.size());
    redhawk::bitops::copy(dest.data(), dest.offset(), data(), offset(), bits);
    this->swap(dest);
}

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
