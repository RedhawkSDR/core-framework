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

#include <ossie/FileStream.h>
#include <ossie/prop_utils.h>
#include <ossie/ComponentDescriptor.h>

#include "PersistenceStore.h"
#include "ApplicationDeployment.h"
#include "ProfileCache.h"

using namespace redhawk;
using namespace ossie;

PREPARE_CF_LOGGING(ApplicationDeployment);

ContainerDeployment::ContainerDeployment(const ossie::SoftPkg* softpkg,
                                         ossie::ComponentInstantiation* instantiation,
                                         const std::string& identifier) :
    ComponentDeployment(softpkg, instantiation, identifier),
    instance(instantiation)
{
}

ApplicationDeployment::ApplicationDeployment(const SoftwareAssembly& sad,
                                             const std::string& instanceName,
                                             const CF::Properties& initConfiguration) :
    sad(sad),
    // Give the application a unique identifier of the form
    // "softwareassemblyid:ApplicationName", where the application name
    // includes the serial number generated for the naming context
    // (e.g. "Application_1").
    identifier(sad.getID() + ":" + instanceName),
    instanceName(instanceName),
    initConfiguration(initConfiguration),
    ac(0)
{
}

ApplicationDeployment::~ApplicationDeployment()
{
    ac=NULL;
    BOOST_FOREACH(ComponentDeployment* component, components) {
        delete component;
    }
    BOOST_FOREACH(ContainerDeployment* container, containers) {
        delete container;
    }
}


const std::string& ApplicationDeployment::getIdentifier() const
{
    return identifier;
}

ComponentDeployment* ApplicationDeployment::getAssemblyController()
{
    BOOST_FOREACH(ComponentDeployment* deployment, components) {
        if (deployment->isAssemblyController()) {
            return deployment;
        }
    }
    return 0;
}

redhawk::PropertyMap ApplicationDeployment::getAllocationContext() const
{
    redhawk::PropertyMap properties;
    BOOST_FOREACH(ComponentDeployment* deployment, components) {
        if (deployment->isAssemblyController()) {
            properties = deployment->getAllocationContext();
        }
    }
    return properties;
}

ComponentDeployment* ApplicationDeployment::createComponentDeployment(const SoftPkg* softpkg,
                                                                      const ComponentInstantiation* instantiation)
{
    // Create a unique identifier for this component instance by appending the
    // application instance's unique name
    std::string component_id = instantiation->getID() + ":" + instanceName;

    if (softpkg->isScaCompliant() && !instantiation->isNamingService()) {
        RH_WARN(_appDeploymentLog, "Component instantiation "
                 << instantiation->getID() << " does not provide a 'findcomponent' name but "
                 << softpkg->getName() << " is SCA-compliant");
    }

    if  ( (instantiation->getID() == sad.getAssemblyControllerRefId() ) && ac ) {
        RH_TRACE(_appDeploymentLog, " Requesting AssemblyController "  << instantiation->getID() );
        return ac;
    }

    ComponentDeployment* deployment = new ComponentDeployment(softpkg, instantiation, component_id);
    components.push_back(deployment);

    // Override properties from initial configuration
    if (instantiation->getID() == sad.getAssemblyControllerRefId()) {
        ac = deployment;
        deployment->setIsAssemblyController(true);
        overrideAssemblyControllerProperties(deployment);
    }
    overrideExternalProperties(deployment);

    overrideImpliedProperties(deployment);

    return deployment;
}

ContainerDeployment* ApplicationDeployment::createContainer(redhawk::ProfileCache& cache,
                                                            const boost::shared_ptr<ossie::DeviceNode>& device)
{
    ContainerDeployment* container = getContainer(device->identifier);
    if (container) {
        RH_DEBUG(_appDeploymentLog, "Using existing container " << container->getIdentifier());
        return container;
    }

    const ossie::SoftPkg* softpkg = cache.loadSoftPkg("/mgr/rh/ComponentHost/ComponentHost.spd.xml");

    // Create an instantiation with the ID and naming service name based on the
    // device label; the deployment will own this object
    ossie::ComponentInstantiation* instantiation = new ossie::ComponentInstantiation;
    instantiation->instantiationId = "ComponentHost_" + device->label;
    instantiation->namingservicename = instantiation->instantiationId;

    // Use the same pattern as components to generate the unique runtime ID
    RH_DEBUG(_appDeploymentLog, "Creating component host " << instantiation->getID());
    std::string container_id = instantiation->getID() + ":" + instanceName;

    container = new ContainerDeployment(softpkg, instantiation, container_id);
    containers.push_back(container);
    return container;
}

const ApplicationDeployment::ComponentList& ApplicationDeployment::getComponentDeployments()
{
    return components;
}

const ApplicationDeployment::ContainerList& ApplicationDeployment::getContainerDeployments()
{
    return containers;
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

ComponentDeployment* ApplicationDeployment::getComponentDeploymentByUniqueId(const std::string& identifier)
{
    BOOST_FOREACH(ComponentDeployment* deployment, components) {
        if (identifier == deployment->getIdentifier()) {
            return deployment;
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
            // the instantiation ID makes more sense. If the usage name does not apply,
            // use the instantiation ID
            reserved = reservations.find(deployment->getInstantiation()->getUsageName());
            if (reserved == reservations.end()) {
                reserved = reservations.find(deployment->getInstantiation()->getID());
            }
        }
        if (reserved != reservations.end()) {
            deployment->setCpuReservation(reserved->second);
        }
    }
}

void ApplicationDeployment::overrideAssemblyControllerProperties(ComponentDeployment* deployment)
{
    BOOST_FOREACH(const redhawk::PropertyType& override, initConfiguration) {
        const std::string propid = override.getId();
        if (propid == "LOGGING_CONFIG_URI") {
            if (deployment->getLoggingConfiguration().empty()) {
                RH_TRACE(_appDeploymentLog, "Adding LOGGING_CONFIG_URI as a command line parameter with value "
                          << override.getValue().toString());
                deployment->overrideProperty(propid, override.getValue());
            }
        } else {
            RH_TRACE(_appDeploymentLog, "Overriding property " << propid
                      << " with " << override.getValue().toString());
            deployment->overrideProperty(propid, override.getValue());
        }
    }
}

void ApplicationDeployment::overrideExternalProperties(ComponentDeployment* deployment)
{
    const std::string& instantiation_id = deployment->getInstantiation()->getID();
    BOOST_FOREACH(const SoftwareAssembly::Property& property, sad.getExternalProperties()) {
        if (property.comprefid == instantiation_id) {
            std::string property_id = property.externalpropid;
            if (property_id.empty()) {
                property_id = property.propid;
            }
            redhawk::PropertyMap::iterator override = initConfiguration.find(property_id);
            if (override != initConfiguration.end()) {
                RH_TRACE(_appDeploymentLog, "Overriding external property " << property_id
                          << " (" << property.propid << ") = " << override->getValue().toString());
                deployment->overrideProperty(property.propid, override->getValue());
            }
        }
    }
}

void ApplicationDeployment::overrideImpliedProperties(ComponentDeployment* deployment) {
    BOOST_FOREACH(const redhawk::PropertyType& override, initConfiguration) {
        const std::string propid = override.getId();
        if (propid == "LOGGING_CONFIG_URI") {
            deployment->overrideProperty(propid, override.getValue());
        }
    }
}

ContainerDeployment* ApplicationDeployment::getContainer(const std::string& deviceId)
{
    BOOST_FOREACH(ContainerDeployment* container, containers) {
        if (container->getAssignedDevice() && container->getAssignedDevice()->identifier == deviceId) {
            return container;
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
    RH_TRACE(_appDeploymentLog, "[DeviceLookup] Lookup device that loaded component " << componentId);

    ComponentDeployment* deployment = getComponentDeployment(componentId);
    if (!deployment) {
        throw ossie::LookupError("component '" + componentId + "' not found");
    }

    boost::shared_ptr<ossie::DeviceNode> device = deployment->getAssignedDevice();
    if (!device) {
        throw ossie::LookupError("component '" + componentId + "' is not assigned to a device");
    }

    RH_TRACE(_appDeploymentLog, "[DeviceLookup] Assigned device id " << device->identifier);
    return CF::Device::_duplicate(device->device);
}

CF::Device_ptr ApplicationDeployment::lookupDeviceUsedByComponentInstantiationId(const std::string& componentId,
                                                                                 const std::string& usesId)
{
    RH_TRACE(_appDeploymentLog, "[DeviceLookup] Lookup device used by component " << componentId);

    ComponentDeployment* deployment = getComponentDeployment(componentId);
    if (!deployment) {
        throw ossie::LookupError("component '" + componentId + "' not found");
    }

    UsesDeviceAssignment* uses = deployment->getUsesDeviceAssignment(usesId);
    if (!uses) {
        throw ossie::LookupError("component '" + componentId + "' has no usesdevice '" + usesId + "'");
    }

    CF::Device_var device = uses->getAssignedDevice();
    RH_TRACE(_appDeploymentLog, "[DeviceLookup] Assigned device id "
              << ossie::corba::returnString(device->identifier()));
    return device._retn();
}

CF::Device_ptr ApplicationDeployment::lookupDeviceUsedByApplication(const std::string& usesRefId)
{
    RH_TRACE(_appDeploymentLog, "[DeviceLookup] Lookup device used by application, Uses Id: " << usesRefId);

    UsesDeviceAssignment* uses = getUsesDeviceAssignment(usesRefId);
    if (!uses) {
        throw ossie::LookupError("application has no usesdevice '" + usesRefId + "'");
    }

    CF::Device_var device = uses->getAssignedDevice();
    RH_TRACE(_appDeploymentLog, "[DeviceLookup] Assigned device id "
              << ossie::corba::returnString(device->identifier()));
    return device._retn();
}
