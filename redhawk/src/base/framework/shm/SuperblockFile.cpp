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

#include <ossie/shm/SuperblockFile.h>

#include <stdexcept>
#include <fstream>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include <ossie/shm/System.h>

#include "Superblock.h"
#include "atomic_counter.h"

using namespace redhawk::shm;

struct SuperblockFile::Header {
    // Magic number to identify superblock files. This should never change.
    typedef uint32_t magic_type;
    static const magic_type SUPERBLOCK_MAGIC = 0xACEB70CC;

    // ABI version of superblock file. If the layout of the header or the
    // Superblock class changes, changes, this version must be incremented.
    typedef uint32_t version_type;
    static const version_type SUPERBLOCK_VERSION = 1;

    Header() :
        magic(SUPERBLOCK_MAGIC),
        version(SUPERBLOCK_VERSION),
        refcount(1),
        creator(getpid())
    {
    }

    const magic_type magic;
    const version_type version;
    atomic_counter<int32_t> refcount;
    const pid_t creator;
};

SuperblockFile::SuperblockFile(const std::string& name) :
    _file(name),
    _attached(false),
    _header(0)
{
}

SuperblockFile::~SuperblockFile()
{
    close();
}

bool SuperblockFile::IsSuperblockFile(const std::string& name)
{
    std::string path = redhawk::shm::getSystemPath();
    path += "/" + name;
    std::ifstream file(path.c_str());
    if (!file) {
        return false;
    }
    Header::magic_type magic = 0;
    if (!file.read(reinterpret_cast<char *>(&magic), sizeof(magic))) {
        return false;
    }

    return magic == Header::SUPERBLOCK_MAGIC;
}

const std::string& SuperblockFile::name() const
{
    return _file.name();
}

MappedFile& SuperblockFile::file()
{
    return _file;
}

pid_t SuperblockFile::creator() const
{
    if (!_header) {
        return 0;
    }
    return _header->creator;
}

int SuperblockFile::refcount() const
{
    if (!_header) {
        return -1;
    }
    return _header->refcount;
}

bool SuperblockFile::isOrphaned() const
{
    if (!_header) {
        throw std::logic_error("not attached");
    }

    // Check that the creator PID is still alive
    return (kill(_header->creator, 0) != 0);
}

SuperblockFile::Statistics SuperblockFile::getStatistics()
{
    Statistics stats;
    stats.size = 0;
    stats.used = 0;
    stats.superblocks = 0;
    stats.unused = 0;

    // First superblock starts at next page after the header
    size_t offset = MappedFile::PAGE_SIZE;
    const size_t end = _file.size();
    while (offset < end) {
        // Map just the header of the superblock; no calls here need to acquire
        // its lock, so this prevents accidental modifications
        void* base = _file.map(MappedFile::PAGE_SIZE, MappedFile::READONLY, offset);
        const Superblock* superblock = reinterpret_cast<const Superblock*>(base);

        // Extra safety check; since we're walking through the superblocks, the
        // offsets should always be correct, but just in case...
        bool valid = (superblock->offset() == offset);
        if (valid) {
            stats.size += superblock->size();
            size_t used = superblock->used();
            stats.used += used;
            if (!used) {
                stats.unused++;
            }
            stats.superblocks++;
            // Account for the superblock overhead
            offset += MappedFile::PAGE_SIZE + superblock->size();
        }
        // Don't forget to unmap--it doesn't happen automatically!
        _file.unmap(base, MappedFile::PAGE_SIZE);

        if (!valid) {
            break;
        }
    }

    return stats;
}

void SuperblockFile::create()
{
    if (_header) {
        throw std::logic_error("file is already open");
    }

    _file.create();

    // Use a page to create the header
    try {
        _file.resize(MappedFile::PAGE_SIZE);
    } catch (const std::exception&) {
        // Something is terribly wrong, probably out of memory; remove the file
        // and relay the exception
        _file.unlink();
        throw;
    }
    void* base = _file.map(MappedFile::PAGE_SIZE, MappedFile::READWRITE);
    _header = new (base) Header;
    _attached = true;
}

void SuperblockFile::open(bool attach)
{
    if (_header) {
        throw std::logic_error("file is already open");
    }

    _file.open();

    // Check for a heap that was created on a full tmpfs--the file exists but
    // has no allocated memory
    if (_file.size() < MappedFile::PAGE_SIZE) {
        throw std::runtime_error("invalid superblock file (no header)");
    }

    // Map the file and overlay the header structure over it, checking the
    // magic number to make sure it's really a superblock file
    void* base = _file.map(MappedFile::PAGE_SIZE, MappedFile::READWRITE);
    Header* header = reinterpret_cast<Header*>(base);
    if (header->magic != Header::SUPERBLOCK_MAGIC) {
        throw std::runtime_error("invalid superblock file (magic number does not match)");
    } else if (header->version != Header::SUPERBLOCK_VERSION) {
        throw std::runtime_error("incompatible superblock file (version mismatch)");
    }
 
    // Store a reference the header and attach, so that we clean up on close
    _header = header;
    if (attach) {
        _header->refcount.increment();
        _attached = true;
    }
}

void SuperblockFile::close()
{
    if (!_header) {
        return;
    }

    _detach();

    // Unmap the header to avoid keeping the file alive
    _file.unmap(_header, MappedFile::PAGE_SIZE);

    _file.close();

    _header = 0;
}

Superblock* SuperblockFile::getSuperblock(size_t offset)
{
    // Check if the superblock is already mapped
    SuperblockMap::iterator existing = _superblocks.find(offset);
    if (existing != _superblocks.end()) {
        return existing->second;
    }

    return _mapSuperblock(offset);
}

Superblock* SuperblockFile::createSuperblock(size_t bytes)
{
    // Allocate 1 page for the header, plus the superblock memory
    size_t current_offset = _file.size();
    size_t total_size = MappedFile::PAGE_SIZE + bytes;
    _file.resize(current_offset + total_size);

    void* base = _file.map(total_size, MappedFile::READWRITE, current_offset);
    Superblock* superblock = new (base) Superblock(_file.name(), current_offset, bytes);
    _superblocks[superblock->offset()] = superblock;
    return superblock;
}

void SuperblockFile::_detach()
{
    if (!_attached) {
        return;
    }

    if (_header->refcount.decrement() == 0) {
        try {
            _file.unlink();
        } catch (const std::exception&) {
            // Ignore exception--someone may have already forcibly removed the
            // file, but that's not a problem here.
        }
    }
}

Superblock* SuperblockFile::_mapSuperblock(size_t offset)
{
    // Map just the superblock's header to get the complete size
    void* base = _file.map(MappedFile::PAGE_SIZE, MappedFile::READWRITE, offset);
    Superblock* superblock = reinterpret_cast<Superblock*>(base);
    if (superblock->offset() != offset) {
        throw std::invalid_argument("offset is not a valid superblock");
    }
    size_t superblock_size = superblock->size();

    // Remap to get the full superblock size
    base = _file.remap(base, MappedFile::PAGE_SIZE, MappedFile::PAGE_SIZE + superblock_size);
    superblock = reinterpret_cast<Superblock*>(base);

    // Store mapping
    _superblocks[superblock->offset()] = superblock;
    return superblock;
}
