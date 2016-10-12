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

#include <ossie/FileStream.h>
#include <ossie/prop_helpers.h>

#include "Application_impl.h"
#include "ApplicationFactory_impl.h"
#include "DomainManager_impl.h"

namespace fs = boost::filesystem;
using namespace ossie;
using namespace std;

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

    _name = _sadParser.getName();
    _identifier = _sadParser.getID();
}

struct DeviceInfoToStr: public std::unary_function< ossie::DeviceNode, std::string > {
    result_type operator()( argument_type d ) const {
            return d.identifier;
    }
};


struct DeviceAssignmentMatchComponent:  public std::binary_function< ossie::DeviceAssignmentInfo, std::string, bool > {
    bool operator()( ossie::DeviceAssignmentInfo dev, std::string comp_id ) const {
        std::string cid;
        cid = dev.deviceAssignment.componentId;
        return (cid == comp_id);
    }
};

void createHelper::_cleanupApplicationCreateFailed()
{
    TRACE_ENTER(ApplicationFactory_impl);

    if (!alreadyCleaned)
    { _cleanupConnectionFailed(); }
}

void createHelper::_cleanupConnectionFailed()
{
    TRACE_ENTER(ApplicationFactory_impl);

    alreadyCleaned = true;
    _cleanupResourceInitializeFailed();
}

void createHelper::_cleanupResourceNotFound()
{
    TRACE_ENTER(ApplicationFactory_impl);

    alreadyCleaned = true;
    _cleanupLoadAndExecuteComponents();
}

void createHelper::_cleanupAssemblyControllerInitializeFailed()
{
    TRACE_ENTER(ApplicationFactory_impl);

    alreadyCleaned = true;
    _cleanupResourceNotFound();
}

void createHelper::_cleanupAssemblyControllerConfigureFailed()
{
    TRACE_ENTER(ApplicationFactory_impl);

    alreadyCleaned = true;
    _cleanupAssemblyControllerInitializeFailed();
}

void createHelper::_cleanupResourceInitializeFailed()
{
    TRACE_ENTER(ApplicationFactory_impl);

    alreadyCleaned = true;
    _cleanupAssemblyControllerConfigureFailed();
}

void createHelper::_cleanupResourceConfigureFailed()
{
    TRACE_ENTER(ApplicationFactory_impl);

    alreadyCleaned = true;
    _cleanupConnectionFailed();
}

void createHelper::_cleanupRequiredComponents()
{
    TRACE_ENTER(ApplicationFactory_impl);

    alreadyCleaned = true;
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

    alreadyCleaned = true;
    for (unsigned int devAssignIndex = 0; devAssignIndex < _appUsedDevs.size(); devAssignIndex++) {
        int component_found = 0;
        CORBA::Object_var _devObj = CORBA::Object::_nil ();
        std::string assignedDeviceId = ossie::corba::returnString(_appUsedDevs[devAssignIndex].deviceAssignment.assignedDeviceId);
        for (DeviceList::iterator device = _registeredDevices.begin(); device != _registeredDevices.end(); ++device) {
            if (device->identifier == assignedDeviceId) {
                _devObj = CF::Device::_duplicate(device->device);
            } else {
                continue;
            }

            if (component_found) {
                break;
            } else {
                component_found = 1;
            }
            std::string id(_appUsedDevs[devAssignIndex].deviceAssignment.componentId);
            try {
                LOG_TRACE(ApplicationFactory_impl, "Entering deallocation function");
                if (_appCapacityTable.count(id) > 0) {
                    for (unsigned int devCount = 0; devCount < _appCapacityTable[id].size(); devCount++) {
                        if (_appCapacityTable[id][devCount].properties.length() > 0) {
                            LOG_TRACE(ApplicationFactory_impl, "deallocating");
                            _appCapacityTable[id][devCount].device->deallocateCapacity(_appCapacityTable[id][devCount].properties);
                            LOG_TRACE(ApplicationFactory_impl, "Finished deallocating");
                        } else {
                            LOG_TRACE(ApplicationFactory_impl, "No capacity to deallocate");
                        }
                    }
                }
            } catch ( ... ) {
                continue;
            }
        }

        LOG_TRACE(ApplicationFactory_impl, "Next component")
    }

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

    alreadyCleaned = true;
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

    alreadyCleaned = true;
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


    // clean up soft package dependencies that were loaded...
    ossie::SoftPkgList::iterator pkg = _softpkgList.begin();
        for ( ;  pkg != _softpkgList.end(); pkg++ ) {
          try {
            if ( ossie::corba::objectExists(pkg->first) ) {
              CF::LoadableDevice_ptr loadDev = CF::LoadableDevice::_narrow(pkg->first);
              if ( CORBA::is_nil(loadDev) == false ) {
                LOG_DEBUG(ApplicationFactory_impl, "Unload soft package dependency:" << pkg->second);
                loadDev->unload(pkg->second.c_str());
              }
              else {
                throw -1;
              }
            }
            else {
              throw -1;
            }
          }
          catch(...) {
            // issue warning the unload failed for soft pkg unload
            LOG_WARN(ApplicationFactory_impl, "Unable to unload soft package dependency:" << pkg->second);
          }
          

        }

    _cleanupAllocateDevices();
}

ApplicationFactory_impl::~ApplicationFactory_impl ()
{
}


/** Creates and instance of the application.
 *  - Assigns components to devices
 *      - First based on user-provided DAS if one is passed in (deviceAssignments)
 *      - Then based on property matching and allocation matching
 *  - Attempts to honor host collocation
 *  @param name user-friendly name of the application to be instantiated
 *  @param initConfiguration properties that can override those from the SAD
 *  @param deviceAssignments optional user-provided component-to-device assignments
 */
CF::Application_ptr ApplicationFactory_impl::create (const char* name,
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
    boost::mutex::scoped_lock lock(_pendingCreateLock);

    ///////////////////////////////////////////////////
    // Establish new naming context for waveform
    LOG_TRACE(ApplicationFactory_impl, "Establishing waveform naming context");
    try {
        // VERY IMPORTANT: we must first lock the operations in this try block
        //    in order to prevent a naming context collision due to multiple create calls
        //RESOLVE boost::mutex::scoped_lock lock(_pendingCreateLock);

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

    // now use the createHelper class to actually run 'create'
    // - createHelper is needed to allow concurrent calls to 'create' without
    //   each instance stomping on the others
    LOG_TRACE(ApplicationFactory_impl, "Creating new createHelper class.");
    createHelper new_createhelper(*this, _waveform_context_name, base_naming_context, _waveformContext);

    // now actually perform the create operation
    LOG_TRACE(ApplicationFactory_impl, "Performing 'create' function.");
    CF::Application_ptr new_app = new_createhelper.create(name, initConfiguration, deviceAssignments);

    // return the new Application
    TRACE_EXIT(ApplicationFactory_impl);
    return new_app;
}


CF::Application_ptr createHelper::create (const char* name,
                                          const CF::Properties& initConfiguration,
                                          const CF::DeviceAssignmentSequence& deviceAssignments)
throw (CORBA::SystemException, CF::ApplicationFactory::CreateApplicationError,
       CF::ApplicationFactory::CreateApplicationRequestError,
       CF::ApplicationFactory::InvalidInitConfiguration)
{
    TRACE_ENTER(ApplicationFactory_impl);

    try {
        try {
            alreadyCleaned = false;

            _pidSeq.length (0);
            _softpkgList.clear();
            _fileTable.clear();
            _loadedComponentTable.clear();  // mapping of component id to filenames/device id tuple
            _runningComponentTable.clear();  // mapping of component id to filenames/device id tuple
            _namingCtxSeq.length (0);
            _implSeq.length (0);


            ///////////////////////////////////////////////////
            // Get a list of all device currently in the domain
            _registeredDevices = _appFact._domainManager->getRegisteredDevices();
            _executableDevices.clear();

            std::string lastExecutableDevice = _appFact._domainManager->getLastDeviceUsedForDeployment();

            // Get the executable devices from the registered devices list
            for (DeviceList::const_iterator devNode = _registeredDevices.begin(); devNode != _registeredDevices.end(); ++devNode) {
                try {
                    if (devNode->device->_is_a("IDL:CF/ExecutableDevice:1.0")) {
                        _executableDevices.push_back(*devNode);
                    }
                } catch (...) {
                    // Ignore devices that throw exceptions, which implies that
                    // they are unreachable anyway
                }
            }

            if (_executableDevices.size() == 0) {
                ostringstream eout;
                eout << "The domain has no executable devices (and therefore cannot support the creation of waveform " << name << ")";
                LOG_WARN(ApplicationFactory_impl, eout.str());
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
            }

            // Rotate the executable device list to put the last successful
            // device first
            LOG_DEBUG(ApplicationFactory_impl, "Rotating executable device list to put \"" << lastExecutableDevice << "\" first");
            rotateDeviceList(_executableDevices, lastExecutableDevice);
            
            // create list of device IDs for allocation collocations
            DeviceIDList  _regDeviceIDs;
            std::transform(_executableDevices.begin(), _executableDevices.end(), std::back_inserter(_regDeviceIDs), DeviceInfoToStr());

            //////////////////////////////////////////////////
            // Load the components to instantiate from the SAD
            if (_requiredComponents.size() != 0)
            { _deleteRequiredComponents(); }

            getRequiredComponents();

            ossie::ComponentInfo* assemblyControllerComponent = NULL;
            overrideAssemblyControllerProperties(initConfiguration,
                                                 assemblyControllerComponent);

            ////////////////////////////////////////////////
            // Assign components to devices
            ////////////////////////////////////////////////

            //
            // _appUsedDevs and appCapacityTable represent all the allocations and assigned made during applicaiton deployment
            // it provides the "context" for the deployment.  This context pattern will be applied again when collocation
            // requests are fullfilled.  There 2 container are used to deploy the waveform, and also to "cleanup" if deployment
            // fails
            //

            // reset list of devices that were used during component allocation/placement process for an application
            _appUsedDevs.resize(0);

            // Start with a empty set of allocation properties, used to keep track of device capacity
            // allocations. If this is not cleared each time, deallocation may start occuring multiple
            // times, resulting in incorrect capacities.
            _appCapacityTable.clear();

            //
            // First, assign components to devices based on the caller supplied DAS.
            //
            LOG_TRACE(ApplicationFactory_impl, "Assigning " << deviceAssignments.length() << " components based on DeviceAssignmentSequence");
            for (unsigned int ii = 0; ii < deviceAssignments.length(); ++ii) {
                std::string componentId(deviceAssignments[ii].componentId);
                LOG_TRACE(ApplicationFactory_impl, "DAS - component " << componentId);
                ossie::ComponentInfo* component = findComponentByInstantiationId(componentId);
                if (!component) {
                    LOG_ERROR(ApplicationFactory_impl, "Failed to create application; "
                              << "unknown component " << componentId << " in user assignment (DAS)");
                    CF::DeviceAssignmentSequence badDAS;
                    badDAS.length(1);
                    badDAS[0].componentId = CORBA::string_dup(deviceAssignments[ii].componentId);
                    badDAS[0].assignedDeviceId = CORBA::string_dup(deviceAssignments[ii].assignedDeviceId);
                    throw CF::ApplicationFactory::CreateApplicationRequestError(badDAS);
                }
                allocateComponent(component, deviceAssignments, _appCapacityTable, _appUsedDevs);
            }

            //
            // Second, attempt to honor host collocation., the host collocation in itself is just a "waveform" inside of
            // a waveform, so we need create a context for each collocation and if that is successfull add it to
            // the applications alloction context
            //
            const std::vector<SoftwareAssembly::HostCollocation>& hostCollocations = _appFact._sadParser.getHostCollocations();
            LOG_TRACE(ApplicationFactory_impl, "Assigning " << hostCollocations.size() << " collocated groups of components");
            for (unsigned int ii = 0; ii < hostCollocations.size(); ++ii) {
                SoftwareAssembly::HostCollocation collocation = hostCollocations[ii];
                LOG_TRACE(ApplicationFactory_impl, "-- Begin placment for Collocation " << collocation.getName() << " " << collocation.getID());

                // Some components may have been placed by a user DAS; keep a list of those that still need to be
                // assigned to a device.
                PlacementList placingComponents;

                // Keep track of devices to which some of the components have been assigned.
                DeviceIDList assignedDevices;

                const std::vector<ComponentPlacement>& collocatedComponents = collocation.getComponents();
                std::vector<ComponentPlacement>::const_iterator placement = collocatedComponents.begin();
                for (; placement != collocatedComponents.end(); ++placement) {
                    ComponentInstantiation instantiation = (placement->getInstantiations()).at(0);
                    ossie::ComponentInfo* component = findComponentByInstantiationId(instantiation.getID());
                    if (!component) {
                        ostringstream eout;
                        eout << "failed to create application; unable to recover component Id (error parsing the SAD file "<<_appFact._softwareProfile<<")";
                        LOG_ERROR(ApplicationFactory_impl, eout.str());
                        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EAGAIN, eout.str().c_str());
                    }
                    LOG_TRACE(ApplicationFactory_impl, "Collocated component " << component->getInstantiationIdentifier());
                    if (component->isAssignedToDevice()) {
                        // This component is already assigned to a device; for collocating other components, the
                        // pre-assigned devices are used in the order they are encountered.
                        LOG_TRACE(ApplicationFactory_impl, "Already assigned to device " << component->getAssignedDeviceId());
                        assignedDevices.push_back( component->getAssignedDeviceId() );
                    } else {
                        // This component needs to be assigned to a device.
                        placingComponents.push_back(component);
                    }
                }

                // Warn if the DAS split up the collocated components, but attempt to honor the collocation.
                if (assignedDevices.size() > 1) {
                    LOG_WARN(ApplicationFactory_impl, "Collocated components assigned to different devices in DAS");
                }

#ifdef DEBUG
                LOG_TRACE(ApplicationFactory_impl, "PLACING: " << placingComponents.size() << " components");
                for ( uint jj = 0; jj < placingComponents.size(); ++jj) {
                    ossie::ComponentInfo* component = placingComponents[jj];
                    std::string c_id = component->getInstantiationIdentifier();
                    LOG_TRACE(ApplicationFactory_impl, "       COLLCATION ID:" << collocation.id << " COMP INST_ID:" << c_id);
                }
#endif

                // Create list of devices that are available to use for placement, executableDevices minus assignedDevices
                DeviceIDList  _availableDevices;
                for (DeviceIDList::const_iterator devIter = _regDeviceIDs.begin(); devIter != _regDeviceIDs.end(); ++devIter) {
                    if (!std::count(assignedDevices.begin(), assignedDevices.end(), *devIter)) {
                        _availableDevices.push_back(*devIter);
                    }
                }

#ifdef DEBUG
                LOG_TRACE(ApplicationFactory_impl, "AVAILABLE DEVICES:");
                DeviceIDList::iterator _did = _availableDevices.begin();
                for ( ; _did != _availableDevices.end(); _did++) {
                    LOG_TRACE(ApplicationFactory_impl, "  DEVICE ID: " << *_did);
                }
#endif

                // Iterate through each of the available devices
                DeviceIDList::iterator devId = _availableDevices.begin();
                bool placement_complete = false;
                for( ; devId != _availableDevices.end() && !placement_complete; devId++ ){

                    //
                    // create list of device ids to be used when generating a DAS when placing this component
                    //  the list of devices are any caller supplied DAS and the current available device we are looking at
                    DeviceIDList  devList;
                    devList = assignedDevices;
                    devList.push_back( *devId );
                    LOG_TRACE(ApplicationFactory_impl, "ALLOCATING ON DEVICE: " << *devId);

                    //
                    // use collocCapacities and collocAssignedDevs to track the collocation context...
                    //
                    std::string               c_id;
                    CapacityAllocationTable   collocCapacities;
                    DeviceAssignmentList      collocAssignedDevs;
                    try {

                        // iterate through eahc component of teh collocation request and try to assign to a device
                        for (unsigned int jj = 0; jj < placingComponents.size(); ++jj) {

                            ossie::ComponentInfo* component = placingComponents[jj];
                            c_id = component->getInstantiationIdentifier();

                            LOG_TRACE(ApplicationFactory_impl, "  COLLOCATION ID:" << collocation.id << " PLACING COMP INST_ID:" << c_id);
                            CF::DeviceAssignmentSequence componentDAS;
                            componentDAS.length(1);
                            componentDAS[0].componentId = CORBA::string_dup(c_id.c_str());
                            DeviceIDList::iterator dev_id = devList.begin();
                            bool c_placed = false;
                            for ( ; dev_id  != devList.end() && !c_placed ; dev_id++ ) {
                                componentDAS[0].assignedDeviceId = CORBA::string_dup( dev_id->c_str() );
                                LOG_TRACE(ApplicationFactory_impl, "  TRYING TO ALLOCATE ON DEVICE: " << *dev_id);
                                try {
                                    // try and place the component... if it does not work we clean up after we try all the different available devices
                                  allocateComponent(component, componentDAS, collocCapacities, collocAssignedDevs, false );
                                    c_placed = true;
                                    LOG_TRACE(ApplicationFactory_impl, "    **ALLOCATION SUCCESS**  COMP_INST_ID: " << c_id << " DEVICE: " << *dev_id);
                                } catch (...) {
                                    // Individual allocation failures are non-fatal; check for failure after all possible
                                    // devices have been exhausted.
                                    LOG_TRACE(ApplicationFactory_impl, "    **ALLOCATION FAILURE**  COMP_INST_ID: " << c_id << " DEVICE: " << *dev_id);
                                }
                            }

                            if ( !c_placed ){
                                throw 0;
                            }

                        }

                        // we placed everything.... so set loop termination and add the collocation's allocations and
                        // assigned devices to the application's context
                        LOG_TRACE(ApplicationFactory_impl, " COLLOCATION **PASSED** Collocation ID:" << collocation.id);
                        LOG_TRACE(ApplicationFactory_impl, "    PRE TABLE:" <<  _appCapacityTable.size() << " DEV:" << _appUsedDevs.size() );
                        placement_complete = true;
                        _appCapacityTable.insert(collocCapacities.begin(), collocCapacities.end());
                        _appUsedDevs.insert( _appUsedDevs.end(), collocAssignedDevs.begin(), collocAssignedDevs.end());
                        LOG_TRACE(ApplicationFactory_impl, "    POST TABLE:" <<  _appCapacityTable.size() << " DEV:" << _appUsedDevs.size() );
                    }
                    catch(...) {
                        LOG_TRACE(ApplicationFactory_impl, "COLLOCATION FAILED: BEGIN CLEANUP: Collocation ID:" << collocation.id << " TRYING TO PLACE COMP INST_ID:" << c_id);
                        // need to clean up allocation for the collocation request
                        _cleanupCollocation( collocCapacities, collocAssignedDevs );
                    }

                }

                if ( placement_complete == false ) {
                    std::ostringstream eout;
                    eout << "Could not collocate components for collocation NAME: " << collocation.getName() << "  ID:" << collocation.id;
                    LOG_ERROR(ApplicationFactory_impl, eout.str());
                    throw CF::ApplicationFactory::CreateApplicationRequestError();
                }

                LOG_TRACE(ApplicationFactory_impl, "-- Completed placement for Collocation ID:" << collocation.id << " Components Placed: " << collocatedComponents.size());
            }

            //
            // Finally, assign any remaining components to devices.
            //
            for (unsigned int rc_idx = 0; rc_idx < _requiredComponents.size (); rc_idx++) {
                ossie::ComponentInfo* component = _requiredComponents[rc_idx];

                if (!component->isAssignedToDevice()) {
                  allocateComponent(component, deviceAssignments, _appCapacityTable, _appUsedDevs);
                }
            }

            ////////////////////////////////////////////////
            // Load and Execute the components
            try {
                try {
                    loadAndExecuteComponents(&_pidSeq, &_fileTable, _waveformContext, &_loadedComponentTable, &_runningComponentTable);
                } catch (...) {
                    _cleanupLoadAndExecuteComponents(); // clean up and rethrow
                    throw;
                }
            } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
                throw;
            } CATCH_THROW_LOG_TRACE(ApplicationFactory_impl,
                "Load-and-execute of component failed (unclear which component/device is the problem)",
                CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, "Load-and-execute of component failed (unclear which component/device is the problem)"));


            CF::Resource_var assemblyController = CF::Resource::_nil();

            ////////////////////////////////////////////////
            // Initialize the components
            try{
                initializeComponents(assemblyController, _waveformContext);
            } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
                throw;
            } CATCH_THROW_LOG_TRACE(ApplicationFactory_impl,
                "Initialize of component failed (unclear which component/device is the problem)",
                CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL,"Initialize of component failed (unclear which component/device is the problem)"));

            ////////////////////////////////////////////////
            // Connect the components
            std::vector<ConnectionNode> connections;
            try{
                connectComponents(connections, _baseNamingContext);
            } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
                throw;
            } CATCH_THROW_LOG_TRACE(ApplicationFactory_impl,
                "Connecting components failed (unclear where this occurred)",
                CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, "Connecting components failed (unclear where this occurred)"));

            ////////////////////////////////////////////////
            // Configure the components
            try{
                configureComponents();
            } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
                throw;
            } CATCH_THROW_LOG_TRACE(ApplicationFactory_impl, 
                "Configure on component failed (unclear where in the process this occurred)",
                CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, "Configure of component failed (unclear where in the process this occurred)"))
                
            // Check to make sure assemblyController was initialized if it was SCA compliant
            if (CORBA::is_nil(assemblyController)) {
                if ((assemblyControllerComponent==NULL) || (assemblyControllerComponent->isScaCompliant())) {
                LOG_DEBUG(ApplicationFactory_impl, "assembly controller is not Sca Compliant or has not been assigned");
                throw (CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, "assembly controller is not Sca Compliant or has not been assigned"));
                }
            }

            ////////////////////////////////////////////////
            // Create the Application servant

            // Give the application a unique identifier of the form "softwareassemblyid:ApplicationName",
            // where the application name includes the serial number generated for the naming context
            // (e.g. "Application_1").
            std::string appIdentifier = _appFact._identifier + ":" + _waveformContextName;

            // Manage the Application servant with an auto_ptr in case something throws an exception.
            std::auto_ptr<Application_impl> application(new Application_impl (appIdentifier.c_str(), 
                                                                               name, 
                                                                               _appFact._softwareProfile.c_str(), 
                                                                               _appFact._domainManager, 
                                                                               _waveformContextName, 
                                                                               _waveformContext));

            ////////////////////////////////////////////////
            // Set up the external ports
            const std::vector<SoftwareAssembly::Port>& ports = _appFact._sadParser.getExternalPorts();
            LOG_TRACE(ApplicationFactory_impl, "Mapping " << ports.size() << " external port(s)");
            for (std::vector<SoftwareAssembly::Port>::const_iterator port = ports.begin(); port != ports.end(); ++port) {
                LOG_TRACE(ApplicationFactory_impl, "Port component: " << port->componentrefid << " Port identifier: " << port->identifier);

                // Get the component from the instantiation identifier.
                CORBA::Object_var obj = lookupComponentByInstantiationId(port->componentrefid);
                if (CORBA::is_nil(obj)) {
                    LOG_ERROR(ApplicationFactory_impl, "Invalid componentinstantiationref ("<<port->componentrefid<<") given for an external port ");
                    throw(CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, "Invalid componentinstantiationref given for external port"));
                }

                if (port->type == SoftwareAssembly::Port::SUPPORTEDIDENTIFIER) {
                    if (!obj->_is_a(port->identifier.c_str())) {
                        LOG_ERROR(ApplicationFactory_impl, "Component does not support requested interface: " << port->identifier);
                        throw(CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, "Component does not support requested interface"));
                    }
                } else {
                    // Must be either "usesidentifier" or "providesidentifier", which are equivalent unless
                    // you want to be extra pedantic and check how the port is described in the component's SCD.

                    CF::PortSupplier_var portSupplier = ossie::corba::_narrowSafe<CF::PortSupplier> (obj);

                    // Try to look up the port.
                    try {
                        obj = portSupplier->getPort(port->identifier.c_str());
                    } CATCH_THROW_LOG_ERROR(ApplicationFactory_impl, "Invalid port id", CF::ApplicationFactory::CreateApplicationError(CF::CF_NOTSET, "Invalid port identifier"))
                }

                // Add it to the list of external ports on the application object.
                application->addExternalPort(port->identifier, obj);
            }


            ////////////////////////////////////////////////
            // Create the application
            //
            // We are assuming that all components and their resources are collocated.
            // This means that we assume the SAD <partitioning> element contains the
            // <hostcollocation> element.
            // NB: Ownership of the ConnectionManager is passed to the application.
            application->populateApplication(
                assemblyController,
                _appUsedDevs, 
                &_implSeq, 
                _startSeq, 
                &_namingCtxSeq, 
                &_pidSeq,
                connections, 
                _fileTable, 
                _softpkgList,
                _appCapacityTable);

            // Activate the new Application servant, and let the POA manage its deletion. The
            // DomainManager POA must exist, but the Applications POA might not have been created yet.
            PortableServer::POA_var dm_poa = ossie::corba::RootPOA()->find_POA("DomainManager", 0);
            PortableServer::POA_var poa = dm_poa->find_POA("Applications", 1);
            PortableServer::ObjectId_var oid = ossie::corba::activatePersistentObject(poa, application.get(), appIdentifier);

            // The POA now has ownership of the Application.
            Application_impl* app_servant = application.release();
            app_servant->_remove_ref();

            // Add a reference to the new application to the ApplicationSequence in DomainManager
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
            _appFact._domainManager->setLastDeviceUsedForDeployment(_executableDevices.front().identifier);

            LOG_TRACE(ApplicationFactory_impl, "Cleaning up");
            _deleteRequiredComponents();

#if ENABLE_EVENTS
            ossie::sendObjectAddedEvent(ApplicationFactory_impl::__logger, _appFact._identifier.c_str(), appIdentifier.c_str(), name,
                                        appObj, StandardEvent::APPLICATION, _appFact._domainManager->proxy_consumer);
#endif

            LOG_INFO(ApplicationFactory_impl, "Done creating application " << appIdentifier << " " << name);
            return appObj._retn();
        } catch (...) {
            _cleanupApplicationCreateFailed(); // Cleanup and rethrow
            throw;
        }
    } catch (CF::ApplicationFactory::CreateApplicationError& ex) {
        LOG_ERROR(ApplicationFactory_impl, "Error in application creation; " << ex.msg);
        throw;
    } catch (CF::ApplicationFactory::CreateApplicationRequestError) {
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


/** Allocate capacity on devices from the 'usesdevice' dependencies
 *  - Find a device that meets the property dependencies for the 'usesdevice'
 *  - Try to allocate capacity on that device
 */
CF::Device_ptr createHelper::allocateUsesDeviceProperties(ossie::ComponentImplementationInfo* component_impl,
                                                          unsigned int usesDevIdx, CF::Properties & allocatedProps,
                                                          const CF::Properties& configureProperties)
{
    CF::Device_ptr device = CF::Device::_nil();
    CF::Properties allocProps;

    // iterate through devices and try to find suitable matches for the 'usesdevice' dependencies
    for (DeviceList::iterator node = _registeredDevices.begin(); node != _registeredDevices.end(); ++node) {
        if (!ossie::corba::objectExists(node->device)) {
            LOG_WARN(ApplicationFactory_impl, "Not using device for uses_device allocation " << node->identifier
                     << " because it no longer exists");
            continue;
        }

        std::string identifier = node->identifier;
        std::string label = node->label;

        // Get the Device Manager
        CF::DeviceManager_ptr devMgr = node->devMgr.deviceManager;
        if (!ossie::corba::objectExists(devMgr)) {
            LOG_ERROR(ApplicationFactory_impl, "Could not locate device manager for device "<< identifier)
            ostringstream eout;
            eout << "Could not locate device manager for device " << identifier;
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
        }

        // Get the device manager's file system
        CF::FileSystem_var fileSystem = devMgr->fileSys();
        if (CORBA::is_nil(fileSystem)) {
            LOG_ERROR(ApplicationFactory_impl, "Could not locate device manager filesystem ")
            ostringstream eout;
            eout << "Could not locate device manager filesystem in Device Manager " << ossie::corba::returnString(devMgr->label());
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
        }

        LOG_TRACE(ApplicationFactory_impl, "Attempting to allocate on (allocateUsesDeviceProperties) device " << identifier
                << " with allocProps of length " << allocProps.length())

        SoftPkg spd;
        try {
            CORBA::String_var profile = node->device->softwareProfile();
            File_stream _spd(fileSystem, profile);
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

        CF::Properties allocProps;
        if ( spd.getPRFFile() ) {

            LOG_TRACE(ApplicationFactory_impl, "Opening device property file" << spd.getPRFFile())
            Properties prf;
            try {
                File_stream _prf(fileSystem, spd.getPRFFile());
                prf.load(_prf);
                _prf.close();
            } catch (ossie::parser_error& e) {
                ostringstream eout;
                eout << "creating application error; error parsing device prf; " << e.what();
                LOG_ERROR(ApplicationFactory_impl, eout.str());
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
            } catch( ... ) {
                ostringstream eout;
                eout << "creating application error; ; unknown error parsing  device prf;";
                LOG_ERROR(ApplicationFactory_impl, eout.str());
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
            }

            // check to see if this device has the necessary properties
            if (!component_impl->checkUsesDevices(prf, allocProps, usesDevIdx, configureProperties)) {
                LOG_INFO(ApplicationFactory_impl, "Device " << identifier << " " << label << " lacks required properties.")
                continue;
            }

            LOG_TRACE(ApplicationFactory_impl, "Allocating Properties of length " << allocProps.length())
            try {
                if (allocProps.length() > 0) {
                    if (!node->device->allocateCapacity (allocProps)) {
                        LOG_TRACE(ApplicationFactory_impl, "Device " << identifier << " " << label << " lacks sufficient capacity")
                        continue;
                    }

                    // return allocated CF::Property objects to the caller
                    allocatedProps = allocProps;

                    LOG_TRACE(ApplicationFactory_impl, "The outgoing allocProps length is " << allocProps.length());
                } else {
                    LOG_TRACE(ApplicationFactory_impl, "component 'usesdevice' requires no capacity from device " << label)
                }
                device = node->device;
                break;
            } catch (CF::Device::InvalidCapacity e) {
                LOG_TRACE(ApplicationFactory_impl, "Device " << identifier << " " << label << " lacks sufficient capacity to satisfy 'usesdevice'")
                continue;
            } catch (CF::Device::InsufficientCapacity e) {
                LOG_TRACE(ApplicationFactory_impl, "Device " << identifier << " " << label << " lacks sufficient capacity to satisfy 'usesdevice'")
                continue;
            }
        }
    }
    LOG_TRACE(ApplicationFactory_impl, "Exiting device search loop " << allocProps.length())

    return device;
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
                                     const CF::DeviceAssignmentSequence& deviceAssignments,
                                     CapacityAllocationTable  &appCapacities,
                                     DeviceAssignmentList     &appAssignedDevs,
                                     bool cleanup)
{
    // get the implementations from the component
    ossie::ImplementationInfo::List  implementations;
    component->getImplementations(implementations);

    CapacityAllocationList usesDevicesCapacities(0);

    // Find the devices that allocate the SPD's minimum required usesdevices properties
    const UsesDeviceInfo::List &usesDevVec = component->getUsesDevices();
    LOG_TRACE(ApplicationFactory_impl, "Looking for Uses Allocation " << usesDevVec.size());
    for (unsigned int uC = 0; uC < usesDevVec.size(); uC++) {
        CF::Properties allocUsesProps;
        CF::Device_ptr usesDevice = allocateUsesDeviceProperties(component, uC, allocUsesProps, component->getConfigureProperties());
        if (CORBA::is_nil(usesDevice)) {
            ostringstream eout;
            eout << "Failed to satisfy 'usesdevice' dependency of component: '";
            eout << component->getName() << "' with component id: '" << component->getIdentifier() << "'; ";
            eout << "Failed to satisfy 'usesdevice' dependency with usesdevice.id: '";
            eout << component->getUsesDevices()[uC]->getId() << "'";
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            LOG_DEBUG(ApplicationFactory_impl, "Calling cleanupAllocateDevices (1)");
            if ( cleanup ) _cleanupAllocateDevices();
                throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
        }
        LOG_DEBUG(ApplicationFactory_impl, "Allocated " << allocUsesProps.length());
        usesDevicesCapacities.push_back( CapacityAllocation(usesDevice, allocUsesProps));

        usesDevVec[uC]->setAssignedDeviceId(ossie::corba::returnString(usesDevice->identifier()));
    }

    //
    // add any usesDevices' allocations  to current application context's capcacity table
    // if we fail to assigned that way it will get clean up properly
    //
    try {
       for ( uint i = 0; i < usesDevicesCapacities.size(); i++) {
           appCapacities[component->getIdentifier()].push_back(usesDevicesCapacities[i]);
       }
    }
    catch(...){
    }

    // now attempt to find an implementation that can have it's allocation requirements met
    unsigned int implCount = 0;
    bool implUsesDeviceDepsMet = false;
    for (implCount = 0; implCount < implementations.size(); implCount++) {
        ossie::ImplementationInfo* impl = implementations[implCount];

        // deal with 'usesdevice' dependencies for the particular implementation
        CapacityAllocationList   implCapacityList;
        const UsesDeviceInfo::List &implUsesDevVec = impl->getUsesDevices();
        bool usesDeviceSetComplete = true;
        CF::Device_ptr usesDevice = CF::Device::_nil();
        LOG_DEBUG(ApplicationFactory_impl, "UsesDevice Impl Vec of length " << implUsesDevVec.size());

        for ( uint uC = 0; uC < implUsesDevVec.size(); uC++) {
            CF::Properties allocUsesProps;
            usesDevice = allocateUsesDeviceProperties(impl, uC, allocUsesProps, component->getConfigureProperties());
            if (CORBA::is_nil(usesDevice)) {
                // undo allocation,  we do not need to clean up totally at this point
                _undoCapacityAllocations(implCapacityList);
                usesDeviceSetComplete = false;
                break;
            }
            implCapacityList.push_back( CapacityAllocation(usesDevice, allocUsesProps));

            implUsesDevVec[uC]->setAssignedDeviceId(ossie::corba::returnString(usesDevice->identifier()));
        }

        CapacityAllocation  devCapacityAlloc;
        bool foundSoftpkgDependencies = false;
        try  {

            if (!usesDeviceSetComplete) throw -1;  // if this implementation's 'usesdevice' dependencies can't be met, try the next one

            // Found an implementation which has it's 'usesdevice' dependencies satisfied
            // Now perform assignment/allocation of component to device
            implUsesDeviceDepsMet = true;

            LOG_DEBUG(ApplicationFactory_impl, "Trying to find the device");
            ossie::Properties devicePRF;
            allocateComponentToDevice(component, impl, deviceAssignments, devCapacityAlloc, devicePRF);

            if (CORBA::is_nil( devCapacityAlloc.device ))  {
               LOG_DEBUG(ApplicationFactory_impl, "Device Allocation Failed.. need to clean up");
               throw -1;
            }

            foundSoftpkgDependencies = resolveSoftpkgDependencies(impl, devCapacityAlloc.device, devicePRF);

            if (!foundSoftpkgDependencies) {
                LOG_DEBUG(ApplicationFactory_impl, "Softpackage dependency failed.need to clean up");
                throw -1;
            }

            // allocation to a device succeeded
            CORBA::String_var deviceId =  devCapacityAlloc.device->identifier();
            LOG_DEBUG(ApplicationFactory_impl, "Assigned component " << component->getInstantiationIdentifier() << " to " << deviceId);
            component->setAssignedDeviceId(deviceId);

            CF::DeviceAssignmentType dat;
            dat.componentId = CORBA::string_dup(component->getIdentifier());
            dat.assignedDeviceId = CORBA::string_dup(deviceId);
            ossie::DeviceAssignmentInfo dai;
            dai.deviceAssignment = dat;
            dai.device =  CF::Device::_duplicate(devCapacityAlloc.device);
            appAssignedDevs.push_back(dai);

            // Add the 'usesdevice' to the device assignment sequence
            if (!CORBA::is_nil(usesDevice)) {
                ossie::DeviceAssignmentInfo dai_uses;
                CF::DeviceAssignmentType dat_uses;
                dat_uses.componentId = CORBA::string_dup(component->getIdentifier());
                CORBA::String_var usesDeviceId = usesDevice->identifier();
                dat_uses.assignedDeviceId = CORBA::string_dup(usesDeviceId);
                dai_uses.deviceAssignment = dat_uses;
                dai_uses.device = CF::Device::_duplicate(usesDevice);
                appAssignedDevs.push_back(dai_uses);
            }

            // add device allocations to current application context's capcacity table
            appCapacities[component->getIdentifier()].push_back( devCapacityAlloc );

            // add any implementation's allocations  to current application context's capcacity table
            for (unsigned propCount = 0; propCount < implCapacityList.size(); propCount++) {
                appCapacities[component->getIdentifier()].push_back(implCapacityList[propCount]);
            }

            component->setSelectedImplementation(impl);

            LOG_DEBUG(ApplicationFactory_impl, "**PASSED** COMP INST_ID:" <<component->getInstantiationIdentifier() << " IMPL:" <<impl->getId() );
            // we're done
            return;

        } catch ( ... ) {

            // unwind all the allocates and settings..
            LOG_DEBUG(ApplicationFactory_impl, "--FAILED-- Allocation of component's implementation " <<impl->getId() << " of "
                      <<component->getInstantiationIdentifier()<<". Undoing allocate devices");
            _undoCapacityAllocations(implCapacityList);

                // check if we allocated against a device....
            if( !CORBA::is_nil(devCapacityAlloc.device)) {
                LOG_DEBUG(ApplicationFactory_impl, "Deallocate capacities for Device: "<< ossie::corba::returnString(devCapacityAlloc.device->identifier()) );
                CapacityAllocationList  _dealloc;
                _dealloc.push_back( devCapacityAlloc );
            }

            std::string no_dev_id("");
            for ( uint i = 0; i < implUsesDevVec.size(); i++) {
                implUsesDevVec[i]->setAssignedDeviceId(no_dev_id);
            }
        }
    }

    LOG_DEBUG(ApplicationFactory_impl, "NO Allocation met for COMP: "<< component->getIdentifier() );
    if (implementations.size() != 0) {

         LOG_DEBUG(ApplicationFactory_impl, "Testing implUsesDeviceDepMet: "<< implUsesDeviceDepsMet);

        // reset uses relationship if everything fails.... Capacity should be cleaned up during _cleanupAllocationDevices or by collocation setup
        //
        if ( usesDevicesCapacities.size() > 0 ) {
            std::string no_dev_id("");
            for ( uint i = 0; i < usesDevVec.size(); i++) {
                usesDevVec[i]->setAssignedDeviceId(no_dev_id);
            }
        }

        if (!implUsesDeviceDepsMet){
            ostringstream eout;
            // fail b/c none of the implementations had their usesdevice deps met
            eout << "Failed satisfy 'usesdevice' dependencies for any implementation of component: '";
            eout << component->getName() << "' with component id: '" << component->getIdentifier() << "'";
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            if (cleanup) _cleanupAllocateDevices();
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
        } else {
            ostringstream eout;
            // fail b/c component's deps couldn't be satisfied
            eout << "Failed to satisfy device dependencies for component: '";
            eout << component->getName() << "' with component id: '" << component->getIdentifier() << "'";
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            if (cleanup) _cleanupAllocateDevices();
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
        }

        LOG_DEBUG(ApplicationFactory_impl, "Calling cleanupAllocateDevices (2)");
        if ( cleanup) _cleanupAllocateDevices(); // TO DO: this function seems to leak memory/corrupt the 'component' variable
    }

    LOG_DEBUG(ApplicationFactory_impl, "End of AllocateComponent: "<< component->getIdentifier());
}

/** Perform allocation/assignment of a particular component to the device.
 *  - First do allocation/assignment based on user provided DAS
 *  - If not specified in DAS, then iterate through devices looking for a device that satisfies
 *    the allocation properties
 */
void createHelper::allocateComponentToDevice( ossie::ComponentInfo* component,
                                              ossie::ImplementationInfo* implementation,
                                              const CF::DeviceAssignmentSequence& deviceAssignments,
                                              CapacityAllocation &deviceCapacityAlloc,
                                              ossie::Properties& devicePRF )
{
    ossie::DeviceNode deviceNode;
    CF::Properties tmpProps;
    CF::Properties allocatedCapacity;
    std::vector<std::string> tmpSoftpkgDeps;
    std::vector<std::string> _localSoftpkgDependencies;

    // First check to see if the component was assigned in the user provided DAS
    // See if a device was assigned in the DAS
    for (unsigned int j = 0; j < deviceAssignments.length (); j++) {
        LOG_TRACE(ApplicationFactory_impl, "DAS Component - " << deviceAssignments[j].componentId <<
                  "   This Component - " << component->getInstantiationIdentifier());

        if (strcmp (deviceAssignments[j].componentId, component->getInstantiationIdentifier()) == 0) {
            LOG_TRACE(ApplicationFactory_impl, "User-provided DAS: Component: '" << component->getName() <<
                      "'  Assigned device: '" << deviceAssignments[j].assignedDeviceId << "'");
            try {
                deviceNode = find_device_node_from_id(deviceAssignments[j].assignedDeviceId);
            }
            catch ( ... ) {
                // Don't segfault when the DAS specifies an invalid device, also comply with SR:196

                // since deviceNode isn't assigned at this point, we must create our own error message
                ostringstream eout;
                eout << "Unsuccessful allocation of component:{name='";
                eout << component->getName() << "',id='" << component->getIdentifier() << "'}";
                eout << " implementation:{id='" << implementation->getId() << "'}";
                eout << " to device:{name='', id='" << deviceAssignments[j].assignedDeviceId << "'}";
                eout << "; Failed with message:(Invalid user-provided assignment (DAS))";
                LOG_ERROR(ApplicationFactory_impl, eout.str());
                CF::DeviceAssignmentSequence badDAS;
                badDAS.length(1);
                badDAS[0].componentId = CORBA::string_dup(deviceAssignments[j].componentId);
                badDAS[0].assignedDeviceId = CORBA::string_dup(deviceAssignments[j].assignedDeviceId);
                throw CF::ApplicationFactory::CreateApplicationRequestError(badDAS);
            }

            // Check that the device meet's the needs of this component
            if ((implementation->getCodeType() == CF::LoadableDevice::EXECUTABLE) ||
                    (implementation->getCodeType() == CF::LoadableDevice::SHARED_LIBRARY)) {
                // Does this device provide LoadableDevice?
                CF::LoadableDevice_var loaddev;
                loaddev = ossie::corba::_narrowSafe<CF::LoadableDevice> (deviceNode.device);
                if(CORBA::is_nil(loaddev)) {
                    ostringstream eout;
                    errorMsgAllocate(eout, component, implementation, deviceNode, "Invalid user-provided assignment (DAS)");
                    LOG_ERROR(ApplicationFactory_impl, eout.str());
                    CF::DeviceAssignmentSequence badDAS;
                    badDAS.length(1);
                    badDAS[0].componentId = CORBA::string_dup(deviceAssignments[j].componentId);
                    badDAS[0].assignedDeviceId = CORBA::string_dup(deviceAssignments[j].assignedDeviceId);
                    throw CF::ApplicationFactory::CreateApplicationRequestError(badDAS);
                }

                if (implementation->getEntryPoint().size() != 0) {
                    // Does this device provide ExecutableDevice?
                    CF::ExecutableDevice_var execdev;
                    execdev = ossie::corba::_narrowSafe<CF::ExecutableDevice> (deviceNode.device);
                    if(CORBA::is_nil(execdev)) {
                        ostringstream eout;
                        errorMsgAllocate(eout, component, implementation, deviceNode, "Invalid user-provided assignment (DAS)");
                        LOG_ERROR(ApplicationFactory_impl, eout.str());
                        CF::DeviceAssignmentSequence badDAS;
                        badDAS.length(1);
                        badDAS[0].componentId = CORBA::string_dup(deviceAssignments[j].componentId);
                        badDAS[0].assignedDeviceId = CORBA::string_dup(deviceAssignments[j].assignedDeviceId);
                        throw CF::ApplicationFactory::CreateApplicationRequestError(badDAS);
                    }
                }
            }

            LOG_TRACE(ApplicationFactory_impl, "Allocating capacity for " << component->getIdentifier() << " using manually assigned device");
            // If a device was manually assigned a component, and there was not enough
            // capacity, then we will fail
            try {
                std::vector<std::string> _tmpSoftpkgDependencies;
                allocatedCapacity = allocateCapacity(component, implementation, deviceNode, devicePRF);
            } catch (CF::ApplicationFactory::CreateApplicationError& e) {
                ostringstream eout;
                std::string eMsg = "User-provided assignment (DAS) could not be completed; " + ossie::corba::returnString(e.msg);
                errorMsgAllocate(eout, component, implementation, deviceNode, eMsg);
                LOG_ERROR(ApplicationFactory_impl, eout.str());
                CF::DeviceAssignmentSequence badDAS;
                badDAS.length(1);
                badDAS[0].componentId = CORBA::string_dup(deviceAssignments[j].componentId);
                badDAS[0].assignedDeviceId = CORBA::string_dup(deviceAssignments[j].assignedDeviceId);
                throw CF::ApplicationFactory::CreateApplicationRequestError(badDAS);
            }

            deviceCapacityAlloc = AllocPropsInfo( deviceNode.device, allocatedCapacity );

            // Move the device used for this deployment to the front of the list
            LOG_TRACE(ApplicationFactory_impl, "Putting device \"" << deviceNode.identifier << "\" at the front of the list");
            rotateDeviceList(_executableDevices, deviceNode.identifier);
            return;
            //return deviceNode.device._retn();
        }
    }

    // If we get here, then we need to find a suitable device
    // Iterate through the devices until we find one that works for us
    // Ideally we will try IDLE devices first
    bool foundDevice = false;
    DeviceList::iterator deviceNodeIter;
    LOG_TRACE(ApplicationFactory_impl, "Searching for a place to deploy component amongst " << _executableDevices.size() << " executable devices");
    for (deviceNodeIter = _executableDevices.begin(); deviceNodeIter != _executableDevices.end(); deviceNodeIter++) {
        foundDevice = false; // make sure this is false
        if (!ossie::corba::objectExists(deviceNodeIter->device)) {
            LOG_WARN(ApplicationFactory_impl,
                     "Not using device for component allocation " << deviceNodeIter->identifier
                     << " because it no longer exists");
            continue;
        }
        //
        // Check that the device meet's the needs of this component
        //  - Validate the type of device meets the requirements in the 'code' section of the implementation
        //
        LOG_TRACE(ApplicationFactory_impl, "Checking Device " << deviceNodeIter->identifier);
        if (deviceNodeIter->device->usageState() == CF::Device::BUSY)
        {
            LOG_TRACE(ApplicationFactory_impl, "Ignoring Device " <<deviceNodeIter->label << " is BUSY");
            continue;
        }

        if ((implementation->getCodeType() == CF::LoadableDevice::EXECUTABLE) ||
                (implementation->getCodeType() == CF::LoadableDevice::SHARED_LIBRARY)) {
            // Does this device provide LoadableDevice?
            LOG_TRACE(ApplicationFactory_impl, "Device " << deviceNodeIter->identifier << " is loadable");
            CF::LoadableDevice_var loaddev;
            loaddev = ossie::corba::_narrowSafe<CF::LoadableDevice> (deviceNodeIter->device);
            if(CORBA::is_nil(loaddev)) {
                LOG_TRACE(ApplicationFactory_impl, "Device " << deviceNodeIter->identifier << " is not loadable");
                continue;
            }

            if (implementation->getEntryPoint().size() != 0) {
                // Does this device provide ExecutableDevice?
                LOG_TRACE(ApplicationFactory_impl, "Device " << deviceNodeIter->identifier << " is executable");

                CF::ExecutableDevice_var execdev;
                execdev = ossie::corba::_narrowSafe<CF::ExecutableDevice> (deviceNodeIter->device);
                if(CORBA::is_nil(execdev)) {
                    LOG_INFO(ApplicationFactory_impl, "Device " << deviceNodeIter->identifier << " is not executable");
                    continue;
                }
            }
        }

        //
        // Attempt to actually allocate capacity on the device
        //
        LOG_TRACE(ApplicationFactory_impl, "Trying to allocate capacities for " << deviceNodeIter->identifier);

        try {
            allocatedCapacity = allocateCapacity(component, implementation, *deviceNodeIter, devicePRF);
            foundDevice = true;
            deviceCapacityAlloc = AllocPropsInfo( deviceNodeIter->device, allocatedCapacity);

            // Move the device used for this deployment to the front of the list
            LOG_TRACE(ApplicationFactory_impl, "Putting device \"" << deviceNodeIter->identifier << "\" at the front of the list");
            if (deviceNodeIter != _executableDevices.begin()) {
                std::rotate(_executableDevices.begin(), deviceNodeIter, _executableDevices.end());
            }
            break;
        } catch(CF::ApplicationFactory::CreateApplicationError& e) {
            std::ostringstream iout;
            errorMsgAllocate(iout, component, implementation, *deviceNodeIter, ossie::corba::returnString(e.msg));
            LOG_DEBUG(ApplicationFactory_impl, iout.str());
            // Carry on my wayward son
        }
    }  // End iterating through the devices

    LOG_TRACE(ApplicationFactory_impl, "Done checking all the devices");

    // device will be nil if we could find a suitable assignment
    TRACE_EXIT(ApplicationFactory_impl);
    if (!foundDevice) {
        /* this isn't an error - just keep moving */
        //return CF::Device::_nil();
    }
    //return CF::Device::_duplicate(deviceNodeIter->device);
    return;

}

void createHelper::errorMsgAllocate(
        std::ostringstream&        eout, 
        ossie::ComponentInfo*      component,
        ossie::ImplementationInfo* implementation, 
        const ossie::DeviceNode&   deviceNode, 
        std::string                eMsg)
{ 
    eout << "Unsuccessful allocation of component:{name='";
    eout << component->getName() << "',id='" << component->getIdentifier() << "'}";
    eout << " implementation:{id='" << implementation->getId() << "'}";
    eout << " to device:{name='" << deviceNode.label << "', id='" << deviceNode.identifier << "'}";
    eout << " on DeviceManager:{name='" << deviceNode.devMgr.label << "', id='" << deviceNode.devMgr.identifier << "'}";
    if (!eMsg.empty()){
        eout << "; Failed with message:(" << eMsg << ")";
    }
}

/**
  Performs property matching and actually attempts to allocate capacity on the device
  if any part of the

  Grab Current FileSystem

  Look up devices, spd, prf files for the device...

  prf = Grab Properties of Device

  IF NO PRF then revert to DEPRECATED Behavior - just check if any implementation for the device EXISTS..

  For the Device's Current Implementation,
    -- Check if Device has Implementation in the PRF that matches...
    -- if we find a match then merge into prf object from above...
<------------ If merge errors out then THROW

  Check OS/Processor Properties for Device's Prop against implementation's parameter
<----------- If not THROW

   Check if device can match the component's implementation requirements....
<----------- If not THROW

   Get set of Property objects in a Sequence that will be allocated for this component against a Device

   For this Property Sequence (set) try and allocate capacities for each propery on the device.. deviceNode.device->allocateCapacity(allocProps)
<----------- If Allocation FAILS... THROW

 */
CF::Properties createHelper::allocateCapacity(ossie::ComponentInfo* component,
                                              ossie::ImplementationInfo* implementation,
                                              ossie::DeviceNode &deviceNode,
                                              ossie::Properties& devicePRF)
throw (CF::ApplicationFactory::CreateApplicationError)
{
    LOG_TRACE(ApplicationFactory_impl, "Device " << deviceNode.label << " software profile is " << deviceNode.softwareProfile)
    CF::Properties allocProps;

    // Get the Device Manager associated with this device
    CF::DeviceManager_ptr devMgr = deviceNode.devMgr.deviceManager;

    // Get the Device Manager's File System
    CF::FileSystem_var fileSystem;
    try {
        fileSystem = devMgr->fileSys();
    } CATCH_THROW_LOG_TRACE(ApplicationFactory_impl, "Could not connect to Device Manager:{name='" << deviceNode.devMgr.label <<
        "',id='" << deviceNode.devMgr.identifier << "'}", CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, "Could not connect to Device Manager"))

    if (CORBA::is_nil(fileSystem)) {
        LOG_TRACE(ApplicationFactory_impl, "Could not get device filesystem")
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, "Could not get device FileSystem");
    }

    CF::File_var _spd; // local vars

    // Parse the device's SPD
    SoftPkg spd;
    try {
        File_stream _spd(fileSystem, deviceNode.softwareProfile.c_str());
        spd.load(_spd, deviceNode.softwareProfile);
        _spd.close();
    } catch (ossie::parser_error& e) {
        ostringstream eout;
        eout << "creating application error; error parsing spd; " << e.what();
        LOG_ERROR(ApplicationFactory_impl, eout.str());
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
    } catch( ... ) {
        LOG_ERROR(ApplicationFactory_impl, "creating application error; unknown error parsing spd;");
        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, "Unknown error parsing SPD");
    }

    // Attempt to allocate capacity based on devices allocation properties and requested capacity
    if ( spd.getPRFFile() ) {

        // Parse the device's PRF
        LOG_TRACE(ApplicationFactory_impl, "Opening device property file" << spd.getPRFFile())
        Properties prf;
        try {
            File_stream _prf(fileSystem, spd.getPRFFile());
            prf.load(_prf);
            _prf.close();
        } catch (ossie::parser_error& e) {
            ostringstream eout;
            eout << "creating application error; error parsing device prf; " << e.what();
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
        } catch( ... ) {
            ostringstream eout;
            eout << "creating application error; ; unknown error parsing device prf;";
            LOG_ERROR(ApplicationFactory_impl, eout.str());
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
        }

        //check to see if there is an implementation specific PRF
        CORBA::String_var devImplId = devMgr->getComponentImplementationId(deviceNode.identifier.c_str());
        std::vector<ossie::SPD::Implementation>::const_iterator devImpl;
        for (devImpl = spd.getImplementations().begin(); devImpl != spd.getImplementations().end();  ++devImpl) {
            if (strcmp(devImpl->getID(), devImplId) == 0) {
                if (devImpl->getPRFFile()) {
                    try {
                        File_stream _implPrf(fileSystem, devImpl->getPRFFile());
                        prf.join(_implPrf);
                        _implPrf.close();
                    } catch (ossie::parser_error& e) {
                        ostringstream eout;
                        eout << "creating application error; error parsing implementation prf; " << e.what();
                        LOG_ERROR(ApplicationFactory_impl, eout.str());
                        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
                    } catch( ... ) {
                        ostringstream eout;
                        eout << "creating application error; ; unknown error parsing implementation prf;";
                        LOG_ERROR(ApplicationFactory_impl, eout.str());
                        throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EIO, eout.str().c_str());
                    }
                }
            }
        }

        devicePRF.join(prf);
        if (!implementation->checkProcessorAndOs(prf)) {
            ostringstream eout;
            eout << "Failed to allocate match processor/os for component '" << component->getName() << "' - '" << component->getIdentifier();
            LOG_DEBUG(ApplicationFactory_impl, eout.str());
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
        }
        LOG_TRACE(ApplicationFactory_impl, "Matched processor and OS")

        if (!implementation->checkMatchingDependencies(prf, deviceNode.softwareProfile, deviceNode.devMgr.deviceManager)) {
            std::ostringstream eout;
            eout << "Failed matching dependencies for component '" << component->getName() << "' - '" << component->getIdentifier();
            LOG_DEBUG(ApplicationFactory_impl, eout.str())
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
        }

        try {
            allocProps = implementation->getAllocationProperties(prf, component->getConfigureProperties());
        } catch (ossie::PropertyMatchingError e){
            std::ostringstream eout;
            eout << "Failed to resolve allocation properties between the component and device - ";
            eout << e.what();
            LOG_TRACE(ApplicationFactory_impl, eout.str());
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
        }

        LOG_TRACE(ApplicationFactory_impl, "Allocating Properties")
    
        try {
            if (allocProps.length() > 0) {
                if (!deviceNode.device->allocateCapacity (allocProps)) {
                    // We failed to allocate capacity
                    std::ostringstream eout;
                    eout << "Failed to allocate capacity on device - lacked ";
                    eout << "sufficient capacity for one or more of the following props: <";
                    for (unsigned int propIdx = 0; propIdx < allocProps.length(); propIdx++){
                        eout << "(" << allocProps[propIdx].id << "," << ossie::any_to_string(allocProps[propIdx].value) << ")";
                    }
                    eout << ">";
                    LOG_TRACE(ApplicationFactory_impl, eout.str())
                    throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, eout.str().c_str());
                }
            } else {
                LOG_TRACE(ApplicationFactory_impl, "No capacity allocated")
            }
        } catch ( CF::ApplicationFactory::CreateApplicationError& ) {
            throw; // make sure we keep any error message information from allocate failure
        } catch ( CF::Device::InsufficientCapacity e ) {
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_ENOSPC, "Failed to allocate capacity on device");
        } catch (CF::Device::InvalidCapacity e) {
            std::ostringstream eout;
            eout << "Invalid capacity passed to device; " << e.msg;
            LOG_ERROR(ApplicationFactory_impl, eout.str())
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
        } CATCH_THROW_LOG_ERROR(ApplicationFactory_impl, "Caught an unexpected error allocating capacity on device",
                    CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, "Caught an unexpected error allocating capacity on device"));
    } else {
        // A device with no PRF can only match a component with no
        // dependencies and no "os" or processor".  It's important
        // to note that per the SCA DTD *all* implementations must
        // have at least one of os, processor, or dependency and as
        // such all devices must have a PRF.  Therefore this is
        // an OSSIE specific behavior for backwards compatibility
        if ((implementation->getDependencyProperties().size() > 0) ||
                (implementation->getProcessorDeps().size() > 0) ||
                (implementation->getOsDeps().size() > 0)) {
            LOG_ERROR(ApplicationFactory_impl, "Failed to allocate capacity on device (does not meet dependency requirements)")
            ostringstream eout;
            eout << "Failed to allocate capacity on device. The device did not meet the dependency requirements ";
            eout << "of the component. Could indicate an invalid device software profile - device needs a PRF.";
            throw CF::ApplicationFactory::CreateApplicationError(CF::CF_EINVAL, eout.str().c_str());
        }
    }

    LOG_TRACE(ApplicationFactory_impl, "Capacity Allocated Successfully (allocateCapacity)")
    return allocProps;
}


bool createHelper::resolveSoftpkgDependencies(ossie::ImplementationInfo* implementation, 
                                              CF::Device_ptr device,  
                                              ossie::Properties& devicePRF)
throw (CF::ApplicationFactory::CreateApplicationError)
{
    std::vector< std::pair<std::string, ossie::optional_value<std::string> > > implementationReference;

    std::vector<SPD::SoftPkgRef> tmpSoftpkg = implementation->getSoftPkgDependency();

    std::vector<SPD::SoftPkgRef>::iterator iterSoftpkg = tmpSoftpkg.begin();

    while (iterSoftpkg != tmpSoftpkg.end()) {
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
                  foundImplementation = checkImplementationDependencyMatch(*implementation, spd_i[implCount], device, devicePRF);
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
              foundImplementation = checkImplementationDependencyMatch(*implementation, spd_i[implCount], device, devicePRF);
                if (foundImplementation) {
                    targetImplementation = implCount;
                    break;
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
            loadableDevice = CF::LoadableDevice::_narrow(device);
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
                    //LOG_WARN(ImplementationInfo, "Bad code type " << type);
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
            _softpkgList.push_back( SoftPkgLoad( loadableDevice, codeLocalFile.string().c_str()) );
            loadableDevice->load(_appFact._fileMgr, codeLocalFile.string().c_str(), codeType);
        } catch ( ... ) {
            return false;
        }
        iterSoftpkg++;
    }

    return true;
}


bool createHelper::checkImplementationDependencyMatch(
    ossie::ImplementationInfo&       implementation_1, 
    const ossie::ImplementationInfo& implementation_2, 
    CF::Device_ptr device,
    ossie::Properties& devicePRF)
{
   if (!implementation_2.checkProcessorAndOs(devicePRF)) {
        return false;
    }
    
    // resolve dependencies on dependencies
    std::vector<SPD::SoftPkgRef> tmpSoftpkg = implementation_1.getSoftPkgDependency();
    std::vector<SPD::SoftPkgRef>::iterator iterSoftpkg = tmpSoftpkg.begin();
    bool retval = true;
    if (iterSoftpkg != tmpSoftpkg.end()) {
        ossie::ImplementationInfo* tmp_impl = const_cast<ossie::ImplementationInfo*>(&implementation_2);
        retval = (resolveSoftpkgDependencies(tmp_impl, device, devicePRF));
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
    //const std::vector<SPD::Implementation>& _devspdimpls;

    const char* assemblyControllerRefId = _appFact._sadParser.getAssemblyControllerRefId();
    std::string assemblyControllerRefId_s(assemblyControllerRefId); //string repr for comparison below
    // get all device implementations deployed by each DevMgr
    //getDeviceImplementations(_devspdimpls, _deviceAssignments);

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
        TRACE_EXIT(ApplicationFactory_impl);
        return CF::Device::_nil();
    }
}

const ossie::DeviceNode& createHelper::find_device_node_from_id(const char* device_id) throw(std::exception)
{
    DeviceList::iterator dn = _registeredDevices.begin();
    while (dn != _registeredDevices.end()) {
        if (dn->identifier == device_id) {
            return *dn;
        }
        dn++;
    }

    TRACE_EXIT(ApplicationFactory_impl);
    throw(std::exception());
}


/** Given a CORBA device pointer, returns its associate Device Manager
 *  - Gets the Device Manager of a particular device
 */
CF::DeviceManager_ptr createHelper::find_devicemgr_for_device(CF::Device_ptr device)
{
    TRACE_ENTER(ApplicationFactory_impl);
    // Do this naively and safely for now
    CORBA::String_var deviceId;
    CF::DomainManager::DeviceManagerSequence_var devMgrs;
    try {
        devMgrs = _appFact._dmnMgr->deviceManagers();
        deviceId = device->identifier();
    } catch (CORBA::Exception& ex) {
        LOG_WARN(ApplicationFactory_impl, "Unexpected CORBA exception while locating the devices manager : " << ex._name());
        return CF::DeviceManager::_nil();
    }

    LOG_TRACE(ApplicationFactory_impl, "Searching for device manager associated with device " << ossie::corba::returnString(deviceId));
    for (unsigned int i = 0; i < devMgrs->length(); i++) {
        CF::DeviceSequence_var devices;
        try {
            devices = devMgrs[i]->registeredDevices();
        } catch (CORBA::Exception& ex) {
            LOG_WARN(ApplicationFactory_impl, "Unexpected CORBA exception while loading registered devices : " << ex._name());
            continue;
        }

        for (unsigned int j = 0; j < devices->length(); j++) {
            try {
                CORBA::String_var otherId = devices[j]->identifier();
                if (strcmp(otherId, deviceId) == 0) {
                    LOG_TRACE(ApplicationFactory_impl, "Located device manager associated with device " << ossie::corba::returnString(deviceId));
                    return devMgrs[i];
                }
            } catch (CORBA::Exception& ex) {
                LOG_WARN(ApplicationFactory_impl, "Unexpected CORBA exception while checking device identifier : " << ex._name());
                continue;
            }
        }
    }

    LOG_WARN(ApplicationFactory_impl, "Failed to locate device manager associated with device " << ossie::corba::returnString(deviceId));
    return CF::DeviceManager::_nil();
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
string ApplicationFactory_impl::getWaveformContextName(string name)
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

/** Configures the components
 *  - Configure each component
 */
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
            _appFact(appFact)
{
    this->_waveformContextName = waveformContextName;
    this->_baseNamingContext   = baseNamingContext;
    this->_waveformContext     = CosNaming::NamingContext::_duplicate(waveformContext);
}

createHelper::~createHelper()
{
}

#if ENABLE_EVENTS

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
#endif

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

/** Rotates a device list to put the device with the given identifier first
 */
void createHelper::rotateDeviceList(DeviceList& devices, const std::string& identifier)
{
    const DeviceList::iterator begin = devices.begin();
    for (DeviceList::iterator node = begin; node != devices.end(); ++node) {
        if (node->identifier == identifier) {
            if (node != begin) {
                std::rotate(devices.begin(), node, devices.end());
            }
            return;
        }
    }
}
