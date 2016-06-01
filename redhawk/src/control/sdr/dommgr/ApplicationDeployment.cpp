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

using namespace ossie;

PREPARE_LOGGING(ApplicationDeployment);

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
    initConfiguration(initConfiguration)
{
}

ApplicationDeployment::~ApplicationDeployment()
{
    for (ComponentList::iterator comp = components.begin(); comp != components.end(); ++comp) {
        delete *comp;
    }
}

const std::string& ApplicationDeployment::getIdentifier() const
{
    return identifier;
}

void ApplicationDeployment::loadProfiles(CF::FileSystem_ptr fileSystem)
{
    // Walk through the host collocations first
    BOOST_FOREACH(const SoftwareAssembly::HostCollocation& collocation, sad.getHostCollocations()) {
        BOOST_FOREACH(const ComponentPlacement& placement, collocation.getComponents()) {
            loadComponentProfile(fileSystem, placement);
        }
    }

    // Then, walk through the remaining non-collocated components
    BOOST_FOREACH(const ComponentPlacement& placement, sad.getComponentPlacements()) {
        loadComponentProfile(fileSystem, placement);
    }
}

ComponentDeployment* ApplicationDeployment::getAssemblyController()
{
    BOOST_FOREACH(ComponentDeployment* deployment, components) {
        if (deployment->getInstantiation()->isAssemblyController()) {
            return deployment;
        }
    }
    return 0;
}

redhawk::PropertyMap ApplicationDeployment::getAllocationContext() const
{
    redhawk::PropertyMap properties;
    BOOST_FOREACH(ComponentDeployment* deployment, components) {
        if (deployment->getInstantiation()->isAssemblyController()) {
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

    ComponentDeployment* deployment = new ComponentDeployment(softpkg, instantiation, component_id);
    components.push_back(deployment);

    // Override properties from initial configuration
    if (instantiation->isAssemblyController()) {
        overrideAssemblyControllerProperties(deployment);
    } else {
        overrideExternalProperties(deployment);
    }

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

void ApplicationDeployment::overrideAssemblyControllerProperties(ComponentDeployment* deployment)
{
    BOOST_FOREACH(const redhawk::PropertyType& override, initConfiguration) {
        const std::string propid = override.getId();
        if (propid == "LOGGING_CONFIG_URI") {
            if (deployment->getLoggingConfiguration().empty()) {
                LOG_TRACE(ApplicationDeployment, "Adding LOGGING_CONFIG_URI as a command line parameter with value "
                          << override.getValue().toString());
                deployment->overrideProperty(propid, override.getValue());
            }
        } else {
            LOG_TRACE(ApplicationDeployment, "Overriding property " << propid
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
                LOG_TRACE(ApplicationDeployment, "Overriding external property " << property_id
                          << " (" << property.propid << ") = " << override->getValue().toString());
                deployment->overrideProperty(property.propid, override->getValue());
            }
        }
    }
}


const SoftPkg* ApplicationDeployment::getSoftPkg(const std::string& filename) const
{
    BOOST_FOREACH(const SoftPkg& softpkg, profiles) {
        if (softpkg.getSPDFile() == filename) {
            return &softpkg;
        }
    }

    throw std::logic_error(filename + " was never loaded");
}

void ApplicationDeployment::loadComponentProfile(CF::FileSystem_ptr fileSystem,
                                                 const ComponentPlacement& placement)
{
    SoftPkg* softpkg = loadProfile(fileSystem, placement.filename);
    if (softpkg->isScaCompliant()){ 
        BOOST_FOREACH(const ComponentInstantiation& instantiation, placement.getInstantiations()) {
            if (!instantiation.isNamingService()) {
                LOG_WARN(ApplicationDeployment, "Component instantiation "
                         << instantiation.getID() << " does not provide a 'findcomponent' name but "
                         << softpkg->getName() << " is SCA-compliant");
            }
        }
    }

    if (softpkg->getPRFFile()) {
        LOG_TRACE(ApplicationDeployment, "Loading PRF file " << softpkg->getPRFFile());
        try {
            File_stream prf_stream(fileSystem, softpkg->getPRFFile());
            softpkg->loadProperties(prf_stream);
        } catch (const std::exception& exc) {
            LOG_ERROR(ApplicationDeployment, "Invalid PRF file " << softpkg->getPRFFile() << ": " << exc.what());
        }
    }

    if (softpkg->getSCDFile()) {
        LOG_TRACE(ApplicationDeployment, "Loading SCD file " << softpkg->getSCDFile());
        try {
            File_stream scd_stream(fileSystem, softpkg->getSCDFile());
            softpkg->loadDescriptor(scd_stream);
        } catch (const std::exception& exc) {
            LOG_ERROR(ApplicationDeployment, "Invalid SCD file " << softpkg->getSCDFile() << ": " << exc.what());
        }
    }
}

SoftPkg* ApplicationDeployment::loadProfile(CF::FileSystem_ptr fileSystem,
                                            const std::string& filename)
{
    BOOST_FOREACH(SoftPkg& profile, profiles) {
        if (profile.getSPDFile() == filename) {
            LOG_TRACE(ApplicationDeployment, "Found existing profile " << filename);
            return &profile;
        }
    }

    LOG_TRACE(ApplicationDeployment, "Loading SPD file " << filename);
    File_stream spd_stream(fileSystem, filename.c_str());
    SoftPkg* spd = new SoftPkg(spd_stream, filename);
    profiles.push_back(spd);
    spd_stream.close();

    const SPD::Implementations& spd_impls = spd->getImplementations();
    for (SPD::Implementations::const_iterator impl = spd_impls.begin(); impl != spd_impls.end(); ++impl) {
        const SPD::SoftPkgDependencies& deps = impl->getSoftPkgDependencies();
        for (SPD::SoftPkgDependencies::const_iterator dep = deps.begin(); dep != deps.end(); ++dep) {
            SPD::SoftPkgRef& ref = const_cast<SPD::SoftPkgRef&>(*dep);
            LOG_TRACE(ApplicationDeployment, "Resolving soft package reference " << ref.localfile);
            loadProfile(fileSystem, ref.localfile);
        }
    }

    return spd;
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
    LOG_TRACE(ApplicationDeployment, "[DeviceLookup] Lookup device that loaded component " << componentId);

    ComponentDeployment* deployment = getComponentDeployment(componentId);
    if (!deployment) {
        LOG_WARN(ApplicationDeployment, "[DeviceLookup] Component not found");
        return CF::Device::_nil();
    }

    boost::shared_ptr<ossie::DeviceNode> device = deployment->getAssignedDevice();
    if (!device) {
        LOG_WARN(ApplicationDeployment, "[DeviceLookup] Component not assigned to device");
        return CF::Device::_nil();
    }

    LOG_TRACE(ApplicationDeployment, "[DeviceLookup] Assigned device id " << device->identifier);
    return CF::Device::_duplicate(device->device);
}

CF::Device_ptr ApplicationDeployment::lookupDeviceUsedByComponentInstantiationId(const std::string& componentId,
                                                                                 const std::string& usesId)
{
    LOG_TRACE(ApplicationDeployment, "[DeviceLookup] Lookup device used by component " << componentId);

    ComponentDeployment* deployment = getComponentDeployment(componentId);
    if (!deployment) {
        LOG_WARN(ApplicationDeployment, "[DeviceLookup] Component not found");
        return CF::Device::_nil();
    }

    UsesDeviceAssignment* uses = deployment->getUsesDeviceAssignment(usesId);
    if (!uses) {
        LOG_WARN(ApplicationDeployment, "[DeviceLookup] UsesDevice not found");
        return CF::Device::_nil();
    }

    //LOG_TRACE(ApplicationDeployment, "[DeviceLookup] Assigned device id " << deviceId);
    return uses->getAssignedDevice();
}

CF::Device_ptr ApplicationDeployment::lookupDeviceUsedByApplication(const std::string& usesRefId)
{
    LOG_TRACE(ApplicationDeployment, "[DeviceLookup] Lookup device used by application, Uses Id: " << usesRefId);

    UsesDeviceAssignment* uses = getUsesDeviceAssignment(usesRefId);
    if (!uses) {
        LOG_WARN(ApplicationDeployment, "[DeviceLookup] UsesDevice not found");
        return CF::Device::_nil();
    }

    //LOG_TRACE(ApplicationDeployment, "[DeviceLookup] Assigned device id " << deviceId);
    return uses->getAssignedDevice();
}
