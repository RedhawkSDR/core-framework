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

#ifndef REDHAWK_SHM_MAPPEDFILE_H
#define REDHAWK_SHM_MAPPEDFILE_H

#include <string>
#include <sys/mman.h>

namespace redhawk {

    namespace shm {

        class MappedFile {
        public:
            enum mode_e {
                READONLY,
                READWRITE
            };

            static const size_t PAGE_SIZE;

            MappedFile(const std::string& name);
            ~MappedFile();

            void create();
            void open();

            const std::string& name() const;

            size_t size() const;
            void resize(size_t bytes);

            void* map(size_t bytes, mode_e mode, off_t offset=0);
            void* remap(void* oldAddr, size_t oldSize, size_t newSize);
            void unmap(void* addr, size_t bytes);

            void close();
            void unlink();

        private:
            // Non-copyable, non-assignable
            MappedFile(const MappedFile&);
            MappedFile& operator=(const MappedFile&);

            const std::string _name;
            int _fd;
        };
    }
}

#endif // REDHAWK_SHM_MAPPEDFILE_H
