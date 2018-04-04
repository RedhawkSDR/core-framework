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

#include <ossie/shm/ProcessHeap.h>
#include <ossie/shm/Heap.h>

#include <boost/thread.hpp>

using namespace redhawk::shm;

static boost::once_flag heapInit = BOOST_ONCE_INIT;
boost::scoped_ptr<Heap> ProcessHeap::_instance(0);

Heap& ProcessHeap::Instance()
{
    boost::call_once(heapInit, &ProcessHeap::_initialize);
    return *_instance;
}

void ProcessHeap::_initialize()
{
    std::ostringstream oss;
    oss << "heap-" << getpid();
    _instance.reset(new Heap(oss.str()));
}
