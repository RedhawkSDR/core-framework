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

#include <ossie/CorbaUtils.h>

#include "ApplicationComponent.h"
#include "Application_impl.h"

using redhawk::ApplicationComponent;

ApplicationComponent::ApplicationComponent(const std::string& identifier) :
    _identifier(identifier),
    _processId(0),
    _componentHost(0)
{
}

const std::string& ApplicationComponent::getIdentifier() const
{
    return _identifier;
}

const std::string& ApplicationComponent::getInstantiationId() const
{
    return _instantiationId;
}

void ApplicationComponent::setInstantiationId(const std::string& instantiationId)
{
    _instantiationId = instantiationId;
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

ApplicationComponent* ApplicationComponent::getComponentHost()
{
    return _componentHost;
}

void ApplicationComponent::setComponentHost(ApplicationComponent* componentHost)
{
    _componentHost = componentHost;
    if (_componentHost) {
        _componentHost->_children.push_back(this);
    }
}

const std::vector<ApplicationComponent*>& ApplicationComponent::getChildren() const
{
    return _children;
}

unsigned long ApplicationComponent::getProcessId() const
{
    if (_componentHost) {
        return _componentHost->getProcessId();
    }
    return _processId;
}

void ApplicationComponent::setProcessId(unsigned long processId)
{
    _processId = processId;
}

bool ApplicationComponent::isResource() const
{
    return !CORBA::is_nil(_resource);
}

bool ApplicationComponent::isTerminated() const
{
    return (getProcessId() == 0);
}

bool ApplicationComponent::isRegistered() const
{
    return !CORBA::is_nil(_componentObject);
}

const std::vector<std::string>& ApplicationComponent::getLoadedFiles() const
{
    return _loadedFiles;
}

void ApplicationComponent::addLoadedFile(const std::string& fileName)
{
    _loadedFiles.push_back(fileName);
}

CORBA::Object_ptr ApplicationComponent::getComponentObject() const
{
    return CORBA::Object::_duplicate(_componentObject);
}

void ApplicationComponent::setComponentObject(CORBA::Object_ptr object)
{
    _componentObject = CORBA::Object::_duplicate(object);
    _resource = ossie::corba::_narrowSafe<CF::Resource>(object);
}

CF::Resource_ptr ApplicationComponent::getResourcePtr() const
{
    return CF::Resource::_duplicate(_resource);
}

const boost::shared_ptr<ossie::DeviceNode>& ApplicationComponent::getAssignedDevice() const
{
    return _assignedDevice;
}

void ApplicationComponent::setAssignedDevice(const boost::shared_ptr<ossie::DeviceNode>& assignedDevice)
{
    _assignedDevice = assignedDevice;
}

void ApplicationComponent::start()
{
    omniORB::setClientCallTimeout(_resource, 0);
    _resource->start();
}

bool ApplicationComponent::stop()
{
    const unsigned long timeout = 3; // seconds
    omniORB::setClientCallTimeout(_resource, timeout * 1000);
    try {
        _resource->stop();
        return true;
    } catch (const CF::Resource::StopError& error) {
        LOG_ERROR(Application_impl, "Failed to stop " << _identifier << "; CF::Resource::StopError '" << error.msg << "'");
    } catch (const CORBA::SystemException& exc) {
        if (!isTerminated()) {
            LOG_ERROR(Application_impl, "Failed to stop component '" << _identifier << "'; "
                      << ossie::corba::describeException(exc));
        } else {
            LOG_DEBUG(Application_impl, "Ignoring CORBA exception stopping terminated component '"
                      << _identifier << "'");
        }
    } catch (...) {
        LOG_ERROR(Application_impl, "Failed to stop " << _identifier);
    }
    return false;
}

void ApplicationComponent::releaseObject()
{
    if (!isResource()) {
        return;
    }

    LOG_DEBUG(Application_impl, "Releasing component '" << _identifier << "'");
    try {
        unsigned long timeout = 3; // seconds;
        omniORB::setClientCallTimeout(_resource, timeout * 1000);
        _resource->releaseObject();
    } catch (const CORBA::SystemException& exc) {
        if (!isTerminated()) {
            LOG_ERROR(Application_impl, "Failed to release component '" << _identifier << "'; "
                      << ossie::corba::describeException(exc));
        } else {
            LOG_DEBUG(Application_impl, "Ignoring CORBA exception releasing terminated component '"
                      << _identifier << "'");
        }
    } CATCH_LOG_WARN(Application_impl, "releaseObject failed for component '" << _identifier << "'");
}

void ApplicationComponent::terminate()
{
    // If the process already terminated, or the component is running inside of
    // a ComponentHost instance, skip termination
    if (isTerminated() || _componentHost) {
        return;
    }

    if (!_assignedDevice || !_assignedDevice->isExecutable()) {
        LOG_WARN(Application_impl, "Cannot find device to terminate component " << _identifier);
    } else {
        _assignedDevice->executableDevice->terminate(_processId);
    }
}

void ApplicationComponent::unloadFiles()
{
    if (_loadedFiles.empty()) {
        return;
    }

    LOG_DEBUG(Application_impl, "Unloading " << _loadedFiles.size() << " file(s) for component '"
              << _identifier << "'");

    if (!_assignedDevice || !_assignedDevice->isLoadable()) {
        LOG_WARN(Application_impl, "Cannot find device to unload files for component " << _identifier);
        return;
    }

    BOOST_FOREACH(const std::string& file, _loadedFiles) {
        LOG_TRACE(Application_impl, "Unloading file " << file);
        try {
            _assignedDevice->loadableDevice->unload(file.c_str());
        } CATCH_LOG_WARN(Application_impl, "Unable to unload file " << file);
    }
}
