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

#ifndef DEPLOYMENT_H
#define DEPLOYMENT_H

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "applicationSupport.h"

namespace ossie {
    class SoftpkgDeployment
    {
    public:
        typedef std::vector<SoftpkgDeployment*> DeploymentList;

        SoftpkgDeployment(SoftpkgInfo* softpkg, ImplementationInfo* impl);
        ~SoftpkgDeployment();

        SoftpkgInfo* getSoftpkg();
        ImplementationInfo* getImplementation();

        std::string getLocalFile();

        void addDependency(SoftpkgDeployment* dependency);
        const DeploymentList& getDependencies();

        std::vector<std::string> getDependencyLocalFiles();

    protected:
        SoftpkgInfo* softpkg;
        ImplementationInfo* impl;
        DeploymentList dependencies;
    };

    class ComponentDeployment : public SoftpkgDeployment
    {
    public:
        ComponentDeployment(ComponentInfo* component, ImplementationInfo* impl,
                            const boost::shared_ptr<DeviceNode>& device);

        ComponentInfo* getComponent();

        boost::shared_ptr<DeviceNode> getAssignedDevice();

    protected:
        boost::shared_ptr<DeviceNode> assignedDevice;
    };
}

#endif // DEPLOYMENT_H
