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

#include "ossie/CF/cf.h"
#include <streambuf>
#include <vector>
#include <istream>
#include <iostream>

#ifndef FILEBUFFER_H
#define FILEBUFFER_H

class File_buffer : public std::streambuf
{
    public:
        explicit File_buffer(CF::File_ptr fptr, std::size_t buff_sz = 32768, std::size_t put_back = 8);

        virtual void close();

    private:
        // overrides base class underflow()
        int_type underflow();

    private:
        CF::File_var fptr_;
        const std::size_t put_back_;
        std::vector<char> buffer_;
};
#endif

#ifndef FILESTREAM_H
#define FILESTREAM_H
class File_stream : public std::istream
{
    public:
        /*
         * Open a stream given a SCA FileSystem and a path on the file system, this is the preferred constructor.
         *
         * Opening a stream using this constructor will ensure that the SCA file get's closed automatically
         * when the file stream is destroyed.
         */
        File_stream(CF::FileSystem_ptr fsysptr, const char* path);

        /*
         * Open a stream given a SCA File.
         *
         * Note: the caller is responsible for closing the provided file. 
         */
        explicit File_stream(CF::File_ptr fptr);

        virtual ~File_stream();

        virtual void close();
        
    private:
        bool needsClose;
        File_buffer* sb;
};
#endif
