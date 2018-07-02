/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef __bulkio_messagebuffer_h
#define __bulkio_messagebuffer_h

#include <vector>

namespace bulkio {
    class MessageBuffer {
    public:
        MessageBuffer(size_t bytes=0) :
            _offset(0)
        {
            _data.resize(bytes);
        }

        char* buffer()
        {
            return &_data[0];
        }

        const char* buffer() const
        {
            return &_data[0];
        }

        size_t size() const
        {
            return _data.size();
        }

        size_t offset() const
        {
            return _offset;
        }

        void resize(size_t size)
        {
            _data.resize(size);
        }

        template <typename U>
        void read(U& val)
        {
            _checkRead(sizeof(U));
            val = *(reinterpret_cast<const U*>(&_data[_offset]));
            _offset += sizeof(U);
        }

        void read(std::string& val)
        {
            size_t length;
            read(length);
            _checkRead(length);
            const char* begin = &_data[_offset];
            const char* end = begin + length;
            val.assign(begin, end);
            _offset += length;
        }

        template <class U>
        void write(const U& val)
        {
            size_t offset = _data.size();
            _data.resize(offset + sizeof(U));
            *(reinterpret_cast<U*>(&_data[offset])) = val;
        }

        void write(const std::string& val)
        {
            _writeString(val.size(), val.data());
        }

        void write(const char* val)
        {
            _writeString(strlen(val), val);
        }

    private:
        inline void _writeString(size_t length, const char* data)
        {
            write(length);
            size_t offset = _data.size();
            _data.resize(offset + length);
            strncpy(&_data[offset], data, length);
        }

        inline void _checkRead(size_t bytes)
        {
            if ((_offset + bytes) > _data.size()) {
                if (_offset >= _data.size()) {
                    throw std::runtime_error("read from empty buffer");
                } else {
                    throw std::runtime_error("read exceeds buffer size");
                }
            }
        }

        std::vector<char> _data;
        size_t _offset;
    };
}

#endif // __bulkio_messagebuffer_h
