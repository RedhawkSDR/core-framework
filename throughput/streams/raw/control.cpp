/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK throughput.
 *
 * REDHAWK throughput is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK throughput is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "control.h"

control* open_control(const std::string& filename)
{
    int fd = open(filename.c_str(), O_RDWR);
    if (fd < 0) {
        return 0;
    }

    void* address = mmap(NULL, sizeof(control), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    return reinterpret_cast<control*>(address);
}

void close_control(control* address)
{
    munmap(address, sizeof(control));
}
