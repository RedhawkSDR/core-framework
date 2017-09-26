#ifndef SHMBLOCK_HH
#define SHMBLOCK_HH

#include <inttypes.h>
#include <assert.h>

#include "atomic_counter.h"
#include "offset_ptr.h"

namespace redhawk {
    class ShmArena;

    struct ShmBlock {
        // NB: Last bit of magic number is reserved for the "previous block in use"
        //     flag
        static const uint32_t BLOCK_MAGIC = 0xbaddc0de;

        // Quantum for block allocation; offset and size are in terms of this block
        // size (i.e., not bytes)
        static const size_t BLOCK_SIZE = 16;

    ShmBlock(uint32_t offset, uint32_t blocks) :
        magic(ShmBlock::BLOCK_MAGIC),
            refcount(-1),
            _offset(offset),
            _size(blocks)
        {
        }

        ~ShmBlock()
        {
            // Invert the bit pattern for the magic number to make it more obvious
            // that a block has been destroyed when debugging
            magic = ~BLOCK_MAGIC;
            _size = -1;
        }

        uint32_t offset() const
        {
            return _offset;
        }

        size_t byte_offset() const
        {
            return _offset * BLOCK_SIZE;
        }

        uint32_t size() const
        {
            return _size;
        }

        size_t byte_size() const
        {
            return _size * BLOCK_SIZE;
        }

        ShmBlock* split(uint32_t blocks)
        {
            // Only free blocks can be split
            assert(is_free());

            // Adjust the size of the this block, then use the next() method to get
            // a pointer to where the new block starts; note that it is not a valid
            // block yet, until the in-place constructor is called
            size_t remainder = _size - blocks;
            assert(remainder < _size);
            _size = blocks;
            void* ptr = next();

            // Initialize the block, which must also be free, so mark the tail in
            // case it gets left-coalesced
            ShmBlock* block = new (ptr) ShmBlock(_offset + blocks, remainder);
            block->mark_tail();
            return block;
        }

        void join(ShmBlock* block)
        {
            assert(block == next());
            _size += block->_size;

            // Invalidate the discarded block
            block->~ShmBlock();
        }

        void incref()
        {
            refcount.increment();
        }

        size_t decref()
        {
            return refcount.decrement();
        }

        void* data()
        {
            return (this + 1);
        }

        ShmArena* arena()
        {
            return offset_ptr<ShmArena>(this, -(ptrdiff_t)byte_offset());
        }

        bool previous_free() const
        {
            return magic & FLAG_PREV;
        }

        void set_previous_free()
        {
            magic |= FLAG_PREV;
        }

        void set_previous_used()
        {
            magic &= ~FLAG_PREV;
        }

        uint32_t* prev_size()
        {
            return reinterpret_cast<uint32_t*>(this) - 1;
        }

        void mark_tail()
        {
            uint32_t* tail = next()->prev_size();
            *tail = _size;
        }

        ShmBlock* prev()
        {
            if (previous_free()) {
                ptrdiff_t diff = *prev_size() * BLOCK_SIZE;
                return offset_ptr<ShmBlock>(this, -diff);
            } else {
                // Previous block is in use
                return 0;
            }
        }

        ShmBlock* next()
        {
            return offset_ptr<ShmBlock>(this, byte_size());
        }

        const ShmBlock* next() const
        {
            return offset_ptr<ShmBlock>(this, byte_size());
        }

        static ShmBlock* from_pointer(void* ptr)
        {
            return reinterpret_cast<ShmBlock*>(ptr) - 1;
        }

        bool valid() const
        {
            return (magic & ~FLAG_PREV) == BLOCK_MAGIC;
        }

        bool is_free() const
        {
            return (refcount < 0);
        }

        void mark_free()
        {
            refcount = -1;
        }

        void mark_used()
        {
            refcount = 1;
        }

        static const uint32_t FLAG_PREV = 1;

        uint32_t magic;
        atomic_counter<int32_t> refcount;

    private:
        uint32_t _offset;
        uint32_t _size;
    };
}

#endif // SHMBLOCK_HH
