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

// TODO: Should probably use its own logger; using Application_impl's for
// consistency with old code
PREPARE_ALT_LOGGING(ApplicationComponent, Application_impl);

ApplicationComponent::ApplicationComponent(const std::string& identifier) :
    _identifier(identifier),
    _name(identifier),
    _isVisible(true),
    _processId(0),
    _componentHost(0)
{
}

const std::string& ApplicationComponent::getIdentifier() const
{
    return _identifier;
}

const std::string& ApplicationComponent::getName() const
{
    return _name;
}

void ApplicationComponent::setName(const std::string& name)
{
    _name = name;
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

bool ApplicationComponent::isVisible() const
{
    return _isVisible;
}

void ApplicationComponent::setVisible(bool visible)
{
    _isVisible = visible;
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
    try {
        _resource->start();
    } catch (const CF::Resource::StartError& exc) {
        std::ostringstream message;
        message << "Component '" << _name << "' failed to start: ";
        message << exc.msg;
        throw CF::Resource::StartError(exc.errorNumber, message.str().c_str());
    } catch (const CORBA::SystemException& exc) {
        std::ostringstream message;
        message << "Component '" << _name << "' failed to start: ";
        message << ossie::corba::describeException(exc);
        throw CF::Resource::StartError(CF::CF_EIO, message.str().c_str());
    }
}

bool ApplicationComponent::stop(float timeout)
{
    if (timeout < 0)
        timeout = 0;
    omniORB::setClientCallTimeout(_resource, timeout * 1000);
    try {
        _resource->stop();
        return true;
    } catch (const CF::Resource::StopError& error) {
        RH_ERROR(_appComponentLog, "Failed to stop " << _identifier << "; CF::Resource::StopError '" << error.msg << "'");
    } catch (const CORBA::SystemException& exc) {
        if (!isTerminated()) {
            RH_ERROR(_appComponentLog, "Failed to stop component '" << _identifier << "'; "
                      << ossie::corba::describeException(exc));
        } else {
            RH_DEBUG(_appComponentLog, "Ignoring CORBA exception stopping terminated component '"
                      << _identifier << "'");
        }
    } catch (...) {
        RH_ERROR(_appComponentLog, "Failed to stop " << _identifier);
    }
    return false;
}

void ApplicationComponent::releaseObject()
{
    if (!isResource()) {
        return;
    }

    RH_DEBUG(_appComponentLog, "Releasing component '" << _identifier << "'");
    try {
        unsigned long timeout = 3; // seconds;
        omniORB::setClientCallTimeout(_resource, timeout * 1000);
        _resource->releaseObject();
    } catch (const CORBA::SystemException& exc) {
        if (!isTerminated()) {
            RH_ERROR(_appComponentLog, "Failed to release component '" << _identifier << "'; "
                      << ossie::corba::describeException(exc));
        } else {
            RH_DEBUG(_appComponentLog, "Ignoring CORBA exception releasing terminated component '"
                      << _identifier << "'");
        }
    } CATCH_RH_WARN(_appComponentLog, "releaseObject failed for component '" << _identifier << "'");
}

void ApplicationComponent::terminate()
{
    // If the process already terminated, or the component is running inside of
    // a ComponentHost instance, skip termination
    if (isTerminated() || _componentHost) {
        return;
    }

    if (!_assignedDevice || !_assignedDevice->isExecutable()) {
        RH_WARN(_appComponentLog, "Cannot find device to terminate component " << _identifier);
        return;
    }

    RH_DEBUG(_appComponentLog, "Terminating component '" << _identifier
              << "' on device '" << _assignedDevice->label
              << "' (" << _assignedDevice->identifier << ")");
    try {
        _assignedDevice->executableDevice->terminate(_processId);
    } catch (const CF::ExecutableDevice::InvalidProcess& ip) {
        RH_ERROR(_appComponentLog, "Failed to terminate process for component '" << _identifier
                  << "': invalid process");
    } catch (const CF::Device::InvalidState& state) {
        RH_ERROR(_appComponentLog, "Failed to terminate process for component '" << _identifier
                  << "': device '" << _assignedDevice->label << "' is in an invalid state");
    } catch (const CORBA::SystemException& exc) {
        RH_ERROR(_appComponentLog, "Failed to terminate process for component '" << _identifier
                  << "': " << ossie::corba::describeException(exc));
    }
}

void ApplicationComponent::unloadFiles()
{
    if (_loadedFiles.empty()) {
        return;
    }

    RH_DEBUG(_appComponentLog, "Unloading " << _loadedFiles.size() << " file(s) for component '"
              << _identifier << "'");

    if (!_assignedDevice || !_assignedDevice->isLoadable()) {
        RH_WARN(_appComponentLog, "Cannot find device to unload files for component " << _identifier);
        return;
    }

    BOOST_FOREACH(const std::string& file, _loadedFiles) {
        RH_TRACE(_appComponentLog, "Unloading file " << file);
        try {
            _assignedDevice->loadableDevice->unload(file.c_str());
        } CATCH_RH_WARN(_appComponentLog, "Unable to unload file " << file);
    }
}
