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

#include "Application_impl.h"
#include "PersistenceStore.h"
#include "Deployment.h"

using namespace ossie;
namespace fs = boost::filesystem;

UsesDeviceDeployment::~UsesDeviceDeployment()
{
    for (AssignmentList::iterator assign = assignments.begin(); assign != assignments.end(); ++assign) {
        delete *assign;
    }
}

void UsesDeviceDeployment::transferUsesDeviceAssignments(UsesDeviceDeployment& other)
{
    other.assignments.insert(other.assignments.end(), assignments.begin(), assignments.end());
    assignments.clear();
}

void UsesDeviceDeployment::addUsesDeviceAssignment(UsesDeviceAssignment* assignment)
{
    assignments.push_back(assignment);
}

UsesDeviceAssignment* UsesDeviceDeployment::getUsesDeviceAssignment(const std::string identifier)
{
    for (AssignmentList::iterator assign = assignments.begin(); assign != assignments.end(); ++assign) {
        if (identifier == (*assign)->getUsesDevice()->getId()) {
            return *assign;
        }
    }

    return 0;
}

const UsesDeviceDeployment::AssignmentList& UsesDeviceDeployment::getUsesDeviceAssignments()
{
    return assignments;
}

UsesDeviceAssignment::UsesDeviceAssignment(UsesDeviceInfo* usesDevice) :
    usesDevice(usesDevice)
{
}

UsesDeviceInfo* UsesDeviceAssignment::getUsesDevice()
{
    return usesDevice;
}

void UsesDeviceAssignment::setAssignedDevice(CF::Device_ptr device)
{
    assignedDevice = CF::Device::_duplicate(device);
}

CF::Device_ptr UsesDeviceAssignment::getAssignedDevice() const
{
    return CF::Device::_duplicate(assignedDevice);
}

SoftpkgDeployment::SoftpkgDeployment(SoftpkgInfo* softpkg, const ImplementationInfo* implementation) :
    softpkg(softpkg),
    implementation(implementation)
{
}

SoftpkgDeployment::~SoftpkgDeployment()
{
    clearDependencies();
}

SoftpkgInfo* SoftpkgDeployment::getSoftpkg()
{
    return softpkg;
}

const SoftPkg* SoftpkgDeployment::getSPD() const
{
    return &(softpkg->spd);
}

void SoftpkgDeployment::setImplementation(const ImplementationInfo* implementation)
{
    this->implementation = implementation;
}

const ImplementationInfo* SoftpkgDeployment::getImplementation() const
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

void SoftpkgDeployment::clearDependencies()
{
    for (DeploymentList::iterator dependency = dependencies.begin(); dependency != dependencies.end(); ++dependency) {
        delete (*dependency);
    }
    dependencies.clear();
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

void SoftpkgDeployment::load(Application_impl* application, CF::FileSystem_ptr fileSystem,
                             CF::LoadableDevice_ptr device, const std::string& componentId)
{
    if (!implementation) {
        throw std::logic_error("no implementation selected for soft package " + softpkg->getName());
    }

    // Recursively load dependencies
    if (!dependencies.empty()) {
        RH_NL_TRACE("ApplicationFactory_impl", "Loading " << dependencies.size() <<
                    "dependency(ies) for soft package " << softpkg->getName());
        for (DeploymentList::iterator dep = dependencies.begin(); dep != dependencies.end(); ++dep) {
            (*dep)->load(application, fileSystem, device, componentId);
        }
    }

    // Determine absolute path of local file
    CF::LoadableDevice::LoadType codeType = implementation->getCodeType();
    const std::string fileName = getLocalFile();
    RH_NL_DEBUG("ApplicationFactory_impl", "Loading file " << fileName
                << " for soft package " << softpkg->getName());
    try {
        device->load(fileSystem, fileName.c_str(), codeType);
    } catch (const CF::Device::InvalidState& exc) {
        std::string message = "device is in invalid state: ";
        message += exc.msg;
        throw std::runtime_error(message);
    } catch (const CF::LoadableDevice::InvalidLoadKind& exc) {
        throw std::runtime_error("invalid load kind for file " + fileName);
    } catch (const CF::InvalidFileName& exc) {
        std::string message = "file name '" + fileName + "' is invalid: ";
        message += exc.msg;
        throw std::runtime_error(message);
    } catch (const CF::LoadableDevice::LoadFail& exc) {
        std::string message = "failure loading file '" + fileName + "': ";
        message += exc.msg;
        throw std::runtime_error(message);
    } catch (const CORBA::SystemException& exc) {
        std::string message = "CORBA system exception ";
        message += exc._name();
        message += " loading " + fileName;
        throw std::runtime_error(message);
    }
    application->addComponentLoadedFile(componentId, fileName);
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

ComponentDeployment::ComponentDeployment(ComponentInfo* component,
                                         const ComponentInstantiation* instantiation,
                                         const std::string& identifier) :
    SoftpkgDeployment(component),
    component(component),
    instantiation(instantiation),
    identifier(identifier),
    affinityOptions(component->getAffinityOptions())
{
}

ComponentInfo* ComponentDeployment::getComponent()
{
    return component;
}

const std::string& ComponentDeployment::getIdentifier() const
{
    return identifier;
}

const ComponentInstantiation* ComponentDeployment::getInstantiation() const
{
    return instantiation;
}

void ComponentDeployment::setAssignedDevice(const boost::shared_ptr<DeviceNode>& device)
{
    assignedDevice = device;
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
    redhawk::PropertyMap options(component->getOptions());

    // Get the PRIORITY and STACK_SIZE from the SPD (if available)
    if (implementation->hasStackSize()) {
        // 3.1.3.3.3.3.6
        // The specification says it's supposed to be an unsigned long, but the
        // parser is set to unsigned long long
        options[CF::ExecutableDevice::STACK_SIZE_ID] = implementation->getStackSize();
    }
    if (implementation->hasPriority()) {
        // 3.1.3.3.3.3.7
        // The specification says it's supposed to be an unsigned long, but the
        // parser is set to unsigned long long
        options[CF::ExecutableDevice::PRIORITY_ID] = implementation->getPriority();
    }

    redhawk::PropertyMap affinity = affinityOptions;
    for (redhawk::PropertyMap::const_iterator prop = affinity.begin(); prop != affinity.end(); ++prop) {
        RH_NL_DEBUG("DomainManager", "ComponentDeployment - Affinity Property: directive id:"
                    <<  prop->getId() << "/" <<  prop->getValue().toString());
    }
    if (!nicAssignment.empty()) {
        redhawk::PropertyMap::iterator nic_prop = affinity.find("nic");
        if (nic_prop == affinity.end() || (nic_prop->getId() != nicAssignment)) {
            // No nic directive, or existing directive differs, append this one
            affinity.push_back(redhawk::PropertyType("nic", nicAssignment));
        }
    }

    if (!affinity.empty()) {
        options["AFFINITY"] = affinity;
    }

    return options;
}

void ComponentDeployment::setNicAssignment(const std::string& nic)
{
    nicAssignment = nic;
}

bool ComponentDeployment::hasNicAssignment() const
{
    return !nicAssignment.empty();
}

const std::string& ComponentDeployment::getNicAssignment() const
{
    return nicAssignment;
}

void ComponentDeployment::setCpuReservation(float reservation)
{
    cpuReservation = reservation;
}

bool ComponentDeployment::hasCpuReservation() const
{
    return cpuReservation.isSet();
}

float ComponentDeployment::getCpuReservation() const
{
    return *cpuReservation;
}

CF::Properties ComponentDeployment::getConfigureProperties() const
{
    return component->getConfigureProperties();
}

CF::Properties ComponentDeployment::getConstructProperties() const
{
    return component->getConstructProperties();
}

redhawk::PropertyMap ComponentDeployment::getAffinityOptionsWithAssignment() const
{
    redhawk::PropertyMap options = affinityOptions;
    for (redhawk::PropertyMap::const_iterator prop = options.begin(); prop != options.end(); ++prop) {
        RH_NL_DEBUG("DomainManager", "ComponentDeployment getAffinityOptionsWithAssignment ... Affinity Property: directive id:"  <<  prop->getId() << "/" <<  prop->getValue().toString());
    }

    if (!nicAssignment.empty()) {
       RH_NL_DEBUG("DomainManager", "ComponentDeployment getAffinityOptionsWithAssignment ... NIC AFFINITY: pol/value "  <<  "nic"  << "/" << nicAssignment);
       options.push_back(redhawk::PropertyType("nic", nicAssignment));
    }

    return options;
}

void ComponentDeployment::mergeAffinityOptions(const CF::Properties& properties)
{
    // Update existing settings with new ones
    affinityOptions.update(properties);
}

const UsesDeviceInfo* ComponentDeployment::getUsesDeviceById(const std::string& usesId)
{
    const UsesDeviceInfo* usesDevice = component->getUsesDeviceById(usesId);
    if (!usesDevice) {
        usesDevice = implementation->getUsesDeviceById(usesId);
    }
    return usesDevice;
}

void ComponentDeployment::setResourcePtr(CF::Resource_ptr resource)
{
    this->resource = CF::Resource::_duplicate(resource);
}

CF::Resource_ptr ComponentDeployment::getResourcePtr() const
{
    return CF::Resource::_duplicate(resource);
}

void ComponentDeployment::load(Application_impl* application, CF::FileSystem_ptr fileSystem,
                               CF::LoadableDevice_ptr device)
{
    SoftpkgDeployment::load(application, fileSystem, device, identifier);
}


PlacementPlan::PlacementPlan()
{
}

PlacementPlan::~PlacementPlan()
{
    for (ComponentList::iterator comp = components.begin(); comp != components.end(); ++comp) {
        delete *comp;
    }
}

PlacementPlan::PlacementPlan(const std::string& id, const std::string& name) :
    id(id),
    name(name)
{
}

const std::string& PlacementPlan::getId() const
{
    return id;
}

const std::string& PlacementPlan::getName() const
{
    return name;
}

const PlacementPlan::ComponentList& PlacementPlan::getComponents() const
{
    return components;
}

void PlacementPlan::addComponent(ComponentInfo* component)
{
    components.push_back(component);
}

ComponentInfo* PlacementPlan::getComponent(const std::string& instantiationId)
{
    for (ComponentList::iterator comp = components.begin(); comp != components.end(); ++comp) {
        if (instantiationId == (*comp)->getInstantiationIdentifier()) {
            return *comp;
        }
    }

    return 0;
}


ApplicationDeployment::ApplicationDeployment(const SoftwareAssembly& sad, const std::string& instanceName) :
    // Give the application a unique identifier of the form
    // "softwareassemblyid:ApplicationName", where the application name
    // includes the serial number generated for the naming context
    // (e.g. "Application_1").
    identifier(sad.getID() + ":" + instanceName),
    instanceName(instanceName)
{
}

ApplicationDeployment::~ApplicationDeployment()
{
    for (ComponentList::iterator comp = components.begin(); comp != components.end(); ++comp) {
        delete *comp;
    }
    for (std::vector<PlacementPlan*>::iterator place = placements.begin(); place != placements.end(); ++place) {
        delete (*place);
    }
}

const std::string& ApplicationDeployment::getIdentifier() const
{
    return identifier;
}

void ApplicationDeployment::addPlacement(PlacementPlan* placement)
{
    placements.push_back(placement);
}

const std::vector<PlacementPlan*>& ApplicationDeployment::getPlacements() const
{
    return placements;
}

ComponentInfo* ApplicationDeployment::getComponent(const std::string& instantiationId)
{
    for (PlacementList::iterator placement = placements.begin(); placement != placements.end(); ++placement) {
        ComponentInfo* component = (*placement)->getComponent(instantiationId);
        if (component) {
            return component;
        }
    }

    return 0;
}

ComponentInfo* ApplicationDeployment::getAssemblyController()
{
    for (PlacementList::iterator placement = placements.begin(); placement != placements.end(); ++placement) {
        const std::vector<ComponentInfo*>& components = (*placement)->getComponents();
        for (std::vector<ComponentInfo*>::const_iterator comp = components.begin(); comp != components.end(); ++comp) {
            if ((*comp)->isAssemblyController()) {
                return *comp;
            }
        }
    }

    return 0;
}

ComponentDeployment* ApplicationDeployment::createComponentDeployment(ComponentInfo* component)
{
    // Create a unique identifier for this component instance by appending the
    // application instance's unique name
    const ComponentInstantiation* instantiation = component->getInstantiation();
    std::string component_id = instantiation->getID() + ":" + instanceName;

    ComponentDeployment* deployment = new ComponentDeployment(component, instantiation, component_id);
    components.push_back(deployment);

    return deployment;
}

const ApplicationDeployment::ComponentList& ApplicationDeployment::getComponentDeployments()
{
    return components;
}

ComponentDeployment* ApplicationDeployment::getComponentDeployment(const std::string& instantiationId)
{
    for (ComponentList::iterator comp = components.begin(); comp != components.end(); ++comp) {
        if (instantiationId == (*comp)->getInstantiation()->getID()) {
            return *comp;
        }
    }

    return 0;
}

CF::Resource_ptr ApplicationDeployment::lookupComponentByInstantiationId(const std::string& identifier)
{
    ComponentDeployment* deployment = getComponentDeployment(identifier);
    if (deployment) {
        return deployment->getResourcePtr();
    }
    return CF::Resource::_nil();
}

CF::Device_ptr ApplicationDeployment::lookupDeviceThatLoadedComponentInstantiationId(const std::string& componentId)
{
    RH_NL_TRACE("ApplicationFactory_impl", "[DeviceLookup] Lookup device that loaded component " << componentId);

    ComponentDeployment* deployment = getComponentDeployment(componentId);
    if (!deployment) {
        RH_NL_WARN("ApplicationFactory_impl", "[DeviceLookup] Component not found");
        return CF::Device::_nil();
    }

    boost::shared_ptr<ossie::DeviceNode> device = deployment->getAssignedDevice();
    if (!device) {
        RH_NL_WARN("ApplicationFactory_impl", "[DeviceLookup] Component not assigned to device");
        return CF::Device::_nil();
    }

    RH_NL_TRACE("ApplicationFactory_impl", "[DeviceLookup] Assigned device id " << device->identifier);
    return CF::Device::_duplicate(device->device);
}

CF::Device_ptr ApplicationDeployment::lookupDeviceUsedByComponentInstantiationId(const std::string& componentId,
                                                                                 const std::string& usesId)
{
    RH_NL_TRACE("ApplicationFactory_impl", "[DeviceLookup] Lookup device used by component " << componentId);

    ComponentDeployment* deployment = getComponentDeployment(componentId);
    if (!deployment) {
        RH_NL_WARN("ApplicationFactory_impl", "[DeviceLookup] Component not found");
        return CF::Device::_nil();
    }

    UsesDeviceAssignment* uses = deployment->getUsesDeviceAssignment(usesId);
    if (!uses) {
        RH_NL_WARN("ApplicationFactory_impl", "[DeviceLookup] UsesDevice not found");
        return CF::Device::_nil();
    }

    //RH_NL_TRACE("ApplicationFactory_impl", "[DeviceLookup] Assigned device id " << deviceId);
    return uses->getAssignedDevice();
}

CF::Device_ptr ApplicationDeployment::lookupDeviceUsedByApplication(const std::string& usesRefId)
{
    RH_NL_TRACE("ApplicationFactory_impl", "[DeviceLookup] Lookup device used by application, Uses Id: " << usesRefId);

    UsesDeviceAssignment* uses = getUsesDeviceAssignment(usesRefId);
    if (!uses) {
        RH_NL_WARN("ApplicationFactory_impl", "[DeviceLookup] UsesDevice not found");
        return CF::Device::_nil();
    }

    //RH_NL_TRACE("ApplicationFactory_impl", "[DeviceLookup] Assigned device id " << deviceId);
    return uses->getAssignedDevice();
}
