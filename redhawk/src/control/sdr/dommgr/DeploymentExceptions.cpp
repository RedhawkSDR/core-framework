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

deployment_error::deployment_error(const ComponentDeployment* deployment, const std::string& message) :
    std::runtime_error(message),
    _identifier(deployment->getIdentifier())
{
    if (deployment->getImplementation()) {
        _implementation = deployment->getImplementation()->getID();
    }
}

placement_failure::placement_failure(const ossie::ComponentInstantiation* instantiation,
                                     const std::string& message) :
    std::runtime_error(message),
    _name("component " + instantiation->getID())
{
}

placement_failure::placement_failure(const ossie::SoftwareAssembly::HostCollocation& collocation,
                                     const std::string& message) :
    std::runtime_error(message),
    _name("host collocation " + collocation.getID() + " (" + collocation.getName() + ")")
{
}
