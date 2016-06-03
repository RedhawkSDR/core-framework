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


#include <iostream>
#include <iomanip>
#include <sstream>
#include <memory>
#include <algorithm>
#include <functional>
#include <set>
#include <list>
#include <unistd.h>

#include <boost/foreach.hpp>
#include <boost/filesystem/path.hpp>

#include <ossie/CF/WellKnownProperties.h>
#include <ossie/FileStream.h>
#include <ossie/prop_helpers.h>
#include <ossie/prop_utils.h>

#include "Application_impl.h"
#include "ApplicationFactory_impl.h"
#include "createHelper.h"
#include "DomainManager_impl.h"
#include "AllocationManager_impl.h"
#include "RH_NamingContext.h"
#include "ApplicationValidator.h"

namespace fs = boost::filesystem;
using namespace ossie;
using namespace std;

ScopedAllocations::ScopedAllocations(AllocationManager_impl& allocator):
    _allocator(allocator)
{
}

ScopedAllocations::~ScopedAllocations()
{
    try {
        deallocate();
    } catch (...) {
        // Destructors must not throw
    }
}

void ScopedAllocations::push_back(const std::string& allocationID)
{
    _allocations.push_back(allocationID);
}

template <class T>
void ScopedAllocations::transfer(T& dest)
{
    std::copy(_allocations.begin(), _allocations.end(), std::back_inserter(dest));
    _allocations.clear();
}

void ScopedAllocations::transfer(ScopedAllocations& dest)
{
    transfer(dest._allocations);
}

void ScopedAllocations::deallocate()
{
    if (!_allocations.empty()) {
        LOG_TRACE(ApplicationFactory_impl, "Deallocating " << _allocations.size() << " allocations");
        _allocator.deallocate(_allocations.begin(), _allocations.end());
    }
}



/* Rotates a device list to put the device with the given identifier first
 */
static void rotateDeviceList(DeviceList& devices, const std::string& identifier)
{
    const DeviceList::iterator begin = devices.begin();
    for (DeviceList::iterator node = begin; node != devices.end(); ++node) {
        if ((*node)->identifier == identifier) {
            if (node != begin) {
                std::rotate(devices.begin(), node, devices.end());
            }
            return;
        }
    }
}

namespace {
    template <class T>
    inline bool mergeDependencies(std::vector<T>& first, const std::vector<T>& second)
    {
        if (second.empty()) {
            return true;
        } else if (first.empty()) {
            first = second;
            return true;
        } else {
            for (typename std::vector<T>::iterator iter = first.begin(); iter != first.end(); ) {
                if (std::find(second.begin(), second.end(), *iter) == second.end()) {
                    iter = first.erase(iter);
                } else {
                    ++iter;
                }
            }
            return !first.empty();
        }
    }
}

PREPARE_LOGGING(ApplicationFactory_impl);

ApplicationFactory_impl::ApplicationFactory_impl (const std::string& softwareProfile,
                                                  const std::string& domainName, 
                                                  DomainManager_impl* domainManager) :
    _softwareProfile(softwareProfile),
    _domainName(domainName),
    _domainManager(domainManager),
    _lastWaveformUniqueId(0)
{
    // Get the naming context from the domain
    _domainContext = RH_NamingContext::GetNamingContext( _domainName, !_domainManager->bindToDomain() );
    if (CORBA::is_nil(_domainContext)) {
        LOG_ERROR(ApplicationFactory_impl, "CosNaming::NamingContext::_narrow threw Unknown Exception");
        throw;
    }

    try {
        _fileMgr = _domainManager->fileMgr();
    } catch ( std::exception& ex ) {
        ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while retrieving the File Manager";
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch ( const CORBA::Exception& ex ) {
        ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while retrieving the File Manager";
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch( ... ) {
        LOG_ERROR(ApplicationFactory_impl, "domainManager->_fileMgr failed with Unknown Exception");
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, "Could not get File Manager from Domain Manager");
    }

    LOG_INFO(ApplicationFactory_impl, "Installing application " << _softwareProfile);
    try {
        if (!_fileMgr->exists(_softwareProfile.c_str())) {
            std::string msg = "File ";
            msg += _softwareProfile;
            msg += " does not exist.";
            throw CF::FileException (CF::CF_ENOENT, msg.c_str());
        }

        File_stream _sad(_fileMgr, _softwareProfile.c_str());
        _sadParser.load(_sad);
        _sad.close();
    } catch (const ossie::parser_error& ex) {
        ostringstream eout;
        std::string parser_error_line = ossie::retrieveParserErrorLineNumber(ex.what());
        eout << "Failed to parse SAD file: " << _softwareProfile << ". " << parser_error_line << " The XML parser returned the following error: " << ex.what();
        LOG_ERROR(ApplicationFactory_impl, eout.str());
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch ( std::exception& ex ) {
        ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<". While loading "<<_softwareProfile;
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch( CF::InvalidFileName& ex ) {
        ostringstream eout;
        eout << "The following InvalidFileName exception occurred, profile: " << _softwareProfile;
        LOG_ERROR(ApplicationFactory_impl, eout.str());
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, eout.str().c_str());
    } catch ( const CF::FileException& ex ) {
        ostringstream eout;
        eout << "The following FileException occurred: "<<ex.msg<<"  While loading "<<_softwareProfile;
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch ( const CORBA::Exception& ex ) {
        ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" While loading "<<_softwareProfile;
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch( ... ) {
        ostringstream eout;
        eout << "Parsing SAD file: " <<_softwareProfile << " Failed with unknown exception.";
        LOG_ERROR(ApplicationFactory_impl, eout.str());
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_ENOENT, eout.str().c_str());
    }

    // Validate the application using the current domain state; however, we
    // cannot assume that the component SPDs will not change between now and a
    // subsequent create call, so the parsed profiles are not saved
    redhawk::ApplicationValidator validator(_fileMgr);
    try {
        validator.validate(_sadParser);
    } catch (const std::runtime_error& exc) {
        LOG_ERROR(ApplicationFactory_impl, "SAD " << softwareProfile
                  << " failed validation: " << exc.what());
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, exc.what());
    }

    _name = _sadParser.getName();
    _identifier = _sadParser.getID();
}

ApplicationFactory_impl::~ApplicationFactory_impl ()
{
  try {
    //
    // remove the naming context assocated with the factory that generates new
    // naming contexts for each application.
    //
    if ( _domainManager && _domainManager->bindToDomain() ) _domainContext->destroy();
  }
  catch(...)
    {};

}

void createHelper::_connectComponents(ossie::ApplicationDeployment& appDeployment,
                                      std::vector<ConnectionNode>& connections){
    try{
        connectComponents(appDeployment, connections, _baseNamingContext);
    } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
        throw;
    } CATCH_THROW_LOG_TRACE(
        ApplicationFactory_impl,
        "Connecting components failed (unclear where this occurred)",
        CF::ApplicationFactory::CreateApplicationError(
            CF::CF_EINVAL, 
            "Connecting components failed (unclear where this occurred)"));
}

void createHelper::_configureComponents(const DeploymentList& deployments)
{
    try{
        configureComponents(deployments);
    } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
        throw;
    } CATCH_THROW_LOG_TRACE(
        ApplicationFactory_impl, 
        "Configure on component failed (unclear where in the process this occurred)",
        CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, "Configure of component failed (unclear where in the process this occurred)"))
}

void createHelper::assignPlacementsToDevices(ossie::ApplicationDeployment& appDeployment,
                                             const DeviceAssignmentMap& devices)
{
    // Try to place all of the collocations first, since they naturally have
    // more restrictive placement constraints
    BOOST_FOREACH(const SoftwareAssembly::HostCollocation& collocation, _appFact._sadParser.getHostCollocations()) {
        _placeHostCollocation(appDeployment, collocation, devices);
    }

    // Place the remaining components one-by-one
    BOOST_FOREACH(const ComponentPlacement& placement, _appFact._sadParser.getComponentPlacements()) {
        const SoftPkg* softpkg = appDeployment.getSoftPkg(placement.filename);
        BOOST_FOREACH(const ComponentInstantiation& instantiation, placement.getInstantiations()) {
            // Even though the XML supports more than one instantiation per
            // component placement, the tooling doesn't support that, so this
            // loop may be strictly academic
            std::string assigned_device;
            DeviceAssignmentMap::const_iterator device = devices.find(instantiation.getID());
            if (device != devices.end()) {
                assigned_device = device->second;
                LOG_TRACE(ApplicationFactory_impl, "Component " << instantiation.getID()
                          << " is assigned to device " << assigned_device);
            }
            ComponentDeployment* deployment = appDeployment.createComponentDeployment(softpkg, &instantiation);
            allocateComponent(appDeployment, deployment, assigned_device);
        }
    }
}

void createHelper::_validateDAS(ossie::ApplicationDeployment& appDeployment,
                                const DeviceAssignmentMap& deviceAssignments)
{
    LOG_TRACE(ApplicationFactory_impl, "Validating device assignment sequence (length "
              << deviceAssignments.size() << ")");
    for (DeviceAssignmentMap::const_iterator ii = deviceAssignments.begin(); ii != deviceAssignments.end(); ++ii) {
        const std::string& componentId = ii->first;
        const std::string& assignedDeviceId = ii->second;

        if (!_appFact._sadParser.getComponentInstantiation(componentId)) {
            LOG_ERROR(ApplicationFactory_impl, "Failed to create application; "
                      << "unknown component " << componentId 
                      << " in user assignment (DAS)");
            CF::DeviceAssignmentSequence badDAS;
            badDAS.length(1);
            badDAS[0].componentId = componentId.c_str();
            badDAS[0].assignedDeviceId = assignedDeviceId.c_str();
            throw CF::ApplicationFactory::CreateApplicationRequestError(badDAS);
        }
    }
}

bool createHelper::placeHostCollocation(ossie::ApplicationDeployment& appDeployment,
                                        const DeploymentList& components,
                                        DeploymentList::const_iterator current,
                                        ossie::DeviceList& deploymentDevices,
                                        const ProcessorList& processorDeps,
                                        const OSList& osDeps)
{
    if (current == components.end()) {
        // Reached the end of the component deployments; all implementations
        // should be set, so give it a try
        return allocateHostCollocation(appDeployment, components, deploymentDevices, processorDeps, osDeps);
    }

    // Try all of the implementations from the current component for matches
    // with the processor and OS dependencies
    ossie::ComponentDeployment* deployment = *current;
    const SPD::Implementations& comp_impls = deployment->getSoftPkg()->getImplementations();
    LOG_TRACE(ApplicationFactory_impl, "Finding collocation-compatible implementations for component "
              << deployment->getInstantiation()->getID());
    ++current;
    for (SPD::Implementations::const_iterator impl = comp_impls.begin(); impl != comp_impls.end(); ++impl) {
        const ossie::SPD::Implementation* implementation = &(*impl);
        LOG_TRACE(ApplicationFactory_impl, "Checking implementation " << implementation->getID());

        // Check that the processor dependencies are compatible, filtering out
        // anything not compatible with the current component
        std::vector<std::string> proc_list = processorDeps;;
        if (!mergeDependencies(proc_list, implementation->getProcessors())) {
            LOG_TRACE(ApplicationFactory_impl, "Skipping implementation " << implementation->getID()
                      << ": no processor match");
            continue;
        }

        // Check that the OS dependencies are compatible, again filtering out
        // anything not compatible with the current component
        std::vector<ossie::SPD::NameVersionPair> os_list = osDeps;
        if (!mergeDependencies(os_list, implementation->getOsDeps())) {
            LOG_TRACE(ApplicationFactory_impl, "Skipping implementation " << implementation->getID()
                      << ": no OS match");
            continue;
        }

        // Set this implementation for deployment and recurse one more level
        deployment->setImplementation(implementation);
        if (placeHostCollocation(appDeployment, components, current, deploymentDevices, proc_list, os_list)) {
            return true;
        }
    }

    return false;
}

bool createHelper::allocateHostCollocation(ossie::ApplicationDeployment& appDeployment,
                                           const DeploymentList& components,
                                           ossie::DeviceList& deploymentDevices,
                                           const ProcessorList& processorDeps,
                                           const OSList& osDeps)
{
    // Consolidate the allocation properties into a single list
    CF::Properties allocationProperties = _consolidateAllocations(components);

    LOG_TRACE(ApplicationFactory_impl, "Allocating deployment for " << components.size()
              << " collocated components");
    for (DeploymentList::const_iterator depl = components.begin(); depl != components.end(); ++depl) {
        LOG_TRACE(ApplicationFactory_impl, "Component " << (*depl)->getInstantiation()->getID()
                  << " implementation " << (*depl)->getImplementation()->getID());
    }

    const std::string requestid = ossie::generateUUID();
    ossie::AllocationResult response = _allocationMgr->allocateDeployment(requestid, allocationProperties, deploymentDevices, appDeployment.getIdentifier(), processorDeps, osDeps);
    if (!response.first.empty()) {
        // Ensure that all capacities get cleaned up, keeping ownership local
        // to this scope until it's clear that the device can support all of
        // the collocated components' dependencies
        ScopedAllocations local_allocations(*_allocationMgr);
        local_allocations.push_back(response.first);

        // Convert from response back into a device node
        boost::shared_ptr<ossie::DeviceNode>& node = response.second;
        const std::string& deviceId = node->identifier;

        for (DeploymentList::const_iterator depl = components.begin(); depl != components.end(); ++depl) {
            // Reset any dependencies that may have been resolved in a prior attempt
            (*depl)->clearDependencies();
            if (!resolveSoftpkgDependencies(appDeployment, *depl, *node)) {
                LOG_TRACE(ApplicationFactory_impl, "Unable to resolve softpackage dependencies for component "
                          << (*depl)->getIdentifier()
                          << " implementation " << (*depl)->getImplementation()->getID());
                return false;
            }
            (*depl)->setAssignedDevice(node);
        }

        // Once all the dependencies have been resolved, take ownership of the
        // allocations
        local_allocations.transfer(_allocations);

        // Move the device to the front of the list
        rotateDeviceList(_executableDevices, deviceId);

        LOG_TRACE(ApplicationFactory_impl, "Successful collocation allocation");
        return true;
    }
    LOG_TRACE(ApplicationFactory_impl, "Failed collocation allocation");
    return false;
 }

CF::Properties createHelper::_consolidateAllocations(const DeploymentList& deployments)
{
    CF::Properties allocs;
    for (DeploymentList::const_iterator depl = deployments.begin(); depl != deployments.end(); ++depl) {
        const std::vector<PropertyRef>& deps = (*depl)->getImplementation()->getDependencies();
        for (std::vector<PropertyRef>::const_iterator dep = deps.begin(); dep != deps.end(); ++dep) {
          ossie::ComponentProperty *prop = dep->property.get();
          ossie::corba::push_back(allocs, ossie::convertPropertyRefToDataType(prop));
        }
    }
    return allocs;
}

void createHelper::_placeHostCollocation(ossie::ApplicationDeployment& appDeployment,
                                         const ossie::SoftwareAssembly::HostCollocation& collocation,
                                         const DeviceAssignmentMap& devices)
{
    LOG_TRACE(ApplicationFactory_impl, "Placing host collocation " << collocation.getID()
              << " " << collocation.getName());

    // Keep track of devices to which some of the components have
    // been assigned.
    DeviceIDList assignedDevices;
    DeploymentList deployments;
    BOOST_FOREACH(const ComponentPlacement& placement, collocation.getComponents()) {
        const SoftPkg* softpkg = appDeployment.getSoftPkg(placement.filename);
        BOOST_FOREACH(const ComponentInstantiation& instantiation, placement.getInstantiations()) {
            // Even though the XML supports more than one instantiation per
            // component placement, the tooling doesn't support that, so this
            // loop may be strictly academic
            ComponentDeployment* deployment = appDeployment.createComponentDeployment(softpkg, &instantiation);
            deployments.push_back(deployment);

            DeviceAssignmentMap::const_iterator device = devices.find(instantiation.getID());
            if (device != devices.end()) {
                assignedDevices.push_back(device->second);
            }
        }
    }

    // Get the executable devices for the domain; if there were any devices
    // assigned, filter out all other devices
    ossie::DeviceList deploymentDevices = _executableDevices;
    if (!assignedDevices.empty()) {
        for (ossie::DeviceList::iterator node = deploymentDevices.begin(); node != deploymentDevices.end(); ++node) {
            if (std::find(assignedDevices.begin(), assignedDevices.end(), (*node)->identifier) == assignedDevices.end()) {
                node = deploymentDevices.erase(node);
            }
        }
    }
    
    LOG_TRACE(ApplicationFactory_impl, "Placing " << deployments.size() << " components");
    if (!placeHostCollocation(appDeployment, deployments, deployments.begin(), deploymentDevices)) {
        std::ostringstream eout;
        eout << "Could not collocate components for collocation NAME: " << collocation.getName() << "  ID:" << collocation.id;
        LOG_ERROR(ApplicationFactory_impl, eout.str());
        throw CF::ApplicationFactory::CreateApplicationRequestError();
    }
    LOG_TRACE(ApplicationFactory_impl, "-- Completed placement for Collocation ID:"
              << collocation.getID() << " Components Placed: " << deployments.size());
}

void createHelper::_handleUsesDevices(ossie::ApplicationDeployment& appDeployment,
                                      const std::string& appName)
{
    // Gets all uses device info from the SAD file
    const std::vector<UsesDevice>& usesDevices = _appFact._sadParser.getUsesDevices();
    LOG_TRACE(ApplicationFactory_impl, "Application has " << usesDevices.size() << " usesdevice dependencies");

    // Get the assembly controller's configure properties for context in the
    // allocations
    CF::Properties appProperties = appDeployment.getAllocationContext();

    // The device assignments for SAD-level usesdevices are never stored
    ossie::UsesDeviceDeployment assignedDevices;
    if (!allocateUsesDevices(usesDevices, appProperties, assignedDevices, this->_allocations)) {
        // There were unsatisfied usesdevices for the application
        ostringstream eout;
        eout << "Failed to satisfy 'usesdevice' dependencies ";
        bool first = true;
        for (std::vector<UsesDevice>::const_iterator uses = usesDevices.begin(); uses != usesDevices.end(); ++uses) {
            if (!assignedDevices.getUsesDeviceAssignment(uses->getID())) {
                if (!first) {
                    eout << ", ";
                } else {
                    first = false;
                }
                eout << uses->getID();
            }
        }
        eout << "for application '" << appName << "'";
        LOG_DEBUG(ApplicationFactory_impl, eout.str());
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
    }

    assignedDevices.transferUsesDeviceAssignments(appDeployment);
}

void createHelper::setUpExternalPorts(ossie::ApplicationDeployment& appDeployment,
                                      Application_impl* application)
{
    typedef std::vector<SoftwareAssembly::Port> PortList;
    const PortList& ports = _appFact._sadParser.getExternalPorts();
    LOG_TRACE(ApplicationFactory_impl,
              "Mapping " << ports.size() << " external port(s)");

    for (PortList::const_iterator port = ports.begin(); port != ports.end(); ++port) {
        LOG_TRACE(ApplicationFactory_impl,
                  "Port component: " << port->componentrefid
                        << " Port identifier: " << port->identifier);

        // Get the component from the instantiation identifier.
        ossie::ComponentDeployment* deployment = appDeployment.getComponentDeployment(port->componentrefid);
        if (!deployment) {
            LOG_ERROR(ApplicationFactory_impl,
                      "Invalid componentinstantiationref ("
                            <<port->componentrefid
                            <<") given for an external port ");
            throw(CF::ApplicationFactory::CreateApplicationError(
                CF::CF_NOTSET,
                "Invalid componentinstantiationref given for external port"));
        }

        CF::Resource_var resource = deployment->getResourcePtr();
        CORBA::Object_var obj;

        if (port->type == SoftwareAssembly::Port::SUPPORTEDIDENTIFIER) {
            if (!resource->_is_a(port->identifier.c_str())) {
                LOG_ERROR(
                    ApplicationFactory_impl,
                    "Component does not support requested interface: "
                        << port->identifier);
                throw(CF::ApplicationFactory::CreateApplicationError(
                    CF::CF_NOTSET,
                    "Component does not support requested interface"));
            }
            obj = CORBA::Object::_duplicate(resource);
        } else {
            // Must be either "usesidentifier" or "providesidentifier",
            // which are equivalent unless you want to be extra
            // pedantic and check how the port is described in the
            // component's SCD.
            // Try to look up the port.
            try {
                obj = resource->getPort(port->identifier.c_str());
            } CATCH_THROW_LOG_ERROR(
                ApplicationFactory_impl,
                "Invalid port id",
                CF::ApplicationFactory::CreateApplicationError(
                    CF::CF_NOTSET,
                    "Invalid port identifier"))
        }

        // Add it to the list of external ports on the application object.
        application->addExternalPort(port->getExternalName(), obj);
    }
}

void createHelper::setUpExternalProperties(ossie::ApplicationDeployment& appDeployment,
                                           Application_impl* application)
{
    const std::vector<SoftwareAssembly::Property>& props = _appFact._sadParser.getExternalProperties();
    LOG_TRACE(ApplicationFactory_impl, "Mapping " << props.size() << " external property(ies)");
    for (std::vector<SoftwareAssembly::Property>::const_iterator prop = props.begin(); prop != props.end(); ++prop) {
        LOG_TRACE(ApplicationFactory_impl, "Property component: " << prop->comprefid << " Property identifier: " << prop->propid);

        // Get the component from the compref identifier.
        ossie::ComponentDeployment* deployment = appDeployment.getComponentDeployment(prop->comprefid);
        if (!deployment) {
            LOG_ERROR(ApplicationFactory_impl, "Unable to find component for comprefid " << prop->comprefid);
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, "Unable to find component for given comprefid");
        }
        const Property* property = deployment->getSoftPkg()->getProperties()->getProperty(prop->propid);
        if (!property){
            LOG_ERROR(ApplicationFactory_impl, "Attempting to promote property: '" <<
                    prop->propid << "' that does not exist in component: '" << prop->comprefid << "'");
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET,
                    "Attempting to promote property that does not exist in component");
        }

        CF::Resource_var comp = deployment->getResourcePtr();
        application->addExternalProperty(prop->propid, prop->getExternalID(), comp);
    }
}

/* Creates and instance of the application.
 *  - Assigns components to devices
 *      - First based on user-provided DAS if one is passed in
 *        (deviceAssignments)
 *      - Then based on property matching and allocation matching
 *  - Attempts to honor host collocation
 *  @param name user-friendly name of the application to be instantiated
 *  @param initConfiguration properties that can override those from the SAD
 *  @param deviceAssignments optional user-provided component-to-device
 *         assignments
 */
CF::Application_ptr ApplicationFactory_impl::create (
    const char* name,
    const CF::Properties& initConfiguration,
    const CF::DeviceAssignmentSequence& deviceAssignments)
throw (CORBA::SystemException, CF::ApplicationFactory::CreateApplicationError,
        CF::ApplicationFactory::CreateApplicationRequestError,
        CF::ApplicationFactory::CreateApplicationInsufficientCapacityError,
        CF::ApplicationFactory::InvalidInitConfiguration)
{
    TRACE_ENTER(ApplicationFactory_impl);
    LOG_TRACE(ApplicationFactory_impl, "Creating application " << name);

    // must declare these here, so we can pass to the createHelper instance
    string _waveform_context_name;
    string base_naming_context;
    CosNaming::NamingContext_var _waveformContext;

    ///////////////////////////////////////////////////
    // Establish new naming context for waveform
    LOG_TRACE(ApplicationFactory_impl, "Establishing waveform naming context");
    try {
        // VERY IMPORTANT: we must first lock the operations in this try block
        //    in order to prevent a naming context collision due to multiple create calls
        boost::mutex::scoped_lock lock(_pendingCreateLock);

        // get new naming context name
        _waveform_context_name = getWaveformContextName(name);
        base_naming_context = getBaseWaveformContext(_waveform_context_name);

        _waveformContext = CosNaming::NamingContext::_nil();

        // create the new naming context
        CosNaming::Name WaveformContextName;
        WaveformContextName.length(1);
        WaveformContextName[0].id = _waveform_context_name.c_str();

        LOG_TRACE(ApplicationFactory_impl, "Binding new context " << _waveform_context_name.c_str());
        try {
            _waveformContext = _domainContext->bind_new_context(WaveformContextName);
        } catch( ... ) {
            // just in case it bound, unbind and error
            // roughly the same code as _cleanupNewContext
            try {
                _domainContext->unbind(WaveformContextName);
            } catch ( ... ) {
            }
            LOG_ERROR(ApplicationFactory_impl, "bind_new_context threw Unknown Exception");
            throw;
        }

    } catch(...){
    }

    // Convert the device assignments into a map for easier lookup
    std::map<std::string,std::string> deviceAssignmentMap;
    for (size_t index = 0; index < deviceAssignments.length(); ++index) {
        const std::string componentId(deviceAssignments[index].componentId);
        const std::string assignedDeviceId(deviceAssignments[index].assignedDeviceId);
        deviceAssignmentMap.insert(std::make_pair(componentId, assignedDeviceId));
    }

    // now use the createHelper class to actually run 'create'
    // - createHelper is needed to allow concurrent calls to 'create' without
    //   each instance stomping on the others
    LOG_TRACE(ApplicationFactory_impl, "Creating new createHelper class.");
    createHelper new_createhelper(*this, _waveform_context_name, base_naming_context, _waveformContext, _domainContext);

    // now actually perform the create operation
    LOG_TRACE(ApplicationFactory_impl, "Performing 'create' function.");
    CF::Application_ptr new_app;
    try {
        new_app = new_createhelper.create(name, initConfiguration, deviceAssignmentMap);
    } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
        LOG_ERROR(ApplicationFactory_impl, "Error in application creation; " << ex.msg);
        throw;
    } catch (CF::ApplicationFactory::CreateApplicationRequestError& ex) {
        LOG_ERROR(ApplicationFactory_impl, "Error in application creation")
        throw;
    } catch (const std::exception& ex) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while creating the application";
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EBADF, eout.str().c_str());
    } catch (const CORBA::Exception& ex) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while creating the application";
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, eout.str().c_str());
    } catch ( ... ) {
        LOG_ERROR(ApplicationFactory_impl, "Unexpected error in application creation - see log")
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, "Unexpected error in application creation - see log.");
    }

    // return the new Application
    TRACE_EXIT(ApplicationFactory_impl);
    return new_app;
}

CF::Application_ptr createHelper::create (
    const char*                         name,
    const CF::Properties&               initConfiguration,
    const DeviceAssignmentMap& deviceAssignments)
{
    TRACE_ENTER(ApplicationFactory_impl);
    
    bool aware_application = true;
    
    CF::Properties modifiedInitConfiguration;

    ///////////////////////////////////////////////////////////////////
    // Check to see if this is an aware application and 
    //  check to see if a different GPP reservation setting is defined
    const std::string aware_app_property_id(ExtendedCF::WKP::AWARE_APPLICATION);
    for (unsigned int initCount = 0; initCount < initConfiguration.length(); initCount++) {
        if (std::string(initConfiguration[initCount].id) == aware_app_property_id) {
            initConfiguration[initCount].value >>= aware_application;
            modifiedInitConfiguration.length(initConfiguration.length()-1);
            for (unsigned int rem_idx=0; rem_idx<initConfiguration.length()-1; rem_idx++) {
                unsigned int idx_mod = 0;
                if (rem_idx == initCount)
                    idx_mod = 1;
                modifiedInitConfiguration[rem_idx] = initConfiguration[rem_idx+idx_mod];
            }
        }
    }

    if (modifiedInitConfiguration.length() == 0) {
        modifiedInitConfiguration = initConfiguration;
    }

    const std::string specialized_reservation("SPECIALIZED_CPU_RESERVATION");
    std::map<std::string,float> specialized_reservations;
    for (unsigned int initCount = 0; initCount < modifiedInitConfiguration.length(); initCount++) {
        if (std::string(modifiedInitConfiguration[initCount].id) == specialized_reservation) {
            CF::Properties *reservations;
            if (modifiedInitConfiguration[initCount].value >>= reservations) {
                for (unsigned int rem_idx=0; rem_idx<reservations->length(); rem_idx++) {
                    double value = 0;
                    std::string component_id((*reservations)[rem_idx].id);
                    if ((*reservations)[rem_idx].value >>= value) {
                        specialized_reservations[component_id] = value;
                    }
                }
            } else {
                // the value of the any is of the wrong type
            }
            for (unsigned int rem_idx=initCount; rem_idx<modifiedInitConfiguration.length()-1; rem_idx++) {
                modifiedInitConfiguration[rem_idx] = modifiedInitConfiguration[rem_idx+1];
            }
            modifiedInitConfiguration.length(modifiedInitConfiguration.length()-1);
        }
    }

    // Get a list of all device currently in the domain
    _registeredDevices = _appFact._domainManager->getRegisteredDevices();
    _executableDevices.clear();
    for (DeviceList::iterator iter = _registeredDevices.begin(); iter != _registeredDevices.end(); ++iter) {
        if ((*iter)->isExecutable) {
            _executableDevices.push_back(*iter);
        }
    }

    // Fail immediately if there are no available devices to execute components
    if (_executableDevices.empty()) {
        const char* message = "Domain has no executable devices (GPPs) to run components";
        LOG_WARN(ApplicationFactory_impl, message);
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENODEV, message);
    }

    const std::string lastExecutableDevice = _appFact._domainManager->getLastDeviceUsedForDeployment();
    if (!lastExecutableDevice.empty()) {
        LOG_TRACE(ApplicationFactory_impl, "Placing device " << lastExecutableDevice
                  << " first in deployment list");
        rotateDeviceList(_executableDevices, lastExecutableDevice);
    }

    //////////////////////////////////////////////////
    // Load the components to instantiate from the SAD
    ossie::ApplicationDeployment app_deployment(_appFact._sadParser, _waveformContextName, modifiedInitConfiguration);
    app_deployment.loadProfiles(_appFact._fileMgr);

    ////////////////////////////////////////////////
    // Assign components to devices
    ////////////////////////////////////////////////

    // Catch invalid device assignments
    _validateDAS(app_deployment, deviceAssignments);

    // Assign all components to devices
    assignPlacementsToDevices(app_deployment, deviceAssignments);

    // Allocate any usesdevice capacities specified in the SAD file; at this
    // point, the complete set of component deployments is known, including any
    // property overrides for allocation context
    _handleUsesDevices(app_deployment, name);

    // Assign CPU reservations to components
    app_deployment.applyCpuReservations(specialized_reservations);

    ////////////////////////////////////////////////
    // Create the Application servant
    _application = new Application_impl(app_deployment.getIdentifier(),
                                        name, 
                                        _appFact._softwareProfile, 
                                        _appFact._domainManager, 
                                        _waveformContextName, 
                                        _waveformContext,
                                        aware_application,
                                        _domainContext);

    // Activate the new Application servant
    PortableServer::ObjectId_var oid = Application_impl::Activate(_application);

    std::vector<ConnectionNode> connections;
    std::vector<std::string> allocationIDs;

    CF::ApplicationRegistrar_var app_reg = _application->appReg();
    loadAndExecuteComponents(app_deployment.getComponentDeployments(), app_reg);
    waitForComponentRegistration(app_deployment.getComponentDeployments());
    initializeComponents(app_deployment.getComponentDeployments());

    // Check that the assembly controller is valid
    LOG_TRACE(ApplicationFactory_impl, "Checking assembly controller");
    ossie::ComponentDeployment* ac_deployment = app_deployment.getAssemblyController();
    if (!ac_deployment) {
        const char* message = "Assembly controller has not been assigned";
        LOG_ERROR(ApplicationFactory_impl, message);
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, message);
    }
    CF::Resource_var assemblyController = assemblyController = ac_deployment->getResourcePtr();
    if (CORBA::is_nil(assemblyController) && ac_deployment->getSoftPkg()->isScaCompliant()) {
        const char* message = "Assembly controller has not registered with the application";
        LOG_ERROR(ApplicationFactory_impl, message);
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, message);
    }

    _connectComponents(app_deployment, connections);
    _configureComponents(app_deployment.getComponentDeployments());

    setUpExternalPorts(app_deployment, _application);
    setUpExternalProperties(app_deployment, _application);

    ////////////////////////////////////////////////
    // Create the application
    //
    // We are assuming that all components and their resources are 
    // collocated. This means that we assume the SAD <partitioning> 
    // element contains the <hostcollocation> element. NB: Ownership 
    // of the ConnectionManager is passed to the application.
    _allocations.transfer(allocationIDs);

    // Fill in the uses devices for the application
    CF::DeviceAssignmentSequence app_devices;
    typedef std::vector<ossie::UsesDeviceAssignment*> UsesList;
    const UsesList& app_uses = app_deployment.getUsesDeviceAssignments();
    for (UsesList::const_iterator uses = app_uses.begin(); uses != app_uses.end(); ++uses) {
        CF::DeviceAssignmentType assignment;
        assignment.componentId = CORBA::string_dup(name);
        std::string deviceId;
        try {
            deviceId = ossie::corba::returnString((*uses)->getAssignedDevice()->identifier());
        } catch (...) {
        }
        assignment.assignedDeviceId = deviceId.c_str();
        ossie::corba::push_back(app_devices, assignment);
    }

    const DeploymentList& deployments = app_deployment.getComponentDeployments();    
    for (DeploymentList::const_iterator dep = deployments.begin(); dep != deployments.end(); ++dep) {
        CF::DeviceAssignmentType comp_assignment;
        comp_assignment.componentId = (*dep)->getIdentifier().c_str();
        comp_assignment.assignedDeviceId = (*dep)->getAssignedDevice()->identifier.c_str();
        ossie::corba::push_back(app_devices, comp_assignment);

        const UsesList& dep_uses = (*dep)->getUsesDeviceAssignments();
        for (UsesList::const_iterator uses = dep_uses.begin(); uses != dep_uses.end(); ++uses) {
            CF::DeviceAssignmentType assignment;
            assignment.componentId = (*dep)->getIdentifier().c_str();
            std::string deviceId;
            try {
                deviceId = ossie::corba::returnString((*uses)->getAssignedDevice()->identifier());
            } catch (...) {
            }
            assignment.assignedDeviceId = deviceId.c_str();
            ossie::corba::push_back(app_devices, assignment);
        }
    }

    std::vector<CF::Resource_var> start_order = getStartOrder(app_deployment.getComponentDeployments());
    _application->populateApplication(
        assemblyController,
        app_devices, 
        start_order, 
        connections, 
        allocationIDs);

    // Add a reference to the new application to the 
    // ApplicationSequence in DomainManager
    CF::Application_var appObj = _application->_this();
    try {
        _appFact._domainManager->addApplication(_application);
    } catch (CF::DomainManager::ApplicationInstallationError& ex) {
        // something bad happened - clean up
        LOG_ERROR(ApplicationFactory_impl, ex.msg);
        throw CF::ApplicationFactory::CreateApplicationError(ex.errorNumber, ex.msg);
    }

    // After all components have been deployed, we know that the first
    // executable device in the list was used for the last deployment,
    // so update the domain manager
    _appFact._domainManager->setLastDeviceUsedForDeployment(_executableDevices.front()->identifier);

    if ( _appFact._domainManager ) {
      _appFact._domainManager->sendAddEvent( _appFact._identifier.c_str(), 
                                             app_deployment.getIdentifier().c_str(), 
                                             name,
                                             appObj,
                                             StandardEvent::APPLICATION);
    }

    LOG_INFO(ApplicationFactory_impl, "Done creating application " << app_deployment.getIdentifier() << " " << name);
    _isComplete = true;
    return appObj._retn();
}

CF::AllocationManager::AllocationResponseSequence* createHelper::allocateUsesDeviceProperties(const std::vector<UsesDevice>& usesDevices, const CF::Properties& configureProperties)
{
    CF::AllocationManager::AllocationRequestSequence request;
    request.length(usesDevices.size());
    
    for (unsigned int usesdev_idx=0; usesdev_idx< usesDevices.size(); usesdev_idx++) {
        const std::string requestid = usesDevices[usesdev_idx].getID();
        request[usesdev_idx].requestID = requestid.c_str();

        // Get the usesdevice dependency properties
        CF::Properties& allocationProperties = request[usesdev_idx].allocationProperties;
        const std::vector<PropertyRef>&prop_refs = usesDevices[usesdev_idx].getDependencies();
        this->_castRequestProperties(allocationProperties, prop_refs);
        
        this->_evaluateMATHinRequest(allocationProperties, configureProperties);
    }
    
    return this->_allocationMgr->allocate(request);
}
                                                          
/* Check all allocation dependencies for a particular component and assign it to a device.
 *  - Check component's overall usesdevice dependencies
 *  - Allocate capacity on usesdevice(s)
 *  - Find and implementation that has it's implementation-specific usesdevice dependencies satisfied
 *  - Allocate the component to a particular device

 Current implementation takes advantage of single failure then clean up everything..... To support collocation
 allocation failover for mulitple devices, then we need to clean up only the allocations that we made during a failed
 collocation request.  This requires that we know and cleanup only those allocations that we made..

 */
void createHelper::allocateComponent(ossie::ApplicationDeployment& appDeployment,
                                     ossie::ComponentDeployment* deployment,
                                     const std::string& assignedDeviceId)
{
    redhawk::PropertyMap alloc_context = deployment->getAllocationContext();
    
    // Find the devices that allocate the SPD's minimum required usesdevices properties
    const std::vector<UsesDevice>& usesDevVec = deployment->getSoftPkg()->getUsesDevices();
    ossie::UsesDeviceDeployment assignedDevices;
    if (!allocateUsesDevices(usesDevVec, alloc_context, assignedDevices, this->_allocations)) {
        // There were unsatisfied usesdevices for the component
        ostringstream eout;
        eout << "Failed to satisfy 'usesdevice' dependencies ";
        bool first = true;
        for (std::vector<UsesDevice>::const_iterator uses = usesDevVec.begin(); uses != usesDevVec.end(); ++uses) {
            if (!assignedDevices.getUsesDeviceAssignment(uses->getID())) {
                if (!first) {
                    eout << ", ";
                } else {
                    first = false;
                }
                eout << uses->getID();
            }
        }
        eout << "for component '" << deployment->getIdentifier() << "'";
        LOG_DEBUG(ApplicationFactory_impl, eout.str());
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
    }

    // now attempt to find an implementation that can have it's allocation requirements met
    const SPD::Implementations& implementations = deployment->getSoftPkg()->getImplementations();
    for (size_t implCount = 0; implCount < implementations.size(); implCount++) {
        const ossie::SPD::Implementation* implementation = &implementations[implCount];

        // Handle 'usesdevice' dependencies for the particular implementation
        UsesDeviceDeployment implAssignedDevices;
        ScopedAllocations implAllocations(*this->_allocationMgr);
        const std::vector<UsesDevice>& implUsesDevVec = implementation->getUsesDevices();
        
        if (!allocateUsesDevices(implUsesDevVec, alloc_context, implAssignedDevices, implAllocations)) {
            LOG_DEBUG(ApplicationFactory_impl, "Unable to satisfy 'usesdevice' dependencies for component "
                      << deployment->getIdentifier() << " implementation " << implementation->getID());
            continue;
        }

        deployment->setImplementation(implementation);

        // Transfer ownership of the uses device assigments to the deployment
        assignedDevices.transferUsesDeviceAssignments(*deployment);
        
        // Found an implementation which has its 'usesdevice' dependencies
        // satisfied, now perform assignment/allocation of component to device
        LOG_DEBUG(ApplicationFactory_impl, "Trying to find the device");
        ossie::AllocationResult response = allocateComponentToDevice(deployment, assignedDeviceId,
                                                                     appDeployment.getIdentifier());
        
        if (response.first.empty()) {
            LOG_DEBUG(ApplicationFactory_impl, "Unable to allocate device for component "
                      << deployment->getIdentifier() << " implementation " << implementation->getID());
            continue;
        }
        
        // Track successful deployment allocation
        implAllocations.push_back(response.first);
        
        // Convert from response back into a device node
        deployment->setAssignedDevice(response.second);
        DeviceNode& node = *(response.second);
        const std::string& deviceId = node.identifier;
        
        if (!resolveSoftpkgDependencies(appDeployment, deployment, node)) {
            LOG_DEBUG(ApplicationFactory_impl, "Unable to resolve softpackage dependencies for component "
                      << deployment->getIdentifier() << " implementation " << implementation->getID());
            continue;
        }
        
        // Allocation to a device succeeded
        LOG_DEBUG(ApplicationFactory_impl, "Assigned component " << deployment->getInstantiation()->getID()
                  << " implementation " << implementation->getID() << " to device " << deviceId);

        // Move the device to the front of the list
        rotateDeviceList(_executableDevices, deviceId);
        
        // Store the implementation-specific usesdevice allocations and
        // device assignments
        implAllocations.transfer(this->_allocations);

        implAssignedDevices.transferUsesDeviceAssignments(*deployment);
        
        return;
    }

    bool allBusy = true;
    for (ossie::DeviceList::iterator dev = _executableDevices.begin(); dev != _executableDevices.end(); ++dev) {
        CF::Device::UsageType state;
        try {
            state = (*dev)->device->usageState();
        } catch (...) {
            LOG_WARN(ApplicationFactory_impl, "Device " << (*dev)->identifier << " is not reachable");
            continue;
        }
        if (state != CF::Device::BUSY) {
            allBusy = false;
            break;
        }
    }
    if (allBusy) {
        // Report failure
        std::ostringstream eout;
        eout << "Unable to launch component '"<<deployment->getSoftPkg()->getName()<<"'. All executable devices (i.e.: GPP) in the Domain are busy";
        LOG_DEBUG(ApplicationFactory_impl, eout.str());
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
    }

    // Report failure
    std::ostringstream eout;
    eout << "Failed to satisfy device dependencies for component: '";
    eout << deployment->getSoftPkg()->getName() << "' with component id: '" << deployment->getIdentifier() << "'";
    LOG_DEBUG(ApplicationFactory_impl, eout.str());
    throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
}

bool createHelper::allocateUsesDevices(const std::vector<UsesDevice>& usesDevices,
                                       const CF::Properties& configureProperties,
                                       ossie::UsesDeviceDeployment& deviceAssignments,
                                       ScopedAllocations& allocations)
{
    // Create a temporary lookup table for reconciling allocation requests with
    // usesdevice identifiers
    typedef std::map<std::string,const UsesDevice*> UsesDeviceMap;
    UsesDeviceMap usesDeviceMap;
    for (std::vector<UsesDevice>::const_iterator iter = usesDevices.begin(); iter != usesDevices.end(); ++iter) {
        // Ensure that no devices are assigned to start; the caller can check
        // for unassigned devices to report which usesdevices failed
        usesDeviceMap[iter->getID()] = &(*iter);
    }
    
    // Track allocations made internally, either to clean up on failure or to
    // pass to the caller
    ScopedAllocations localAllocations(*_allocationMgr);
    
    CF::AllocationManager::AllocationResponseSequence_var response = allocateUsesDeviceProperties(usesDevices, configureProperties);
    for (unsigned int resp = 0; resp < response->length(); resp++) {
        // Ensure that this allocation is recorded so that it can be cleaned up
        const std::string allocationId(response[resp].allocationID);
        LOG_TRACE(ApplicationFactory_impl, "Allocated " << allocationId);
        localAllocations.push_back(allocationId);
        
        // Find the usesdevice that matches the request and update it, removing
        // the key from the map
        const std::string requestID(response[resp].requestID);
        UsesDeviceMap::iterator uses = usesDeviceMap.find(requestID);
        if (uses == usesDeviceMap.end()) {
            // This condition should never occur
            LOG_WARN(ApplicationFactory_impl, "Allocation request " << requestID
                     << " does not match any usesdevice");
            continue;
        }
        const std::string deviceId = ossie::corba::returnString(response[resp].allocatedDevice->identifier());
        usesDeviceMap.erase(uses);

        ossie::UsesDeviceAssignment* assignment = new ossie::UsesDeviceAssignment(uses->second);
        assignment->setAssignedDevice(response[resp].allocatedDevice);
        deviceAssignments.addUsesDeviceAssignment(assignment);
    }
    
    if (usesDeviceMap.empty()) {
        // All usesdevices were satisfied; give the caller ownership of all the
        // allocations
        localAllocations.transfer(allocations);
        return true;
    } else {
        // Some usesdevices were not satisfied--these will have no assigned
        // device id; successful allocations will be deallocated when the
        // ScopedAllocations goes out of scope
        return false;
    }
}

void createHelper::_evaluateMATHinRequest(CF::Properties &request, const CF::Properties &configureProperties)
{
    for (unsigned int math_prop=0; math_prop<request.length(); math_prop++) {
        CF::Properties *tmp_prop;
        if (request[math_prop].value >>= tmp_prop) {
            this->_evaluateMATHinRequest(*tmp_prop, configureProperties);
            request[math_prop].value <<= *tmp_prop;
            continue;
        }
        std::string value = ossie::any_to_string(request[math_prop].value);
        if (value.find("__MATH__") != string::npos) {
            // Turn propvalue into a string for easy parsing
            std::string mathStatement = value.substr(8);
            if ((*mathStatement.begin() == '(') && (*mathStatement.rbegin() == ')')) {
                mathStatement.erase(mathStatement.begin(), mathStatement.begin() + 1);
                mathStatement.erase(mathStatement.end() - 1, mathStatement.end());
                std::vector<std::string> args;
                while ((mathStatement.length() > 0) && (mathStatement.find(',') != std::string::npos)) {
                    LOG_TRACE(ApplicationFactory_impl, "__MATH__ ARG: " << mathStatement.substr(0, mathStatement.find(',')) );
                    args.push_back(mathStatement.substr(0, mathStatement.find(',')));
                    mathStatement.erase(0, mathStatement.find(',') + 1);
                }
                args.push_back(mathStatement);

                if (args.size() != 3) {
                    std::ostringstream eout;
                    eout << " invalid __MATH__ statement; '" << mathStatement << "'";
                    throw ossie::PropertyMatchingError(eout.str());
                }

                double operand = strtod(args[0].c_str(), NULL);
                if (args[0].size() == 0) {
                    std::ostringstream eout;
                    eout << " invalid __MATH__ argument (argument empty);";
                    throw ossie::PropertyMatchingError(eout.str());
                }
                if (not std::isdigit(args[0][0])) { // if the first character is not numeric, then cannot apply __MATH__
                    std::ostringstream eout;
                    eout << " invalid __MATH__ argument; '" << args[0] << "'";
                    if (args[0][0] != '.') {
                        throw ossie::PropertyMatchingError(eout.str());
                    }
                    if (args[0].size() == 1) { // the string is only '.'
                        throw ossie::PropertyMatchingError(eout.str());
                    }
                    if (not std::isdigit(args[0][1])) { // the string starts with '.' but is not followed by a number
                        throw ossie::PropertyMatchingError(eout.str());
                    }
                }

                // See if there is a property in the component
                const CF::DataType* matchingCompProp = 0;
                for (unsigned int j = 0; j < configureProperties.length(); j++) {
                    if (strcmp(configureProperties[j].id, args[1].c_str()) == 0) {
                        matchingCompProp = &configureProperties[j];
                    }
                }

                CF::Properties *tmp_prop;
                if (matchingCompProp == 0) {
                    // see if it's in a struct
                    for (unsigned int j = 0; j < configureProperties.length(); j++) {
                        if (configureProperties[j].value >>= tmp_prop) {
                            for (unsigned int jj = 0; jj < (*tmp_prop).length(); jj++) {
                                if (strcmp((*tmp_prop)[jj].id, args[1].c_str()) == 0) {
                                    matchingCompProp = &(*tmp_prop)[jj];
                                    break;
                                }
                            }
                        }
                        if (matchingCompProp != 0)
                            break;
                    }
                }

                if (matchingCompProp == 0) {
                    std::ostringstream eout;
                    eout << " failed to match component property in __MATH__ statement; property id = " << args[1] << " does not exist in component as a configure property";
                    throw ossie::PropertyMatchingError(eout.str());
                }

                std::string math = args[2];
                CORBA::Any compValue = matchingCompProp->value;
                CORBA::TypeCode_var matchingCompPropType = matchingCompProp->value.type();
                request[math_prop].value = ossie::calculateDynamicProp(operand, compValue, math, matchingCompPropType->kind());
                std::string retval = ossie::any_to_string(request[math_prop].value);
                LOG_DEBUG(ApplicationFactory_impl, "__MATH__ RESULT: " << retval << " op1: " << operand << " op2:" << ossie::any_to_string(compValue) );
            } else {
                std::ostringstream eout;
                eout << " invalid __MATH__ statement; '" << mathStatement << "'";
                throw ossie::PropertyMatchingError(eout.str());
            }
        }
    }
}

/* Perform allocation/assignment of a particular component to the device.
 *  - First do allocation/assignment based on user provided DAS
 *  - If not specified in DAS, then iterate through devices looking for a device that satisfies
 *    the allocation properties
 */
ossie::AllocationResult createHelper::allocateComponentToDevice(ossie::ComponentDeployment* deployment,
                                              const std::string& assignedDeviceId,
                                              const std::string& appIdentifier)
{
    const ossie::SPD::Implementation* implementation = deployment->getImplementation();
    ossie::DeviceList devices = _registeredDevices;

    // First check to see if the component was assigned in the user provided DAS
    // See if a device was assigned in the DAS
    if (!assignedDeviceId.empty()) {
        LOG_TRACE(ApplicationFactory_impl, "User-provided DAS: Component: '" << deployment->getSoftPkg()->getName() <<
                  "'  Assigned device: '" << assignedDeviceId << "'");
        ossie::DeviceList::iterator device;
        for (device = devices.begin(); device != devices.end(); ++device) {
            if (assignedDeviceId == (*device)->identifier) {
                break;
            }
        }

        if (device == devices.end()) {
            LOG_DEBUG(ApplicationFactory_impl, "DAS specified unknown device " << assignedDeviceId <<
                      " for component " << deployment->getIdentifier());
            CF::DeviceAssignmentSequence badDAS;
            badDAS.length(1);
            badDAS[0].componentId = deployment->getIdentifier().c_str();
            badDAS[0].assignedDeviceId = assignedDeviceId.c_str();
            throw CF::ApplicationFactory::CreateApplicationRequestError(badDAS);
        }

        // Remove all non-requested devices
        devices.erase(devices.begin(), device++);
        devices.erase(device, devices.end());
    }

    const std::string requestid = ossie::generateUUID();
    const std::vector<PropertyRef>& prop_refs = implementation->getDependencies();
    redhawk::PropertyMap allocationProperties;
    this->_castRequestProperties(allocationProperties, prop_refs);

    redhawk::PropertyMap alloc_context = deployment->getAllocationContext();
    this->_evaluateMATHinRequest(allocationProperties, alloc_context);
    
    LOG_TRACE(ApplicationFactory_impl, "alloc prop size " << allocationProperties.size() );
    redhawk::PropertyMap::iterator iter=allocationProperties.begin();
    for( ; iter != allocationProperties.end(); iter++){
      LOG_TRACE(ApplicationFactory_impl, "alloc prop: " << iter->id  <<" value:" <<  ossie::any_to_string(iter->value) );
    }
    
    redhawk::PropertyMap::iterator nic_alloc = allocationProperties.find("nic_allocation");
    std::string alloc_id;
    if (nic_alloc != allocationProperties.end()) {
        redhawk::PropertyMap& substr = nic_alloc->getValue().asProperties();
        alloc_id = substr["nic_allocation::identifier"].toString();
        if (alloc_id.empty()) {
          alloc_id = ossie::generateUUID();
          substr["nic_allocation::identifier"] = alloc_id;
        }
    }
    
    ossie::AllocationResult response = this->_allocationMgr->allocateDeployment(requestid, allocationProperties, devices, appIdentifier, implementation->getProcessors(), implementation->getOsDeps());
    if (allocationProperties.contains("nic_allocation")) {
        if (!response.first.empty()) {
            redhawk::PropertyMap query_props;
            query_props["nic_allocation_status"] = redhawk::Value();
            response.second->device->query(query_props);
            redhawk::ValueSequence& retstruct = query_props["nic_allocation_status"].asSequence();
            for (redhawk::ValueSequence::iterator it = retstruct.begin(); it!=retstruct.end(); it++) {
                redhawk::PropertyMap& struct_prop = it->asProperties();
                std::string identifier = struct_prop["nic_allocation_status::identifier"].toString();
                if (identifier == alloc_id) {
                    const std::string interface = struct_prop["nic_allocation_status::interface"].toString();
                    LOG_DEBUG(ApplicationFactory_impl, "Allocation NIC assignment: " << interface );
                    deployment->setNicAssignment(interface);
                }
            }
        }
    }
    TRACE_EXIT(ApplicationFactory_impl);
    return response;
}

void createHelper::_castRequestProperties(CF::Properties& allocationProperties, const std::vector<ossie::PropertyRef> &prop_refs, unsigned int offset)
{
    allocationProperties.length(offset+prop_refs.size());
    for (unsigned int i=0; i<prop_refs.size(); i++) {
        allocationProperties[offset+i] = ossie::convertPropertyRefToDataType(prop_refs[i].property.get());
    }
}

bool createHelper::resolveSoftpkgDependencies(ossie::ApplicationDeployment& appDeployment,
                                              ossie::SoftpkgDeployment* deployment,
                                              ossie::DeviceNode& device)
{
    const ossie::SPD::Implementation* implementation = deployment->getImplementation();
    const SPD::SoftPkgDependencies& deps = implementation->getSoftPkgDependencies();
    SPD::SoftPkgDependencies::const_iterator iterSoftpkg;

    for (iterSoftpkg = deps.begin(); iterSoftpkg != deps.end(); ++iterSoftpkg) {
        // Find an implementation whose dependencies match
        ossie::SoftpkgDeployment* dependency = resolveDependencyImplementation(appDeployment, *iterSoftpkg, device);
        if (dependency) {
            deployment->addDependency(dependency);
        } else {
            LOG_DEBUG(ApplicationFactory_impl, "resolveSoftpkgDependencies: implementation match not found between soft package dependency and device");
            return false;
        }
    }

    return true;
}

ossie::SoftpkgDeployment* createHelper::resolveDependencyImplementation(ossie::ApplicationDeployment& appDeployment,
                                                                        const ossie::SPD::SoftPkgRef& ref,
                                                                        ossie::DeviceNode& device)
{
    LOG_TRACE(ApplicationFactory_impl, "Resolving dependency " << ref);
    const SoftPkg* softpkg = appDeployment.getSoftPkg(ref.localfile);
    const SPD::Implementations& spd_list = softpkg->getImplementations();

    for (size_t implCount = 0; implCount < spd_list.size(); implCount++) {
        const ossie::SPD::Implementation& implementation = spd_list[implCount];
        if (ref.implref.isSet() && (implementation.getID() != *ref.implref)) {
            continue;
        }

        // Check that this implementation can run on the device
        if (!checkProcessor(implementation.getProcessors(), device.prf.getAllocationProperties())) {
            continue;
        } else if (!checkOs(implementation.getOsDeps(), device.prf.getAllocationProperties())) {
            continue;
        }

        ossie::SoftpkgDeployment* dependency = new ossie::SoftpkgDeployment(softpkg, &implementation);
        // Recursively check any softpkg dependencies
        if (resolveSoftpkgDependencies(appDeployment, dependency, device)) {
            return dependency;
        }
        delete dependency;
    }

    return 0;
}

/* Given a waveform/application name, return a unique waveform naming context
 *  - Returns a unique waveform naming context
 *  THIS FUNCTION IS NOT THREAD SAFE
 */
string ApplicationFactory_impl::getWaveformContextName(string name )
{
    //
    // Find a new unique waveform naming for the naming context
    //


    bool found_empty = false;
    string waveform_context_name;

    // iterate through N for waveformname_N until a unique naming context if found
    CosNaming::NamingContext_ptr inc = ossie::corba::InitialNamingContext();
    do {
        ++_lastWaveformUniqueId;
        // Never use 0
        if (_lastWaveformUniqueId == 0) ++_lastWaveformUniqueId;
        waveform_context_name = "";
        waveform_context_name.append(name);
        std::string mod_waveform_context_name = waveform_context_name;
        for (int i=mod_waveform_context_name.size()-1;i>=0;i--) {
            if (mod_waveform_context_name[i]=='.') {
                mod_waveform_context_name.insert(i, 1, '\\');
            }
        }
        waveform_context_name.append("_");
        mod_waveform_context_name.append("_");
        ostringstream number_str;
        number_str << _lastWaveformUniqueId;
        waveform_context_name.append(number_str.str());
        mod_waveform_context_name.append(number_str.str());
        string temp_waveform_context(_domainName + string("/"));
        temp_waveform_context.append(mod_waveform_context_name);
        CosNaming::Name_var cosName = ossie::corba::stringToName(temp_waveform_context);
        try {
            CORBA::Object_var obj_WaveformContext = inc->resolve(cosName);
        } catch (const CosNaming::NamingContext::NotFound&) {
            found_empty = true;
        }
    } while (!found_empty);

    return waveform_context_name;

}

/* Given a waveform/application-specific context, return the full waveform naming context
 *  - Returns a full context path for the waveform
 */
string ApplicationFactory_impl::getBaseWaveformContext(string waveform_context)
{
    string base_naming_context(_domainName + string("/"));
    base_naming_context.append(waveform_context);

    return base_naming_context;
}

/* Perform 'load' and 'execute' operations to launch component on the assigned device
 *  - Actually loads and executes the component on the given device
 */
void createHelper::loadAndExecuteComponents(const DeploymentList& deployments,
                                            CF::ApplicationRegistrar_ptr _appReg)
{
    LOG_TRACE(ApplicationFactory_impl, "Loading and Executing " << deployments.size() << " components");
    // apply application affinity options to required components
    applyApplicationAffinityOptions(deployments);

    for (unsigned int rc_idx = 0; rc_idx < deployments.size (); rc_idx++) {
        ossie::ComponentDeployment* deployment = deployments[rc_idx];
        const ossie::SoftPkg* softpkg = deployment->getSoftPkg();
        const ossie::ComponentInstantiation* instantiation = deployment->getInstantiation();
        const ossie::SPD::Implementation* implementation = deployment->getImplementation();

        boost::shared_ptr<ossie::DeviceNode> device = deployment->getAssignedDevice();
        if (!device) {
            std::ostringstream message;
            message << "component " << deployment->getIdentifier() << " was not assigned to a device";
            throw std::logic_error(message.str());
        }

        LOG_TRACE(ApplicationFactory_impl, "Component - " << softpkg->getName()
                  << "   Assigned device - " << device->identifier);
        LOG_INFO(ApplicationFactory_impl, "APPLICATION: " << _waveformContextName << " COMPONENT ID: " 
                 << component->getIdentifier()  << " ASSIGNED TO DEVICE ID/LABEL: " << device->identifier << "/" << device->label);

        // Let the application know to expect the given component
        _application->addComponent(deployment->getIdentifier(), softpkg->getSPDFile());
        _application->setComponentImplementation(deployment->getIdentifier(), implementation->getID());
        if (instantiation->isNamingService()) {
            std::string lookupName = _baseNamingContext + "/" + instantiation->getFindByNamingServiceName();
            _application->setComponentNamingContext(deployment->getIdentifier(), lookupName);
        }
        _application->setComponentDevice(deployment->getIdentifier(), device->device);

        // get the code.localfile
        LOG_TRACE(ApplicationFactory_impl, "Host is " << device->label << " Local file name is "
                  << deployment->getLocalFile());

        // Get file name, load if it is not empty
        std::string codeLocalFile = deployment->getLocalFile();
        if (codeLocalFile.empty()) {
            ostringstream eout;
            eout << "code.localfile is empty for component: '";
            eout << softpkg->getName();
            eout << "' with component id: '" << deployment->getIdentifier() << "' ";
            eout << " with implementation id: '" << implementation->getID() << "'";
            eout << " on device id: '" << device->identifier << "'";
            eout << " in waveform '" << _waveformContextName<<"'";
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            LOG_TRACE(ApplicationFactory_impl, eout.str())
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EBADF, eout.str().c_str());
        }

        // narrow to LoadableDevice interface
        CF::LoadableDevice_var loadabledev = ossie::corba::_narrowSafe<CF::LoadableDevice>(device->device);
        if (CORBA::is_nil(loadabledev)) {
            std::ostringstream message;
            message << "component " << deployment->getIdentifier() << " was assigned to non-loadable device "
                    << device->identifier;
            LOG_ERROR(ApplicationFactory_impl, message);
            throw std::logic_error(message.str());
        }

        LOG_TRACE(ApplicationFactory_impl, "Loading " << codeLocalFile << " and dependencies on device "
                  << device->label);
        try {
            deployment->load(_application, _appFact._fileMgr, loadabledev);
        } catch (const std::exception& exc) {
            std::ostringstream message;
            message << "Unable to load component " << softpkg->getName()
                    << " implementation " << implementation->getID()
                    << " on device " << device->identifier
                    << ": " << exc.what();
            LOG_ERROR(ApplicationFactory_impl, message.str());
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, message.str().c_str());
        }
                
        if (deployment->isExecutable()) {
            attemptComponentExecution(_appReg, deployment);
        }
    }
}

std::string createHelper::resolveLoggingConfiguration(ossie::ComponentDeployment* deployment)
{
    // Use the log config resolver (if enabled)
    const ossie::ComponentInstantiation* instantiation = deployment->getInstantiation();
    if (_appFact._domainManager->getUseLogConfigResolver()) {
        ossie::logging::LogConfigUriResolverPtr logcfg_resolver = ossie::logging::GetLogConfigUriResolver();
        if (logcfg_resolver) {
            std::string logcfg_path = ossie::logging::GetComponentPath(_appFact._domainName, _waveformContextName,
                                                                       instantiation->getFindByNamingServiceName());
            std::string uri = logcfg_resolver->get_uri(logcfg_path);
            LOG_DEBUG(ApplicationFactory_impl, "Using LogConfigResolver plugin: path " << logcfg_path
                      << " logcfg:" << uri );
            if (!uri.empty()) {
                return uri;
            }
        }
    }

    // Ask the component for its configuration
    std::string logging_uri = deployment->getLoggingConfiguration();
    if (!logging_uri.empty()) {
        LOG_TRACE(ApplicationFactory_impl, "Resource logging configuration provided, logcfg:" << logging_uri);
        return logging_uri;
    }

    // Query the DomainManager for the logging configuration
    LOG_TRACE(ApplicationFactory_impl, "Checking DomainManager for LOGGING_CONFIG_URI");
    PropertyInterface* log_prop = _appFact._domainManager->getPropertyFromId("LOGGING_CONFIG_URI");
    StringProperty* logProperty = dynamic_cast<StringProperty*>(log_prop);
    if (!logProperty->isNil()) {
        logging_uri = logProperty->getValue();
    } else {
        LOG_TRACE(ApplicationFactory_impl, "DomainManager LOGGING_CONFIG_URI is not set");
    }

    return logging_uri;
}

void createHelper::attemptComponentExecution (CF::ApplicationRegistrar_ptr registrar,
                                              ossie::ComponentDeployment* deployment)
{
    const ossie::SPD::Implementation* implementation = deployment->getImplementation();
    const ossie::SoftPkg* softpkg = deployment->getSoftPkg();

    // Get executable device reference
    boost::shared_ptr<DeviceNode> device = deployment->getAssignedDevice();
    CF::ExecutableDevice_var execdev = ossie::corba::_narrowSafe<CF::ExecutableDevice>(device->device);
    if (CORBA::is_nil(execdev)){
        std::ostringstream message;
        message << "component " << deployment->getIdentifier() << " was assigned to non-executable device "
                << device->identifier;
        throw std::logic_error(message.str());
    }

    // Build up the list of command line parameters
    redhawk::PropertyMap execParameters = deployment->getCommandLineParameters();
    const std::string& nic = deployment->getNicAssignment();
    if (!nic.empty()) {
        execParameters["NIC"] = nic;
    }

    // Add specialized CPU reservation if given
    if (deployment->hasCpuReservation()) {
        execParameters["RH::GPP::MODIFIED_CPU_RESERVATION_VALUE"] = deployment->getCpuReservation();
    }

    // Add the required parameters specified in SR:163
    // Naming Context IOR, Name Binding, and component identifier
    execParameters["COMPONENT_IDENTIFIER"] = deployment->getIdentifier();
    if (deployment->getInstantiation()->isNamingService()) {
        execParameters["NAME_BINDING"] = deployment->getInstantiation()->getFindByNamingServiceName();
    }
    execParameters["DOM_PATH"] = _baseNamingContext;
    execParameters["PROFILE_NAME"] = softpkg->getSPDFile();

    // Pass logging configuration
    std::string logging_uri = resolveLoggingConfiguration(deployment);
    if (!logging_uri.empty()) {
        // Check for sca: URI type, and append the IOR for the file system
        if (logging_uri.find("sca:/") == 0) {
            string ior = ossie::corba::objectToString(_appFact._domainManager->_fileMgr);
            logging_uri += ("?fs=" + ior);
            LOG_TRACE(ApplicationFactory_impl, "Adding file system IOR " << logging_uri);
        }
        LOG_DEBUG(ApplicationFactory_impl, " LOGGING_CONFIG_URI: " << logging_uri);
        execParameters["LOGGING_CONFIG_URI"] = logging_uri;
    } else {
        // No LOGGING_CONFIG_URI can be found, pass DEBUG_LEVEL
        rh_logger::LoggerPtr dom_logger = _appFact._domainManager->getLogger();
        if (dom_logger) {
            rh_logger::LevelPtr dlevel = dom_logger->getLevel();
            if (!dlevel) {
                dlevel = rh_logger::Logger::getRootLogger()->getLevel();
            }
            execParameters["DEBUG_LEVEL"] = static_cast<CORBA::Long>(ossie::logging::ConvertRHLevelToDebug(dlevel));
        }
    }

    // Add the Naming Context IOR last to make it easier to parse the command line
    execParameters["NAMING_CONTEXT_IOR"] = ossie::corba::objectToString(registrar);

    // Get entry point
    std::string entryPoint = deployment->getEntryPoint();
    if (entryPoint.empty()) {
        LOG_WARN(ApplicationFactory_impl, "executing using code file as entry point; this is non-SCA compliant behavior; entrypoint must be set");
        entryPoint = deployment->getLocalFile();
    }

    // Get the complete list of dependencies to include in executeLinked
    std::vector<std::string> resolved_softpkg_deps = deployment->getDependencyLocalFiles();
    CF::StringSequence dep_seq;
    dep_seq.length(resolved_softpkg_deps.size());
    for (unsigned int p=0;p!=dep_seq.length();p++) {
        dep_seq[p]=CORBA::string_dup(resolved_softpkg_deps[p].c_str());
    }

    CF::ExecutableDevice::ProcessID_Type tempPid = -1;

    // attempt to execute the component
    try {
        LOG_TRACE(ApplicationFactory_impl, "executing " << entryPoint << " on device " << device->label);
        for (redhawk::PropertyMap::iterator prop = execParameters.begin(); prop != execParameters.end(); ++prop) {
            LOG_TRACE(ApplicationFactory_impl, " exec param " << prop->getId() << " " << prop->getValue().toString());
        }

        // Get options list
        redhawk::PropertyMap options = deployment->getOptions(); 
        for (redhawk::PropertyMap::iterator opt = options.begin(); opt != options.end(); ++opt) {
            LOG_TRACE(ApplicationFactory_impl, " RESOURCE OPTION: " << opt->getId()
                      << " " << opt->getValue().toString());
        }

        // call 'execute' on the ExecutableDevice to execute the component
        tempPid = execdev->executeLinked(entryPoint.c_str(), options, execParameters, dep_seq);
    } catch( CF::InvalidFileName& _ex ) {
        std::string added_message = this->createVersionMismatchMessage(component_version);
        ostringstream eout;
        eout << "InvalidFileName when calling 'execute' on device with device id: '" << device->identifier << "' for component: '";
        eout << softpkg->getName() << "' with component id: '" << deployment->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getID() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " with error: <" << _ex.msg << ">;";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } catch( CF::Device::InvalidState& _ex ) {
        std::string added_message = this->createVersionMismatchMessage(component_version);
        ostringstream eout;
        eout << "InvalidState when calling 'execute' on device with device id: '" << device->identifier << "' for component: '";
        eout << softpkg->getName() << "' with component id: '" << deployment->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getID() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " with error: <" << _ex.msg << ">;";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } catch( CF::ExecutableDevice::InvalidParameters& _ex ) {
        std::string added_message = this->createVersionMismatchMessage(component_version);
        ostringstream eout;
        eout << "InvalidParameters when calling 'execute' on device with device id: '" << device->identifier << "' for component: '";
        eout << softpkg->getName() << "' with component id: '" << deployment->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getID() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " with invalid params: <";
        for (unsigned int propIdx = 0; propIdx < _ex.invalidParms.length(); propIdx++){
            eout << "(" << _ex.invalidParms[propIdx].id << "," << ossie::any_to_string(_ex.invalidParms[propIdx].value) << ")";
        }
        eout << " > error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } catch( CF::ExecutableDevice::InvalidOptions& _ex ) {
        std::string component_version(component->spd.getSoftPkgType());
        std::string added_message = this->createVersionMismatchMessage(component_version);
        ostringstream eout;
        eout << "InvalidOptions when calling 'execute' on device with device id: '" << device->identifier << "' for component: '";
        eout << softpkg->getName() << "' with component id: '" << deployment->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getID() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " with invalid options: <";
        for (unsigned int propIdx = 0; propIdx < _ex.invalidOpts.length(); propIdx++){
            eout << "(" << _ex.invalidOpts[propIdx].id << "," << ossie::any_to_string(_ex.invalidOpts[propIdx].value) << ")";
        }
        eout << " > error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } catch (CF::ExecutableDevice::ExecuteFail& ex) {
        std::string added_message = this->createVersionMismatchMessage(component_version);
        ostringstream eout;
        eout << "ExecuteFail when calling 'execute' on device with device id: '" << device->identifier << "' for component: '";
        eout << softpkg->getName() << "' with component id: '" << deployment->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getID() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " with message: '" << ex.msg << "'";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } CATCH_THROW_LOG_ERROR(
            ApplicationFactory_impl, "Caught an unexpected error when calling 'execute' on device with device id: '"
            << device->identifier << "' for component: '" << softpkg->getName()
            << "' with component id: '" << deployment->getIdentifier() << "' "
            << " with implementation id: '" << implementation->getID() << "'"
            << " in waveform '" << _waveformContextName<<"'"
            << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__,
            CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, "Caught an unexpected error when calling 'execute' on device"));

    // handle pid output
    if (tempPid < 0) {
        std::string added_message = this->createVersionMismatchMessage(component_version);
        ostringstream eout;
        eout << added_message;
        eout << "Failed to 'execute' component for component: '";
        eout << softpkg->getName() << "' with component id: '" << deployment->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getID() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EAGAIN, eout.str().c_str());
    } else {
        _application->setComponentPid(deployment->getIdentifier(), tempPid);
    }
}


void createHelper::applyApplicationAffinityOptions(const DeploymentList& deployments)
{
    // RESOLVE - need SAD file directive to control this behavior.. i.e if promote_nic_to_affinity==true...
    // for now add nic assignment as application affinity to all components deployed by this device
    redhawk::PropertyMap app_affinity;
    for (DeploymentList::const_iterator dep = deployments.begin(); dep != deployments.end(); ++dep) {
        if ((*dep)->hasNicAssignment()) {
            app_affinity = (*dep)->getAffinityOptionsWithAssignment();
        }
    }

    if (!app_affinity.empty()) {
      // log deployments with application affinity 
      for ( uint32_t i=0; i < app_affinity.length(); i++ ) {
          CF::DataType dt = app_affinity[i];
          LOG_INFO(ApplicationFactory_impl, " Applying Application Affinity: directive id:"  <<  dt.id << "/" <<  ossie::any_to_string( dt.value )) ;
      }
    
      //
      // Promote NIC affinity for all components deployed on the same device
      //
      boost::shared_ptr<ossie::DeviceNode> deploy_on_device;
      for (unsigned int rc_idx = 0; rc_idx < deployments.size(); rc_idx++) {
          ossie::ComponentDeployment* deployment = deployments[rc_idx];
          if (!(deployment->getNicAssignment().empty())) {
              deploy_on_device = deployment->getAssignedDevice();
          }
      }

      if (deploy_on_device) {
          for (unsigned int rc_idx = 0; rc_idx < deployments.size (); rc_idx++) {
              ossie::ComponentDeployment* deployment = deployments[rc_idx];
              boost::shared_ptr<ossie::DeviceNode> dev = deployment->getAssignedDevice();
              // for matching device deployments then apply nic affinity settings
              if (dev->identifier == deploy_on_device->identifier) {
                  deployment->mergeAffinityOptions(app_affinity);
              }
          }
      }
    }
}


void createHelper::waitForComponentRegistration(const DeploymentList& deployments)
{
    // Wait for all components to be registered before continuing
    int componentBindingTimeout = _appFact._domainManager->getComponentBindingTimeout();
    LOG_TRACE(ApplicationFactory_impl, "Waiting " << componentBindingTimeout << "s for all components register");

    // Track only SCA-compliant components; non-compliant components will never
    // register with the application, nor do they need to be initialized
    std::set<std::string> expected_components;
    for (DeploymentList::const_iterator dep = deployments.begin(); dep != deployments.end(); ++dep) {
        if ((*dep)->getSoftPkg()->isScaCompliant()) {
            expected_components.insert((*dep)->getIdentifier());
        }
    }

    // Record current time, to measure elapsed time in the event of a failure
    time_t start = time(NULL);

    if (!_application->waitForComponents(expected_components, componentBindingTimeout)) {
        // For reference, determine much time has really elapsed.
        time_t elapsed = time(NULL)-start;
        LOG_ERROR(ApplicationFactory_impl, "Timed out waiting for component to bind to naming context (" << elapsed << "s elapsed)");
        ostringstream eout;
        for (unsigned int req_idx = 0; req_idx < deployments.size(); req_idx++) {
            ossie::ComponentDeployment* deployment = deployments[req_idx];
            if (expected_components.count(deployment->getIdentifier())) {
                eout << "Timed out waiting for component to register: '" << deployment->getSoftPkg()->getName()
                     << "' with component id: '" << deployment->getIdentifier()
                     << " assigned to device: '" << deployment->getAssignedDevice()->identifier;
                break;
            }
        }
        eout << " in waveform '" << _waveformContextName<<"';";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    }
}

/* Initializes the components
 *  - Make sure internal lists are up to date
 *  - Ensure components have started and are bound to Naming Service
 *  - Initialize each component
 */
void createHelper::initializeComponents(const DeploymentList& deployments)
{
    // Install the different components in the system
    LOG_TRACE(ApplicationFactory_impl, "initializing " << deployments.size() << " waveform components");

    for (unsigned int rc_idx = 0; rc_idx < deployments.size (); rc_idx++) {
        ossie::ComponentDeployment* deployment = deployments[rc_idx];
        const ossie::SoftPkg* softpkg = deployment->getSoftPkg();

        // If the component is non-SCA compliant then we don't expect anything beyond this
        if (!softpkg->isScaCompliant()) {
            LOG_TRACE(ApplicationFactory_impl, "Component is non SCA-compliant, continuing to next component");
            continue;
        }

        if (!deployment->isResource()) {
            LOG_TRACE(ApplicationFactory_impl, "Component is not a resource, continuing to next component");
            continue;
        }

        // Find the component on the Application
        const std::string componentId = deployment->getIdentifier();
        CORBA::Object_var objref = _application->getComponentObject(componentId);
        if (CORBA::is_nil(objref)) {
            ostringstream eout;
            eout << "No object found for component: '" << softpkg->getName()
                 << "' with component id: '" << componentId
                 << " assigned to device: '"<<deployment->getAssignedDevice()->identifier<<"'";
            eout << " in waveform '" << _waveformContextName<<"';";
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
        }

        CF::Resource_var resource = ossie::corba::_narrowSafe<CF::Resource>(objref);
        if (CORBA::is_nil(resource)) {
            ostringstream eout;
            eout << "CF::Resource::_narrow failed with Unknown Exception for component: '"
                 << softpkg->getName()
                 << "' with component id: '" << componentId
                 << " assigned to device: '"<<deployment->getAssignedDevice()->identifier<<"'";
            eout << " in waveform '" << _waveformContextName<<"';";
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
        }

        deployment->setResourcePtr(resource);

        int initAttempts=3;
        while ( initAttempts > 0 ) {
            initAttempts--;
            if ( ossie::corba::objectExists(resource) == true ) { initAttempts = 0; continue; }
            LOG_DEBUG(ApplicationFactory_impl, "Retrying component ping............ comp:" << deployment->getIdentifier() << " waveform: " << _waveformContextName);
            usleep(1000);
        }


        //
        // call resource's initializeProperties method to handle any properties required for construction
        //
        LOG_DEBUG(ApplicationFactory_impl, "Initialize properties for component " << componentId);
        if (deployment->isResource() && deployment->isConfigurable()) {
            redhawk::PropertyMap initProps = deployment->getInitializeProperties();
            CF::Properties partialStruct = ossie::getPartialStructs(initProps);
            if (partialStruct.length() != 0) {
                ostringstream eout;
                eout << "Failed to 'configure' Assembly Controller: '";
                eout << softpkg->getName() << "' with component id: '" << deployment->getIdentifier() << " assigned to device: '"<< deployment->getAssignedDevice()->identifier << "' ";
                eout << " in waveform '"<< _waveformContextName<<"';";
                eout <<  "This component contains structure"<<partialStruct[0].id<<" with a mix of defined and nil values.";
                LOG_ERROR(ApplicationFactory_impl, eout.str());
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
            }
            try {
                // Try to set the initial values for the component's properties
                resource->initializeProperties(initProps);
            } catch(CF::PropertySet::InvalidConfiguration& e) {
                ostringstream eout;
                eout << "Failed to initialize component properties: '";
                eout << softpkg->getName() << "' with component id: '" << deployment->getIdentifier() << " assigned to device: '"<<deployment->getAssignedDevice()->identifier << "' ";
                eout << " in waveform '"<< _waveformContextName<<"';";
                eout <<  "InvalidConfiguration with this info: <";
                eout << e.msg << "> for these invalid properties: ";
                for (unsigned int propIdx = 0; propIdx < e.invalidProperties.length(); propIdx++){
                    eout << "(" << e.invalidProperties[propIdx].id << ",";
                    eout << ossie::any_to_string(e.invalidProperties[propIdx].value) << ")";
                }
                eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                LOG_ERROR(ApplicationFactory_impl, eout.str());
                throw CF::ApplicationFactory::InvalidInitConfiguration(e.invalidProperties);
            } catch(CF::PropertySet::PartialConfiguration& e) {
                ostringstream eout;
                eout << "Failed to initialize component properties: '";
                eout << softpkg->getName() << "' with component id: '" << deployment->getIdentifier() << " assigned to device: '"<<deployment->getAssignedDevice()->identifier << "' ";
                eout << " in waveform '"<< _waveformContextName<<"';";
                eout << "PartialConfiguration for these invalid properties: ";
                for (unsigned int propIdx = 0; propIdx < e.invalidProperties.length(); propIdx++){
                    eout << "(" << e.invalidProperties[propIdx].id << ",";
                    eout << ossie::any_to_string(e.invalidProperties[propIdx].value) << ")";
                }
                eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                LOG_ERROR(ApplicationFactory_impl, eout.str());
                throw CF::ApplicationFactory::InvalidInitConfiguration(e.invalidProperties);
            } catch( ... ) {
                ostringstream eout;
                eout << "Failed to initialize component properties: '";
                eout << softpkg->getName() << "' with component id: '" << deployment->getIdentifier() << " assigned to device: '"<<deployment->getAssignedDevice()->identifier << "' ";
                eout << " in waveform '"<< _waveformContextName<<"';";
                eout << "'initializeProperties' failed with Unknown Exception";
                eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                LOG_ERROR(ApplicationFactory_impl, eout.str());
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
            }
        }

        LOG_TRACE(ApplicationFactory_impl, "Initializing component " << componentId);
        try {
            resource->initialize();
        } catch (const CF::LifeCycle::InitializeError& error) {
            // Dump the detailed initialization failure to the log
            ostringstream logmsg;
            std::string component_version(component->spd.getSoftPkgType());
            std::string added_message = this->createVersionMismatchMessage(component_version);
            logmsg << added_message;
            logmsg << "Initializing component " << componentId << " failed";
            for (CORBA::ULong index = 0; index < error.errorMessages.length(); ++index) {
                logmsg << std::endl << error.errorMessages[index];
            }
            LOG_ERROR(ApplicationFactory_impl, logmsg.str());

            ostringstream eout;
            eout << added_message;
            eout << "Unable to initialize component " << componentId;
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
        } catch (const CORBA::SystemException& exc) {
            ostringstream eout;
            std::string component_version(component->spd.getSoftPkgType());
            std::string added_message = this->createVersionMismatchMessage(component_version);
            eout << added_message;
            eout << "CORBA " << exc._name() << " exception initializing component " << componentId;
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
        }
    }
}

void createHelper::configureComponents(const DeploymentList& deployments)
{
    DeploymentList configure_list;
    ossie::ComponentDeployment* ac_deployment = 0;
    for (DeploymentList::const_iterator depl = deployments.begin(); depl != deployments.end(); ++depl) {
        ossie::ComponentDeployment* deployment = (*depl);
        if (!deployment->getSoftPkg()->isScaCompliant()) {
            // If the component is non-SCA compliant then we don't expect anything beyond this
            LOG_TRACE(ApplicationFactory_impl, "Skipping configure of non SCA-compliant component "
                      << deployment->getIdentifier());
        } else if (!deployment->isResource()) {
            LOG_TRACE(ApplicationFactory_impl, "Skipping configure of non-resource component "
                      << deployment->getIdentifier());
        } else {
            // The component is configurable; if it's the assembly controller,
            // save it for the end
            if (deployment->getInstantiation()->isAssemblyController()) {
                ac_deployment = deployment;
            } else {
                configure_list.push_back(deployment);
            }
        }
    }
    // Configure the assembly controller last, if it's configurable
    if (ac_deployment) {
        configure_list.push_back(ac_deployment);
    }

    for (DeploymentList::iterator depl = configure_list.begin(); depl != configure_list.end(); ++depl) {
        ossie::ComponentDeployment* deployment = *depl;
        const ossie::SoftPkg* softpkg = deployment->getSoftPkg();
        
        // Assuming 1 instantiation for each componentplacement
        if (deployment->getInstantiation()->isNamingService()) {

            CF::Resource_var _rsc = deployment->getResourcePtr();

            if (CORBA::is_nil(_rsc)) {
                LOG_ERROR(ApplicationFactory_impl, "Could not get component reference");
                ostringstream eout;
                std::string component_version(component->spd.getSoftPkgType());
                std::string added_message = this->createVersionMismatchMessage(component_version);
                eout << added_message;
                eout << "Could not get component reference for component: '" 
                     << softpkg->getName() << "' with component id: '" 
                     << deployment->getIdentifier() << " assigned to device: '"
                     << deployment->getAssignedDevice()->identifier<<"'";
                eout << " in waveform '" << _waveformContextName<<"';";
                eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
            }

            redhawk::PropertyMap config_props = deployment->getInitialConfigureProperties();
            CF::Properties partialStruct = ossie::getPartialStructs(config_props);
            bool partialWarn = false;
            if (partialStruct.length() != 0) {
                ostringstream eout;
                eout <<  "Component " << deployment->getIdentifier() << " contains structure"<< partialStruct[0].id <<" with a mix of defined and nil values. The behavior for the component is undefined";
                LOG_WARN(ApplicationFactory_impl, eout.str());
                partialWarn = true;
            }
            try {
                // try to configure the component
                _rsc->configure(config_props);
            } catch(CF::PropertySet::InvalidConfiguration& e) {
                ostringstream eout;
                eout << "Failed to 'configure' component: '";
                eout << softpkg->getName() << "' with component id: '" << deployment->getIdentifier() << " assigned to device: '"<<deployment->getAssignedDevice()->identifier << "' ";
                eout << " in waveform '"<< _waveformContextName<<"';";
                eout <<  "InvalidConfiguration with this info: <";
                eout << e.msg << "> for these invalid properties: ";
                for (unsigned int propIdx = 0; propIdx < e.invalidProperties.length(); propIdx++){
                    eout << "(" << e.invalidProperties[propIdx].id << ",";
                    eout << ossie::any_to_string(e.invalidProperties[propIdx].value) << ")";
                }
                if (partialWarn) {
                    eout << ". Note that this component contains a property with a mix of defined and nil values.";
                }
                eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                LOG_ERROR(ApplicationFactory_impl, eout.str());
                throw CF::ApplicationFactory::InvalidInitConfiguration(e.invalidProperties);
            } catch(CF::PropertySet::PartialConfiguration& e) {
                ostringstream eout;
                eout << "Failed to instantiate component: '";
                eout << softpkg->getName() << "' with component id: '" << deployment->getIdentifier() << " assigned to device: '"<<deployment->getAssignedDevice()->identifier << "' ";
                eout << " in waveform '"<< _waveformContextName<<"';";
                eout << "Failed to 'configure' component; PartialConfiguration for these invalid properties: ";
                for (unsigned int propIdx = 0; propIdx < e.invalidProperties.length(); propIdx++){
                    eout << "(" << e.invalidProperties[propIdx].id << ",";
                    eout << ossie::any_to_string(e.invalidProperties[propIdx].value) << ")";
                }
                if (partialWarn) {
                    eout << ". Note that this component contains a property with a mix of defined and nil values.";
                }
                eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                LOG_ERROR(ApplicationFactory_impl, eout.str());
                throw CF::ApplicationFactory::InvalidInitConfiguration(e.invalidProperties);
            } catch( ... ) {
                ostringstream eout;
                eout << "Failed to instantiate component: '";
                eout << softpkg->getName() << "' with component id: '" << deployment->getIdentifier() << " assigned to device: '"<< deployment->getAssignedDevice()->identifier << "' ";
                eout << " in waveform '"<< _waveformContextName<<"';";
                eout << "'configure' failed with Unknown Exception";
                if (partialWarn) {
                    eout << ". Note that this component contains a property with a mix of defined and nil values.";
                }
                eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                LOG_ERROR(ApplicationFactory_impl, eout.str());
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
            }
        }
    }
}

/* Connect the components
 *  - Connect the components
 */
void createHelper::connectComponents(ossie::ApplicationDeployment& appDeployment,
                                     std::vector<ConnectionNode>& connections,
                                     string base_naming_context)
{
    const std::vector<Connection>& _connection = _appFact._sadParser.getConnections ();

    // Create an AppConnectionManager to resolve and track all connections in the application.
    using ossie::AppConnectionManager;
    AppConnectionManager connectionManager(_appFact._domainManager, &appDeployment, &appDeployment, base_naming_context);

    // Create all resource connections
    LOG_TRACE(ApplicationFactory_impl, "Establishing " << _connection.size() << " waveform connections")
    for (int c_idx = _connection.size () - 1; c_idx >= 0; c_idx--) {
        const Connection& connection = _connection[c_idx];

        LOG_TRACE(ApplicationFactory_impl, "Processing connection " << connection.getID());

        // Attempt to resolve the connection; if any connection fails, application creation fails.
        if (!connectionManager.resolveConnection(connection)) {
            LOG_ERROR(ApplicationFactory_impl, "Unable to make connection " << connection.getID());
            ostringstream eout;
            eout << "Unable to make connection " << connection.getID();
            eout << " in waveform '"<< _waveformContextName<<"';";
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
        }
    }

    // Copy all established connections into the connection array
    const std::vector<ConnectionNode>& establishedConnections = connectionManager.getConnections();
    std::copy(establishedConnections.begin(), establishedConnections.end(), std::back_inserter(connections));
}

std::vector<CF::Resource_var> createHelper::getStartOrder(const DeploymentList& deployments)
{
    LOG_TRACE(ApplicationFactory_impl, "Assigning start order");

    // Now that all of the components are known, bin the start orders based on
    // the values in the SAD. Using a multimap, keyed on the start order value,
    // accounts for duplicate keys and allows assigning the effective order
    // easily by iterating through all entries.
    typedef std::multimap<int,ossie::ComponentDeployment*> StartOrderMap;
    StartOrderMap start_map;
    for (size_t index = 0; index < deployments.size(); ++index) {
        ossie::ComponentDeployment* deployment = deployments[index];
        const ossie::ComponentInstantiation* instantiation = deployment->getInstantiation();
        if (instantiation->isAssemblyController()) {
            LOG_TRACE(ApplicationFactory_impl, "Component " << instantiation->getID()
                      << " is the assembly controller");
        } else if (instantiation->hasStartOrder()) {
            // Only track start order if it was provided, and the component is
            // not the assembly controller
            start_map.insert(std::make_pair(instantiation->getStartOrder(), deployment));
        }
    }

    // Build the start order vector in the right order
    std::vector<CF::Resource_var> start_order;
    int index = 1;
    for (StartOrderMap::iterator ii = start_map.begin(); ii != start_map.end(); ++ii, ++index) {
        LOG_TRACE(ApplicationFactory_impl, index << ": "
                  << ii->second->getInstantiation()->getID());
        start_order.push_back(ii->second->getResourcePtr());
    }
    return start_order;
}

createHelper::createHelper (
        const ApplicationFactory_impl& appFact,
        string                         waveformContextName,
        string                         baseNamingContext,
        CosNaming::NamingContext_ptr   waveformContext,
        CosNaming::NamingContext_ptr   domainContext ):

    _appFact(appFact),
    _allocationMgr(_appFact._domainManager->_allocationMgr),
    _allocations(*_allocationMgr),
    _waveformContextName(waveformContextName),
    _baseNamingContext(baseNamingContext),
    _waveformContext(CosNaming::NamingContext::_duplicate(waveformContext)),
    _domainContext(domainContext),
    _isComplete(false),
    _application(0)
{
}

createHelper::~createHelper()
{
    if (!_isComplete) {
        _cleanupFailedCreate();
    }
    if (_application) {
        _application->_remove_ref();
    }
}

void createHelper::_cleanupFailedCreate()
{
    if (_application) {
        _application->releaseComponents();
        _application->terminateComponents();
        _application->unloadComponents();
        _application->_cleanupActivations();
    }

    LOG_TRACE(ApplicationFactory_impl, "Removing all bindings from naming context");
    try {
      if ( _appFact._domainManager && !_appFact._domainManager->bindToDomain() ) {
        ossie::corba::unbindAllFromContext(_waveformContext);
      }
    } CATCH_LOG_WARN(ApplicationFactory_impl, "Could not unbind contents of naming context");

    CosNaming::Name DNContextname;
    DNContextname.length(1);
    DNContextname[0].id = _waveformContextName.c_str();
    LOG_TRACE(ApplicationFactory_impl, "Unbinding the naming context")
    try {
        _appFact._domainContext->unbind(DNContextname);
    } catch ( ... ) {
    }

    LOG_TRACE(ApplicationFactory_impl, "Destroying naming context");
    try {
        _waveformContext->destroy();
    } CATCH_LOG_WARN(ApplicationFactory_impl, "Could not destroy naming context");
}
