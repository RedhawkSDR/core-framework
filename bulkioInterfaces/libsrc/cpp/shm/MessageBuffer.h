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
            write(val.size());
            size_t offset = _data.size();
            _data.resize(offset + val.size());
            strncpy(&_data[offset], val.data(), val.size());
        }

    private:
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
