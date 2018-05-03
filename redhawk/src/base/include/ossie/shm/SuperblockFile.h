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

#ifndef REDHAWK_SHM_SUPERBLOCKFILE_H
#define REDHAWK_SHM_SUPERBLOCKFILE_H

#include <map>

#include "MappedFile.h"

namespace redhawk {

    namespace shm {

        class Superblock;

        class SuperblockFile {
        public:
            struct Statistics
            {
                size_t size;
                size_t used;
                size_t superblocks;
                size_t unused;
            };

            SuperblockFile(const std::string& name);
            ~SuperblockFile();

            static bool IsSuperblockFile(const std::string& name);

            pid_t creator() const;
            int refcount() const;
            bool isOrphaned() const;

            Statistics getStatistics();

            void create();
            void open(bool attach=true);
            void close();

            Superblock* getSuperblock(size_t offset);
            Superblock* createSuperblock(size_t bytes);

            const std::string& name() const;

            MappedFile& file();

        private:
            // Non-copyable, non-assignable
            SuperblockFile(const SuperblockFile&);
            SuperblockFile& operator=(const SuperblockFile&);

            void _detach();

            Superblock* _mapSuperblock(size_t offset);

            MappedFile _file;
            bool _attached;

            struct Header;
            Header* _header;

            typedef std::map<size_t,Superblock*> SuperblockMap;
            SuperblockMap _superblocks;
        };
    }
}

#endif // REDHAWK_SHM_SUPERBLOCKFILE_H
