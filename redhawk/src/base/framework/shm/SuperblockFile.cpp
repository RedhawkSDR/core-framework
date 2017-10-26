#include <ossie/shm/SuperblockFile.h>

#include <stdexcept>

#include "Superblock.h"
#include "atomic_counter.h"

using namespace redhawk::shm;

struct SuperblockFile::Header {
    atomic_counter<int> refcount;
};

SuperblockFile::SuperblockFile(const std::string& name) :
    _file(name),
    _header(0)
{
}

SuperblockFile::~SuperblockFile()
{
    close();
}

const std::string& SuperblockFile::name() const
{
    return _file.name();
}

void SuperblockFile::create()
{
    if (_header) {
        throw std::logic_error("file is already open");
    }

    _file.create();

    // Use a page to create the header
    _file.resize(MappedFile::PAGE_SIZE);
    void* base = _file.map(MappedFile::PAGE_SIZE, MappedFile::READWRITE);
    _header = new (base) Header;
    _header->refcount = 1;
}

void SuperblockFile::open()
{
    if (_header) {
        throw std::logic_error("file is already open");
    }

    _file.open();

    void* base = _file.map(MappedFile::PAGE_SIZE, MappedFile::READWRITE);
    _header = reinterpret_cast<Header*>(base);
    _header->refcount.increment();
}

void SuperblockFile::close()
{
    if (!_header) {
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
