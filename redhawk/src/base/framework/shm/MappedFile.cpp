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

#include <ossie/shm/MappedFile.h>

#include <stdexcept>
#include <cstring>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

using namespace redhawk::shm;

static std::string error_string()
{
    return strerror(errno);
}

const size_t MappedFile::PAGE_SIZE = sysconf(_SC_PAGESIZE);

MappedFile::MappedFile(const std::string& name) :
    _name(name),
    _fd(-1)
{
}

void MappedFile::create()
{
    if (_fd >= 0) {
        throw std::runtime_error("shm file is already open");
    }

    _fd = shm_open(_name.c_str(), O_RDWR|O_CREAT|O_TRUNC, 0666);
    if (_fd < 0) {
        throw std::runtime_error("shm_open: " + error_string());
    }
}

void MappedFile::open()
{
    if (_fd >= 0) {
        throw std::runtime_error("shm file is already open");
    }

    _fd = shm_open(_name.c_str(), O_RDWR, 0);
    if (_fd < 0) {
        throw std::runtime_error("shm_open: " + error_string());
    }
}

MappedFile::~MappedFile()
{
    close();
}

const std::string& MappedFile::name() const
{
    return _name;
}

size_t MappedFile::size() const
{
    struct stat statbuf;
    if (fstat(_fd, &statbuf)) {
        throw std::runtime_error("fstat: " + error_string());
    }
    return statbuf.st_size;
}

void MappedFile::resize(size_t bytes)
{
    size_t current_size = size();
    if (bytes <= current_size) {
        return;
    }
    int status = posix_fallocate(_fd, current_size, bytes - current_size);
    if (status == 0) {
        return;
    } else if (status == ENOSPC) {
        throw std::bad_alloc();
    } else {
        throw std::runtime_error("fallocate failed");
    }
}

void* MappedFile::map(size_t bytes, mode_e mode, off_t offset)
{
    int prot = PROT_READ;
    if (mode == READWRITE) {
        prot |= PROT_WRITE;
    }

    void* addr = mmap(0, bytes, prot, MAP_SHARED, _fd, offset);
    if (addr == MAP_FAILED) {
        throw std::runtime_error("mmap: " + error_string());
    }
    return addr;
}

void* MappedFile::remap(void* oldAddr, size_t oldSize, size_t newSize)
{
    int flags = MREMAP_MAYMOVE;
    void* addr = mremap(oldAddr, oldSize, newSize, flags);
    if (addr == MAP_FAILED) {
        throw std::runtime_error("mremap: " + error_string());
    }
    return addr;
}

void MappedFile::unmap(void* ptr, size_t bytes)
{
    if (munmap(ptr, bytes)) {
        throw std::runtime_error("munmap: " + error_string());
    }
}

void MappedFile::close()
{
    if (_fd >= 0) {
        ::close(_fd);
        _fd = -1;
    }
}

void MappedFile::unlink()
{
    if (shm_unlink(_name.c_str())) {
        throw std::runtime_error("unlink: " + error_string());
    }
}
