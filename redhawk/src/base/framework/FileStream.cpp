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

#include "ossie/FileStream.h"

#include <algorithm>
#include <iostream>
#include <cassert>
#include <cstring>

using std::size_t;

File_buffer::File_buffer(CF::File_ptr fptr, size_t buff_sz, size_t put_back) :
    fptr_(CF::File::_duplicate(fptr)),
    put_back_(std::max(put_back, size_t(1))),
    buffer_(std::max(buff_sz, put_back_) + put_back_)
{
        char *end = &buffer_.front() + buffer_.size();
        setg(end, end, end);
}

std::streambuf::int_type File_buffer::underflow()
{
    try {
        if (gptr() < egptr()) { //buffer not exhausted
            return traits_type::to_int_type(*gptr());
        }

        char *base = &buffer_.front();
        char *start = base;

        if (eback() == base) { // true when this isn't the first fill
            // Make arrangements for putback characters
            std::memmove(base, egptr() - put_back_, put_back_);
            start += put_back_;
        }

        // start is now the start of the buffer, proper.
        // Read from fptr_ in to the provided buffer
        CF::OctetSequence_var data;
        if (!CORBA::is_nil(fptr_) && (!fptr_->_non_existent())) {
            fptr_->read(data, buffer_.size() - (start - base));
        } else {
            return traits_type::eof();
        }

        if (data->length() == 0) {
            return traits_type::eof();
        }

        memcpy(start, (const char*)data->get_buffer(), data->length());

        // Set buffer pointers
        setg(base, start, start + data->length());

        return traits_type::to_int_type(*gptr());
    } catch (...) {
        return traits_type::eof();
    }
}

void File_buffer::close()
{
    try {
        if (!CORBA::is_nil(fptr_) && (!fptr_->_non_existent())) {
            fptr_->close();
        }

    } catch (CORBA::SystemException& se) {
        throw std::ios_base::failure("CORBA SystemException while closing file");
    } catch (...) {
        throw std::ios_base::failure("Unexpected error while closing file");
    }
}

File_stream::File_stream(CF::FileSystem_ptr fsysptr, const char* path) :
    std::ios(0),
    needsClose(true) 
{
    try {
        sb = new File_buffer((CF::File_var)fsysptr->open(path, true));
        this->init(sb);
    } catch (const CF::InvalidFileName& ifn) {
        throw std::ios_base::failure(std::string(ifn.msg));
    } catch (const CF::FileException& fe) {
        throw std::ios_base::failure(std::string(fe.msg));
    } catch( ... ) {
        throw std::ios_base::failure("exception while opening file");
    }
}

File_stream::File_stream(CF::File_ptr fptr) :
    std::ios(0),
    needsClose(false) 
{
    try {
        sb = new File_buffer(fptr);
        this->init(sb);
    } catch( ... ) {
        throw std::ios_base::failure("exception while opening file");
    }
}

void File_stream::close()
{
    if ((needsClose) && (sb != 0)) {
        sb->close();
    }
    delete sb;
    sb = 0;
}

File_stream::~File_stream() {
    try {
        this->close();
    } catch (std::ios_base::failure& ex) {
        // pass
    }
    if (sb != 0) {
        delete sb;
        sb = 0;
    }
}

