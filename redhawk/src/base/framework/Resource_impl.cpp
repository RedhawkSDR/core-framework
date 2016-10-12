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


#include "ossie/Resource_impl.h"
#include <ossie/prop_helpers.h>

PREPARE_LOGGING(Resource_impl)

Resource_impl::Resource_impl (const char* _uuid) :
    component_running_mutex(),
    component_running(&component_running_mutex)
{
    _identifier = _uuid;
}


Resource_impl::Resource_impl (const char* _uuid, const char *label) :
    component_running_mutex(),
    component_running(&component_running_mutex)
{
    _identifier = _uuid;
    naming_service_name = label;
}


void Resource_impl::start () throw (CORBA::SystemException, CF::Resource::StartError)
{
    _started = true;
}


void Resource_impl::stop () throw (CORBA::SystemException, CF::Resource::StopError)
{
    _started = false;
}

char* Resource_impl::identifier () throw (CORBA::SystemException)
{
    return CORBA::string_dup(_identifier.c_str());
}

CORBA::Boolean Resource_impl::started () throw (CORBA::SystemException)
{
    return _started;
}

void Resource_impl::releaseObject()
throw (CORBA::SystemException, CF::LifeCycle::ReleaseError) {
    PortableServer::POA_ptr root_poa = ossie::corba::RootPOA();
    PortableServer::ObjectId_var oid = root_poa->servant_to_id(this);
    root_poa->deactivate_object(oid);

    component_running.signal();
}

void Resource_impl::run() {
    // Start handling CORBA requests
    LOG_TRACE(Resource_impl, "handling CORBA requests");
    component_running.wait();
    LOG_TRACE(Resource_impl, "leaving run()");
}

void Resource_impl::halt() {
    LOG_DEBUG(Resource_impl, "Halting component")

    LOG_TRACE(Resource_impl, "Sending device running signal");
    component_running.signal();
    LOG_TRACE(Resource_impl, "Done sending device running signal");
}

void Resource_impl::registerInPort(Port_Provides_base_impl *port) {
    std::string portName(port->getName());
    std::map<std::string, Port_Provides_base_impl *>::iterator p;
    if (( p = inPorts.find(portName)) != inPorts.end()) {
        // port is already registered. Assume that the new one must replace the old one
        PortableServer::ServantBase* ptr = dynamic_cast<PortableServer::ServantBase*>(p->second);
        if (ptr) {
            PortableServer::ObjectId_var oid;
            PortableServer::POA_ptr poa = ossie::corba::RootPOA();
            oid = poa->servant_to_id(ptr);
            poa->deactivate_object(oid);
        }
        delete p->second;
        inPorts.erase(p);
    }
    inPorts.insert(std::pair<std::string, Port_Provides_base_impl *>(portName, port));
}

void Resource_impl::registerOutPort(Port_Uses_base_impl *port, CF::Port_ptr ref) {
    std::string portName(port->getName());
    std::map<std::string, Port_Uses_base_impl *>::iterator p;
    if (( p = outPorts.find(portName)) != outPorts.end()) {
        // port is already registered. Assume that the new one must replace the old one
        PortableServer::ServantBase* ptr = dynamic_cast<PortableServer::ServantBase*>(p->second);
        if (ptr) {
            PortableServer::ObjectId_var oid;
            PortableServer::POA_ptr poa = ossie::corba::RootPOA();
            oid = poa->servant_to_id(ptr);
            poa->deactivate_object(oid);
        }
        delete p->second;
        outPorts.erase(p);
    }
    outPorts.insert(std::pair<std::string, Port_Uses_base_impl *>(portName, port));
    outPorts_var.erase(portName);
    outPorts_var.insert(std::pair<std::string, CF::Port_var>(portName, ref));
}

void Resource_impl::releaseInPorts() {
    deactivateInPorts();
    std::map<std::string, Port_Provides_base_impl *>::iterator p;
    while (inPorts.size() != 0) {
        p = inPorts.begin();
        inPorts.erase(p);
    }
}

void Resource_impl::releaseOutPorts() {
    deactivateOutPorts();
    std::map<std::string, Port_Uses_base_impl *>::iterator p;
    std::map<std::string, CF::Port_var>::iterator p_var;
    while (outPorts_var.size() != 0) {
        p_var = outPorts_var.begin();
        outPorts_var.erase(p_var);
    }
    while (! outPorts.empty() ) {
        p = outPorts.begin();
        (p->second)->releasePort();
        outPorts.erase(p);
    }
}

void Resource_impl::deactivateOutPorts() {
    PortableServer::ObjectId_var oid;
    PortableServer::POA_ptr poa = ossie::corba::RootPOA();
    std::map<std::string, Port_Uses_base_impl *>::iterator p;
    for (p = outPorts.begin(); p != outPorts.end(); p++) {
        PortableServer::ServantBase* ptr = dynamic_cast<PortableServer::ServantBase*>(p->second);
        if (ptr) {
            oid = poa->servant_to_id(ptr);
            poa->deactivate_object(oid);
        }
    }
}

void Resource_impl::deactivateInPorts() {
    PortableServer::ObjectId_var oid;
    PortableServer::POA_ptr poa = ossie::corba::RootPOA();
    std::map<std::string, Port_Provides_base_impl *>::iterator p;
    for (p = inPorts.begin(); p != inPorts.end(); p++) {
        PortableServer::ServantBase* ptr = dynamic_cast<PortableServer::ServantBase*>(p->second);
        if (ptr) {
            oid = poa->servant_to_id(ptr);
            poa->deactivate_object(oid);
        }
    }
}
