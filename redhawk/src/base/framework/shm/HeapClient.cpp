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
