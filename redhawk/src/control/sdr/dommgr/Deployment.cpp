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

#include <boost/filesystem.hpp>

#include "Deployment.h"

using namespace ossie;
namespace fs = boost::filesystem;

SoftpkgDeployment::SoftpkgDeployment(SoftpkgInfo* softpkg, ImplementationInfo* implementation) :
    softpkg(softpkg),
    implementation(implementation)
{
}

SoftpkgDeployment::~SoftpkgDeployment()
{
    for (DeploymentList::iterator dependency = dependencies.begin(); dependency != dependencies.end(); ++dependency) {
        delete (*dependency);
    }
}

SoftpkgInfo* SoftpkgDeployment::getSoftpkg()
{
    return softpkg;
}

ImplementationInfo* SoftpkgDeployment::getImplementation()
{
    return implementation;
}

void SoftpkgDeployment::addDependency(SoftpkgDeployment* dependency)
{
    dependencies.push_back(dependency);
}

const std::vector<SoftpkgDeployment*>& SoftpkgDeployment::getDependencies()
{
    return dependencies;
}

std::vector<std::string> SoftpkgDeployment::getDependencyLocalFiles()
{
    std::vector<std::string> files;
    for (DeploymentList::iterator dependency = dependencies.begin(); dependency != dependencies.end(); ++dependency) {
        std::vector<std::string> depfiles = (*dependency)->getDependencyLocalFiles();
        std::copy(depfiles.begin(), depfiles.end(), std::back_inserter(files));
        files.push_back((*dependency)->getLocalFile());
    }
    return files;
}

std::string SoftpkgDeployment::getLocalFile()
{
    fs::path codeLocalFile = fs::path(implementation->getLocalFileName());
    if (!codeLocalFile.has_root_directory()) {
        // Path is relative to SPD file location
        fs::path base_dir = fs::path(softpkg->getSpdFileName()).parent_path();
        codeLocalFile = base_dir / codeLocalFile;
    }
    codeLocalFile = codeLocalFile.normalize();
    if (codeLocalFile.has_leaf() && codeLocalFile.leaf() == ".") {
        codeLocalFile = codeLocalFile.branch_path();
    }

    return codeLocalFile.string();
}

ComponentDeployment::ComponentDeployment(ComponentInfo* component, ImplementationInfo* implementation,
                                         const boost::shared_ptr<DeviceNode>& device) :
    SoftpkgDeployment(component, implementation),
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

std::string ComponentDeployment::getEntryPoint()
{
    std::string entryPoint = implementation->getEntryPoint();
    if (!entryPoint.empty()) {
        fs::path entryPointPath = fs::path(entryPoint);
        if (!entryPointPath.has_root_directory()) {
            // Path is relative to SPD file location
            fs::path base_dir = fs::path(softpkg->getSpdFileName()).parent_path();
            entryPointPath = base_dir / entryPointPath;
        }
        entryPoint = entryPointPath.normalize().string();
    }
    return entryPoint;
}

redhawk::PropertyMap ComponentDeployment::getOptions()
{
    // Get the options from the softpkg
    redhawk::PropertyMap options(getComponent()->getOptions());

    // Get the PRIORITY and STACK_SIZE from the SPD (if available)
    if (implementation->hasStackSize()) {
        // 3.1.3.3.3.3.6
        // The specification says it's supposed to be an unsigned long, but the
        // parser is set to unsigned long long
        options["STACK_SIZE"] = implementation->getStackSize();
    }
    if (implementation->hasPriority()) {
        // 3.1.3.3.3.3.7
        // The specification says it's supposed to be an unsigned long, but the
        // parser is set to unsigned long long
        options["PRIORITY"] = implementation->getPriority();
    }

    return options;
}

const UsesDeviceInfo* ComponentDeployment::getUsesDeviceById(const std::string& usesId)
{
    const UsesDeviceInfo* usesDevice = getComponent()->getUsesDeviceById(usesId);
    if (!usesDevice) {
        usesDevice = implementation->getUsesDeviceById(usesId);
    }
    return usesDevice;
}
