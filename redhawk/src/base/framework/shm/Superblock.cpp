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

#include "Superblock.h"
#include "Block.h"
#include "ThreadState.h"
#include "offset_ptr.h"

#include <ossie/shm/MappedFile.h>
#include <ossie/BufferManager.h>

#include <stdexcept>
#include <cstring>
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <assert.h>

using namespace redhawk;
using redhawk::shm::Superblock;
using redhawk::shm::Block;
using redhawk::shm::ThreadState;

#define ALLOC_DEBUG 0
#if ALLOC_DEBUG > 0
#define LOG_ALLOC(x) std::cout << "+ " << ((x) - sizeof(Block)) << std::endl;
#define LOG_DEALLOC(x) std::cout << "- " << ((x) - sizeof(Block)) << std::endl;
#else
#define LOG_ALLOC(x)
#define LOG_DEALLOC(x)
#endif

struct Superblock::FreeBlock : public Block {
    FreeBlock(size_t offset, size_t bytes) :
        Block(offset, bytes),
        prev_free(0),
        next_free(0),
        prev_size(0),
        next_size(0)
    {
        markTail();
    }
    
    uint32_t prev_free;
    uint32_t next_free;
    uint32_t prev_size;
    uint32_t next_size;
};

Superblock::Superblock(const std::string& heap, size_t offset, size_t size) :
    _offset(offset),
    _size(size),
    _dataStart(MappedFile::PAGE_SIZE),
    _used(0),
    _first(0),
    _last(0)
{
    assert(heap.size() < 256);
    strcpy(_heapname, heap.c_str());

    uint32_t block_start = _dataStart / Block::BLOCK_SIZE;
    uint32_t block_count = size / Block::BLOCK_SIZE;
    FreeBlock* block = new (_data()) FreeBlock(block_start, block_count);
   _queueFree(block);
}

Superblock::~Superblock()
{
}

const char* Superblock::heap() const
{
    return _heapname;
}

size_t Superblock::offset() const
{
    return _offset;
}

size_t Superblock::size() const
{
    return _size;
}

size_t Superblock::used() const
{
    return _used;
}

void* Superblock::attach(size_t offset)
{
    Block* block = offset_ptr<Block>(this, offset * Block::BLOCK_SIZE);
    if (!block->valid()) {
        throw std::bad_alloc();
    }
    block->incref();
    return block->data();
}

void Superblock::deallocate(void* ptr)
{
    // The C standard says it's safe to call free() with a null pointer, so for
    // consistency, handle that case here
    if (!ptr) {
        return;
    }

    Block* block = Block::from_pointer(ptr);
    assert(block->valid());
    if (block->decref() == 0) {
        Superblock* superblock = block->getSuperblock();
        superblock->_deallocate(block);
    }
}

void Superblock::dump(std::ostream& stream) const
{
    scoped_lock lock(_lock);
    _dump(stream);
}

void Superblock::_dump(std::ostream& stream) const
{
    stream << "Free list:" << std::endl;
    int index = 0;
    for (const FreeBlock* bin = _offsetToBlock(_first); bin; bin = _offsetToBlock(bin->next_size)) {
        if (!bin->valid()) {
            stream << "Bad bin@" << bin << " [" << index << "]" << std::endl;
            break;
        }
        std::cout << "Bin " << bin->byteSize() << std::endl;
        for (const FreeBlock* free_block = bin; free_block; free_block = _offsetToBlock(free_block->next_free)) {
            stream << "block@" << free_block << ":" << std::endl;
            if (!free_block->valid()) {
                stream << "  INVALID" << std::endl;
                break;
            }
            stream << "  refcount: " << free_block->getRefcount() << std::endl;
            stream << "  offset:   " << free_block->offset() << std::endl;
            stream << "  prev:     " << free_block->prev_free << std::endl;
            stream << "  next:     " << free_block->next_free << std::endl;
            ++index;
        }
    }

    stream << std::endl
           << "All blocks:" << std::endl;
    size_t blocks = 0;
    for (const Block* block = _begin(); block != _end(); block = block->next()) {
        stream << "block@" << block << ":" << std::endl;
        if (!block->valid()) {
            stream << "  INVALID" << std::endl;
            break;
        } else {
            stream << "  prevfree: " << block->isPreviousFree() << std::endl;
            stream << "  refcount: " << block->getRefcount() << std::endl;
            stream << "  offset:   " << block->offset() << std::endl;
            stream << "  size:     " << block->byteSize() << std::endl;
            blocks++;
        }
    }
    stream << blocks << " block(s)" << std::endl;
}

char* Superblock::_data()
{
    char* ptr = reinterpret_cast<char*>(this);
    return ptr + _dataStart;
}

const char* Superblock::_data() const
{
    return const_cast<Superblock*>(this)->_data();
}

const Block* Superblock::_begin() const
{
    return reinterpret_cast<const Block*>(_data());
}

const Block* Superblock::_end() const
{
    return reinterpret_cast<const Block*>(_data() + _size);
}

Superblock::FreeBlock* Superblock::_offsetToBlock(uint32_t offset)
{
    if (offset) {
        return offset_ptr<FreeBlock>(this, offset * Block::BLOCK_SIZE);
    } else {
        return 0;
    }
}

const Superblock::FreeBlock* Superblock::_offsetToBlock(uint32_t offset) const
{
    if (offset) {
        return offset_ptr<FreeBlock>(this, offset * Block::BLOCK_SIZE);
    } else {
        return 0;
    }
}

void Superblock::_deallocate(Block* block)
{
    scoped_lock lock(_lock);
    assert(block->byteSize() >= sizeof(FreeBlock));
    LOG_DEALLOC(block->byteSize());

    _used -= block->byteSize();

#if ALLOC_DEBUG > 1
    std::cout << "Returning block@" << block << std::endl;
    std::cout << "  offset: " << block->offset() << std::endl;
    std::cout << "  size:   " << block->byteSize() << std::endl;
#endif

    // Try to recombine adjacent memory
    _coalesceNext(block);
    if (block->isPreviousFree()) {
        // Previous block is free, combine and re-queue the "new" block
        block = _coalescePrevious(block);
    } else {
        // Mark this block as free to allow it to be right-coalesced in the
        // future
        block->markFree();
    }

    // Write the tail so that the next block can determine the size of this one
    // (after all coalescing is complete)
    block->markTail();

    _queueFree(block);

#if ALLOC_DEBUG > 1
    _dump(std::cout);
#endif
}

void Superblock::_queueFree(Block* block)
{
    assert(block->isFree());

    FreeBlock* free_block = reinterpret_cast<FreeBlock*>(block);
    if (!_first) {
        // No free blocks exist
        free_block->prev_free = 0;
        free_block->next_free = 0;
        free_block->prev_size = 0;
        free_block->next_size = 0;
        _first = _last = free_block->offset();
    } else {
        for (FreeBlock* bin = _offsetToBlock(_first); bin; bin = _offsetToBlock(bin->next_size)) {
            assert(bin->valid());
            assert(bin->isFree());
            if (bin->size() == free_block->size()) {
                // Insert into existing bin
                free_block->prev_free = bin->offset();
                free_block->next_free = bin->next_free;
                if (free_block->next_free) {
                    FreeBlock* next_free = _offsetToBlock(free_block->next_free);
                    next_free->prev_free = free_block->offset();
                }
                free_block->prev_size = 0;
                free_block->next_size = 0;
                bin->next_free = free_block->offset();
                return;
            } else if (bin->size() > free_block->size()) {
                // The current bin is larger, insert block as a new bin before
                // the current one
                free_block->prev_free = 0;
                free_block->next_free = 0;
                free_block->next_size = bin->offset();
                FreeBlock* prev_size = _offsetToBlock(bin->prev_size);
                free_block->prev_size = bin->prev_size;
                if (prev_size) {
                    prev_size->next_size = free_block->offset();
                }
                bin->prev_size = free_block->offset();
                if (bin->offset() == _first) {
                    _first = free_block->offset();
                }
                return;
            }
        }
        // No matching bin, and the block is larger than all existing bins
        FreeBlock* prev_size = _offsetToBlock(_last);
        _last = free_block->offset();
        prev_size->next_size = _last;
        free_block->prev_free = 0;
        free_block->next_free = 0;
        free_block->prev_size = prev_size->offset();
        free_block->next_size = 0;
    }
}

void* Superblock::allocate(ThreadState* thread, size_t bytes)
{
    scoped_lock lock(_lock, false);
    if (lock.trylock()) {
        thread->contention = std::max(thread->contention - 1, 0);
    } else {
        thread->contention++;
        lock.lock();
    }

    // Add overhead for block metadata, making sure that the total byte size
    // is enough for a free block, then round up to the nearest block size to
    // preserve alignment on all architectures; otherwise, atomic operations
    // may cause a fatal bus error.
    bytes = std::max(bytes + sizeof(Block), sizeof(FreeBlock));
    size_t blocks = Block::bytes_to_blocks(bytes);

    LOG_ALLOC(bytes);

    FreeBlock* block = _findAvailable(blocks);
    if (!block) {
        // No free blocks, give up
        return 0;
    }
    assert(block->valid());
    assert(block->isFree());
    assert(block->size() >= blocks);

#if ALLOC_DEBUG > 1
    std::cout << "Allocating from block@" << block << std::endl;
    std::cout << "  offset: " << block->offset() << std::endl;
    std::cout << "  size:   " << block->byteSize() << std::endl;
    std::cout << "  prev:   " << block->prev_free << std::endl;
    std::cout << "  next:   " << block->next_free << std::endl;
#endif

    // Remove the block from the free list; if there's a remainder, it will be
    // be of a different size, so it gets re-inserted later
    _removeFreeBlock(block);

    // If it's not taking the entire block, assign the remainder to a new block
    // (taking into account that there has to be enough left over to store the
    // extra pointers)
    size_t remainder = block->size() - blocks;
    if ((remainder * Block::BLOCK_SIZE) > sizeof(FreeBlock)) {
        FreeBlock* next_block = reinterpret_cast<FreeBlock*>(block->split(blocks));
        _queueFree(next_block);

#if ALLOC_DEBUG > 1
        std::cout << "Used block@" << block << std::endl;
        std::cout << "  offset: " << block->offset() << std::endl;
        std::cout << "  size:   " << block->byteSize() << std::endl;

        std::cout << "Remain block@" << next_block << std::endl;
        std::cout << "  offset: " << next_block->offset() << std::endl;
        std::cout << "  size:   " << next_block->byteSize() << std::endl;
#endif
        assert(next_block->valid());
    }

    // Mark the block as "allocated"
    block->markUsed();
    Block* next = block->next();
    if (next != _end()) {
        assert(next->valid());
        next->setPreviousUsed();
    }

#if ALLOC_DEBUG > 1
    _dump(std::cout);
#endif
    _used += block->byteSize();

    return block->data();
}

void Superblock::_removeFreeBlock(FreeBlock* block)
{
    FreeBlock* prev_block = _offsetToBlock(block->prev_free);
    FreeBlock* next_block = _offsetToBlock(block->next_free);
    if (prev_block) {
        // Block is not the first in its bin
        prev_block->next_free = block->next_free;
        if (next_block) {
            next_block->prev_free = block->prev_free;
        }
    } else {
        FreeBlock* prev_size = _offsetToBlock(block->prev_size);
        FreeBlock* next_size = _offsetToBlock(block->next_size);
        if (next_block) {
            // There's another block in this bin, make it the head of the bin
            next_block->prev_free = 0;
            next_block->prev_size = block->prev_size;
            next_block->next_size = block->next_size;
            if (prev_size) {
                prev_size->next_size = block->next_free;
            } else {
                _first = block->next_free;
            }
            if (next_size) {
                next_size->prev_size = block->next_free;
            } else {
                _last = block->next_free;
            }
        } else {
            // Removing this bin entirely
            if (prev_size) {
                prev_size->next_size = block->next_size;
            } else {
                _first = block->next_size;
            }
            if (next_size) {
                next_size->prev_size = block->prev_size;
            } else {
                _last = block->prev_size;
            }
        }
    }
}

Superblock::FreeBlock* Superblock::_findAvailable(size_t blocks)
{
    for (FreeBlock* bin = _offsetToBlock(_first); bin; bin = _offsetToBlock(bin->next_size)) {
        assert(bin->valid());
        assert(bin->isFree());
        if (bin->size() >= blocks) {
            if (bin->next_free) {
                // Take the second entry in the bin to avoid having to alter
                // the bin list
                FreeBlock* next_free = _offsetToBlock(bin->next_free);
                assert(bin->size() == next_free->size());
                return next_free;
            } else {
                // This is the only block in the bin
                return bin;
            }
        }
    }
    return 0;
}

void Superblock::_coalesceNext(Block* block)
{
    Block* next = block->next();
    if (next == _end()) {
        return;
    }

    assert(next->valid());
    if (next->isFree()) {
        // Next block is free, append it to this block
        _removeFreeBlock(reinterpret_cast<FreeBlock*>(next));
        block->join(next);
    } else {
        // Next block is in use, just inform it that this block is free so that
        // it can coalesce this block when it gets freed
        next->setPreviousFree();
    }
}

Block* Superblock::_coalescePrevious(Block* block)
{
    FreeBlock* prev = reinterpret_cast<FreeBlock*>(block->prev()); 
    assert(prev->valid());
    assert(prev->isFree());

    // Pull the block off of the free list; its size is going to change, so
    // its position will need to be adjusted
    _removeFreeBlock(prev);
    prev->join(block);

    // Return the "new" block
    return prev;
}
