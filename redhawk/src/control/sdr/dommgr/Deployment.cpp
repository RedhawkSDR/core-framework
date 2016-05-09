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

#include "Deployment.h"

using namespace ossie;

SoftpkgDeployment::SoftpkgDeployment(SoftpkgInfo* softpkg, ImplementationInfo* impl) :
    softpkg(softpkg),
    impl(impl)
{
}

SoftpkgDeployment::~SoftpkgDeployment()
{
    for (std::vector<SoftpkgDeployment*>::iterator dependency = dependencies.begin(); dependency != dependencies.end(); ++dependency) {
        delete (*dependency);
    }
}

SoftpkgInfo* SoftpkgDeployment::getSoftpkg()
{
    return softpkg;
}

ImplementationInfo* SoftpkgDeployment::getImplementation()
{
    return impl;
}

void SoftpkgDeployment::addDependency(SoftpkgDeployment* dependency)
{
    dependencies.push_back(dependency);
}

const std::vector<SoftpkgDeployment*>& SoftpkgDeployment::getDependencies()
{
    return dependencies;
}

ComponentDeployment::ComponentDeployment(ComponentInfo* component, ImplementationInfo* impl,
                                         const boost::shared_ptr<DeviceNode>& device) :
    SoftpkgDeployment(component, impl),
    assignedDevice(device)
{
}

ComponentInfo* ComponentDeployment::getComponent()
{
    return dynamic_cast<ComponentInfo*>(getSoftpkg());
}

boost::shared_ptr<DeviceNode> ComponentDeployment::getAssignedDevice()
{
    return assignedDevice;
}

