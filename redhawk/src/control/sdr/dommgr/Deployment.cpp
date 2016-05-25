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
#include <boost/foreach.hpp>

#include <ossie/prop_utils.h>

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
        if (identifier == (*assign)->getUsesDevice()->getID()) {
            return *assign;
        }
    }

    return 0;
}

const UsesDeviceDeployment::AssignmentList& UsesDeviceDeployment::getUsesDeviceAssignments()
{
    return assignments;
}

UsesDeviceAssignment::UsesDeviceAssignment(const UsesDevice* usesDevice) :
    usesDevice(usesDevice)
{
}

const UsesDevice* UsesDeviceAssignment::getUsesDevice() const
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

SoftpkgDeployment::SoftpkgDeployment(const SoftPkg* softpkg,
                                     const SPD::Implementation* implementation) :
    softpkg(softpkg),
    implementation(implementation)
{
}

SoftpkgDeployment::~SoftpkgDeployment()
{
    clearDependencies();
}

const SoftPkg* SoftpkgDeployment::getSoftPkg() const
{
    return &(*softpkg);
}

void SoftpkgDeployment::setImplementation(const SPD::Implementation* implementation)
{
    this->implementation = implementation;
}

const SPD::Implementation* SoftpkgDeployment::getImplementation() const
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
                    " dependency(ies) for soft package " << softpkg->getName());
        for (DeploymentList::iterator dep = dependencies.begin(); dep != dependencies.end(); ++dep) {
            (*dep)->load(application, fileSystem, device, componentId);
        }
    }

    // Determine absolute path of local file
    CF::LoadableDevice::LoadType codeType = getCodeType();
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
    fs::path codeLocalFile = fs::path(implementation->getCodeFile());
    if (!codeLocalFile.has_root_directory()) {
        // Path is relative to SPD file location
        fs::path base_dir = fs::path(softpkg->getSPDFile()).parent_path();
        codeLocalFile = base_dir / codeLocalFile;
    }
    codeLocalFile = codeLocalFile.normalize();
    if (codeLocalFile.has_leaf() && codeLocalFile.leaf() == ".") {
        codeLocalFile = codeLocalFile.branch_path();
    }

    return codeLocalFile.string();
}

CF::LoadableDevice::LoadType SoftpkgDeployment::getCodeType() const
{
    const std::string type = implementation->getCodeType();
    if (type == "KernelModule") {
        return CF::LoadableDevice::KERNEL_MODULE;
    } else if (type == "SharedLibrary") {
        return CF::LoadableDevice::SHARED_LIBRARY;
    } else if (type == "Executable") {
        return CF::LoadableDevice::EXECUTABLE;
    } else if (type == "Driver") {
        return CF::LoadableDevice::DRIVER;
    } else {
        return CF::LoadableDevice::LoadType();
    }
}

ComponentDeployment::ComponentDeployment(ComponentInfo* component,
                                         const ComponentInstantiation* instantiation,
                                         const std::string& identifier) :
    SoftpkgDeployment(component->spd),
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

bool ComponentDeployment::isResource() const
{
    return softpkg->getDescriptor()->isResource();
}

bool ComponentDeployment::isConfigurable() const
{
    return softpkg->getDescriptor()->isConfigurable();
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
    const char* entryPoint = implementation->getEntryPoint();
    if (entryPoint) {
        fs::path entryPointPath = fs::path(entryPoint);
        if (!entryPointPath.has_root_directory()) {
            // Path is relative to SPD file location
            fs::path base_dir = fs::path(softpkg->getSPDFile()).parent_path();
            entryPointPath = base_dir / entryPointPath;
        }
        return entryPointPath.normalize().string();
    }
    return std::string();
}

redhawk::PropertyMap ComponentDeployment::getOptions()
{
    // In prior versions, the options could be overridden, at least from the
    // perspective of ComponentInfo; this may need to be re-implemented
    redhawk::PropertyMap options;

    // Get the PRIORITY and STACK_SIZE from the SPD (if available)
    const ossie::SPD::Code& code = implementation->code;
    if (code.stacksize.isSet()) {
        // 3.1.3.3.3.3.6
        // The specification says it's supposed to be an unsigned long, but the
        // parser is set to unsigned long long
        options[CF::ExecutableDevice::STACK_SIZE_ID] = static_cast<CORBA::ULong>(*code.stacksize);
    }
    if (code.priority.isSet()) {
        // 3.1.3.3.3.3.7
        // The specification says it's supposed to be an unsigned long, but the
        // parser is set to unsigned long long
        options[CF::ExecutableDevice::PRIORITY_ID] = static_cast<CORBA::ULong>(*code.priority);
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

redhawk::PropertyMap ComponentDeployment::getAllocationContext() const
{
    redhawk::PropertyMap properties(component->getConfigureProperties());
    ossie::corba::extend(properties, component->getConstructProperties());
    return properties;
}

redhawk::PropertyMap ComponentDeployment::getInitialConfigureProperties() const
{
   redhawk::PropertyMap properties;
   if (softpkg->getProperties()) {
       BOOST_FOREACH(const Property* property, softpkg->getProperties()->getProperties()) {
           if (property->isConfigure() && !property->isReadOnly()) {
               CF::DataType dt = getPropertyValue(property);
               if (!ossie::any::isNull(dt.value)) {
                   properties.push_back(dt);
               }
           }
       }
    }
    return properties;
}

redhawk::PropertyMap ComponentDeployment::getInitializeProperties() const
{
    redhawk::PropertyMap properties;
    if (softpkg->getProperties()) {
        BOOST_FOREACH(const Property* property, softpkg->getProperties()->getProperties()) {
            if (property->isProperty() && !property->isCommandLine()) {
                CF::DataType dt = getPropertyValue(property);
                if (!ossie::any::isNull(dt.value)) {
                    properties.push_back(dt);
                }
            }
        }
    }
    return properties;
}

CF::DataType ComponentDeployment::getPropertyValue(const Property* property) const
{
    const ComponentProperty* override = getPropertyOverride(property->getID());
    if (override) {
        return ossie::overridePropertyValue(property, override);
    } else {
        return ossie::convertPropertyToDataType(property);
    }
}

const ComponentProperty* ComponentDeployment::getPropertyOverride(const std::string& id) const
{
    BOOST_FOREACH(const ComponentProperty& override, instantiation->getProperties()) {
        if (override.getID() == id) {
            return &override;
        }
    }
    return 0;
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
        if (instantiationId == (*comp)->getInstantiation()->getID()) {
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

ComponentInfo* ApplicationDeployment::getAssemblyController() const
{
    for (PlacementList::const_iterator placement = placements.begin(); placement != placements.end(); ++placement) {
        const std::vector<ComponentInfo*>& components = (*placement)->getComponents();
        for (std::vector<ComponentInfo*>::const_iterator comp = components.begin(); comp != components.end(); ++comp) {
            if ((*comp)->getInstantiation()->isAssemblyController()) {
                return *comp;
            }
        }
    }

    return 0;
}

redhawk::PropertyMap ApplicationDeployment::getAllocationContext() const
{
    redhawk::PropertyMap properties;
    const ossie::ComponentInfo* assembly_controller = getAssemblyController();
    if (assembly_controller) {
        properties = assembly_controller->getConfigureProperties();
    }
    return properties;
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

void ApplicationDeployment::applyCpuReservations(const CpuReservations& reservations)
{
    BOOST_FOREACH(ComponentDeployment* deployment, components) {
        CpuReservations::const_iterator reserved = reservations.find(deployment->getIdentifier());
        if (reserved == reservations.end()) {
            // NB: Check for the usage name for consistency with 2.0, although
            // the instantiation ID would make more sense. In most cases, this
            // is probably a moot point, since the IDE uses the same value for
            // both.
            reserved = reservations.find(deployment->getInstantiation()->getUsageName());
        }
        if (reserved != reservations.end()) {
            deployment->setCpuReservation(reserved->second);
        }
    }
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
