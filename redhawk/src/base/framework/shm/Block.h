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

#ifndef REDHAWK_SHM_BLOCK_H
#define REDHAWK_SHM_BLOCK_H

#include <inttypes.h>
#include <assert.h>

#include "atomic_counter.h"
#include "offset_ptr.h"

namespace redhawk {

    namespace shm {

        class Superblock;

        struct Block {
            // NB: Least significant bit of magic number is reserved for the
            //     "previous block in use" flag
            typedef uint32_t magic_type;
            static const magic_type BLOCK_MAGIC = 0xbaddc0de;

            // Quantum for block allocation; offset and size are in terms of
            // this block size (i.e., not bytes)
            static const size_t BLOCK_SIZE = 16;

            typedef uint32_t blocksize_type;

            Block(blocksize_type offset, blocksize_type blocks) :
                _magic(Block::BLOCK_MAGIC),
                _refcount(-1),
                _offset(offset),
                _size(blocks)
            {
            }

            ~Block()
            {
                // Invert the bit pattern for the magic number to make it more
                // obvious that a block has been destroyed when debugging
                _magic = ~BLOCK_MAGIC;
                _size = -1;
            }

            blocksize_type offset() const
            {
                return _offset;
            }

            size_t byteOffset() const
            {
                return _offset * BLOCK_SIZE;
            }

            blocksize_type size() const
            {
                return _size;
            }

            size_t byteSize() const
            {
                return _size * BLOCK_SIZE;
            }

            Block* split(blocksize_type blocks)
            {
                // Only free blocks can be split
                assert(isFree());

                // Adjust the size of the this block, then use the next() method to get
                // a pointer to where the new block starts; note that it is not a valid
                // block yet, until the in-place constructor is called
                size_t remainder = _size - blocks;
                assert(remainder < _size);
                _size = blocks;
                void* ptr = next();

                // Initialize the block, which must also be free, so mark the tail in
                // case it gets left-coalesced
                Block* block = new (ptr) Block(_offset + blocks, remainder);
                block->markTail();
                return block;
            }

            void join(Block* block)
            {
                assert(block == next());
                _size += block->_size;

                // Invalidate the discarded block
                block->~Block();
            }

            void incref()
            {
                _refcount.increment();
            }

            size_t decref()
            {
                return _refcount.decrement();
            }

            void* data()
            {
                return (this + 1);
            }

            Superblock* getSuperblock()
            {
                if (_offset == 0) {
                    return 0;
                }
                return offset_ptr<Superblock>(this, -(ptrdiff_t)byteOffset());
            }

            bool isPreviousFree() const
            {
                return _magic & FLAG_PREV;
            }

            void setPreviousFree()
            {
                _magic |= FLAG_PREV;
            }

            void setPreviousUsed()
            {
                _magic &= ~FLAG_PREV;
            }

            void markTail()
            {
                blocksize_type* tail = next()->_getPreviousSize();
                *tail = _size;
            }

            Block* prev()
            {
                if (isPreviousFree()) {
                    ptrdiff_t diff = *_getPreviousSize() * BLOCK_SIZE;
                    return offset_ptr<Block>(this, -diff);
                } else {
                    // Previous block is in use
                    return 0;
                }
            }

            Block* next()
            {
                return offset_ptr<Block>(this, byteSize());
            }

            const Block* next() const
            {
                return offset_ptr<Block>(this, byteSize());
            }

            static Block* from_pointer(void* ptr)
            {
                return reinterpret_cast<Block*>(ptr) - 1;
            }

            static size_t bytes_to_blocks(size_t bytes)
            {
                return (bytes + sizeof(Block) + Block::BLOCK_SIZE - 1) / Block::BLOCK_SIZE;
            }

            bool valid() const
            {
                return (_magic & ~FLAG_PREV) == BLOCK_MAGIC;
            }

            bool isFree() const
            {
                return (_refcount < 0);
            }

            void markFree()
            {
                _refcount = -1;
            }

            void markUsed()
            {
                _refcount = 1;
            }

            int getRefcount() const
            {
                return _refcount;
            }

        private:
            static const magic_type FLAG_PREV = 1;

            blocksize_type* _getPreviousSize()
            {
                return reinterpret_cast<blocksize_type*>(this) - 1;
            }

            magic_type _magic;
            atomic_counter<int32_t> _refcount;
            blocksize_type _offset;
            blocksize_type _size;
        };
    }
}

#endif // REDHAWK_SHM_BLOCK_H
