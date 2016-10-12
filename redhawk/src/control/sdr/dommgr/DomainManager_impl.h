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

#include <vector>
#include <string>

#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#if ENABLE_EVENTS
#include <COS/CosEventComm.hh>
#include <COS/CosEventChannelAdmin.hh>
#endif

#include <ossie/CF/cf.h>
#include <ossie/PropertySet_impl.h>
#include <ossie/DomainManagerConfiguration.h>
#include <ossie/Runnable.h>

#include "PersistanceStore.h"
#include "connectionSupport.h"

class Application_impl;

class DomainManager_impl: public virtual POA_CF::DomainManager, public PropertySet_impl, public ossie::ComponentLookup, public ossie::DomainLookup, public ossie::Runnable
{
    ENABLE_LOGGING

///////////////////////////
// Constructors/Destructors
///////////////////////////
public:
    DomainManager_impl (const char*, const char*, const char*, const char*);
    ~DomainManager_impl ();

    friend class IDM_Channel_Consumer_i;
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
        
    char * domainManagerProfile (void)
        throw (CORBA::SystemException);

    CF::FileManager_ptr fileMgr (void) throw (CORBA::SystemException);
    
    CF::DomainManager::ApplicationFactorySequence * applicationFactories (void) throw (CORBA::SystemException);
    
    CF::DomainManager::ApplicationSequence * applications (void) throw (CORBA::SystemException);
    
    CF::DomainManager::DeviceManagerSequence * deviceManagers (void) throw (CORBA::SystemException);
        
    void registerDevice (CF::Device_ptr registeringDevice, CF::DeviceManager_ptr registeredDeviceMgr)
        throw (CF::DomainManager::RegisterError, CF::DomainManager::DeviceManagerNotRegistered, CF::InvalidProfile, CF::InvalidObjectReference, CORBA::SystemException);
    void _local_registerDevice (CF::Device_ptr registeringDevice, CF::DeviceManager_ptr registeredDeviceMgr)
        throw (CF::DomainManager::RegisterError, CF::DomainManager::DeviceManagerNotRegistered, CF::InvalidProfile, CF::InvalidObjectReference, CORBA::SystemException);
        
    void registerDeviceManager (CF::DeviceManager_ptr deviceMgr)
        throw (CF::DomainManager::RegisterError, CF::InvalidProfile, CF::InvalidObjectReference, CORBA::SystemException);
    void _local_registerDeviceManager (CF::DeviceManager_ptr deviceMgr)
        throw (CF::DomainManager::RegisterError, CF::InvalidProfile, CF::InvalidObjectReference, CORBA::SystemException);
        
    void unregisterDeviceManager (CF::DeviceManager_ptr deviceMgr)
        throw (CF::DomainManager::UnregisterError, CF::InvalidObjectReference, CORBA::SystemException);
        
    void unregisterDevice (CF::Device_ptr unregisteringDevice)
        throw (CF::DomainManager::UnregisterError, CF::InvalidObjectReference, CORBA::SystemException);
        
    void installApplication (const char* profileFileName)
        throw (CF::DomainManager::ApplicationInstallationError, CF::InvalidFileName, CF::InvalidProfile, CORBA::SystemException, CF::DomainManager::ApplicationAlreadyInstalled);
    void _local_installApplication (const char* profileFileName)
        throw (CF::DomainManager::ApplicationInstallationError, CF::InvalidFileName, CF::InvalidProfile, CORBA::SystemException, CF::DomainManager::ApplicationAlreadyInstalled);
           
    void uninstallApplication (const char* applicationId)
        throw (CF::DomainManager::ApplicationUninstallationError, CF::DomainManager::InvalidIdentifier, CORBA::SystemException);
    void _local_uninstallApplication (const char* applicationId)
        throw (CF::DomainManager::ApplicationUninstallationError, CF::DomainManager::InvalidIdentifier, CORBA::SystemException);
           
    void registerService (CORBA::Object_ptr registeringService, CF::DeviceManager_ptr registeredDeviceMgr, const char* name)
        throw (CF::DomainManager::RegisterError, CF::DomainManager::DeviceManagerNotRegistered, CF::InvalidObjectReference, CORBA::SystemException);
    void _local_registerService (CORBA::Object_ptr registeringService, CF::DeviceManager_ptr registeredDeviceMgr, const char* name)
        throw (CF::DomainManager::RegisterError, CF::DomainManager::DeviceManagerNotRegistered, CF::InvalidObjectReference, CORBA::SystemException);
           
    void unregisterService (CORBA::Object_ptr unregisteringService, const char* name)
        throw (CF::DomainManager::UnregisterError, CF::InvalidObjectReference, CORBA::SystemException);
           
    void registerWithEventChannel (CORBA::Object_ptr registeringObject, const char* registeringId, const char* eventChannelName)
        throw (CF::DomainManager::AlreadyConnected, CF::DomainManager::InvalidEventChannelName, CF::InvalidObjectReference, CORBA::SystemException);
    void _local_registerWithEventChannel (CORBA::Object_ptr registeringObject, const char* registeringId, const char* eventChannelName)
        throw (CF::DomainManager::AlreadyConnected, CF::DomainManager::InvalidEventChannelName, CF::InvalidObjectReference, CORBA::SystemException);
           
    void unregisterFromEventChannel (const char* unregisteringId, const char* eventChannelName)
        throw (CF::DomainManager::NotConnected, CF::DomainManager::InvalidEventChannelName, CORBA::SystemException);
    void _local_unregisterFromEventChannel (const char* unregisteringId, const char* eventChannelName)
        throw (CF::DomainManager::NotConnected, CF::DomainManager::InvalidEventChannelName, CORBA::SystemException);
    
////////////////////////////////////////////////////////
// Public Helper Functions and Members
// Mostly used by ApplicationFactory and Application
////////////////////////////////////////////////////////
public:
    void shutdown(int signal);

    void restoreState(const std::string& _db_uri);

    void addApplication(Application_impl* new_app);
    
    void releaseAllApplications();
    
    void shutdownAllDeviceManagers();
    
    void removeApplication(std::string app_id);

    const std::string& getDomainManagerName (void) const {
        return _domainName;
    }

    const std::string& getFullDomainManagerName (void) const {
        return _fullDomainManagerName;
    }
    
    int getComponentBindingTimeout (void) const {
        return componentBindingTimeout;
    }

    ossie::DeviceList getRegisteredDevices(); // Get a copy of registered devices

    // DomainLookup methods
    CORBA::Object_ptr lookupDomainObject (const std::string& type, const std::string& name);

    // ComponentLookup methods
    CF::Resource_ptr lookupComponentByInstantiationId(const std::string& identifier);
    CF::DeviceManager_ptr lookupDeviceManagerByInstantiationId(const std::string& identifier);
#if ENABLE_EVENTS
    CosEventChannelAdmin::EventChannel_ptr lookupEventChannel(const std::string &EventChannelName);
    unsigned int incrementEventChannelConnections(const std::string &EventChannelName);
    unsigned int decrementEventChannelConnections(const std::string &EventChannelName);
#endif

#if ENABLE_EVENTS
    CosEventChannelAdmin::EventChannel_ptr getEventChannel(const std::string &name);
    bool eventChannelExists(const std::string &name);
    CosEventChannelAdmin::EventChannel_ptr createEventChannel (const std::string& name);
    void destroyEventChannel (const std::string& name);

    CosEventChannelAdmin::ProxyPushConsumer_var proxy_consumer;
    std::map<std::string, ossie::consumerContainer> registeredConsumers;
#endif

    CF::FileManager_var _fileMgr;

    std::string getLastDeviceUsedForDeployment();
    void setLastDeviceUsedForDeployment(const std::string& identifier);

/////////////////////////////
// Internal Helper Functions
/////////////////////////////
protected:

    ossie::DeviceManagerList::iterator _local_unregisterDeviceManager (ossie::DeviceManagerList::iterator deviceManager);
    ossie::DeviceList::iterator _local_unregisterDevice (ossie::DeviceList::iterator device);
    ossie::ServiceList::iterator _local_unregisterService (ossie::ServiceList::iterator service);

    void validate (const char* _profile);
    void validateSPD (const char* _spdProfile, int _cnt = 0) throw (CF::DomainManager::ApplicationInstallationError);
    void removeSPD (const char* _spdProfile, int _cnt = 0);
    void storeDeviceInDomainMgr (CF::Device_ptr, CF::DeviceManager_ptr);
    void storeServiceInDomainMgr (CORBA::Object_ptr, CF::DeviceManager_ptr, const char*, const char*);
    void getDeviceProperties (ossie::DeviceNode&);
    bool deviceMgrIsRegistered (CF::DeviceManager_ptr);
    bool deviceIsRegistered (CF::Device_ptr);
    bool serviceIsRegistered (const char*);
    void disconectEventService ();
    void sendEventToOutgoingChannel (CORBA::Any& _event);
    void addDeviceMgr (CF::DeviceManager_ptr deviceMgr);
    void addDeviceMgrDevices (CF::DeviceManager_ptr deviceMgr);
    void addDeviceMgrServices (CF::DeviceManager_ptr deviceMgr);
    void mountDeviceMgrFileSys (CF::DeviceManager_ptr deviceMgr);

    void catastrophicUnregisterDeviceManager (ossie::DeviceManagerList::iterator node);
    void removeDeviceManagerDevices (const std::string& deviceManagerId);
    void removeDeviceManagerServices (const std::string& deviceManagerId);
    ossie::DeviceManagerList::iterator removeDeviceManager (ossie::DeviceManagerList::iterator deviceManager);

    void cleanupDomainNamingContext (CosNaming::NamingContext_ptr nc);

    ossie::DeviceManagerList::iterator findDeviceManagerByObject (CF::DeviceManager_ptr deviceManager);
    ossie::DeviceManagerList::iterator findDeviceManagerById (const std::string& identifier);

    ossie::DeviceList::iterator findDeviceByObject (CF::Device_ptr device);
    ossie::DeviceList::iterator findDeviceById (const std::string& identifier);

    ossie::ServiceList::iterator findServiceByName (const std::string& name);
    ossie::ServiceList::iterator findServiceByType (const std::string& repId);

#if ENABLE_EVENTS
    void createEventChannels (void);
    void destroyEventChannels (void);
    void connectToOutgoingEventChannel (void);
    void connectToIncomingEventChannel (void);
#endif

/////////////////////////
// Protected Domain State
/////////////////////////
protected:
    ossie::PersistenceStore db;

    boost::recursive_mutex stateAccess;

    // The PersistenceStore is used to persist the state of the following members
    ossie::DeviceManagerList _registeredDeviceManagers;
    ossie::DeviceList _registeredDevices;
    std::vector < ossie::ApplicationFactoryNode > _installedApplications;
    std::vector < ossie::ApplicationNode > _runningApplications;
    ossie::ServiceList _registeredServices;
#if ENABLE_EVENTS
    std::vector < ossie::EventChannelNode > _eventChannels;
#endif

///////////////////////
// Private Domain State
///////////////////////
private:
    ossie::DomainManagerConfiguration _configuration;
    std::string _domainName;
    std::string _domainManagerProfile;

    std::string _fullDomainManagerName;
    
    PortableServer::POA_var poa;
    PortableServer::POA_var appFact_poa;

    CosNaming::Name_var base_context;
    
    CosNaming::NamingContext_var rootContext;

    ossie::DomainConnectionManager _connectionManager;
    
    CF::DomainManager::ApplicationSequence _applications;
    CF::DomainManager::ApplicationFactorySequence _applicationFactories;

    // Identifier of last device that was successfully used for deployment
    std::string _lastDeviceUsedForDeployment;

    std::string logging_config_uri;
    StringProperty* logging_config_prop;
    CORBA::ULong componentBindingTimeout;
    std::string redhawk_version;
};                                            /* END CLASS DEFINITION DomainManager */


#endif                                            /* __DOMAINMANAGER_IMPL__ */
