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

#include "FifoIPC.h"

#include <iostream>
#include <cstdio>
#include <stdexcept>

#include <boost/thread.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

namespace bulkio {

    namespace {
        static std::string getErrorMessage()
        {
            char temp[1024];
            char* msg = strerror_r(errno, temp, sizeof(temp));
            return msg;
        }
    }

    TempFifo::TempFifo() :
        _filename(_makeUniqueName())
    {
        // Create the FIFO on the local filesystem 
        if (mkfifo(_filename.c_str(), 0666)) {
            throw std::runtime_error("mkfifo " + _filename + ": " + getErrorMessage());
        }
    }

    TempFifo::~TempFifo()
    {
        // Always unlink the FIFO; while it may have been explicitly unlinked,
        // for exception cleanup it's best to be sure.
        unlink();
    }

    const std::string& TempFifo::filename() const
    {
        return _filename;
    }

    void TempFifo::unlink()
    {
        ::unlink(_filename.c_str());
    }

    std::string TempFifo::_makeUniqueName()
    {
        // Generate a unique name for the FIFO using the process ID and the
        // address of the this object (as a hex number). There can only be one
        // object at a given address at a time in a single process, so there
        // should be no name collisions.
        std::ostringstream oss;
        oss << "/tmp/fifo-" << getpid() << '-' << std::hex << (size_t) this;
        return oss.str();
    }


    Pipe::Pipe() :
        _fd(-1),
        _blockSize(32768)
    {
        // Setting a default block size of 32K seems to give the best performance
    }

    Pipe::~Pipe()
    {
        close();
    }

    void Pipe::open(const std::string& filename, int flags)
    {
        _fd = ::open(filename.c_str(), flags);
        if (_fd < 0) {
            throw std::runtime_error("open " + filename + ": " + getErrorMessage());
        }
    }

    void Pipe::setFlags(int flags)
    {
        fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL) | flags);
    }

    void Pipe::clearFlags(int flags)
    {
        fcntl(_fd, F_SETFL, fcntl(_fd, F_GETFL) & ~flags);
    }

    void Pipe::close()
    {
        if (_fd >= 0) {
            ::close(_fd);
            _fd = -1;
        }
    }

    bool Pipe::poll(int events, int timeout)
    {
        struct pollfd pfd;
        pfd.fd = _fd;
        pfd.events = events;

        return ::poll(&pfd, 1, timeout) == 1;
    }

    size_t Pipe::read(void* buffer, size_t bytes)
    {
        char* ptr = static_cast<char*>(buffer);
        size_t remain = bytes;
        while (remain > 0) {
            boost::this_thread::interruption_point();
            size_t todo = std::min(remain, _blockSize);
            ssize_t pass = ::read(_fd, ptr, todo);
            if (pass < 0) {
                throw std::runtime_error("read failed: " + getErrorMessage());
            }
            remain -= pass;
            ptr += pass;
        }
        return bytes;
    }

    void Pipe::write(const void* data, size_t bytes)
    {
        const char* ptr = static_cast<const char*>(data);
        size_t remain = bytes;
        while (remain > 0) {
            boost::this_thread::interruption_point();
            size_t todo = std::min(remain, _blockSize);
            ssize_t pass = ::write(_fd, ptr, todo);
            if (pass <= 0) {
                if (pass < 0) {
                    throw std::runtime_error("write failed: " + getErrorMessage());
                }
                return;
            }
            if (((size_t) pass) < todo) {
                std::cerr << "only wrote " << pass << std::endl;
            }
            ptr += pass;
            remain -= pass;
        }
    }


    FifoEndpoint::FifoEndpoint() :
        _fifo(),
        _read(),
        _write()
    {
        // For read mode, set non-blocking mode; this allows open() to return
        // immediately, even though the write side has not been opened yet.
        _read.open(_fifo.filename(), O_RDONLY|O_NONBLOCK);

        // Clear the non-blocking flag now, as future reads should be blocking.
        _read.clearFlags(O_NONBLOCK);
    }

    FifoEndpoint::~FifoEndpoint()
    {
    }

    const std::string& FifoEndpoint::name() const
    {
        return _fifo.filename();
    }

    void FifoEndpoint::connect(const std::string& name)
    {
        // Open the filename for write and Send a sync character to allow the
        // read end to resynchronize.
        _write.open(name, O_WRONLY);
        char token = 'w';
        write(&token, sizeof(token));
    }

    void FifoEndpoint::sync(int timeout)
    {
        if (!_read.poll(POLLIN, timeout)) {
            throw std::runtime_error("sync timed out");
        }

        // Read the sync character. Even though the non-blocking flag has been
        // cleared, read may return immediately with no data, if the write side
        // has not been opened yet. Retrying the read until it returns data
        // ensures that the FIFO is fully connected.
        char token;
        if (read(&token, sizeof(token)) != sizeof(token)) {
            throw std::runtime_error("read failed");
        }

        if (token != 'w') {
            throw std::runtime_error("open failed");
        }

        // Once both ends have been established, it's safe to remove the FIFO
        // from the file system.
        _fifo.unlink();
    }

    size_t FifoEndpoint::read(void* buffer, size_t size)
    {
        return _read.read(buffer, size);
    }

    void FifoEndpoint::write(const void* data, size_t bytes)
    {
        _write.write(data, bytes);
    }

    void FifoEndpoint::disconnect()
    {
        _write.close();
    }

}
