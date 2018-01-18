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

#ifndef REDHAWK_BITBUFFER_H
#define REDHAWK_BITBUFFER_H

#include <stdint.h>
#include <stddef.h>

#include "refcount_memory.h"

namespace redhawk {

    class bitbuffer;

    class shared_bitbuffer
    {
    public:
        typedef unsigned char data_type;
        static const size_t npos = (size_t)-1;

        shared_bitbuffer();
        shared_bitbuffer(data_type* ptr, size_t bits);
        ~shared_bitbuffer();

        bool empty() const;

        size_t size() const;

        const data_type* data() const;
        size_t offset() const;

        int operator[] (size_t pos) const;

        bitbuffer copy() const;

        void trim(size_t start, size_t end=npos);

        shared_bitbuffer slice(size_t start, size_t end=npos) const;

        void swap(shared_bitbuffer& other);

        /**
         * @brief  Returns true if the bit array's lifetime is not managed.
         *
         * Transient shared_bitbuffers do not own the underlying data. If the
         * receiver of a transient buffer needs to hold on to it past the
         * lifetime of the call, they must make a copy.
         */
        bool transient() const
        {
            return !(this->_M_memory);
        }

    protected:
        template <class Alloc>
        shared_bitbuffer(size_t bits, const Alloc& allocator) :
            _M_memory(_M_bits_to_bytes(bits), allocator),
            _M_base((data_type*) _M_memory.address()),
            _M_offset(0),
            _M_size(bits)
        {
        }

        static inline size_t _M_bits_to_bytes(size_t bits)
        {
            return (bits + 7) / 8;
        }

        void _M_normalize();

        refcount_memory _M_memory;
        data_type* _M_base;
        size_t _M_offset;
        size_t _M_size;
    };

    class bitbuffer : public shared_bitbuffer
    {
    public:
        explicit bitbuffer(size_t bits);

        template <class Alloc>
        bitbuffer(size_t bits, const Alloc& allocator) :
            shared_bitbuffer(bits, allocator)
        {
        }

        data_type* data();

        void fill(size_t start, size_t end, bool value);
    };
}

#endif // REDHAWK_BITBUFFER_H
