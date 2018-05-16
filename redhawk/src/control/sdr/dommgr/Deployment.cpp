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
#include <ossie/ComponentDescriptor.h>

#include "Application_impl.h"
#include "PersistenceStore.h"
#include "Deployment.h"
#include "DeploymentExceptions.h"
#include <ossie/CF/DataType.h>

using namespace redhawk;
using namespace ossie;
namespace fs = boost::filesystem;

rh_logger::LoggerPtr redhawk::deploymentLog;

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

SoftPkgDeployment::SoftPkgDeployment(const SoftPkg* softpkg,
                                     const SPD::Implementation* implementation) :
    softpkg(softpkg),
    implementation(implementation)
{
}

SoftPkgDeployment::~SoftPkgDeployment()
{
    clearDependencies();
}

const SoftPkg* SoftPkgDeployment::getSoftPkg() const
{
    return &(*softpkg);
}

void SoftPkgDeployment::setImplementation(const SPD::Implementation* implementation)
{
    this->implementation = implementation;
}

const SPD::Implementation* SoftPkgDeployment::getImplementation() const
{
    return implementation;
}

void SoftPkgDeployment::addDependency(SoftPkgDeployment* dependency)
{
    dependencies.push_back(dependency);
}

const std::vector<SoftPkgDeployment*>& SoftPkgDeployment::getDependencies()
{
    return dependencies;
}

void SoftPkgDeployment::clearDependencies()
{
    for (DeploymentList::iterator dependency = dependencies.begin(); dependency != dependencies.end(); ++dependency) {
        delete (*dependency);
    }
    dependencies.clear();
}

std::vector<std::string> SoftPkgDeployment::getDependencyLocalFiles()
{
    std::vector<std::string> files;
    for (DeploymentList::iterator dependency = dependencies.begin(); dependency != dependencies.end(); ++dependency) {
        std::vector<std::string> depfiles = (*dependency)->getDependencyLocalFiles();
        std::copy(depfiles.begin(), depfiles.end(), std::back_inserter(files));
        files.push_back((*dependency)->getLocalFile());
    }
    return files;
}

void SoftPkgDeployment::load(redhawk::ApplicationComponent* appComponent, CF::FileSystem_ptr fileSystem,
                             CF::LoadableDevice_ptr device)
{
    if (!implementation) {
        throw std::logic_error("no implementation selected for soft package " + softpkg->getName());
    }

    // Recursively load dependencies
    if (!dependencies.empty()) {
        RH_TRACE(deploymentLog, "Loading " << dependencies.size() <<
                    " dependency(ies) for soft package " << softpkg->getName());
        for (DeploymentList::iterator dep = dependencies.begin(); dep != dependencies.end(); ++dep) {
            (*dep)->load(appComponent, fileSystem, device);
        }
    }

    // Determine absolute path of local file
    CF::LoadableDevice::LoadType codeType = getCodeType();
    const std::string fileName = getLocalFile();
    RH_DEBUG(deploymentLog, "Loading file " << fileName
                << " for soft package " << softpkg->getName());
    try {
        device->load(fileSystem, fileName.c_str(), codeType);
    } catch (const CF::Device::InvalidState& exc) {
        std::string message = "device is in invalid state: ";
        message += exc.msg;
        throw DeploymentError(message);
    } catch (const CF::LoadableDevice::InvalidLoadKind& exc) {
        throw DeploymentError("invalid load kind for file " + fileName);
    } catch (const CF::InvalidFileName& exc) {
        std::string message = "file name '" + fileName + "' is invalid: ";
        message += exc.msg;
        throw DeploymentError(message);
    } catch (const CF::LoadableDevice::LoadFail& exc) {
        std::string message = "failure loading file '" + fileName + "': ";
        message += exc.msg;
        throw DeploymentError(message);
    } catch (const CORBA::SystemException& exc) {
        std::string message = ossie::corba::describeException(exc);
        message += " loading " + fileName;
        throw DeploymentError(message);
    }
    appComponent->addLoadedFile(fileName);
}

std::string SoftPkgDeployment::getLocalFile()
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

CF::LoadableDevice::LoadType SoftPkgDeployment::getCodeType() const
{
    switch (implementation->getCodeType()) {
    case SPD::Code::KERNEL_MODULE:
        return CF::LoadableDevice::KERNEL_MODULE;
    case SPD::Code::SHARED_LIBRARY:
        return CF::LoadableDevice::SHARED_LIBRARY;
    case SPD::Code::EXECUTABLE:
        return CF::LoadableDevice::EXECUTABLE;
    case SPD::Code::DRIVER:
        return CF::LoadableDevice::DRIVER;
    default:
        return CF::LoadableDevice::LoadType();
    }
}

bool SoftPkgDeployment::isExecutable() const
{
    // REDHAWK extends section D.2.1.6.3 to support loading a directory
    // and execute a file in that directory using a entrypoint
    // 1. Executable means to use CF LoadableDevice::load and CF ExecutableDevice::execute operations. This is a "main" process.
    //    - A Executable that references a directory instead of a file means to recursively load the contents of the directory
    //      and then execute the program specified via entrypoint
    // 2. Driver and Kernel Module means load only.
    // 3. SharedLibrary means dynamic linking.
    // 4. A (SharedLibrary) Without a code entrypoint element means load only.
    // 5. A (SharedLibrary) With a code entrypoint element means load and CF Device::execute.
    switch (implementation->getCodeType()) {
    case SPD::Code::EXECUTABLE:
        // Returns true if the entry point is non-null
        return bool(implementation->getEntryPoint());
    case SPD::Code::SHARED_LIBRARY:
        return true;
    default:
        return false;
    }
}

ComponentDeployment::ComponentDeployment(const SoftPkg* softpkg,
                                         const ComponentInstantiation* instantiation,
                                         const std::string& identifier) :
    SoftPkgDeployment(softpkg),
    instantiation(instantiation),
    identifier(identifier),
    assemblyController(false),
    container(0),
    appComponent(0)
{
    std::string sadLoggingConfig;
    // If the SoftPkg has an associated Properties, check the overrides for
    // validity
    if (softpkg->getProperties()) {
        BOOST_FOREACH(const ComponentProperty& override, instantiation->getProperties()) {
            const Property* property = softpkg->getProperties()->getProperty(override.getID());
            if (!property) {
                if (override.getID() == "LOGGING_CONFIG_URI") {
                    // It's legal to override the logging configuration, even
                    // if it isn't defined in the PRF
                    const SimplePropertyRef* ref = dynamic_cast<const SimplePropertyRef*>(&override);
                    if ( ref ) {
                        CORBA::Any logging_any;
                        logging_any <<= ref->getValue();
                        overrideProperty("LOGGING_CONFIG_URI", logging_any);
                        sadLoggingConfig = ref->getValue();
                    }
                } else if (override.getID() == "LOG_LEVEL") {
                    // It's legal to override the logging configuration, even
                    // if it isn't defined in the PRF
                    const SimplePropertyRef* ref = dynamic_cast<const SimplePropertyRef*>(&override);
                    if ( ref ) {
                        CORBA::Any log_level_any;
                        log_level_any <<= ref->getValue();
                        overrideProperty("LOG_LEVEL", log_level_any);
                    }
                } else {
                    RH_WARN(deploymentLog, "Ignoring attempt to override property "
                              << override.getID() << " that does not exist in component");
                }
            } else if (!property->canOverride()) {
                RH_WARN(deploymentLog, "Ignoring attempt to override read-only property "
                          << property->getID());
            }
        }
    }

    
    
    ComponentInstantiation::LoggingConfig lcfg = instantiation->getLoggingConfig();
    // if a LOGGING_CONFIG_URI was provided in the SAD file, override the one from the component's profile
    if (not sadLoggingConfig.empty()) {
        lcfg.first = sadLoggingConfig;
    }
    if ( !lcfg.first.empty() ){
        RH_TRACE(deploymentLog, "Logging Config: <" << lcfg.first << ">" );
        loggingConfig["LOGGING_CONFIG_URI"] = lcfg.first;
    }
    if ( !lcfg.second.empty() ){
        RH_TRACE(deploymentLog, "Logging Level: <" << lcfg.second << ">" );
        loggingConfig["LOG_LEVEL"] = lcfg.second;
    }

    if (!instantiation->getAffinity().empty()) {
        RH_TRACE(deploymentLog, "Setting affinity options");
        affinityOptions = ossie::getAffinityOptions(instantiation->getAffinity());
    }

    if (!instantiation->getDeviceRequires().empty()) {
        RH_TRACE(deploymentLog, "Getting devicerequires property set");
        ossie::convertComponentProperties(instantiation->getDeviceRequires(),deviceRequires);
    }
}

const std::string& ComponentDeployment::getIdentifier() const
{
    return identifier;
}

const ComponentInstantiation* ComponentDeployment::getInstantiation() const
{
    return instantiation;
}

void ComponentDeployment::setContainer(ComponentDeployment* container)
{
    this->container = container;
}

ComponentDeployment* ComponentDeployment::getContainer()
{
    return container;
}

bool ComponentDeployment::isResource() const
{
    return softpkg->getDescriptor()->isResource();
}

bool ComponentDeployment::isConfigurable() const
{
    return softpkg->getDescriptor()->isConfigurable();
}

bool ComponentDeployment::isAssemblyController() const
{
    return assemblyController;
}

void ComponentDeployment::setIsAssemblyController(bool state)
{
    assemblyController = state;
}

void ComponentDeployment::setAssignedDevice(const boost::shared_ptr<DeviceNode>& device)
{
    assignedDevice = device;
}

const boost::shared_ptr<DeviceNode>& ComponentDeployment::getAssignedDevice() const
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
        RH_DEBUG(deploymentLog, "ComponentDeployment - Affinity Property: directive id:"
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

redhawk::PropertyMap ComponentDeployment::getDeviceRequires() const {
    return deviceRequires;
}

void ComponentDeployment::setDeviceRequires( const redhawk::PropertyMap &devReqs ) {
    deviceRequires = devReqs;
}


redhawk::PropertyMap ComponentDeployment::getAllocationContext() const
{
    redhawk::PropertyMap properties;
    if (softpkg->getProperties()) {
        BOOST_FOREACH(const Property* property, softpkg->getProperties()->getProperties()) {
            // Old logic (2.0):
            // * "configure" kind properties that are not read-only
            // * "property" kind properties that are not command line (or execparams)
            // New logic:
            // * all "configure" and "property" kind properties
            // Rationale: this is strictly used as context for math statements,
            // so it doesn't matter how it gets initialized or whether it's
            // writable.
            if (property->isConfigure() || property->isProperty()) {
                properties.push_back(getPropertyValue(property));
            }
        }
    }
    return properties;
}

redhawk::PropertyMap ComponentDeployment::getCommandLineParameters() const
{
   redhawk::PropertyMap properties;
   bool has_LOGGING_CONFIG_URI = false;
   bool has_LOG_LEVEL = false;
   if (softpkg->getProperties()) {
       BOOST_FOREACH(const Property* property, softpkg->getProperties()->getProperties()) {
           if (property->isExecParam()) {
               if (property->isReadOnly()) {
                   // NB: Not only can read-only execparams not be overridden,
                   // they are not included in the command line
                   continue;
               }
           } else if (!(property->isProperty() && property->isCommandLine())) {
               continue;
           }
           std::string property_id = property->getID();
           if (property_id == "LOGGING_CONFIG_URI") {
               has_LOGGING_CONFIG_URI = true;
           }
           if (property_id == "LOG_LEVEL") {
               has_LOG_LEVEL = true;
           }
           CF::DataType dt = getPropertyValue(property);
           if (!ossie::any::isNull(dt.value)) {
               properties.push_back(dt);
           }
       }
   }
   if ((not has_LOGGING_CONFIG_URI) and appComponent->isVisible()) {
       if (overrides.find("LOGGING_CONFIG_URI") != overrides.end()) {
           CF::DataType dt;
           dt.id = CORBA::string_dup("LOGGING_CONFIG_URI");
           dt.value <<= overrides["LOGGING_CONFIG_URI"].toString().c_str();
           properties.push_back(dt);
       }
   }
   if ((not has_LOG_LEVEL) and appComponent->isVisible()) {
       if (overrides.find("LOG_LEVEL") != overrides.end()) {
           CF::DataType dt;
           dt.id = CORBA::string_dup("LOG_LEVEL");
           dt.value <<= overrides["LOG_LEVEL"].toString().c_str();
           properties.push_back(dt);
       }
   }

   // Handle special Docker image property if set in component instantiation
   const ComponentProperty* docker = getPropertyOverride("__DOCKER_IMAGE__");
   if (docker) {
       properties["__DOCKER_IMAGE__"] = dynamic_cast<const SimplePropertyRef*>(docker)->getValue();
   }

   return properties;
}

redhawk::PropertyMap ComponentDeployment::getAllInitialProperties() const
{
    redhawk::PropertyMap properties;
    if (softpkg->getProperties()) {
        BOOST_FOREACH(const Property* property, softpkg->getProperties()->getProperties()) {
            properties.push_back(getPropertyValue(property));
        }
    }
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

void ComponentDeployment::overrideProperty(const std::string& id, const CORBA::Any& value)
{
    overrides[id] = value;
}

CF::DataType ComponentDeployment::getPropertyValue(const Property* property) const
{
    if (property->canOverride()) {
        // Check for a runtime override first
        redhawk::PropertyMap::const_iterator override = overrides.find(property->getID());
        if (override != overrides.end()) {
            return *override;
        }
        // Then, check for an override in the component instantiation
        const ComponentProperty* propref = getPropertyOverride(property->getID());
        if (propref) {
            return ossie::overridePropertyValue(property, propref);
        }
    }
    // Default to the PRF value
    return ossie::convertPropertyToDataType(property);
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
        RH_DEBUG(deploymentLog, "ComponentDeployment getAffinityOptionsWithAssignment ... Affinity Property: directive id:"  <<  prop->getId() << "/" <<  prop->getValue().toString());
    }

    if (!nicAssignment.empty()) {
       RH_DEBUG(deploymentLog, "ComponentDeployment getAffinityOptionsWithAssignment ... NIC AFFINITY: pol/value "  <<  "nic"  << "/" << nicAssignment);
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

void ComponentDeployment::load(CF::FileSystem_ptr fileSystem, CF::LoadableDevice_ptr device)
{
    if (!appComponent) {
        throw std::logic_error("deployment is not assigned to an application component");
    }
    SoftPkgDeployment::load(appComponent, fileSystem, device);
}


redhawk::PropertyMap ComponentDeployment::getLoggingConfiguration() const
{
    std::string logcfg_uri;
    std::string debug_level;

    // check for a PRF value
    if (softpkg->getProperties()) {
        const Property* property = softpkg->getProperties()->getProperty("LOGGING_CONFIG_URI");
        if (property) {
            const SimpleProperty* simple = dynamic_cast<const SimpleProperty*>(property);
            if (simple && simple->getValue()) {
                logcfg_uri = simple->getValue();
            }
        }
    }

    // Check for a runtime override first
    redhawk::PropertyMap::const_iterator  override;
    if (overrides.contains("LOGGING_CONFIG_URI") ) {
        override = overrides.find("LOGGING_CONFIG_URI");
        if (!override->getValue().isNil()) {
            logcfg_uri = override->getValue().toString();
        }
    }

    if (overrides.contains("LOG_LEVEL") ) {
        override = overrides.find("LOG_LEVEL");
        if (!override->getValue().isNil()) {
            debug_level = override->getValue().toString();
        }
    }

    // Then, check for an override in the component instantiation
    const ComponentProperty* propref = getPropertyOverride("LOGGING_CONFIG_URI");
    if (propref) {
        const SimplePropertyRef* simple = dynamic_cast<const SimplePropertyRef*>(propref);
        if (simple) {
            logcfg_uri=simple->getValue();
        }
    }

    propref = getPropertyOverride("LOG_LEVEL");
    if (propref) {
        const SimplePropertyRef* simple = dynamic_cast<const SimplePropertyRef*>(propref);
        if (simple) {
            debug_level=simple->getValue();
        }
    }

    redhawk::PropertyMap ret;

    // prefer logging config if provide via sad, else use property setting
    if ( loggingConfig.contains("LOGGING_CONFIG_URI") )  {
        logcfg_uri = loggingConfig["LOGGING_CONFIG_URI"].toString();
    }

    if ( loggingConfig.contains("LOG_LEVEL") )  {
        debug_level = loggingConfig["LOG_LEVEL"].toString();
    }

    if ( !logcfg_uri.empty() ) {
        ret["LOGGING_CONFIG_URI"]=logcfg_uri;
    }

    if ( !debug_level.empty() ) {
        ret["LOG_LEVEL"]=debug_level;
    }

    return ret;
}

redhawk::ApplicationComponent* ComponentDeployment::getApplicationComponent()
{
    return appComponent;
}

void ComponentDeployment::setApplicationComponent(redhawk::ApplicationComponent* component)
{
    appComponent = component;
}

void ComponentDeployment::initializeProperties()
{
    redhawk::PropertyMap init_props = getInitializeProperties();

    CF::Properties partials = ossie::getPartialStructs(init_props);
    if (partials.length() > 0) {
        std::ostringstream eout;
        eout << "cannot be initialized due to " << partials.length();
        eout << " structure(s) with a mix of defined and nil values: ";
        bool first = true;
        for (size_t index = 0; index < partials.length(); ++index) {
            if (!first) {
                eout << ", ";
            }
            eout << partials[index].id;
        }
        throw ComponentError(this, eout.str());
    }

    RH_DEBUG(deploymentLog, "Initializing properties for component " << identifier);
    try {
        resource->initializeProperties(init_props);
    } catch (const CF::PropertySet::InvalidConfiguration& exc) {
        throw PropertiesError(this, exc.invalidProperties, "invalid configuration in property initialization");
    } catch (const CF::PropertySet::PartialConfiguration& exc) {
        throw PropertiesError(this, exc.invalidProperties, "partial configuration in property initialization");
    } catch (const CF::PropertyEmitter::AlreadyInitialized&) {
        // The component should never be initialized twice, at least not by the
        // ApplicationFactory
        throw ComponentError(this, "already initialized");
    } catch (const CORBA::SystemException& exc) {
        throw ComponentError(this, "initializing properties raised " + ossie::corba::describeException(exc));
    } catch (...) {
        // Should never happen, but turn anything else into a ComponentError
        // just in case
        throw ComponentError(this, "unexpected error initializing properties");
    }
}

void ComponentDeployment::initialize()
{
    if (isConfigurable()) {
        initializeProperties();
    }

    RH_TRACE(deploymentLog, "Initializing component " << identifier);
    try {
        resource->initialize();
    } catch (const CF::LifeCycle::InitializeError& error) {
        // Dump the detailed initialization failure to the log
        std::ostringstream logmsg;
        logmsg << "initialize error";
        for (CORBA::ULong index = 0; index < error.errorMessages.length(); ++index) {
            if (index > 0) {
                logmsg << ",";
            }
            logmsg << " '" << error.errorMessages[index] << "'";
        }
        throw ComponentError(this, logmsg.str());
    } catch (const CORBA::SystemException& exc) {
        throw ComponentError(this, "initialize raised " + ossie::corba::describeException(exc));
    } catch (...) {
        // Should never happen, but turn anything else into a ComponentError
        // just in case
        throw ComponentError(this, "unexpected error in initialize");
    }
}

void ComponentDeployment::configure()
{
    if (!softpkg->isScaCompliant()) {
        // If the component is non-SCA compliant then we don't expect anything beyond this
        RH_TRACE(deploymentLog, "Skipping configure of non SCA-compliant component "
                    << identifier);
        return;
    } else if (!isResource()) {
        RH_TRACE(deploymentLog, "Skipping configure of non-resource component "
                    << identifier);
        return;
    }

    if (!instantiation->isNamingService()) {
        // Per the old code, we only configure if the instantiation uses naming
        // service to locate the component
        return;
    }

    if (CORBA::is_nil(resource)) {
        // NB: I think having a valid CORBA reference is a pre-condition of
        // getting to this point in the first place
        RH_ERROR(deploymentLog, "Could not get component reference");
        throw redhawk::ComponentError(this, "no CORBA reference");
    }

    redhawk::PropertyMap config_props = getInitialConfigureProperties();

    // Skip empty configure call
    if (config_props.empty()) {
        return;
    }

    // Check and warn for partial structs
    CF::Properties partials = ossie::getPartialStructs(config_props);
    if (partials.length() > 0) {
        std::ostringstream eout;
        eout << "Component " << identifier << " contains " << partials.length()
             << " structure(s) with a mix of defined and nil values: ";
        bool first = true;
        for (size_t index = 0; index < partials.length(); ++index) {
            if (!first) {
                eout << ", ";
            }
            eout << partials[index].id;
        }
        eout << ". The behavior for the component is undefined";
        RH_WARN(deploymentLog, eout.str());
    }

    RH_TRACE(deploymentLog, "Configuring component " << identifier);
    try {
        resource->configure(config_props);
    } catch (const CF::PropertySet::InvalidConfiguration& exc) {
        throw PropertiesError(this, exc.invalidProperties, "invalid configuration in configure");
    } catch (const CF::PropertySet::PartialConfiguration& exc) {
        throw PropertiesError(this, exc.invalidProperties, "partial configuration in configure");
    } catch (const CORBA::SystemException& exc) {
        throw ComponentError(this, "configure raised " + ossie::corba::describeException(exc));
    } catch (...) {
        throw ComponentError(this, "unexpected error configuring component");
    }
}
