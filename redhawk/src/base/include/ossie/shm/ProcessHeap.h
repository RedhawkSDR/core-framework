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

#ifndef REDHAWK_SHM_PROCESSHEAP_H
#define REDHAWK_SHM_PROCESSHEAP_H

#include <boost/scoped_ptr.hpp>

#include "Heap.h"

namespace redhawk {

    namespace shm {

        class ProcessHeap {
        public:
            static Heap& Instance();
        private:
            // Cannot instantiate
            ProcessHeap();

            static void _initialize();

            static boost::scoped_ptr<Heap> _instance;
        };
    }
}

#endif // REDHAWK_SHM_PROCESSHEAP_H
