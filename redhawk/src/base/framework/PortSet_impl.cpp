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

#include <ossie/PortSet_impl.h>
#include <ossie/CorbaUtils.h>

PREPARE_CF_LOGGING(PortSet_impl);

PortSet_impl::PortSet_impl ()
{
}

CF::PortSet::PortInfoSequence* PortSet_impl::getPortSet ()
{
    CF::PortSet::PortInfoSequence_var retval = new CF::PortSet::PortInfoSequence();

    for (PortServantMap::iterator port=_portServants.begin(); port!=_portServants.end(); ++port) {
        CF::PortSet::PortInfoType info;
        info.obj_ptr = getPort(port->first.c_str());
        info.name = port->first.c_str();
        info.repid = port->second->getRepid().c_str();;
        info.description = port->second->getDescription().c_str();
        info.direction = port->second->getDirection().c_str();

        ossie::corba::push_back(retval, info);
    }

    return retval._retn();
}
