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

#ifndef REDHAWK_BITSTRING_H
#define REDHAWK_BITSTRING_H

#include <ostream>

#include <stdint.h>
#include <stddef.h>

namespace redhawk {

    class bitstring {
    public:
        typedef unsigned char data_type;
        typedef size_t size_type;
        static const size_type npos = (size_type)-1;

    private:
        class reference {
        public:
            reference(data_type* data, size_type pos);
            operator int () const;
            reference& operator= (bool);
        private:
            data_type* _M_data;
            size_type _M_pos;
        };

    public:
        bitstring();

        bitstring(uint64_t value, size_type bits);

        bitstring(const data_type* data, size_type bits);

        bitstring(const bitstring& other);

        ~bitstring();

        bitstring& operator=(const bitstring& other);

        void swap(bitstring& other);

        bool empty() const;

        size_type size() const;
        void resize(size_type bits, bool val=0);

        data_type* data();
        const data_type* data() const;

        reference operator[] (size_type pos);
        int operator[] (size_type pos) const;

        uint64_t intval(size_type pos, size_type bits) const;
        void intval(size_type pos, uint64_t value, size_type bits);

        void fill(size_type start, size_type end, bool value);

        bitstring substr(size_type start, size_type end) const;

        size_type find(const bitstring& pattern, int maxErrors);
        size_type find(size_type start, const bitstring& pattern, int maxErrors);

        int popcount() const;

    private:
        static data_type* _M_allocate(size_t bits);
    
        data_type* _M_data;
        size_type _M_size;
    };

    bool operator==(const bitstring& lhs, const bitstring& rhs);
    bool operator!=(const bitstring& lhs, const bitstring& rhs);
    std::ostream& operator<<(std::ostream& stream, const bitstring& str);
}

#endif // REDHAWK_BITSTRING_H
