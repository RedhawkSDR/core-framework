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

#include <ossie/Transport.h>
#include <ossie/debug.h>

namespace redhawk {

    TransportRegistry::TransportRegistry()
    {
    }

    void TransportRegistry::RegisterTransport(TransportFactory* transport)
    {
        Instance()._registerTransport(transport);
    }

    TransportStack* TransportRegistry::GetTransports(const std::string& repid)
    {
        return Instance()._getTransport(repid);
    }

    void TransportRegistry::_registerTransport(TransportFactory* transport)
    {
        const std::string repid = transport->repid();
        _registry[repid].push_back(transport);
    }

    TransportStack* TransportRegistry::_getTransport(const std::string& repid)
    {
        TransportMap::iterator transport = _registry.find(repid);
        if (transport != _registry.end()) {
            return &(transport->second);
        }
        return 0;
    }

    TransportRegistry& TransportRegistry::Instance()
    {
        static TransportRegistry instance;
        return instance;
    }
}
