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

#ifndef REDHAWK_SHMHEAP_H
#define REDHAWK_SHMHEAP_H

#include <map>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

#include "ShmFile.h"

namespace redhawk {

    class ShmArena;
    class ThreadState;

    class ShmHeap {
    public:
        struct ID {
            size_t arena;
            size_t offset;
        };

        ShmHeap(const std::string& name);
        ~ShmHeap();

        void* allocate(size_t bytes);
        void deallocate(void* ptr);

        void unlink();

        ID getID(void* ptr);

        const std::string& name() const;

    private:
        struct PrivateHeap;

        // Not copyable
        ShmHeap& operator=(const ShmHeap&);

        // Default to 2MB arena
        static const size_t DEFAULT_ARENA_SIZE = 2097152;

        void* _allocateFromArena(ThreadState* state, size_t bytes);
        ShmArena* _createArena(size_t minSize);

        PrivateHeap* _getPrivateHeap();
        ThreadState* _getThreadState();

        // Serializes access to all members except thread-specific data
        boost::mutex _mutex;

        ShmFile _shm;

        std::vector<PrivateHeap*> _allocs;

        boost::thread_specific_ptr<ThreadState> _threadState;

        typedef std::vector<ShmArena*> ArenaList;
        ArenaList _arenas;
    };

    class ShmHeapClient {
    public:
        ShmHeapClient(const std::string& name);

        void* fetch(ShmHeap::ID id);
        static void deallocate(void* ptr);

    private:
        ShmFile _shm;

        ShmArena* _attach(size_t offset);

        typedef std::map<size_t,ShmArena*> ArenaMap;
        ArenaMap _arenas;
    };
}

#endif // REDHAWK_SHMHEAP_H
