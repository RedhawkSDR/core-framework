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

#include "Metrics.h"
#include "Environment.h"

#include <cstring>
#include <cassert>
#include <iostream>

#include <sys/types.h>
#include <unistd.h>

// Essentially, argv[0], used for distinguishing the executable whose metrics
// are being dumped
extern char* program_invocation_name;

using redhawk::shm::Metrics;

const bool Metrics::enabled = Metrics::_initializeMetrics();

Metrics::Metrics()
{
}

Metrics::~Metrics()
{
    dump(std::cout);
}

void Metrics::dump(std::ostream& oss)
{
    oss << "SHM metrics (" << getpid() << "):" << std::endl;
    oss << "  Executable: " << program_invocation_name << std::endl;
    oss << "  Files created: " << files_created << std::endl;
    oss << "  Files opened: " << files_opened << std::endl;
    oss << "  Files closed: " << files_closed << std::endl;
    oss << "  Files total bytes: " << files_bytes << std::endl;
    oss << "  Heaps created: "<< heaps_created << std::endl;
    oss << "  Pools created: "<< pools_created << std::endl;
    oss << "  Pools used: "<< pools_used << std::endl;
    oss << "  Pool allocations hot: "<< pools_alloc_hot << std::endl;
    oss << "  Pool allocations cold: "<< pools_alloc_cold << std::endl;
    oss << "  Pool allocations failed: "<< pools_alloc_failed << std::endl;
    oss << "  Superblocks created: " << superblocks_created << std::endl;
    oss << "  Superblocks mapped: " << superblocks_mapped << std::endl;
    oss << "  Superblocks reused: " << superblocks_reused << std::endl;
    oss << "  Superblocks unmapped: " << superblocks_unmapped << std::endl;
    oss << "  Heap clients created: " << clients_created << std::endl;
    oss << "  Heap clients destroyed: " << clients_destroyed << std::endl;
    oss << "  Blocks created: " << blocks_created << std::endl;
    oss << "  Blocks attached: " << blocks_attached << std::endl;
    oss << "  Blocks released: " << blocks_released << std::endl;
    oss << "  Blocks destroyed: " << blocks_destroyed << std::endl;
}

Metrics& Metrics::Instance()
{
    // This should never be called with metrics disabled
    assert(enabled);
    return _instance();
}

// This method should only be called once, when initializing the static
// Metrics::enabled variable
bool Metrics::_initializeMetrics()
{
    if (!redhawk::env::getEnable("RH_SHMALLOC_METRICS", false)) {
        return false;
    }
    // Instantiate the singleton now (which should be process startup time) to
    // avoid a race condition if it's deferred to point-of-use
    _instance();
    return true;
}

Metrics& Metrics::_instance()
{
    static Metrics instance;
    return instance;
}
