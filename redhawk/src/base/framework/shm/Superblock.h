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

            size_t used() const;

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

            volatile size_t _used;

            // Free list pointers
            uint32_t _first;
            uint32_t _last;
        };

    }
}

#endif //  REDHAWK_SHM_SUPERBLOCK_H
