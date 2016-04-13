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
#ifndef CONTROL_H
#define CONTROL_H

#include <string>

#include <inttypes.h>

struct control {
    volatile uint64_t total_bytes;
    volatile double average_time;
    volatile uint32_t transfer_size;
};

control* open_control(const std::string& filename);
void close_control(control* address);

#endif // CONTROL_H
