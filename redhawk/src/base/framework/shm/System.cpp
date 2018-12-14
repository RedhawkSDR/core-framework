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

#include <sys/statvfs.h>

#include <ossie/shm/System.h>

#define SHMDIR "/dev/shm"

namespace redhawk {
    namespace shm {

        const char* getSystemPath()
        {
            return SHMDIR;
        }

        size_t getSystemTotalMemory()
        {
            struct statvfs status;
            if (statvfs(SHMDIR, &status)) {
                return 0;
            }

            return (status.f_blocks * status.f_frsize);
        }

        size_t getSystemFreeMemory()
        {
            struct statvfs status;
            if (statvfs(SHMDIR, &status)) {
                return 0;
            }
            return (status.f_bfree * status.f_frsize);
        }
    }
}
