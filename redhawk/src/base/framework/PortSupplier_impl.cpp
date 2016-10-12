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

#include <ossie/PortSupplier_impl.h>

PREPARE_LOGGING(PortSupplier_impl);

PortSupplier_impl::PortSupplier_impl ()
{
}

CORBA::Object* PortSupplier_impl::getPort (const char* name) throw (CORBA::SystemException, CF::PortSupplier::UnknownPort)
{
    PortServantMap::iterator port = _portServants.find(name);
    if (port != _portServants.end()) {
        PortBase* servant = port->second;
        PortableServer::POA_var poa = servant->_default_POA();
        return poa->servant_to_reference(port->second);
    }
    throw CF::PortSupplier::UnknownPort();
}

void PortSupplier_impl::addPort (const std::string& name, PortBase* servant)
{
    LOG_TRACE(PortSupplier_impl, "Adding port '" << name << "'");
    insertPort(name, servant);

    // Activate the port in its default POA (usually, the root)
    LOG_TRACE(PortSupplier_impl, "Activating port '" << name << "'");
    PortableServer::POA_var poa = servant->_default_POA();
    PortableServer::ObjectId_var oid = poa->activate_object(servant);
}

void PortSupplier_impl::startPorts ()
{
    TRACE_ENTER(PortSupplier_impl);
    for (PortServantMap::iterator port = _portServants.begin(); port != _portServants.end(); ++port) {
        port->second->startPort();
    }
    TRACE_EXIT(PortSupplier_impl);
}

void PortSupplier_impl::stopPorts ()
{
    TRACE_ENTER(PortSupplier_impl);
    for (PortServantMap::iterator port = _portServants.begin(); port != _portServants.end(); ++port) {
        port->second->stopPort();
    }
    TRACE_EXIT(PortSupplier_impl);
}

void PortSupplier_impl::releasePorts ()
{
    TRACE_ENTER(PortSupplier_impl);
    for (PortServantMap::iterator port = _portServants.begin(); port != _portServants.end(); ++port) {
        deactivatePort(port->second);
        port->second->releasePort();
    }
    _portServants.clear();
    TRACE_EXIT(PortSupplier_impl);
}

void PortSupplier_impl::deactivatePort (PortBase* servant)
{
    LOG_TRACE(PortSupplier_impl, "Deactivating port '" << servant->getName() << "'");
    PortableServer::POA_var poa = servant->_default_POA();
    PortableServer::ObjectId_var oid = poa->servant_to_id(servant);
    poa->deactivate_object(oid);
}

void PortSupplier_impl::insertPort (const std::string& name, PortBase* servant)
{
    PortServantMap::iterator existing = _portServants.find(name);
    if (existing != _portServants.end()) {
        // A port is already registered with the given name, assume that the
        // new one must replace the old one
        LOG_DEBUG(PortSupplier_impl, "Replacing existing port '" << name << "'");
        deactivatePort(existing->second);
    }
    _portServants[name] = servant;
}

void PortSupplier_impl::registerInPort(Port_Provides_base_impl *port) {
    const std::string name(port->getName());
    insertPort(name, port);

    // Backwards-compatibility 
    std::map<std::string, Port_Provides_base_impl *>::iterator existing = inPorts.find(name);
    if (existing != inPorts.end()) {
        delete existing->second;
    }
    inPorts[name] = port;
}

void PortSupplier_impl::registerOutPort(Port_Uses_base_impl *port, CF::Port_ptr ref) {
    const std::string name(port->getName());
    insertPort(name, port);

    // Backwards-compatibility 
    RH_UsesPortMap::iterator existing = outPorts.find(name);
    if (existing != outPorts.end()) {
        delete existing->second;
    }
    outPorts[name] = port;
    outPorts_var[name] = ref;
}

void PortSupplier_impl::releaseInPorts() {
    deactivateInPorts();
    // further attemps are made to access them
    for (RH_ProvidesPortMap::iterator port = inPorts.begin(); port != inPorts.end(); ++port) {
        _portServants.erase(port->first);
    }
    inPorts.clear();
}

void PortSupplier_impl::releaseOutPorts() {
    deactivateOutPorts();
    outPorts_var.clear();
    for (RH_UsesPortMap::iterator port = outPorts.begin(); port != outPorts.end(); ++port) {
        port->second->releasePort();
        // Remove the uses port servant from the main map to ensure that no
        // further attemps are made to access it
        _portServants.erase(port->first);
    }
    outPorts.clear();
}

void PortSupplier_impl::deactivateOutPorts() {
    for (RH_UsesPortMap::iterator port = outPorts.begin(); port != outPorts.end(); ++port) {
        deactivatePort(port->second);
    }
}

void PortSupplier_impl::deactivateInPorts() {
    for (RH_ProvidesPortMap::iterator port = inPorts.begin(); port != inPorts.end(); ++port) {
        deactivatePort(port->second);
    }
}
