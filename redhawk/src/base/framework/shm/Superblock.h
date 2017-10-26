#ifndef REDHAWK_SHM_SUPERBLOCK_H
#define REDHAWK_SHM_SUPERBLOCK_H

#include <ostream>
#include <inttypes.h>

#include "shared_mutex.h"

namespace redhawk {

    namespace shm {

        class ThreadState;
        class Block;

        class Superblock {
        public:
            Superblock(const std::string& heap, size_t offset, size_t size);
            ~Superblock();

            const char* heap() const;

            size_t offset() const;

            size_t size() const;

            void* allocate(ThreadState* thread, size_t bytes);

            void* attach(size_t offset);

            static void deallocate(void* ptr);

            void dump(std::ostream& stream) const;

        protected:
            struct FreeBlock;

            void _deallocate(Block* block);

            void _dump(std::ostream& stream) const;

            char* _data();
            const char* _data() const;
            const Block* _begin() const;
            const Block* _end() const;
            void _queueFree(Block* block);

            FreeBlock* _findAvailable(size_t bytes);
            void _removeFreeBlock(FreeBlock* block);

            void _coalesceNext(Block* block);
            Block* _coalescePrevious(Block* block);

            FreeBlock* _offsetToBlock(uint32_t offset);
            const FreeBlock* _offsetToBlock(uint32_t offset) const;

            char _heapname[256];
            const uint64_t _offset;
            const uint32_t _size;
            const uint32_t _dataStart;
            mutable redhawk::shared_mutex _lock;

            // Free list pointers
            uint32_t _first;
            uint32_t _last;
        };

    }
}

#endif //  REDHAWK_SHM_SUPERBLOCK_H
