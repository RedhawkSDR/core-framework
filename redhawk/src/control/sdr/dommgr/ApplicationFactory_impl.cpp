
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
#include <boost/algorithm/string.hpp>

#include <ossie/CF/WellKnownProperties.h>
#include <ossie/FileStream.h>
#include <ossie/prop_helpers.h>
#include <ossie/Versions.h>
#include <ossie/prop_utils.h>

#include "Application_impl.h"
#include "ApplicationFactory_impl.h"
#include "createHelper.h"
#include "DomainManager_impl.h"
#include "AllocationManager_impl.h"
#include "RH_NamingContext.h"
#include "ApplicationValidator.h"
#include "DeploymentExceptions.h"

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

    static std::string getVersionMismatchMessage(const SoftPkg* softpkg)
    {
        const std::string& softpkg_version = softpkg->getSoftPkgType();
        if (redhawk::compareVersions(VERSION, softpkg_version) > 0) {
            return " (attempting to run a component from version " + softpkg_version + " on REDHAWK version " VERSION ")";
        } else {
            return std::string();
        }
    }
}

PREPARE_CF_LOGGING(ApplicationFactory_impl);

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
        RH_ERROR(_appFactoryLog, "CosNaming::NamingContext::_narrow threw Unknown Exception");
        throw;
    }

    try {
        _fileMgr = _domainManager->fileMgr();
    } catch ( std::exception& ex ) {
        ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while retrieving the File Manager";
        RH_ERROR(_appFactoryLog, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch ( const CORBA::Exception& ex ) {
        ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while retrieving the File Manager";
        RH_ERROR(_appFactoryLog, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch( ... ) {
        RH_ERROR(_appFactoryLog, "domainManager->_fileMgr failed with Unknown Exception");
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, "Could not get File Manager from Domain Manager");
    }

    RH_INFO(_appFactoryLog, "Installing application " << _softwareProfile);
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
        RH_ERROR(_appFactoryLog, eout.str());
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch ( std::exception& ex ) {
        ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<". While loading "<<_softwareProfile;
        RH_ERROR(_appFactoryLog, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch( CF::InvalidFileName& ex ) {
        ostringstream eout;
        eout << "The following InvalidFileName exception occurred, profile: " << _softwareProfile;
        RH_ERROR(_appFactoryLog, eout.str());
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, eout.str().c_str());
    } catch ( const CF::FileException& ex ) {
        ostringstream eout;
        eout << "The following FileException occurred: "<<ex.msg<<"  While loading "<<_softwareProfile;
        RH_ERROR(_appFactoryLog, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch ( const CORBA::Exception& ex ) {
        ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" While loading "<<_softwareProfile;
        RH_ERROR(_appFactoryLog, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch( ... ) {
        ostringstream eout;
        eout << "Parsing SAD file: " <<_softwareProfile << " Failed with unknown exception.";
        RH_ERROR(_appFactoryLog, eout.str());
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_ENOENT, eout.str().c_str());
    }

    // Validate the application using the current domain state; however, we
    // cannot assume that the component SPDs will not change between now and a
    // subsequent create call, so the parsed profiles are not saved
    redhawk::ApplicationValidator validator(_fileMgr, _appFactoryLog);
    try {
        validator.validate(_sadParser);
    } catch (const std::runtime_error& exc) {
        RH_ERROR(_appFactoryLog, "SAD " << softwareProfile
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

const std::string& ApplicationFactory_impl::getIdentifier() const
{
    return _identifier;
}

const std::string& ApplicationFactory_impl::getName() const
{
    return _name;
}

const std::string& ApplicationFactory_impl::getSoftwareProfile() const
{
    return _softwareProfile;
}

void createHelper::assignPlacementsToDevices(redhawk::ApplicationDeployment& appDeployment,
                                             const DeviceAssignmentMap& devices,
                                             const std::map<std::string,float>& specialized_reservations)
{
    // Try to place all of the collocations first, since they naturally have
    // more restrictive placement constraints
    BOOST_FOREACH(const SoftwareAssembly::HostCollocation& collocation, _appFact._sadParser.getHostCollocations()) {
        _placeHostCollocation(appDeployment, collocation, devices, specialized_reservations);
    }

    // Place the remaining components one-by-one
    BOOST_FOREACH(const ComponentPlacement& placement, _appFact._sadParser.getComponentPlacements()) {
        const SoftPkg* softpkg = _profileCache.loadProfile(placement.filename);
        BOOST_FOREACH(const ComponentInstantiation& instantiation, placement.getInstantiations()) {
            // Even though the XML supports more than one instantiation per
            // component placement, the tooling doesn't support that, so this
            // loop may be strictly academic
            std::string assigned_device;
            DeviceAssignmentMap::const_iterator device = devices.find(instantiation.getID());
            if (device != devices.end()) {
                assigned_device = device->second;
                RH_TRACE(_createHelperLog, "Component " << instantiation.getID()
                          << " is assigned to device " << assigned_device);
            }
            redhawk::ComponentDeployment* deployment = appDeployment.createComponentDeployment(softpkg, &instantiation);
            allocateComponent(appDeployment, deployment, assigned_device, specialized_reservations);

            // For components that run as shared libraries, create or reuse a
            // matching container deployment
            if (deployment->getImplementation()->getCodeType() == SPD::Code::SHARED_LIBRARY) {
                RH_DEBUG(_createHelperLog, "Component " << deployment->getInstantiation()->getID()
                          << "' implementation " << deployment->getImplementation()->getID()
                          << " is a shared library");
                redhawk::ContainerDeployment* container = appDeployment.createContainer(_profileCache, deployment->getAssignedDevice());
                if (!container->getAssignedDevice()) {

                    const redhawk::PropertyMap& devReqs = deployment->getDeviceRequires();
                    if ( devReqs.size() ) container->setDeviceRequires(devReqs);
                    // Use whether the device is assigned as a sentinel to check
                    // whether the container was already created, and if not,
                    // allocate it to the device
                    allocateComponent(appDeployment, container, deployment->getAssignedDevice()->identifier, specialized_reservations);
                }
                deployment->setContainer(container);
            }
            
        }
    }
}

void createHelper::_validateDAS(redhawk::ApplicationDeployment& appDeployment,
                                const DeviceAssignmentMap& deviceAssignments)
{
    RH_TRACE(_createHelperLog, "Validating device assignment sequence (length "
              << deviceAssignments.size() << ")");
    for (DeviceAssignmentMap::const_iterator ii = deviceAssignments.begin(); ii != deviceAssignments.end(); ++ii) {
        const std::string& componentId = ii->first;
        const std::string& assignedDeviceId = ii->second;

        if (!_appFact._sadParser.getComponentInstantiation(componentId)) {
            RH_ERROR(_createHelperLog, "Failed to create application; "
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

bool createHelper::placeHostCollocation(redhawk::ApplicationDeployment& appDeployment,
                                        const DeploymentList& components,
                                        DeploymentList::const_iterator current,
                                        ossie::DeviceList& deploymentDevices,
                                        const redhawk::PropertyMap& deviceRequires,
                                        const ReservationList& reservations,
                                        const ProcessorList& processorDeps,
                                        const OSList& osDeps)

{
    if (current == components.end()) {
        // Reached the end of the component deployments; all implementations
        // should be set, so give it a try
        if ( !deviceRequires.empty() ) {
            RH_TRACE(_createHelperLog, "Collocation has devicerequires: " << deviceRequires );
        }
        return allocateHostCollocation(appDeployment, components, deploymentDevices, processorDeps, osDeps, deviceRequires, reservations );
    }

    // Try all of the implementations from the current component for matches
    // with the processor and OS dependencies
    redhawk::ComponentDeployment* deployment = *current;
    const SPD::Implementations& comp_impls = deployment->getSoftPkg()->getImplementations();
    RH_TRACE(_createHelperLog, "Finding collocation-compatible implementations for component "
              << deployment->getInstantiation()->getID());
    ++current;
    for (SPD::Implementations::const_iterator impl = comp_impls.begin(); impl != comp_impls.end(); ++impl) {
        const ossie::SPD::Implementation* implementation = &(*impl);
        RH_TRACE(_createHelperLog, "Checking implementation " << implementation->getID());

        // Check that the processor dependencies are compatible, filtering out
        // anything not compatible with the current component
        std::vector<std::string> proc_list = processorDeps;;
        if (!mergeDependencies(proc_list, implementation->getProcessors())) {
            RH_TRACE(_createHelperLog, "Skipping implementation " << implementation->getID()
                      << ": no processor match");
            continue;
        }

        // Check that the OS dependencies are compatible, again filtering out
        // anything not compatible with the current component
        std::vector<ossie::SPD::NameVersionPair> os_list = osDeps;
        if (!mergeDependencies(os_list, implementation->getOsDeps())) {
            RH_TRACE(_createHelperLog, "Skipping implementation " << implementation->getID()
                      << ": no OS match");
            continue;
        }

        // Set this implementation for deployment and recurse one more level
        deployment->setImplementation(implementation);
        if (placeHostCollocation(appDeployment, components, current, deploymentDevices, deviceRequires, reservations, proc_list, os_list)) {
            return true;
        }
    }

    return false;
}

bool createHelper::allocateHostCollocation(redhawk::ApplicationDeployment& appDeployment,
                                           const DeploymentList& components,
                                           ossie::DeviceList& deploymentDevices,
                                           const ProcessorList& processorDeps,
                                           const OSList& osDeps,
                                           const redhawk::PropertyMap& deviceRequires,
                                           const ReservationList& reservations )
{
    // Consolidate the allocation properties into a single list
    CF::Properties allocationProperties = _consolidateAllocations(appDeployment, components);
    redhawk::PropertyMap &_allocationProperties = redhawk::PropertyMap::cast(allocationProperties);
    if (reservations.size() != 0) {
        redhawk::PropertyMap _struct;
        std::vector<std::string> _kinds, _values;
        for (ReservationList::const_iterator it=reservations.begin(); it!=reservations.end(); it++) {
            _kinds.push_back(it->kind);
            _values.push_back(it->value);
        }
        _struct["redhawk::reservation_request::kinds"].setValue(_kinds);
        _struct["redhawk::reservation_request::values"].setValue(_values);
        _struct["redhawk::reservation_request::obj_id"].setValue(appDeployment.getIdentifier());
        _allocationProperties["redhawk::reservation_request"].setValue(_struct);
    }

    RH_TRACE(_createHelperLog, "Allocating deployment for " << components.size()
              << " collocated components");
    for (DeploymentList::const_iterator depl = components.begin(); depl != components.end(); ++depl) {
        RH_TRACE(_createHelperLog, "Component " << (*depl)->getInstantiation()->getID()
                  << " implementation " << (*depl)->getImplementation()->getID());
    }

    if ( !deviceRequires.empty() ) {
        RH_TRACE(_createHelperLog, "Collocation has devicerequires:  " << deviceRequires );
    }

    const std::string requestid = ossie::generateUUID();
    ossie::AllocationResult response = _allocationMgr->allocateDeployment(requestid, _allocationProperties, deploymentDevices, appDeployment.getIdentifier(), processorDeps, osDeps, deviceRequires);
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
                RH_TRACE(_createHelperLog, "Unable to resolve softpackage dependencies for component "
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

        RH_TRACE(_createHelperLog, "Successful collocation allocation");
        return true;
    }
    RH_TRACE(_createHelperLog, "Failed collocation allocation");
    return false;
 }

CF::Properties createHelper::_consolidateAllocations(redhawk::ApplicationDeployment& appDeployment, const DeploymentList& deployments)
{
    CF::Properties allocs;
    for (DeploymentList::const_iterator depl = deployments.begin(); depl != deployments.end(); ++depl) {
        redhawk::PropertyMap _init;
        for (redhawk::ApplicationDeployment::ComponentList::const_iterator _comp = appDeployment.getComponentDeployments().begin(); _comp!=appDeployment.getComponentDeployments().end(); _comp++) {
            redhawk::SoftPkgDeployment * _tmp_comp = static_cast<redhawk::SoftPkgDeployment* const>(*_comp);
            if (_tmp_comp == (*depl)) {
                _init = (*_comp)->getAllInitialProperties();
            }
        }
        const std::vector<PropertyRef>& deps = (*depl)->getImplementation()->getDependencies();
        for (std::vector<PropertyRef>::const_iterator dep = deps.begin(); dep != deps.end(); ++dep) {
          CF::Properties _tmp_allocs;
          _tmp_allocs.length(1);
          ossie::ComponentProperty *prop = dep->property.get();
          _tmp_allocs[0] = ossie::convertPropertyRefToDataType(prop);
          this->_evaluateMATHinRequest(_tmp_allocs, _init);
          ossie::corba::push_back(allocs, _tmp_allocs[0]);
        }
    }
    return allocs;
}

void createHelper::_placeHostCollocation(redhawk::ApplicationDeployment& appDeployment,
                                         const ossie::SoftwareAssembly::HostCollocation& collocation,
                                         const DeviceAssignmentMap& devices,
                                         const std::map<std::string,float>& specialized_reservations)
{
    RH_TRACE(_createHelperLog, "Placing host collocation " << collocation.getID()
              << " " << collocation.getName());

    std::pair < std::string, redhawk::PropertyMap > devReq(std::string(""), redhawk::PropertyMap());
    // Keep track of devices to which some of the components have
    // been assigned.
    DeviceIDList assignedDevices;
    DeploymentList deployments;
    BOOST_FOREACH(const ComponentPlacement& placement, collocation.getComponents()) {
        const SoftPkg* softpkg = _profileCache.loadProfile(placement.filename);
        BOOST_FOREACH(const ComponentInstantiation& instantiation, placement.getInstantiations()) {
            // Even though the XML supports more than one instantiation per
            // component placement, the tooling doesn't support that, so this
            // loop may be strictly academic
            redhawk::ComponentDeployment* deployment = appDeployment.createComponentDeployment(softpkg, &instantiation);
            deployments.push_back(deployment);

            DeviceAssignmentMap::const_iterator device = devices.find(instantiation.getID());
            if (device != devices.end()) {
                assignedDevices.push_back(device->second);
            }

            // check if collocation contains a devicerequires set
            if ( !deployment->getDeviceRequires().empty() ) {
                devReq.first = deployment->getIdentifier();
                devReq.second = deployment->getDeviceRequires();
                RH_DEBUG(_createHelperLog, "Collocation contains devicerequires instance: " << devReq.first << " props :"  << devReq.second);
            }
        }
    }

    //
    // if there are any collocated device requires, get the actual resolved usesdevice
    //
    std::vector< CF::Device_var > req_usesDevices;
    BOOST_FOREACH(const UsesDeviceRef& devref, collocation.getUsesDeviceRefs()) {
        std::string refid= devref.getID();
        CF::Device_var dev=appDeployment.lookupDeviceUsedByApplication(refid);
        RH_DEBUG(_createHelperLog, "UsesDevice for collocation: " << dev->label() );
        if ( !CORBA::is_nil(dev) ) {
            req_usesDevices.push_back(dev);
        }
    }

    // Get the executable devices for the domain; if there were any devices
    // assigned, filter out all other devices
    ossie::DeviceList deploymentDevices = _executableDevices;
    if (!assignedDevices.empty()) {
        for (ossie::DeviceList::iterator node = deploymentDevices.begin(); node != deploymentDevices.end(); ) {
            if (std::find(assignedDevices.begin(), assignedDevices.end(), (*node)->identifier) == assignedDevices.end()) {
                node = deploymentDevices.erase(node);
            }
            else {
                node++;
            }
        }
    }

    // if the collocation contains a deviceRequires then filter down the deployment list
    if ( !devReq.first.empty() ) {
        for (ossie::DeviceList::iterator node = deploymentDevices.begin(); node != deploymentDevices.end(); ) {
            if ( (*node)->requiresProps != devReq.second ) {
                node = deploymentDevices.erase(node);
            }
            node++;
        }
    }

    // no deployment devices available so we can stop
    if ( deploymentDevices.size() == 0 ) {
        ostringstream os;
        os << "No ExecutableDevices available to satisfy collocation.";
        throw redhawk::PlacementFailure(collocation, os.str() );
    }

    // if there is a usesdevice in the collocation that filter out the GPPs that we can use.
    if ( req_usesDevices.size() > 0 ) {
        // from the remaining list of deploymentDevices filter out those that not on the same host
        for ( std::vector< CF::Device_var >::iterator dev = req_usesDevices.begin(); dev != req_usesDevices.end(); ++dev) {
            RH_TRACE(_createHelperLog, "Find GPP for device collocation, device: " << (*dev)->label()  << " Number available GPPs:" << deploymentDevices.size() );
            for (ossie::DeviceList::iterator node = deploymentDevices.begin(); node != deploymentDevices.end(); ) {
                bool retval = ossie::sameHost( *dev, (*node)->device );
                RH_TRACE(_createHelperLog, "Check Collocation, Device: " << (*dev)->label()  << " Executable Device: " << (*node)->device->label() << "  --> RESULTS:" << retval );
                if ( retval == false )  {
                    node = deploymentDevices.erase(node);
                }
                else {
                    node++;
                }
            }
        }

        if ( deploymentDevices.size() == 0 ) {
            ostringstream os;
            os << "No Collocated ExecutableDevices for Device: ";
            for ( std::vector< CF::Device_var >::iterator dev = req_usesDevices.begin(); dev != req_usesDevices.end(); ++dev) {
                os << (*dev)->label();
                if (dev+1 != req_usesDevices.end()) os << ", ";
            }
            RH_DEBUG(_createHelperLog, os.str() );
            throw redhawk::PlacementFailure(collocation, os.str() );
        }

    }

    // load any collocation-based reservations
    std::vector<Reservation> _overloadedReservations = overloadReservations(collocation, specialized_reservations);

    RH_TRACE(_createHelperLog, "Placing " << deployments.size() << " components");
    if (!placeHostCollocation(appDeployment, deployments, deployments.begin(), deploymentDevices, devReq.second, _overloadedReservations)) {
        if (_allDevicesBusy(deploymentDevices)) {
            throw redhawk::PlacementFailure(collocation, "all executable devices (GPPs) in the Domain are busy");
        }
        throw redhawk::PlacementFailure(collocation, "failed to satisfy device dependencies");
    }

    for (DeploymentList::iterator deployment = deployments.begin(); deployment != deployments.end(); deployment++) {
        if ((*deployment)->getImplementation()->getCodeType() == SPD::Code::SHARED_LIBRARY) {
            RH_DEBUG(_createHelperLog, "Component " << (*deployment)->getInstantiation()->getID()
                << "' implementation " << (*deployment)->getImplementation()->getID()
                << " is a shared library");
            redhawk::ContainerDeployment* container = appDeployment.createContainer(_profileCache, (*deployment)->getAssignedDevice());
            if (!container->getAssignedDevice()) {
                const redhawk::PropertyMap& devReqs = (*deployment)->getDeviceRequires();
                if ( devReqs.size() ) container->setDeviceRequires(devReqs);
                // Use whether the device is assigned as a sentinel to check
                // whether the container was already created, and if not,
                // allocate it to the device
                allocateComponent(appDeployment, container, (*deployment)->getAssignedDevice()->identifier, specialized_reservations);
            }
            (*deployment)->setContainer(container);
        }
    }

    RH_TRACE(_createHelperLog, "-- Completed placement for Collocation ID:"
              << collocation.getID() << " Components Placed: " << deployments.size());
}

std::vector<ossie::Reservation> createHelper::overloadReservations(const ossie::SoftwareAssembly::HostCollocation& collocation,
                                                                   const std::map<std::string,float>& specialized_reservations)
{
    const std::vector<Reservation>& reservations = collocation.getReservations();
    std::vector<ossie::Reservation> retval = reservations;
    if ((reservations.size() == 0) and (specialized_reservations.size() == 0)) {
        return retval;
    }

    int number_collocations = _appFact._sadParser.getHostCollocations().size();
    if (number_collocations == 0) {
        return retval;
    }
    int number_blank_specialization = 0;
    for (std::map<std::string,float>::const_iterator _it=specialized_reservations.begin();_it!=specialized_reservations.end();_it++) {
        if (_it->first.empty()) {
            number_blank_specialization++;
        }
    }
    if (number_blank_specialization > 1) {
        throw std::logic_error("Ambiguous specialized CPU usage; cannot have more than one blank specialization");
    }
    if ((number_blank_specialization == 1) and (number_collocations > 1)) {
        throw std::logic_error("Ambiguous specialized CPU usage; more than one host collocation cannot be matched to a blank specialization");
    }
    if ((number_blank_specialization == 1) and (number_collocations == 1)) {
        if (reservations.size() != 0) {
            for (std::vector<Reservation>::iterator _it=retval.begin();_it!=retval.end();_it++) {
                if (_it->getKind() == "cpucores") {
                    std::string value_str;
                    std::ostringstream ss;
                    ss<<specialized_reservations.find(value_str)->second;
                    value_str = ss.str();
                    _it->overloadValue(value_str);
                }
            }
        }
        return retval;
    }
    bool found_overload = false;
    bool has_value = false;
    for (std::map<std::string,float>::const_iterator _it_spec=specialized_reservations.begin();_it_spec!=specialized_reservations.end();_it_spec++) {
        if (_it_spec->first == collocation.getID()) {
            has_value = true;
            for (std::vector<Reservation>::iterator _it=retval.begin();_it!=retval.end();_it++) {
                if (_it->getKind() == "cpucores") {
                    found_overload = true;
                    std::string value_str;
                    std::ostringstream ss;
                    ss<<specialized_reservations.find(_it_spec->first)->second;
                    value_str = ss.str();
                    _it->overloadValue(value_str);
                }
            }
        }
    }
    if ((not found_overload) and has_value) {
        Reservation res;
        res.kind = "cpucores";
        std::string value_str;
        std::ostringstream ss;
        ss<<specialized_reservations.find(collocation.getID())->second;
        res.value = ss.str();
        retval.push_back(res);
    }
    return retval;
}

void createHelper::_handleUsesDevices(redhawk::ApplicationDeployment& appDeployment,
                                      const std::string& appName)
{
    // Gets all uses device info from the SAD file
    const std::vector<UsesDevice>& usesDevices = _appFact._sadParser.getUsesDevices();
    RH_TRACE(_createHelperLog, "Application has " << usesDevices.size() << " usesdevice dependencies");

    // Get the assembly controller's configure properties for context in the
    // allocations
    CF::Properties appProperties = appDeployment.getAllocationContext();

    // The device assignments for SAD-level usesdevices are never stored
    redhawk::UsesDeviceDeployment assignedDevices;
    if (!allocateUsesDevices(usesDevices, appProperties, assignedDevices, this->_allocations)) {
        // There were unsatisfied usesdevices for the application
        std::vector<std::string> failed_ids = _getFailedUsesDevices(usesDevices, assignedDevices);
        throw redhawk::UsesDeviceFailure(appDeployment, failed_ids);
    }

    assignedDevices.transferUsesDeviceAssignments(appDeployment);
}

std::vector<std::string> createHelper::_getFailedUsesDevices(const std::vector<ossie::UsesDevice>& usesDevices,
                                                             redhawk::UsesDeviceDeployment& assignedDevices)
{
    std::vector<std::string> failed_ids;
    BOOST_FOREACH(const ossie::UsesDevice& uses, usesDevices) {
        if (!assignedDevices.getUsesDeviceAssignment(uses.getID())) {
            failed_ids.push_back(uses.getID());
        }
    }
    return failed_ids;
}

void createHelper::checkOptions()
{
    RH_TRACE(_createHelperLog,
              "Number of optionss: " << _appFact._sadParser.getOptions().size());

    BOOST_FOREACH(const SoftwareAssembly::Option& option, _appFact._sadParser.getOptions()) {
        if (option.name == "AWARE_APPLICATION") {
            if ((option.value == "true") || (option.value == "True") || (option.value == "TRUE") || (option.value == "1")) {
                this->_aware = true;
            } else if ((option.value == "false") || (option.value == "False") || (option.value == "FALSE") || (option.value == "0")) {
                this->_aware = false;
            }
        } else if (option.name == "STOP_TIMEOUT") {
            this->_stopTimeout = strtof(option.value.c_str(), NULL);
        }
    }
}

void createHelper::setUpExternalPorts(redhawk::ApplicationDeployment& appDeployment,
                                      Application_impl* application)
{
    RH_TRACE(_createHelperLog,
              "Mapping " << _appFact._sadParser.getExternalPorts().size() << " external port(s)");

    BOOST_FOREACH(const SoftwareAssembly::Port& port, _appFact._sadParser.getExternalPorts()) {
        RH_TRACE(_createHelperLog, "External port '" << port.getExternalName()
                  << "' from component '" << port.componentrefid
                  << "' identifier '" << port.identifier << "'");

        // Get the component from the instantiation identifier.
        redhawk::ComponentDeployment* deployment = appDeployment.getComponentDeployment(port.componentrefid);
        if (!deployment) {
            // The SAD parser should have rejected invalid component references
            throw std::logic_error("component not found for external port '" + port.getExternalName() + "'");
        }

        CF::Resource_var resource = deployment->getResourcePtr();
        CORBA::Object_var obj;

        if (port.type == SoftwareAssembly::Port::SUPPORTEDIDENTIFIER) {
            ossie::corba::overrideBlockingCall(resource);
            if (!resource->_is_a(port.identifier.c_str())) {
                throw redhawk::BadExternalPort(port, "component does not support interface " + port.identifier);
            }
            obj = CORBA::Object::_duplicate(resource);
        } else {
            // Must be either "usesidentifier" or "providesidentifier",
            // which are equivalent unless you want to be extra
            // pedantic and check how the port is described in the
            // component's SCD.
            // Try to look up the port.
            try {
                obj = resource->getPort(port.identifier.c_str());
            } catch (const CF::PortSupplier::UnknownPort& exc) {
                throw redhawk::BadExternalPort(port, "component has no port '" + port.identifier + "'");
            } catch (const CORBA::SystemException& exc) {
                throw redhawk::BadExternalPort(port, ossie::corba::describeException(exc));
            } catch (...) {
                // Should never happen, but turn anything else into a
                // BadExternalPort just in case
                throw redhawk::BadExternalPort(port, "unexpected error");
            }
        }

        // Add it to the list of external ports on the application object.
        application->addExternalPort(port.getExternalName(), obj);
    }
}

void createHelper::setUpExternalProperties(redhawk::ApplicationDeployment& appDeployment,
                                           Application_impl* application)
{
    const std::vector<SoftwareAssembly::Property>& props = _appFact._sadParser.getExternalProperties();
    RH_TRACE(_createHelperLog, "Mapping " << props.size() << " external property(ies)");
    for (std::vector<SoftwareAssembly::Property>::const_iterator prop = props.begin(); prop != props.end(); ++prop) {
        RH_TRACE(_createHelperLog, "Property component: " << prop->comprefid << " Property identifier: " << prop->propid);

        // Get the component from the compref identifier.
        redhawk::ComponentDeployment* deployment = appDeployment.getComponentDeployment(prop->comprefid);
        if (!deployment) {
            // The SAD parser should have rejected invalid component references
            throw std::logic_error("component not found for external property '" + prop->getExternalID() + "'");
        }
        const Property* property = deployment->getSoftPkg()->getProperties()->getProperty(prop->propid);
        if (!property) {
            throw redhawk::DeploymentError("Attempting to promote property '" + prop->propid + "' that does not exist in component '" + prop->comprefid + "'");
        }

        std::ostringstream _access;
        if (property->getMode()) {
            _access << property->getMode();
        } else {
            _access << "readwrite";
        }

        CF::Resource_var comp = deployment->getResourcePtr();
        application->addExternalProperty(prop->propid, prop->getExternalID(), _access.str(), comp);
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
    RH_TRACE(_appFactoryLog, "Creating application " << name);

    // must declare these here, so we can pass to the createHelper instance
    string _waveform_context_name;
    string base_naming_context;
    CosNaming::NamingContext_var _waveformContext;

    ///////////////////////////////////////////////////
    // Establish new naming context for waveform
    RH_TRACE(_appFactoryLog, "Establishing waveform naming context");
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

        RH_TRACE(_appFactoryLog, "Binding new context " << _waveform_context_name.c_str());
        try {
            _waveformContext = _domainContext->bind_new_context(WaveformContextName);
        } catch( ... ) {
            // just in case it bound, unbind and error
            // roughly the same code as _cleanupNewContext
            try {
                _domainContext->unbind(WaveformContextName);
            } catch ( ... ) {
            }
            RH_ERROR(_appFactoryLog, "bind_new_context threw Unknown Exception");
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
    RH_TRACE(_appFactoryLog, "Creating new createHelper class.");
    createHelper new_createhelper(*this, _waveform_context_name, base_naming_context, _waveformContext, _domainContext);

    // now actually perform the create operation
    RH_TRACE(_appFactoryLog, "Performing 'create' function.");
    CF::Application_ptr new_app;
    try {
        new_app = new_createhelper.create(name, initConfiguration, deviceAssignmentMap);
    } catch (const redhawk::DeploymentError& exc) {
        // Convert from internal error to CORBA exception and report the error
        const std::string message = exc.message();
        RH_ERROR(_appFactoryLog, "Failed to create application '" << name << "': " << message);
        throw CF::ApplicationFactory::CreateApplicationError(exc.errorNumber(), message.c_str());
    } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
        RH_ERROR(_appFactoryLog, "Error in application creation; " << ex.msg);
        throw;
    } catch (CF::ApplicationFactory::CreateApplicationRequestError& ex) {
        RH_ERROR(_appFactoryLog, "Error in application creation")
        throw;
    } catch (const std::exception& ex) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while creating the application";
        RH_ERROR(_appFactoryLog, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EBADF, eout.str().c_str());
    } catch (const CORBA::Exception& ex) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while creating the application";
        RH_ERROR(_appFactoryLog, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, eout.str().c_str());
    } catch ( ... ) {
        RH_ERROR(_appFactoryLog, "Unexpected error in application creation - see log")
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, "Unexpected error in application creation - see log.");
    }

    // return the new Application
    return new_app;
}

CF::Application_ptr createHelper::create (
    const char*                         name,
    const CF::Properties&               initConfiguration,
    const DeviceAssignmentMap& deviceAssignments)
{
    
    redhawk::PropertyMap modifiedInitConfiguration;

    checkOptions();

    ///////////////////////////////////////////////////////////////////
    // Check to see if this is an aware application and 
    //  check to see if a different GPP reservation setting is defined
    const std::string aware_app_deprecated_property_id("AWARE_APPLICATION");
    const std::string specialized_reservation_id("SPECIALIZED_CPU_RESERVATION");

    std::map<std::string,float> specialized_reservations;
    for (unsigned int initCount = 0; initCount < initConfiguration.length(); initCount++) {
        const std::string stringId(initConfiguration[initCount].id);
        const redhawk::Value& value = redhawk::Value::cast(initConfiguration[initCount].value);
        if ((stringId == ExtendedCF::WKP::AWARE_APPLICATION) or (stringId == aware_app_deprecated_property_id)) {
            _aware = value.toBoolean();
        } else if (stringId == ExtendedCF::WKP::STOP_TIMEOUT) {
            _stopTimeout = value.toFloat();
        } else if (stringId == specialized_reservation_id) {
            if (value.getType() == redhawk::Value::TYPE_PROPERTIES) {
                const redhawk::PropertyMap& reservations = value.asProperties();
                for (unsigned int idx=0; idx<reservations.size(); idx++) {
                    const std::string component_id = reservations[idx].getId();
                    try {
                        specialized_reservations[component_id] = reservations[idx].getValue().toDouble();
                    } catch (const std::exception&) {
                        // Ignore bad type
                    }
                }
            }
        } else {
            modifiedInitConfiguration.push_back(initConfiguration[initCount]);
        }
    }

    // Get a list of all device currently in the domain
    _registeredDevices = _appFact._domainManager->getRegisteredDevices();
    _executableDevices.clear();
    for (DeviceList::iterator iter = _registeredDevices.begin(); iter != _registeredDevices.end(); ++iter) {
        if ((*iter)->isExecutable()) {
            _executableDevices.push_back(*iter);
        }
    }

    // Fail immediately if there are no available devices to execute components
    if (_executableDevices.empty()) {
        throw redhawk::NoExecutableDevices();
    }

    const std::string lastExecutableDevice = _appFact._domainManager->getLastDeviceUsedForDeployment();
    if (!lastExecutableDevice.empty()) {
        RH_TRACE(_createHelperLog, "Placing device " << lastExecutableDevice
                  << " first in deployment list");
        rotateDeviceList(_executableDevices, lastExecutableDevice);
    }

    //////////////////////////////////////////////////
    // Load the components to instantiate from the SAD
    redhawk::ApplicationDeployment app_deployment(_appFact._sadParser, _waveformContextName, modifiedInitConfiguration);
    app_deployment.setLogger(_createHelperLog);

    ////////////////////////////////////////////////
    // Assign components to devices
    ////////////////////////////////////////////////

    // Catch invalid device assignments
    _validateDAS(app_deployment, deviceAssignments);

    // resolve assembly controller to assist with usesdevices that
    // require matching properties
    _resolveAssemblyController(app_deployment);

    // check to make sure that there's no collision between sad-based and command-line reservations
    if (specialized_reservations.size() != 0) {
        verifyNoCpuSpecializationCollisions(_appFact._sadParser, specialized_reservations);
    }

    // Allocate any usesdevice capacities specified in the SAD file
    _handleUsesDevices(app_deployment, name);

    // Assign all components to devices
    assignPlacementsToDevices(app_deployment, deviceAssignments, specialized_reservations);

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
                                        this->_aware,
                                        this->_stopTimeout,
                                        _domainContext);
    _appFact._domainManager->addPendingApplication(_application);

    // Activate the new Application servant
    PortableServer::ObjectId_var oid = Application_impl::Activate(_application);

    CF::ApplicationRegistrar_var app_reg = _application->appReg();
    loadAndExecuteContainers(app_deployment.getContainerDeployments(), app_reg);
    waitForContainerRegistration(app_deployment);

    loadAndExecuteComponents(app_deployment.getComponentDeployments(), app_reg);
    waitForComponentRegistration(app_deployment);

    // Check that the assembly controller is valid
    RH_TRACE(_createHelperLog, "Checking assembly controller");
    redhawk::ComponentDeployment* ac_deployment = app_deployment.getAssemblyController();
    if (!ac_deployment) {
        // This condition should have been prevented by parser validation
        throw std::logic_error("Assembly controller has not been assigned");
    }
    CF::Resource_var assemblyController = ac_deployment->getResourcePtr();
    if (CORBA::is_nil(assemblyController) && ac_deployment->getSoftPkg()->isScaCompliant()) {
        // Likewise, component registration should have already thrown an
        // exception if an SCA-compliant component did not register
        throw std::logic_error("Assembly controller has not registered with the application");
    }
    _application->setAssemblyController(ac_deployment->getIdentifier());

    initializeComponents(app_deployment.getComponentDeployments());

    std::vector<ConnectionNode> connections;
    connectComponents(app_deployment, connections, _baseNamingContext);
    configureComponents(app_deployment.getComponentDeployments());

    setUpExternalPorts(app_deployment, _application);
    setUpExternalProperties(app_deployment, _application);

    ////////////////////////////////////////////////
    // Create the application
    //
    // We are assuming that all components and their resources are 
    // collocated. This means that we assume the SAD <partitioning> 
    // element contains the <hostcollocation> element. NB: Ownership 
    // of the ConnectionManager is passed to the application.
    std::vector<std::string> allocationIDs;
    _allocations.transfer(allocationIDs);

    // Fill in the uses devices for the application
    CF::DeviceAssignmentSequence app_devices;
    typedef std::vector<redhawk::UsesDeviceAssignment*> UsesList;
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

    std::vector<std::string> start_order = getStartOrder(app_deployment.getComponentDeployments());
    _application->setStartOrder(start_order);

    _application->populateApplication(app_devices, 
                                      connections, 
                                      allocationIDs);

    // Add a reference to the new application to the 
    // ApplicationSequence in DomainManager
    CF::Application_var appObj = _application->_this();
    try {
        _appFact._domainManager->completePendingApplication(_application);
    } catch (CF::DomainManager::ApplicationInstallationError& ex) {
        // something bad happened - clean up
        RH_ERROR(_createHelperLog, ex.msg);
        throw CF::ApplicationFactory::CreateApplicationError(ex.errorNumber, ex.msg);
    }

    // After all components have been deployed, we know that the first
    // executable device in the list was used for the last deployment,
    // so update the domain manager
    _appFact._domainManager->setLastDeviceUsedForDeployment(_executableDevices.front()->identifier);

    _appFact._domainManager->sendAddEvent(_appFact._identifier,
                                          app_deployment.getIdentifier(),
                                          name,
                                          appObj,
                                          StandardEvent::APPLICATION);

    RH_INFO(_createHelperLog, "Done creating application " << app_deployment.getIdentifier() << " " << name);
    _isComplete = true;
    return appObj._retn();
}

void createHelper::verifyNoCpuSpecializationCollisions(const ossie::SoftwareAssembly& sad, std::map<std::string,float> specialized_reservations) {
    std::vector<std::string> host_collocation_names = this->getHostCollocationsIds();
    int number_empty = 0;
    bool found_host_collocation = false;
    BOOST_FOREACH(const SoftwareAssembly::HostCollocation& collocation, _appFact._sadParser.getHostCollocations()) {
        if (collocation.getReservations().size() > 0) {
            found_host_collocation = true;
            break;
        }
    }
    bool found_component = false;
    bool name_is_nothing = false;
    std::string bad_name;
    for (std::map<std::string,float>::iterator _reservation=specialized_reservations.begin();_reservation!=specialized_reservations.end();_reservation++) {
        if (not _reservation->first.empty()) {
            bool host_collocation = false;
            for (std::vector<std::string>::iterator _name=host_collocation_names.begin();_name!=host_collocation_names.end();_name++) {
                if (*_name == _reservation->first) {
                    host_collocation = true;
                    found_host_collocation = true;
                    break;
                }
            }
            if (not host_collocation) {
                BOOST_FOREACH(const ComponentPlacement& placement, sad.getComponentPlacements()) {
                    if (placement.getInstantiations()[0].getID() == _reservation->first) {
                        found_component = true;
                        break;
                    }
                }
                BOOST_FOREACH(const SoftwareAssembly::HostCollocation& _hostcollocation, sad.getHostCollocations()) {
                    BOOST_FOREACH(const ComponentPlacement& placement, _hostcollocation.getComponents()) {
                        if (placement.getInstantiations()[0].getID() == _reservation->first) {
                            found_component = true;
                            break;
                        }
                    }
                }
                if (found_component)
                    break;
                name_is_nothing = true;
                bad_name = _reservation->first;
                break;
            }
        } else {
            number_empty++;
        }
    }
    if (name_is_nothing) {
        throw std::logic_error("'SPECIALIZED_CPU_RESERVATION must include a hostcollocation id, a component id, or (when not ambiguous), a blank, bad id is: "+bad_name+"'");
    }
    if (number_empty > 1) {
        throw std::logic_error("'SPECIALIZED_CPU_RESERVATION cannot have more than 1 hostcollocation without an id'");
    }
    if (number_empty > 0)
        found_host_collocation = true;
    if (found_host_collocation and found_component) {
        throw std::logic_error("'SPECIALIZED_CPU_RESERVATION cannot mix hostcollocation and component reservations'");
    }
}

std::vector<std::string> createHelper::getComponentUsageNames(redhawk::ApplicationDeployment& appDeployment) {
    std::vector<std::string> retval;
    BOOST_FOREACH(const redhawk::ComponentDeployment* compdep, appDeployment.getComponentDeployments()) {
        retval.push_back(compdep->getInstantiation()->usageName);
    }
    return retval;
}

std::vector<std::string> createHelper::getHostCollocationsIds() {
    std::vector<std::string> retval;
    BOOST_FOREACH(const SoftwareAssembly::HostCollocation& collocation, _appFact._sadParser.getHostCollocations()) {
        std::string _name;
        if (not collocation.id.empty()) {
            _name = collocation.id;
        }
        retval.push_back(_name);
    }
    return retval;
}

void  createHelper::_resolveAssemblyController( redhawk::ApplicationDeployment& appDeployment  ) {

    // Place the remaining components one-by-one
    std::string asm_refid = _appFact._sadParser.getAssemblyControllerRefId();
    const ComponentPlacement *asm_placement = _appFact._sadParser.getAssemblyControllerPlacement();
    if ( asm_placement && asm_refid != "" and asm_refid.size() > 0 ) {
        const SoftPkg* softpkg = _profileCache.loadProfile(asm_placement->filename);
        const ComponentInstantiation *asm_inst = asm_placement->getInstantiation(asm_refid);
        if ( asm_inst ) {
            std::string inst_id = asm_inst->getID();
            RH_DEBUG(_createHelperLog, "Resolved ASSEMBLY CONTROLLER: " << asm_refid );
            redhawk::ComponentDeployment *cp  __attribute__((unused));
            cp = appDeployment.createComponentDeployment(softpkg, asm_inst);
            return;
        }
    }
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
 */
void createHelper::allocateComponent(redhawk::ApplicationDeployment& appDeployment,
                                     redhawk::ComponentDeployment* deployment,
                                     const std::string& assignedDeviceId,
                                     const std::map<std::string,float>& specialized_reservations)
{
    redhawk::PropertyMap alloc_context = deployment->getAllocationContext();
    
    // Find the devices that allocate the SPD's minimum required usesdevices properties
    const std::vector<UsesDevice>& usesDevices = deployment->getSoftPkg()->getUsesDevices();
    redhawk::UsesDeviceDeployment assignedDevices;
    if (!allocateUsesDevices(usesDevices, alloc_context, assignedDevices, this->_allocations)) {
        // There were unsatisfied usesdevices for the component
        std::vector<std::string> failed_ids = _getFailedUsesDevices(usesDevices, assignedDevices);
        throw redhawk::UsesDeviceFailure(deployment, failed_ids);
    }

    // now attempt to find an implementation that can have it's allocation requirements met
    const SPD::Implementations& implementations = deployment->getSoftPkg()->getImplementations();
    for (size_t implCount = 0; implCount < implementations.size(); implCount++) {
        const ossie::SPD::Implementation* implementation = &implementations[implCount];

        // TODO: Validate code file and dependency files exist

        // Handle 'usesdevice' dependencies for the particular implementation
        redhawk::UsesDeviceDeployment implAssignedDevices;
        ScopedAllocations implAllocations(*this->_allocationMgr);
        const std::vector<UsesDevice>& implUsesDevVec = implementation->getUsesDevices();
        
        if (!allocateUsesDevices(implUsesDevVec, alloc_context, implAssignedDevices, implAllocations)) {
            RH_DEBUG(_createHelperLog, "Unable to satisfy 'usesdevice' dependencies for component "
                      << deployment->getIdentifier() << " implementation " << implementation->getID());
            continue;
        }

        deployment->setImplementation(implementation);

        // Transfer ownership of the uses device assigments to the deployment
        assignedDevices.transferUsesDeviceAssignments(*deployment);
        
        // Found an implementation which has its 'usesdevice' dependencies
        // satisfied, now perform assignment/allocation of component to device
        RH_DEBUG(_createHelperLog, "Trying to find the device");
        ossie::AllocationResult response = allocateComponentToDevice(deployment, assignedDeviceId,
                                                                     appDeployment.getIdentifier(),
                                                                     specialized_reservations);
        
        if (response.first.empty()) {
            RH_DEBUG(_createHelperLog, "Unable to allocate device for component "
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
            RH_DEBUG(_createHelperLog, "Unable to resolve softpackage dependencies for component "
                      << deployment->getIdentifier() << " implementation " << implementation->getID());
            continue;
        }
        
        // Allocation to a device succeeded
        RH_DEBUG(_createHelperLog, "Assigned component " << deployment->getInstantiation()->getID()
                  << " implementation " << implementation->getID() << " to device " << deviceId);

        // Move the device to the front of the list
        rotateDeviceList(_executableDevices, deviceId);
        
        // Store the implementation-specific usesdevice allocations and
        // device assignments
        implAllocations.transfer(this->_allocations);

        implAssignedDevices.transferUsesDeviceAssignments(*deployment);
        
        return;
    }

    // Report failure, checking if the problem was that all executable devices
    // were busy
    if (_allDevicesBusy(_executableDevices)) {
        throw redhawk::PlacementFailure(deployment->getInstantiation(), "all executable devices (GPPs) in the Domain are busy");
    }
    throw redhawk::PlacementFailure(deployment->getInstantiation(), "failed to satisfy device dependencies");
}

bool createHelper::_allDevicesBusy(ossie::DeviceList& devices)
{
    // While this can yield false negatives (or positives) since it's not
    // atomic with component allocation, it should provide a little extra
    // insight in most cases
    for (ossie::DeviceList::iterator dev = devices.begin(); dev != devices.end(); ++dev) {
        CF::Device::UsageType state;
        try {
            state = (*dev)->device->usageState();
        } catch (...) {
            RH_WARN(_createHelperLog, "Device " << (*dev)->identifier << " is not reachable");
            continue;
        }
        if (state != CF::Device::BUSY) {
            return false;
        }
    }
    return true;
}

bool createHelper::allocateUsesDevices(const std::vector<UsesDevice>& usesDevices,
                                       const CF::Properties& configureProperties,
                                       redhawk::UsesDeviceDeployment& deviceAssignments,
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
        RH_TRACE(_createHelperLog, "Allocated " << allocationId);
        localAllocations.push_back(allocationId);
        
        // Find the usesdevice that matches the request and update it, removing
        // the key from the map
        const std::string requestID(response[resp].requestID);
        UsesDeviceMap::iterator uses = usesDeviceMap.find(requestID);
        if (uses == usesDeviceMap.end()) {
            // This condition should never occur
            RH_WARN(_createHelperLog, "Allocation request " << requestID
                     << " does not match any usesdevice");
            continue;
        }
        const std::string deviceId = ossie::corba::returnString(response[resp].allocatedDevice->identifier());
        usesDeviceMap.erase(uses);

        redhawk::UsesDeviceAssignment* assignment = new redhawk::UsesDeviceAssignment(uses->second);
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
                    RH_TRACE(_createHelperLog, "__MATH__ ARG: " << mathStatement.substr(0, mathStatement.find(',')) );
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
                RH_DEBUG(_createHelperLog, "__MATH__ RESULT: " << retval << " op1: " << operand << " op2:" << ossie::any_to_string(compValue) );
            } else {
                std::ostringstream eout;
                eout << " invalid __MATH__ statement; '" << mathStatement << "'";
                throw ossie::PropertyMatchingError(eout.str());
            }
        }
    }
}

/* Perform allocation/assignment of a particular component to the device.
 *  - Check if deployment has required device properties.. 
 *  - next,  do allocation/assignment based on user provided DAS
 *  - If not specified in DAS, then iterate through devices looking for a device that satisfies
 *    the allocation properties
 */
ossie::AllocationResult createHelper::allocateComponentToDevice(redhawk::ComponentDeployment* deployment,
                                                                const std::string& assignedDeviceId,
                                                                const std::string& appIdentifier,
                                                                const std::map<std::string,float>& specialized_reservations)
{
    const ossie::SPD::Implementation* implementation = deployment->getImplementation();
    ossie::DeviceList devices = _registeredDevices;
    const CF::Properties& deviceRequires = deployment->getDeviceRequires();

    if ( deviceRequires.length() > 0 ) {
        RH_TRACE(_createHelperLog, "Compnent: '" << deployment->getSoftPkg()->getName() << "' has device requires");
        // filter out devices that only match devicerequires property set
        ossie::DeviceList::iterator device;
        for (device = devices.begin(); device != devices.end(); ) {
            boost::shared_ptr<ossie::DeviceNode> devnode = *device;
            RH_DEBUG(_createHelperLog, "allocateDevice::PartitionMatching required props: " << devnode->requiresProps );
            if ( !checkPartitionMatching( *devnode, deviceRequires ))  {
                RH_TRACE(_createHelperLog, "Partition Matching failed");
                device=devices.erase(device);
                continue;
            }
            device++;
        }

        if ( devices.size() == 0 ) {
            throw redhawk::PlacementFailure(deployment->getInstantiation(), "failed to satisfy devicerequires specification.");
        }
    }


    // First check to see if the component was assigned in the user provided DAS
    // See if a device was assigned in the DAS
    if (!assignedDeviceId.empty()) {
        RH_TRACE(_createHelperLog, "User-provided DAS: Component: '" << deployment->getSoftPkg()->getName() <<
                  "'  Assigned device: '" << assignedDeviceId << "'");
        ossie::DeviceList::iterator device;
        for (device = devices.begin(); device != devices.end(); ++device) {
            if (assignedDeviceId == (*device)->identifier) {
                break;
            }
        }

        if (device == devices.end()) {
            RH_DEBUG(_createHelperLog, "DAS specified unknown device " << assignedDeviceId <<
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
    
    RH_TRACE(_createHelperLog, "alloc prop size " << allocationProperties.size() );
    redhawk::PropertyMap::iterator iter=allocationProperties.begin();
    for( ; iter != allocationProperties.end(); iter++){
      RH_TRACE(_createHelperLog, "alloc prop: " << iter->id  <<" value:" <<  ossie::any_to_string(iter->value) );
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

    redhawk::PropertyMap &_allocationProperties = redhawk::PropertyMap::cast(allocationProperties);
    if ( specialized_reservations.size() > 0 ) {
        redhawk::PropertyMap _struct;
        std::string instantiationId = deployment->getInstantiation()->instantiationId;
        std::vector<std::string> _kinds, _values;
        _kinds.push_back("cpucores");
        if (specialized_reservations.find(instantiationId) == specialized_reservations.end()) {
            _values.push_back("-1");
        } else {
            std::ostringstream ss;
            ss<<specialized_reservations.find(instantiationId)->second;
            _values.push_back(ss.str());
        }
        _struct["redhawk::reservation_request::kinds"].setValue(_kinds);
        _struct["redhawk::reservation_request::values"].setValue(_values);
        _struct["redhawk::reservation_request::obj_id"].setValue(deployment->getIdentifier());
        _allocationProperties["redhawk::reservation_request"].setValue(_struct);
    }

    ossie::AllocationResult response = this->_allocationMgr->allocateDeployment(requestid,
                                                                                _allocationProperties,
                                                                                devices,
                                                                                appIdentifier,
                                                                                implementation->getProcessors(),
                                                                                implementation->getOsDeps(),
                                                                                deviceRequires );
    if (_allocationProperties.contains("nic_allocation")) {
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
                    RH_DEBUG(_createHelperLog, "Allocation NIC assignment: " << interface );
                    deployment->setNicAssignment(interface);
                }
            }
        }
    }
    return response;
}


bool createHelper::checkPartitionMatching( ossie::DeviceNode& devnode,
                                           const CF::Properties& devicerequires )
{
    //
    // perform matching of a device's deployrequires property set against a componentplacment's devicerequires list
    //

    if ( devnode.requiresProps.size() != devicerequires.length()) {
        RH_TRACE(_createHelperLog, "Number of devicerequired properties for deployment does not match, Device: " << devnode.label );
        return false;
    }

    // Check if the device has a required property set for deployment
    if ( devicerequires.length() == 0 ) {
        RH_TRACE(_createHelperLog, "Component and Device have no devicerequires/deployerrequires property sets.");
        return true;
    }

    const redhawk::PropertyMap &devReqs = redhawk::PropertyMap::cast( devicerequires );
    for ( redhawk::PropertyMap::const_iterator iter=devReqs.begin(); iter != devReqs.end(); ++iter) {
        std::string pid(iter->getId());
        RH_TRACE(_createHelperLog, "checkPartitionMatching source devicerequires id:  " << pid );
        redhawk::PropertyMap::const_iterator dev_prop = devnode.requiresProps.find( pid );
        if ( dev_prop == devnode.requiresProps.end() ) {
            RH_DEBUG(_createHelperLog, "Missing devicerequires property: " << pid << " for deployment from Device: " << devnode.label );
            return false;
        }

        // Convert the input Any to the property's data type via string; if it came
        // from the ApplicationFactory, it's already a string, but a remote request
        // could be of any type
        std::string action("eq");
        if (  !ossie::compare_anys(iter->getValue(), dev_prop->getValue(), action)  ) {
            return false;
        }
    }

    RH_TRACE(_createHelperLog, "checkPartitionMatch PASSED, found match with device: " << devnode.label );
    return true;
}


void createHelper::_castRequestProperties(CF::Properties& allocationProperties, const std::vector<ossie::PropertyRef> &prop_refs, unsigned int offset)
{
    allocationProperties.length(offset+prop_refs.size());
    for (unsigned int i=0; i<prop_refs.size(); i++) {
        allocationProperties[offset+i] = ossie::convertPropertyRefToDataType(prop_refs[i].property.get());
    }
}

bool createHelper::resolveSoftpkgDependencies(redhawk::ApplicationDeployment& appDeployment,
                                              redhawk::SoftPkgDeployment* deployment,
                                              ossie::DeviceNode& device)
{
    const ossie::SPD::Implementation* implementation = deployment->getImplementation();
    const SPD::SoftPkgDependencies& deps = implementation->getSoftPkgDependencies();
    SPD::SoftPkgDependencies::const_iterator iterSoftpkg;

    for (iterSoftpkg = deps.begin(); iterSoftpkg != deps.end(); ++iterSoftpkg) {
        // Find an implementation whose dependencies match
        redhawk::SoftPkgDeployment* dependency = resolveDependencyImplementation(appDeployment, *iterSoftpkg, device);
        if (dependency) {
            deployment->addDependency(dependency);
        } else {
            RH_DEBUG(_createHelperLog, "resolveSoftpkgDependencies: implementation match not found between soft package dependency and device");
            return false;
        }
    }

    return true;
}

redhawk::SoftPkgDeployment*
createHelper::resolveDependencyImplementation(redhawk::ApplicationDeployment& appDeployment,
                                              const ossie::SPD::SoftPkgRef& ref,
                                              ossie::DeviceNode& device)
{
    RH_TRACE(_createHelperLog, "Resolving dependency " << ref);
    const SoftPkg* softpkg = _profileCache.loadSoftPkg(ref.localfile);
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

        redhawk::SoftPkgDeployment* dependency = new redhawk::SoftPkgDeployment(softpkg, &implementation);
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

void createHelper::loadAndExecuteContainers(const ContainerList& containers,
                                            CF::ApplicationRegistrar_ptr _appReg)
{
    RH_TRACE(_createHelperLog, "Loading and Executing " << containers.size() << " containers");
    // TODO: Promote contained component affinity values

    BOOST_FOREACH(redhawk::ContainerDeployment* container, containers) {
        boost::shared_ptr<ossie::DeviceNode> device = container->getAssignedDevice();
        if (!device) {
            std::ostringstream message;
            message << "component " << container->getIdentifier() << " was not assigned to a device";
            throw std::logic_error(message.str());
        }

        // Let the application know to expect the given component
        redhawk::ApplicationComponent* app_container = _application->addContainer(container);
        const ossie::ComponentInstantiation* instantiation = container->getInstantiation();
        if (instantiation->isNamingService()) {
            app_container->setNamingContext(_baseNamingContext + "/" + instantiation->getFindByNamingServiceName());
        }
        container->setApplicationComponent(app_container);

        // get the code.localfile
        RH_TRACE(_createHelperLog, "Host is " << device->label << " Local file name is "
                  << container->getLocalFile());

        // Get file name, load if it is not empty
        std::string codeLocalFile = container->getLocalFile();
        if (codeLocalFile.empty()) {
            // This should be caught by validation, but just in case
            throw redhawk::ComponentError(container, "empty localfile");
        }

        // Check for LoadableDevice interface
        if (!device->isLoadable()) {
            std::ostringstream message;
            message << "container " << container->getIdentifier() << " was assigned to non-loadable device "
                    << device->identifier;
            RH_ERROR(_createHelperLog, message);
            throw std::logic_error(message.str());
        }

        RH_TRACE(_createHelperLog, "Loading " << codeLocalFile << " and dependencies on device "
                  << device->label);
        try {
            container->load(_appFact._fileMgr, device->loadableDevice);
        } catch (const std::exception& exc) {
            throw redhawk::ComponentError(container, exc.what());
        }
                
        attemptComponentExecution(_appReg, container);
    }
}

/* Perform 'load' and 'execute' operations to launch component on the assigned device
 *  - Actually loads and executes the component on the given device
 */
void createHelper::loadAndExecuteComponents(const DeploymentList& deployments,
                                            CF::ApplicationRegistrar_ptr _appReg)
{
    RH_TRACE(_createHelperLog, "Loading and Executing " << deployments.size() << " components");
    // apply application affinity options to required components
    applyApplicationAffinityOptions(deployments);

    BOOST_FOREACH(redhawk::ComponentDeployment* deployment, deployments) {
        const std::string& component_id = deployment->getIdentifier();
        RH_TRACE(_createHelperLog, "Loading and executing component '" << component_id << "'");

        boost::shared_ptr<ossie::DeviceNode> device = deployment->getAssignedDevice();
        if (!device) {
            std::ostringstream message;
            message << "component " << component_id << " was not assigned to a device";
            throw std::logic_error(message.str());
        }

        RH_INFO(_createHelperLog, "Application '" << _waveformContextName << "' component '"
                 << component_id << "' assigned to device '" << device->label
                 << "' (" << device->identifier << ")");

        // Let the application know to expect the given component
        redhawk::ApplicationComponent* app_component = _application->addComponent(deployment);
        const ossie::ComponentInstantiation* instantiation = deployment->getInstantiation();
        if (instantiation->isNamingService()) {
            app_component->setNamingContext(_baseNamingContext + "/" + instantiation->getFindByNamingServiceName());
        }
        if (deployment->getContainer()) {
            app_component->setComponentHost(deployment->getContainer()->getApplicationComponent());
        }
        deployment->setApplicationComponent(app_component);

        // get the code.localfile
        RH_TRACE(_createHelperLog, "Host is " << device->label << " Local file name is "
                  << deployment->getLocalFile());

        // Get file name, load if it is not empty
        std::string codeLocalFile = deployment->getLocalFile();
        if (codeLocalFile.empty()) {
            // This should be caught by validation, but just in case
            throw redhawk::ComponentError(deployment, "empty localfile");
        }

        // Check for LoadableDevice interface
        if (!device->isLoadable()) {
            std::ostringstream message;
            message << "component " << component_id << " was assigned to non-loadable device "
                    << device->identifier;
            RH_ERROR(_createHelperLog, message);
            throw std::logic_error(message.str());
        }

        RH_TRACE(_createHelperLog, "Loading " << codeLocalFile << " and dependencies on device "
                  << device->label);
        try {
            deployment->load(_appFact._fileMgr, device->loadableDevice);
        } catch (const std::exception& exc) {
            throw redhawk::ComponentError(deployment, exc.what());
        }
                
        if (deployment->isExecutable()) {
            attemptComponentExecution(_appReg, deployment);
        }
    }
}

int createHelper::resolveDebugLevel( const std::string &level_in ) {
    int  debug_level=-1;
    std::string dlevel = boost::to_upper_copy(level_in);
    rh_logger::LevelPtr rhlevel=ossie::logging::ConvertCanonicalLevelToRHLevel( dlevel );
    debug_level = ossie::logging::ConvertRHLevelToDebug( rhlevel );
    if ( dlevel.at(0) != 'I' and debug_level == 3 ) debug_level=-1;

    // test if number was provided. 
    if ( debug_level == -1  ){
        char *p=NULL;
        int dl=strtol(dlevel.c_str(), &p, 10 );
        if ( p == 0 ) {
            // this will for check valid value and force to info for errant values
            rh_logger::LevelPtr rhlevel=ossie::logging::ConvertDebugToRHLevel( dl );
            debug_level = ossie::logging::ConvertRHLevelToDebug( rhlevel );
        }
    }
    
    return debug_level;    
}

void createHelper::resolveLoggingConfiguration(redhawk::ComponentDeployment* deployment, redhawk::PropertyMap &execParams )
{

    std::string logging_uri("");
    int  debug_level=-1;
    bool resolved_loggingconfig = false;

   // check if logging configuration is part of component placement (loggingconfig in the sad file)
    redhawk::PropertyMap log_config=deployment->getLoggingConfiguration();
    if ( log_config.contains("LOGGING_CONFIG_URI") ) {
        logging_uri = log_config["LOGGING_CONFIG_URI"].toString();
        RH_TRACE(_createHelperLog, "resolveLoggingConfig: loggingconfig log config: " << logging_uri);
    }
    if ( log_config.contains("LOG_LEVEL") ) {
        debug_level = resolveDebugLevel(log_config["LOG_LEVEL"].toString());
        resolved_loggingconfig = true;
        RH_TRACE(_createHelperLog, "resolveLoggingConfig: loggingconfig debug_level: " << debug_level);
    }

    // Use the log config resolver (if enabled)
    const ossie::ComponentInstantiation* instantiation = deployment->getInstantiation();
    if (_appFact._domainManager->getUseLogConfigResolver()) {
        ossie::logging::LogConfigUriResolverPtr logcfg_resolver = ossie::logging::GetLogConfigUriResolver();
        if ( logcfg_resolver ) {
            std::string logcfg_path = ossie::logging::GetComponentPath(_appFact._domainName, _waveformContextName,
                                                                       instantiation->getFindByNamingServiceName());
            std::string uri = logcfg_resolver->get_uri(logcfg_path);
            RH_TRACE(_createHelperLog, "Using LogConfigResolver plugin: path " << logcfg_path << " logcfg: " << uri );
            if ( !uri.empty() ) logging_uri = uri;
        }
    }

    // check for runtime overloads
    if ( execParams.contains("LOGGING_CONFIG_URI") ) {
        logging_uri = execParams["LOGGING_CONFIG_URI"].toString();
        RH_TRACE(_createHelperLog, "resolveLoggingContext:  exec parameter provided, logging cfg uri: " << logging_uri);
        if (debug_level != -1) {
            debug_level = -1;
            resolved_loggingconfig = false;
            RH_TRACE(_createHelperLog, "exec parameter provided, logging cfg uri, remove the debug_level set from loggingconfig");
        }
    }
    if ( execParams.contains("DEBUG_LEVEL") ) {
        debug_level = resolveDebugLevel( execParams["DEBUG_LEVEL"].toString() );
        resolved_loggingconfig = true;
        RH_TRACE(_createHelperLog, "resolveLoggingConfig: exec parameter provided debug_level: " << debug_level);
    }

    if ( execParams.contains("LOG_LEVEL") ) {
        debug_level = resolveDebugLevel( execParams["LOG_LEVEL"].toString() );
        resolved_loggingconfig = true;
        RH_TRACE(_createHelperLog, "resolveLoggingConfig: exec parameter provided log_level: " << debug_level);
    }

    // nothing is provided, use DomainManger's context
    if ( logging_uri.empty() ) {
        // Query the DomainManager for the logging configuration
        RH_DEBUG(_createHelperLog, "Checking DomainManager for LOGGING_CONFIG_URI");
        PropertyInterface *log_prop = _appFact._domainManager->getPropertyFromId("LOGGING_CONFIG_URI");
        StringProperty *logProperty = (StringProperty *)log_prop;
        if (!logProperty->isNil()) {
            logging_uri = logProperty->getValue();
        } else {
            RH_TRACE(_createHelperLog, "DomainManager LOGGING_CONFIG_URI is not set");
        }
    }

    // if logging uri is resolved, then add as execparam
    if (!logging_uri.empty()) {
        if (logging_uri.substr(0, 4) == "sca:") {
            string fileSysIOR = ossie::corba::objectToString(_appFact._domainManager->_fileMgr);
            logging_uri += ("?fs=" + fileSysIOR);
            RH_TRACE(_createHelperLog, "Adding DomainManager's FileSystem IOR " << logging_uri);
        }

        execParams["LOGGING_CONFIG_URI"] = logging_uri;
        RH_DEBUG(_createHelperLog, "resolveLoggingConfiguration: COMP: " << deployment->getIdentifier() << " LOGGING_CONFIG_URI: " << logging_uri);
    }

    // if debug level is resolved, then add as execparam
    if (resolved_loggingconfig) { // check to see if loggingconfig is set on the SAD file
        execParams["DEBUG_LEVEL"] = static_cast<CORBA::Long>(debug_level);
        RH_DEBUG(_createHelperLog, "resolveLoggingConfiguration: COMP: " << deployment->getIdentifier() << " LOG_LEVEL: " << _appFact._domainManager->getInitialLogLevel() );
    } else if ( _appFact._domainManager->getInitialLogLevel() != -1 ) { // check to see if a command-line debug level was used in nodeBooter
        execParams["DEBUG_LEVEL"] = static_cast<CORBA::Long>(_appFact._domainManager->getInitialLogLevel());
        RH_DEBUG(_createHelperLog, "resolveLoggingConfiguration: COMP: " << deployment->getIdentifier() << " LOG_LEVEL: " << _appFact._domainManager->getInitialLogLevel() );
    }
}

void createHelper::attemptComponentExecution (CF::ApplicationRegistrar_ptr registrar,
                                              redhawk::ComponentDeployment* deployment)
{
    // Get executable device reference
    boost::shared_ptr<DeviceNode> device = deployment->getAssignedDevice();
    if (!device->isExecutable()){
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
    execParameters["PROFILE_NAME"] = deployment->getSoftPkg()->getSPDFile();

    execParameters["DOM_PATH"] = _baseNamingContext;
    resolveLoggingConfiguration(deployment, execParameters);

    // Add the Naming Context IOR last to make it easier to parse the command line
    execParameters["NAMING_CONTEXT_IOR"] = ossie::corba::objectToString(registrar);

    // Get entry point
    std::string entryPoint = deployment->getEntryPoint();
    if (entryPoint.empty()) {
        RH_WARN(_createHelperLog, "executing using code file as entry point; this is non-SCA compliant behavior; entrypoint must be set");
        entryPoint = deployment->getLocalFile();
    }

    // Get the complete list of dependencies to include in executeLinked
    std::vector<std::string> resolved_softpkg_deps = deployment->getDependencyLocalFiles();
    CF::StringSequence dep_seq;
    dep_seq.length(resolved_softpkg_deps.size());
    for (unsigned int p=0;p!=dep_seq.length();p++) {
        dep_seq[p]=CORBA::string_dup(resolved_softpkg_deps[p].c_str());
    }

    // Attempt to execute the component
    CF::ExecutableDevice_var execdev;
    if (deployment->getContainer()) {
        RH_TRACE(_createHelperLog, "Executing " << entryPoint << " via container on device " << device->label);
        redhawk::ComponentDeployment* container = deployment->getContainer();
        CF::Resource_var resource = container->getResourcePtr();
        execdev = CF::ExecutableDevice::_narrow(resource);
    } else {
        RH_TRACE(_createHelperLog, "Executing " << entryPoint << " on device " << device->label);
        execdev = CF::ExecutableDevice::_duplicate(device->executableDevice);
    }
    for (redhawk::PropertyMap::iterator prop = execParameters.begin(); prop != execParameters.end(); ++prop) {
        RH_TRACE(_createHelperLog, " exec param " << prop->getId() << " " << prop->getValue().toString());
    }

    // Get options list
    redhawk::PropertyMap options = deployment->getOptions(); 
    for (redhawk::PropertyMap::iterator opt = options.begin(); opt != options.end(); ++opt) {
        RH_TRACE(_createHelperLog, " RESOURCE OPTION: " << opt->getId()
                  << " " << opt->getValue().toString());
    }

    CF::ExecutableDevice::ProcessID_Type pid = -1;
    try {
        // call 'execute' on the ExecutableDevice to execute the component
        pid = execdev->executeLinked(entryPoint.c_str(), options, execParameters, dep_seq);
    } catch (const CF::InvalidFileName& exc) {
        throw redhawk::ExecuteError(deployment, "invalid filename " + std::string(exc.msg));
    } catch (const CF::Device::InvalidState& exc) {
        std::string message = "invalid device state " + std::string(exc.msg);
        throw redhawk::ExecuteError(deployment, message);
    } catch (const CF::ExecutableDevice::InvalidParameters& exc) {
        std::string message = "invalid parameters " + redhawk::PropertyMap::cast(exc.invalidParms).toString();
        throw redhawk::ExecuteError(deployment, message);
    } catch (const CF::ExecutableDevice::InvalidOptions& exc) {
        std::string message = "invalid options " + redhawk::PropertyMap::cast(exc.invalidOpts).toString();
        throw redhawk::ExecuteError(deployment, message);
    } catch (const CF::ExecutableDevice::ExecuteFail& exc) {
        std::string message = "execute failure " + std::string(exc.msg);
        throw redhawk::ExecuteError(deployment, message);
    } catch (const CORBA::SystemException& exc) {
        throw redhawk::ExecuteError(deployment, ossie::corba::describeException(exc));
    } catch (...) {
        // Should never happen, but turn anything else into an ExecuteError
        // just in case
        throw redhawk::ExecuteError(deployment, "unexpected error");
    }

    // handle pid output
    if (pid < 0) {
        throw redhawk::ExecuteError(deployment, "execute returned invalid process ID");
    } else {
        redhawk::ApplicationComponent* app_component = deployment->getApplicationComponent();
        app_component->setProcessId(pid);
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
          RH_INFO(_createHelperLog, " Applying Application Affinity: directive id:"  <<  dt.id << "/" <<  ossie::any_to_string( dt.value )) ;
      }
    
      //
      // Promote NIC affinity for all components deployed on the same device
      //
      boost::shared_ptr<ossie::DeviceNode> deploy_on_device;
      for (unsigned int rc_idx = 0; rc_idx < deployments.size(); rc_idx++) {
          redhawk::ComponentDeployment* deployment = deployments[rc_idx];
          if (!(deployment->getNicAssignment().empty())) {
              deploy_on_device = deployment->getAssignedDevice();
          }
      }

      if (deploy_on_device) {
          for (unsigned int rc_idx = 0; rc_idx < deployments.size (); rc_idx++) {
              redhawk::ComponentDeployment* deployment = deployments[rc_idx];
              boost::shared_ptr<ossie::DeviceNode> dev = deployment->getAssignedDevice();
              // for matching device deployments then apply nic affinity settings
              if (dev->identifier == deploy_on_device->identifier) {
                  deployment->mergeAffinityOptions(app_affinity);
              }
          }
      }
    }
}


void createHelper::waitForContainerRegistration(redhawk::ApplicationDeployment& appDeployment)
{
    // Wait for any containers to be registered before continuing
    int timeout = _appFact._domainManager->getComponentBindingTimeout();
    RH_TRACE(_createHelperLog, "Waiting " << timeout << "s for containers to register");
    std::set<std::string> expected_components;
    BOOST_FOREACH(redhawk::ContainerDeployment* container, appDeployment.getContainerDeployments()) {
        expected_components.insert(container->getIdentifier());
    }

    // Record current time, to measure elapsed time in the event of a failure
    time_t start = time(NULL);

    // Wait for all required components to register, adding additional context
    // to any termination exceptions that may be raised
    bool complete = _application->waitForComponents(expected_components, timeout);
    // TODO: convert into ExecuteError

    // For reference, determine much time has really elapsed.
    time_t elapsed = time(NULL)-start;
    if (!complete) {
        RH_ERROR(_createHelperLog, "Timed out waiting for container to register (" << elapsed << "s elapsed)");
    } else {
        RH_DEBUG(_createHelperLog, "Container registration completed in " << elapsed << "s");
    }

    // Fetch the objects, finding any components that did not register
    BOOST_FOREACH(redhawk::ContainerDeployment* container, appDeployment.getContainerDeployments()) {
        // Check that the component host registered with the application; it
        // should have a valid CORBA reference
        CORBA::Object_ptr objref = container->getApplicationComponent()->getComponentObject();
        if (CORBA::is_nil(objref)) {
            throw redhawk::ExecuteError(container, "container did not register with application");
        }

        CF::Resource_var resource = ossie::corba::_narrowSafe<CF::Resource>(objref);
        if (CORBA::is_nil(resource)) {
            throw redhawk::ComponentError(container, "component object is not a CF::Resource");
        }

        container->setResourcePtr(resource);
    }
}

void createHelper::waitForComponentRegistration(redhawk::ApplicationDeployment& appDeployment)
{
    // Wait for all components to be registered before continuing
    int componentBindingTimeout = _appFact._domainManager->getComponentBindingTimeout();
    RH_TRACE(_createHelperLog, "Waiting " << componentBindingTimeout << "s for all components to register");

    // Track only SCA-compliant components; non-compliant components will never
    // register with the application, nor do they need to be initialized
    std::set<std::string> expected_components;
    const DeploymentList& deployments = appDeployment.getComponentDeployments();
    for (DeploymentList::const_iterator dep = deployments.begin(); dep != deployments.end(); ++dep) {
        if ((*dep)->getSoftPkg()->isScaCompliant()) {
            expected_components.insert((*dep)->getIdentifier());
        }
    }

    // Record current time, to measure elapsed time in the event of a failure
    time_t start = time(NULL);

    // Wait for all required components to register, adding additional context
    // to any termination exceptions that may be raised
    bool complete;
    try {
        complete = _application->waitForComponents(expected_components, componentBindingTimeout);
    } catch (const redhawk::ComponentTerminated& exc) {
        redhawk::ComponentDeployment* deployment = appDeployment.getComponentDeploymentByUniqueId(exc.identifier());
        if (!deployment) {
            // The deployment should always be found, but in the event that it
            // isn't, rethrow the original exception just in case; the outer
            // create() exception handler will turn it into a CF exception
            throw;
        }
        std::string message = "component terminated before registering with application";
        message += ::getVersionMismatchMessage(deployment->getSoftPkg());
        throw redhawk::ExecuteError(deployment, message);
    }

    // For reference, determine much time has really elapsed.
    time_t elapsed = time(NULL)-start;
    if (!complete) {
        RH_ERROR(_createHelperLog, "Timed out waiting for components to register (" << elapsed << "s elapsed)");
    } else {
        RH_DEBUG(_createHelperLog, "Component registration completed in " << elapsed << "s");
    }

    // Fetch the objects, finding any components that did not register
    BOOST_FOREACH(redhawk::ComponentDeployment* deployment, deployments) {
        const SoftPkg* softpkg = deployment->getSoftPkg();
        if (softpkg->isScaCompliant()) {
            // Check that the component registered with the application; it
            // should have a valid CORBA reference
            CORBA::Object_ptr objref = deployment->getApplicationComponent()->getComponentObject();
            if (CORBA::is_nil(objref)) {
                std::string message = "component did not register with application";
                message += ::getVersionMismatchMessage(softpkg);
                throw redhawk::ExecuteError(deployment, message);
            }

            // Occasionally, omniORB may have a cached connection where the
            // other end has terminated (this is particularly a problem with
            // Java, because the Sun ORB never closes connections on shutdown).
            // If the new component just happens to have the same TCP/IP
            // address and port, the first time we try to reach the component,
            // it will get a CORBA.COMM_FAILURE exception even though the
            // reference is valid. In this case, a call to _non_existent()
            // should cause omniORB to clean up the stale socket, and any
            // subsequent calls behave normally.
            try {
                objref->_non_existent();
            } catch (...) {
                RH_DEBUG(_createHelperLog, "Component object did not respond to initial ping");
            }

            // Convert to a CF::Resource object
            if (deployment->isResource()) {
                CF::Resource_var resource = ossie::corba::_narrowSafe<CF::Resource>(objref);
                if (CORBA::is_nil(resource)) {
                    throw redhawk::ComponentError(deployment, "component object is not a CF::Resource");
                }

                deployment->setResourcePtr(resource);
            }
        }
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
    RH_TRACE(_createHelperLog, "initializing " << deployments.size() << " waveform components");

    for (unsigned int rc_idx = 0; rc_idx < deployments.size (); rc_idx++) {
        redhawk::ComponentDeployment* deployment = deployments[rc_idx];
        const ossie::SoftPkg* softpkg = deployment->getSoftPkg();

        // If the component is non-SCA compliant then we don't expect anything beyond this
        if (!softpkg->isScaCompliant()) {
            RH_TRACE(_createHelperLog, "Component is non SCA-compliant, continuing to next component");
            continue;
        } else if (!deployment->isResource()) {
            RH_TRACE(_createHelperLog, "Component is not a resource, continuing to next component");
            continue;
        }

        deployment->initialize();
    }
}

void createHelper::configureComponents(const DeploymentList& deployments)
{
    redhawk::ComponentDeployment* ac_deployment = 0;
    for (DeploymentList::const_iterator depl = deployments.begin(); depl != deployments.end(); ++depl) {
        redhawk::ComponentDeployment* deployment = (*depl);
        if (deployment->isAssemblyController()) {
            ac_deployment = deployment;
        } else {
            deployment->configure();
        }
    }

    // Configure the assembly controller last, if it's configurable
    if (ac_deployment) {
        ac_deployment->configure();
    }
}

/* Connect the components
 *  - Connect the components
 */
void createHelper::connectComponents(redhawk::ApplicationDeployment& appDeployment,
                                     std::vector<ConnectionNode>& connections,
                                     string base_naming_context)
{
    const std::vector<Connection>& _connection = _appFact._sadParser.getConnections ();

    // Create an AppConnectionManager to resolve and track all connections in the application.
    using ossie::AppConnectionManager;
    AppConnectionManager connectionManager(_appFact._domainManager, &appDeployment, &appDeployment, base_naming_context);

    // Create all resource connections
    RH_TRACE(_createHelperLog, "Establishing " << _connection.size() << " waveform connections")
    for (int c_idx = _connection.size () - 1; c_idx >= 0; c_idx--) {
        const Connection& connection = _connection[c_idx];

        RH_TRACE(_createHelperLog, "Processing connection " << connection.getID());

        // Attempt to resolve the connection; if any connection fails, application creation fails.
        bool resolved;
        try {
            resolved = connectionManager.resolveConnection(connection);
        } catch (const std::exception& exc) {
            throw redhawk::ConnectionError(connection.getID(), exc.what());
        }
        if (!resolved) {
            throw redhawk::ConnectionError(connection.getID(), "connection failed");
        }
    }

    // Copy all established connections into the connection array
    const std::vector<ConnectionNode>& establishedConnections = connectionManager.getConnections();
    std::copy(establishedConnections.begin(), establishedConnections.end(), std::back_inserter(connections));
}

std::vector<std::string> createHelper::getStartOrder(const DeploymentList& deployments)
{
    RH_TRACE(_createHelperLog, "Assigning start order");

    // Now that all of the components are known, bin the start orders based on
    // the values in the SAD. Using a multimap, keyed on the start order value,
    // accounts for duplicate keys and allows assigning the effective order
    // easily by iterating through all entries.
    typedef std::multimap<int,redhawk::ComponentDeployment*> StartOrderMap;
    StartOrderMap start_map;
    for (size_t index = 0; index < deployments.size(); ++index) {
        redhawk::ComponentDeployment* deployment = deployments[index];
        const ossie::ComponentInstantiation* instantiation = deployment->getInstantiation();
        if (deployment->isAssemblyController()) {
            RH_TRACE(_createHelperLog, "Component " << instantiation->getID()
                      << " is the assembly controller");
        } else if (instantiation->hasStartOrder()) {
            // Only track start order if it was provided, and the component is
            // not the assembly controller
            start_map.insert(std::make_pair(instantiation->getStartOrder(), deployment));
        }
    }

    // Build the start order vector in the right order
    std::vector<std::string> start_order;
    int index = 1;
    for (StartOrderMap::iterator ii = start_map.begin(); ii != start_map.end(); ++ii, ++index) {
        RH_TRACE(_createHelperLog, index << ": "
                  << ii->second->getInstantiation()->getID());
        start_order.push_back(ii->second->getIdentifier());
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
    _createHelperLog(appFact.returnLogger()),
    _allocationMgr(_appFact._domainManager->_allocationMgr),
    _allocations(*_allocationMgr),
    _waveformContextName(waveformContextName),
    _baseNamingContext(baseNamingContext),
    _waveformContext(CosNaming::NamingContext::_duplicate(waveformContext)),
    _domainContext(domainContext),
    _profileCache(_appFact._fileMgr, appFact.returnLogger()),
    _isComplete(false),
    _application(0),
    _stopTimeout(DEFAULT_STOP_TIMEOUT),
    _aware(true)
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
        _appFact._domainManager->cancelPendingApplication(_application);
        _application->releaseComponents();
        _application->terminateComponents();
        _application->unloadComponents();
        _application->_cleanupActivations();
    }

    RH_TRACE(_createHelperLog, "Removing all bindings from naming context");
    try {
      if ( _appFact._domainManager && !_appFact._domainManager->bindToDomain() ) {
        ossie::corba::unbindAllFromContext(_waveformContext);
      }
    } CATCH_RH_WARN(_createHelperLog, "Could not unbind contents of naming context");

    CosNaming::Name DNContextname;
    DNContextname.length(1);
    DNContextname[0].id = _waveformContextName.c_str();
    RH_TRACE(_createHelperLog, "Unbinding the naming context")
    try {
        _appFact._domainContext->unbind(DNContextname);
    } catch ( ... ) {
    }

    RH_TRACE(_createHelperLog, "Destroying naming context");
    try {
        _waveformContext->destroy();
    } CATCH_RH_WARN(_createHelperLog, "Could not destroy naming context");
}
