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

#ifndef __bulkio_ipcfifo_h
#define __bulkio_ipcfifo_h

#include <string>

namespace bulkio {

    class TempFifo {
    public:
        TempFifo();
        ~TempFifo();

        const std::string& filename() const;

        void unlink();

    private:
        std::string _makeUniqueName();

        const std::string _filename;
    };

    class Pipe {
    public:
        Pipe();
        ~Pipe();

        void open(const std::string& filename, int flags);
        void close();

        void setFlags(int flags);
        void clearFlags(int flags);

        bool poll(int events, int timeout);

        size_t read(void* buffer, size_t bytes);
        void write(const void* data, size_t bytes);

    private:
        int _fd;

        // Maximum size to read/write on a single pass
        size_t _blockSize;
    };

    class FifoEndpoint {
    public:
        FifoEndpoint();
        ~FifoEndpoint();

        const std::string& name() const;

        void connect(const std::string& name);
        void sync(int timeout);

        size_t read(void* buffer, size_t bytes);
        void write(const void* data, size_t bytes);

        void disconnect();
        void unlink();

    private:
        TempFifo _fifo;
        Pipe _read;
        Pipe _write;
    };
}

#endif // __bulkio_ipcfifo_h
