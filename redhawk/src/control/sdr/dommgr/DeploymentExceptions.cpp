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

#include <sstream>

#include <boost/foreach.hpp>

#include <ossie/componentProfile.h> 

#include "DeploymentExceptions.h"
#include "Deployment.h"

using namespace redhawk;

UsesDeviceFailure::UsesDeviceFailure(const ApplicationDeployment&, const std::vector<std::string>& ids) :
    DeploymentError(CF::CF_ENOSPC, "failed to satisfy usesdevice dependencies"),
    _context("application"),
    _ids(ids)
{
}

UsesDeviceFailure::UsesDeviceFailure(const ComponentDeployment* component, const std::vector<std::string>& ids) :
    DeploymentError(CF::CF_ENOSPC, "failed to satisfy usesdevice dependencies"),
    _context("component '" + component->getInstantiation()->getID() + "'"),
    _ids(ids)
{
}

std::string UsesDeviceFailure::message() const
{
    std::ostringstream msg;
    msg << "Failed to satisfy 'usesdevice' dependencies ";
    bool first = true;
    BOOST_FOREACH(const std::string& id, ids()) {
        if (!first) {
            msg << ", ";
        } else {
            first = false;
        }
        msg << id;
    }
    msg << " for " << context();
    return msg.str();
}

std::string ConnectionError::message() const
{
    std::ostringstream msg;
    msg << "Unable to make connection '" << identifier() << "': " << what();
    return msg.str();
}

ComponentError::ComponentError(const ComponentDeployment* deployment, const std::string& message) :
    DeploymentError(CF::CF_EINVAL, message),
    _identifier(deployment->getInstantiation()->getID())
{
    if (deployment->getImplementation()) {
        _implementation = deployment->getImplementation()->getID();
    }
}

std::string ComponentError::message() const
{
    std::ostringstream msg;
    msg << "Deploying component " << identifier();
    if (!implementation().empty()) {
        msg << " implementation " << implementation();
    }
    msg << " failed: " << what();
    return msg.str();
}

ExecuteError::ExecuteError(const ComponentDeployment* deployment, const std::string& message) :
    ComponentError(deployment, message),
    _device(deployment->getAssignedDevice())
{
    // Override the default ComponentError errorNumber; this is simpler than
    // having an extra ComponentError constructor
    errorNumber(CF::CF_EIO);
}

std::string ExecuteError::message() const
{
    std::ostringstream msg;
    msg << "Executing component " << identifier();
    msg << " implementation " << implementation();
    msg << " failed on device " << device()->identifier;
    msg << ": " << what();
    return msg.str();
}

std::string PropertiesError::message() const
{
    std::ostringstream msg;
    msg << "Component " << identifier();
    msg << " " << what();
    msg << " " << properties();
    return msg.str();
}

PlacementFailure::PlacementFailure(const ossie::ComponentInstantiation* instantiation,
                                   const std::string& message) :
    DeploymentError(CF::CF_EIO, message),
    _name("component " + instantiation->getID())
{
}

PlacementFailure::PlacementFailure(const ossie::SoftwareAssembly::HostCollocation& collocation,
                                   const std::string& message) :
    DeploymentError(CF::CF_EIO, message),
    _name("host collocation " + collocation.getID() + " (" + collocation.getName() + ")")
{
}

std::string PlacementFailure::message() const
{
    std::ostringstream msg;
    msg << "Failed to place " << name() << ": " << what();
    return msg.str();
}

BadExternalPort::BadExternalPort(const ossie::SoftwareAssembly::Port& port, const std::string& message) :
    DeploymentError(CF::CF_EINVAL, message),
    _name(port.getExternalName()),
    _component(port.componentrefid)
{
}

std::string BadExternalPort::message() const
{
    std::ostringstream msg;
    msg << "Could not create external port '" << name();
    msg << "' from component '" << component();
    msg << "': " << what();
    return msg.str();
}

std::string ComponentTerminated::message() const
{
    std::ostringstream msg;
    msg << "Component '" << identifier() << "' terminated abnormally";
    return msg.str();
}
