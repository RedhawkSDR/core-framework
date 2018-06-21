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


/* SCA */
/* Include files */

#include <ios>
#include <string>
#include <vector>
#include <signal.h>

#include <boost/foreach.hpp>

#include <omniORB4/CORBA.h>
#include <omniORB4/internal/orbParameters.h>
#include <ossie/debug.h>
#include <ossie/exceptions.h>
#include <ossie/FileManager_impl.h>
#include <ossie/SoftwareAssembly.h>
#include <ossie/CorbaUtils.h>
#include <ossie/EventChannelSupport.h>
#include <ossie/FileStream.h>
#include <ossie/ComponentDescriptor.h>
#include <ossie/DeviceManagerConfiguration.h>
#include <ossie/DomainManagerConfiguration.h>
#include <ossie/GCThread.h>
#include <ossie/EventTypes.h>
#include <ossie/prop_utils.h>

#include "Application_impl.h"
#include "ApplicationFactory_impl.h"
#include "DomainManager_impl.h"
#include "connectionSupport.h"
#include "AllocationManager_impl.h"
#include "EventChannelManager.h"
#include "ConnectionManager.h"

using namespace ossie;
using namespace std;

static const ComponentInstantiation* findComponentInstantiation (const std::vector<DevicePlacement>& placements,
                                                                 const std::string& identifier)
{
    for (std::vector<DevicePlacement>::const_iterator iter = placements.begin(); iter != placements.end(); ++iter) {
        const std::vector<ComponentInstantiation>& instantiations = iter->getInstantiations();
        for (std::vector<ComponentInstantiation>::const_iterator ii = instantiations.begin(); ii != instantiations.end(); ++ii) {
            if (identifier == ii->getID()) {
                return &(*ii);
            }
        }
    }
    return 0;
}

rh_logger::LoggerPtr DomainManager_impl::__logger;

// If _overrideDomainName == NULL read the domain name from the DMD file
DomainManager_impl::DomainManager_impl (const char* dmdFile, const char* _rootpath, const char* domainName, 
					const char *db_uri,
					const char* _logconfig_uri, bool useLogCfgResolver, bool bindToDomain, bool _persistence, int initialLogLevel) :
  Logging_impl("DomainManager"),
  _eventChannelMgr(NULL),
  _domainName(domainName),
  _domainManagerProfile(dmdFile),
  _connectionManager(this, this, domainName),
  _useLogConfigUriResolver(useLogCfgResolver),
  _strict_spd_validation(false),
  _initialLogLevel(initialLogLevel),
  _bindToDomain(bindToDomain)
{

    std::string std_logconfig_uri;
    if (_logconfig_uri) {
        std::string _lu(_logconfig_uri);
        std::string _rp(_rootpath);
        std_logconfig_uri = ossie::logging::ResolveLocalUri(_lu, _rp, _lu);
    }
    std::string expanded_config = getExpandedLogConfig(std_logconfig_uri);
    this->_baseLog->configureLogger(expanded_config, true);

    redhawk::setupParserLoggers(this->_baseLog);
    PropertySet_impl::setLogger(this->_baseLog->getChildLogger("PropertySet", ""));

    RH_TRACE(this->_baseLog, "Looking for DomainManager POA");
    _connectionManager.setLogger(this->_baseLog->getChildLogger("ConnectionManager", ""));
    poa = ossie::corba::RootPOA()->find_POA("DomainManager", 1);

    // Initialize properties
    logging_config_prop = (StringProperty*)addProperty(logging_config_uri, "LOGGING_CONFIG_URI", "LOGGING_CONFIG_URI",
                                                       "readonly", "", "external", "configure");

    if (_logconfig_uri) {
        logging_config_prop->setValue(_logconfig_uri);
    }

    addProperty(componentBindingTimeout, 60, "COMPONENT_BINDING_TIMEOUT", "component_binding_timeout",
                "readwrite", "seconds", "external", "configure");

    addProperty(redhawk_version, VERSION, "REDHAWK_VERSION", "redhawk_version",
                "readonly", "", "external", "configure");
    
    addProperty(PERSISTENCE,
                "PERSISTENCE",
                "",
                "readonly",
                "",
                "external",
                "property");
    
    PERSISTENCE = _persistence;

    addProperty(client_wait_times,
                client_wait_times_struct(),
                "client_wait_times",
                "client_wait_times",
                "readwrite",
                "",
                "external",
                "property");


    // Create file manager and register with the parent POA.
    fileMgr_servant = new FileManager_impl (_rootpath);
    std::string fileManagerId = _domainName + "/FileManager";
    PortableServer::ObjectId_var oid = ossie::corba::activatePersistentObject(poa, fileMgr_servant, fileManagerId);
    fileMgr_servant->_remove_ref();
    _fileMgr = fileMgr_servant->_this();
    fileMgr_servant->setLogger(_baseLog->getChildLogger("FileManager", ""));

    // Create allocation manager and register with the parent POA
    _allocationMgr = new AllocationManager_impl (this);
    std::string allocationManagerId = _domainName + "/AllocationManager";
    oid = ossie::corba::activatePersistentObject(poa, _allocationMgr, allocationManagerId);
    _allocationMgr->_remove_ref();
    _allocationMgr->setLogger(_baseLog->getChildLogger("AllocationManager", ""));

    ossie::proputilsLog = _baseLog->getChildLogger("proputils","");
    fileLog = _baseLog->getChildLogger("File","");
    redhawk::deploymentLog = _baseLog->getChildLogger("Deployment","");
    ossie::connectionSupportLog = _baseLog->getChildLogger("ConnectionSupport","");

    // Likewise, create the domain-level connection manager
    _connectionMgr = new ConnectionManager_impl(this);
    std::string connectionManagerId = _domainName + "/ConnectionManager";
    oid = ossie::corba::activatePersistentObject(poa, _connectionMgr, connectionManagerId);
    _connectionMgr->setLogger(_baseLog->getChildLogger("ConnectionManager", ""));

    // Parse the DMD profile
    parseDMDProfile();

    RH_TRACE(this->_baseLog, "Establishing domain manager naming context")
    base_context = ossie::corba::stringToName(_domainName);
    CosNaming::NamingContext_ptr inc = CosNaming::NamingContext::_nil();
    try {
        inc = ossie::corba::InitialNamingContext();
    } catch ( ... ) {
        RH_FATAL(this->_baseLog, "Unable to find Naming Service; make sure that it is configured correctly and running.");
        _exit(EXIT_FAILURE);
    }
    try {
        rootContext = inc->bind_new_context (base_context);
    } catch (CosNaming::NamingContext::AlreadyBound&) {
        RH_TRACE(this->_baseLog, "Naming context already exists");
        CORBA::Object_var obj = inc->resolve(base_context);
        rootContext = CosNaming::NamingContext::_narrow(obj);
        try {
            cleanupDomainNamingContext(rootContext);
        } catch (CORBA::Exception& e) {
            RH_FATAL(this->_baseLog, "Stopping domain manager; error cleaning up context for domain due to: " << e._name());
            _exit(EXIT_FAILURE);
        }
    } catch ( ... ) {
        RH_FATAL(this->_baseLog, "Stopping domain manager; error creating new context for domain " << this->_domainName);
        _exit(EXIT_FAILURE);
    }

    //
    // Setup EventChannelManager to support domain wide event channel registrations and use
    //
    try {
      // Create event channel  manager and register with the parent POA
      // RESOLVE -- need to add command line args to DomainManager to support EventChannel resolution
      _eventChannelMgr = new EventChannelManager(this, true, true, true);
      std::string id = _domainName + "/EventChannelManager";
      oid = ossie::corba::activatePersistentObject(poa, _eventChannelMgr, id );
      _allocationMgr->setLogger(_baseLog->getChildLogger("EventChannelManager", ""));
      _eventChannelMgr->_remove_ref();
      RH_DEBUG(this->_baseLog, "Started EventChannelManager for the domain.");
      // setup IDM and ODM Channels for this domain
      std::string dburi = (db_uri) ? db_uri : "";
      establishDomainManagementChannels( dburi );

    } catch ( ... ) {
        RH_FATAL(this->_baseLog, "Stopping domain manager; EventChannelManager - EventChannelFactory unavailable" )
        _exit(EXIT_FAILURE);
    }

// \todo lookup and install any services specified in the DMD

    RH_TRACE(this->_baseLog, "Looking for ApplicationFactories POA");
    appFact_poa = poa->find_POA("ApplicationFactories", 1);

    RH_TRACE(this->_baseLog, "Done instantiating Domain Manager")
}

CF::LogLevel DomainManager_impl::log_level() {
    int _level = this->_baseLog->getLevel()->toInt();
    if (_level == rh_logger::Level::OFF_INT)
        _level = CF::LogLevels::OFF;
    else if (_level == rh_logger::Level::ALL_INT)
        _level = CF::LogLevels::ALL;
    return _level;
}

void DomainManager_impl::parseDMDProfile()
{
    ossie::DomainManagerConfiguration configuration;
    try {
        File_stream dmdStream(_fileMgr, _domainManagerProfile.c_str());
        configuration.load(dmdStream);
        dmdStream.close();
    } catch (const parser_error& e) {
        std::string parser_error_line = ossie::retrieveParserErrorLineNumber(e.what());
        RH_FATAL(this->_baseLog, "Stopping domain manager; error parsing domain manager configuration DMD: " <<  _domainManagerProfile << ". " << parser_error_line << " The XML parser returned the following error: " << e.what())
        _exit(EXIT_FAILURE);
    } catch (const std::ios_base::failure& e) {
        RH_FATAL(this->_baseLog, "Stopping domain manager;  domain manager configuration DMD: " <<  _domainManagerProfile << " IO failure exception: " << e.what())
        _exit(EXIT_FAILURE);
    } catch( CF::InvalidFileName& _ex ) {
        RH_FATAL(this->_baseLog, "Stopping domain manager;  domain manager configuration DMD: " <<  _domainManagerProfile << " Invalid file name exception: " << _ex.msg)
        _exit(EXIT_FAILURE);
    } catch( CF::FileException& _ex ) {
        RH_FATAL(this->_baseLog, "Stopping domain manager; domain manager configuration DMD: " <<  _domainManagerProfile << " File exception: " << _ex.msg)
        _exit(EXIT_FAILURE);
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while loading domain manager configuration DMD: " << _domainManagerProfile;
        RH_FATAL(this->_baseLog, eout.str())
        _exit(EXIT_FAILURE);
    } catch ( CORBA::Exception& ex ) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while loading domain manager configuration DMD: " << _domainManagerProfile;
        RH_FATAL(this->_baseLog, eout.str())
        _exit(EXIT_FAILURE);
    }

    _identifier = configuration.getID();
}


void DomainManager_impl::restoreEventChannels(const std::string& _db_uri) {
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    RH_INFO(this->_baseLog, "Restoring state from URL " << _db_uri);
    try {
        db.open(_db_uri);
    } catch (const ossie::PersistenceException& e) {
        RH_ERROR(this->_baseLog, "Error loading persistent state: " << e.what());
        return;
    }

    RH_TRACE(this->_baseLog, "Recovering event channels");
    // Recover the event channels
    std::vector<EventChannelNode> _restoredEventChannels;
    try {
        db.fetch("EVENT_CHANNELS", _restoredEventChannels, true);
    } catch (const ossie::PersistenceException& e) {
        RH_ERROR(this->_baseLog, "Error loading event channels persistent state: " << e.what());
        _restoredEventChannels.clear();
    }

    for (std::vector<EventChannelNode>::iterator i = _restoredEventChannels.begin();
         i != _restoredEventChannels.end();
         ++i) {
        RH_TRACE(this->_baseLog, "Attempting to recover connection to Event Channel " << i->boundName);
        try {
            if (ossie::corba::objectExists(i->channel)) {
                RH_INFO(this->_baseLog, "Recovered connection to Event Channel: " << i->boundName);
                
                // try to restore channel with event channel manager..
                try {
                  if ( _eventChannelMgr ) _eventChannelMgr->restore( i->channel, i->name, i->boundName);
                }
                catch( CF::EventChannelManager::ChannelAlreadyExists){
                  RH_INFO(this->_baseLog, "EventChannelManager::restore, Channel already exists: " << i->boundName);
                }
                catch( CF::EventChannelManager::InvalidChannelName){
                  RH_WARN(this->_baseLog, "EventChannelManager::restore, Invalid Channel Name, " << i->boundName);
                }
                catch( CF::EventChannelManager::ServiceUnavailable){
                  RH_WARN(this->_baseLog, "EventChannelManager::restore, Event Service seems to be down. ");
                }
                catch( CF::EventChannelManager::OperationFailed){
                  RH_WARN(this->_baseLog, "EventChannelManager::restore, Failed to recover Event Channel: " << i->boundName);
                }
                catch( ... ){
                  RH_WARN(this->_baseLog, "EventChannelManager, Failed to recover Event Channel: " << i->boundName);
                }
                CosEventChannelAdmin::EventChannel_var channel = i->channel;
                CosNaming::Name_var cosName = ossie::corba::stringToName(i->boundName);
                try {
                    // Use rebind to force the naming service to replace any existing object with the same name.
                    rootContext->rebind(cosName, channel);
                } catch ( ... ) {
                    channel = ossie::events::connectToEventChannel(rootContext, i->boundName);
                }
                bool foundEventChannel = false;
                for (std::vector<EventChannelNode>::iterator j=_eventChannels.begin(); j!=_eventChannels.end(); j++) {
                    if ((*j).boundName == i->boundName) {
                        (*j).connectionCount = i->connectionCount;
                        foundEventChannel = true;
                        break;
                    }
                }
                if (!foundEventChannel) {
                    _eventChannels.push_back(*i);
                }
            } else {
                RH_WARN(this->_baseLog, "Failed to recover Event Channel: " << i->boundName);
            }
        } CATCH_RH_WARN(this->_baseLog, "Unable to restore connection to Event Channel: " << i->boundName);
    }

    try {
        db.store("EVENT_CHANNELS", _restoredEventChannels);
    } catch (const ossie::PersistenceException& e) {
        RH_ERROR(this->_baseLog, "Error restoring event channels from persistent state: " << e.what());
    }
}

void DomainManager_impl::restoreState(const std::string& _db_uri) {
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    RH_INFO(this->_baseLog, "Restoring state from URL " << _db_uri);
    try {
        db.open(_db_uri);
    } catch (const ossie::PersistenceException& e) {
        RH_ERROR(this->_baseLog, "Error loading persistent state: " << e.what());
        return;
    }

    RH_DEBUG(this->_baseLog, "Recovering device manager connections");
    // Recover device manager connections and consume the value so that
    // the persistence store no longer has any device manager stored
    DeviceManagerList _restoredDeviceManagers;
    try {
        db.fetch("DEVICE_MANAGERS", _restoredDeviceManagers, true);
    } catch (const ossie::PersistenceException& e) {
        RH_ERROR(this->_baseLog, "Error loading device managers persistent state: " << e.what());
        _restoredDeviceManagers.clear();
    }

    for (DeviceManagerList::iterator ii = _restoredDeviceManagers.begin(); ii != _restoredDeviceManagers.end(); ++ii) {
        RH_TRACE(this->_baseLog, "Attempting to recover connection to Device Manager " << ii->identifier << " " << ii->label);
        try {
            if (ossie::corba::objectExists(ii->deviceManager)) {
                RH_INFO(this->_baseLog, "Recovered connection to Device Manager: " << ii->identifier << " " << ii->label);
                addDeviceMgr(ii->deviceManager);
                mountDeviceMgrFileSys(ii->deviceManager);
            } else {
                RH_WARN(this->_baseLog, "Failed to recover connection to Device Manager: " << ii->label << ": device manager servant no longer exists");
            }
        } CATCH_RH_WARN(this->_baseLog, "Unable to restore connection to DeviceManager: " << ii->label);
    }

    RH_DEBUG(this->_baseLog, "Recovering device connections");
    DeviceList _restoredDevices;
    try {
        db.fetch("DEVICES", _restoredDevices, true);
    } catch (const ossie::PersistenceException& e) {
        RH_ERROR(this->_baseLog, "Error loading devices persistent state: " << e.what());
        _restoredDevices.clear();
    }

    for (DeviceList::iterator iter = _restoredDevices.begin(); iter != _restoredDevices.end(); ++iter) {
        boost::shared_ptr<DeviceNode> i = *iter;
        RH_TRACE(this->_baseLog, "Attempting to recover connection to Device " << i->identifier << " " << i->label);
        try {
            if (ossie::corba::objectExists(i->device)) {
                RH_INFO(this->_baseLog, "Recovered connection to Device: " << i->identifier << " " << i->label);
                if (ossie::corba::objectExists(i->devMgr.deviceManager)) {
                    storeDeviceInDomainMgr(i->device, i->devMgr.deviceManager);
                } else {
                    RH_WARN(this->_baseLog, "Failed to recover connection to Device: " << i->identifier << ": device manager no longer exists");
                }
            } else {
                RH_WARN(this->_baseLog, "Failed to recover connection to Device: " << i->identifier << ": device servant no longer exists");
            }
        } CATCH_RH_WARN(this->_baseLog, "Unable to restore connection to Device: " << i->identifier);
    }

    RH_DEBUG(this->_baseLog, "Recovering registered services");
    ServiceList _restoredServices;
    try {
        db.fetch("SERVICES", _restoredServices, true);
    } catch (const ossie::PersistenceException& e) {
        RH_ERROR(this->_baseLog, "Error loading services persistent state: " << e.what());
        _restoredServices.clear();
    }

    for (ServiceList::iterator ii = _restoredServices.begin(); ii != _restoredServices.end(); ++ii) {
        RH_TRACE(this->_baseLog, "Attempting to recover connection to Service " << ii->name);
        try {
            if (ossie::corba::objectExists(ii->service)) {
                RH_INFO(this->_baseLog, "Recovered connection to Service: " << ii->name);
                ossie::DeviceManagerList::iterator deviceManager = findDeviceManagerById(ii->deviceManagerId);
                if (deviceManager != _registeredDeviceManagers.end()) {
                    storeServiceInDomainMgr(ii->service, deviceManager->deviceManager, ii->name, ii->serviceId);
                } else {
                    RH_WARN(this->_baseLog, "Failed to recover connection to Service: " << ii->name << ": DeviceManager "
                             << ii->deviceManagerId << " no longer exists");
                }
            } else {
                RH_WARN(this->_baseLog, "Failed to recover connection to Service: " << ii->name << ": servant no longer exists");
            }
        } CATCH_RH_WARN(this->_baseLog, "Unable to restore connection to Service: " << ii->name);
    }

    RH_DEBUG(this->_baseLog, "Recovering DCD connections");
    ConnectionTable _restoredConnections;
    try {
        db.fetch("CONNECTIONS", _restoredConnections, true);
    } catch (const ossie::PersistenceException& e) {
        RH_ERROR(this->_baseLog, "Error loading services persistent state: " << e.what());
        _restoredConnections.clear();
    }

    for (ConnectionTable::iterator ii = _restoredConnections.begin(); ii != _restoredConnections.end(); ++ii) {
        const std::string& deviceManagerId = ii->first;
        const ConnectionList& connections = ii->second;
        RH_TRACE(this->_baseLog, "Restoring port connections for DeviceManager " << deviceManagerId);
        for (ConnectionList::const_iterator jj = connections.begin(); jj != connections.end(); ++jj) {
            RH_TRACE(this->_baseLog, "Restoring port connection " << jj->identifier);
            _connectionManager.restoreConnection(deviceManagerId, *jj);
        }
    }

    RH_DEBUG(this->_baseLog, "Recovering application factories");
    std::set<std::string> restoredSADs;
    try {
        db.fetch("APP_FACTORIES", restoredSADs, true);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error loading application factory persistent state: " << ex.what());
        restoredSADs.clear();
    }

    for (std::set<std::string>::iterator profile = restoredSADs.begin(); profile != restoredSADs.end(); ++profile) {
        RH_TRACE(this->_baseLog, "Attempting to restore application factory " << *profile);
        try {
            _local_installApplication(*profile);
            RH_INFO(this->_baseLog, "Restored application factory " << *profile);
        } CATCH_RH_WARN(this->_baseLog, "Failed to restore application factory " << *profile);
    }

    RH_DEBUG(this->_baseLog, "Recovering applications");
    std::vector<ApplicationNode> _restoredApplications;
    try {
        db.fetch("APPLICATIONS", _restoredApplications, true);
    } catch (const ossie::PersistenceException& e) {
        RH_ERROR(this->_baseLog, "Error loading application persistent state: " << e.what());
        _restoredApplications.clear();
    }

    for (std::vector<ApplicationNode>::iterator i = _restoredApplications.begin();
         i != _restoredApplications.end();
         ++i) {
        RH_TRACE(this->_baseLog, "Attempting to restore application  " << i->name << " " << i->identifier << " " << i->profile);
        try {
            if (ossie::corba::objectExists(i->context)) {
                RH_TRACE(this->_baseLog, "Creating application " << i->identifier << " " << _domainName << " " << i->contextName);
                Application_impl* application = _restoreApplication(*i);
                Application_impl::Activate(application);
                addApplication(application);
                application->_remove_ref();

                RH_INFO(this->_baseLog, "Restored application " << application->getIdentifier());
            }
        } CATCH_RH_WARN(this->_baseLog, "Failed to restore application" << i->identifier);
    }

    RH_DEBUG(this->_baseLog, "Recovering remote domains");
    // Recover domain manager connections and consume the value so that
    // the persistence store no longer has any domain manager stored
    DomainManagerList _restoredDomainManagers;
    try {
        db.fetch("DOMAIN_MANAGERS", _restoredDomainManagers, true);
    } catch (const ossie::PersistenceException& e) {
        RH_ERROR(this->_baseLog, "Error loading domain managers persistent state: " << e.what());
        _restoredDomainManagers.clear();
    }

    for (DomainManagerList::iterator ii = _restoredDomainManagers.begin(); ii != _restoredDomainManagers.end(); ++ii) {
        RH_TRACE(this->_baseLog, "Attempting to recover connection to domain '" << ii->name << "'");
        try {
            if (ossie::corba::objectExists(ii->domainManager)) {
                RH_INFO(this->_baseLog, "Recovered connection to domain '" << ii->name << "'");
                addDomainMgr(ii->domainManager);
            } else {
                RH_WARN(this->_baseLog, "Failed to recover connection to domain '" << ii->name << "': domain manager object no longer exists");
            }
        } CATCH_RH_WARN(this->_baseLog, "Unable to restore connection to domain '" << ii->name << "'");
    }

    RH_DEBUG(this->_baseLog, "Recovering allocation manager");
    ossie::AllocationTable _restoredLocalAllocations;
    try {
        db.fetch("LOCAL_ALLOCATIONS", _restoredLocalAllocations);
    } catch (const ossie::PersistenceException& e) {
        RH_ERROR(this->_baseLog, "Error loading local allocation persistent state: " << e.what());
    }
    _allocationMgr->restoreLocalAllocations(_restoredLocalAllocations);

    ossie::RemoteAllocationTable _restoredRemoteAllocations;
    try {
        db.fetch("REMOTE_ALLOCATIONS", _restoredRemoteAllocations);
    } catch (const ossie::PersistenceException& e) {
        RH_ERROR(this->_baseLog, "Error loading remote allocations persistent state: " << e.what());
    }
    _allocationMgr->restoreRemoteAllocations(_restoredRemoteAllocations);

    RH_DEBUG(this->_baseLog, "Done restoring state from URL " << _db_uri);
}

void DomainManager_impl::cleanupDomainNamingContext (CosNaming::NamingContext_ptr nc)
{
    CosNaming::BindingIterator_var it;
    CosNaming::BindingList_var bl;
    const CORBA::ULong CHUNK = 100;

    nc->list(CHUNK, bl, it);

    for (unsigned int ii = 0; ii < bl->length(); ++ii) {
        if (bl[ii].binding_type == CosNaming::ncontext) {
            CORBA::Object_var obj = nc->resolve(bl[ii].binding_name);
            CosNaming::NamingContext_var new_context = CosNaming::NamingContext::_narrow(obj);
            ossie::corba::overrideBlockingCall(obj);
            if (obj->_non_existent()) {
                // If it no longer exists, unbind it
                RH_TRACE(this->_baseLog, "Unbinding naming context which no longer exists; this is probably due to an omniNames bug")
                nc->unbind(bl[ii].binding_name);
            } else {
                cleanupDomainNamingContext(new_context);
                // Is the naming context empty?
                CosNaming::BindingIterator_var _it;
                CosNaming::BindingList_var _bl;
                new_context->list(CHUNK, _bl, _it);
                // If it's empty, then delete it
                if (_bl->length() == 0) {
                    nc->unbind(bl[ii].binding_name);
                }
            }
        } else if (bl[ii].binding_type == CosNaming::nobject) {
            // Clean up defunct servers bound to the naming service
            CORBA::Object_var obj = nc->resolve(bl[ii].binding_name);
            bool _unbind = false;
            try {
                ossie::corba::overrideBlockingCall(obj);
                if (obj->_non_existent()) {
                    _unbind = true;
                }
            } catch ( ... ) {
                _unbind = true;
            }
            if (_unbind) {
                try {
                    nc->unbind(bl[ii].binding_name);
                } catch ( ... ) {
                }
            }
        }
    }
}

void DomainManager_impl::releaseAllApplications()
{
    // Clear installed application list and update persistence store
    _installedApplications.clear();
    try {
        db.store("APP_FACTORIES", _installedApplications);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to installed applications");
    }

    std::vector<Application_impl*> releasedApps;
    for (ApplicationTable::iterator iter = _applications.begin(); iter != _applications.end(); ++iter) {
        Application_impl* app = iter->second;
        app->_add_ref();
        releasedApps.push_back(app);
    }

    for (std::vector<Application_impl*>::iterator app = releasedApps.begin(); app != releasedApps.end(); ++app) {
        try {
            (*app)->releaseObject();
        } catch ( ... ) {
            RH_TRACE(this->_baseLog, "Error releasing application " << (*app)->getName());
        }
        (*app)->_remove_ref();
    }
}


void DomainManager_impl::shutdownAllDeviceManagers()
{
    while (_registeredDeviceManagers.size() > 0) {
        unsigned int lenRegDevMgr = _registeredDeviceManagers.size();
        CF::DeviceManager_var devMgr = (*_registeredDeviceManagers.begin()).deviceManager;
        std::string dm_label = (*_registeredDeviceManagers.begin()).label;
        try {
            if ( !CORBA::is_nil(devMgr)  ) {
                devMgr->shutdown();
            }
            else {
                RH_WARN(this->_baseLog, "A DeviceManager: " << dm_label << ",  reference is empty, possible lingering process after shutdown completes.");
            }
        } catch ( std::exception& ex ) {
            RH_ERROR(this->_baseLog, "The following standard exception occurred: "<<ex.what()<<" while attempting to shutdown a DeviceManager : " << dm_label << ", continuing shutdown process.");
            if (lenRegDevMgr == _registeredDeviceManagers.size()) {
                _registeredDeviceManagers.erase(_registeredDeviceManagers.begin());
            }
        } catch( const CORBA::Exception& e ) {
            RH_ERROR(this->_baseLog, "DeviceManager: " << dm_label << " failed shutdown, continuing shutdown process. Exception: " << e._name());
            if (lenRegDevMgr == _registeredDeviceManagers.size()) {
                _registeredDeviceManagers.erase(_registeredDeviceManagers.begin());
            }
        } catch ( ... ) {
            if (lenRegDevMgr == _registeredDeviceManagers.size()) {
                _registeredDeviceManagers.erase(_registeredDeviceManagers.begin());
            }
            RH_ERROR(this->_baseLog, "Error shutting down Device Manager: " << dm_label << ", an unknown exception occurred." );
        }
    }
}


void DomainManager_impl::shutdown (int signal)
{
    RH_DEBUG(this->_baseLog, "Shutdown: signal=" << signal);

    if (!ossie::corba::isPersistenceEnabled() || (ossie::corba::isPersistenceEnabled()and (signal == SIGINT)) ) {
        releaseAllApplications();
        shutdownAllDeviceManagers();
        destroyEventChannels();
    }
    else {
      disconnectDomainManagementChannels();      
    }

    ossie::GCThread::shutdown();

    boost::recursive_mutex::scoped_lock lock(stateAccess);
    db.close();

    // stop any incoming requests while we shut down....
    PortableServer::POAManager_var mgr = ossie::corba::RootPOA()->the_POAManager();
    mgr->discard_requests(0);

    PortableServer::ObjectId_var oid;

    for (ApplicationFactoryTable::iterator iter = _applicationFactories.begin();
         iter != _applicationFactories.end(); ++iter) {
        oid = appFact_poa->servant_to_id(iter->second);
        appFact_poa->deactivate_object(oid);
        iter->second->_remove_ref();
    }
    appFact_poa->destroy(false, true);

    // Deactivate and destroy the ConnectionManager
    oid = poa->servant_to_id(_connectionMgr);
    poa->deactivate_object(oid);
    _connectionMgr->_remove_ref();

    // Deactivate and destroy the EventChannelManager
    if ( _eventChannelMgr ) {
      oid = poa->servant_to_id(_eventChannelMgr);
      poa->deactivate_object(oid);
    }

    // Deactivate and destroy the AllocationManager
    oid = poa->servant_to_id(_allocationMgr);
    poa->deactivate_object(oid);

    oid = poa->reference_to_id(_fileMgr);
    poa->deactivate_object(oid);

    // The domain manager does not eliminate the naming context that it was using unless it's empty. The assumption
    //  is that it can be re-started and anything that was running before re-associated
    if ( signal != -1 ) {
      try {
        CosNaming::Name_var DomainName = ossie::corba::stringToName(_domainName);
        rootContext->unbind(DomainName);
        unsigned int number_items = ossie::corba::numberBoundObjectsToContext(rootContext);
        if (number_items == 0) {
          base_context = ossie::corba::stringToName(_domainName);
          CosNaming::NamingContext_ptr inc = ossie::corba::InitialNamingContext();
          inc->unbind(base_context);
        }
      } catch ( ... ) {
      }
    }
}


DomainManager_impl::~DomainManager_impl ()
{
    // Don't make CORBA calls in the destructor because
    // the servant is being deleted after the ORB
    // has shutdown in the nodebooter


    /**************************************************
     *    Save current state for configuration recall   *
     *    this is not supported by this version          *
     **************************************************/
}

uint32_t  DomainManager_impl::getManagerWaitTime() {
    return client_wait_times.managers;
}
uint32_t  DomainManager_impl::getDeviceWaitTime() {
    return client_wait_times.devices;
}
uint32_t  DomainManager_impl::getServiceWaitTime() {
    return client_wait_times.services;
}

char *
DomainManager_impl::identifier (void)
throw (CORBA::SystemException)
{
    return CORBA::string_dup(_identifier.c_str());
}

char *
DomainManager_impl::name (void)
throw (CORBA::SystemException)
{
    return CORBA::string_dup(this->_domainName.c_str());
}


char *
DomainManager_impl::domainManagerProfile (void)
throw (CORBA::SystemException)
{
    return CORBA::string_dup(_domainManagerProfile.c_str());
}


CF::AllocationManager_ptr DomainManager_impl::allocationMgr (void) throw (CORBA::
                                                              SystemException)
{
    return _allocationMgr->_this();
}


CF::EventChannelManager_ptr DomainManager_impl::eventChannelMgr (void) throw (CORBA::
                                                              SystemException)
{
    return _eventChannelMgr->_this();
}

namespace {
    template <class Sequence, class Iterator>
    void map_to_sequence(Sequence& out, Iterator begin, const Iterator end)
    {
        for (; begin != end; ++begin) {
            ossie::corba::push_back(out, begin->second->_this());
        }
    }

    template <class Sequence, class Container>
    void map_to_sequence(Sequence& out, Container& in)
    {
        map_to_sequence(out, in.begin(), in.end());
    }
}


CF::FileManager_ptr DomainManager_impl::fileMgr (void) throw (CORBA::
SystemException)
{
    return CF::FileManager::_duplicate(_fileMgr);
}


CF::ConnectionManager_ptr DomainManager_impl::connectionMgr (void) throw (CORBA::SystemException)
{
    return _connectionMgr->_this();
}


CF::DomainManager::ApplicationFactorySequence *
DomainManager_impl::applicationFactories (void) throw (CORBA::
                                                       SystemException)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    CF::DomainManager::ApplicationFactorySequence_var result = new CF::DomainManager::ApplicationFactorySequence();
    map_to_sequence(result, _applicationFactories);

    return result._retn();
}


CF::DomainManager::ApplicationSequence *
DomainManager_impl::applications (void) throw (CORBA::SystemException)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    CF::DomainManager::ApplicationSequence_var result = new CF::DomainManager::ApplicationSequence();
    map_to_sequence(result, _applications);

    return result._retn();
}


CF::DomainManager::DeviceManagerSequence *
DomainManager_impl::deviceManagers (void) throw (CORBA::SystemException)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    CF::DomainManager::DeviceManagerSequence_var result = new CF::DomainManager::DeviceManagerSequence();
    result->length(_registeredDeviceManagers.size());
    DeviceManagerList::iterator deviceManager = _registeredDeviceManagers.begin();
    for (CORBA::ULong ii = 0; ii < result->length(); ++ii, ++deviceManager) {
        result[ii] = CF::DeviceManager::_duplicate(deviceManager->deviceManager);
    }

    return result._retn();
}


CF::DomainManager::DomainManagerSequence *
DomainManager_impl::remoteDomainManagers (void) throw (CORBA::SystemException)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    CF::DomainManager::DomainManagerSequence_var result = new CF::DomainManager::DomainManagerSequence();
    result->length(this->_registeredDomainManagers.size());
    DomainManagerList::iterator dmnMgr = this->_registeredDomainManagers.begin();
    for (CORBA::ULong ii = 0; ii < result->length(); ++ii, ++dmnMgr) {
        result[ii] = CF::DomainManager::_duplicate(dmnMgr->domainManager);
    }

    return result._retn();
}

ossie::DomainManagerList::iterator DomainManager_impl::findDomainManagerByObject (CF::DomainManager_ptr domainManager)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    try {
        std::string identifier = ossie::corba::returnString(domainManager->identifier());
        return findDomainManagerById(identifier);
    } catch (...) {
        // DeviceManager is not reachable for some reason, try checking by object.
    }

    DomainManagerList::iterator node;
    for (node = _registeredDomainManagers.begin(); node != _registeredDomainManagers.end(); ++node) {
        if (domainManager->_is_equivalent(node->domainManager)) {
            break;
        }
    }

    return node;
}

ossie::DomainManagerList::iterator DomainManager_impl::findDomainManagerById (const std::string& identifier)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    DomainManagerList::iterator node;
    for (node = _registeredDomainManagers.begin(); node != _registeredDomainManagers.end(); ++node) {
        if (identifier == node->identifier) {
            break;
        }
    }

    return node;
}

void
DomainManager_impl::registerRemoteDomainManager (CF::DomainManager_ptr domainMgr)
throw (CORBA::SystemException, CF::InvalidObjectReference,
       CF::DomainManager::RegisterError)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    try {
        ossie::corba::overrideBlockingCall(domainMgr, getManagerWaitTime());
        std::string identifier = ossie::corba::returnString(domainMgr->identifier());
        DomainManagerList::iterator node = findDomainManagerById(identifier);
        if (node != _registeredDomainManagers.end()) {
            if (!ossie::corba::objectExists(node->domainManager)) {
                RH_WARN(this->_baseLog, "Cleaning up registration of dead device manager: " << identifier);
                //catastrophicUnregisterDeviceManager(node);
                RH_TRACE(this->_baseLog, "Continuing with registration of new device manager: " << identifier);
            } else {
                bool DomMgr_alive = false;
                try {
                    CORBA::String_var identifier = domainMgr->identifier();
                    DomMgr_alive = true;
                } catch ( ... ) {
                    RH_WARN(this->_baseLog, "Cleaning up registration of dead device manager: " << identifier);
                    //catastrophicUnregisterDeviceManager(node);
                    RH_TRACE(this->_baseLog, "Continuing with registration of new device manager: " << identifier);
                }
                if (DomMgr_alive) {
                    ostringstream eout;
                    eout << "Attempt re-register existing domain manager: " << identifier;
                    RH_ERROR(this->_baseLog, eout.str());
                    throw CF::DomainManager::RegisterError(CF::CF_NOTSET, eout.str().c_str());
                }
            }
        }

        addDomainMgr (domainMgr);
    } catch ( ... ) {
        throw;
    }
}

void
DomainManager_impl::unregisterRemoteDomainManager (CF::DomainManager_ptr domainMgr)
throw (CORBA::SystemException, CF::InvalidObjectReference,
       CF::DomainManager::UnregisterError)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    if (CORBA::is_nil(domainMgr)) {
        RH_ERROR(this->_baseLog, "Cannot unregister nil DomainManager");
        throw CF::InvalidObjectReference("Cannot unregister nil DomainManager");
    }

    DomainManagerList::iterator domMgrIter = findDomainManagerByObject(domainMgr);
    if (domMgrIter == _registeredDomainManagers.end()) {
        RH_WARN(this->_baseLog, "Ignoring attempt to unregister domain manager that was not registered with this domain");
        return;
    }

    if (!domMgrIter->domainManager->_is_equivalent(domainMgr)) {
        RH_TRACE(this->_baseLog, "Ignoring attempt to unregister domain manager with same identifier but different object");
        return;
    }

    _registeredDomainManagers.erase(domMgrIter);
    try {
        db.store("DOMAIN_MANAGERS", _registeredDomainManagers);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to domain managers");
    }
}


void
DomainManager_impl::registerDeviceManager (CF::DeviceManager_ptr deviceMgr)
throw (CORBA::SystemException, CF::InvalidObjectReference, CF::InvalidProfile,
       CF::DomainManager::RegisterError)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    ossie::corba::overrideBlockingCall(deviceMgr,getManagerWaitTime());
    _local_registerDeviceManager(deviceMgr);

    std::string identifier = ossie::corba::returnString(deviceMgr->identifier());
    std::string label = ossie::corba::returnString(deviceMgr->label());
    sendAddEvent(_identifier, identifier, label, deviceMgr, StandardEvent::DEVICE_MANAGER);
}

void DomainManager_impl::_local_registerDeviceManager (CF::DeviceManager_ptr deviceMgr)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    if (CORBA::is_nil (deviceMgr)) {
        throw (CF::InvalidObjectReference("Cannot register DeviceManager. It is a nil reference."));
    }

    // Per the specification two conditions can arise if the device manager is already registered
    //  1. If the registered device manager refers to an existing object:
    //        - Return without exception and NOT register a new device manager
    //  2. The registered device manager refers to a non-existent object
    //        - Register the new deviceMgr
    std::string identifier = ossie::corba::returnString(deviceMgr->identifier());

    DeviceManagerList::iterator node = findDeviceManagerById(identifier);
    if (node != _registeredDeviceManagers.end()) {
        if (!ossie::corba::objectExists(node->deviceManager)) {
            RH_WARN(this->_baseLog, "Cleaning up registration of dead device manager: " << identifier);
            catastrophicUnregisterDeviceManager(node);
            RH_TRACE(this->_baseLog, "Continuing with registration of new device manager: " << identifier);
        } else {
            bool DevMgr_alive = false;
            try {
                CORBA::String_var identifier = deviceMgr->identifier();
                DevMgr_alive = true;
            } catch ( ... ) {
                RH_WARN(this->_baseLog, "Cleaning up registration of dead device manager: " << identifier);
                catastrophicUnregisterDeviceManager(node);
                RH_TRACE(this->_baseLog, "Continuing with registration of new device manager: " << identifier);
            }
            if (DevMgr_alive) {
                ostringstream eout;
                eout << "Attempt re-register existing device manager: " << identifier;
                RH_ERROR(this->_baseLog, eout.str());
                throw CF::DomainManager::RegisterError(CF::CF_NOTSET, eout.str().c_str());
            }
        }
    }

    addDeviceMgr (deviceMgr);
    node = findDeviceManagerById(identifier);
    CORBA::String_var devMgrLabel;
    try {
        mountDeviceMgrFileSys(deviceMgr);

        RH_TRACE(this->_baseLog, "Getting connections from DeviceManager DCD");
        DeviceManagerConfiguration *dcdParser;
        DeviceManagerConfiguration _dcdParser;
        try {
            CF::FileSystem_var devMgrFileSys = deviceMgr->fileSys();
            CORBA::String_var profile = deviceMgr->deviceConfigurationProfile();
            if ( node != _registeredDeviceManagers.end() ) {
                if ( node->dcd.isLoaded() == false ) {
                    File_stream dcd(devMgrFileSys, profile);
                    node->dcd.load(dcd);
                    dcd.close();            
                }
                dcdParser = &(node->dcd);
            }
            else {
                File_stream dcd(devMgrFileSys, profile);
                _dcdParser.load(dcd);
                dcdParser = &_dcdParser;
                dcd.close();
            }
        } catch ( ossie::parser_error& e ) {
            std::string parser_error_line = ossie::retrieveParserErrorLineNumber(e.what());
            RH_ERROR(this->_baseLog, "Failed device manager registration; error parsing device manager DCD: " << deviceMgr->deviceConfigurationProfile() << ". " << parser_error_line << " The XML parser returned the following error: " << e.what())
            throw(CF::DomainManager::RegisterError());
        }

        const std::vector<Connection>& connections = dcdParser->getConnections();

        for (size_t ii = 0; ii < connections.size(); ++ii) {
            try {
                _connectionManager.addConnection(dcdParser->getName(), connections[ii]);
            } catch (const ossie::InvalidConnection& ex) {
                RH_ERROR(this->_baseLog, "Ignoring unresolvable connection: " << ex.what());
            }
        }
        try {
            db.store("CONNECTIONS", _connectionManager.getConnections());
        } catch (const ossie::PersistenceException& ex) {
            RH_ERROR(this->_baseLog, "Error persisting change to device manager connections");
        }
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while recovering the change to device manager connections";
        RH_ERROR(this->_baseLog, eout.str())
        removeDeviceManager(findDeviceManagerById(identifier));
        throw CF::DomainManager::RegisterError(CF::CF_NOTSET, eout.str().c_str());
    } catch ( CORBA::Exception& ex ) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while recovering the change to device manager connections";
        RH_ERROR(this->_baseLog, eout.str())
        removeDeviceManager(findDeviceManagerById(identifier));
        throw CF::DomainManager::RegisterError(CF::CF_NOTSET, eout.str().c_str());
    } catch ( ... ) {
        // Any exceptions at this point require us to remove the device manager and cleanup any other issues
        removeDeviceManager(findDeviceManagerById(identifier));
        throw CF::DomainManager::RegisterError(CF::CF_NOTSET, "Unexpected error registering device manager");
    }

    RH_TRACE(this->_baseLog, "Leaving DomainManager::registerDeviceManager");
}


void
DomainManager_impl::addDeviceMgr (CF::DeviceManager_ptr deviceMgr)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    if (!deviceMgrIsRegistered (deviceMgr)) {
        RH_TRACE(this->_baseLog, "Adding DeviceManager ref to list")
        DeviceManagerNode tmp_devMgr;
        CORBA::String_var identifier = deviceMgr->identifier();
        CORBA::String_var label = deviceMgr->label();
        tmp_devMgr.deviceManager = CF::DeviceManager::_duplicate(deviceMgr);
        tmp_devMgr.identifier = static_cast<char*>(identifier);
        tmp_devMgr.label = static_cast<char*>(label);
        try{
            // preload DCD
            CF::FileSystem_var devMgrFileSys = tmp_devMgr.deviceManager->fileSys();
            CORBA::String_var profile = tmp_devMgr.deviceManager->deviceConfigurationProfile();
            File_stream dcd(devMgrFileSys, profile);
            tmp_devMgr.dcd.load(dcd);
            dcd.close();            
        }
        catch(...){
        }
        _registeredDeviceManagers.push_back(tmp_devMgr);

        try {
            db.store("DEVICE_MANAGERS", _registeredDeviceManagers);
        } catch (const ossie::PersistenceException& ex) {
            RH_ERROR(this->_baseLog, "Error persisting change to device managers");
        }
    }
}

void DomainManager_impl::addDomainMgr (CF::DomainManager_ptr domainMgr)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    if (!domainMgrIsRegistered (domainMgr)) {
        RH_TRACE(this->_baseLog, "Adding DomainManager ref to list")
        DomainManagerNode node;
        node.domainManager = CF::DomainManager::_duplicate(domainMgr);
        node.identifier = ossie::corba::returnString(domainMgr->identifier());
        node.name = ossie::corba::returnString(domainMgr->name());
        _registeredDomainManagers.push_back(node);

        try {
            db.store("DOMAIN_MANAGERS", _registeredDomainManagers);
        } catch (const ossie::PersistenceException& ex) {
            RH_ERROR(this->_baseLog, "Error persisting change to domain managers");
        }
    }
}

void DomainManager_impl::mountDeviceMgrFileSys (CF::DeviceManager_ptr deviceMgr) {
    string mountPoint = "/";
    CORBA::String_var devMgrLabel;
    // mount filesystem under "/<DomainName>/<deviceMgr.label>"
    devMgrLabel = deviceMgr->label();
    mountPoint += devMgrLabel;
    RH_TRACE(this->_baseLog, "Mounting DeviceManager FileSystem at " << mountPoint)

    CF::FileSystem_var devMgrFileSys = deviceMgr->fileSys();
    _fileMgr->mount(mountPoint.c_str(), devMgrFileSys);
}

void
DomainManager_impl::unregisterDeviceManager (CF::DeviceManager_ptr deviceMgr)
throw (CORBA::SystemException, CF::InvalidObjectReference,
       CF::DomainManager::UnregisterError)
{
    boost::mutex::scoped_lock lock(interfaceAccess);

    if (CORBA::is_nil(deviceMgr)) {
        RH_ERROR(this->_baseLog, "Cannot unregister nil DeviceManager");
        throw CF::InvalidObjectReference("Cannot unregister nil DeviceManager");
    }

    DeviceManagerList::iterator devMgrIter = findDeviceManagerByObject(deviceMgr);
    if (devMgrIter == _registeredDeviceManagers.end()) {
        RH_WARN(this->_baseLog, "Ignoring attempt to unregister device manager that was not registered with this domain");
        return;
    }

    if (!devMgrIter->deviceManager->_is_equivalent(deviceMgr)) {
        RH_TRACE(this->_baseLog, "Ignoring attempt to unregister device manager with same identifier but different object");
        return;
    }

    // Save the identifier and label, as the iterator will be invalidated.
    const std::string identifier = devMgrIter->identifier;
    const std::string label = devMgrIter->label;

    try {
        _local_unregisterDeviceManager(devMgrIter);
    } CATCH_RH_ERROR(this->_baseLog, "Exception unregistering device manager");

    sendRemoveEvent(_identifier, identifier, label, StandardEvent::DEVICE_MANAGER);
}

ossie::DeviceManagerList::iterator DomainManager_impl::_local_unregisterDeviceManager (ossie::DeviceManagerList::iterator deviceManager)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    //  For this function, an exception will be raised only when the Device Manager cannot be found
    //  If the individual Device references are bad, the unregistration of the device from the Domain
    //  will be done, but no error will be raised

    // Break and release connections owned by the unregistering DeviceManager.
    _connectionManager.deviceManagerUnregistered(deviceManager->label);
    try {
        db.store("CONNECTIONS", _connectionManager.getConnections());
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to device manager connections");
    }

    // Release all devices and services, which may break connections from other
    // DeviceManagers' DCDs.
    removeDeviceManagerDevices(deviceManager->identifier);
    removeDeviceManagerServices(deviceManager->identifier);

    // Unmount all DeviceManager FileSystems from the FileManager; currently just
    // the DeviceManager's root file system.
    string mountPoint = "/" + deviceManager->label;
    RH_TRACE(this->_baseLog, "Unmounting DeviceManager FileSystem at " << mountPoint)
    try {
        _fileMgr->unmount(mountPoint.c_str());
    } CATCH_RH_ERROR(this->_baseLog, "Unmounting DeviceManager FileSystem failed during unregistration");

    // Remove the DeviceManager from the domain.
    deviceManager = removeDeviceManager(deviceManager);

    //The unregisterDeviceManager operation shall, upon successful unregistration, send an event to
    //the Outgoing Domain Management event channel with event data consisting of a
    //DomainManagementObjectRemovedEventType. The event data will be populated as follows:
    //      1. The producerId shall be the identifier attribute of the DomainManager.
    //      2. The sourceId shall be the identifier attribute of the unregistered DeviceManager.
    //      3. The sourceName shall be the label attribute of the unregistered DeviceManager.
    //      4. The sourceCategory shall be DEVICE_MANAGER.

    return deviceManager;
}


void DomainManager_impl::removeDeviceManagerDevices (const std::string& deviceManagerId)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    // Unregister all devices for the DeviceManager
    for (DeviceList::iterator device = _registeredDevices.begin(); device != _registeredDevices.end(); ) {
        if ((*device)->devMgr.identifier == deviceManagerId) {
            RH_TRACE(this->_baseLog, "Unregistering device " << (*device)->label << " " << (*device)->identifier);
            device = _local_unregisterDevice(device);
        } else {
            ++device;
        }
    }
}


void DomainManager_impl::removeDeviceManagerServices (const std::string& deviceManagerId)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    for (ServiceList::iterator service = _registeredServices.begin(); service != _registeredServices.end(); ) {
        if (service->deviceManagerId == deviceManagerId) {
            RH_TRACE(this->_baseLog, "Unregistering service " << service->name);
            service = _local_unregisterService(service);
        } else {
            ++service;
        }
    }
}


ossie::DeviceManagerList::iterator DomainManager_impl::removeDeviceManager (ossie::DeviceManagerList::iterator deviceManager)
{
    // This function must work regardless of whether the DeviceManager object
    // is reachable or not. Therefore, no CORBA calls can be made.
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    deviceManager = _registeredDeviceManagers.erase(deviceManager);
    try {
        db.store("DEVICE_MANAGERS", _registeredDeviceManagers);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to device managers");
    }

    return deviceManager;
}

void
DomainManager_impl::registerDevice (CF::Device_ptr registeringDevice,
                                    CF::DeviceManager_ptr registeredDeviceMgr)
throw (CORBA::SystemException, CF::InvalidObjectReference, CF::InvalidProfile,
       CF::DomainManager::DeviceManagerNotRegistered,
       CF::DomainManager::RegisterError)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    ossie::corba::overrideBlockingCall(registeringDevice,getDeviceWaitTime());
    ossie::corba::overrideBlockingCall(registeredDeviceMgr,getManagerWaitTime());
    _local_registerDevice(registeringDevice, registeredDeviceMgr);

    std::string identifier = ossie::corba::returnString(registeringDevice->identifier());
    std::string label = ossie::corba::returnString(registeringDevice->label());
    sendAddEvent(_identifier, identifier, label, registeringDevice, StandardEvent::DEVICE);
}

void DomainManager_impl::_local_registerDevice (CF::Device_ptr registeringDevice,
                                                CF::DeviceManager_ptr registeredDeviceMgr)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    //Verify they are not a nil reference
    if (CORBA::is_nil (registeringDevice)
            || CORBA::is_nil (registeredDeviceMgr)) {
        throw (CF::InvalidObjectReference
               ("[DomainManager::registerDevice] Cannot register Device. Either Device or DeviceMgr is a nil reference."));
    }

    //Verify that input is a registered DeviceManager
    if (!deviceMgrIsRegistered (registeredDeviceMgr)) {
        throw CF::DomainManager::DeviceManagerNotRegistered ();
    }

    std::string devId = ossie::corba::returnString(registeringDevice->identifier());
    RH_TRACE(this->_baseLog, "Registering Device " << devId);

    DeviceList::iterator deviceNode = findDeviceById(devId);
    if (deviceNode != _registeredDevices.end()) {
        RH_TRACE(this->_baseLog, "Device <" << devId << "> already registered; checking existence");
        if (!ossie::corba::objectExists((*deviceNode)->device)) {
            RH_WARN(this->_baseLog, "Cleaning up registration; device <" << devId << "> is registered and no longer exists");
            try {
                _local_unregisterDevice(deviceNode);
            } CATCH_RH_WARN(this->_baseLog, "_local_unregisterDevice failed");
        } else {
            ostringstream eout;
            eout << "Attempt re-register existing device : " << devId;
            RH_ERROR(this->_baseLog, eout.str());
            throw CF::DomainManager::RegisterError(CF::CF_NOTSET, eout.str().c_str());
        }
    }

    //Add registeringDevice and its attributes to domain manager
    storeDeviceInDomainMgr (registeringDevice, registeredDeviceMgr);

    //Check the DCD for connections and establish them
    try {
        RH_TRACE(this->_baseLog, "Establishing Service Connections");
        _connectionManager.deviceRegistered(devId.c_str());
        try {
            db.store("CONNECTIONS", _connectionManager.getConnections());
        } catch (const ossie::PersistenceException& ex) {
            RH_ERROR(this->_baseLog, "Error persisting change to device manager connections");
        }
    } catch ( ... ) {
        RH_ERROR(this->_baseLog, "Service connections could not be established")
    }

//NOTE: This function only checks that the input references are valid and the device manager is registered.
//No other sanity test is performed. In the event of any internal error that impedes a successful registration,
// a FAILURE_ALARM log record should be written and the RegisterError exception should be raised
}


//This function adds the registeringDevice and its attributes to the DomainMgr.
//if the device already exists it does nothing
void DomainManager_impl::storeDeviceInDomainMgr (CF::Device_ptr registeringDevice,
                                                 CF::DeviceManager_ptr registeredDeviceMgr)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

//check if device is already registered
    if (deviceIsRegistered (registeringDevice)) {
        RH_TRACE(this->_baseLog, "Device already registered, refusing to store into domain manager")
        return;
    }

    std::string devMgrId;
    try {
        devMgrId = ossie::corba::returnString(registeredDeviceMgr->identifier());
    } CATCH_RH_ERROR(this->_baseLog, "DeviceManager is unreachable during device registrations")
    if (devMgrId.empty()){
        return;
    }

    DeviceManagerList::iterator pDevMgr = findDeviceManagerById(devMgrId);
    if (pDevMgr ==  _registeredDeviceManagers.end()) {
        RH_ERROR(this->_baseLog, "Device Manager for Device is not registered")
        return;
    }

    // If this part is reached, the registering device has to be added
    // Get read-only attributes from registeringDevice
    boost::shared_ptr<DeviceNode> newDeviceNode(new DeviceNode());
    newDeviceNode->device = CF::Device::_duplicate(registeringDevice);
    newDeviceNode->devMgr = *pDevMgr;
    newDeviceNode->label = ossie::corba::returnString(registeringDevice->label());
    newDeviceNode->softwareProfile = ossie::corba::returnString(registeringDevice->softwareProfile());
    newDeviceNode->identifier = ossie::corba::returnString(registeringDevice->identifier());
    newDeviceNode->implementationId = ossie::corba::returnString(registeredDeviceMgr->getComponentImplementationId(newDeviceNode->identifier.c_str()));
    newDeviceNode->loadableDevice = ossie::corba::_narrowSafe<CF::LoadableDevice>(registeringDevice);
    newDeviceNode->executableDevice = ossie::corba::_narrowSafe<CF::ExecutableDevice>(registeringDevice);

    parseDeviceProfile(*newDeviceNode);

    _registeredDevices.push_back (newDeviceNode);

    try {
        db.store("DEVICES", _registeredDevices);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to device managers");
    }
}

//This function adds the registeringService and its name to the DomainMgr.
//if the service already exists it does nothing
void
DomainManager_impl::storeServiceInDomainMgr (CORBA::Object_ptr registeringService, CF::DeviceManager_ptr registeredDeviceMgr, const std::string& name, const std::string& serviceId)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    // If a service is already registered with that name, do nothing.
    if (serviceIsRegistered(name)) {
        RH_INFO(this->_baseLog, "Ignoring duplicate registration of service " << name);
        return;
    }

    // The service needs to be added to the list.
    std::string devMgrId;
    try {
        devMgrId = ossie::corba::returnString(registeredDeviceMgr->identifier());
    } CATCH_RH_ERROR(this->_baseLog, "DeviceManager is unreachable during service registration")

    ServiceNode node;
    node.service = CORBA::Object::_duplicate(registeringService);
    node.deviceManagerId = devMgrId;
    node.name = name;
    node.serviceId = serviceId;

    // Add service to registered list, updating changes in the persistence store.
    _registeredServices.push_back(node);
    try {
        db.store("SERVICES", _registeredServices);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to services");
    }
}


void
DomainManager_impl::unregisterDevice (CF::Device_ptr unregisteringDevice)
throw (CORBA::SystemException, CF::InvalidObjectReference,
       CF::DomainManager::UnregisterError)
{
    boost::mutex::scoped_lock lock(interfaceAccess);

    DeviceList::iterator deviceNode = findDeviceByObject(unregisteringDevice);
    if (deviceNode == _registeredDevices.end()) {
        throw CF::InvalidObjectReference("Device not registered with domain");
    }

    try {
        _local_unregisterDevice(deviceNode);
    } CATCH_RETHROW_RH_ERROR(this->_baseLog, "Error unregistering a Device")  // rethrow for calling object's benefit
}

ossie::DeviceList::iterator DomainManager_impl::_local_unregisterDevice (ossie::DeviceList::iterator deviceNode)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    // Reset the last successful device pointer for deployments
    if ((*deviceNode)->identifier == _lastDeviceUsedForDeployment) {
        _lastDeviceUsedForDeployment.clear();
    }

    // Break any connections depending on the device.
    _connectionManager.deviceUnregistered((*deviceNode)->identifier);
    try {
        db.store("CONNECTIONS", _connectionManager.getConnections());
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to device manager connections");
    }

    try {
        const std::string device_id = (*deviceNode)->identifier;
        // Release all applications that are using this device
        // THIS BEHAVIOR ISN'T SPECIFIED IN SCA, BUT IT MAKES GOOD SENSE
        std::vector<Application_impl*> releasedApps;
        for (ApplicationTable::iterator app = _applications.begin(); app != _applications.end(); ++app) {
            if (applicationDependsOnDevice(app->second, device_id)) {
                app->second->_add_ref();
                releasedApps.push_back(app->second);
            }
        }

        for (std::vector<Application_impl*>::iterator iter = releasedApps.begin(); iter != releasedApps.end(); ++iter) {
            RH_WARN(this->_baseLog, "Releasing application that depends on registered device " << (*deviceNode)->identifier);
            Application_impl* app = *iter;
            app->releaseObject();
            app->_remove_ref();
        }
    } CATCH_RH_ERROR(this->_baseLog, "Releasing stale applications from stale device failed");

    // Sent event here (as opposed to unregisterDevice), so we see the event on regular
    // unregisterDevice calls, and on cleanup (deviceManager shutdown, catastropic cleanup, etc.)
    sendRemoveEvent(_identifier, (*deviceNode)->identifier, (*deviceNode)->label, StandardEvent::DEVICE);

    // Remove the device from the internal list.
    deviceNode = _registeredDevices.erase(deviceNode);

    // Write the updated device list to the persistence store.
    try {
        db.store("DEVICES", _registeredDevices);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to devices");
    }
    return deviceNode;
}


//This function returns TRUE if the input registeredDevice is contained in the _registeredDevices
bool DomainManager_impl::deviceIsRegistered (CF::Device_ptr registeredDevice)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    DeviceList::iterator device = findDeviceByObject(registeredDevice);

    return (device !=_registeredDevices.end());
}


bool DomainManager_impl::serviceIsRegistered (const std::string& serviceName)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    ossie::ServiceList::iterator service = findServiceByName(serviceName);

    return (service != _registeredServices.end());
}

void DomainManager_impl::closeAllOpenFileHandles()
{
    RH_INFO(this->_baseLog, "Received SIGUSR1. Closing all open file handles");
    this->fileMgr_servant->closeAllFiles();
}


//This function returns TRUE if the input registeredDeviceMgr is contained in the _deviceManagers list attribute
bool DomainManager_impl::deviceMgrIsRegistered (CF::DeviceManager_ptr registeredDeviceMgr)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    DeviceManagerList::iterator node = findDeviceManagerByObject(registeredDeviceMgr);

    return (node != _registeredDeviceManagers.end());;
}

//This function returns TRUE if the input registeredDomainMgr is contained in the _domainManagers list attribute
bool DomainManager_impl::domainMgrIsRegistered (CF::DomainManager_ptr registeredDomainMgr)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    DomainManagerList::iterator node = findDomainManagerByObject(registeredDomainMgr);

    return (node != _registeredDomainManagers.end());;
}


CF::Application_ptr DomainManager_impl::createApplication(const char* profileFileName,
                                                          const char* name,
                                                          const CF::Properties& initConfiguration,
                                                          const CF::DeviceAssignmentSequence& deviceAssignments)
{
    ApplicationFactory_impl factory(profileFileName, _domainName, this);
    factory.setLogger(_baseLog->getChildLogger("ApplicationFactory", ""));
    CF::Application_var application = factory.create(name, initConfiguration, deviceAssignments);

    return application._retn();
}


//      METHOD:         installApplication
//      PURPOSE:        verify that all application file dependencies are available within
//                              the domain managers file manager
//      EXCEPTIONS:
//              --              InvalidProfile
//              --              InvalidFileName
//              --              ApplicationInstallationError

void DomainManager_impl::installApplication (const char* profileFileName)
throw (CORBA::SystemException, 
       CF::InvalidProfile, 
       CF::InvalidFileName,
       CF::DomainManager::ApplicationInstallationError, 
       CF::DomainManager::ApplicationAlreadyInstalled)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    _local_installApplication(profileFileName);

    ApplicationFactoryTable::iterator appFact = _applicationFactories.find(profileFileName);
    if (appFact != _applicationFactories.end()) {
        const std::string& identifier = appFact->first;
        const std::string& name = appFact->second->getName();
        CF::ApplicationFactory_var appFactRef = appFact->second->_this();
        sendAddEvent(_identifier, identifier, name, appFactRef, StandardEvent::APPLICATION_FACTORY);
    }
}

void DomainManager_impl::_local_installApplication (const std::string& profileFileName)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

// NOTE: the <softwareassembly> name attribute is the name of the App Factory
//               that is currently installed because it is the installed factory that
//               provides the value of profileFileName

    // check the profile ends with .sad.xml, warn if it doesn't
    if (profileFileName.find(".sad.xml") == std::string::npos) {
        RH_WARN(this->_baseLog, "File " << profileFileName << " should end with .sad.xml.");
    }

    try {
        RH_TRACE(this->_baseLog, "installApplication: Createing new AppFac");
        ApplicationFactory_impl* appFact = new ApplicationFactory_impl(profileFileName, this->_domainName, this);
        appFact->setLogger(_baseLog->getChildLogger("ApplicationFactory", ""));
        const std::string& appFactoryId = appFact->getIdentifier();

        // Check if application factory already exists for this profile
        RH_TRACE(this->_baseLog, "Installing application ID " << appFactoryId);
        if (_applicationFactories.count(appFactoryId)) {
            RH_INFO(this->_baseLog, "Application " << appFact->getName() << " with id " << appFactoryId
                     << " already installed (Application Factory already exists)");
            delete appFact;
            appFact=NULL;
            throw CF::DomainManager::ApplicationAlreadyInstalled();
        }

        std::string activationId = _domainName + "/" + appFactoryId;
        PortableServer::ObjectId_var oid = ossie::corba::activatePersistentObject(appFact_poa, appFact, activationId);
        _applicationFactories[appFactoryId] = appFact;
        _installedApplications.insert(profileFileName);

        try {
            db.store("APP_FACTORIES", _installedApplications);
        } catch (const ossie::PersistenceException& ex) {
            RH_ERROR(this->_baseLog, "Error persisting change to device managers");
        }
    } catch (CF::FileException& ex) {
        RH_ERROR(this->_baseLog, "installApplication: While validating the SAD profile: " << ex.msg);
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
    } catch( CF::InvalidFileName& ex ) {
        RH_ERROR(this->_baseLog, "installApplication: Invalid file name: " << profileFileName);
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, "Invalid file name");
    } catch (CF::DomainManager::ApplicationInstallationError& e) {
        RH_TRACE(this->_baseLog, "rethrowing ApplicationInstallationError" << e.msg);
        throw;
    } catch (CF::DomainManager::ApplicationAlreadyInstalled &) {
        throw;
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while restoring the application factories";
        RH_ERROR(this->_baseLog, eout.str())
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_NOTSET, eout.str().c_str());
    } catch ( const CORBA::Exception& ex ) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while restoring the application factories";
        RH_ERROR(this->_baseLog, eout.str())
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_NOTSET, eout.str().c_str());
    } catch (...) {
        RH_ERROR(this->_baseLog, "unexpected exception occurred while installing application");
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_NOTSET, "unknown exception");
    }
}


void
DomainManager_impl::uninstallApplication (const char* applicationId)
throw (CORBA::SystemException, CF::DomainManager::InvalidIdentifier,
       CF::DomainManager::ApplicationUninstallationError)
{
    RH_INFO(this->_baseLog, "Uninstalling application " << applicationId);
    boost::mutex::scoped_lock lock(interfaceAccess);

    std::string appFactory_id;
    std::string appFactory_name;

    ApplicationFactoryTable::iterator appFact = _applicationFactories.find(applicationId);
    if (appFact != _applicationFactories.end()) {
        appFactory_id = appFact->first;
        appFactory_name = appFact->second->getName();
    }

    _local_uninstallApplication(applicationId);

        // if SUCCESS, write an ADMINISTRATIVE_EVENT to the DomainMgr's Log

        // send event to Outgoing Domain Management channel consisting of:
        // DomainManager identifier attribute
        // uninstalled AppFactory identifier
        // sadParser.getId()
        // uninstalled AppFactory name
        // sadParser.getName()
        // uninstalled AppFactory IOR
        // ask the ORB
        // sourceCategory = APPLICATION_FACTORY
        // StandardEvent enumeration

    sendRemoveEvent(_identifier, appFactory_id, appFactory_name, StandardEvent::APPLICATION_FACTORY);
}

void DomainManager_impl::_local_uninstallApplication (const char* applicationId)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    // Find the factory in the table, which also validates the identifier
    ApplicationFactoryTable::iterator appFact = _applicationFactories.find(applicationId);
    if (appFact == _applicationFactories.end()) {
        throw CF::DomainManager::InvalidIdentifier();
    }

    // Update the persistence database
    const std::string sad_file = appFact->second->getSoftwareProfile();
    _installedApplications.erase(sad_file);
    try {
        db.store("APP_FACTORIES", _installedApplications);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to installed applications");
    }

    // Deactivate the servant
    PortableServer::ObjectId_var oid = appFact_poa->servant_to_id(appFact->second);
    appFact_poa->deactivate_object(oid);

    // Remove the servant from the list and clean up the reference
    _applicationFactories.erase(appFact);
    appFact->second->_remove_ref();
}

void DomainManager_impl::updateLocalAllocations(const ossie::AllocationTable& localAllocations)
{
    try {
        db.store("LOCAL_ALLOCATIONS", localAllocations);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting local allocations");
    }
}

void DomainManager_impl::updateRemoteAllocations(const ossie::RemoteAllocationTable& remoteAllocations)
{
    try {
        db.store("REMOTE_ALLOCATIONS", remoteAllocations);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting remote allocation");
    }
}

void
DomainManager_impl::addApplication(Application_impl* new_app)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    const std::string& identifier = new_app->getIdentifier();
    RH_TRACE(this->_baseLog, "Attempting to add application to AppSeq with id: " << identifier);
    try {
        _applications[identifier] = new_app;
        new_app->_add_ref();

        // Make any deferred connections dependent on this application
        _connectionManager.applicationRegistered(identifier);

        _persistApplication(new_app);
    } catch (...) {
        ostringstream eout;
        eout << "Could not add new application to AppSeq; ";
        eout << " application id: " << identifier << "; ";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        RH_ERROR(this->_baseLog, eout.str());
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EFAULT, eout.str().c_str());
    }
}

void DomainManager_impl::addPendingApplication(Application_impl* application)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);
    application->_add_ref();
    _pendingApplications[application->getIdentifier()] = application;
}

void DomainManager_impl::cancelPendingApplication(Application_impl* application)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);
    ApplicationTable::iterator iter = _pendingApplications.find(application->getIdentifier());
    if (iter == _pendingApplications.end()) {
        RH_ERROR(this->_baseLog, "No pending application '" << application->getIdentifier() << "' to cancel");
    } else {
        _pendingApplications.erase(iter);
        application->_remove_ref();
    }
}

void DomainManager_impl::completePendingApplication(Application_impl* application)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);
    ApplicationTable::iterator iter = _pendingApplications.find(application->getIdentifier());
    if (iter == _pendingApplications.end()) {
        RH_ERROR(this->_baseLog, "No pending application '" << application->getIdentifier()
                  << "' to move to active state");
    } else {
        addApplication(application);
        application->_remove_ref();
        _pendingApplications.erase(iter);
    }
}

void
DomainManager_impl::removeApplication(std::string app_id)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    RH_TRACE(this->_baseLog, "Attempting to remove application from AppSeq with id: " << app_id)

    ApplicationTable::iterator app = _applications.find(app_id);
    // remove the application from the sequence
    if (app != _applications.end()) {
        // Break dependent connections
        _connectionManager.applicationUnregistered(app_id);

        app->second->_remove_ref();
        _applications.erase(app);

        // Remove the application node as well, then reserialize.
        for (std::vector<ApplicationNode>::iterator ii = _runningApplications.begin(); ii != _runningApplications.end(); ++ii) {
            if (ii->identifier == app_id) {
                _runningApplications.erase(ii);
                break;
            }
        }

        try {
            db.store("APPLICATIONS", _runningApplications);
        } catch (const ossie::PersistenceException& ex) {
            RH_ERROR(this->_baseLog, "Error persisting change to device managers");
        }
    } else {
        ostringstream eout;
        eout << "Could find application in AppSeq; ";
        eout << " with application id: " << app_id << "; ";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        throw CF::DomainManager::ApplicationUninstallationError(CF::CF_EFAULT, eout.str().c_str());
    }
}




void
DomainManager_impl::registerWithEventChannel (CORBA::
                                              Object_ptr registeringObject,
                                              const char* registeringId,
                                              const char* eventChannelName)
throw (CORBA::SystemException, CF::InvalidObjectReference,
       CF::DomainManager::InvalidEventChannelName,
       CF::DomainManager::AlreadyConnected)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    _local_registerWithEventChannel(registeringObject, registeringId, eventChannelName);
}

void DomainManager_impl::_local_registerWithEventChannel (CORBA::Object_ptr registeringObject,
                                                          const std::string& registeringId,
                                                          const std::string& eventChannelName)
{
    if (registeredConsumers.find(registeringId) != registeredConsumers.end()) {
        throw CF::DomainManager::AlreadyConnected ();
    }
    
    CORBA::Object_var generic_channel = lookupDomainObject("eventchannel", eventChannelName);


    // 1. determine whether or not the channel name is valid
    // 2. determine whether or not the channel id is already being used
    // 3. make sure that the registering object is a consumer
    // 4. create the connection
    // 5. store the new connection reference

    CosEventChannelAdmin::EventChannel_var channel = ossie::corba::_narrowSafe<CosEventChannelAdmin::EventChannel>(generic_channel);
    if (CORBA::is_nil(channel)) {
        throw CF::InvalidObjectReference ("The stored channel reference is invalid. Unable to register the consumer.");
    }
    try {
        CosEventChannelAdmin::ConsumerAdmin_var consumer_admin = channel->for_consumers();
        CosEventChannelAdmin::ProxyPushSupplier_var proxy_supplier = consumer_admin->obtain_push_supplier();
        CosEventComm::PushConsumer_var consumer = CosEventComm::PushConsumer::_narrow (registeringObject);
        proxy_supplier->connect_push_consumer(consumer);
        registeredConsumers[registeringId].proxy_supplier = proxy_supplier;
        registeredConsumers[registeringId].channelName = eventChannelName;
    } catch ( ... ) {
        throw CF::InvalidObjectReference ("A problem occurred while trying to create a connection between the consumer and the event channel. One of the references is invalid.");
    }
}


void
DomainManager_impl::unregisterFromEventChannel (const char* unregisteringId,
                                                const char* eventChannelName)
throw (CORBA::SystemException, CF::DomainManager::InvalidEventChannelName,
       CF::DomainManager::NotConnected)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    _local_unregisterFromEventChannel(unregisteringId, eventChannelName);
}

void DomainManager_impl::_local_unregisterFromEventChannel (const std::string& unregisteringId,
                                                            const std::string& eventChannelName)
{
    if (!eventChannelExists(eventChannelName)) {
        throw CF::DomainManager::InvalidEventChannelName ();
    }
    
    if (registeredConsumers.find(unregisteringId) == registeredConsumers.end()) {
        throw CF::DomainManager::NotConnected ();
    }
    try {
        registeredConsumers[unregisteringId].proxy_supplier->disconnect_push_supplier();
    } catch ( ... ) {
        throw CF::DomainManager::NotConnected ();
    }
    registeredConsumers.erase(unregisteringId);
}


void DomainManager_impl::registerService (CORBA::Object_ptr registeringService, CF::DeviceManager_ptr registeredDeviceMgr, const char* name)
    throw (CF::DomainManager::RegisterError, CF::DomainManager::DeviceManagerNotRegistered, CF::InvalidObjectReference, CORBA::SystemException)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    ossie::corba::overrideBlockingCall(registeringService,getServiceWaitTime());
    ossie::corba::overrideBlockingCall(registeredDeviceMgr,getManagerWaitTime());
    _local_registerService(registeringService, registeredDeviceMgr, name);
}

void DomainManager_impl::_local_registerService (CORBA::Object_ptr registeringService, CF::DeviceManager_ptr registeredDeviceMgr, const char* name)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);


    // Verify that the service and DeviceManager are not nil references
    if (CORBA::is_nil(registeringService)) {
        RH_ERROR(this->_baseLog, "Ignoring registration of nil Service");
        throw CF::InvalidObjectReference("Registering Service is nil");
    } else if (CORBA::is_nil(registeredDeviceMgr)) {
        RH_ERROR(this->_baseLog, "Ignoring registration of Service with nil DeviceManager");
        throw CF::InvalidObjectReference("Registered DeviceManager is nil");
    }

    // Verify that DeviceManager is registered
    if (!deviceMgrIsRegistered(registeredDeviceMgr)) {
        RH_WARN(this->_baseLog, "Ignoring attempt to register a Service from an unregistered DeviceManager");
        throw CF::DomainManager::DeviceManagerNotRegistered();
    }
    
    DeviceManagerConfiguration *_DCDParser=0;
    DeviceManagerConfiguration _dcdParser;
    bool readDCD = true;
    std::string serviceId("");
    std::string inputUsageName(name);
    try {
        CF::FileSystem_var devMgrFileSys = registeredDeviceMgr->fileSys();
        CORBA::String_var profile = registeredDeviceMgr->deviceConfigurationProfile();
         DeviceManagerList::iterator node = findDeviceManagerByObject(registeredDeviceMgr);
          if ( node != _registeredDeviceManagers.end() ) {
                if ( node->dcd.isLoaded() == false ) {
                    File_stream dcd(devMgrFileSys, profile);
                    node->dcd.load(dcd);
                    dcd.close();            
                }
                _DCDParser = &(node->dcd);
          }
          else {
              File_stream dcd(devMgrFileSys, profile);
              _dcdParser.load(dcd);
              _DCDParser = &_dcdParser;
              dcd.close();
          }
    } catch ( ... ) {
        readDCD = false;
    }
    if (readDCD) {
        const std::vector<ossie::DevicePlacement>& componentPlacements = _DCDParser->getComponentPlacements();
        bool foundId = false;
        for (unsigned int i = 0; i < componentPlacements.size(); i++) {
            for (unsigned int j=0; j<componentPlacements[i].getInstantiations().size(); j++) {
                std::string usageName(componentPlacements[i].getInstantiations()[j].getUsageName());
                if (usageName == inputUsageName) {
                    serviceId = componentPlacements[i].getInstantiations()[j].getID();
                    foundId = true;
                    break;
                }
            }
            if (foundId)
                break;
        }
    }

//The registerService operation shall add the registeringServices object reference and the
//registeringServices name to the DomainManager, if the name for the type of service being
//registered does not exist within the DomainManager. However, if the name of the registering
//service is a duplicate of a registered service of the same type, then the new service shall not be
//registered with the DomainManager.

//The registerService operation shall associate the input registeringService parameter with the
//input registeredDeviceMgr parameter in the DomainManagers, when the registeredDeviceMgr
//parameter indicates a DeviceManager registered with the DomainManager.

//Add registeringService and its name to domain manager
    try {
        storeServiceInDomainMgr(registeringService, registeredDeviceMgr, name, serviceId);
    } catch ( ... ) {
        throw;
    }

//The registerService operation shall, upon successful service registration, establish any pending
//connection requests for the registeringService. The registerService operation shall, upon
//successful service registration, write an ADMINISTRATIVE_EVENT log record to a
//DomainManagers Log.

    // Make any pending connections depending on this service.
    _connectionManager.serviceRegistered(name);
    try {
        db.store("CONNECTIONS", _connectionManager.getConnections());
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to device manager connections");
    }

//The registerService operation shall, upon unsuccessful service registration, write a
//FAILURE_ALARM log record to a DomainManagers Log.

//The registerService operation shall, upon successful service registration, send an event to the
//Outgoing Domain Management event channel with event data consisting of a
//DomainManagementObjectAddedEventType. The event data will be populated as follows:
//1. The producerId shall be the identifier attribute of the DomainManager.
//2. The sourceId shall be the identifier attribute from the componentinstantiation element
//associated with the registered service.
//3. The sourceName shall be the input name parameter for the registering service.
//4. The sourceIOR shall be the registered service object reference.
//5. The sourceCategory shall be SERVICE.
    sendAddEvent(_identifier, serviceId, name, registeringService, StandardEvent::SERVICE);

//The registerService operation shall raise the RegisterError exception when an internal error
//exists which causes an unsuccessful registration.
}

void DomainManager_impl::storePubProxies()
{
    EventProxies _proxies;
    EventChannelManager::PubProxyMap _retVal = _eventChannelMgr->getPubProxies();
    for (EventChannelManager::PubProxyMap::iterator it = _retVal.begin(); it != _retVal.end(); it++) {
        _proxies[it->first] = _orbCtx.orb->object_to_string(it->second);
    }
    try {
        db.store("EVTCHMGR_PUBPROXIES", _proxies);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to event channel manager publisher proxies");
    }
}

void DomainManager_impl::storeSubProxies()
{
    EventProxies _proxies;
    EventChannelManager::SubProxyMap _retVal = _eventChannelMgr->getSubProxies();
    for (EventChannelManager::SubProxyMap::iterator it = _retVal.begin(); it != _retVal.end(); it++) {
        _proxies[it->first] = _orbCtx.orb->object_to_string(it->second);
    }
    try {
        db.store("EVTCHMGR_SUBPROXIES", _proxies);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to event channel manager subscriber proxies");
    }
}

void DomainManager_impl::storeEventChannelRegistrations()
{
    ChannelRegistrationNodes _nodes;
    EventChannelManager::ChannelRegistrationTable _retVal = _eventChannelMgr->getChannelRegistrations();
    for (EventChannelManager::ChannelRegistrationTable::iterator it = _retVal.begin(); it != _retVal.end(); it++) {
        ChannelRegistrationNode _tmp;
        _tmp.channel_name = it->second.channel_name;
        _tmp.fqn = it->second.fqn;
        _tmp.channel = _orbCtx.orb->object_to_string(it->second.channel);
        _tmp.autoRelease = it->second.autoRelease;
        _tmp.release = it->second.release;
        _tmp.registrants = it->second.registrants;
        _nodes[it->first] = _tmp;
    }
    try {
        db.store("EVTCHMGR_CHANNELREGISTRATIONS", _nodes);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to event channel manager event channel registrations");
    }
}

void DomainManager_impl::restorePubProxies(const std::string& _db_uri)
{
    try {
        db.open(_db_uri);
    } catch (const ossie::PersistenceException& e) {
        RH_ERROR(this->_baseLog, "Error loading persistent state: " << e.what());
        return;
    }
    EventProxies _proxies;
    try {
        db.fetch("EVTCHMGR_PUBPROXIES", _proxies, true);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to event channel manager publisher proxies");
    }
    EventChannelManager::PubProxyMap _newVal;
    for (EventProxies::iterator it = _proxies.begin(); it != _proxies.end(); it++) {
        _newVal[it->first] = ossie::corba::_narrowSafe<CosEventChannelAdmin::ProxyPushConsumer>(_orbCtx.orb->string_to_object(it->second.c_str()));
    }
    _eventChannelMgr->setPubProxies(_newVal);
}

void DomainManager_impl::restoreSubProxies(const std::string& _db_uri)
{
    try {
        db.open(_db_uri);
    } catch (const ossie::PersistenceException& e) {
        RH_ERROR(this->_baseLog, "Error loading persistent state: " << e.what());
        return;
    }
    EventProxies _proxies;
    try {
        db.fetch("EVTCHMGR_SUBPROXIES", _proxies, true);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to event channel manager subscriber proxies");
    }
    EventChannelManager::SubProxyMap _newVal;
    for (EventProxies::iterator it = _proxies.begin(); it != _proxies.end(); it++) {
        _newVal[it->first] = ossie::corba::_narrowSafe<CosEventChannelAdmin::ProxyPushSupplier>(_orbCtx.orb->string_to_object(it->second.c_str()));
    }
    _eventChannelMgr->setSubProxies(_newVal);
}

void DomainManager_impl::restoreEventChannelRegistrations(const std::string& _db_uri)
{
    try {
        db.open(_db_uri);
    } catch (const ossie::PersistenceException& e) {
        RH_ERROR(this->_baseLog, "Error loading persistent state: " << e.what());
        return;
    }
    ChannelRegistrationNodes _nodes;
    try {
        db.fetch("EVTCHMGR_CHANNELREGISTRATIONS", _nodes, true);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to event channel manager event channel registrations");
    }
    EventChannelManager::ChannelRegistrationTable _newVal;// = _eventChannelMgr->getChannelRegistrations();
    for (ChannelRegistrationNodes::iterator it = _nodes.begin(); it != _nodes.end(); it++) {
        EventChannelManager::ChannelRegistration _tmp;
        _tmp.channel_name = it->second.channel_name;
        _tmp.fqn = it->second.fqn;
        _tmp.channel = ossie::corba::_narrowSafe<CosEventChannelAdmin::EventChannel>(_orbCtx.orb->string_to_object(it->second.channel.c_str()));
        _tmp.autoRelease = it->second.autoRelease;
        _tmp.release = it->second.release;
        _tmp.registrants = it->second.registrants;
        _newVal[it->first] = _tmp;
    }
    _eventChannelMgr->setChannelRegistrations(_newVal);
}

void DomainManager_impl::unregisterService(CORBA::Object_ptr unregisteringService, const char* name)
    throw (CF::DomainManager::UnregisterError, CF::InvalidObjectReference, CORBA::SystemException)
{
    boost::mutex::scoped_lock lock(interfaceAccess);

    // Try to find a service registered with the given name.
    ossie::ServiceList::iterator service = findServiceByName(name);
    if (service == _registeredServices.end()) {
        RH_ERROR(this->_baseLog, "Cannot unregister service '" << name << "' not registered with domain");
        throw CF::InvalidObjectReference("Service is not registered with domain");
    }

    // Refuse to unregister if the unregistering object is not the same as the
    // registered one.
    if (!service->service->_is_equivalent(unregisteringService)) {
        RH_ERROR(this->_baseLog, "Not unregistering service '" << name << "' because object does not match prior registration");
        throw CF::InvalidObjectReference("Unregistering service is not the same as registered object");
    }
    
    _local_unregisterService(service);
}


ossie::ServiceList::iterator DomainManager_impl::_local_unregisterService(ossie::ServiceList::iterator service)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    // Disconnect any connections involving this service.
    _connectionManager.serviceUnregistered(service->name);
    try {
        db.store("CONNECTIONS", _connectionManager.getConnections());
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to device manager connections");
    }
    
    std::string serviceName(service->name);
    std::string serviceId(service->serviceId);

    // Remove the service from the internal list.
    service = _registeredServices.erase(service);

    if (!_applications.empty()) {
        std::vector<Application_impl*> appsToRelease;
        for (ApplicationTable::iterator app = _applications.begin(); app != _applications.end(); ++app) {
            if (app->second->checkConnectionDependency(ossie::Endpoint::SERVICENAME, serviceName)) {
                app->second->_add_ref();
                appsToRelease.push_back(app->second);
            }
        }

        RH_DEBUG(this->_baseLog, "Releasing " << appsToRelease.size() << " applications");
        for (std::vector<Application_impl*>::iterator iter = appsToRelease.begin(); iter != appsToRelease.end(); ++iter) {
            Application_impl* app = *iter;
            RH_DEBUG(this->_baseLog, "Releasing " << app->getIdentifier());
            app->releaseObject();
            app->_remove_ref();
        }
    }

    // Write the updated service list to the persistence store.
    try {
        db.store("SERVICES", _registeredServices);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to services");
    }

    //The unregisterService operation shall, upon successful service unregistration, send an event to
    //the Outgoing Domain Management event channel with event data consisting of a
    //DomainManagementObjectRemovedEventType. The event data will be populated as follows:
    //1. The producerId shall be the identifier attribute of the DomainManager.
    //2. The sourceId shall be the ID attribute from the componentinstantiation element
    //associated with the unregistered service.
    //3. The sourceName shall be the input name parameter for the unregistering service.
    //4. The sourceCategory shall be SERVICE.
    // Sent event here (as opposed to unregisterDevice), so we see the event on regular
    // unregisterDevice calls, and on cleanup (deviceManager shutdown, catastropic cleanup, etc.)
    sendRemoveEvent(_identifier, serviceId, serviceName, StandardEvent::SERVICE);

    return service;
}


CF::Resource_ptr DomainManager_impl::lookupComponentByInstantiationId(const std::string& identifier)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);
    DeviceList::iterator deviceNode = findDeviceById(identifier);
    if (deviceNode != _registeredDevices.end()) {
        return CF::Resource::_duplicate((*deviceNode)->device);
    }
    // Search for a Component matching this id
    //  This needs to reconcile the fact that the application id is <softwareassembly id>:<app name>, the component id is <componentinstantiation id>:<app name>
    //   and the unambiguous endpoint for a component is <componentinstantiation id>:<softwareassembly id>:<app name>
    std::size_t begin_pos = 0;
    if (identifier.size() > 4) {
        if (identifier.substr(0,3) == "DCE") {
            begin_pos = 4;
        }
    }
    std::size_t pos = identifier.find(":", begin_pos);
    if (pos != std::string::npos) {
        std::string appid = identifier.substr(pos+1);
        if (_applications.find(appid) == _applications.end()) {
            // search by id substr instead of id
            ApplicationTable::iterator it = _applications.begin();
            while (it != _applications.end()) {
                if (appid == it->first.substr(it->first.size()-appid.size(),appid.size())) {
                    appid = it->first;
                    break;
                }
                it++;
            }
            if (it == _applications.end())
                return CF::Resource::_nil();
        }
        std::string normalized_comp_id = identifier.substr(0,pos)+std::string(":")+appid.substr(appid.rfind(":")+1);
        for (Application_impl::ComponentList::iterator _comp=_applications[appid]->_components.begin(); _comp!=_applications[appid]->_components.end(); _comp++) {\
            if (normalized_comp_id == _comp->getIdentifier()) {
                return _comp->getResourcePtr();
            }
        }
    }

    return CF::Resource::_nil();
}

CF::DeviceManager_ptr DomainManager_impl::lookupDeviceManagerByInstantiationId(const std::string& identifier)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);
    DeviceManagerList::iterator deviceManagerNode = findDeviceManagerById(identifier);
    if (deviceManagerNode != _registeredDeviceManagers.end()) {
        return CF::DeviceManager::_duplicate(deviceManagerNode->deviceManager);
    }

    return CF::DeviceManager::_nil();
}

CORBA::Object_ptr DomainManager_impl::lookupDomainObject (const std::string& type, const std::string& name)
{
    RH_TRACE(this->_baseLog, "Resolving domainfinder type='" << type << "' name='" << name << "'");
    if (type == "filemanager") {
        return CF::FileManager::_duplicate(_fileMgr);
    } else if (type == "log") {
        RH_WARN(this->_baseLog, "No support for log in domainfinder element");
    } else if (type == "eventchannel") {
        // If no name is given, return the IDM channel.
        std::string channelName = name;
        if (channelName.empty()) {
            channelName = "IDM_Channel";
        }

        // Check whether the event channel already exists; if it does, return a reference,
        // otherwise create it.
        CORBA::Object_var channelObj;
        if (eventChannelExists(channelName)) {
            channelObj = getEventChannel(channelName);
        } else {
            channelObj = createEventChannel(channelName);
        }

        // Increment the reference count for the desired channel.
        incrementEventChannelConnections(channelName);
        return channelObj._retn();
    } else if (type == "namingservice") {
        RH_WARN(this->_baseLog, "No support for namingservice in domainfinder element");
    } else if (type == "servicename") {
        ServiceList::iterator serviceNode = findServiceByName(name);
        if (serviceNode != _registeredServices.end()) {
            RH_TRACE(this->_baseLog, "Found service " << name);
            return CORBA::Object::_duplicate(serviceNode->service);
        }
        throw ossie::LookupError("no service '" + name + "' found");
    } else if (type == "servicetype") {
        ServiceList::iterator serviceNode = findServiceByType(name);
        if (serviceNode != _registeredServices.end()) {
            RH_TRACE(this->_baseLog, "Found service " << serviceNode->name << " supporting '" << name << "'");
            return CORBA::Object::_duplicate(serviceNode->service);
        }
        throw ossie::LookupError("no service found for type '" + name + "'");
    } else if (type == "domainmanager") {
        return _this();
    } else if (type == "application") {
        Application_impl* application = findApplicationById(name);
        if (!application) {
            throw ossie::LookupError("no application '" + name + "' found");
        }
        return application->_this();
    } else {
        throw ossie::LookupError("invalid domainfinder type '" + type + "'");
    }
    return CORBA::Object::_nil();
}


void DomainManager_impl::catastrophicUnregisterDeviceManager (ossie::DeviceManagerList::iterator deviceManager)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    // NOTE: Assume that the DeviceManager doesn't exist, so make no CORBA calls to it.

    // Release all devices associated with the DeviceManager that is being unregistered.
    RH_TRACE(this->_baseLog, "Finding devices for device manager " << deviceManager->identifier);
    for (DeviceList::iterator device = _registeredDevices.begin(); device != _registeredDevices.end(); ++device) {
        if ((*device)->devMgr.identifier == deviceManager->identifier) {
            RH_TRACE(this->_baseLog, "Releasing registered device " << (*device)->label);
            try {
                (*device)->device->releaseObject();
            } CATCH_RH_WARN(this->_baseLog, "Failed to release device " << (*device)->label);
        }
    }

    // Continue with the normal unregistration code path.
    _local_unregisterDeviceManager(deviceManager);
}


ossie::DeviceList::iterator DomainManager_impl::findDeviceById (const std::string& identifier)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    DeviceList::iterator device;
    for (device = _registeredDevices.begin(); device != _registeredDevices.end(); ++device) {
        if (identifier == (*device)->identifier) {
            break;
        }
    }
    return device;
}

ossie::DeviceList::iterator DomainManager_impl::findDeviceByObject (CF::Device_ptr device)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    try {
        std::string identifier = ossie::corba::returnString(device->identifier());
        return findDeviceById(identifier);
    } catch (...) {
        // Device is not reachable for some reason, try checking by object.
    }
    DeviceList::iterator node;
    for (node = _registeredDevices.begin(); node != _registeredDevices.end(); ++node) {
        if (device->_is_equivalent((*node)->device)) {
            break;
        }
    }
    return node;
}

ossie::DeviceList DomainManager_impl::getRegisteredDevices() {
    boost::recursive_mutex::scoped_lock lock(stateAccess);
    return _registeredDevices;
}

std::string DomainManager_impl::getLastDeviceUsedForDeployment ()
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);
    return _lastDeviceUsedForDeployment;
}

void DomainManager_impl::setLastDeviceUsedForDeployment (const std::string& identifier)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);
    _lastDeviceUsedForDeployment = identifier;
}

ossie::DomainManagerList DomainManager_impl::getRegisteredRemoteDomainManagers() {
    boost::recursive_mutex::scoped_lock lock(stateAccess);
    return this->_registeredDomainManagers;
}

ossie::DeviceManagerList::iterator DomainManager_impl::findDeviceManagerById (const std::string& identifier)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    DeviceManagerList::iterator node;
    for (node = _registeredDeviceManagers.begin(); node != _registeredDeviceManagers.end(); ++node) {
        if (identifier == node->identifier) {
            break;
        }
    }

    return node;
}


ossie::DeviceManagerList::iterator DomainManager_impl::findDeviceManagerByObject (CF::DeviceManager_ptr deviceManager)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    try {
        std::string identifier = ossie::corba::returnString(deviceManager->identifier());
        return findDeviceManagerById(identifier);
    } catch (...) {
        // DeviceManager is not reachable for some reason, try checking by object.
    }

    DeviceManagerList::iterator node;
    for (node = _registeredDeviceManagers.begin(); node != _registeredDeviceManagers.end(); ++node) {
        if (deviceManager->_is_equivalent(node->deviceManager)) {
            break;
        }
    }

    return node;
}


ossie::ServiceList::iterator DomainManager_impl::findServiceByName (const std::string& name)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    ServiceList::iterator node;
    for (node = _registeredServices.begin(); node != _registeredServices.end(); ++node) {
        if (name == node->name) {
            break;
        }
    }
    return node;
}


ossie::ServiceList::iterator DomainManager_impl::findServiceByType (const std::string& repId)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    ServiceList::iterator node;
    for (node = _registeredServices.begin(); node != _registeredServices.end(); ++node) {
        try {
            if (node->service->_is_a(repId.c_str())) {
                break;
            }

        } catch (...) {
            // Service is unreachable, just ignore it.
        }
    }
    return node;
}


Application_impl* DomainManager_impl::findApplicationById (const std::string& identifier)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    ApplicationTable::iterator application = _applications.find(identifier);
    if (application == _applications.end()) {
        return 0;
    } else {
        return application->second;
    }
}


bool DomainManager_impl::applicationDependsOnDevice(Application_impl* application, const std::string& deviceId)
{
    CF::DeviceAssignmentSequence_var compDevices = application->componentDevices();
    for  (size_t index = 0; index < compDevices->length(); ++index) {
        if (deviceId == static_cast<const char*>(compDevices[index].assignedDeviceId)) {
            return true;
        }
    }
    return false;
}


void DomainManager_impl::parseDeviceProfile (ossie::DeviceNode& node)
{
    CF::FileSystem_var devMgrFS = node.devMgr.deviceManager->fileSys();

    // Parse and cache the device's SPD
    RH_TRACE(this->_baseLog, "Parsing SPD for device " << node.identifier);
    try {
        File_stream spd(devMgrFS, node.softwareProfile.c_str());
        node.spd.load(spd, node.softwareProfile);
    } catch (const ossie::parser_error& error) {
        std::string parser_error_line = ossie::retrieveParserErrorLineNumber(error.what());
        RH_WARN(this->_baseLog, "Error parsing SPD: " <<  node.softwareProfile << "  Device: " << node.identifier << ". " << parser_error_line << " The XML parser returned the following error: " << error.what());
    } catch (...) {
        RH_WARN(this->_baseLog, "Unable to cache SPD for device " << node.identifier);
    }

    // Parse and cache the device's PRF, if it has one
    if (node.spd.getPRFFile()) {
        RH_TRACE(this->_baseLog, "Parsing PRF for device " << node.identifier);
        try {
            File_stream prf(devMgrFS, node.spd.getPRFFile());
            node.prf.load(prf);
        } catch (const ossie::parser_error& error) {
            std::string parser_error_line = ossie::retrieveParserErrorLineNumber(error.what());
            RH_WARN(this->_baseLog, "Error parsing PRF: " << node.spd.getPRFFile() << " Device: " << node.identifier << ". " << parser_error_line << " The XML parser returned the following error: " << error.what());
        } catch (...) {
            RH_WARN(this->_baseLog, "Unable to cache PRF for device " << node.identifier);
        }
    }

    // Check for an implementation-specific PRF
    for (std::vector<ossie::SPD::Implementation>::const_iterator impl = node.spd.getImplementations().begin(); impl != node.spd.getImplementations().end(); ++impl) {
        if (impl->getID() != node.implementationId) {
            continue;
        }
        if (impl->getPRFFile()) {
            RH_TRACE(this->_baseLog, "Parsing implementation-specific PRF for device " << node.identifier);
            try {
                File_stream prf_file(devMgrFS, impl->getPRFFile());
                node.prf.join(prf_file);
            } catch (const ossie::parser_error& error) {
            std::string parser_error_line = ossie::retrieveParserErrorLineNumber(error.what());
                RH_WARN(this->_baseLog, "Error parsing implementation-specific PRF for Device: " << node.identifier << ". " << parser_error_line << " The XML parser returned the following error: " << error.what());
            } catch (...) {
                RH_WARN(this->_baseLog, "Unable to cache implementation-specific PRF for Device: " << node.identifier);
            }
        }
    }

    // Override with values from the DCD
    RH_TRACE(this->_baseLog, "Parsing DCD overrides for device " << node.identifier);
    const std::string deviceManagerProfile = ossie::corba::returnString(node.devMgr.deviceManager->deviceConfigurationProfile());
    if ( node.devMgr.dcd.isLoaded() == false ) {
        RH_WARN(this->_baseLog, "DCD file was not loaded for node: " << node.identifier  );
    }
    else {
        const ComponentInstantiation* instantiation = findComponentInstantiation(node.devMgr.dcd.getComponentPlacements(), node.identifier); 
        if (instantiation) {
            node.prf.override(instantiation->properties);
            ossie::convertComponentProperties( instantiation->getDeployerRequires(),
                                               node.requiresProps );
        } else {
            RH_WARN(this->_baseLog, "Unable to find device " << node.identifier << " in DCD");
        }
    }
}

Application_impl* DomainManager_impl::_restoreApplication(ossie::ApplicationNode& node)
{
    Application_impl* application = new Application_impl(node.identifier, 
                                                         node.name,
                                                         node.profile, 
                                                         this,
                                                         node.contextName,
                                                         node.context,
                                                         node.aware_application,
                                                         node.stop_timeout,
                                                         CosNaming::NamingContext::_nil());
    RH_TRACE(this->_baseLog, "Restored " << node.connections.size() << " connections");

    application->populateApplication(node.componentDevices,
                                     node.connections,
                                     node.allocationIDs);

    // Restore various state about the components in the waveform
    BOOST_FOREACH(ossie::ComponentNode& compNode, node.components) {
        redhawk::ApplicationComponent* component = application->addComponent(compNode.identifier, compNode.softwareProfile);
        component->setName(compNode.name);
        component->setNamingContext(compNode.namingContext);
        component->setImplementationId(compNode.implementationId);
        component->setVisible(compNode.isVisible);
        BOOST_FOREACH(const std::string& filename, compNode.loadedFiles) {
            component->addLoadedFile(filename);
        }
        component->setProcessId(compNode.processId);
        component->setComponentObject(compNode.componentObject);
        ossie::DeviceList::iterator device = findDeviceById(compNode.assignedDeviceId);
        if (device == _registeredDevices.end()) {
            RH_WARN(this->_baseLog, "Could not find assigned device '" << compNode.assignedDeviceId
                     << "' for application '" << node.name
                     << "' component '" << compNode.identifier);
        } else {
            component->setAssignedDevice(*device);
        }
        if (!compNode.componentHostId.empty()) {
            redhawk::ApplicationComponent* host = application->findComponent(compNode.componentHostId);
            if (!host) {
                RH_WARN(this->_baseLog, "Could not find component host " << compNode.componentHostId);
            } else {
                component->setComponentHost(host);
            }
        }
    }
    application->setAssemblyController(node.assemblyControllerId);
    application->setStartOrder(node.startOrder);

    // Add external ports
    for (std::map<std::string, CORBA::Object_var>::const_iterator it = node.ports.begin();
         it != node.ports.end();
         ++it) {
        application->addExternalPort(it->first, it->second);
    }

    // Add external properties
    for (std::map<std::string, externalPropertyType>::const_iterator it = node.properties.begin();
         it != node.properties.end();
         ++it) {
        std::string extId = it->first;
        std::string propId = it->second.property_id;
        std::string access = it->second.access;
        application->addExternalProperty(propId, extId, access, it->second.component);
    }

    return application;
}

void DomainManager_impl::_persistApplication(Application_impl* application)
{
    ApplicationNode appNode;
    appNode.name = application->getName();
    appNode.identifier = application->getIdentifier();
    appNode.profile = application->getProfile();
    appNode.contextName = application->_waveformContextName;
    appNode.context = CosNaming::NamingContext::_duplicate(application->_waveformContext);
    appNode.componentDevices = application->_componentDevices;
    BOOST_FOREACH(redhawk::ApplicationComponent& component, application->_components) {
        ossie::ComponentNode compNode;
        compNode.identifier = component.getIdentifier();
        compNode.name = component.getName();
        compNode.softwareProfile = component.getSoftwareProfile();
        compNode.namingContext = component.getNamingContext();
        compNode.implementationId = component.getImplementationId();
        compNode.isVisible = component.isVisible();
        compNode.loadedFiles = component.getLoadedFiles();
        compNode.processId = component.getProcessId();
        compNode.componentObject = component.getComponentObject();
        compNode.assignedDeviceId = component.getAssignedDevice()->identifier;
        if (component.getComponentHost()) {
            compNode.componentHostId = component.getComponentHost()->getIdentifier();
        }
        appNode.components.push_back(compNode);
    }

    // If an assembly controller is set, store it by identifier
    redhawk::ApplicationComponent* assembly_controller = application->getAssemblyController();
    if (assembly_controller) {
        appNode.assemblyControllerId = assembly_controller->getIdentifier();
    }

    // Save the start order, storing just the component identifiers
    BOOST_FOREACH(redhawk::ApplicationComponent* component, application->_startOrder) {
        appNode.startOrder.push_back(component->getIdentifier());
    }

    appNode.allocationIDs = application->_allocationIDs;
    appNode.connections = application->_connections;
    appNode.aware_application = application->_isAware;
    appNode.stop_timeout = application->_stopTimeout;
    appNode.ports = application->_ports;
    appNode.properties = application->_properties;

    _runningApplications.push_back(appNode);

    try {
        db.store("APPLICATIONS", _runningApplications);
    } catch (const ossie::PersistenceException& ex) {
        RH_ERROR(this->_baseLog, "Error persisting change to device managers");
    }
}
