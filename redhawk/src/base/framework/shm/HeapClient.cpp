#include <ossie/shm/HeapClient.h>
#include <ossie/shm/Heap.h>

#include "Superblock.h"

using namespace redhawk::shm;

HeapClient::HeapClient()
{
}

HeapClient::~HeapClient()
{
    detach();
}

void* HeapClient::fetch(const MemoryRef& ref)
{
    SuperblockFile* file = _getSuperblockFile(ref.heap);
    Superblock* superblock = file->getSuperblock(ref.superblock);
    return superblock->attach(ref.offset);
}

void HeapClient::deallocate(void* ptr)
{
    Superblock::deallocate(ptr);
}

void HeapClient::detach()
{
    for (FileMap::iterator file = _files.begin(); file != _files.end(); ++file) {
        file->second->close();
        delete file->second;
    }
    _files.clear();
}

SuperblockFile* HeapClient::_getSuperblockFile(const std::string& name)
{
    FileMap::iterator existing = _files.find(name);
    if (existing != _files.end()) {
        return existing->second;
    }

    SuperblockFile* file = new SuperblockFile(name);
    try {
        file->open();
    } catch (const std::exception& exc) {
        delete file;
        throw std::invalid_argument("cannot open superblock file");
    }
    _files[name] = file;
    return file;
}
