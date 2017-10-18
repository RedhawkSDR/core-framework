/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef __bulkio_corbatransport_h
#define __bulkio_corbatransport_h

#include <BulkioTransport.h>

namespace bulkio {

    template <typename PortType>
    class CorbaTransportFactory {
    public:
        typedef typename PortType::_ptr_type PtrType;

        static OutputTransport<PortType>* Create(OutPort<PortType>* parent, PtrType port);
    };

}

#endif // __bulkio_corbatransport_h
