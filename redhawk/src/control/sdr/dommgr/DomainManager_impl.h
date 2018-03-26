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


#ifndef __DOMAINMANAGER_IMPL__
#define __DOMAINMANAGER_IMPL__

#include <set>
#include <vector>
#include <string>

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <COS/CosEventChannelAdmin.hh>

#include <ossie/CF/cf.h>
#include <ossie/PropertySet_impl.h>
#include <ossie/Logging_impl.h>
#include <ossie/Runnable.h>
#include <ossie/Events.h>
#include <ossie/logging/loghelpers.h>
#include <ossie/FileManager_impl.h>
#include <ossie/File_impl.h>
#include <ossie/ParserLogs.h>
#include <ossie/logging/loghelpers.h>

#include "PersistenceStore.h"
#include "connectionSupport.h"
#include "DomainManager_EventSupport.h"
#include "EventChannelManager.h"
#include "struct_props.h"
#include "struct_props.h"

#include "../../parser/internal/dcd-pimpl.h"


class Application_impl;
class ApplicationFactory_impl;
class AllocationManager_impl;
class ConnectionManager_impl;


class DomainManager_impl: public virtual POA_CF::DomainManager, public PropertySet_impl, public Logging_impl, public ossie::ComponentLookup, public ossie::DomainLookup, public ossie::Runnable
{
    ENABLE_LOGGING

///////////////////////////
// Constructors/Destructors
///////////////////////////
public:

      DomainManager_impl (const char*, const char*, const char*, const char *, const char*, bool, bool, bool, int);
    ~DomainManager_impl ();

    friend class ODM_Channel_Supplier_i;

private:
    DomainManager_impl(); // no default constructor
    DomainManager_impl(const DomainManager_impl&);  // No copying

//////////////////
// CORBA Functions
//////////////////
public:
    boost::mutex interfaceAccess;

    char * identifier (void)
        throw (CORBA::SystemException);

    char * name (void)
        throw (CORBA::SystemException);
        
    char * domainManagerProfile (void)
        throw (CORBA::SystemException);

    CF::FileManager_ptr fileMgr (void) throw (CORBA::SystemException);
    
    CF::AllocationManager_ptr allocationMgr (void) throw (CORBA::SystemException);

    CF::EventChannelManager_ptr eventChannelMgr (void) throw (CORBA::SystemException);

    CF::ConnectionManager_ptr connectionMgr (void) throw (CORBA::SystemException);
    
    CF::DomainManager::ApplicationFactorySequence * applicationFactories (void) throw (CORBA::SystemException);
    
    CF::DomainManager::ApplicationSequence * applications (void) throw (CORBA::SystemException);
    
    CF::DomainManager::DeviceManagerSequence * deviceManagers (void) throw (CORBA::SystemException);

    CF::DomainManager::DomainManagerSequence * remoteDomainManagers (void) throw (CORBA::SystemException);
        
    void registerDevice (CF::Device_ptr registeringDevice, CF::DeviceManager_ptr registeredDeviceMgr)
        throw (CF::DomainManager::RegisterError, CF::DomainManager::DeviceManagerNotRegistered, CF::InvalidProfile, CF::InvalidObjectReference, CORBA::SystemException);
    void _local_registerDevice (CF::Device_ptr registeringDevice, CF::DeviceManager_ptr registeredDeviceMgr);
        
    void registerDeviceManager (CF::DeviceManager_ptr deviceMgr)
        throw (CF::DomainManager::RegisterError, CF::InvalidProfile, CF::InvalidObjectReference, CORBA::SystemException);
    void _local_registerDeviceManager (CF::DeviceManager_ptr deviceMgr);
        
    void unregisterDeviceManager (CF::DeviceManager_ptr deviceMgr)
        throw (CF::DomainManager::UnregisterError, CF::InvalidObjectReference, CORBA::SystemException);
        
    void unregisterDevice (CF::Device_ptr unregisteringDevice)
        throw (CF::DomainManager::UnregisterError, CF::InvalidObjectReference, CORBA::SystemException);
        
    CF::Application_ptr createApplication(const char* profileFileName, const char* name, const CF::Properties& initConfiguration, const CF::DeviceAssignmentSequence& deviceAssignments);

    void installApplication (const char* profileFileName)
        throw (CF::DomainManager::ApplicationInstallationError, CF::InvalidFileName, CF::InvalidProfile, CORBA::SystemException, CF::DomainManager::ApplicationAlreadyInstalled);
    void _local_installApplication (const std::string& profileFileName);
           
    void uninstallApplication (const char* applicationId)
        throw (CF::DomainManager::ApplicationUninstallationError, CF::DomainManager::InvalidIdentifier, CORBA::SystemException);
    void _local_uninstallApplication (const char* applicationId);
           
    void registerService (CORBA::Object_ptr registeringService, CF::DeviceManager_ptr registeredDeviceMgr, const char* name)
        throw (CF::DomainManager::RegisterError, CF::DomainManager::DeviceManagerNotRegistered, CF::InvalidObjectReference, CORBA::SystemException);
    void _local_registerService (CORBA::Object_ptr registeringService, CF::DeviceManager_ptr registeredDeviceMgr, const char* name);
           
    void unregisterService (CORBA::Object_ptr unregisteringService, const char* name)
        throw (CF::DomainManager::UnregisterError, CF::InvalidObjectReference, CORBA::SystemException);
           
    void registerWithEventChannel (CORBA::Object_ptr registeringObject, const char* registeringId, const char* eventChannelName)
        throw (CF::DomainManager::AlreadyConnected, CF::DomainManager::InvalidEventChannelName, CF::InvalidObjectReference, CORBA::SystemException);
           
    void unregisterFromEventChannel (const char* unregisteringId, const char* eventChannelName)
        throw (CF::DomainManager::NotConnected, CF::DomainManager::InvalidEventChannelName, CORBA::SystemException);

    void registerRemoteDomainManager (CF::DomainManager_ptr registeringRemoteDomainManager)
        throw (CF::DomainManager::RegisterError, CF::InvalidObjectReference, CORBA::SystemException);

    void unregisterRemoteDomainManager (CF::DomainManager_ptr unregisteringRemoteDomainManager)
        throw (CF::DomainManager::UnregisterError, CF::InvalidObjectReference, CORBA::SystemException);

////////////////////////////////////////////////////////
// Public Helper Functions and Members
// Mostly used by ApplicationFactory and Application
////////////////////////////////////////////////////////
public:
    void shutdown(int signal);

    void restoreState(const std::string& _db_uri);
    void restoreEventChannels(const std::string& _db_uri);

    void addApplication(Application_impl* new_app);

    void addPendingApplication(Application_impl* application);
    void cancelPendingApplication(Application_impl* application);
    void completePendingApplication(Application_impl* application);
    
    void releaseAllApplications();
    
    void shutdownAllDeviceManagers();
    
    void removeApplication(std::string app_id);

    void updateLocalAllocations(const ossie::AllocationTable& localAllocations);
    void updateRemoteAllocations(const ossie::RemoteAllocationTable& remoteAllocations);

    const std::string& getDomainManagerName (void) const {
        return _domainName;
    }

    std::string getFullDomainManagerName (void) const {
        return _domainName + "/" + _domainName;
    }
    
    int getComponentBindingTimeout (void) const {
      return componentBindingTimeout;
    }

    ossie::DeviceList getRegisteredDevices(); // Get a copy of registered devices

    ossie::DomainManagerList getRegisteredRemoteDomainManagers(); // Get a copy of registered devices

    // DomainLookup methods
    CORBA::Object_ptr lookupDomainObject (const std::string& type, const std::string& name);
    CF::DeviceManager_ptr lookupDeviceManagerByInstantiationId(const std::string& identifier);


    ossie::events::EventChannel_ptr lookupEventChannel(const std::string &EventChannelName);
    unsigned int incrementEventChannelConnections(const std::string &EventChannelName);
    unsigned int decrementEventChannelConnections(const std::string &EventChannelName);

    // ComponentLookup methods
    CF::Resource_ptr lookupComponentByInstantiationId(const std::string& identifier);

    CosEventChannelAdmin::EventChannel_ptr getEventChannel(const std::string &name);
    bool eventChannelExists(const std::string &name);
    ::ossie::events::EventChannel_ptr createEventChannel (const std::string& name);

    void destroyEventChannel (const std::string& name);

    void sendAddEvent( const std::string &prod_id, 
                       const std::string &source_id, 
                       const std::string &source_name,
                       CORBA::Object_ptr obj,
                       StandardEvent::SourceCategoryType sourceCategory);

    void sendRemoveEvent( const std::string &prod_id, 
                       const std::string &source_id, 
                       const std::string &source_name,
                       StandardEvent::SourceCategoryType sourceCategory);
    
    void sendResourceStateChange( const std::string &source_id,
                       const std::string &source_name,
                       const ExtendedEvent::ResourceStateChangeType stateChangeFrom, 
                       const ExtendedEvent::ResourceStateChangeType stateChangeTo);

    std::map<std::string, ossie::consumerContainer> registeredConsumers;

    CF::FileManager_var _fileMgr;

    AllocationManager_impl* _allocationMgr;


    std::string getLastDeviceUsedForDeployment();
    void setLastDeviceUsedForDeployment(const std::string& identifier);

    bool getUseLogConfigResolver() { return _useLogConfigUriResolver; };
    
    void closeAllOpenFileHandles();

    rh_logger::LoggerPtr  getLogger() const { return __logger; };

    rh_logger::LoggerPtr  getInstanceLogger(const char *name) {
        std::string n(name);
        return getInstanceLogger(n);
    };

    rh_logger::LoggerPtr  getInstanceLogger(std::string &name) {
        return this->_baseLog->getChildLogger(name, "");
    };

    bool   bindToDomain() { return _bindToDomain; };
    
    std::string getRedhawkVersion() { return redhawk_version; };

    bool  strictSPDValidation() {  return _strict_spd_validation;  };

    uint32_t  getManagerWaitTime();
    uint32_t  getDeviceWaitTime();
    uint32_t  getServiceWaitTime();
    CF::LogLevel log_level();
    int getInitialLogLevel() {
        return _initialLogLevel;
    };

/////////////////////////////
// Internal Helper Functions
/////////////////////////////
protected:

    ossie::DeviceManagerList::iterator _local_unregisterDeviceManager (ossie::DeviceManagerList::iterator deviceManager);
    ossie::DeviceList::iterator _local_unregisterDevice (ossie::DeviceList::iterator device);
    ossie::ServiceList::iterator _local_unregisterService (ossie::ServiceList::iterator service);

    void _local_registerWithEventChannel (CORBA::Object_ptr registeringObject, const std::string& registeringId, const std::string& eventChannelName);
    void _local_unregisterFromEventChannel (const std::string& unregisteringId, const std::string& eventChannelName);

    void parseDMDProfile();
    void storeDeviceInDomainMgr (CF::Device_ptr, CF::DeviceManager_ptr);
    void storeServiceInDomainMgr (CORBA::Object_ptr, CF::DeviceManager_ptr, const std::string&, const std::string&);
    bool deviceMgrIsRegistered (CF::DeviceManager_ptr);
    bool domainMgrIsRegistered (CF::DomainManager_ptr);
    bool deviceIsRegistered (CF::Device_ptr);
    bool serviceIsRegistered (const std::string&);
    void addDeviceMgr (CF::DeviceManager_ptr deviceMgr);
    void mountDeviceMgrFileSys (CF::DeviceManager_ptr deviceMgr);
    void addDomainMgr (CF::DomainManager_ptr domainMgr);
    void catastrophicUnregisterDeviceManager (ossie::DeviceManagerList::iterator node);
    void removeDeviceManagerDevices (const std::string& deviceManagerId);
    void removeDeviceManagerServices (const std::string& deviceManagerId);
    ossie::DeviceManagerList::iterator removeDeviceManager (ossie::DeviceManagerList::iterator deviceManager);

    void cleanupDomainNamingContext (CosNaming::NamingContext_ptr nc);

    ossie::DeviceManagerList::iterator findDeviceManagerByObject (CF::DeviceManager_ptr deviceManager);
    ossie::DeviceManagerList::iterator findDeviceManagerById (const std::string& identifier);

    ossie::DomainManagerList::iterator findDomainManagerByObject (CF::DomainManager_ptr deviceManager);
    ossie::DomainManagerList::iterator findDomainManagerById (const std::string& identifier);

    ossie::DeviceList::iterator findDeviceByObject (CF::Device_ptr device);
    ossie::DeviceList::iterator findDeviceById (const std::string& identifier);

    ossie::ServiceList::iterator findServiceByName (const std::string& name);
    ossie::ServiceList::iterator findServiceByType (const std::string& repId);

    void parseDeviceProfile (ossie::DeviceNode& node);

    //
    // Events/Event Channel Management 
    //
    void establishDomainManagementChannels( const std::string &db_uri );
    void disconnectDomainManagementChannels();
    void idmTerminationMessages( const redhawk::events::ComponentTerminationEvent &msg );
    void destroyEventChannels (void);
    void storePubProxies();
    void storeSubProxies();
    void storeEventChannelRegistrations();
    void restorePubProxies(const std::string& _db_uri);
    void restoreSubProxies(const std::string& _db_uri);
    void restoreEventChannelRegistrations(const std::string& _db_uri);

    bool applicationDependsOnDevice (Application_impl* application, const std::string& deviceId);

    Application_impl* findApplicationById (const std::string& identifier);

/////////////////////////
// Protected Domain State
/////////////////////////
protected:
    ossie::PersistenceStore db;

    boost::recursive_mutex stateAccess;

    // The PersistenceStore is used to persist the state of the following members
    ossie::DomainManagerList _registeredDomainManagers;
    ossie::DeviceManagerList _registeredDeviceManagers;
    ossie::DeviceList _registeredDevices;
    std::set<std::string> _installedApplications;
    std::vector < ossie::ApplicationNode > _runningApplications;
    ossie::ServiceList _registeredServices;
    std::vector < ossie::EventChannelNode > _eventChannels;

    Application_impl* _restoreApplication(ossie::ApplicationNode& node);
    void _persistApplication(Application_impl* application);

    //
    // Handle to EventChannelManager for the Domain
    //
    friend class EventChannelManager;
    EventChannelManager*                 _eventChannelMgr;


    DOM_Publisher_ptr                    publisher( const std::string &cname );
    DOM_Subscriber_ptr                   subscriber( const std::string &cname );

    DOM_Publisher_ptr                    _odm_publisher;
    redhawk::events::DomainEventReader   _idm_reader;
    bool PERSISTENCE;

///////////////////////
// Private Domain State
///////////////////////
private:
    std::string _identifier;
    const std::string _domainName;
    const std::string _domainManagerProfile;

    PortableServer::POA_var poa;
    PortableServer::POA_var appFact_poa;

    CosNaming::Name_var base_context;
    
    CosNaming::NamingContext_var rootContext;

    ossie::DomainConnectionManager _connectionManager;

    friend class ConnectionManager_impl;
    ConnectionManager_impl* _connectionMgr;
    
    typedef std::map<std::string,Application_impl*> ApplicationTable;
    ApplicationTable _applications;
    ApplicationTable _pendingApplications;

    typedef std::map<std::string,ApplicationFactory_impl*> ApplicationFactoryTable;
    ApplicationFactoryTable _applicationFactories;

    // Identifier of last device that was successfully used for deployment
    std::string _lastDeviceUsedForDeployment;

    std::string      logging_config_uri;
    StringProperty*  logging_config_prop;
    CORBA::ULong     componentBindingTimeout;
    std::string      redhawk_version;
    bool             _useLogConfigUriResolver;
    bool             _strict_spd_validation;

    // orb context
    ossie::corba::OrbContext                         _orbCtx;

    void _exit(int __status) {
        ossie::logging::Terminate();            //no more logging....
        exit(__status);
    };
    FileManager_impl* fileMgr_servant;
    client_wait_times_struct   client_wait_times;

    int _initialLogLevel;
    bool             _bindToDomain;
};                                            /* END CLASS DEFINITION DomainManager */


#endif                                            /* __DOMAINMANAGER_IMPL__ */
