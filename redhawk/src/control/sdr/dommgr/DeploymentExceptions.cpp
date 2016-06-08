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

#include <ossie/componentProfile.h> 

#include "DeploymentExceptions.h"
#include "Deployment.h"

using namespace redhawk;

UsesDeviceFailure::UsesDeviceFailure(const ApplicationDeployment&, const std::vector<std::string>& ids) :
    DeploymentError("failed to satisfy usesdevice dependencies"),
    _context("application"),
    _ids(ids)
{
}

UsesDeviceFailure::UsesDeviceFailure(const ComponentDeployment* component, const std::vector<std::string>& ids) :
    DeploymentError("failed to satisfy usesdevice dependencies"),
    _context("component '" + component->getInstantiation()->getID() + "'"),
    _ids(ids)
{
}

ComponentError::ComponentError(const ComponentDeployment* deployment, const std::string& message) :
    DeploymentError(message),
    _identifier(deployment->getInstantiation()->getID())
{
    if (deployment->getImplementation()) {
        _implementation = deployment->getImplementation()->getID();
    }
}

ExecuteError::ExecuteError(const ComponentDeployment* deployment, const std::string& message) :
    ComponentError(deployment, message),
    _device(deployment->getAssignedDevice())
{
}

PlacementFailure::PlacementFailure(const ossie::ComponentInstantiation* instantiation,
                                   const std::string& message) :
    DeploymentError(message),
    _name("component " + instantiation->getID())
{
}

PlacementFailure::PlacementFailure(const ossie::SoftwareAssembly::HostCollocation& collocation,
                                   const std::string& message) :
    DeploymentError(message),
    _name("host collocation " + collocation.getID() + " (" + collocation.getName() + ")")
{
}
