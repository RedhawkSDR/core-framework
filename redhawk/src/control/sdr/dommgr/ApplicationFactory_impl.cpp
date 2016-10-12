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

#include <boost/filesystem/path.hpp>

#include <ossie/CF/WellKnownProperties.h>
#include <ossie/FileStream.h>
#include <ossie/prop_helpers.h>

#include "Application_impl.h"
#include "ApplicationFactory_impl.h"
#include "DomainManager_impl.h"
#include "AllocationManager_impl.h"

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



/** Rotates a device list to put the device with the given identifier first
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

static std::vector<std::string> mergeProcessorDeps(const ossie::ImplementationInfo::List& implementations)
{
    // this function merges the overlap in processors between the different components that have been selected
    std::vector<std::string> processorDeps;
    for (ossie::ImplementationInfo::List::const_iterator impl = implementations.begin(); impl != implementations.end(); ++impl) {
        const std::vector<std::string>& implDeps = (*impl)->getProcessorDeps();
        if (!implDeps.empty()) {
            if (processorDeps.empty()) {
                // No prior processor dependencies, so overwrite
                processorDeps = implDeps;
            } else {
                std::vector<std::string> toremove;
                toremove.resize(0);
                for (std::vector<std::string>::iterator proc = processorDeps.begin(); proc != processorDeps.end(); ++proc) {
                    if (std::find(implDeps.begin(), implDeps.end(), *proc) == implDeps.end()) {
                        toremove.push_back(*proc);
                    }
                }
                for (std::vector<std::string>::iterator _rem = toremove.begin(); _rem != toremove.end(); ++_rem) {
                    std::vector<std::string>::iterator proc = std::find(processorDeps.begin(), processorDeps.end(), *_rem);
                    if (proc != processorDeps.end()) {
                        processorDeps.erase(proc);
                    }
                }
            }
        }
    }
    return processorDeps;
}

static std::vector<ossie::SPD::NameVersionPair> mergeOsDeps(const ossie::ImplementationInfo::List& implementations)
{
    // this function merges the overlap in operating systems between the different components that have been selected
    std::vector<ossie::SPD::NameVersionPair> osDeps;
    for (ossie::ImplementationInfo::List::const_iterator impl = implementations.begin(); impl != implementations.end(); ++impl) {
        const std::vector<ossie::SPD::NameVersionPair>& implDeps = (*impl)->getOsDeps();
        if (!implDeps.empty()) {
            if (osDeps.empty()) {
                // No prior OS dependencies, so overwrite
                osDeps = implDeps;
            } else {
                std::vector<ossie::SPD::NameVersionPair> toremove;
                toremove.resize(0);
                for (std::vector<ossie::SPD::NameVersionPair>::iterator pair = osDeps.begin(); pair != osDeps.end(); ++pair) {
                    if (std::find(implDeps.begin(), implDeps.end(), *pair) == implDeps.end()) {
                        toremove.push_back(*pair);
                    }
                }
                for (std::vector<ossie::SPD::NameVersionPair>::iterator _rem = toremove.begin(); _rem != toremove.end(); ++_rem) {
                    std::vector<ossie::SPD::NameVersionPair>::iterator pair = std::find(osDeps.begin(), osDeps.end(), *_rem);
                    if (pair != osDeps.end()) {
                        osDeps.erase(pair);
                    }
                }
            }
        }
    }
    return osDeps;
}

PREPARE_LOGGING(ApplicationFactory_impl);

ApplicationFactory_impl::ApplicationFactory_impl (
    const char*                             softwareProfile, 
    CF::DomainManager::ApplicationSequence* unused, 
    string                                  domainName, 
    string                                  domainManagerName, 
    DomainManager_impl*                     domainManager) :
        _lastWaveformUniqueId(0)
    {

    // save off private data fields
    _domainName        = domainName;
    _domainManagerName = domainManagerName;
    _domainManager     = domainManager;
    _softwareProfile   = softwareProfile;

    // Get a reference to the domain
    CORBA::Object_var obj_DN;
    try {
        obj_DN = ossie::corba::objectFromName(_domainName.c_str());
    } catch( CORBA::SystemException& ex ) {
        LOG_ERROR(ApplicationFactory_impl, "get_object_from_name threw CORBA::SystemException");
        throw;
    } catch ( std::exception& ex ) {
        LOG_ERROR(ApplicationFactory_impl, "The following standard exception occurred: "<<ex.what()<<" while retrieving the domain name")
        throw;
    } catch ( const CORBA::Exception& ex ) {
        LOG_ERROR(ApplicationFactory_impl, "The following CORBA exception occurred: "<<ex._name()<<" while retrieving the domain name")
        throw;
    } catch( ... ) {
        LOG_ERROR(ApplicationFactory_impl, "get_object_from_name threw Unknown Exception");
        throw;
    }

    // Get the naming context from the domain
    _domainContext = ossie::corba::_narrowSafe<CosNaming::NamingContext> (obj_DN);
    if (CORBA::is_nil(_domainContext)) {
        LOG_ERROR(ApplicationFactory_impl, "CosNaming::NamingContext::_narrow threw Unknown Exception");
        throw;
    }

    _dmnMgr = domainManager->_this();

    try {
        _fileMgr = _dmnMgr->fileMgr();
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
        LOG_ERROR(ApplicationFactory_impl, "_dmnMgr->_fileMgr failed with Unknown Exception");
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, "Could not get File Manager from Domain Manager");
    }

    try {
        File_stream _sad(_fileMgr, _softwareProfile.c_str());
        _sadParser.load(_sad);
        _sad.close();
    } catch (const ossie::parser_error& ex) {
        ostringstream eout;
        eout << "Failed to parse SAD file " << _softwareProfile << " " << ex.what();
        LOG_ERROR(ApplicationFactory_impl, eout.str());
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_ENOENT, eout.str().c_str());
    } catch ( std::exception& ex ) {
        ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while loading "<<_softwareProfile;
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch ( const CORBA::Exception& ex ) {
        ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while loading "<<_softwareProfile;
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, eout.str().c_str());
    } catch( ... ) {
        ostringstream eout;
        eout << "Parsing SAD failed with unknown exception;";
        LOG_ERROR(ApplicationFactory_impl, eout.str());
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_ENOENT, eout.str().c_str());
    }

    // Makes sure all external port names are unique
    const std::vector<SoftwareAssembly::Port>& ports = _sadParser.getExternalPorts();
    std::vector<std::string> extPorts;
    for (std::vector<SoftwareAssembly::Port>::const_iterator port = ports.begin(); port != ports.end(); ++port) {
        // Gets name to use
        std::string extName;
        if (port->externalname != "") {
            extName = port->externalname;
        } else {
            extName = port->identifier;
        }
        // Check for duplicate
        if (std::find(extPorts.begin(), extPorts.end(), extName) == extPorts.end()) {
            extPorts.push_back(extName);
        } else {
            ostringstream eout;
            eout << "Duplicate External Port name: " << extName;
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::DomainManager::ApplicationInstallationError(CF::CF_NOTSET, eout.str().c_str());
        }
    }

    // Gets the assembly controller software profile by looping through each
    // component instantiation to find a matching ID to the AC's
    std::string assemblyControllerId = _sadParser.getAssemblyControllerRefId();
    CORBA::String_var profile = "";
    bool foundAc = false;
    std::vector<ComponentPlacement> components = _sadParser.getAllComponents();
    for (std::vector<ComponentPlacement>::const_iterator comp = components.begin();
            comp != components.end() && !foundAc; ++comp) {
        std::vector<ComponentInstantiation> compInstantiations = comp->instantiations;
        for (std::vector<ComponentInstantiation>::const_iterator compInst = compInstantiations.begin();
                compInst != compInstantiations.end() && !foundAc; ++compInst){
            if (assemblyControllerId == compInst->instantiationId) {
                profile = _sadParser.getSPDById(comp->getFileRefId());
                foundAc = true;
            }
        }
    }

    // Gets the assembly controllers properties
    SoftPkg spd;
    Properties prf;
    if (foundAc) {
        if (profile == NULL) {
            ostringstream eout;
            eout << "Invalid assembly controller SPD filename";
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::DomainManager::ApplicationInstallationError(CF::CF_NOTSET, eout.str().c_str());
        }
        try {
            File_stream _spd(_fileMgr, profile);
            spd.load(_spd, static_cast<const char*>(profile));
            _spd.close();
        } catch( ... ) {
            // Errors are reported at create time
        }
        if ( spd.getPRFFile() ) {
            try {
                File_stream _prf(_fileMgr, spd.getPRFFile());
                prf.load(_prf);
                _prf.close();
            } catch( ... ) {
                // Errors are reported at create time
            }
        }
    }

    // Makes sure all external property names are unique
    const std::vector<SoftwareAssembly::Property>& properties = _sadParser.getExternalProperties();
    std::vector<std::string> extProps;
    for (std::vector<SoftwareAssembly::Property>::const_iterator prop = properties.begin(); prop != properties.end(); ++prop) {
        // Gets name to use
        std::string extName;
        if (prop->externalpropid != "") {
            extName = prop->externalpropid;
        } else {
            extName = prop->propid;
        }
        // Check for duplicate
        if (std::find(extProps.begin(), extProps.end(), extName) == extProps.end()) {
            extProps.push_back(extName);
        } else {
            ostringstream eout;
            eout << "Duplicate External Property name: " << extName;
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::DomainManager::ApplicationInstallationError(CF::CF_NOTSET, eout.str().c_str());
        }
    }

    // Make sure AC prop ID's aren't in conflict with external ones
    const std::vector<const Property*>& acProps = prf.getProperties();
    for (unsigned int i = 0; i < acProps.size(); ++i) {
        // Check for duplicate
        if (std::find(extProps.begin(), extProps.end(), acProps[i]->getID()) == extProps.end()) {
            extProps.push_back(acProps[i]->getID());
        } else {
            ostringstream eout;
            eout << "Assembly controller property in use as External Property: " << acProps[i]->getID();
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::DomainManager::ApplicationInstallationError(CF::CF_NOTSET, eout.str().c_str());
        }
    }

    _name = _sadParser.getName();
    _identifier = _sadParser.getID();
}

void createHelper::_cleanupApplicationCreateFailed()
{
    TRACE_ENTER(ApplicationFactory_impl);

    if (!_alreadyCleaned)
    { _cleanupConnectionFailed(); }
}

void createHelper::_cleanupConnectionFailed()
{
    TRACE_ENTER(ApplicationFactory_impl);

    _alreadyCleaned = true;
    _cleanupResourceInitializeFailed();
}

void createHelper::_cleanupResourceNotFound()
{
    TRACE_ENTER(ApplicationFactory_impl);

    _alreadyCleaned = true;
    _cleanupLoadAndExecuteComponents();
}

void createHelper::_cleanupAssemblyControllerInitializeFailed()
{
    TRACE_ENTER(ApplicationFactory_impl);

    _alreadyCleaned = true;
    _cleanupResourceNotFound();
}

void createHelper::_cleanupAssemblyControllerConfigureFailed()
{
    TRACE_ENTER(ApplicationFactory_impl);

    _alreadyCleaned = true;
    _cleanupAssemblyControllerInitializeFailed();
}

void createHelper::_cleanupResourceInitializeFailed()
{
    TRACE_ENTER(ApplicationFactory_impl);

    _alreadyCleaned = true;

    // Try to call releaseObject() on any components that were successfully 
    // launched and registered before terminating the processes
    for (size_t ii = 0; ii < _requiredComponents.size(); ++ii) {
        CF::Resource_var resource = _requiredComponents[ii]->getResourcePtr();
        if (!CORBA::is_nil(resource)) {
            try {
                unsigned long timeout = 3; // seconds
                omniORB::setClientCallTimeout(resource, timeout * 1000);
                resource->releaseObject();
            } catch (...) {
            }
        }
    }

    _cleanupAssemblyControllerConfigureFailed();
}

void createHelper::_cleanupResourceConfigureFailed()
{
    TRACE_ENTER(ApplicationFactory_impl);

    _alreadyCleaned = true;
    _cleanupConnectionFailed();
}

void createHelper::_cleanupRequiredComponents()
{
    TRACE_ENTER(ApplicationFactory_impl);

    _alreadyCleaned = true;
    _deleteRequiredComponents();
}

void createHelper::_deleteRequiredComponents()
{
    TRACE_ENTER(ApplicationFactory_impl);

    for (unsigned int x = 0; x < _requiredComponents.size(); x++)
        { delete _requiredComponents[x]; }

    _requiredComponents.resize(0);
}

/**
 * _cleanupCollocation
 * For a list of ComponentInfo objects, try to deallocate any capacities that were assigned during the create process
 * by the ApplicationFactory.  This method is usually called when an host collocation requests failed to allocate all the
 * components in the request due to capacity failures  It could be possible the failure occurred before all the components
 * were allocated against any devices thus the allocPropTable might not have an entry for the component in the placedComponents
 * list.
 *
 * @param  placedComponents the list of ComponentInfo objects for a host collocation request in a SAD.xml file
 */
void createHelper::_cleanupCollocation(  CapacityAllocationTable &collocCapacities, DeviceAssignmentList &collocAssignedDevs )
{

    TRACE_ENTER(ApplicationFactory_impl);

    //
    // deallocate all items in the capacity table
    //
    LOG_TRACE(ApplicationFactory_impl, "   CAPACITY TABLE SIZE: " << collocCapacities.size());
    CapacityAllocationTable::iterator comp_alloc = collocCapacities.begin();
    for( ; comp_alloc != collocCapacities.end(); ) {
        LOG_TRACE(ApplicationFactory_impl, "   DEALLOCATE: COMP INST_ID : " << comp_alloc->first << " -- START -- ");
        if ( comp_alloc->second.size() > 0) {
            _undoCapacityAllocations(  comp_alloc->second );
        }

         // remove all entries for the set
         comp_alloc->second.clear();
         LOG_TRACE(ApplicationFactory_impl, "   DEALLOCATE: COMP INST_ID : " << comp_alloc->first << " ## END ## ");
         // remove the component's entry from the table
         collocCapacities.erase( comp_alloc++);

    }
    // remove device associations for this component
    try {
        LOG_TRACE(ApplicationFactory_impl, "COLLOCATION CLEANUP ALLOCATED DEVICES SIZE: " << collocAssignedDevs.size() );
        DeviceAssignmentList::iterator d = collocAssignedDevs.begin();
        for ( ; d != collocAssignedDevs.end(); d++ ){
            LOG_TRACE(ApplicationFactory_impl, "    COMP ID:" << d->deviceAssignment.componentId << " DEV ID:" <<     d->deviceAssignment.assignedDeviceId );
        }
        collocAssignedDevs.clear();
        LOG_TRACE(ApplicationFactory_impl, "FINSHED_CLEANUP: ALLOCATED DEVICES SIZE: " << collocAssignedDevs.size() );
    }
    catch(...) {

    }

    LOG_TRACE(ApplicationFactory_impl, "FINSHED COLLOCATION CLEANUP ");
}

void createHelper::_cleanupAllocateDevices()
{
    TRACE_ENTER(ApplicationFactory_impl);

    _alreadyCleaned = true;
    this->_allocations.deallocate();

    _cleanupNewContext();
}

void createHelper::_undoCapacityAllocations( CapacityAllocationList & alloc_set)
{
    for (unsigned int i = 0; i < alloc_set.size(); i++) {
        LOG_TRACE(ApplicationFactory_impl, "_undoCapacityAllocations: deallocating")
        if ( alloc_set[i].device )
            alloc_set[i].device->deallocateCapacity(alloc_set[i].properties);
        else {
            LOG_TRACE(ApplicationFactory_impl, "_undoCapacityAllocations: Device Object INVALID");
        }
        LOG_TRACE(ApplicationFactory_impl, "_undoCapacityAllocations: Finished deallocating")
    }
}

void createHelper::_cleanupNewContext()
{
    TRACE_ENTER(ApplicationFactory_impl);

    _alreadyCleaned = true;
    CosNaming::Name DNContextname;
    DNContextname.length(1);
    DNContextname[0].id = _waveformContextName.c_str();
    LOG_TRACE(ApplicationFactory_impl, "Unbinding the naming context")
    try {
        _appFact._domainContext->unbind(DNContextname);
    } catch ( ... ) {
    }

    _cleanupRequiredComponents();
}

void createHelper::_cleanupLoadAndExecuteComponents()
{
    TRACE_ENTER(ApplicationFactory_impl);

    _alreadyCleaned = true;
    for (unsigned int rc_idx = 0; rc_idx < _requiredComponents.size (); rc_idx++) {
        ossie::ComponentInfo* component = _requiredComponents[rc_idx];
        std::string componentId(component->getIdentifier());

        if (_runningComponentTable.count(componentId) != 0) {
            std::string deviceId = _runningComponentTable[componentId].first;
            unsigned long processId = _runningComponentTable[componentId].second;
            CF::Device_var device = find_device_from_id(deviceId.c_str());
            if (CORBA::is_nil(device)){
                ostringstream eout;
                eout << "Not cleaning up for device: '" << deviceId << "';";
                eout << " Could not find the id";
                LOG_WARN(ApplicationFactory_impl, eout)
            }

            CF::ExecutableDevice_var execdev = ossie::corba::_narrowSafe<CF::ExecutableDevice> (device);
            if (CORBA::is_nil(execdev)) {
                LOG_WARN(ApplicationFactory_impl, "CF::ExecutableDevice::_narrow failed for " << deviceId);
            } else {
                try {
                    execdev->terminate (processId);
                } CATCH_LOG_WARN(ApplicationFactory_impl, "CF::ExecutableDevice::terminate failed for " << deviceId);
            }
        }

        if (_loadedComponentTable.count(componentId) != 0) {
            std::string deviceId = _loadedComponentTable[componentId].first;
            std::string fileName = _loadedComponentTable[componentId].second;
            CF::Device_var device = find_device_from_id(deviceId.c_str());
            if (CORBA::is_nil(device)){
                ostringstream eout;
                eout << "Not cleaning up for device: '" << deviceId << "';";
                eout << " Could not find the id";
                LOG_WARN(ApplicationFactory_impl, eout)
            }

            CF::LoadableDevice_var loadabledev = ossie::corba::_narrowSafe<CF::LoadableDevice> (device);
            if (CORBA::is_nil(loadabledev)) {
                LOG_WARN(ApplicationFactory_impl, "CF::LoadableDevice::_narrow failed for " << deviceId);
            } else {
                try {
                    loadabledev->unload (fileName.c_str());
                } CATCH_LOG_WARN(ApplicationFactory_impl, "CF::LoadableDevice::unload failed for " << deviceId);
            }
        }
    }

    _cleanupAllocateDevices();
}

ApplicationFactory_impl::~ApplicationFactory_impl ()
{
}

void createHelper::_loadAndExecuteComponents() 
{
    try {
        try {
            loadAndExecuteComponents(&_pidSeq, 
                                     &_fileTable, 
                                     _waveformContext, 
                                     &_loadedComponentTable, 
                                     &_runningComponentTable);
        } catch (...) {
            _cleanupLoadAndExecuteComponents(); // clean up and rethrow
            throw;
        }
    } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
        throw;
    } CATCH_THROW_LOG_TRACE(ApplicationFactory_impl,
        "Load-and-execute of component failed (unclear which component/device is the problem)",
        CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, "Load-and-execute of component failed (unclear which component/device is the problem)"));
}

void createHelper::_initializeComponents(CF::Resource_var& assemblyController){
    try{
        initializeComponents(assemblyController, _waveformContext);
    } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
        throw;
    } CATCH_THROW_LOG_TRACE(ApplicationFactory_impl,
        "Initialize of component failed (unclear which component/device is the problem)",
        CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL,"Initialize of component failed (unclear which component/device is the problem)"));
}

/**
 * Check to make sure assemblyController was initialized if it was SCA compliant
 */
void createHelper::_checkAssemblyController(
    CF::Resource_var&      assemblyController,
    ossie::ComponentInfo*& assemblyControllerComponent) const
{
    if (CORBA::is_nil(assemblyController)) {
        if ((assemblyControllerComponent==NULL) || 
            (assemblyControllerComponent->isScaCompliant())
           ) {
        LOG_DEBUG(ApplicationFactory_impl, "assembly controller is not Sca Compliant or has not been assigned");
        throw (CF::ApplicationFactory::CreateApplicationError(
                    CF::CF_NOTSET, 
                    "assembly controller is not Sca Compliant or has not been assigned"));
        }
    }
}

void createHelper::_connectComponents(std::vector<ConnectionNode>& connections){
    try{
        connectComponents(connections, _baseNamingContext);
    } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
        throw;
    } CATCH_THROW_LOG_TRACE(
        ApplicationFactory_impl,
        "Connecting components failed (unclear where this occurred)",
        CF::ApplicationFactory::CreateApplicationError(
            CF::CF_EINVAL, 
            "Connecting components failed (unclear where this occurred)"));
}

void createHelper::_configureComponents()
{
    try{
        configureComponents();
    } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
        throw;
    } CATCH_THROW_LOG_TRACE(
        ApplicationFactory_impl, 
        "Configure on component failed (unclear where in the process this occurred)",
        CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, "Configure of component failed (unclear where in the process this occurred)"))
}

void createHelper::assignRemainingComponentsToDevices()
{
    PlacementList::iterator componentIter;
    for (componentIter  = _requiredComponents.begin(); 
         componentIter != _requiredComponents.end(); 
         componentIter++)
    {
        if (!(*componentIter)->isAssignedToDevice()) {
            allocateComponent(*componentIter, std::string(), _appUsedDevs);
        }
    }
}

void createHelper::initialize(void) {
    _alreadyCleaned = false;

    _pidSeq.length (0);
    _fileTable.clear();
    _loadedComponentTable.clear();  // mapping of component id to 
                                    // filenames/device id tuple
    _runningComponentTable.clear(); // mapping of component id to 
                                    // filenames/device id tuple
    _namingCtxSeq.length (0);
    _implSeq.length (0);
}

void createHelper::checkRegisteredDevicesSize(const char* name)
{
    if (_registeredDevices.size() == 0) {
        ostringstream eout;
        eout << "The domain has no devices (and therefore cannot support the creation of waveform " << name << ")";
        LOG_WARN(ApplicationFactory_impl, eout.str());
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
    }
}

void createHelper::_assignComponentsUsingDAS(const DeviceAssignmentMap& deviceAssignments)
{
    LOG_TRACE(ApplicationFactory_impl, "Assigning " << deviceAssignments.size() 
              << " component(s) based on DeviceAssignmentSequence");

    for (DeviceAssignmentMap::const_iterator ii = deviceAssignments.begin(); ii != deviceAssignments.end(); ++ii) {
        const std::string& componentId = ii->first;
        const std::string& assignedDeviceId = ii->second;
        LOG_TRACE(ApplicationFactory_impl, "Component " << componentId << " is assigned to device " << assignedDeviceId);
        ossie::ComponentInfo* component = findComponentByInstantiationId(componentId);

        if (!component) {
            LOG_ERROR(ApplicationFactory_impl, "Failed to create application; "
                      << "unknown component " << componentId 
                      << " in user assignment (DAS)");
            CF::DeviceAssignmentSequence badDAS;
            badDAS.length(1);
            badDAS[0].componentId = componentId.c_str();
            badDAS[0].assignedDeviceId = assignedDeviceId.c_str();
            throw CF::ApplicationFactory::CreateApplicationRequestError(badDAS);
        }
        allocateComponent(component, assignedDeviceId, _appUsedDevs);
    }
}

void createHelper::_resolveImplementations(PlacementList::iterator comp, PlacementList& compList, std::vector<ossie::ImplementationInfo::List> &res_vec)
{
    if (comp == compList.end()) {
        return;
    }
    ossie::ImplementationInfo::List comp_imps;
    std::vector<ossie::ImplementationInfo::List> tmp_res_vec = res_vec;
    (*comp)->getImplementations(comp_imps);
    unsigned int old_res_vec_size = res_vec.size();
    if (old_res_vec_size == 0) {
        res_vec.resize(comp_imps.size());
        for (unsigned int ii=0; ii<comp_imps.size(); ii++) {
            res_vec[ii].resize(1);
            res_vec[ii][0] = comp_imps[ii];
        }
    } else {
        res_vec.resize(old_res_vec_size * comp_imps.size());
        for (unsigned int i=0; i<old_res_vec_size; i++) {
            for (unsigned int ii=0; ii<comp_imps.size(); ii++) {
                unsigned int res_vec_idx = i*comp_imps.size()+ii;
                res_vec[res_vec_idx] = tmp_res_vec[i];
                res_vec[res_vec_idx].insert(res_vec[res_vec_idx].begin(),comp_imps[ii]);
            }
        }
    }
    this->_resolveImplementations(++comp, compList, res_vec);
    return;
}

void createHelper::_removeUnmatchedImplementations(std::vector<ossie::ImplementationInfo::List> &res_vec)
{
    std::vector<ossie::ImplementationInfo::List>::iterator impl_list = res_vec.begin();
    while (impl_list != res_vec.end()) {
        std::vector<ossie::ImplementationInfo::List>::iterator old_impl_list = impl_list;
        ossie::ImplementationInfo::List::iterator impl = (*impl_list).begin();
        std::vector<ossie::SPD::NameVersionPair> reference_pair = (*impl)->getOsDeps();
        std::vector<std::string> reference_procs = (*impl)->getProcessorDeps();
        bool os_init_to_zero = (reference_pair.size()==0);
        bool proc_init_to_zero = (reference_procs.size()==0);
        impl++;
        bool match = true;
        while (impl != (*impl_list).end()) {
            std::vector<ossie::SPD::NameVersionPair> pair = (*impl)->getOsDeps();
            std::vector<std::string> procs = (*impl)->getProcessorDeps();
            bool os_must_match = false;
            bool proc_must_match = false;
            if (os_init_to_zero)
                os_must_match = false;
            if (proc_init_to_zero)
                proc_must_match = false;
            if ((reference_pair.size() != 0) and (pair.size() != 0)) {os_must_match = true;}
            if ((reference_procs.size() != 0) and (procs.size() != 0)) {proc_must_match = true;}
            // if os must match (because both lists are non-zero length), check that at least one of the sets matches
            if (os_must_match) {
                bool at_least_one_match = false;
                for (std::vector<ossie::SPD::NameVersionPair>::iterator ref=reference_pair.begin(); ref<reference_pair.end(); ref++) {
                    for (std::vector<ossie::SPD::NameVersionPair>::iterator cur=pair.begin(); cur<pair.end(); cur++) {
                        if ((*ref)==(*cur)) {
                            at_least_one_match = true;
                            break;
                        }
                    }
                }
                if (!at_least_one_match) {match = false;break;}
            }
            // if proc must match (because both lists are non-zero length), check that at least one of the sets matches
            if (proc_must_match) {
                bool at_least_one_match = false;
                for (std::vector<std::string>::iterator ref=reference_procs.begin(); ref<reference_procs.end(); ref++) {
                    for (std::vector<std::string>::iterator cur=procs.begin(); cur<procs.end(); cur++) {
                        if ((*ref)==(*cur)) {
                            at_least_one_match = true;
                        }
                    }
                }
                if (!at_least_one_match) {match = false;break;}
            }
            // reduce the number of os that can be used as a reference to the overlapping set
            if (reference_pair.size()>pair.size()) {
                for (std::vector<ossie::SPD::NameVersionPair>::iterator ref=reference_pair.begin(); ref<reference_pair.end(); ref++) {
                    bool found_match = false;
                    for (std::vector<ossie::SPD::NameVersionPair>::iterator cur=pair.begin(); cur<pair.end(); cur++) {
                        if ((*ref)==(*cur)) {
                            found_match = true;
                        }
                    }
                    if (not found_match) {
                        reference_pair.erase(ref);
                    }
                }
            }
            // reduce the number of procs that can be used as a reference to the overlapping set
            if (reference_procs.size()>procs.size()) {
                for (std::vector<std::string>::iterator ref=reference_procs.begin(); ref<reference_procs.end(); ref++) {
                    bool found_match = false;
                    for (std::vector<std::string>::iterator cur=procs.begin(); cur<procs.end(); cur++) {
                        if ((*ref)==(*cur)) {
                            found_match = true;
                        }
                    }
                    if (not found_match) {
                        reference_procs.erase(ref);
                    }
                }
            }
            // if the initial entity did not have an os, add it if the current one holds an os requirement
            if (os_init_to_zero) {
                if (pair.size() != 0) {
                    os_init_to_zero = false;
                    for (std::vector<ossie::SPD::NameVersionPair>::iterator cur=pair.begin(); cur<pair.end(); cur++) {
                        reference_pair.push_back((*cur));
                    }
                }
            }
            // if the initial entity did not have a proc, add it if the current one holds a proc requirement
            if (proc_init_to_zero) {
                if (procs.size() != 0) {
                    proc_init_to_zero = false;
                    for (std::vector<std::string>::iterator cur=procs.begin(); cur<procs.end(); cur++) {
                        reference_procs.push_back((*cur));
                    }
                }
            }
            impl++;
        }
        if (not match) {
            (*impl_list).erase(impl);
        }
        impl_list++;
    }
    return;
}

void createHelper::_consolidateAllocations(const ossie::ImplementationInfo::List& impls, CF::Properties& allocs)
{
    allocs.length(0);
    for (ossie::ImplementationInfo::List::const_iterator impl= impls.begin(); impl != impls.end(); ++impl) {
        const std::vector<SPD::PropertyRef>& deps = (*impl)->getDependencyProperties();
        for (std::vector<SPD::PropertyRef>::const_iterator dep = deps.begin(); dep != deps.end(); ++dep) {
            if (dynamic_cast<const SimplePropertyRef*>((*dep).property) != NULL) {
                const SimplePropertyRef* dependency = dynamic_cast<const SimplePropertyRef*>((*dep).property);
                ossie::corba::push_back(allocs, convertPropertyToDataType(dependency));
            } else if (dynamic_cast<const SimpleSequencePropertyRef*>((*dep).property) != NULL) {
                const SimpleSequencePropertyRef* dependency = dynamic_cast<const SimpleSequencePropertyRef*>((*dep).property);
                ossie::corba::push_back(allocs, convertPropertyToDataType(dependency));
            } else if (dynamic_cast<const ossie::StructPropertyRef*>((*dep).property) != NULL) {
                const ossie::StructPropertyRef* dependency = dynamic_cast<const ossie::StructPropertyRef*>((*dep).property);
                const std::map<std::string, std::string> structval = dependency->getValue();
                ossie::corba::push_back(allocs, convertPropertyToDataType(dependency));
            } else if (dynamic_cast<const ossie::StructSequencePropertyRef*>((*dep).property) != NULL) {
                const ossie::StructSequencePropertyRef* dependency = dynamic_cast<const ossie::StructSequencePropertyRef*>((*dep).property);
                ossie::corba::push_back(allocs, convertPropertyToDataType(dependency));
            }
        }
    }
}

void createHelper::_handleHostCollocation()
{
    const std::vector<SoftwareAssembly::HostCollocation>& hostCollocations =
        _appFact._sadParser.getHostCollocations();
    LOG_TRACE(ApplicationFactory_impl,
              "Assigning " << hostCollocations.size()
                    << " collocated groups of components");

    for (unsigned int ii = 0; ii < hostCollocations.size(); ++ii) {
        _placeHostCollocation(hostCollocations[ii]);
    }
}

void createHelper::_placeHostCollocation(const SoftwareAssembly::HostCollocation& collocation)
{
    LOG_TRACE(ApplicationFactory_impl,
              "-- Begin placment for Collocation " <<
              collocation.getName() << " " <<
              collocation.getID());

    PlacementList placingComponents;
    std::vector<ossie::ImplementationInfo::List> res_vec;

    // Some components may have been placed by a user DAS; keep a
    // list of those that still need to be assigned to a device.
    //PlacementList placingComponents;

    // Keep track of devices to which some of the components have
    // been assigned.
    DeviceIDList assignedDevices;

    const std::vector<ComponentPlacement>& collocatedComponents =
        collocation.getComponents();

    _getComponentsToPlace(collocatedComponents,
                          assignedDevices,
                          placingComponents);

    // create every combination of implementations for the components in the set
    // for each combination:
    //  consolidate allocations
    //  attempt allocation
    //  if the allocation succeeds, break the loop
    this->_resolveImplementations(placingComponents.begin(), placingComponents, res_vec);
    this->_removeUnmatchedImplementations(res_vec);

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
    

    for (size_t index = 0; index < res_vec.size(); ++index) {
        // Merge processor and OS dependencies from all implementations
        std::vector<std::string> processorDeps = mergeProcessorDeps(res_vec[index]);
        std::vector<ossie::SPD::NameVersionPair> osDeps = mergeOsDeps(res_vec[index]);

        // Consolidate the allocation properties into a single list
        CF::Properties allocationProperties;
        this->_consolidateAllocations(res_vec[index], allocationProperties);

        const std::string requestid = ossie::generateUUID();
        ossie::AllocationResult response = this->_allocationMgr->allocateDeployment(requestid, allocationProperties, deploymentDevices, processorDeps, osDeps);
        if (!response.first.empty()) {
            // Ensure that all capacities get cleaned up
            this->_allocations.push_back(response.first);

            // Convert from response back into a device node
            boost::shared_ptr<ossie::DeviceNode>& node = response.second;
            const std::string& deviceId = node->identifier;

            PlacementList::iterator comp = placingComponents.begin();
            ossie::ImplementationInfo::List::iterator impl = res_vec[index].end()-1;
            DeviceAssignmentList      collocAssignedDevs;
            collocAssignedDevs.resize(placingComponents.size());
            for (unsigned int i=0; i<collocAssignedDevs.size(); i++,comp++,impl--) {
                collocAssignedDevs[i].device = CF::Device::_duplicate(node->device);
                collocAssignedDevs[i].deviceAssignment.assignedDeviceId = CORBA::string_dup(deviceId.c_str());
                (*comp)->setSelectedImplementation(*impl);
                if (!resolveSoftpkgDependencies(*impl, *node)) {
                    LOG_TRACE(ApplicationFactory_impl, "Unable to resolve softpackage dependencies for component "
                              << (*comp)->getIdentifier() << " implementation " << (*impl)->getId());
                    continue;
                }
                (*comp)->setAssignedDeviceId(deviceId.c_str());
                collocAssignedDevs[i].deviceAssignment.componentId = CORBA::string_dup((*comp)->getIdentifier());
            }
            
            // Move the device to the front of the list
            rotateDeviceList(_executableDevices, deviceId);

            _appUsedDevs.insert(_appUsedDevs.end(),
                                collocAssignedDevs.begin(),
                                collocAssignedDevs.end());
            LOG_TRACE(ApplicationFactory_impl, "-- Completed placement for Collocation ID:" << collocation.id << " Components Placed: " << collocatedComponents.size());
            return;
        }
    }

    std::ostringstream eout;
    eout << "Could not collocate components for collocation NAME: " << collocation.getName() << "  ID:" << collocation.id;
    LOG_ERROR(ApplicationFactory_impl, eout.str());
    throw CF::ApplicationFactory::CreateApplicationRequestError();
}

void createHelper::_getComponentsToPlace(
    const std::vector<ComponentPlacement>& collocatedComponents,
    DeviceIDList&                          assignedDevices,
    PlacementList&                         placingComponents)
{
    std::vector<ComponentPlacement>::const_iterator placement =
        collocatedComponents.begin();

    for (; placement != collocatedComponents.end(); ++placement) {
        ComponentInstantiation instantiation =
            (placement->getInstantiations()).at(0);
        ossie::ComponentInfo* component =
            findComponentByInstantiationId(instantiation.getID());

        if (!component) {
            ostringstream eout;
            eout << "failed to create application; unable to recover component Id (error parsing the SAD file "<<_appFact._softwareProfile<<")";
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::ApplicationFactory::CreateApplicationError(
                CF::CF_EAGAIN,
                eout.str().c_str());
        }
        LOG_TRACE(ApplicationFactory_impl,
                  "Collocated component " <<
                        component->getInstantiationIdentifier());

        if (component->isAssignedToDevice()) {
            // This component is already assigned to a device; for collocating
            // other components, the pre-assigned devices are used in the order
            // they are encountered.
            LOG_TRACE(ApplicationFactory_impl,
                      "Already assigned to device " <<
                      component->getAssignedDeviceId());
            assignedDevices.push_back( component->getAssignedDeviceId() );

        } else {
            // This component needs to be assigned to a device.
            placingComponents.push_back(component);
        }
    }
}

void createHelper::_handleUsesDevices(const std::string& appName)
{
    // Gets all uses device info from the SAD file
    const UsesDeviceInfo::List& usesDevices = _appInfo.getUsesDevices();
    LOG_TRACE(ApplicationFactory_impl, "Application has " << usesDevices.size() << " usesdevice dependencies");
    const CF::Properties& appProperties = _appInfo.getACProperties();
    // The device assignments for SAD-level usesdevices are never stored
    DeviceAssignmentList assignedDevices;
    if (!allocateUsesDevices(appName, usesDevices, appProperties, assignedDevices, this->_allocations)) {
        // There were unsatisfied usesdevices for the application
        ostringstream eout;
        eout << "Failed to satisfy 'usesdevice' dependencies ";
        bool first = true;
        for (UsesDeviceInfo::List::const_iterator uses = usesDevices.begin(); uses != usesDevices.end(); ++uses) {
            if ((*uses)->getAssignedDeviceId().empty()) {
                if (!first) {
                    eout << ", ";
                } else {
                    first = false;
                }
                eout << (*uses)->getId();
            }
        }
        eout << "for application '" << appName << "'";
        LOG_DEBUG(ApplicationFactory_impl, eout.str());
        _cleanupAllocateDevices();
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
    }
}

void createHelper::setUpExternalPorts(
    std::auto_ptr<Application_impl>& application)
{
    const std::vector<SoftwareAssembly::Port>& ports =
        _appInfo.getExternalPorts();
    LOG_TRACE(ApplicationFactory_impl,
              "Mapping " << ports.size() << " external port(s)");
    std::vector<SoftwareAssembly::Port>::const_iterator port;

    for (port = ports.begin(); port != ports.end(); ++port) {
        LOG_TRACE(ApplicationFactory_impl,
                  "Port component: " << port->componentrefid
                        << " Port identifier: " << port->identifier);

        // Get the component from the instantiation identifier.
        CORBA::Object_var obj =
            lookupComponentByInstantiationId(port->componentrefid);
        if (CORBA::is_nil(obj)) {
            LOG_ERROR(ApplicationFactory_impl,
                      "Invalid componentinstantiationref ("
                            <<port->componentrefid
                            <<") given for an external port ");
            throw(CF::ApplicationFactory::CreateApplicationError(
                CF::CF_NOTSET,
                "Invalid componentinstantiationref given for external port"));
        }

        if (port->type == SoftwareAssembly::Port::SUPPORTEDIDENTIFIER) {
            if (!obj->_is_a(port->identifier.c_str())) {
                LOG_ERROR(
                    ApplicationFactory_impl,
                    "Component does not support requested interface: "
                        << port->identifier);
                throw(CF::ApplicationFactory::CreateApplicationError(
                    CF::CF_NOTSET,
                    "Component does not support requested interface"));
            }
        } else {
            // Must be either "usesidentifier" or "providesidentifier",
            // which are equivalent unless you want to be extra
            // pedantic and check how the port is described in the
            // component's SCD.

            CF::PortSupplier_var portSupplier =
                ossie::corba::_narrowSafe<CF::PortSupplier> (obj);

            // Try to look up the port.
            try {
                obj = portSupplier->getPort(port->identifier.c_str());
            } CATCH_THROW_LOG_ERROR(
                ApplicationFactory_impl,
                "Invalid port id",
                CF::ApplicationFactory::CreateApplicationError(
                    CF::CF_NOTSET,
                    "Invalid port identifier"))
        }

        // Add it to the list of external ports on the application object.
        if (port->externalname == ""){
            application->addExternalPort(port->identifier, obj);
        } else {
            application->addExternalPort(port->externalname, obj);
        }
    }
}

void createHelper::setUpExternalProperties(
    std::auto_ptr<Application_impl>& application)
{
    const std::vector<SoftwareAssembly::Property>& props = _appInfo.getExternalProperties();
    LOG_TRACE(ApplicationFactory_impl, "Mapping " << props.size() << " external property(ies)");
    for (std::vector<SoftwareAssembly::Property>::const_iterator prop = props.begin(); prop != props.end(); ++prop) {
        LOG_TRACE(ApplicationFactory_impl, "Property component: " << prop->comprefid << " Property identifier: " << prop->propid);

        // Verify internal property
        ComponentInfo *tmp = findComponentByInstantiationId(prop->comprefid);
        if (tmp == 0) {
            LOG_ERROR(ApplicationFactory_impl, "Unable to find component for comprefid " << prop->comprefid);
            throw(CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, "Unable to find component for given comprefid"));
        }
        const std::vector<const Property*>& props = tmp->prf.getProperties();
        bool foundProp = false;
        for (unsigned int i = 0; i < props.size(); ++i) {
            if (props[i]->getID() == prop->propid){
                foundProp = true;
            }
        }
        if (!foundProp){
            LOG_ERROR(ApplicationFactory_impl, "Attempting to promote property: '" <<
                    prop->propid << "' that does not exist in component: '" << prop->comprefid << "'");
            throw (CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET,
                    "Attempting to promote property that does not exist in component"));
        }

        // Get the component from the compref identifier.
        CF::Resource_ptr comp = lookupComponentByInstantiationId(prop->comprefid);
        if (CORBA::is_nil(comp)) {
            LOG_ERROR(ApplicationFactory_impl, "Invalid comprefid (" << prop->comprefid << ") given for an external property");
            throw(CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, "Invalid comprefid given for external property"));
        }

        if (prop->externalpropid == "") {
            application->addExternalProperty(prop->propid,
                                             prop->propid,
                                             comp);
        } else {
            application->addExternalProperty(prop->propid,
                                             prop->externalpropid,
                                             comp);
        }
    }
}

/** Creates and instance of the application.
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
    createHelper new_createhelper(*this, _waveform_context_name, base_naming_context, _waveformContext);

    // now actually perform the create operation
    LOG_TRACE(ApplicationFactory_impl, "Performing 'create' function.");
    CF::Application_ptr new_app = new_createhelper.create(name, initConfiguration, deviceAssignmentMap);

    // return the new Application
    TRACE_EXIT(ApplicationFactory_impl);
    return new_app;
}

CF::Application_ptr createHelper::create (
    const char*                         name,
    const CF::Properties&               initConfiguration,
    const DeviceAssignmentMap& deviceAssignments)
throw (CORBA::SystemException,
       CF::ApplicationFactory::CreateApplicationError,
       CF::ApplicationFactory::CreateApplicationRequestError,
       CF::ApplicationFactory::InvalidInitConfiguration)
{
    TRACE_ENTER(ApplicationFactory_impl);

    try {
        try {
            initialize();

            // Get a list of all device currently in the domain
            _registeredDevices = _appFact._domainManager->getRegisteredDevices();
            _executableDevices.clear();
            for (DeviceList::iterator iter = _registeredDevices.begin(); iter != _registeredDevices.end(); ++iter) {
                if ((*iter)->isExecutable) {
                    _executableDevices.push_back(*iter);
                }
            }
            checkRegisteredDevicesSize(name);

            const std::string lastExecutableDevice = _appFact._domainManager->getLastDeviceUsedForDeployment();
            if (!lastExecutableDevice.empty()) {
                LOG_TRACE(ApplicationFactory_impl, "Placing device " << lastExecutableDevice << " first in deployment list");
                rotateDeviceList(_executableDevices, lastExecutableDevice);
            }

            //////////////////////////////////////////////////
            // Load the components to instantiate from the SAD
            if (_requiredComponents.size() != 0)
            { _deleteRequiredComponents(); }

            getRequiredComponents();

            ossie::ComponentInfo* assemblyControllerComponent = NULL;
            overrideAssemblyControllerProperties(initConfiguration,
                                                 assemblyControllerComponent);

            //////////////////////////////////////////////////
            // Store information about this application
            _appInfo.populateApplicationInfo(_appFact._sadParser);
            for (unsigned int i = 0; i < _requiredComponents.size(); ++i) {
                ComponentInfo *comp = _requiredComponents[i];
                if (comp->isAssemblyController()) {
                    _appInfo.setACProperties(comp->getConfigureProperties());
                }
                _appInfo.addComponent(comp);
            }

            overrideExternalProperties(initConfiguration);

            ////////////////////////////////////////////////
            // Assign components to devices
            ////////////////////////////////////////////////

            /*
             * _appUsedDevs and appCapacityTable represent all the allocations
             * and assigned made during applicaiton deployment. It provides the
             * "context" for the deployment.  This context pattern will be
             * applied again when collocation requests are fullfilled.  There 2
             * container are used to deploy the waveform, and also to "cleanup"
             *  if deployment fails
             */

            // reset list of devices that were used during component
            // allocation/placement process for an application
            _appUsedDevs.resize(0);

            // Start with a empty set of allocation properties, used to keep
            // track of device capacity allocations. If this is not cleared
            // each time, deallocation may start occuring multiple times,
            // resulting in incorrect capacities.
            //_appCapacityTable.clear();

            // Allocate any usesdevice capacities specified in the SAD file
            _handleUsesDevices(name);

            // First, assign components to devices based on the caller supplied
            // DAS.
            _assignComponentsUsingDAS(deviceAssignments);

            // Second, attempt to honor host collocation.
            _handleHostCollocation();

            assignRemainingComponentsToDevices();

            _loadAndExecuteComponents();

            CF::Resource_var assemblyController = CF::Resource::_nil();

            _initializeComponents(assemblyController);

            std::vector<ConnectionNode> connections;

            _connectComponents(connections);

            _configureComponents();

            _checkAssemblyController(assemblyController,
                                     assemblyControllerComponent);


            ////////////////////////////////////////////////
            // Create the Application servant

            // Give the application a unique identifier of the form 
            // "softwareassemblyid:ApplicationName", where the application 
            // name includes the serial number generated for the naming context
            // (e.g. "Application_1").
            std::string appIdentifier = 
                _appFact._identifier + ":" + _waveformContextName;

            // Manage the Application servant with an auto_ptr in case 
            // something throws an exception.
            std::auto_ptr<Application_impl> application(new Application_impl (
                appIdentifier.c_str(), 
                name, 
                _appFact._softwareProfile.c_str(), 
                _appFact._domainManager, 
                _waveformContextName, 
                _waveformContext));

            setUpExternalPorts(application);
            setUpExternalProperties(application);


            ////////////////////////////////////////////////
            // Create the application
            //
            // We are assuming that all components and their resources are 
            // collocated. This means that we assume the SAD <partitioning> 
            // element contains the <hostcollocation> element. NB: Ownership 
            // of the ConnectionManager is passed to the application.
            std::vector<std::string> allocationIDs;
            _allocations.transfer(allocationIDs);
            application->populateApplication(
                assemblyController,
                _appUsedDevs, 
                &_implSeq, 
                _startSeq, 
                &_namingCtxSeq, 
                &_pidSeq,
                connections, 
                _fileTable,
                allocationIDs);

            // Activate the new Application servant, and let the POA manage its
            //  deletion. The DomainManager POA must exist, but the 
            //  Applications POA might not have been created yet.
            PortableServer::POA_var dm_poa = 
                ossie::corba::RootPOA()->find_POA("DomainManager", 0);
            PortableServer::POA_var poa = dm_poa->find_POA("Applications", 1);
            PortableServer::ObjectId_var oid = 
                ossie::corba::activatePersistentObject(poa, 
                                                       application.get(), 
                                                       appIdentifier);

            // The POA now has ownership of the Application.
            Application_impl* app_servant = application.release();
            app_servant->_remove_ref();

            // Add a reference to the new application to the 
            // ApplicationSequence in DomainManager
            CF::Application_var appObj = app_servant->_this();
            addComponentsToApplication(app_servant);

            try {
                _appFact._domainManager->addApplication(app_servant);
            } catch (CF::DomainManager::ApplicationInstallationError& ex) {
                // something bad happened - clean up
                LOG_ERROR(ApplicationFactory_impl, ex.msg);
                poa->deactivate_object(oid);
                throw CF::ApplicationFactory::CreateApplicationError(ex.errorNumber, ex.msg);
            }

            // After all components have been deployed, we know that the first
            // executable device in the list was used for the last deployment,
            // so update the domain manager
            _appFact._domainManager->setLastDeviceUsedForDeployment(_executableDevices.front()->identifier);

            LOG_TRACE(ApplicationFactory_impl, "Cleaning up");
            _deleteRequiredComponents();

            ossie::sendObjectAddedEvent(ApplicationFactory_impl::__logger, _appFact._identifier.c_str(), appIdentifier.c_str(), name,
                                        appObj, StandardEvent::APPLICATION, _appFact._domainManager->proxy_consumer);

            LOG_INFO(ApplicationFactory_impl, "Done creating application " << appIdentifier << " " << name);
            return appObj._retn();
        } catch (...) {
            _cleanupApplicationCreateFailed(); // Cleanup and rethrow
            throw;
        }
    } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
        LOG_ERROR(ApplicationFactory_impl, "Error in application creation; " << ex.msg);
        throw;
    } catch (CF::ApplicationFactory::CreateApplicationRequestError& ex) {
        LOG_ERROR(ApplicationFactory_impl, "Error in application creation")
        throw;
    } catch ( std::exception& ex ) {
        ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while creating the application";
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw (CF::ApplicationFactory::CreateApplicationError(CF::CF_EBADF, eout.str().c_str()));
    } catch ( const CORBA::Exception& ex ) {
        ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while creating the application";
        LOG_ERROR(ApplicationFactory_impl, eout.str())
        throw (CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, eout.str().c_str()));
    } catch ( ... ) {
        LOG_ERROR(ApplicationFactory_impl, "Unexpected error in application creation - see log")
        throw (CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, "Unexpected error in application creation - see log."));
    }

}

/**
 * Call overrideProperties on all components within _requiredComponents
 * that are set to be the assembly controller.
 *
 * Set assemblyControllerComponent.
 */
void createHelper::overrideAssemblyControllerProperties(
        const CF::Properties& initConfiguration,
        ossie::ComponentInfo*& assemblyControllerComponent) {

    for (unsigned int rc_idx = 0; rc_idx < _requiredComponents.size (); rc_idx++) {
        ossie::ComponentInfo* component = _requiredComponents[rc_idx];
        if (component->isAssemblyController()) {
            assemblyControllerComponent = component;

            overrideProperties(initConfiguration, component);
        }
    }
}

void createHelper::overrideExternalProperties(const CF::Properties& initConfiguration)
{
    const std::vector<SoftwareAssembly::Property>& props = _appInfo.getExternalProperties();

    for (unsigned int i = 0; i < initConfiguration.length(); ++i) {
        for (std::vector<SoftwareAssembly::Property>::const_iterator prop = props.begin(); prop != props.end(); ++prop) {
            std::string id;
            if (prop->externalpropid == "") {
                id = prop->propid;
            } else {
                id = prop->externalpropid;
            }

            if (ossie::corba::returnString(initConfiguration[i].id) == id) {
                ComponentInfo *comp = findComponentByInstantiationId(prop->comprefid);
                // Only configure on non AC components
                if (comp != 0 && !comp->isAssemblyController()) {
                    comp->overrideProperty(prop->propid.c_str(), initConfiguration[i].value);
                }
            }
        }
    }
}

void createHelper::overrideProperties(
    const CF::Properties& initConfiguration,
    ossie::ComponentInfo*& component) {
        
    // Override properties
    for (unsigned int initCount = 0; initCount < initConfiguration.length(); initCount++) {
        if(ossie::corba::returnString(initConfiguration[initCount].id) == "LOGGING_CONFIG_URI"){
            // See if the LOGGING_CONFIG_URI has already been set
            // via <componentproperties> or initParams
            bool alreadyHasLoggingConfigURI = false;
            CF::Properties execParameters = component->getExecParameters();
            for (unsigned int i = 0; i < execParameters.length(); ++i) {
                std::string propid = static_cast<const char*>(execParameters[i].id);
                if (propid == "LOGGING_CONFIG_URI") {
                    alreadyHasLoggingConfigURI = true;
                    break;
                }
            }
            // If LOGGING_CONFIG_URI isn't already an exec param, add it
            // Otherwise, don't override component exec param value 
            if (!alreadyHasLoggingConfigURI) {
                // Add LOGGING_CONFIG_URI as an exec param now so that it can be set to the overridden value
                CF::DataType lcuri = initConfiguration[initCount];
                component->addExecParameter(lcuri);
                LOG_TRACE(ApplicationFactory_impl, "Adding LOGGING_CONFIG_URI as exec param with value "
                      << ossie::any_to_string(lcuri.value));
            }
        } else {
            LOG_TRACE(ApplicationFactory_impl, 
                      "Overriding property " << initConfiguration[initCount].id 
                      << " with " << ossie::any_to_string(initConfiguration[initCount].value));
            component->overrideProperty(ossie::corba::returnString(initConfiguration[initCount].id).c_str(), 
                                        initConfiguration[initCount].value);
        }
    }
}

CF::AllocationManager::AllocationResponseSequence* createHelper::allocateUsesDeviceProperties(const UsesDeviceInfo::List& usesDevices, const CF::Properties& configureProperties)
{
    CF::AllocationManager::AllocationRequestSequence request;
    request.length(usesDevices.size());

    for (unsigned int usesdev_idx=0; usesdev_idx< usesDevices.size(); usesdev_idx++) {
        const std::string requestid = usesDevices[usesdev_idx]->getId();
        request[usesdev_idx].requestID = requestid.c_str();

        // Get the usesdevice dependency properties, first from the SPD...
        CF::Properties& allocationProperties = request[usesdev_idx].allocationProperties;
        const std::vector<SPD::PropertyRef>&prop_refs = usesDevices[usesdev_idx]->getProperties();
        this->_castRequestProperties(allocationProperties, prop_refs);

        // ...then from the SAD; in practice, these are mutually exclusive, but
        // there is no harm in doing both, as one set will always be empty
        const std::vector<SoftwareAssembly::PropertyRef>& sad_refs = usesDevices[usesdev_idx]->getSadDeps();
        this->_castRequestProperties(allocationProperties, sad_refs, allocationProperties.length());

        this->_evaluateMATHinRequest(allocationProperties, configureProperties);
    }

    return this->_allocationMgr->allocate(request);
}
                                                          
/** Check all allocation dependencies for a particular component and assign it to a device.
 *  - Check component's overall usesdevice dependencies
 *  - Allocate capacity on usesdevice(s)
 *  - Find and implementation that has it's implementation-specific usesdevice dependencies satisfied
 *  - Allocate the component to a particular device

 TODO: Current implementation takes advantage of single failure then clean up everything..... To support collocation
 allocation failover for mulitple devices, then we need to clean up only the allocations that we made during a failed
 collocation request.  This requires that we know and cleanup only those allocations that we made..
 appCapacityTable holds all the applications that were made during the entire application deployment process.

 I think for each try of a collocation request... we need to swap out the current appCapacityTable for a
 temporary table, to assist with the allocation and clean up

 */
void createHelper::allocateComponent(ossie::ComponentInfo*  component,
                                     const std::string& assignedDeviceId,
                                     DeviceAssignmentList &appAssignedDevs)
{
    // get the implementations from the component
    ossie::ImplementationInfo::List  implementations;
    component->getImplementations(implementations);

    const CF::Properties& configureProperties = component->getConfigureProperties();

    // Find the devices that allocate the SPD's minimum required usesdevices properties
    const UsesDeviceInfo::List &usesDevVec = component->getUsesDevices();
    if (!allocateUsesDevices(component->getIdentifier(), usesDevVec, configureProperties, appAssignedDevs, this->_allocations)) {
        // There were unsatisfied usesdevices for the component
        ostringstream eout;
        eout << "Failed to satisfy 'usesdevice' dependencies ";
        bool first = true;
        for (UsesDeviceInfo::List::const_iterator uses = usesDevVec.begin(); uses != usesDevVec.end(); ++uses) {
            if ((*uses)->getAssignedDeviceId().empty()) {
                if (!first) {
                    eout << ", ";
                } else {
                    first = false;
                }
                eout << (*uses)->getId();
            }
        }
        eout << "for component '" << component->getIdentifier() << "'";
        LOG_DEBUG(ApplicationFactory_impl, eout.str());
        _cleanupAllocateDevices();
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
    }
    
    // now attempt to find an implementation that can have it's allocation requirements met
    for (size_t implCount = 0; implCount < implementations.size(); implCount++) {
        ossie::ImplementationInfo* impl = implementations[implCount];

        // Handle 'usesdevice' dependencies for the particular implementation
        DeviceAssignmentList implAllocatedDevices;
        ScopedAllocations implAllocations(*this->_allocationMgr);
        const UsesDeviceInfo::List &implUsesDevVec = impl->getUsesDevices();
        if (!allocateUsesDevices(component->getIdentifier(), implUsesDevVec, configureProperties, implAllocatedDevices, implAllocations)) {
            LOG_TRACE(ApplicationFactory_impl, "Unable to satisfy 'usesdevice' dependencies for component "
                      << component->getIdentifier() << " implementation " << impl->getId());
            continue;
        }

        // Found an implementation which has its 'usesdevice' dependencies
        // satisfied, now perform assignment/allocation of component to device
        LOG_DEBUG(ApplicationFactory_impl, "Trying to find the device");
        ossie::AllocationResult response = allocateComponentToDevice(component, impl, assignedDeviceId);

        if (response.first.empty()) {
            LOG_TRACE(ApplicationFactory_impl, "Unable to allocate device for component "
                      << component->getIdentifier() << " implementation " << impl->getId());
            continue;
        }

        // Track successful deployment allocation
        implAllocations.push_back(response.first);

        // Convert from response back into a device node
        DeviceNode& node = *(response.second);
        const std::string& deviceId = node.identifier;

        if (!resolveSoftpkgDependencies(impl, node)) {
            LOG_TRACE(ApplicationFactory_impl, "Unable to resolve softpackage dependencies for component "
                      << component->getIdentifier() << " implementation " << impl->getId());
            continue;
        }

        // Allocation to a device succeeded
        LOG_DEBUG(ApplicationFactory_impl, "Assigned component " << component->getInstantiationIdentifier()
                  << " implementation " << impl->getId() << " to device " << deviceId);
        component->setAssignedDeviceId(deviceId.c_str());

        // Move the device to the front of the list
        rotateDeviceList(_executableDevices, deviceId);

        ossie::DeviceAssignmentInfo dai;
        dai.deviceAssignment.componentId = CORBA::string_dup(component->getIdentifier());
        dai.deviceAssignment.assignedDeviceId = deviceId.c_str();
        dai.device = CF::Device::_duplicate(node.device);
        appAssignedDevs.push_back(dai);

        // Store the implementation-specific usesdevice allocations and
        // device assignments
        implAllocations.transfer(this->_allocations);
        std::copy(implAllocatedDevices.begin(), implAllocatedDevices.end(), std::back_inserter(appAssignedDevs));

        component->setSelectedImplementation(impl);
        return;
    }

    // Report failure
    std::ostringstream eout;
    eout << "Failed to satisfy device dependencies for component: '";
    eout << component->getName() << "' with component id: '" << component->getIdentifier() << "'";
    LOG_DEBUG(ApplicationFactory_impl, eout.str());
    _cleanupAllocateDevices();
    throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
}

bool createHelper::allocateUsesDevices(const std::string& componentIdentifier,
                                       const ossie::UsesDeviceInfo::List& usesDevices,
                                       const CF::Properties& configureProperties,
                                       DeviceAssignmentList& deviceAssignments,
                                       ScopedAllocations& allocations)
{
    // Create a temporary lookup table for reconciling allocation requests with
    // usesdevice identifiers
    typedef std::map<std::string,UsesDeviceInfo*> UsesDeviceMap;
    UsesDeviceMap usesDeviceMap;
    for (UsesDeviceInfo::List::const_iterator iter = usesDevices.begin(); iter != usesDevices.end(); ++iter) {
        // Ensure that no devices are assigned to start; the caller can check
        // for unassigned devices to report which usesdevices failed
        (*iter)->clearAssignedDeviceId();
        usesDeviceMap[(*iter)->getId()] = *iter;
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
        uses->second->setAssignedDeviceId(deviceId);
        usesDeviceMap.erase(uses);

        DeviceAssignmentInfo assignment;
        assignment.deviceAssignment.componentId = componentIdentifier.c_str();
        assignment.deviceAssignment.assignedDeviceId = deviceId.c_str();
        assignment.device = CF::Device::_duplicate(response[resp].allocatedDevice);
        deviceAssignments.push_back(assignment);
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

void createHelper::_evaluateMATHinRequest(CF::Properties &request, CF::Properties configureProperties)
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
                // TODO - implement a more relaxed parser
                mathStatement.erase(mathStatement.begin(), mathStatement.begin() + 1);
                mathStatement.erase(mathStatement.end() - 1, mathStatement.end());
                std::vector<std::string> args;
                while ((mathStatement.length() > 0) && (mathStatement.find(',') != std::string::npos)) {
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
            } else {
                std::ostringstream eout;
                eout << " invalid __MATH__ statement; '" << mathStatement << "'";
                throw ossie::PropertyMatchingError(eout.str());
            }
        }
    }
}

/** Perform allocation/assignment of a particular component to the device.
 *  - First do allocation/assignment based on user provided DAS
 *  - If not specified in DAS, then iterate through devices looking for a device that satisfies
 *    the allocation properties
 */
ossie::AllocationResult createHelper::allocateComponentToDevice( ossie::ComponentInfo* component,
                                              ossie::ImplementationInfo* implementation,
                                              const std::string& assignedDeviceId)
{
    ossie::DeviceList devices = _registeredDevices;

    // First check to see if the component was assigned in the user provided DAS
    // See if a device was assigned in the DAS
    if (!assignedDeviceId.empty()) {
        LOG_TRACE(ApplicationFactory_impl, "User-provided DAS: Component: '" << component->getName() <<
                  "'  Assigned device: '" << assignedDeviceId << "'");
        ossie::DeviceList::iterator device;
        for (device = devices.begin(); device != devices.end(); ++device) {
            if (assignedDeviceId == (*device)->identifier) {
                break;
            }
        }

        if (device == devices.end()) {
            LOG_DEBUG(ApplicationFactory_impl, "DAS specified unknown device " << assignedDeviceId <<
                      " for component " << component->getIdentifier());
            CF::DeviceAssignmentSequence badDAS;
            badDAS.length(1);
            badDAS[0].componentId = CORBA::string_dup(component->getIdentifier());
            badDAS[0].assignedDeviceId = assignedDeviceId.c_str();
            throw CF::ApplicationFactory::CreateApplicationRequestError(badDAS);
        }

        // Remove all non-requested devices
        devices.erase(devices.begin(), device++);
        devices.erase(device, devices.end());
    }

    const std::string requestid = ossie::generateUUID();
    std::vector<SPD::PropertyRef> prop_refs = implementation->getDependencyProperties();
    CF::Properties allocationProperties;
    this->_castRequestProperties(allocationProperties, prop_refs);
    this->_evaluateMATHinRequest(allocationProperties, component->getConfigureProperties());
    ossie::AllocationResult response = this->_allocationMgr->allocateDeployment(requestid, allocationProperties, devices, implementation->getProcessorDeps(), implementation->getOsDeps());
    TRACE_EXIT(ApplicationFactory_impl);
    return response;
}

void createHelper::_castRequestProperties(CF::Properties& allocationProperties, const std::vector<ossie::SPD::PropertyRef> &prop_refs, unsigned int offset)
{
    allocationProperties.length(offset+prop_refs.size());
    for (unsigned int i=0; i<prop_refs.size(); i++) {
        allocationProperties[offset+i] = castProperty(prop_refs[i].property);
    }
}

void createHelper::_castRequestProperties(CF::Properties& allocationProperties, const std::vector<ossie::SoftwareAssembly::PropertyRef> &prop_refs, unsigned int offset)
{
    allocationProperties.length(offset+prop_refs.size());
    for (unsigned int i=0; i<prop_refs.size(); i++) {
        allocationProperties[offset+i] = castProperty(prop_refs[i].property);
    }
}

CF::DataType createHelper::castProperty(const ossie::ComponentProperty* property)
{
    if (dynamic_cast<const SimplePropertyRef*>(property) != NULL) {
        const SimplePropertyRef* dependency = dynamic_cast<const SimplePropertyRef*>(property);
        return convertPropertyToDataType(&(*dependency));
    } else if (dynamic_cast<const SimpleSequencePropertyRef*>(property) != NULL) {
        const SimpleSequencePropertyRef* dependency = dynamic_cast<const SimpleSequencePropertyRef*>(property);
        return convertPropertyToDataType(dependency);
    } else if (dynamic_cast<const ossie::StructPropertyRef*>(property) != NULL) {
        const ossie::StructPropertyRef* dependency = dynamic_cast<const ossie::StructPropertyRef*>(property);
        const std::map<std::string, std::string> structval = dependency->getValue();
        return convertPropertyToDataType(dependency);
    } else if (dynamic_cast<const ossie::StructSequencePropertyRef*>(property) != NULL) {
        const ossie::StructSequencePropertyRef* dependency = dynamic_cast<const ossie::StructSequencePropertyRef*>(property);
        return convertPropertyToDataType(dependency);
    }
    CF::DataType dataType;
    dataType.id = CORBA::string_dup(property->_id.c_str());
    return dataType;
}

bool createHelper::resolveSoftpkgDependencies(ossie::ImplementationInfo* implementation, ossie::DeviceNode& device)
{
    const std::vector<SPD::SoftPkgRef>& tmpSoftpkg = implementation->getSoftPkgDependency();
    std::vector<SPD::SoftPkgRef>::const_iterator iterSoftpkg;

    for (iterSoftpkg = tmpSoftpkg.begin(); iterSoftpkg != tmpSoftpkg.end(); ++iterSoftpkg) {
        SoftPkg spd;
        CORBA::String_var profile = iterSoftpkg->localfile.c_str();
        try {
            File_stream _spd(_appFact._fileMgr, profile);
            spd.load(_spd, static_cast<const char*>(profile));
            _spd.close();
        } catch (ossie::parser_error& e) {
            ostringstream eout;
            eout << "creating application error; error parsing spd; " << e.what();
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
        } catch( ... ) {
            ostringstream eout;
            eout << "creating application error; ; unknown error parsing spd;";
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
        }
        
        // Is there an implementation?
        const std::vector <SPD::Implementation>& spd_i = spd.getImplementations();
        unsigned int targetImplementation = 0;
        bool foundImplementation = false;
        if (iterSoftpkg->implref.isSet()) {
            std::string requestedImplementation = *(iterSoftpkg->implref.get());

            for (unsigned int implCount = 0; implCount < spd_i.size(); implCount++) {
                if (requestedImplementation==spd_i[implCount].implementationID) {
                    foundImplementation = checkImplementationDependencyMatch(*implementation, spd_i[implCount], device);
                    if (foundImplementation) {
                        targetImplementation = implCount;
                        break;
                    }
                }
            }
            if (!foundImplementation) {
                LOG_DEBUG(ApplicationFactory_impl, "resolveSoftpkgDependencies: implementation match not found between soft package dependency and device");
                return false;
            }
        } else {
            // If no implementation preference exists, then find the one whose dependencies match
            const std::vector <SPD::Implementation>& spd_i = spd.getImplementations();

            for (unsigned int implCount = 0; implCount < spd_i.size(); implCount++) {
                foundImplementation = checkImplementationDependencyMatch(*implementation, spd_i[implCount], device);
                if (foundImplementation) {
                    targetImplementation = implCount;
                    break;
                } else {
                }
            }
            if (!foundImplementation) {
                LOG_DEBUG(ApplicationFactory_impl, "resolveSoftpkgDependencies: implementation match not found between soft package dependency and device");
                return false;
            }
        }

        ossie::ImplementationInfo spdImplInfo(spd_i[targetImplementation]);
        CF::LoadableDevice_var loadableDevice;
        try {
            loadableDevice = CF::LoadableDevice::_narrow(device.device);
        } catch ( ... ) {
            return false;
        }
        if (CORBA::is_nil(loadableDevice)){
            return false;
        }

        try {
            CF::LoadableDevice::LoadType codeType = CF::LoadableDevice::EXECUTABLE;
            if (spd_i[targetImplementation].code.type.isSet()) {
                std::string type = (*(spd_i[targetImplementation].code.type));
                if (type == "KernelModule") {
                    codeType = CF::LoadableDevice::KERNEL_MODULE;
                } else if (type == "SharedLibrary") {
                    codeType = CF::LoadableDevice::SHARED_LIBRARY;
                } else if (type == "Executable") {
                    codeType = CF::LoadableDevice::EXECUTABLE;
                } else if (type == "Driver") {
                    codeType = CF::LoadableDevice::DRIVER;
                } else {
                }
            }
            fs::path codeLocalFile = fs::path(spdImplInfo.getLocalFileName());
            if (!codeLocalFile.has_root_directory()) {
                codeLocalFile = fs::path(spd.getSPDPath()) / codeLocalFile;
            }
            codeLocalFile = codeLocalFile.normalize();
            if (codeLocalFile.has_leaf() && codeLocalFile.leaf() == ".") {
                codeLocalFile = codeLocalFile.branch_path();
            }
            loadableDevice->load(_appFact._fileMgr, codeLocalFile.string().c_str(), codeType);
        } catch ( ... ) {
            return false;
        }
    }

    return true;
}

bool createHelper::checkImplementationDependencyMatch(
    ossie::ImplementationInfo&       implementation_1, 
    const ossie::ImplementationInfo& implementation_2, 
    ossie::DeviceNode& device)
{
    if (!implementation_2.checkProcessorAndOs(device.prf)) {
        return false;
    }
    
    // resolve dependencies on dependencies
    std::vector<SPD::SoftPkgRef> tmpSoftpkg = implementation_1.getSoftPkgDependency();
    std::vector<SPD::SoftPkgRef>::iterator iterSoftpkg = tmpSoftpkg.begin();
    bool retval = true;
    if (iterSoftpkg != tmpSoftpkg.end()) {
        ossie::ImplementationInfo* tmp_impl = const_cast<ossie::ImplementationInfo*>(&implementation_2);
        retval = resolveSoftpkgDependencies(tmp_impl, device);
    }
    return retval;
}

/** Create a vector of all the components for the SAD associated with this App Factory
 *  - Get component information from the SAD and store in _requiredComponents vector
 */
void createHelper::getRequiredComponents()
 throw (CF::ApplicationFactory::CreateApplicationError)
{
    TRACE_ENTER(ApplicationFactory_impl);

    std::vector<ComponentPlacement> componentsFromSAD = _appFact._sadParser.getAllComponents();

    const char* assemblyControllerRefId = _appFact._sadParser.getAssemblyControllerRefId();
    std::string assemblyControllerRefId_s(assemblyControllerRefId); //string repr for comparison below

    std::vector< std::pair< int, std::string> > _startOrderPairs;
    for (unsigned int i = 0; i < componentsFromSAD.size(); i++) {
        const ComponentPlacement& component = componentsFromSAD[i];
        
        // Create a list of pairs of start orders and instantiation IDs
        for (unsigned int ii = 0; ii < component.getInstantiations().size(); ii++) {
            // Only add a pair if a start order was provided, and the component is not the assembly controller
            if (strcmp(component.getInstantiations()[ii].getStartOrder(), "") != 0 &&
                    component.getInstantiations()[ii].getID() != assemblyControllerRefId_s) {
                // Get the start order of the component
                int startOrder = atoi(component.getInstantiations()[ii].getStartOrder());
                std::string instId = component.getInstantiations()[ii].getID();
                _startOrderPairs.push_back(std::make_pair(startOrder, instId));
            }
        }
        
        // Size the start order instantiation ID vector and initialize it
        _startOrderIds.resize(_startOrderPairs.size());
        for (unsigned int j = 0; j < _startOrderIds.size(); j++) {
            _startOrderIds[j] = "";
        }
        
        // Build the start order instantiation ID vector in the right order
        for (unsigned int jj = 0; jj < _startOrderPairs.size(); jj++) {
            int pos = 0;
            for (unsigned int k = 0; k < _startOrderPairs.size(); k++) {
                if (_startOrderPairs[jj].first > _startOrderPairs[k].first) {
                    pos++;
                }
            }
            
            // Account for multiple start orders with the same value
            for (unsigned int kk = pos; kk < _startOrderIds.size(); kk++) {
                if (strcmp(_startOrderIds[kk].c_str(),"") == 0) {
                    _startOrderIds[pos] = _startOrderPairs[jj].second;
                    break;
                } else {
                    pos++;
                }
            }
        }

        // Extract required data from SPD file
        ossie::ComponentInfo* newComponent = 0;
        LOG_TRACE(ApplicationFactory_impl, "Getting the SPD Filename")
        const char *spdFileName = _appFact._sadParser.getSPDById(component.getFileRefId());
        if (spdFileName == NULL) {
            ostringstream eout;
            eout << "The SPD file reference for componentfile "<<component.getFileRefId()<<" is missing";
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
        }
        LOG_TRACE(ApplicationFactory_impl, "Building Component Info From SPD File")
        newComponent = ossie::ComponentInfo::buildComponentInfoFromSPDFile(_appFact._fileMgr, spdFileName);
        if (newComponent == 0) {
            ostringstream eout;
            eout << "Error loading component information for file ref " << component.getFileRefId();
            LOG_ERROR(ApplicationFactory_impl, eout.str())
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
        }

        newComponent->setSpdFileName(_appFact._sadParser.getSPDById(component.getFileRefId()));

        LOG_TRACE(ApplicationFactory_impl, "Done building Component Info From SPD File")
        // Extract Instantiation data from SAD
        // This is wrong, there can be more than one instantiation per placement
        // Basic fix, iterate over instantiations
        ///\todo Fix for multiple instantiations per component
        const vector<ComponentInstantiation>& instantiations = component.getInstantiations();

        const ComponentInstantiation& instance = instantiations[0];

        ostringstream identifier;
        identifier << instance.getID();
        // Violate SR:172, we use the uniquified name rather than the passed in name
        identifier << ":" << _waveformContextName;
        assert(newComponent != 0);
        newComponent->setIdentifier(identifier.str().c_str(), instance.getID());

        if (strcmp(newComponent->getInstantiationIdentifier(), assemblyControllerRefId) == 0) {
            newComponent->setIsAssemblyController(true);
        }

        newComponent->setNamingService(instance.isNamingService());

        if (newComponent->getNamingService()) {
            ostringstream nameBinding;
            nameBinding << instance.getFindByNamingServiceName();
#if UNIQUIFY_NAME_BINDING
// DON'T USE THIS YET AS IT WILL BREAK OTHER PARTS OF REDHAWK
            nameBinding << "_" << i;  // Add a _UniqueIdentifier, per SR:169
#endif
            newComponent->setNamingServiceName(nameBinding.str().c_str());  // SR:169
        } else {
            if (newComponent->isScaCompliant()) {
                LOG_WARN(ApplicationFactory_impl, "component instantiation is sca compliant but does not provide a 'findcomponent' name...this is probably an error")
            }
        }
    
        newComponent->setUsageName(instance.getUsageName());
        const std::vector<ComponentProperty*>& ins_prop = instance.getProperties();

        for (unsigned int i = 0; i < ins_prop.size(); ++i) {
            newComponent->overrideProperty(ins_prop[i]);
        }

        _requiredComponents.push_back(newComponent);
    }
    TRACE_EXIT(ApplicationFactory_impl);
}

/** Given a device id, returns a CORBA pointer to the device
 *  - Gets a CORBA pointer for a device from a given id
 */
CF::Device_ptr createHelper::find_device_from_id(const char* device_id)
{
    try {
        return CF::Device::_duplicate(find_device_node_from_id(device_id).device);
    } catch ( ... ){
    }

    for (DeviceAssignmentList::iterator iter = _appUsedDevs.begin(); iter != _appUsedDevs.end(); ++iter) {
        if (strcmp(device_id, iter->deviceAssignment.assignedDeviceId) == 0) {
            return CF::Device::_duplicate(iter->device);
        }
    }

    TRACE_EXIT(ApplicationFactory_impl);
    return CF::Device::_nil();
}

const ossie::DeviceNode& createHelper::find_device_node_from_id(const char* device_id) throw(std::exception)
{
    for (DeviceList::iterator dn = _registeredDevices.begin(); dn != _registeredDevices.end(); ++dn) {
        if ((*dn)->identifier == device_id) {
            return **dn;
        }
    }

    TRACE_EXIT(ApplicationFactory_impl);
    throw(std::exception());
}

/** Given a component instantiation id, returns the associated ossie::ComponentInfo object
 *  - Gets the ComponentInfo class instance for a particular component instantiation id
 */
ossie::ComponentInfo* createHelper::findComponentByInstantiationId(const std::string& identifier)
{
    for (size_t ii = 0; ii < _requiredComponents.size(); ++ii) {
        if (identifier == _requiredComponents[ii]->getInstantiationIdentifier()) {
            return _requiredComponents[ii];
        }
    }

    return 0;
}

/** Given a waveform/application name, return a unique waveform naming context
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
        waveform_context_name.append("_");
        ostringstream number_str;
        number_str << _lastWaveformUniqueId;
        waveform_context_name.append(number_str.str());
        string temp_waveform_context(_domainName + string("/"));
        temp_waveform_context.append(waveform_context_name);
        CosNaming::Name_var cosName = ossie::corba::stringToName(temp_waveform_context);
        try {
            CORBA::Object_var obj_WaveformContext = inc->resolve(cosName);
        } catch (const CosNaming::NamingContext::NotFound&) {
            found_empty = true;
        }
    } while (!found_empty);

    return waveform_context_name;

}

/** Given a waveform/application-specific context, return the full waveform naming context
 *  - Returns a full context path for the waveform
 */
string ApplicationFactory_impl::getBaseWaveformContext(string waveform_context)
{
    string base_naming_context(_domainName + string("/"));
    base_naming_context.append(waveform_context);

    return base_naming_context;
}

/** Perform 'load' and 'execute' operations to launch component on the assigned device
 *  - Actually loads and executes the component on the given device
 */
void createHelper::loadAndExecuteComponents(
        CF::Application::ComponentProcessIdSequence*              pid,
        std::map<std::string, std::string>*                       fileTable,
        CosNaming::NamingContext_ptr                              WaveformContext,
        map<std::string, std::pair<std::string, std::string> >*   loadedComponentTable,
        map<std::string, std::pair<std::string, unsigned long> >* runningComponentTable)
{
    LOG_TRACE(ApplicationFactory_impl, "Loading and Executing " << _requiredComponents.size() << " components");

    for (unsigned int rc_idx = 0; rc_idx < _requiredComponents.size (); rc_idx++) {
        ossie::ComponentInfo* component = _requiredComponents[rc_idx];
        const ossie::ImplementationInfo* implementation = component->getSelectedImplementation();

        LOG_TRACE(ApplicationFactory_impl, "Component - " << component->getName()
                << "   Assigned device - " << component->getAssignedDeviceId());

        // get CF::Device pointer to the device to which this component will be assigned
        CF::Device_var device;
        device = find_device_from_id(component->getAssignedDeviceId());
        if (CORBA::is_nil(device)) {
            ostringstream eout;
            eout << "Could not find device with device id: '" << component->getAssignedDeviceId() << "' for component: '";
            eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
            eout << " with implementation id: '" << implementation->getId() << "'";
            eout << " on device id: '" << component->getAssignedDeviceId() << "'";
            eout << " in waveform '" << _waveformContextName<<"'";
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            LOG_TRACE(ApplicationFactory_impl, eout.str())
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
        }

        // get the code.localfile
        fs::path codeLocalFile = fs::path(implementation->getLocalFileName());
        LOG_TRACE(ApplicationFactory_impl, "Host is " << ossie::corba::returnString(device->label()) << " Local file name is "
                << codeLocalFile);
        if (!codeLocalFile.has_root_directory()) {
            codeLocalFile = fs::path(component->spd.getSPDPath()) / codeLocalFile;
        }
        codeLocalFile = codeLocalFile.normalize();
        if (codeLocalFile.has_leaf() && codeLocalFile.leaf() == ".") {
            codeLocalFile = codeLocalFile.branch_path();
        }

        // narrow to LoadableDevice interface
        CF::LoadableDevice_var loadabledev;
        loadabledev = ossie::corba::_narrowSafe<CF::LoadableDevice> (device);
        if (CORBA::is_nil(loadabledev)) {
            ostringstream eout;
            eout << "LoadableDevice narrow failed (probably a non-loadable device) with device id: '" << component->getAssignedDeviceId() << "' for component: '";
            eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
            eout << " with implementation id: '" << implementation->getId() << "'";
            eout << " in waveform '" << _waveformContextName <<"'";
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            LOG_TRACE(ApplicationFactory_impl, eout.str())
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
        }

        // Get file name, load if it is not empty
        if (codeLocalFile.string().size() <=  0) {
            ostringstream eout;
            eout << "code.localfile is empty for component: '";
            eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
            eout << " with implementation id: '" << implementation->getId() << "'";
            eout << " on device id: '" << component->getAssignedDeviceId() << "'";
            eout << " in waveform '" << _waveformContextName<<"'";
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            LOG_TRACE(ApplicationFactory_impl, eout.str())
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EBADF, eout.str().c_str());
        }

        // get File Manager
        CF::FileManager_var fileMgr;
        try {
            fileMgr = _appFact._dmnMgr->fileMgr();
        } catch( ... ) {
            ostringstream eout;
            eout << "Could not get File Manager from Domain Manager for component: '";
            eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
            eout << " with implementation id: '" << implementation->getId() << "';";
            eout << " on device id: '" << component->getAssignedDeviceId() << "'";
            eout << " in waveform '" << _waveformContextName<<"'";
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            LOG_TRACE(ApplicationFactory_impl, eout.str())
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
        }

        // load the file(s)
        (*fileTable)[component->getIdentifier()] = codeLocalFile.string();

        ostringstream load_eout; // used for any error messages dealing with load
        try {
            try {
                LOG_TRACE(ApplicationFactory_impl, "loading " << codeLocalFile << " on device " << ossie::corba::returnString(loadabledev->label()));
                loadabledev->load (fileMgr, codeLocalFile.string().c_str(), implementation->getCodeType());
            } catch( ... ) {
                load_eout << "'load' failed for component: '";
                load_eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
                load_eout << " with implementation id: '" << implementation->getId() << "';";
                load_eout << " on device id: '" << component->getAssignedDeviceId() << "'";
                load_eout << " in waveform '" << _waveformContextName<<"'";
                load_eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                throw;
            }
        } catch( CF::InvalidFileName& _ex ) {
            load_eout << " with error: <" << _ex.msg << ">;";
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, load_eout.str().c_str());
        } catch( CF::Device::InvalidState& _ex ) {
            load_eout << " with error: <" << _ex.msg << ">;";
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, load_eout.str().c_str());
        } CATCH_THROW_LOG_TRACE(ApplicationFactory_impl, "", CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, load_eout.str().c_str()))
                
        (*loadedComponentTable)[component->getIdentifier()] = std::make_pair(component->getAssignedDeviceId(), codeLocalFile.string());

        // OSSIE extends section D.2.1.6.3 to support loading a directory
        // and execute a file in that directory using a entrypoint
        // 1. Executable means to use CF LoadableDevice::load and CF ExecutableDevice::execute operations. This is a "main" process.
        //    - A Executable that references a directory instead of a file means to recursively load the contents of the directory
        //      and then execute the program specified via entrypoint
        // 2. Driver and Kernel Module means load only.
        // 3. SharedLibrary means dynamic linking.
        // 4. A (SharedLibrary) Without a code entrypoint element means load only.
        // 5. A (SharedLibrary) With a code entrypoint element means load and CF Device::execute.
        if (((implementation->getCodeType() == CF::LoadableDevice::EXECUTABLE) ||
                (implementation->getCodeType() == CF::LoadableDevice::SHARED_LIBRARY)) && (implementation->getEntryPoint().size() != 0)) {

            // get executable device reference
            CF::ExecutableDevice_var execdev;
            execdev = ossie::corba::_narrowSafe<CF::ExecutableDevice> (loadabledev);
            if (CORBA::is_nil(execdev)){
                ostringstream eout;
                eout << "ExecutableDevice narrow failed to device with device id: '" << component->getAssignedDeviceId() << "' for component: '";
                eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
                eout << " with implementation id: '" << implementation->getId() << "'";
                eout << " in waveform '" << _waveformContextName<<"'";
                eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                LOG_TRACE(ApplicationFactory_impl, eout.str())
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
            }

            // Add the required parameters specified in SR:163
            // Naming Context IOR, Name Binding, and component identifier
            CF::DataType ncior;
            ncior.id = "NAMING_CONTEXT_IOR";
            ncior.value <<= ossie::corba::objectToString(WaveformContext);
            component->addExecParameter(ncior);

            CF::DataType ci;
            ci.id = "COMPONENT_IDENTIFIER";
            ci.value <<= component->getIdentifier();
            component->addExecParameter(ci);

            CF::DataType nb;
            nb.id = "NAME_BINDING";
            nb.value <<= component->getNamingServiceName();
            component->addExecParameter(nb);

            CF::DataType dp;
            dp.id = "DOM_PATH";
	    dp.value <<= _baseNamingContext;
            component->addExecParameter(dp);

            CF::DataType pn;
            pn.id = "PROFILE_NAME";
            pn.value <<= component->getSpdFileName();
            component->addExecParameter(pn);
            
            // See if the LOGGING_CONFIG_URI has already been set
            // via <componentproperties> or initParams
            bool alreadyHasLoggingConfigURI = false;
            CF::Properties execParameters = component->getExecParameters();
            for (unsigned int i = 0; i < execParameters.length(); ++i) {
                std::string propid = static_cast<const char*>(execParameters[i].id);
                if (propid == "LOGGING_CONFIG_URI") {
                    alreadyHasLoggingConfigURI = true;
                    break;
                }
            }

            if (!alreadyHasLoggingConfigURI) {
                // Query the DomainManager for the logging configuration
                LOG_TRACE(ApplicationFactory_impl, "Checking DomainManager for LOGGING_CONFIG_URI");
                PropertyInterface* logProperty = _appFact._domainManager->getPropertyFromId("LOGGING_CONFIG_URI");
                if (!logProperty->isNil()) {
                    CF::DataType prop;
                    prop.id = logProperty->id.c_str();
                    logProperty->getValue(prop.value);
                    component->addExecParameter(prop);
                } else {
                    LOG_TRACE(ApplicationFactory_impl, "DomainManager LOGGING_CONFIG_URI is not set");
                }
            }

            // prepare LOGGING_CONFIG_URI execparam
            CF::DataType* lc = NULL;
            execParameters = component->getExecParameters();
            for (unsigned int i = 0; i < execParameters.length(); ++i) {
                std::string propid = static_cast<const char*>(execParameters[i].id);
                if (propid == "LOGGING_CONFIG_URI") {
                    lc = &execParameters[i];
                    break;
                }
            }

            if (lc != NULL) {
                const char* tmpstr;
                lc->value >>= tmpstr;
                LOG_TRACE(ApplicationFactory_impl, "Logging configuration provided " << tmpstr);
                string logging_uri = string(tmpstr);

                if (logging_uri.substr(0, 4) == "sca:") {
                    string fileSysIOR = ossie::corba::objectToString(_appFact._domainManager->_fileMgr);
                    logging_uri += ("?fs=" + fileSysIOR);
                    LOG_TRACE(ApplicationFactory_impl, "Adding file system IOR " << logging_uri);
                }
                lc->value <<= logging_uri.c_str();
                component->overrideProperty("LOGGING_CONFIG_URI", lc->value);
            } else {
                LOG_TRACE(ApplicationFactory_impl, "No logging configuration provided");
            }

            fs::path executeName;
            if ((implementation->getCodeType() == CF::LoadableDevice::EXECUTABLE) && (implementation->getEntryPoint().size() == 0)) {
                LOG_WARN(ApplicationFactory_impl, "executing using code file as entry point; this is non-SCA compliant behavior; entrypoint must be set")
                executeName = codeLocalFile;
            } else {
                executeName = fs::path(implementation->getEntryPoint());
                LOG_TRACE(ApplicationFactory_impl, "Using provided entry point " << executeName)
                if (!executeName.has_root_directory()) {
                    executeName = fs::path(component->spd.getSPDPath()) / executeName;
                }
                executeName = executeName.normalize();
            }

            attemptComponentExecution(executeName, execdev, component, implementation, pid, runningComponentTable);
        }
    }
}

void createHelper::attemptComponentExecution (
        const fs::path&                                            executeName,
        const CF::ExecutableDevice_var&                            execdev,
        ossie::ComponentInfo*&                                     component,
        const ossie::ImplementationInfo*&                          implementation,
        CF::Application::ComponentProcessIdSequence*&              pid,
        map<std::string, std::pair<std::string, unsigned long> >*& runningComponentTable) {

    CF::Properties execParameters;
    
    // get entrypoint
    CF::ExecutableDevice::ProcessID_Type tempPid = -1;

    // attempt to execute the component
    try {
        LOG_TRACE(ApplicationFactory_impl, "executing " << executeName << " on device " << ossie::corba::returnString(execdev->label()));
        execParameters = component->getExecParameters();
        for (unsigned int i = 0; i < execParameters.length(); ++i) {
            LOG_TRACE(ApplicationFactory_impl, " exec param " << execParameters[i].id << " " << ossie::any_to_string(execParameters[i].value))
        }
        // call 'execute' on the ExecutableDevice to execute the component
        tempPid = execdev->execute (executeName.string().c_str(), component->getOptions(), component->getExecParameters());
    } catch( CF::InvalidFileName& _ex ) {
        ostringstream eout;
        eout << "InvalidFileName when calling 'execute' on device with device id: '" << component->getAssignedDeviceId() << "' for component: '";
        eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getId() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " with error: <" << _ex.msg << ">;";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } catch( CF::Device::InvalidState& _ex ) {
        ostringstream eout;
        eout << "InvalidState when calling 'execute' on device with device id: '" << component->getAssignedDeviceId() << "' for component: '";
        eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getId() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " with error: <" << _ex.msg << ">;";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } catch( CF::ExecutableDevice::InvalidParameters& _ex ) {
        ostringstream eout;
        eout << "InvalidParameters when calling 'execute' on device with device id: '" << component->getAssignedDeviceId() << "' for component: '";
        eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getId() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " with invalid params: <";
        for (unsigned int propIdx = 0; propIdx < _ex.invalidParms.length(); propIdx++){
            eout << "(" << _ex.invalidParms[propIdx].id << "," << ossie::any_to_string(_ex.invalidParms[propIdx].value) << ")";
        }
        eout << " > error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } catch( CF::ExecutableDevice::InvalidOptions& _ex ) {
        ostringstream eout;
        eout << "InvalidOptions when calling 'execute' on device with device id: '" << component->getAssignedDeviceId() << "' for component: '";
        eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getId() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " with invalid options: <";
        for (unsigned int propIdx = 0; propIdx < _ex.invalidOpts.length(); propIdx++){
            eout << "(" << _ex.invalidOpts[propIdx].id << "," << ossie::any_to_string(_ex.invalidOpts[propIdx].value) << ")";
        }
        eout << " > error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } catch (CF::ExecutableDevice::ExecuteFail& ex) {
        ostringstream eout;
        eout << "ExecuteFail when calling 'execute' on device with device id: '" << component->getAssignedDeviceId() << "' for component: '";
        eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getId() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " with message: '" << ex.msg << "'";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } CATCH_THROW_LOG_ERROR(
            ApplicationFactory_impl, "Caught an unexpected error when calling 'execute' on device with device id: '"
            << component->getAssignedDeviceId() << "' for component: '" << component->getName()
            << "' with component id: '" << component->getIdentifier() << "' "
            << " with implementation id: '" << implementation->getId() << "'"
            << " in waveform '" << _waveformContextName<<"'"
            << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__,
            CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, "Caught an unexpected error when calling 'execute' on device"));

    // handle pid output
    if (tempPid < 0) {
        ostringstream eout;
        eout << "Failed to 'execute' component for component: '";
        eout << component->getName() << "' with component id: '" << component->getIdentifier() << "' ";
        eout << " with implementation id: '" << implementation->getId() << "'";
        eout << " in waveform '" << _waveformContextName<<"'";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_TRACE(ApplicationFactory_impl, eout.str())
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EAGAIN, eout.str().c_str());
    } else {
        pid->length (pid->length() + 1);
        (* pid)[pid->length() - 1].processId = tempPid;
        (* pid)[pid->length() - 1].componentId = component->getIdentifier();
        (*runningComponentTable)[component->getIdentifier()] = std::make_pair(component->getAssignedDeviceId(), tempPid);
    }
}

/** Initializes the components
 *  - Make sure internal lists are up to date
 *  - Ensure components have started and are bound to Naming Service
 *  - Initialize each component
 */
void createHelper::initializeComponents(CF::Resource_var& assemblyController,
                                        CosNaming::NamingContext_ptr WaveformContext)
{
    _implSeq.length (_requiredComponents.size ());

    // Install the different components in the system
    LOG_TRACE(ApplicationFactory_impl, "initializing " << _requiredComponents.size() << " waveform components")

    // Resize the _startSeq vector to the right size
    _startSeq.resize(_startOrderIds.size());

    for (unsigned int rc_idx = 0; rc_idx < _requiredComponents.size (); rc_idx++) {
        ossie::ComponentInfo* component = _requiredComponents[rc_idx];

        _implSeq[rc_idx].componentId = component->getIdentifier ();
        _implSeq[rc_idx].elementId =  component->getSelectedImplementation()->getId().c_str();
        
        // If the component is non-SCA compliant then we don't expect anything beyond this
        if (!component->isScaCompliant()) {
            LOG_TRACE(ApplicationFactory_impl, "Component is non SCA-compliant, continuing to next component");
            continue;
        }

        if (!component->isResource ()) {
            LOG_TRACE(ApplicationFactory_impl, "Component in not resource, continuing to next component");
            continue;
        }

        // Assuming 1 instantiation for each componentplacement
        if (component->getNamingService ()) {
            _namingCtxSeq.length (_namingCtxSeq.length() + 1);

            std::string _lookupName = _appFact._domainName + "/" + _waveformContextName + "/" + component->getNamingServiceName() ;

            // This is for the naming-service based configuration,
            // it assumes that the component already exists (like a device)
            _namingCtxSeq[_namingCtxSeq.length() - 1].componentId = component->getIdentifier();
            _namingCtxSeq[_namingCtxSeq.length() - 1].elementId = _lookupName.c_str();

            int componentBindingTimeout = _appFact._domainManager->getComponentBindingTimeout();

            LOG_TRACE(ApplicationFactory_impl, "Waiting " << componentBindingTimeout << "s for component to bind to " << _lookupName);
            // Wait for component to start
            CORBA::Object_var obj;
            CosNaming::Name_var cosName = ossie::corba::stringToName(component->getNamingServiceName());

            // Determine the current time, then add the timeout value to calculate when we should
            // stop retrying as an absolute time.
            time_t start = time(NULL);
            time_t now = start;
            const time_t end = now + componentBindingTimeout;
            struct timespec retry = {0, 50*1000000}; // Default retry is 50 ms (in nsec)
            while (now < end) {
                try {
                    obj = WaveformContext->resolve(cosName);
                    if (!CORBA::is_nil(obj)) {
                        break;
                    }
                } catch (CosNaming::NamingContext::NotFound) {
                    ///\Todo Check the name not found exceptions and make certain this is correct
                };
                // Sleep for the retry period.
                nanosleep(&retry, NULL);
                now = time(NULL);
            }

            if (CORBA::is_nil(obj)) {
                // For reference, determine much time has really elapsed.
                time_t elapsed = now-start;
                LOG_ERROR(ApplicationFactory_impl, "Timed out waiting for component to bind to naming context (" << elapsed << "s elapsed)");
                ostringstream eout;
                eout << "Timed out waiting for component to bind to naming context component: '" << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId()<<"'";
                eout << " in waveform '" << _waveformContextName<<"';";
                eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                _cleanupResourceNotFound();
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
            }

            // Check to see if the resource is the assembly controller
            // either way, the resource is initialized and configured
            CF::Resource_var _rsc;
            _rsc = ossie::corba::_narrowSafe<CF::Resource> (obj);
            if (CORBA::is_nil(_rsc)) {
                LOG_ERROR(ApplicationFactory_impl, "CF::Resource::_narrow failed");
                ostringstream eout;
                eout << "CF::Resource::_narrow failed with Unknown Exception for component: '" << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId()<<"'";
                eout << " in waveform '" << _waveformContextName<<"';";
                eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                _cleanupResourceNotFound();
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
            }

            component->setResourcePtr(_rsc);

            if (component->isResource ()) {
                try {
                    _rsc->initialize ();
                } catch( ... ) {
                    LOG_ERROR(ApplicationFactory_impl, "rsc->initialize failed with Unknown Exception");
                    ostringstream eout;
                    eout << "rsc->initialize failed with Unknown Exception for component: '" << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId()<<"'";
                    eout << " in waveform '" << _waveformContextName<<"';";
                    eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                    _cleanupResourceInitializeFailed();
                    throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
                }
            }

            // Set the assembly controller
            if (component->isAssemblyController()) {
                assemblyController = _rsc._retn();
            } else {
                // Get the CF::Resource_ptr
                CF::Resource_ptr val = _rsc;

                // Try and find the right location in the vector to add the reference
                unsigned int pos = 0;
                for (unsigned int i = 0; i < _startOrderIds.size(); i++) {
                    std::string currID = _startOrderIds[i];
                    currID = currID.append(":");
                    currID = currID.append(_waveformContextName);

                    if (strcmp(ossie::corba::returnString(val->identifier()).c_str(), currID.c_str()) == 0) {
                        break;
                    }
                    pos++;
                }

                // Add the reference if it belongs in the list
                if (pos < _startOrderIds.size()) {
                    _startSeq[pos] = val;
                }
            }
        }    
    }
}

/** Registers the components to the application object
 *  - Invokes the registerComponent function on the application object.
 *    If the naming service is deprecated, this function would be called
 *    by the component instead of registering itself with the name service
 */
void createHelper::addComponentsToApplication(Application_impl *application)
{
    // Install the different components in the system
    LOG_TRACE(ApplicationFactory_impl, "adding " << _requiredComponents.size() << " waveform components to the application")

    for (unsigned int rc_idx = 0; rc_idx < _requiredComponents.size (); rc_idx++) {
        ossie::ComponentInfo* componentInfo = _requiredComponents[rc_idx];

        CF::ComponentType component;

        component.identifier      = componentInfo->getIdentifier();
        component.softwareProfile = componentInfo->getSpdFileName();
        component.type            = CF::APPLICATION_COMPONENT;
        component.componentObject = componentInfo->getResourcePtr();

        application->registerComponent(component);
    }
}

void createHelper::configureComponents()
{
    for (unsigned int rc_idx = 0; rc_idx < _requiredComponents.size (); rc_idx++) {
        ossie::ComponentInfo* component = _requiredComponents[rc_idx];
        
        if (component->isAssemblyController ()) {
            continue;
        }
        
        // If the component is non-SCA compliant then we don't expect anything beyond this
        if (!component->isScaCompliant()) {
            LOG_TRACE(ApplicationFactory_impl, "Skipping configure; Component is non SCA-compliant, continuing to next component");
            continue;
        }

        if (!component->isResource ()) {
            LOG_TRACE(ApplicationFactory_impl, "Skipping configure; Component in not resource, continuing to next component");
            continue;
        }

        // Assuming 1 instantiation for each componentplacement
        if (component->getNamingService ()) {

            CF::Resource_var _rsc = component->getResourcePtr();

            if (CORBA::is_nil(_rsc)) {
                _cleanupResourceNotFound();
                LOG_ERROR(ApplicationFactory_impl, "Could not get component reference");
                ostringstream eout;
                eout << "Could not get component reference for component: '" 
                     << component->getName() << "' with component id: '" 
                     << component->getIdentifier() << " assigned to device: '"
                     << component->getAssignedDeviceId()<<"'";
                eout << " in waveform '" << _waveformContextName<<"';";
                eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
            }

            if (component->isResource () && component->isConfigurable ()) {
                try {
                    // try to configure the component
                    _rsc->configure (component->getNonNilConfigureProperties());
                } catch(CF::PropertySet::InvalidConfiguration& e) {
                    ostringstream eout;
                    eout << "Failed to 'configure' component: '";
                    eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
                    eout << " in waveform '"<< _waveformContextName<<"';";
                    eout <<  "InvalidConfiguration with this info: <";
                    eout << e.msg << "> for these invalid properties: ";
                    for (unsigned int propIdx = 0; propIdx < e.invalidProperties.length(); propIdx++){
                        eout << "(" << e.invalidProperties[propIdx].id << ",";
                        eout << ossie::any_to_string(e.invalidProperties[propIdx].value) << ")";
                    }
                    eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                    LOG_ERROR(ApplicationFactory_impl, eout.str());
                    _cleanupResourceConfigureFailed();
                    throw CF::ApplicationFactory::InvalidInitConfiguration(e.invalidProperties);
                } catch(CF::PropertySet::PartialConfiguration& e) {
                    ostringstream eout;
                    eout << "Failed to instantiate component: '";
                    eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
                    eout << " in waveform '"<< _waveformContextName<<"';";
                    eout << "Failed to 'configure' component; PartialConfiguration for these invalid properties: ";
                    for (unsigned int propIdx = 0; propIdx < e.invalidProperties.length(); propIdx++){
                        eout << "(" << e.invalidProperties[propIdx].id << ",";
                        eout << ossie::any_to_string(e.invalidProperties[propIdx].value) << ")";
                    }
                    eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                    LOG_ERROR(ApplicationFactory_impl, eout.str());
                    _cleanupResourceConfigureFailed();
                    throw CF::ApplicationFactory::InvalidInitConfiguration(e.invalidProperties);
                } catch( ... ) {
                    ostringstream eout;
                    eout << "Failed to instantiate component: '";
                    eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
                    eout << " in waveform '"<< _waveformContextName<<"';";
                    eout << "'configure' failed with Unknown Exception";
                    eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                    LOG_ERROR(ApplicationFactory_impl, eout.str());
                    _cleanupResourceConfigureFailed();
                    throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
                }
            }
        }
    }
    
    //  configure the assembly controller last
    for (unsigned int rc_idx = 0; rc_idx < _requiredComponents.size (); rc_idx++) {
        ossie::ComponentInfo* component = _requiredComponents[rc_idx];
        
        if (!component->isAssemblyController ()) {
            continue;
        }
        
        // If the component is non-SCA compliant then we don't expect anything beyond this
        if (!component->isScaCompliant()) {
            LOG_TRACE(ApplicationFactory_impl, "Skipping configure; Assembly controller is non SCA-compliant");
            break;
        }
        
        if (!component->isResource ()) {
            LOG_TRACE(ApplicationFactory_impl, "Skipping configure; Assembly controller is not resource");
            break;
        }
        
        // Assuming 1 instantiation for each componentplacement
        if (component->getNamingService ()) {
            
            CF::Resource_var _rsc = component->getResourcePtr();
            
            if (CORBA::is_nil(_rsc)) {
                _cleanupResourceNotFound();
                LOG_ERROR(ApplicationFactory_impl, "Could not get Assembly Controller reference");
                ostringstream eout;
                eout << "Could not get reference for Assembly Controller: '" 
                << component->getName() << "' with component id: '" 
                << component->getIdentifier() << " assigned to device: '"
                << component->getAssignedDeviceId()<<"'";
                eout << " in waveform '" << _waveformContextName<<"';";
                eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
            }
            
            if (component->isResource () && component->isConfigurable ()) {
                try {
                    // try to configure the component
                    _rsc->configure (component->getNonNilConfigureProperties());
                } catch(CF::PropertySet::InvalidConfiguration& e) {
                    ostringstream eout;
                    eout << "Failed to 'configure' Assembly Controller: '";
                    eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
                    eout << " in waveform '"<< _waveformContextName<<"';";
                    eout <<  "InvalidConfiguration with this info: <";
                    eout << e.msg << "> for these invalid properties: ";
                    for (unsigned int propIdx = 0; propIdx < e.invalidProperties.length(); propIdx++){
                        eout << "(" << e.invalidProperties[propIdx].id << ",";
                        eout << ossie::any_to_string(e.invalidProperties[propIdx].value) << ")";
                    }
                    eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                    LOG_ERROR(ApplicationFactory_impl, eout.str());
                    _cleanupResourceConfigureFailed();
                    throw CF::ApplicationFactory::InvalidInitConfiguration(e.invalidProperties);
                } catch(CF::PropertySet::PartialConfiguration& e) {
                    ostringstream eout;
                    eout << "Failed to instantiate Assembly Controller: '";
                    eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
                    eout << " in waveform '"<< _waveformContextName<<"';";
                    eout << "Failed to 'configure' Assembly Controller; PartialConfiguration for these invalid properties: ";
                    for (unsigned int propIdx = 0; propIdx < e.invalidProperties.length(); propIdx++){
                        eout << "(" << e.invalidProperties[propIdx].id << ",";
                        eout << ossie::any_to_string(e.invalidProperties[propIdx].value) << ")";
                    }
                    eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                    LOG_ERROR(ApplicationFactory_impl, eout.str());
                    _cleanupResourceConfigureFailed();
                    throw CF::ApplicationFactory::InvalidInitConfiguration(e.invalidProperties);
                } catch( ... ) {
                    ostringstream eout;
                    eout << "Failed to instantiate Assembly Controller: '";
                    eout << component->getName() << "' with component id: '" << component->getIdentifier() << " assigned to device: '"<<component->getAssignedDeviceId() << "' ";
                    eout << " in waveform '"<< _waveformContextName<<"';";
                    eout << "'configure' failed with Unknown Exception";
                    eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
                    LOG_ERROR(ApplicationFactory_impl, eout.str());
                    _cleanupResourceConfigureFailed();
                    throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
                }
            }
        }
        break;
    }
}

/** Connect the components
 *  - Connect the components
 */
void createHelper::connectComponents(std::vector<ConnectionNode>& connections, string base_naming_context)
{
    const std::vector<Connection>& _connection = _appFact._sadParser.getConnections ();

    // Create an AppConnectionManager to resolve and track all connections in the application.
    // NB: Use an auto_ptr instead of a bare pointer so that it will automatically be deleted
    //     in the event of a failure.
    using ossie::AppConnectionManager;
    std::auto_ptr<AppConnectionManager> connectionManager(new AppConnectionManager(_appFact._domainManager, this, this, base_naming_context));

    // Create all resource connections
    LOG_TRACE(ApplicationFactory_impl, "Establishing " << _connection.size() << " waveform connections")
    for (int c_idx = _connection.size () - 1; c_idx >= 0; c_idx--) {
        const Connection& connection = _connection[c_idx];

        LOG_TRACE(ApplicationFactory_impl, "Processing connection " << connection.getID());

        // Attempt to resolve the connection; if any connection fails, application creation fails.
        if (!connectionManager->resolveConnection(connection)) {
            _cleanupConnectionFailed();
            LOG_ERROR(ApplicationFactory_impl, "Unable to make connection " << connection.getID());
            ostringstream eout;
            eout << "Unable to make connection " << connection.getID();
            eout << " in waveform '"<< _waveformContextName<<"';";
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
        }
    }

    // Copy all established connections into the connection array
    const std::vector<ConnectionNode>& establishedConnections = connectionManager->getConnections();
    std::copy(establishedConnections.begin(), establishedConnections.end(), std::back_inserter(connections));
}

createHelper::createHelper (
        const ApplicationFactory_impl& appFact,
        string                         waveformContextName,
        string                         baseNamingContext,
        CosNaming::NamingContext_ptr   waveformContext) :
    _appFact(appFact),
    _allocationMgr(_appFact._domainManager->_allocationMgr),
    _allocations(*_allocationMgr)
{
    this->_waveformContextName = waveformContextName;
    this->_baseNamingContext   = baseNamingContext;
    this->_waveformContext     = CosNaming::NamingContext::_duplicate(waveformContext);
}

createHelper::~createHelper()
{
}

unsigned int createHelper::incrementEventChannelConnections(const std::string &EventChannelName) {
    return _appFact._domainManager->incrementEventChannelConnections(EventChannelName);
}

unsigned int createHelper::decrementEventChannelConnections(const std::string &EventChannelName) {
    return _appFact._domainManager->decrementEventChannelConnections(EventChannelName);
}

CosEventChannelAdmin::EventChannel_ptr createHelper::lookupEventChannel(const std::string &EventChannelName) {

    bool _existsEventChannel = _appFact._domainManager->eventChannelExists(EventChannelName);

    if (_existsEventChannel) {
        return _appFact._domainManager->getEventChannel(EventChannelName);
    } else {
        return _appFact._domainManager->createEventChannel (EventChannelName);
    }

    return CosEventChannelAdmin::EventChannel::_nil();
}

/** Given a component instantiation id, returns the associated CORBA Resource pointer
 *  - Gets the Resource pointer for a particular component instantiation id
 */
CF::Resource_ptr createHelper::lookupComponentByInstantiationId(const std::string& identifier)
{
    ossie::ComponentInfo* component = findComponentByInstantiationId(identifier);
    if (component) {
        return component->getResourcePtr();
    }

    return CF::Resource::_nil();
}

/** Given a component instantiation id, returns the associated CORBA Resource pointer
 *  - Gets the Resource pointer for a particular component instantiation id
 */
CF::DeviceManager_ptr createHelper::lookupDeviceManagerByInstantiationId(const std::string& identifier)
{
    CF::DomainManager::DeviceManagerSequence_var _deviceManagers;
    _deviceManagers = _appFact._domainManager->deviceManagers();
    try {   // this is here in case the length call fails
        for (size_t ii = 0; ii < _deviceManagers->length(); ++ii) {
            try {   // this is here in case the device manager ceased to exist while the loop is operating
                std::string deviceManagerId = ossie::corba::returnString(_deviceManagers[ii]->identifier());
                if (identifier == deviceManagerId) {
                    return CF::DeviceManager::_duplicate(_deviceManagers[ii]);
                }
            } catch ( ... ) {
                continue;
            }
        }
    } catch ( ... ) {
        return CF::DeviceManager::_nil();
    }

    return CF::DeviceManager::_nil();
}

/** Given a component instantiation id, returns the associated CORBA Device pointer
 *  - Gets the Device pointer for a particular component instantiation id
 */
CF::Device_ptr createHelper::lookupDeviceThatLoadedComponentInstantiationId(const std::string& componentId)
{
    LOG_TRACE(ApplicationFactory_impl, "[DeviceLookup] Lookup device that loaded component " << componentId);

    ossie::ComponentInfo* component = findComponentByInstantiationId(componentId);
    if (!component) {
        LOG_WARN(ApplicationFactory_impl, "[DeviceLookup] Component not found");
        return CF::Device::_nil();
    }

    const char* deviceId = component->getAssignedDeviceId();
    LOG_TRACE(ApplicationFactory_impl, "[DeviceLookup] Assigned device id " << deviceId);

    return find_device_from_id(deviceId);
}


/** Given a component instantiation id and uses id, returns the associated CORBA Device pointer
 *  - Gets the Device pointer for a particular component instantiation id and uses id
 */
CF::Device_ptr createHelper::lookupDeviceUsedByComponentInstantiationId(const std::string& componentId, const std::string& usesId)
{
    LOG_TRACE(ApplicationFactory_impl, "[DeviceLookup] Lookup device used by component " << componentId);
    ossie::ComponentInfo* component = findComponentByInstantiationId(componentId.c_str());
    if (!component) {
        LOG_WARN(ApplicationFactory_impl, "[DeviceLookup] Component not found");
        return CF::Device::_nil();
    }

    LOG_TRACE(ApplicationFactory_impl, "[DeviceLookup] Uses id " << usesId);
    const ossie::UsesDeviceInfo* usesdevice = component->getUsesDeviceById(usesId);
    if (!usesdevice) {
        LOG_WARN(ApplicationFactory_impl, "[DeviceLookup] UsesDevice not found");
        return CF::Device::_nil();
    }

    std::string deviceId = usesdevice->getAssignedDeviceId();
    LOG_TRACE(ApplicationFactory_impl, "[DeviceLookup] Assigned device id " << deviceId);

    return find_device_from_id(deviceId.c_str());
}

CF::Device_ptr createHelper::lookupDeviceUsedByApplication(const std::string& usesRefId)
{
    LOG_TRACE(ApplicationFactory_impl, "[DeviceLookup] Lookup device used by application, Uses Id: " << usesRefId);


    const ossie::UsesDeviceInfo* usesdevice = _appInfo.getUsesDeviceById(usesRefId);
    if (!usesdevice) {
        LOG_WARN(ApplicationFactory_impl, "[DeviceLookup] UsesDevice not found");
        return CF::Device::_nil();
    }

    std::string deviceId = usesdevice->getAssignedDeviceId();
    LOG_TRACE(ApplicationFactory_impl, "[DeviceLookup] Assigned device id " << deviceId);

    return find_device_from_id(deviceId.c_str());
}

