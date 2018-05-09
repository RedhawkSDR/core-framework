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

#include "PortManager.h"

#include <algorithm>
#include <functional>

PortManager::PortManager() :
    PortSupplier_impl()
{
}

PortManager::~PortManager()
{
    releaseObject();
}

void PortManager::addPort(PortBase* port)
{
    PortSupplier_impl::addPort(port->getName(), port);

    // Take ownership of the port; if the caller requires a longer lifetime for
    // the port, they must increment the reference count themselves
    _ports.push_back(port);
}

void PortManager::start()
{
    startPorts();
}

void PortManager::stop()
{
    stopPorts();
}

void PortManager::releaseObject()
{
    releasePorts();

    std::for_each(_ports.begin(), _ports.end(), std::mem_fun(&PortBase::_remove_ref));
    _ports.clear();
}
