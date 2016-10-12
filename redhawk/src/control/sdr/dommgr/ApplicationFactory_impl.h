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

#ifndef APPLICATIONFACTORY_H
#define APPLICATIONFACTORY_H

#include <string>
#include <omniORB4/CORBA.h>

#include <ossie/CF/cf.h>
#include <ossie/CF/StandardEvent.h>
#include <ossie/SoftwareAssembly.h>
#include <ossie/debug.h>
#include <ossie/prop_utils.h>
#include <ossie/prop_helpers.h>
#include <ossie/PropertyMap.h>

#include "PersistenceStore.h"
#include "applicationSupport.h"
#include "connectionSupport.h"

class DomainManager_impl;
class Application_impl;
class AllocationManager_impl;

class createHelper;

class ApplicationFactory_impl: public virtual POA_CF::ApplicationFactory
{
    ENABLE_LOGGING

private:
    
    ApplicationFactory_impl();  // Node default constructor
    ApplicationFactory_impl(ApplicationFactory_impl&);  // No copying

    std::string _name;
    std::string _identifier;
    std::string _softwareProfile;

    CosNaming::NamingContext_var _domainContext;

    ossie::SoftwareAssembly _sadParser;
    CF::FileManager_var     _fileMgr;
    CF::DomainManager_var   _dmnMgr;

    boost::mutex _pendingCreateLock;

    std::string         _domainName;
    DomainManager_impl* _domainManager;

    unsigned short _lastWaveformUniqueId;
    // Support for creating a new waveform naming context
    std::string getWaveformContextName(std::string name);
    std::string getBaseWaveformContext(std::string waveform_context);

    static void ValidateFileLocation ( CF::FileManager_ptr fileMgr, const std::string &profile );
    static void ValidateSoftPkgDep( CF::FileManager_ptr fileMgr, const std::string &profile );
    static void ValidateSPD ( CF::FileManager_ptr fileMgr, const std::string &profile, const bool require_prf=true, const bool require_scd=true );
    static void ValidateSPD (CF::FileManager_ptr fileMgr, ossie::SoftPkg &spd, const std::string &profile, const bool require_prf=true, const bool require_scd=true );

public:
    ApplicationFactory_impl (const std::string& softwareProfile, 
                             const std::string& domainName, 
                             DomainManager_impl* domainManager);
    ~ApplicationFactory_impl ();

    CF::Application_ptr create (const char* name,
                                const CF::Properties& initConfiguration,
                                const CF::DeviceAssignmentSequence& deviceAssignments)
    throw (CF::ApplicationFactory::InvalidInitConfiguration,
           CF::ApplicationFactory::CreateApplicationRequestError,
           CF::ApplicationFactory::CreateApplicationInsufficientCapacityError,
           CF::ApplicationFactory::CreateApplicationError,
           CORBA::SystemException);

    // getters for attributes
    char* name () throw (CORBA::SystemException) {
        return CORBA::string_dup(_name.c_str());
    }

    char* identifier () throw (CORBA::SystemException) {
        return CORBA::string_dup(_identifier.c_str());
    }

    char* softwareProfile () throw (CORBA::SystemException) {
        return CORBA::string_dup(_softwareProfile.c_str());
    }

    const std::string & getID () { return _identifier; }
    const std::string & getName () { return _name; }

    // allow createHelper to have access to ApplicationFactory_impl
    friend class createHelper;
    friend class ScopedAllocations;
};
#endif

#ifndef CREATEHELPER_H
#define CREATEHELPER_H

class ScopedAllocations {
public:
    ScopedAllocations(AllocationManager_impl& allocator);
    ~ScopedAllocations();

    void push_back(const std::string& allocationID);

    template <class T>
    void transfer(T& dest);

    void transfer(ScopedAllocations& dest);

    void deallocate();

private:
    AllocationManager_impl& _allocator;
    std::list<std::string> _allocations;
};

class createHelper:
public ossie::ComponentLookup,
public ossie::DeviceLookup
{

public:
    struct componentReservation {
        std::string id;
        float       reservation;
    };
    std::vector<componentReservation> componentReservations;
    
    typedef std::map<std::string,std::string> DeviceAssignmentMap;

    createHelper (const ApplicationFactory_impl& appFact,
                  std::string waveformContextName,
                  std::string base_naming_context,
                  CosNaming::NamingContext_ptr WaveformContext,
                  CosNaming::NamingContext_ptr DomainContext);
    ~createHelper ();

    CF::Application_ptr create (const char* name,
                                const CF::Properties& initConfiguration,
                                const DeviceAssignmentMap& deviceAssignments)
    throw (CF::ApplicationFactory::InvalidInitConfiguration,
           CF::ApplicationFactory::CreateApplicationRequestError,
           CF::ApplicationFactory::CreateApplicationError,
           CORBA::SystemException);

private:

    // List of used devices assignments
    typedef std::vector< ossie::DeviceAssignmentInfo >                 DeviceAssignmentList;

    // list of components that are part of a collocation
    typedef std::vector <ossie::ComponentInfo* >                       PlacementList;

    // Used for storing the current state of the OE & create process
    const ApplicationFactory_impl& _appFact;

    // Local pointer to the allocation manager
    AllocationManager_impl* _allocationMgr;
 
    // Tracks allocation IDs made during creation, and automates cleanup on
    // failure
    ScopedAllocations _allocations;
    CF::Properties    _app_affinity;

    ossie::DeviceList _registeredDevices;
    ossie::DeviceList _executableDevices;
    PlacementList     _requiredComponents;
    std::map<std::string,float> specialized_reservations;

    //
    // List of used devices allocated during application creation
    //
    DeviceAssignmentList          _appUsedDevs;
    std::vector<CF::Resource_var> _startSeq;
    std::vector<std::string>      _startOrderIds;
    
    // waveform instance-specific naming context (unique to the instance of the waveform)
    std::string _waveformContextName; 

    // full (includes full context path) waveform instance-specific naming context
    std::string _baseNamingContext; 

    // CORBA naming context
    CosNaming::NamingContext_var _waveformContext; 
    CosNaming::NamingContext_ptr _domainContext; 

    ossie::ApplicationInfo _appInfo;

    // createHelper helper methods
    ossie::ComponentInfo* getAssemblyController();
    void overrideExternalProperties(const CF::Properties& initConfiguration);
    void overrideProperties(const CF::Properties& initConfiguration, ossie::ComponentInfo* component);
    void assignRemainingComponentsToDevices(const std::string &appIdentifier);
    void _assignComponentsUsingDAS(
        const DeviceAssignmentMap& deviceAssignments, const std::string &appIdentifier);
    void _getComponentsToPlace(
        const std::vector<ossie::ComponentPlacement>& collocatedComponents,
        ossie::DeviceIDList&                          assignedDevices,
        PlacementList&                                placingComponents);
    void _connectComponents(
        std::vector<ossie::ConnectionNode>& connections);
    void _configureComponents();
    void _checkAssemblyController(
        CF::Resource_ptr      assemblyController,
        ossie::ComponentInfo* assemblyControllerComponent) const;
    void setUpExternalPorts(Application_impl* application);
    void setUpExternalProperties(Application_impl* application);
    void _handleHostCollocation(const std::string &appIdentifier);
    void _placeHostCollocation(const ossie::SoftwareAssembly::HostCollocation& collocation, const std::string &appIdentifier);
    void _handleUsesDevices(const std::string& appName);
    void _resolveImplementations(PlacementList::iterator comp, PlacementList& compList, std::vector<ossie::ImplementationInfo::List> &res_vec);
    void _removeUnmatchedImplementations(std::vector<ossie::ImplementationInfo::List> &res_vec);
    void _consolidateAllocations(const ossie::ImplementationInfo::List& implementations, CF::Properties& allocs);
    void _evaluateMATHinRequest(CF::Properties &request, const CF::Properties &configureProperties);
    void _castRequestProperties(CF::Properties& allocationProperties, const std::vector<ossie::SPD::PropertyRef> &prop_refs, unsigned int offset=0);
    void _castRequestProperties(CF::Properties& allocationProperties, const std::vector<ossie::SoftwareAssembly::PropertyRef> &prop_refs,
            unsigned int offset=0);
    CF::DataType castProperty(const ossie::ComponentProperty* property);

    // Populate _requiredComponents vector
    void getRequiredComponents() throw (CF::ApplicationFactory::CreateApplicationError); 

    // Supports allocation
    bool allocateUsesDevices(const std::string& componentIdentifier,
                             const ossie::UsesDeviceInfo::List& usesDevices,
                             const CF::Properties& configureProperties,
                             DeviceAssignmentList& deviceAssignments,
                             ScopedAllocations& allocations);
    CF::AllocationManager::AllocationResponseSequence* allocateUsesDeviceProperties(
        const ossie::UsesDeviceInfo::List& component,
        const CF::Properties& configureProperties);
    void allocateComponent(ossie::ComponentInfo* component,
                           const std::string& assignedDeviceId,
                           DeviceAssignmentList &appAssignedDevices,
                           const std::string& appIdentifier);

    ossie::AllocationResult allocateComponentToDevice(ossie::ComponentInfo* component,
                                   ossie::ImplementationInfo* implementation,
                                   const std::string& assignedDeviceId,
                                   const std::string& appIdentifier);

    bool resolveSoftpkgDependencies(ossie::ImplementationInfo* implementation, ossie::DeviceNode& device);
    ossie::ImplementationInfo* resolveDependencyImplementation(ossie::SoftpkgInfo* softpkg, ossie::DeviceNode& device);
    
    // Supports loading, executing, initializing, configuring, & connecting
    void loadDependencies(ossie::ComponentInfo& component,
                          CF::LoadableDevice_ptr device,
                          const std::vector<ossie::SoftpkgInfo*>& dependencies);

    void loadAndExecuteComponents(CF::ApplicationRegistrar_ptr _appReg);
    void applyApplicationAffinityOptions();

    void attemptComponentExecution(
        const boost::filesystem::path&                                  executeName,
        CF::ExecutableDevice_ptr                                        execdev,
        ossie::ComponentInfo*                                           component,
        const ossie::ImplementationInfo*                                implementation);

    void waitForComponentRegistration();
    void initializeComponents();

    void configureComponents();
    void connectComponents(
        std::vector<ossie::ConnectionNode>& connections, 
        std::string                         base_naming_context);

    // Functions for looking up particular components/devices
    CF::Device_ptr find_device_from_id(const char*);
    const ossie::DeviceNode& find_device_node_from_id(const char*) throw(std::exception);
    ossie::ComponentInfo* findComponentByInstantiationId(const std::string& identifier);

    // Cleanup - used when create fails/doesn't succeed for some reason
    bool _isComplete;
    void _cleanupFailedCreate();
    Application_impl* _application;

    /* Implements the ConnectionManager functions
     *  - Makes this class compatible with the ConnectionManager
     */
    // ComponentLookup interface
    CF::Resource_ptr lookupComponentByInstantiationId(const std::string& identifier);

    // DeviceLookup interface
    CF::Device_ptr lookupDeviceThatLoadedComponentInstantiationId(const std::string& componentId);
    CF::Device_ptr lookupDeviceUsedByComponentInstantiationId(
        const std::string& componentId, 
        const std::string& usesId);
    CF::Device_ptr lookupDeviceUsedByApplication(const std::string& usesRefId);
};
#endif
