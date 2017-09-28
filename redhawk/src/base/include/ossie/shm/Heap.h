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

#ifndef REDHAWK_SHM_HEAP_H
#define REDHAWK_SHM_HEAP_H

#include <map>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

#include "MappedFile.h"

namespace redhawk {

    namespace shm {
        class Superblock;
        class ThreadState;

        class Heap {
        public:
            struct ID {
                size_t superblock;
                size_t offset;
            };

            Heap(const std::string& name);
            ~Heap();

            void* allocate(size_t bytes);
            void deallocate(void* ptr);

            void unlink();

            ID getID(const void* ptr);

            const std::string& name() const;

        private:
            struct PrivateHeap;

            // Not copyable
            Heap(const Heap&);
            Heap& operator=(const Heap&);

            // Default to 2MB superblock
            static const size_t DEFAULT_SUPERBLOCK_SIZE = 2097152;

            Superblock* _createSuperblock(size_t minSize);

            PrivateHeap* _getPrivateHeap();
            ThreadState* _getThreadState();

            // Serializes access to all members except thread-specific data
            boost::mutex _mutex;

            MappedFile _shm;

            std::vector<PrivateHeap*> _allocs;

            boost::thread_specific_ptr<ThreadState> _threadState;

            typedef std::vector<Superblock*> SuperblockList;
            SuperblockList _superblocks;
        };

        class HeapClient {
        public:
            HeapClient(const std::string& name);

            void* fetch(Heap::ID id);
            static void deallocate(void* ptr);

        private:
            MappedFile _shm;

            Superblock* _attach(size_t offset);

            typedef std::map<size_t,Superblock*> SuperblockMap;
            SuperblockMap _superblocks;
        };
    }
}

#endif // REDHAWK_SHM_HEAP_H
