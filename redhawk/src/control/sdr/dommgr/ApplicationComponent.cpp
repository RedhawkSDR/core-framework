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

#include <boost/foreach.hpp>

#include "ApplicationComponent.h"
#include "Application_impl.h"

using redhawk::ApplicationComponent;

ApplicationComponent::ApplicationComponent(const std::string& identifier) :
    _identifier(identifier),
    _processId(0)
{
}

const std::string& ApplicationComponent::getIdentifier() const
{
    return _identifier;
}

const std::string& ApplicationComponent::getSoftwareProfile() const
{
    return _softwareProfile;
}

void ApplicationComponent::setSoftwareProfile(const std::string& softwareProfile)
{
    _softwareProfile = softwareProfile;
}

bool ApplicationComponent::hasNamingContext() const
{
    return !_namingContext.empty();
}

const std::string& ApplicationComponent::getNamingContext() const
{
    return _namingContext;
}

void ApplicationComponent::setNamingContext(const std::string& namingContext)
{
    _namingContext = namingContext;
}

const std::string& ApplicationComponent::getImplementationId() const
{
    return _implementationId;
}

void ApplicationComponent::setImplementationId(const std::string& implementationId)
{
    _implementationId = implementationId;
}

unsigned long ApplicationComponent::getProcessId() const
{
    return _processId;
}

void ApplicationComponent::setProcessId(unsigned long processId)
{
    _processId = processId;
}

bool ApplicationComponent::isTerminated() const
{
    return (_processId == 0);
}

bool ApplicationComponent::isRegistered() const
{
    return !CORBA::is_nil(_componentObject);
}

CORBA::Object_ptr ApplicationComponent::getComponentObject() const
{
    return CORBA::Object::_duplicate(_componentObject);
}

void ApplicationComponent::setComponentObject(CORBA::Object_ptr object)
{
    _componentObject = CORBA::Object::_duplicate(object);
}

CF::Resource_ptr ApplicationComponent::getResourcePtr() const
{
    return CF::Resource::_narrow(_componentObject);
}

CF::Device_ptr ApplicationComponent::getAssignedDevice() const
{
    return CF::Device::_duplicate(_assignedDevice);
}

void ApplicationComponent::setAssignedDevice(CF::Device_ptr assignedDevice)
{
    _assignedDevice = CF::Device::_duplicate(assignedDevice);
}

void ApplicationComponent::releaseObject()
{
    if (!isRegistered()) {
        return;
    }

    LOG_DEBUG(Application_impl, "Releasing component '" << _identifier << "'");
    try {
        CF::Resource_var resource = CF::Resource::_narrow(_componentObject);
        unsigned long timeout = 3; // seconds;
        omniORB::setClientCallTimeout(resource, timeout * 1000);
        resource->releaseObject();
    } CATCH_LOG_WARN(Application_impl, "releaseObject failed for component '" << _identifier << "'");
}

void ApplicationComponent::terminate()
{
    // TODO: PIDs are not necessarily capped at 64K; the limit is system-
    // dependent
    if (_processId == 0 || _processId >= 65536) {
        return;
    }

    CF::ExecutableDevice_var device = ossie::corba::_narrowSafe<CF::ExecutableDevice>(_assignedDevice);
    if (CORBA::is_nil(device)) {
        LOG_WARN(Application_impl, "Cannot find device to terminate component " << _identifier);
    } else {
        device->terminate(_processId);
    }
}

void ApplicationComponent::unloadFiles()
{
    if (loadedFiles.empty()) {
        return;
    }

    LOG_DEBUG(Application_impl, "Unloading " << loadedFiles.size() << " file(s) for component '"
              << _identifier << "'");
        
    CF::LoadableDevice_var device = ossie::corba::_narrowSafe<CF::LoadableDevice>(_assignedDevice);
    if (CORBA::is_nil(device)) {
        LOG_WARN(Application_impl, "Cannot find device to unload files for component " << _identifier);
        return;
    }

    BOOST_FOREACH(const std::string& file, loadedFiles) {
        LOG_TRACE(Application_impl, "Unloading file " << file);
        try {
            device->unload(file.c_str());
        } CATCH_LOG_WARN(Application_impl, "Unable to unload file " << file);
    }
}
