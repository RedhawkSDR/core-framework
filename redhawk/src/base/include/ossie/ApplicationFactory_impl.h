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


#include <string>

#include "CF/cf.h"
#include "CF/StandardEvent.h"

#include "DomainManager_impl.h"
#include "ossie/connectionSupport.h"
#include "PersistanceStore.h"

#include "ossiecf.h"
#include "FileSystem_impl.h"
#include "SoftwareAssembly.h"
#include "Properties.h"

#include "ossie/CorbaUtils.h"
#include "ossie/EventChannelSupport.h"
#include "ossie/prop_helpers.h"
#include "ossie/FileStream.h"
#include "ossie/debug.h"
#include "ossie/portability.h"

#include "Application_impl.h"

#ifdef HAVEOMNIORB4
#include <omniORB4/CORBA.h>
#endif

#include "ossie/ossieSupport.h"
#include "ossie/debug.h"

#ifndef APPLICATIONFACTORY_H
#define APPLICATIONFACTORY_H

using namespace std;

class OSSIECF_API ApplicationFactory_impl;
class createHelper;

class OSSIECF_API
    ApplicationFactory_impl:
public virtual POA_CF::ApplicationFactory
//public ossie::ComponentLookup,
//public ossie::DeviceLookup
{
    ENABLE_LOGGING

private:
    ApplicationFactory_impl();  // Node default constructor
    ApplicationFactory_impl(ApplicationFactory_impl&);  // No copying

    string _name;
    string _identifier;
    string _softwareProfile;
    CosNaming::NamingContext_var DomainContext;

    ossie::SoftwareAssembly _sadParser;
    CF::FileManager_var fileMgr;
    CF::DomainManager_var dmnMgr;
    CF::DomainManager::ApplicationSequence* appseq;

    // Support for creating a new waveform naming context
    string getWaveformContextName(string name);
    string getBaseWaveformContext(string waveform_context);
    boost::mutex pendingCreateLock;

    string _domainName;
    string _domainManagerName;
    DomainManager_impl* _domainManager;

    unsigned short _lastWaveformUniqueId;
public:
    ApplicationFactory_impl (const char* _softwareProfile, CF::DomainManager::ApplicationSequence* _appseq, string domainName, string domainManagerName, DomainManager_impl* _domainManager);
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

    const ossie::SoftwareAssembly& getSadParser() const {
        return _sadParser;
    };


    // allow createHelper to have access to ApplicationFactory_impl
    friend class createHelper;

};
#endif

#ifndef CREATEHELPER_H
#define CREATEHELPER_H

class createHelper:
public ossie::ComponentLookup,
public ossie::DeviceLookup
{

public:
    createHelper (const ApplicationFactory_impl& appFact,
                  string _waveform_context_name,
                  string _base_naming_context,
                  CosNaming::NamingContext_ptr _WaveformContext);
    ~createHelper ();

    CF::Application_ptr create (const char* name,
                                const CF::Properties& initConfiguration,
                                const CF::DeviceAssignmentSequence& deviceAssignments)
    throw (CF::ApplicationFactory::InvalidInitConfiguration,
           CF::ApplicationFactory::CreateApplicationRequestError,
           CF::ApplicationFactory::CreateApplicationError,
           CORBA::SystemException);

private:
    // Used for storing the current state of the OE & create process
    const ApplicationFactory_impl& _appFact;
    
    ossie::DeviceList _registeredDevices;
    std::vector <ossie::ComponentInfo*> requiredComponents;
    
    std::map<std::string, vector<ossie::AllocPropsInfo> > allocPropsTable;
    vector<ossie::DeviceAssignmentInfo> _usedDevs;
    std::vector<CF::Resource_ptr> _startSeq;
    std::vector<std::string> _startOrderIds;
    PROC_ID_SEQ _pidSeq;
    map<std::string, std::string> fileTable;
    map<std::string, std::pair<std::string, std::string> > loadedComponentTable; // mapping of component id to filenames/device id tuple
    map<std::string, std::pair<std::string, unsigned long> > runningComponentTable; // mapping of component id to filenames/device id tuple

    string waveform_context_name; // waveform instance-specific naming context (unique to the instance of the waveform)
    string base_naming_context; // full (includes full context path) waveform instance-specific naming context
    CosNaming::NamingContext_var WaveformContext; // CORBA naming context
    ELEM_SEQ _namingCtxSeq;
    ELEM_SEQ _implSeq;

    void getRequiredComponents() throw (CF::ApplicationFactory::CreateApplicationError); // Populate requiredComponents vector
    void getDeviceImplementations(std::vector<ossie::SPD::Implementation>&, const CF::DeviceAssignmentSequence&);

    // Supports allocation
    CF::Device_ptr allocateUsesDeviceProperties(ossie::ComponentImplementationInfo* component_impl,
                                                unsigned int usesDevIdx, CF::Properties* allocProps, const CF::Properties& configureProperties);
    void allocateComponent(ossie::ComponentInfo* component, const CF::DeviceAssignmentSequence& deviceAssignments);
    CF::Device_ptr allocateComponentToDevice(ossie::ComponentInfo* component,
                                             ossie::ImplementationInfo* implementation,
                                             const CF::DeviceAssignmentSequence& deviceAssignments, CF::Properties* tmp);

    CF::Properties allocateCapacity(ossie::ComponentInfo* component, ossie::ImplementationInfo* implementation,
                                    ossie::DeviceNode deviceNode)
    throw (CF::ApplicationFactory::CreateApplicationError);
    
    bool resolveSoftpkgDependencies(ossie::ImplementationInfo* implementation, CF::Device_ptr device)
    throw (CF::ApplicationFactory::CreateApplicationError);
    
    bool checkImplementationDependencyMatch(ossie::ImplementationInfo& implementation_1, const ossie::ImplementationInfo& implementation_2);
    
    void errorMsgAllocate(std::ostringstream&, ossie::ComponentInfo*, ossie::ImplementationInfo*, const ossie::DeviceNode&, std::string);

    // Supports loading, executing, initializing, configuring, & connecting
    void loadAndExecuteComponents(PROC_ID_SEQ* pid, std::map<std::string, std::string> *fileTable,
                                  CosNaming::NamingContext_ptr WaveformContext,
                                  map<std::string, std::pair<std::string, std::string> > *loadedComponentTable,
                                  map<std::string, std::pair<std::string, unsigned long> > *runningComponentTable);
    void initializeComponents(CF::Resource_var& _assemblyController, CosNaming::NamingContext_ptr WaveformContext);
    void addComponentsToApplication(CosNaming::NamingContext_ptr WaveformContext, Application_impl *_application);
    void configureComponents();
    void connectComponents(std::vector<ossie::ConnectionNode>& connections, string _base_naming_context);

    // Functions for looking up particular components/devices
    CF::Device_ptr find_device_from_id(const char*);
    const ossie::DeviceNode& find_device_node_from_id(const char*) throw(std::exception);
    CF::DeviceManager_ptr find_devicemgr_for_device(CF::Device_ptr device);
    ossie::ComponentInfo* findComponentByInstantiationId(const std::string& identifier);

    // Cleanup - used when create fails/doesn't succeed for some reason
    bool alreadyCleaned;
    void _cleanupLoadAndExecuteComponents();
    void _cleanupNewContext();
    void _cleanupAllocateDevices();
    void _cleanupRequiredComponents();
    void _cleanupResourceNotFound();
    void _cleanupAssemblyControllerInitializeFailed();
    void _cleanupAssemblyControllerConfigureFailed();
    void _cleanupResourceInitializeFailed();
    void _cleanupResourceConfigureFailed();
    void _cleanupApplicationCreateFailed();
    void _cleanupConnectionFailed();
    void _undoAllocateUsesDevices(std::vector<ossie::AllocPropsInfo >& propSet);
    
    // Shutdown - clean up memory
    void _deleteRequiredComponents();

    /** Implements the ConnectionManager functions
     *  - Makes this class compatible with the ConnectionManager
     */
    // ComponentLookup interface
    CF::Resource_ptr lookupComponentByInstantiationId(const std::string& identifier);
    CF::DeviceManager_ptr lookupDeviceManagerByInstantiationId(const std::string& identifier);
#if ENABLE_EVENTS
    CosEventChannelAdmin::EventChannel_ptr lookupEventChannel(const std::string &EventChannelName);
    unsigned int incrementEventChannelConnections(const std::string &EventChannelName);
    unsigned int decrementEventChannelConnections(const std::string &EventChannelName);
#endif
    // DeviceLookup interface
    CF::Device_ptr lookupDeviceThatLoadedComponentInstantiationId(const std::string& componentId);
    CF::Device_ptr lookupDeviceUsedByComponentInstantiationId(const std::string& componentId, const std::string& usesId);
};
#endif
