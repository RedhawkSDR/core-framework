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

#include <ossie/debug.h>
#include <ossie/exceptions.h>
#include <ossie/FileManager_impl.h>
#include <ossie/SoftwareAssembly.h>
#include <ossie/CorbaUtils.h>
#include <ossie/EventChannelSupport.h>
#include <ossie/FileStream.h>
#include <ossie/ComponentDescriptor.h>
#include <ossie/DeviceManagerConfiguration.h>

#include "Application_impl.h"
#include "ApplicationFactory_impl.h"
#include "DomainManager_impl.h"
#include "connectionSupport.h"
#include "AllocationManager_impl.h"

using namespace ossie;
using namespace std;

static const ComponentInstantiation* findComponentInstantiation (const std::vector<ComponentPlacement>& placements,
                                                                 const std::string& identifier)
{
    for (std::vector<ComponentPlacement>::const_iterator iter = placements.begin(); iter != placements.end(); ++iter) {
        const std::vector<ComponentInstantiation>& instantiations = iter->getInstantiations();
        for (std::vector<ComponentInstantiation>::const_iterator ii = instantiations.begin(); ii != instantiations.end(); ++ii) {
            if (identifier == ii->getID()) {
                return &(*ii);
            }
        }
    }
    return 0;
}

PREPARE_LOGGING(DomainManager_impl)

// If _overrideDomainName == NULL read the domain name from the DMD file
DomainManager_impl::DomainManager_impl (const char* _dmdFile, const char* _rootpath, const char* domainName, const char* _logconfig_uri, const char* db_uri) :
    _connectionManager(this, this, domainName)
{

    TRACE_ENTER(DomainManager_impl)


    LOG_TRACE(DomainManager_impl, "Looking for DomainManager POA");
    poa = ossie::corba::RootPOA()->find_POA("DomainManager", 1);

    _domainManagerProfile = _dmdFile;

    _applications.length(0);

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

    // Create file manager and register with the parent POA.
    FileManager_impl* fileMgr_servant = new FileManager_impl (_rootpath);
    std::string fileManagerId = _domainName + "/FileManager";
    PortableServer::ObjectId_var oid = ossie::corba::activatePersistentObject(poa, fileMgr_servant, fileManagerId);
    fileMgr_servant->_remove_ref();
    _fileMgr = fileMgr_servant->_this();
    
    // Create allocation manager and register with the parent POA
    _allocationMgr = new AllocationManager_impl (this);
    std::string allocationManagerId = _domainName + "/AllocationManager";
    oid = ossie::corba::activatePersistentObject(poa, _allocationMgr, allocationManagerId);
    _allocationMgr->_remove_ref();
    
    // Parse the DMD profile
    parseDMDProfile();

    LOG_TRACE(DomainManager_impl, "Setting domain name to " << domainName);
    this->_domainName = domainName;

    LOG_TRACE(DomainManager_impl, "Establishing domain manager naming context")
    base_context = ossie::corba::stringToName(_domainName);
    CosNaming::NamingContext_ptr inc = CosNaming::NamingContext::_nil();
    try {
        inc = ossie::corba::InitialNamingContext();
    } catch ( ... ) {
        LOG_FATAL(DomainManager_impl, "Unable to find Naming Service; make sure that it is configured correctly and running.");
        exit(EXIT_FAILURE);
    }
    try { ///\todo review this code and see what alternative solutions exist
        rootContext = inc->bind_new_context (base_context);
    } catch (CosNaming::NamingContext::AlreadyBound&) {
        LOG_TRACE(DomainManager_impl, "Naming context already exists");
        CORBA::Object_var obj = inc->resolve(base_context);
        rootContext = CosNaming::NamingContext::_narrow(obj);
        try {
            cleanupDomainNamingContext(rootContext);
        } catch (CORBA::Exception& e) {
            LOG_FATAL(DomainManager_impl, "Stopping domain manager; error cleaning up context for domain due to: " << e._name());
            exit(EXIT_FAILURE);
        }
    } catch ( ... ) {
        LOG_FATAL(DomainManager_impl, "Stopping domain manager; error creating new context for domain " << this->_domainName.c_str())
        exit(EXIT_FAILURE);
    }

/// \todo lookup and install any services specified in the DMD

    this->_fullDomainManagerName = this->_domainName + string("/") + this->_domainName;

    /*******************************************************

    At some point, memory has to be added to the system,
    where the domain manager recalls the last configuration

    *******************************************************/

//      create Incoming Domain Management and Outgoing Domain Management event channels
    LOG_TRACE(DomainManager_impl, "Creating event channels");
    createEventChannels ();

    vector < ossie::EventChannelNode >::iterator p = _eventChannels.begin();
    while (p != _eventChannels.end()) {
        if ((*p).name == std::string(_domainName+".ODM_Channel")) {
            if (!CORBA::is_nil((*p).channel)) {
                connectToOutgoingEventChannel();
            } else {
                LOG_WARN(DomainManager_impl, "Disabling outgoing events");
            }
        }
        if ((*p).name == std::string(_domainName+".IDM_Channel")) {
            if (!CORBA::is_nil((*p).channel)) {
                connectToIncomingEventChannel();
            } else {
                LOG_WARN(DomainManager_impl, "Disabling incoming events");
            }
        }
        p++;
    }

    LOG_TRACE(DomainManager_impl, "Looking for ApplicationFactories POA");
    appFact_poa = poa->find_POA("ApplicationFactories", 1);

    LOG_TRACE(DomainManager_impl, "Done instantiating Domain Manager")
    TRACE_EXIT(DomainManager_impl)
}

void DomainManager_impl::parseDMDProfile()
{
    try {
        LOG_TRACE(DomainManager_impl, "Loading domain manager configuration from " << _domainManagerProfile);
        File_stream dmdStream(_fileMgr, _domainManagerProfile.c_str());
        _configuration.load(dmdStream);
        dmdStream.close();
    } catch (const parser_error& e) {
        LOG_FATAL(DomainManager_impl, "Stopping domain manager; error parsing domain manager configuration " <<  _domainManagerProfile << "; " << e.what())
        exit(EXIT_FAILURE);
    } catch (const std::ios_base::failure& e) {
        LOG_FATAL(DomainManager_impl, "Stopping domain manager; IO error reading domain manager configuration; " << e.what())
        exit(EXIT_FAILURE);
    } catch( CF::InvalidFileName& _ex ) {
        LOG_FATAL(DomainManager_impl, "Stopping domain manager; invalid domain manager configuration file name; " << _ex.msg)
        exit(EXIT_FAILURE);
    } catch( CF::FileException& _ex ) {
        LOG_FATAL(DomainManager_impl, "Stopping domain manager; file error while opening domain manager configuration; " << _ex.msg)
        exit(EXIT_FAILURE);
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while loading domain manager configuration from " << _domainManagerProfile;
        LOG_FATAL(DomainManager_impl, eout.str())
        exit(EXIT_FAILURE);
    } catch ( CORBA::Exception& ex ) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while loading domain manager configuration from " << _domainManagerProfile;
        LOG_FATAL(DomainManager_impl, eout.str())
        exit(EXIT_FAILURE);
    }
}

void DomainManager_impl::restoreState(const std::string& _db_uri) {
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    LOG_INFO(DomainManager_impl, "Restoring state from URL " << _db_uri);
    try {
        db.open(_db_uri);
    } catch (ossie::PersistenceException& e) {
        LOG_ERROR(DomainManager_impl, "Error loading persistent state: " << e.what());
        return;
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while loading "<<_db_uri;
        LOG_ERROR(DomainManager_impl, eout.str())
        return;
    } catch ( CORBA::Exception& ex ) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while loading "<<_db_uri;
        LOG_ERROR(DomainManager_impl, eout.str())
        return;
    }

    LOG_TRACE(DomainManager_impl, "Recovering event channels");
    // Recover the event channels
    std::vector<EventChannelNode> _restoredEventChannels;
    try {
        db.fetch("EVENT_CHANNELS", _restoredEventChannels, true);
    } catch (ossie::PersistenceException& e) {
        LOG_ERROR(DomainManager_impl, "Error loading event channels persistent state: " << e.what());
        _restoredEventChannels.clear();
    } catch ( std::exception& ex ) {
        _restoredEventChannels.clear();
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while recovering the event channels";
        LOG_ERROR(DomainManager_impl, eout.str())
    } catch ( CORBA::Exception& ex ) {
        _restoredEventChannels.clear();
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while recovering the event channels";
        LOG_ERROR(DomainManager_impl, eout.str())
    }

    for (std::vector<EventChannelNode>::iterator i = _restoredEventChannels.begin();
         i != _restoredEventChannels.end();
         ++i) {
        LOG_TRACE(DomainManager_impl, "Attempting to recover connection to Event Channel " << i->boundName);
        try {
            if (ossie::corba::objectExists(i->channel)) {
                LOG_INFO(DomainManager_impl, "Recovered connection to Event Channel: " << i->boundName);
                CosEventChannelAdmin::EventChannel_var channel = i->channel;
                CosNaming::Name_var cosName = ossie::corba::stringToName(i->boundName);
                try {
                    // Use rebind to force the naming service to replace any existing object with the same name.
                    rootContext->rebind(cosName, channel);
                } catch ( ... ) {
                    channel = ossie::event::connectToEventChannel(rootContext, i->boundName);
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
                LOG_WARN(DomainManager_impl, "Failed to recover Event Channel: " << i->boundName);
            }
        } CATCH_LOG_WARN(DomainManager_impl, "Unable to restore connection to Event Channel: " << i->boundName);
    }

    LOG_DEBUG(DomainManager_impl, "Recovering device manager connections");
    // Recover device manager connections and consume the value so that
    // the persistence store no longer has any device manager stored
    DeviceManagerList _restoredDeviceManagers;
    try {
        db.fetch("DEVICE_MANAGERS", _restoredDeviceManagers, true);
    } catch (ossie::PersistenceException& e) {
        LOG_ERROR(DomainManager_impl, "Error loading device managers persistent state: " << e.what());
        _restoredDeviceManagers.clear();
    } catch ( std::exception& ex ) {
        _restoredDeviceManagers.clear();
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while recovering the device manager connections";
        LOG_ERROR(DomainManager_impl, eout.str())
    } catch ( CORBA::Exception& ex ) {
        _restoredDeviceManagers.clear();
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while recovering the device manager connections";
        LOG_ERROR(DomainManager_impl, eout.str())
    }

    for (DeviceManagerList::iterator ii = _restoredDeviceManagers.begin(); ii != _restoredDeviceManagers.end(); ++ii) {
        LOG_TRACE(DomainManager_impl, "Attempting to recover connection to Device Manager " << ii->identifier << " " << ii->label);
        try {
            if (ossie::corba::objectExists(ii->deviceManager)) {
                LOG_INFO(DomainManager_impl, "Recovered connection to Device Manager: " << ii->identifier << " " << ii->label);
                addDeviceMgr(ii->deviceManager);
                mountDeviceMgrFileSys(ii->deviceManager);
            } else {
                LOG_WARN(DomainManager_impl, "Failed to recover connection to Device Manager: " << ii->label << ": device manager servant no longer exists");
            }
        } CATCH_LOG_WARN(DomainManager_impl, "Unable to restore connection to DeviceManager: " << ii->label);
    }

    LOG_DEBUG(DomainManager_impl, "Recovering device connections");
    DeviceList _restoredDevices;
    try {
        db.fetch("DEVICES", _restoredDevices, true);
    } catch (ossie::PersistenceException& e) {
        LOG_ERROR(DomainManager_impl, "Error loading devices persistent state: " << e.what());
        _restoredDevices.clear();
    } catch ( std::exception& ex ) {
        _restoredDevices.clear();
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while recovering the device connections";
        LOG_ERROR(DomainManager_impl, eout.str())
    } catch ( CORBA::Exception& ex ) {
        _restoredDevices.clear();
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while recovering the device connections";
        LOG_ERROR(DomainManager_impl, eout.str())
    }

    for (DeviceList::iterator iter = _restoredDevices.begin(); iter != _restoredDevices.end(); ++iter) {
        boost::shared_ptr<DeviceNode> i = *iter;
        LOG_TRACE(DomainManager_impl, "Attempting to recover connection to Device " << i->identifier << " " << i->label);
        try {
            if (ossie::corba::objectExists(i->device)) {
                LOG_INFO(DomainManager_impl, "Recovered connection to Device: " << i->identifier << " " << i->label);
                if (ossie::corba::objectExists(i->devMgr.deviceManager)) {
                    storeDeviceInDomainMgr(i->device, i->devMgr.deviceManager);
                } else {
                    LOG_WARN(DomainManager_impl, "Failed to recover connection to Device: " << i->identifier << ": device manager no longer exists");
                }
            } else {
                LOG_WARN(DomainManager_impl, "Failed to recover connection to Device: " << i->identifier << ": device servant no longer exists");
            }
        } CATCH_LOG_WARN(DomainManager_impl, "Unable to restore connection to Device: " << i->identifier);
    }

    LOG_DEBUG(DomainManager_impl, "Recovering registered services");
    ServiceList _restoredServices;
    try {
        db.fetch("SERVICES", _restoredServices, true);
    } catch (ossie::PersistenceException& e) {
        LOG_ERROR(DomainManager_impl, "Error loading services persistent state: " << e.what());
        _restoredServices.clear();
    } catch ( std::exception& ex ) {
        _restoredServices.clear();
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while recovering the service connections";
        LOG_ERROR(DomainManager_impl, eout.str())
    } catch ( CORBA::Exception& ex ) {
        _restoredServices.clear();
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while recovering the service connections";
        LOG_ERROR(DomainManager_impl, eout.str())
    }

    for (ServiceList::iterator ii = _restoredServices.begin(); ii != _restoredServices.end(); ++ii) {
        LOG_TRACE(DomainManager_impl, "Attempting to recover connection to Service " << ii->name);
        try {
            if (ossie::corba::objectExists(ii->service)) {
                LOG_INFO(DomainManager_impl, "Recovered connection to Service: " << ii->name);
                ossie::DeviceManagerList::iterator deviceManager = findDeviceManagerById(ii->deviceManagerId);
                if (deviceManager != _registeredDeviceManagers.end()) {
                    storeServiceInDomainMgr(ii->service, deviceManager->deviceManager, ii->name.c_str(), ii->serviceId.c_str());
                } else {
                    LOG_WARN(DomainManager_impl, "Failed to recover connection to Service: " << ii->name << ": DeviceManager "
                             << ii->deviceManagerId << " no longer exists");
                }
            } else {
                LOG_WARN(DomainManager_impl, "Failed to recover connection to Service: " << ii->name << ": servant no longer exists");
            }
        } CATCH_LOG_WARN(DomainManager_impl, "Unable to restore connection to Service: " << ii->name);
    }

    LOG_DEBUG(DomainManager_impl, "Recovering DCD connections");
    ConnectionTable _restoredConnections;
    try {
        db.fetch("CONNECTIONS", _restoredConnections, true);
    } catch (ossie::PersistenceException& e) {
        LOG_ERROR(DomainManager_impl, "Error loading services persistent state: " << e.what());
        _restoredConnections.clear();
    } catch ( std::exception& ex ) {
        _restoredConnections.clear();
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while recovering the device manager connections";
        LOG_ERROR(DomainManager_impl, eout.str())
    } catch ( CORBA::Exception& ex ) {
        _restoredConnections.clear();
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while recovering the device manager connections";
        LOG_ERROR(DomainManager_impl, eout.str())
    }

    for (ConnectionTable::iterator ii = _restoredConnections.begin(); ii != _restoredConnections.end(); ++ii) {
        const std::string& deviceManagerId = ii->first;
        const ConnectionList& connections = ii->second;
        LOG_TRACE(DomainManager_impl, "Restoring port connections for DeviceManager " << deviceManagerId);
        if (findDeviceManagerById(deviceManagerId) == _registeredDeviceManagers.end()) {
            LOG_WARN(DomainManager_impl, "DeviceManager " << deviceManagerId << " no longer exists, not restoring port connections");
            continue;
        }
        for (ConnectionList::const_iterator jj = connections.begin(); jj != connections.end(); ++jj) {
            LOG_TRACE(DomainManager_impl, "Restoring port connection " << jj->identifier);
            _connectionManager.restoreConnection(deviceManagerId, *jj);
        }
    }

    LOG_DEBUG(DomainManager_impl, "Recovering application factories");
    std::vector<ApplicationFactoryNode> _restoredAppFactories;
    try {
        db.fetch("APP_FACTORIES", _restoredAppFactories, true);
    } catch (ossie::PersistenceException& e) {
        LOG_ERROR(DomainManager_impl, "Error loading application factory persistent state: " << e.what());
        _restoredAppFactories.clear();
    } catch ( std::exception& ex ) {
        _restoredAppFactories.clear();
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while recovering the application factories";
        LOG_ERROR(DomainManager_impl, eout.str())
    } catch ( CORBA::Exception& ex ) {
        _restoredAppFactories.clear();
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while recovering the application factories";
        LOG_ERROR(DomainManager_impl, eout.str())
    }

    for (std::vector<ApplicationFactoryNode>::iterator i = _restoredAppFactories.begin();
         i != _restoredAppFactories.end();
         ++i) {
        LOG_TRACE(DomainManager_impl, "Attempting to restore application factory " << i->profile);
        try {
            installApplication(i->profile.c_str());
            LOG_INFO(DomainManager_impl, "Restored application factory " << i->profile);
        } CATCH_LOG_WARN(DomainManager_impl, "Failed to restore application factory " << i->profile);
    }

    LOG_DEBUG(DomainManager_impl, "Recovering applications");
    std::vector<ApplicationNode> _restoredApplications;
    try {
        db.fetch("APPLICATIONS", _restoredApplications, true);
    } catch (ossie::PersistenceException& e) {
        LOG_ERROR(DomainManager_impl, "Error loading application persistent state: " << e.what());
        _restoredApplications.clear();
    } catch ( std::exception& ex ) {
        _restoredApplications.clear();
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while recovering the applications";
        LOG_ERROR(DomainManager_impl, eout.str())
    } catch ( CORBA::Exception& ex ) {
        _restoredApplications.clear();
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while recovering the applications";
        LOG_ERROR(DomainManager_impl, eout.str())
    }

    for (std::vector<ApplicationNode>::iterator i = _restoredApplications.begin();
         i != _restoredApplications.end();
         ++i) {
        LOG_TRACE(DomainManager_impl, "Attempting to restore application  " << i->name << " " << i->identifier << " " << i->profile);
        try {
            CosNaming::NamingContext_var context = CosNaming::NamingContext::_narrow(ossie::corba::stringToObject(i->contextIOR));
            if (ossie::corba::objectExists(context)) {
                LOG_TRACE(DomainManager_impl, "Creating application " << i->identifier << " " << _domainName << " " << i->contextName << " " << i->contextIOR);
                Application_impl* _application = new Application_impl (i->identifier.c_str(), i->name.c_str(), i->profile.c_str(), this, i->contextName, context);
                LOG_TRACE(DomainManager_impl, "Restored " << i->connections.size() << " connections");

                i->_startOrder.resize(i->componentIORS.size());
                for (unsigned int ii = 0; ii < i->componentIORS.size(); ++ii) {
                    CORBA::Object_var obj = ossie::corba::stringToObject(i->componentIORS[ii]);
                    if (!CORBA::is_nil(obj)) {
                        i->_startOrder[ii] = CF::Resource::_duplicate(ossie::corba::_narrowSafe<CF::Resource> (obj));
                    }
                }

                _application->populateApplication(i->assemblyController,
                                                  i->componentDevices,
                                                  &(i->componentImplementations),
                                                  i->_startOrder,
                                                  &(i->componentNamingContexts),
                                                  &(i->componentProcessIds),
                                                  i->connections,
                                                  i->fileTable,
                                                  i->allocationIDs);

                // Recover and register components
                for (unsigned int ii = 0; ii < i->components.size() ; ++ii) {
                    CF::ComponentType c;
                    c.identifier = i->components[ii].identifier.c_str();
                    c.softwareProfile = i->components[ii].softwareProfile.c_str();
                    c.type = CF::APPLICATION_COMPONENT;
                    c.componentObject = ossie::corba::stringToObject(i->components[ii].ior);
                    _application->registerComponent(c);
                }

                // Add external ports
                for (std::map<std::string, CORBA::Object_var>::const_iterator it = i->ports.begin();
                        it != i->ports.end();
                        ++it) {
                    _application->addExternalPort(it->first, it->second);
                }

                // Add external properties
                for (std::map<std::string, std::pair<std::string, std::string> >::const_iterator it = i->properties.begin();
                        it != i->properties.end();
                        ++it) {
                    std::string extId = it->first;
                    std::string propId = it->second.first;
                    std::string compId = it->second.second;
                    std::vector<CF::Resource_ptr> comps = i->_startOrder;
                    comps.push_back(i->assemblyController);
                    for (unsigned int ii = 0; ii < comps.size(); ++ii) {
                        if (compId == ossie::corba::returnString(comps[ii]->identifier())) {
                            _application->addExternalProperty(propId, extId, comps[ii]);
                            break;
                        }
                    }
                }

                PortableServer::POA_var dm_poa = ossie::corba::RootPOA()->find_POA("DomainManager", 0);
                PortableServer::POA_var poa = dm_poa->find_POA("Applications", 1);
                PortableServer::ObjectId_var app_oid = ossie::corba::activatePersistentObject(poa, _application, i->identifier);

                addApplication(_application);
                _application->_remove_ref();

                LOG_INFO(DomainManager_impl, "Restored application " << i->identifier);
            }
        } CATCH_LOG_WARN(DomainManager_impl, "Failed to restore application" << i->identifier);
    }

    LOG_DEBUG(DomainManager_impl, "Recovering remote domains");
    // Recover domain manager connections and consume the value so that
    // the persistence store no longer has any domain manager stored
    DomainManagerList _restoredDomainManagers;
    try {
        db.fetch("DOMAIN_MANAGERS", _restoredDomainManagers, true);
    } catch (const ossie::PersistenceException& e) {
        LOG_ERROR(DomainManager_impl, "Error loading domain managers persistent state: " << e.what());
        _restoredDomainManagers.clear();
    } catch (const std::exception& ex) {
        _restoredDomainManagers.clear();
        LOG_ERROR(DomainManager_impl, "The following standard exception occurred: "<<ex.what()<<" while recovering the domain manager connections");
    }

    for (DomainManagerList::iterator ii = _restoredDomainManagers.begin(); ii != _restoredDomainManagers.end(); ++ii) {
        LOG_TRACE(DomainManager_impl, "Attempting to recover connection to domain '" << ii->name << "'");
        try {
            if (ossie::corba::objectExists(ii->domainManager)) {
                LOG_INFO(DomainManager_impl, "Recovered connection to domain '" << ii->name << "'");
                addDomainMgr(ii->domainManager);
            } else {
                LOG_WARN(DomainManager_impl, "Failed to recover connection to domain '" << ii->name << "': domain manager object no longer exists");
            }
        } CATCH_LOG_WARN(DomainManager_impl, "Unable to restore connection to domain '" << ii->name << "'");
    }

    LOG_DEBUG(DomainManager_impl, "Recovering allocation manager");
    ossie::AllocationTable _restoredLocalAllocations;
    try {
        db.fetch("LOCAL_ALLOCATIONS", _restoredLocalAllocations);
    } catch (ossie::PersistenceException& e) {
        LOG_ERROR(DomainManager_impl, "Error loading local allocation persistent state: " << e.what());
    } catch (std::exception& ex) {
        LOG_ERROR(DomainManager_impl, "The following standard exception occurred: " << ex.what()
                  << " while recovering local allocations");
    }
    _allocationMgr->restoreLocalAllocations(_restoredLocalAllocations);

    ossie::RemoteAllocationTable _restoredRemoteAllocations;
    try {
        db.fetch("REMOTE_ALLOCATIONS", _restoredRemoteAllocations);
    } catch (const ossie::PersistenceException& e) {
        LOG_ERROR(DomainManager_impl, "Error loading remote allocations persistent state: " << e.what());
    } catch (const std::exception& ex) {
        LOG_ERROR(DomainManager_impl, "The following standard exception occurred: " << ex.what()
                  << " while recovering remote allocations");
    }
    _allocationMgr->restoreRemoteAllocations(_restoredRemoteAllocations);

    if (_restoredLocalAllocations.empty() && _restoredRemoteAllocations.empty()) {
        // Migration from 1.10.0 to 1.10.1; database format was changed, so if
        // no local or remote allocations were restored, try to recover from the
        // old format
        AllocationManagerNode _restoredAllocations;
        try {
            db.fetch("ALLOCATION_MANAGER", _restoredAllocations, true);
        } catch (const ossie::PersistenceException& e) {
            LOG_ERROR(DomainManager_impl, "Error loading allocation manager persistent state: " << e.what());
        } catch ( std::exception& ex ) {
            std::ostringstream eout;
            eout << "The following standard exception occurred: "<<ex.what()<<" while recovering the allocation manager";
            LOG_ERROR(DomainManager_impl, eout.str());
        } catch ( CORBA::Exception& ex ) {
            std::ostringstream eout;
            eout << "The following CORBA exception occurred: "<<ex._name()<<" while recovering the allocation manager";
            LOG_ERROR(DomainManager_impl, eout.str());
        }
        if (!_restoredAllocations._allocations.empty() || !_restoredAllocations._remoteAllocations.empty()) {
            LOG_DEBUG(DomainManager_impl, "Migrating old allocation state");
            _allocationMgr->restoreAllocations(_restoredAllocations._allocations, _restoredAllocations._remoteAllocations);
        }
    }

    LOG_DEBUG(DomainManager_impl, "Done restoring state from URL " << _db_uri);
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

            if (obj->_non_existent()) {
                // If it no longer exists, unbind it
                LOG_TRACE(DomainManager_impl, "Unbinding naming context which no longer exists; this is probably due to an omniNames bug")
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
    while (_applicationFactories.length() > 0) {
        CORBA::String_var appFactoryId = _applicationFactories[0]->identifier();
        this->uninstallApplication(appFactoryId);
    }
    while (_applications.length() > 0) {
        unsigned int lenApps = _applications.length();
        try {
            _applications[0]->releaseObject();
        } catch ( ... ) {
            std::string message("Error releasing application ");
            message += _applications[0]->name();
            if (lenApps == _applications.length()) {
                for (unsigned int i=0; i<(_applications.length()-1); i++) {
                    _applications[i] = _applications[i+1];
                }
                _applications.length(_applications.length()-1);
            }
            LOG_TRACE(DomainManager_impl, message)
        }
    }
}


void DomainManager_impl::shutdownAllDeviceManagers()
{
    while (_registeredDeviceManagers.size() > 0) {
        unsigned int lenRegDevMgr = _registeredDeviceManagers.size();
        CF::DeviceManager_var devMgr = (*_registeredDeviceManagers.begin()).deviceManager;
        try {
            devMgr->shutdown();
        } catch ( ... ) {
            std::string message("Error shutting down Device Manager ");
            message += (*_registeredDeviceManagers.begin()).label;
            if (lenRegDevMgr == _registeredDeviceManagers.size()) {
                _registeredDeviceManagers.erase(_registeredDeviceManagers.begin());
            }
            LOG_TRACE(DomainManager_impl, message)
        }
    }
}


void DomainManager_impl::shutdown (int signal)
{
    TRACE_ENTER(DomainManager_impl)

    if ((not ossie::corba::isPersistenceEnabled()) or ((ossie::corba::isPersistenceEnabled()) and (signal == SIGINT))) {
        releaseAllApplications();
        shutdownAllDeviceManagers();
        destroyEventChannels ();
    }

    boost::recursive_mutex::scoped_lock lock(stateAccess);
    db.close();

    PortableServer::ObjectId_var oid;

    for (std::vector<ApplicationFactoryNode>::iterator appFactNode = _installedApplications.begin();
            appFactNode != _installedApplications.end();
            ++appFactNode) {
        appFact_poa->deactivate_object(appFactNode->servant_id);
    }
    appFact_poa->destroy(false, true);

    // Deactivate and destroy the AllocationManager
    oid = poa->servant_to_id(_allocationMgr);
    poa->deactivate_object(oid);

    oid = poa->reference_to_id(_fileMgr);
    poa->deactivate_object(oid);

    // The domain manager does not eliminate the naming context that it was using unless it's empty. The assumption
    //  is that it can be re-started and anything that was running before re-associated
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


DomainManager_impl::~DomainManager_impl ()
{
    // Don't make CORBA calls in the destructor because
    // the servant is being deleted after the ORB
    // has shutdown in the nodebooter

    TRACE_ENTER(DomainManager_impl)

    /**************************************************
     *    Save current state for configuration recall   *
     *    this is not supported by this version          *
     **************************************************/

    TRACE_EXIT(DomainManager_impl)
}

char *
DomainManager_impl::identifier (void)
throw (CORBA::SystemException)
{
    TRACE_ENTER(DomainManager_impl)

    TRACE_EXIT(DomainManager_impl)
    return CORBA::string_dup(_configuration.getID());
}

char *
DomainManager_impl::name (void)
throw (CORBA::SystemException)
{
    TRACE_ENTER(DomainManager_impl)

    TRACE_EXIT(DomainManager_impl)
    return CORBA::string_dup(this->_domainName.c_str());
}


char *
DomainManager_impl::domainManagerProfile (void)
throw (CORBA::SystemException)
{
    TRACE_ENTER(DomainManager_impl)

    TRACE_EXIT(DomainManager_impl)
    return CORBA::string_dup(_domainManagerProfile.c_str());
}


CF::AllocationManager_ptr DomainManager_impl::allocationMgr (void) throw (CORBA::
                                                              SystemException)
{
    TRACE_ENTER(DomainManager_impl)

    TRACE_EXIT(DomainManager_impl)
    return _allocationMgr->_this();
}


CF::FileManager_ptr DomainManager_impl::fileMgr (void) throw (CORBA::
SystemException)
{
    TRACE_ENTER(DomainManager_impl)
    
    TRACE_EXIT(DomainManager_impl)
    return CF::FileManager::_duplicate(_fileMgr);
}


CF::DomainManager::ApplicationFactorySequence *
DomainManager_impl::applicationFactories (void) throw (CORBA::
                                                       SystemException)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    CF::DomainManager::ApplicationFactorySequence_var result = new CF::DomainManager::ApplicationFactorySequence(_applicationFactories);

    TRACE_EXIT(DomainManager_impl)
    return result._retn();
}


CF::DomainManager::ApplicationSequence *
DomainManager_impl::applications (void) throw (CORBA::SystemException)
{
    TRACE_ENTER(DomainManager_impl);
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    CF::DomainManager::ApplicationSequence_var result = new CF::DomainManager::ApplicationSequence(_applications);
    TRACE_EXIT(DomainManager_impl)
    return result._retn();
}


CF::DomainManager::DeviceManagerSequence *
DomainManager_impl::deviceManagers (void) throw (CORBA::SystemException)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    CF::DomainManager::DeviceManagerSequence_var result = new CF::DomainManager::DeviceManagerSequence();
    result->length(_registeredDeviceManagers.size());
    DeviceManagerList::iterator deviceManager = _registeredDeviceManagers.begin();
    for (CORBA::ULong ii = 0; ii < result->length(); ++ii, ++deviceManager) {
        result[ii] = CF::DeviceManager::_duplicate(deviceManager->deviceManager);
    }

    TRACE_EXIT(DomainManager_impl)
    return result._retn();
}


CF::DomainManager::DomainManagerSequence *
DomainManager_impl::remoteDomainManagers (void) throw (CORBA::SystemException)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    CF::DomainManager::DomainManagerSequence_var result = new CF::DomainManager::DomainManagerSequence();
    result->length(this->_registeredDomainManagers.size());
    DomainManagerList::iterator dmnMgr = this->_registeredDomainManagers.begin();
    for (CORBA::ULong ii = 0; ii < result->length(); ++ii, ++dmnMgr) {
        result[ii] = CF::DomainManager::_duplicate(dmnMgr->domainManager);
    }

    TRACE_EXIT(DomainManager_impl)
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
        std::string identifier = ossie::corba::returnString(domainMgr->identifier());
        DomainManagerList::iterator node = findDomainManagerById(identifier);
        if (node != _registeredDomainManagers.end()) {
            if (!ossie::corba::objectExists(node->domainManager)) {
                LOG_WARN(DomainManager_impl, "Cleaning up registration of dead device manager: " << identifier);
                //catastrophicUnregisterDeviceManager(node);
                LOG_TRACE(DomainManager_impl, "Continuing with registration of new device manager: " << identifier);
            } else {
                bool DomMgr_alive = false;
                try {
                    CORBA::String_var identifier = domainMgr->identifier();
                    DomMgr_alive = true;
                } catch ( ... ) {
                    LOG_WARN(DomainManager_impl, "Cleaning up registration of dead device manager: " << identifier);
                    //catastrophicUnregisterDeviceManager(node);
                    LOG_TRACE(DomainManager_impl, "Continuing with registration of new device manager: " << identifier);
                }
                if (DomMgr_alive) {
                    ostringstream eout;
                    eout << "Attempt re-register existing domain manager: " << identifier;
                    LOG_ERROR(DomainManager_impl, eout.str());
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
        LOG_ERROR(DomainManager_impl, "Cannot unregister nil DomainManager");
        throw CF::InvalidObjectReference("Cannot unregister nil DomainManager");
    }

    DomainManagerList::iterator domMgrIter = findDomainManagerByObject(domainMgr);
    if (domMgrIter == _registeredDomainManagers.end()) {
        LOG_WARN(DomainManager_impl, "Ignoring attempt to unregister domain manager that was not registered with this domain");
        return;
    }

    if (!domMgrIter->domainManager->_is_equivalent(domainMgr)) {
        LOG_TRACE(DomainManager_impl, "Ignoring attempt to unregister domain manager with same identifier but different object");
        return;
    }

    _registeredDomainManagers.erase(domMgrIter);
    try {
        db.store("DOMAIN_MANAGERS", _registeredDomainManagers);
    } catch (const ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting change to domain managers");
    } catch (const std::exception& ex) {
        LOG_ERROR(DomainManager_impl, "The following standard exception occurred: "<<ex.what()<<" persisting the domain managers");
    }
}


void
DomainManager_impl::registerDeviceManager (CF::DeviceManager_ptr deviceMgr)
throw (CORBA::SystemException, CF::InvalidObjectReference, CF::InvalidProfile,
       CF::DomainManager::RegisterError)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    try {
        _local_registerDeviceManager(deviceMgr);
    } catch ( ... ) {
        throw;
    }

    CORBA::String_var identifier = deviceMgr->identifier();
    CORBA::String_var label = deviceMgr->label();
    ossie::sendObjectAddedEvent(DomainManager_impl::__logger, _configuration.getID(), identifier, label,
            deviceMgr, StandardEvent::DEVICE_MANAGER, proxy_consumer);
}

void
DomainManager_impl::_local_registerDeviceManager (CF::DeviceManager_ptr deviceMgr)
throw (CORBA::SystemException, CF::InvalidObjectReference, CF::InvalidProfile,
       CF::DomainManager::RegisterError)
{
    TRACE_ENTER(DomainManager_impl)
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
            LOG_WARN(DomainManager_impl, "Cleaning up registration of dead device manager: " << identifier);
            catastrophicUnregisterDeviceManager(node);
            LOG_TRACE(DomainManager_impl, "Continuing with registration of new device manager: " << identifier);
        } else {
            bool DevMgr_alive = false;
            try {
                CORBA::String_var identifier = deviceMgr->identifier();
                DevMgr_alive = true;
            } catch ( ... ) {
                LOG_WARN(DomainManager_impl, "Cleaning up registration of dead device manager: " << identifier);
                catastrophicUnregisterDeviceManager(node);
                LOG_TRACE(DomainManager_impl, "Continuing with registration of new device manager: " << identifier);
            }
            if (DevMgr_alive) {
                ostringstream eout;
                eout << "Attempt re-register existing device manager: " << identifier;
                LOG_ERROR(DomainManager_impl, eout.str());
                throw CF::DomainManager::RegisterError(CF::CF_NOTSET, eout.str().c_str());
            }
        }
    }

    addDeviceMgr (deviceMgr);

    CORBA::String_var devMgrLabel;
    try {
        mountDeviceMgrFileSys(deviceMgr);

        LOG_TRACE(DomainManager_impl, "Adding DeviceManager Devices to list")
        // add all devices under device manager to registereddevice att.
        // associate input deviceMgr with registeredDevices
        addDeviceMgrDevices (deviceMgr);

        LOG_TRACE(DomainManager_impl, "Adding DeviceManager Services to list");
        // add all services under device manager to registeredservices
        // associate input deviceMgr with registeredservices
        // TODO RENABLE
        //addDeviceMgrServices (deviceMgr);

        LOG_TRACE(DomainManager_impl, "Getting connections from DeviceManager DCD");


        DeviceManagerConfiguration dcdParser;
        try {
            CF::FileSystem_var devMgrFileSys = deviceMgr->fileSys();
            CORBA::String_var profile = deviceMgr->deviceConfigurationProfile();
            File_stream dcd(devMgrFileSys, profile);
            dcdParser.load(dcd);
            dcd.close();
        } catch ( ossie::parser_error& e ) {
            LOG_ERROR(DomainManager_impl, "failed device manager registration; error parsing device manager DCD " << deviceMgr->deviceConfigurationProfile() << "; " << e.what())
            throw(CF::DomainManager::RegisterError());
        }

        const std::vector<Connection>& connections = dcdParser.getConnections();

        for(size_t ii = 0; ii < connections.size(); ++ii) {
            _connectionManager.addConnection(dcdParser.getID(), connections[ii]);
        }
        try {
            db.store("CONNECTIONS", _connectionManager.getConnections());
        } catch (ossie::PersistenceException& ex) {
            LOG_ERROR(DomainManager_impl, "Error persisting change to device manager connections");
        }
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while recovering the change to device manager connections";
        LOG_ERROR(DomainManager_impl, eout.str())
        removeDeviceManager(findDeviceManagerById(identifier));
        throw CF::DomainManager::RegisterError(CF::CF_NOTSET, eout.str().c_str());
    } catch ( CORBA::Exception& ex ) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while recovering the change to device manager connections";
        LOG_ERROR(DomainManager_impl, eout.str())
        removeDeviceManager(findDeviceManagerById(identifier));
        throw CF::DomainManager::RegisterError(CF::CF_NOTSET, eout.str().c_str());
    } catch ( ... ) {
        // Any exceptions at this point require us to remove the device manager and cleanup any other issues
        removeDeviceManager(findDeviceManagerById(identifier));
        throw CF::DomainManager::RegisterError(CF::CF_NOTSET, "Unexpected error registering device manager");
    }

    LOG_TRACE(DomainManager_impl, "Leaving DomainManager::registerDeviceManager");
    //NOTE: The SCA V2.2 describes that all service connections should be established at this point.
    //That step has been performed by the registerDevice operation (called by addDeviceMgrDevices) and by
    //registerService ( called by addDeviceMgrServices).
    //The registerDevice function establish all the connections required for the registering device and stores
    //any connection when the requested service is not registered yet.
    //The registerService function will establish any pending connection with the registering service.
    //These two functions together will establish all service connections required.

    //Note: In the event of an internal error that prevents the deviceMgr registration from success, the
    //RegisterError exception should be raised and a FAILURE_ALARM log record written to a DomainManager's Log
}


void
DomainManager_impl::addDeviceMgr (CF::DeviceManager_ptr deviceMgr)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    if (!deviceMgrIsRegistered (deviceMgr)) {
        LOG_TRACE(DomainManager_impl, "Adding DeviceManager ref to list")
        DeviceManagerNode tmp_devMgr;
        CORBA::String_var identifier = deviceMgr->identifier();
        CORBA::String_var label = deviceMgr->label();
        tmp_devMgr.deviceManager = CF::DeviceManager::_duplicate(deviceMgr);
        tmp_devMgr.identifier = static_cast<char*>(identifier);
        tmp_devMgr.label = static_cast<char*>(label);
        _registeredDeviceManagers.push_back(tmp_devMgr);

        try {
            db.store("DEVICE_MANAGERS", _registeredDeviceManagers);
        } catch ( ossie::PersistenceException& ex) {
            LOG_ERROR(DomainManager_impl, "Error persisting change to device managers");
        } catch ( std::exception& ex ) {
            LOG_ERROR(DomainManager_impl, "The following standard exception occurred: "<<ex.what()<<" persisting the device managers")
        }
    }
    TRACE_EXIT(DomainManager_impl)
}

void DomainManager_impl::addDomainMgr (CF::DomainManager_ptr domainMgr)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    if (!domainMgrIsRegistered (domainMgr)) {
        LOG_TRACE(DomainManager_impl, "Adding DomainManager ref to list")
        DomainManagerNode node;
        node.domainManager = CF::DomainManager::_duplicate(domainMgr);
        node.identifier = ossie::corba::returnString(domainMgr->identifier());
        node.name = ossie::corba::returnString(domainMgr->name());
        _registeredDomainManagers.push_back(node);

        try {
            db.store("DOMAIN_MANAGERS", _registeredDomainManagers);
        } catch (const ossie::PersistenceException& ex) {
            LOG_ERROR(DomainManager_impl, "Error persisting change to domain managers");
        } catch (const std::exception& ex) {
            LOG_ERROR(DomainManager_impl, "The following standard exception occurred: "<<ex.what()<<" persisting the domain managers");
        }
    }
    TRACE_EXIT(DomainManager_impl)
}


void
DomainManager_impl::addDeviceMgrDevices (CF::DeviceManager_ptr deviceMgr)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    CF::DeviceSequence_var devices = deviceMgr->registeredDevices();

//Call registerDevice for each device in the DeviceMgr
    for (unsigned int i = 0; i < devices->length (); i++) {
        CF::Device_ptr _dev = devices[i];
        _local_registerDevice (_dev, deviceMgr);
    }
//The registerDevice operation will try to establish any service connections specified in the
//deviceMgr's DCD for each device.
    TRACE_EXIT(DomainManager_impl)
}


void
DomainManager_impl::addDeviceMgrServices (CF::DeviceManager_ptr deviceMgr)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    CF::DeviceManager::ServiceSequence_var services = deviceMgr->registeredServices();

//Call registerService for each service in the DeviceMgr
    for (unsigned int i = 0; i < services->length (); i++ ) {
        CF::DeviceManager::ServiceType _serv = services[i];
        _local_registerService ( _serv.serviceObject, deviceMgr, _serv.serviceName);
    }

//The registerDeviceManager operation shall add the input deviceMgr's registeredServices and
//each registeredService's names to the DomainManager. The registerDeviceManager operation
//associates the input deviceMgr's with the input deviceMgr's registeredServices in the
//DomainManager in order to support the unregisterDeviceManager operation.

//Note: The registerService operation will establish any pending connection for the registering service
    TRACE_EXIT(DomainManager_impl)
}

void DomainManager_impl::mountDeviceMgrFileSys (CF::DeviceManager_ptr deviceMgr) {
    string mountPoint = "/";
    CORBA::String_var devMgrLabel;
    // mount filesystem under "/<DomainName>/<deviceMgr.label>"
    devMgrLabel = deviceMgr->label();
    mountPoint += devMgrLabel;
    LOG_TRACE(DomainManager_impl, "Mounting DeviceManager FileSystem at " << mountPoint)

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
        LOG_ERROR(DomainManager_impl, "Cannot unregister nil DeviceManager");
        throw CF::InvalidObjectReference("Cannot unregister nil DeviceManager");
    }

    DeviceManagerList::iterator devMgrIter = findDeviceManagerByObject(deviceMgr);
    if (devMgrIter == _registeredDeviceManagers.end()) {
        LOG_WARN(DomainManager_impl, "Ignoring attempt to unregister device manager that was not registered with this domain");
        return;
    }

    if (!devMgrIter->deviceManager->_is_equivalent(deviceMgr)) {
        LOG_TRACE(DomainManager_impl, "Ignoring attempt to unregister device manager with same identifier but different object");
        return;
    }

    // Save the identifier and label, as the iterator will be invalidated.
    const std::string identifier = devMgrIter->identifier;
    const std::string label = devMgrIter->label;

    try {
        _local_unregisterDeviceManager(devMgrIter);
    } CATCH_LOG_ERROR(DomainManager_impl, "Exception unregistering device manager");

    ossie::sendObjectRemovedEvent(DomainManager_impl::__logger, _configuration.getID(), identifier.c_str(), label.c_str(),
                                  StandardEvent::DEVICE_MANAGER, proxy_consumer);
}

ossie::DeviceManagerList::iterator DomainManager_impl::_local_unregisterDeviceManager (ossie::DeviceManagerList::iterator deviceManager)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    //  For this function, an exception will be raised only when the Device Manager cannot be found
    //  If the individual Device references are bad, the unregistration of the device from the Domain
    //  will be done, but no error will be raised

    // Break and release connections owned by the unregistering DeviceManager.
    _connectionManager.deviceManagerUnregistered(deviceManager->identifier);
    try {
        db.store("CONNECTIONS", _connectionManager.getConnections());
    } catch (ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting change to device manager connections");
    } catch ( std::exception& ex ) {
        LOG_ERROR(DomainManager_impl, "The following standard exception occurred: "<<ex.what()<<" persisting the connections")
    } catch ( const CORBA::Exception& ex ) {
        LOG_ERROR(DomainManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" persisting the connections")
    }

    // Release all devices and services, which may break connections from other
    // DeviceManagers' DCDs.
    removeDeviceManagerDevices(deviceManager->identifier);
    removeDeviceManagerServices(deviceManager->identifier);

    // Unmount all DeviceManager FileSystems from the FileManager; currently just
    // the DeviceManager's root file system.
    string mountPoint = "/" + deviceManager->label;
    LOG_TRACE(DomainManager_impl, "Unmounting DeviceManager FileSystem at " << mountPoint)
    try {
        _fileMgr->unmount(mountPoint.c_str());
    } CATCH_LOG_ERROR(DomainManager_impl, "Unmounting DeviceManager FileSystem failed during unregistration");

    // Remove the DeviceManager from the domain.
    deviceManager = removeDeviceManager(deviceManager);

    //The unregisterDeviceManager operation shall, upon successful unregistration, send an event to
    //the Outgoing Domain Management event channel with event data consisting of a
    //DomainManagementObjectRemovedEventType. The event data will be populated as follows:
    //      1. The producerId shall be the identifier attribute of the DomainManager.
    //      2. The sourceId shall be the identifier attribute of the unregistered DeviceManager.
    //      3. The sourceName shall be the label attribute of the unregistered DeviceManager.
    //      4. The sourceCategory shall be DEVICE_MANAGER.

    TRACE_EXIT(DomainManager_impl);
    return deviceManager;
}


void DomainManager_impl::removeDeviceManagerDevices (const std::string& deviceManagerId)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    // Unregister all devices for the DeviceManager
    for (DeviceList::iterator device = _registeredDevices.begin(); device != _registeredDevices.end(); ) {
        if ((*device)->devMgr.identifier == deviceManagerId) {
            LOG_TRACE(DomainManager_impl, "Unregistering device " << (*device)->label << " " << (*device)->identifier);
            device = _local_unregisterDevice(device);
        } else {
            ++device;
        }
    }

    TRACE_EXIT(DomainManager_impl)
}


void DomainManager_impl::removeDeviceManagerServices (const std::string& deviceManagerId)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    for (ServiceList::iterator service = _registeredServices.begin(); service != _registeredServices.end(); ) {
        if (service->deviceManagerId == deviceManagerId) {
            LOG_TRACE(DomainManager_impl, "Unregistering service " << service->name);
            service = _local_unregisterService(service);
        } else {
            ++service;
        }
    }

    TRACE_EXIT(DomainManager_impl)
}


ossie::DeviceManagerList::iterator DomainManager_impl::removeDeviceManager (ossie::DeviceManagerList::iterator deviceManager)
{
    // This function must work regardless of whether the DeviceManager object
    // is reachable or not. Therefore, no CORBA calls can be made.
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    deviceManager = _registeredDeviceManagers.erase(deviceManager);
    try {
        db.store("DEVICE_MANAGERS", _registeredDeviceManagers);
    } catch ( ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting change to device managers");
    } catch ( std::exception& ex ) {
        LOG_ERROR(DomainManager_impl, "The following standard exception occurred: "<<ex.what()<<" persisting the device managers")
    } catch ( const CORBA::Exception& ex ) {
        LOG_ERROR(DomainManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" persisting the device managers")
    }

    return deviceManager;
    TRACE_EXIT(DomainManager_impl)
}

void
DomainManager_impl::registerDevice (CF::Device_ptr registeringDevice,
                                    CF::DeviceManager_ptr registeredDeviceMgr)
throw (CORBA::SystemException, CF::InvalidObjectReference, CF::InvalidProfile,
       CF::DomainManager::DeviceManagerNotRegistered,
       CF::DomainManager::RegisterError)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    try {
        _local_registerDevice(registeringDevice, registeredDeviceMgr);
    } catch ( ... ) {
        throw;
    }

    CORBA::String_var identifier = registeringDevice->identifier();
    CORBA::String_var label = registeringDevice->label();
    ossie::sendObjectAddedEvent(DomainManager_impl::__logger, _configuration.getID(), identifier, label,
            registeringDevice, StandardEvent::DEVICE, proxy_consumer);
}

void
DomainManager_impl::_local_registerDevice (CF::Device_ptr registeringDevice,
                                    CF::DeviceManager_ptr registeredDeviceMgr)
throw (CORBA::SystemException, CF::InvalidObjectReference, CF::InvalidProfile,
       CF::DomainManager::DeviceManagerNotRegistered,
       CF::DomainManager::RegisterError)
{
    TRACE_ENTER(DomainManager_impl)
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
    LOG_TRACE(DomainManager_impl, "Registering Device " << devId);

    DeviceList::iterator deviceNode = findDeviceById(devId);
    if (deviceNode != _registeredDevices.end()) {
        LOG_TRACE(DomainManager_impl, "Device <" << devId << "> already registered; checking existence");
        if (!ossie::corba::objectExists((*deviceNode)->device)) {
            LOG_WARN(DomainManager_impl, "Cleaning up registration; device <" << devId << "> is registered and no longer exists");
            try {
                _local_unregisterDevice(deviceNode);
            } CATCH_LOG_WARN(DomainManager_impl, "_local_unregisterDevice failed");
        } else {
            ostringstream eout;
            eout << "Attempt re-register existing device : " << devId;
            LOG_ERROR(DomainManager_impl, eout.str());
            throw CF::DomainManager::RegisterError(CF::CF_NOTSET, eout.str().c_str());
        }
    }

//Add registeringDevice and its attributes to domain manager
    storeDeviceInDomainMgr (registeringDevice, registeredDeviceMgr);
        /*writeLogRecord(FAILURE_ALARM,the registeringDevice has an invalid profile.); */
        // TODO
//The registerDevice operation shall raise the CF InvalidProfile exception when:
//      1. The Device's SPD file and the SPD's referenced files do not exist or cannot be processed
//      due to the file not being compliant with XML syntax, or
//      2. The Device's SPD does not reference allocation properties.
//              throw( CF::InvalidProfile() );
//        throw CF::InvalidProfile ();
//    }


//Check the DCD for connections and establish them
    try {
        LOG_TRACE(DomainManager_impl, "Establishing Service Connections");
        _connectionManager.deviceRegistered(devId.c_str());
        try {
            db.store("CONNECTIONS", _connectionManager.getConnections());
        } catch (ossie::PersistenceException& ex) {
            LOG_ERROR(DomainManager_impl, "Error persisting change to device manager connections");
        } catch ( std::exception& ex ) {
            LOG_ERROR(DomainManager_impl, "The following standard exception occurred: "<<ex.what()<<" while persisting the connections")
        } catch ( const CORBA::Exception& ex ) {
            LOG_ERROR(DomainManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" while persisting the connections")
        }
    } catch ( ... ) {
        LOG_ERROR(DomainManager_impl, "Service connections could not be established")
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
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

//check if device is already registered
    if (deviceIsRegistered (registeringDevice)) {
        LOG_TRACE(DomainManager_impl, "Device already registered, refusing to store into domain manager")
        TRACE_EXIT(DomainManager_impl)
        return;
    }

    std::string devMgrId;
    try {
        devMgrId = ossie::corba::returnString(registeredDeviceMgr->identifier());
    } CATCH_LOG_ERROR(DomainManager_impl, "DeviceManager is unreachable during device registrations")
    if (devMgrId.empty()){
        return;
    }

    DeviceManagerList::iterator pDevMgr = findDeviceManagerById(devMgrId);
    if (pDevMgr ==  _registeredDeviceManagers.end()) {
        LOG_ERROR(DomainManager_impl, "Device Manager for Device is not registered")
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
    newDeviceNode->isLoadable = registeringDevice->_is_a(CF::LoadableDevice::_PD_repoId);
    newDeviceNode->isExecutable = registeringDevice->_is_a(CF::ExecutableDevice::_PD_repoId);

    parseDeviceProfile(*newDeviceNode);

    _registeredDevices.push_back (newDeviceNode);

    try {
        db.store("DEVICES", _registeredDevices);
    } catch ( ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting change to device managers");
    } catch ( std::exception& ex ) {
        LOG_ERROR(DomainManager_impl, "The following standard exception occurred: "<<ex.what()<<" while persisting the device managers")
    } catch ( const CORBA::Exception& ex ) {
        LOG_ERROR(DomainManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" while persisting the device managers")
    }

    TRACE_EXIT(DomainManager_impl)
}

//This function adds the registeringService and its name to the DomainMgr.
//if the service already exists it does nothing
void
DomainManager_impl::storeServiceInDomainMgr (CORBA::Object_ptr registeringService, CF::DeviceManager_ptr registeredDeviceMgr, const char* name, const char * serviceId)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    // If a service is already registered with that name, do nothing.
    if (serviceIsRegistered(name)) {
        LOG_INFO(DomainManager_impl, "Ignoring duplicate registration of service " << name);
        TRACE_EXIT(DomainManager_impl)
        return;
    }

    // The service needs to be added to the list.
    std::string devMgrId;
    try {
        devMgrId = ossie::corba::returnString(registeredDeviceMgr->identifier());
    } CATCH_LOG_ERROR(DomainManager_impl, "DeviceManager is unreachable during service registration")

    ServiceNode node;
    node.service = CORBA::Object::_duplicate(registeringService);
    node.deviceManagerId = devMgrId;
    node.name = name;
    node.serviceId = serviceId;

    // Add service to registered list, updating changes in the persistence store.
    _registeredServices.push_back(node);
    try {
        db.store("SERVICES", _registeredServices);
    } catch (ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting change to services");
    } catch ( std::exception& ex ) {
        LOG_ERROR(DomainManager_impl, "The following standard exception occurred: "<<ex.what()<<" while persisting the services")
    } catch ( const CORBA::Exception& ex ) {
        LOG_ERROR(DomainManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" while persisting the services")
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
    } CATCH_RETHROW_LOG_ERROR(DomainManager_impl, "Error unregistering a Device")  // rethrow for calling object's benefit
}

ossie::DeviceList::iterator DomainManager_impl::_local_unregisterDevice (ossie::DeviceList::iterator deviceNode)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    // Reset the last successful device pointer for deployments
    if ((*deviceNode)->identifier == _lastDeviceUsedForDeployment) {
        _lastDeviceUsedForDeployment.clear();
    }

    // Break any connections depending on the device.
    _connectionManager.deviceUnregistered((*deviceNode)->identifier);
    try {
        db.store("CONNECTIONS", _connectionManager.getConnections());
    } catch (ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting change to device manager connections");
    } catch ( std::exception& ex ) {
        LOG_ERROR(DomainManager_impl, "The following standard exception occurred: "<<ex.what()<<" while persisting the connections")
    } catch ( const CORBA::Exception& ex ) {
        LOG_ERROR(DomainManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" while persisting the connections")
    }

    try {

        // Release all applications that are using this device
        // THIS BEHAVIOR ISN'T SPECIFIED IN SCA, BUT IT MAKES GOOD SENSE
        bool done = false;
        unsigned int startCounter = 0;
        while ((!done) and (_applications.length() > 0)) {  // this additional loop is needed to make sure that the application sequence is correctly managed
            for (unsigned int i = startCounter; i < _applications.length(); i++) {
                CF::Application_ptr app = _applications[i];
                if (CORBA::is_nil(app)) {
                    if (i == (_applications.length()-1)) {
                        done = true;
                    }
                    startCounter++;
                    continue;
                }

                CF::DeviceAssignmentSequence_var compDevices = app->componentDevices();
                bool foundMatch = false;
                for  (unsigned int j = 0; j < compDevices->length(); j++) {
                    if (strcmp((*deviceNode)->identifier.c_str(), compDevices[j].assignedDeviceId) == 0) {
                        LOG_WARN(DomainManager_impl, "Releasing application that depends on registered device " << (*deviceNode)->identifier)
                        app->releaseObject();
                        foundMatch = true;
                        break;  // No need to call releaseObject twice
                    }
                }
                if (foundMatch) {
                    startCounter--;
                    if (startCounter > _applications.length())
                        startCounter = 0;
                    break;
                }
                if (i == (_applications.length()-1)) {
                    startCounter++;
                    done = true;
                }
            }
        }


    } CATCH_LOG_ERROR(DomainManager_impl, "Releasing stale applications from stale device failed");

    // Sent event here (as opposed to unregisterDevice), so we see the event on regular
    // unregisterDevice calls, and on cleanup (deviceManager shutdown, catastropic cleanup, etc.)
    ossie::sendObjectRemovedEvent(DomainManager_impl::__logger, _configuration.getID(), (*deviceNode)->identifier.c_str(), (*deviceNode)->label.c_str(),
                                  StandardEvent::DEVICE, proxy_consumer);

    // Remove the device from the internal list.
    deviceNode = _registeredDevices.erase(deviceNode);

    // Write the updated device list to the persistence store.
    try {
        db.store("DEVICES", _registeredDevices);
    } catch ( ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting change to devices");
    } catch ( std::exception& ex ) {
        LOG_ERROR(DomainManager_impl, "The following standard exception occurred: "<<ex.what()<<" while persisting the devices")
    } catch ( const CORBA::Exception& ex ) {
        LOG_ERROR(DomainManager_impl, "The following CORBA exception occurred: "<<ex._name()<<" while persisting the devices")
    }
    TRACE_EXIT(DomainManager_impl);
    return deviceNode;
}


//This function returns TRUE if the input registeredDevice is contained in the _registeredDevices
bool DomainManager_impl::deviceIsRegistered (CF::Device_ptr registeredDevice)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    DeviceList::iterator device = findDeviceByObject(registeredDevice);

    TRACE_EXIT(DomainManager_impl);
    return (device !=_registeredDevices.end());
}


bool DomainManager_impl::serviceIsRegistered (const char* serviceName)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    ossie::ServiceList::iterator service = findServiceByName(serviceName);

    TRACE_EXIT(DomainManager_impl);
    return (service != _registeredServices.end());
}


//This function returns TRUE if the input registeredDeviceMgr is contained in the _deviceManagers list attribute
bool DomainManager_impl::deviceMgrIsRegistered (CF::DeviceManager_ptr registeredDeviceMgr)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    DeviceManagerList::iterator node = findDeviceManagerByObject(registeredDeviceMgr);

    TRACE_EXIT(DomainManager_impl);
    return (node != _registeredDeviceManagers.end());;
}

//This function returns TRUE if the input registeredDomainMgr is contained in the _domainManagers list attribute
bool DomainManager_impl::domainMgrIsRegistered (CF::DomainManager_ptr registeredDomainMgr)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    DomainManagerList::iterator node = findDomainManagerByObject(registeredDomainMgr);

    TRACE_EXIT(DomainManager_impl);
    return (node != _registeredDomainManagers.end());;
}


//      METHOD:         installApplication
//      PURPOSE:        verify that all application file dependencies are available within
//                              the domain managers file manager
//      EXCEPTIONS:
//              --              InvalidProfile
//              --              InvalidFileName
//              --              ApplicationInstallationError

void
DomainManager_impl::installApplication (const char* profileFileName)
throw (CORBA::SystemException, CF::InvalidProfile, CF::InvalidFileName,
       CF::DomainManager::ApplicationInstallationError, CF::DomainManager::ApplicationAlreadyInstalled)
{
    boost::mutex::scoped_lock lock(interfaceAccess);
    try {
        _local_installApplication(profileFileName);
    } catch ( ... ) {
        throw;
    }

    for (unsigned int i=0; i<_applicationFactories.length(); i++) {
        CORBA::String_var softwareProfile = _applicationFactories[i]->softwareProfile();
        if (!strcmp(profileFileName, softwareProfile)) {
            CORBA::String_var identifier = _applicationFactories[i]->identifier();
            CORBA::String_var name =_applicationFactories[i]->name();
            ossie::sendObjectAddedEvent(DomainManager_impl::__logger, _configuration.getID(), identifier, name, _applicationFactories[i],
                                        StandardEvent::APPLICATION_FACTORY, proxy_consumer);
        }
    }
}

void
DomainManager_impl::_local_installApplication (const char* profileFileName)
throw (CORBA::SystemException, CF::InvalidProfile, CF::InvalidFileName,
       CF::DomainManager::ApplicationInstallationError, CF::DomainManager::ApplicationAlreadyInstalled)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

// NOTE: the <softwareassembly> name attribute is the name of the App Factory
//               that is currently installed because it is the installed factory that
//               provides the value of profileFileName

    SoftwareAssembly sadParser;

    try {
        // check the profile ends with .sad.xml, warn if it doesn't
        if ((strstr (profileFileName, ".sad.xml")) == NULL)
            { LOG_WARN(DomainManager_impl, "File " << profileFileName << " should end with .sad.xml."); }

        LOG_INFO(DomainManager_impl, "Installing application " << profileFileName);
        this->validate (profileFileName);

/// \todo verify that SAD conforms to DTD by allowing parser to throw an exception
//               then the DomainManager throws a CF::InvalidProfile if the SAD does not conform
        LOG_TRACE(DomainManager_impl, "Parsing SAD file");
        try {
            File_stream _sad(_fileMgr, profileFileName);
            sadParser.load( _sad );
            _sad.close();
        } catch (ossie::parser_error& e ) {
            LOG_ERROR(DomainManager_impl, "not installing application;  error parsing SAD file: " << profileFileName << "; " << e.what());
            throw(CF::InvalidProfile());
        } catch ( ... ) {
            LOG_ERROR(DomainManager_impl, "not installing application;  unexpected error parsing SAD file: " << profileFileName << "; ");
            throw(CF::DomainManager::ApplicationInstallationError());
        }

// check if application factory already exists for this profile
        try {
            LOG_TRACE(DomainManager_impl, "Installing application ID " << sadParser.getID());
            for (unsigned int i = 0; i < _installedApplications.size(); i++) {
                if (!strcmp(sadParser.getID (), _installedApplications[i].identifier.c_str())) {
                    LOG_INFO(DomainManager_impl, "Application "<<sadParser.getName()<<" with id " << sadParser.getID() << " already installed (Application Factory already exists)")
                    throw(CF::DomainManager::ApplicationAlreadyInstalled());
                }
            }
        } catch (CF::DomainManager::ApplicationAlreadyInstalled &) {
            throw;
        } catch ( ... ) {
            LOG_ERROR(DomainManager_impl, "[DomainManager::installApplication] Unable to determine if application is already installed");
            throw(CF::DomainManager::ApplicationInstallationError());
        }

        LOG_TRACE(DomainManager_impl, "Loading component placements");
// query the SAD for all ComponentPlacement's
        std::vector<ComponentPlacement> sadComponents = sadParser.getAllComponents();

// query each ComponentPlacement for its SPD file
        LOG_TRACE(DomainManager_impl, "installApplication: Validating " << sadComponents.size() << " SPDs");
        std::vector<ComponentPlacement>::const_iterator _iterator = sadComponents.begin ();
        while (_iterator != sadComponents.end ()) {
            this->validateSPD( sadParser.getSPDById(_iterator->getFileRefId()));
            ++_iterator;
        }
        LOG_TRACE(DomainManager_impl, "installApplication: Adding new AppFac");
        ApplicationFactory_impl* appFact = new ApplicationFactory_impl (profileFileName, &_applications, this->_domainName, this->_fullDomainManagerName, this);
        CORBA::String_var appFactId_obj = appFact->identifier();
        std::string appFactoryId = _domainName + "/" + static_cast<char*>(appFactId_obj);
        PortableServer::ObjectId_var oid = ossie::corba::activatePersistentObject(appFact_poa, appFact, appFactoryId);
        appFact->_remove_ref(); // The POA now holds our servant reference

        ApplicationFactoryNode appFactNode;
        appFactNode.servant_id = appFact_poa->servant_to_id(appFact);
        appFactNode.profile = profileFileName;
        appFactNode.identifier = appFactId_obj;
        _installedApplications.push_back(appFactNode);

        try {
            db.store("APP_FACTORIES", _installedApplications);
        } catch ( ossie::PersistenceException& ex) {
            LOG_ERROR(DomainManager_impl, "Error persisting change to device managers");
        }

        _applicationFactories.length(_applicationFactories.length() + 1);
        _applicationFactories[_applicationFactories.length() - 1] = appFact->_this();
    } catch (CF::FileException& ex) {
        LOG_ERROR(DomainManager_impl, "installApplication: While validating the SAD profile: " << ex.msg);
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
    } catch (CF::DomainManager::ApplicationInstallationError& e) {
        LOG_TRACE(DomainManager_impl, "rethrowing ApplicationInstallationError" << e.msg);
        throw;
    } catch (CF::DomainManager::ApplicationAlreadyInstalled &) {
        throw;
    } catch ( std::exception& ex ) {
        std::ostringstream eout;
        eout << "The following standard exception occurred: "<<ex.what()<<" while restoring the application factories";
        LOG_ERROR(DomainManager_impl, eout.str())
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_NOTSET, eout.str().c_str());
    } catch ( const CORBA::Exception& ex ) {
        std::ostringstream eout;
        eout << "The following CORBA exception occurred: "<<ex._name()<<" while restoring the application factories";
        LOG_ERROR(DomainManager_impl, eout.str())
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_NOTSET, eout.str().c_str());
    } catch (...) {
        LOG_ERROR(DomainManager_impl, "unexpected exception occurred while installing application");
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_NOTSET, "unknown exception");
    }

    TRACE_EXIT(DomainManager_impl)
}


void
DomainManager_impl::uninstallApplication (const char* applicationId)
throw (CORBA::SystemException, CF::DomainManager::InvalidIdentifier,
       CF::DomainManager::ApplicationUninstallationError)
{
    LOG_INFO(DomainManager_impl, "Uninstalling application " << applicationId);
    boost::mutex::scoped_lock lock(interfaceAccess);

    std::string appFactory_id;
    std::string appFactory_name;

    for (unsigned int i=0; i<_applicationFactories.length(); i++) {
        CORBA::String_var identifier = _applicationFactories[i]->identifier();
        if (!strcmp(applicationId, identifier)) {
            appFactory_id = ossie::corba::returnString(_applicationFactories[i]->identifier());
            appFactory_name = ossie::corba::returnString(_applicationFactories[i]->name());
            break;
        }
    }

    try {
        _local_uninstallApplication(applicationId);
    } catch ( ... ) {
        throw;
    }

        // if SUCCESS, write an ADMINISTRATIVE_EVENT to the DomainMgr's Log

        // send event to Outgoing Domain Management channel consisting of:
        // DomainManager identifier attribute
        // this->_configuration.getID()
        // uninstalled AppFactory identifier
        // sadParser.getId()
        // uninstalled AppFactory name
        // sadParser.getName()
        // uninstalled AppFactory IOR
        // ask the ORB
        // sourceCategory = APPLICATION_FACTORY
        // StandardEvent enumeration

        ossie::sendObjectRemovedEvent(DomainManager_impl::__logger, _configuration.getID(), appFactory_id.c_str(), appFactory_name.c_str(),
        StandardEvent::APPLICATION_FACTORY, proxy_consumer);
}

void
DomainManager_impl::_local_uninstallApplication (const char* applicationId)
throw (CORBA::SystemException, CF::DomainManager::InvalidIdentifier,
       CF::DomainManager::ApplicationUninstallationError)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

// NOTE: applicationId is the value of the <softwareassembly> name attribute
//               for the App Factory's SAD profile
    try {
        bool foundAppFactoryNode = false;
        for (std::vector<ApplicationFactoryNode>::iterator appFactNode = _installedApplications.begin();
         appFactNode != _installedApplications.end();
         ++appFactNode) {
            const char * appFactoryId = appFactNode->identifier.c_str();
            if (strcmp(applicationId, appFactoryId) == 0) {
                bool shift = false;
                for (size_t i = 0; i < _applicationFactories.length() - 1; i++) {
                    std::string appFactId = ossie::corba::returnString(_applicationFactories[i]->identifier());
                    if (appFactId ==  applicationId) {
                        shift = true;
                    }
                    if (shift) {
                        _applicationFactories[i] = _applicationFactories[i + 1];
                    }
                }
                foundAppFactoryNode = true;
                appFact_poa->deactivate_object(appFactNode->servant_id);
                _installedApplications.erase(appFactNode);

                // Eliminate the application factory from the applicationFactory sequence
                _applicationFactories[_applicationFactories.length() - 1] = CF::ApplicationFactory::_nil();
                _applicationFactories.length(_applicationFactories.length() - 1);
                break;
            }
        }

        if (not foundAppFactoryNode) {
            LOG_ERROR(DomainManager_impl, "ApplicationFactory not found")
            throw CF::DomainManager::InvalidIdentifier ();
        }


        try {
            db.store("APP_FACTORIES", _installedApplications);
        } catch ( ossie::PersistenceException& ex) {
            LOG_ERROR(DomainManager_impl, "Error persisting change to device managers");
        }

         TRACE_EXIT(DomainManager_impl)
    } catch (const CF::DomainManager::InvalidIdentifier& ex) {
         TRACE_EXIT(DomainManager_impl)
        throw;
    } catch (...) {
         TRACE_EXIT(DomainManager_impl)
        throw CF::DomainManager::ApplicationUninstallationError (CF::CF_NOTSET, "unknown exception");
    }
}

void DomainManager_impl::updateLocalAllocations(const ossie::AllocationTable& localAllocations)
{
    TRACE_ENTER(DomainManager_impl)
    try {
        db.store("LOCAL_ALLOCATIONS", localAllocations);
    } catch ( ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting local allocations");
    }
    TRACE_EXIT(DomainManager_impl)
}

void DomainManager_impl::updateRemoteAllocations(const ossie::RemoteAllocationTable& remoteAllocations)
{
    TRACE_ENTER(DomainManager_impl)
    try {
        db.store("REMOTE_ALLOCATIONS", remoteAllocations);
    } catch ( ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting remote allocation");
    }
    TRACE_EXIT(DomainManager_impl)
}

void
DomainManager_impl::addApplication(Application_impl* new_app)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(appAccess);

    LOG_TRACE(DomainManager_impl, "Attempting to add application to AppSeq with id: " << ossie::corba::returnString(new_app->identifier()));

    try {
        long old_length = _applications.length();
        _applications.length(old_length + 1);
        CF::Application_var appobj = new_app->_this();
        _applications[old_length] = CF::Application::_duplicate(appobj);

        ApplicationNode appNode;
        appNode.name = ossie::corba::returnString(new_app->name());
        appNode.identifier = ossie::corba::returnString(new_app->identifier());
        appNode.profile = ossie::corba::returnString(new_app->profile());
        appNode.contextName = new_app->_waveformContextName;
        appNode.contextIOR = ossie::corba::objectToString(new_app->_WaveformContext);
        appNode.componentDevices = new_app->_componentDevices;
        appNode.componentNamingContexts = new_app->appComponentNamingContexts;
        appNode.componentProcessIds = new_app->appComponentProcessIds;
        appNode.componentImplementations = new_app->appComponentImplementations;
        appNode.assemblyController = CF::Resource::_duplicate(new_app->assemblyController);
        appNode._startOrder = new_app->_appStartSeq;
        appNode.componentIORS.resize(0);
        for (unsigned int i = 0; i < new_app->_appStartSeq.size(); ++i) {
            appNode.componentIORS.push_back(ossie::corba::objectToString(new_app->_appStartSeq[i]));
        }
        appNode.fileTable = new_app->_fileTable;
        appNode.allocationIDs = new_app->_allocationIDs;
        //appNode.allocPropsTable = new_app->_allocPropsTable;
        appNode.connections = new_app->_connections;
        //appNode.usesDeviceCapacities = new_app->_usesDeviceCapacities;
        appNode._registeredComponents.length(new_app->_registeredComponents.length());
        for (unsigned int i=0; i<appNode._registeredComponents.length(); i++) {
            ComponentNode compNode;
            compNode.identifier = new_app->_registeredComponents[i].identifier;
            compNode.softwareProfile = new_app->_registeredComponents[i].softwareProfile;
            compNode.ior = ossie::corba::objectToString(new_app->_registeredComponents[i].componentObject);
            appNode.components.push_back(compNode);
            appNode._registeredComponents[i].identifier = CORBA::string_dup(new_app->_registeredComponents[i].identifier);
            appNode._registeredComponents[i].softwareProfile = CORBA::string_dup(new_app->_registeredComponents[i].softwareProfile);
            appNode._registeredComponents[i].type = new_app->_registeredComponents[i].type;
            appNode._registeredComponents[i].componentObject = CORBA::Object::_duplicate(new_app->_registeredComponents[i].componentObject);
        }
        appNode.ports = new_app->_ports;
        // Adds external properties
        for (std::map<std::string, std::pair<std::string, CF::Resource_ptr> >::const_iterator it = new_app->_properties.begin();
                it != new_app->_properties.end();
                ++it) {
            std::string extId = it->first;
            std::string propId = it->second.first;
            std::string compId = ossie::corba::returnString(it->second.second->identifier());
            appNode.properties[extId] = std::pair<std::string, std::string>(propId, compId);
        }

        _runningApplications.push_back(appNode);

        try {
            db.store("APPLICATIONS", _runningApplications);
        } catch ( ossie::PersistenceException& ex) {
            LOG_ERROR(DomainManager_impl, "Error persisting change to device managers");
        }
    } catch (...) {
        ostringstream eout;
        eout << "Could not add new application to AppSeq; ";
        eout << " application id: " << new_app->identifier() << "; ";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        LOG_ERROR(DomainManager_impl, eout.str());
        throw CF::DomainManager::ApplicationInstallationError(CF::CF_EFAULT, eout.str().c_str());
    }

    TRACE_EXIT(DomainManager_impl)
}

void
DomainManager_impl::removeApplication(std::string app_id)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(appAccess);

    LOG_TRACE(DomainManager_impl, "Attempting to remove application from AppSeq with id: " << app_id)

    unsigned long old_length = _applications.length();
    unsigned long index_found_app;
    bool found_index = false;

    try {
        // find app to remove
        for (unsigned long i=0; i < old_length; i++) {
            if (ossie::corba::returnString(_applications[i]->identifier()) == app_id) {
                index_found_app = i;
                found_index = true;
                break;
            }
        }
    } catch (...) {
        // don't need to output something here b/c
        // we do it below if found_index == false
    }

    // remove the application from the sequence
    if (found_index) {
        try {
            for (unsigned long i = index_found_app; i < old_length - 1; i++) {
                _applications[i] = _applications[i+1];
            }
            // now resize sequence
            _applications.length(old_length - 1);

            // Remove the application node as well, then reserialize.
            for (std::vector<ApplicationNode>::iterator ii = _runningApplications.begin(); ii != _runningApplications.end(); ++ii) {
                if (ii->identifier == app_id) {
                    _runningApplications.erase(ii);
                    break;
                }
            }

            try {
                db.store("APPLICATIONS", _runningApplications);
            } catch ( ossie::PersistenceException& ex) {
                LOG_ERROR(DomainManager_impl, "Error persisting change to device managers");
            }
        }
        catch (...){
            ostringstream eout;
            eout << "Failed to remove application from AppSeq; ";
            eout << " with application id: " << app_id << "; ";
            eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
            throw CF::DomainManager::ApplicationUninstallationError(CF::CF_EFAULT, eout.str().c_str());
        }

    } else {
        ostringstream eout;
        eout << "Could find application in AppSeq; ";
        eout << " with application id: " << app_id << "; ";
        eout << " error occurred near line:" <<__LINE__ << " in file:" <<  __FILE__ << ";";
        throw CF::DomainManager::ApplicationUninstallationError(CF::CF_EFAULT, eout.str().c_str());
    }

    TRACE_EXIT(DomainManager_impl)
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

void
DomainManager_impl::_local_registerWithEventChannel (CORBA::
                                              Object_ptr registeringObject,
                                              const char* registeringId,
                                              const char* eventChannelName)
throw (CORBA::SystemException, CF::InvalidObjectReference,
       CF::DomainManager::InvalidEventChannelName,
       CF::DomainManager::AlreadyConnected)
{
    TRACE_ENTER(DomainManager_impl)
    
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

void
DomainManager_impl::_local_unregisterFromEventChannel (const char* unregisteringId,
                                                const char* eventChannelName)
throw (CORBA::SystemException, CF::DomainManager::InvalidEventChannelName,
       CF::DomainManager::NotConnected)
{
    TRACE_ENTER(DomainManager_impl)
    
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
    _local_registerService(registeringService, registeredDeviceMgr, name);
}

void DomainManager_impl::_local_registerService (CORBA::Object_ptr registeringService, CF::DeviceManager_ptr registeredDeviceMgr, const char* name)
    throw (CF::DomainManager::RegisterError, CF::DomainManager::DeviceManagerNotRegistered, CF::InvalidObjectReference, CORBA::SystemException)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);


    // Verify that the service and DeviceManager are not nil references
    if (CORBA::is_nil(registeringService)) {
        LOG_ERROR(DomainManager_impl, "Ignoring registration of nil Service");
        throw CF::InvalidObjectReference("Registering Service is nil");
    } else if (CORBA::is_nil(registeredDeviceMgr)) {
        LOG_ERROR(DomainManager_impl, "Ignoring registration of Service with nil DeviceManager");
        throw CF::InvalidObjectReference("Registered DeviceManager is nil");
    }

    // Verify that DeviceManager is registered
    if (!deviceMgrIsRegistered(registeredDeviceMgr)) {
        LOG_WARN(DomainManager_impl, "Ignoring attempt to register a Service from an unregistered DeviceManager");
        throw CF::DomainManager::DeviceManagerNotRegistered();
    }
    
    DeviceManagerConfiguration _DCDParser;
    bool readDCD = true;
    std::string serviceId("");
    std::string inputUsageName(name);
    try {
        CF::FileSystem_var devMgrFileSys = registeredDeviceMgr->fileSys();
        CORBA::String_var profile = registeredDeviceMgr->deviceConfigurationProfile();
        File_stream _dcd(devMgrFileSys, profile);
        _DCDParser.load(_dcd);
        _dcd.close();
    } catch ( ... ) {
        readDCD = false;
    }
    if (readDCD) {
        const std::vector<ossie::ComponentPlacement>& componentPlacements = _DCDParser.getComponentPlacements();
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
        storeServiceInDomainMgr(registeringService, registeredDeviceMgr, name, serviceId.c_str());
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
    } catch (ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting change to device manager connections");
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
    ossie::sendObjectAddedEvent(DomainManager_impl::__logger, _configuration.getID(), serviceId.c_str(), name,
            registeringService, StandardEvent::SERVICE, proxy_consumer);

//The registerService operation shall raise the RegisterError exception when an internal error
//exists which causes an unsuccessful registration.
}


void DomainManager_impl::unregisterService(CORBA::Object_ptr unregisteringService, const char* name)
    throw (CF::DomainManager::UnregisterError, CF::InvalidObjectReference, CORBA::SystemException)
{
    boost::mutex::scoped_lock lock(interfaceAccess);

    // Try to find a service registered with the given name.
    ossie::ServiceList::iterator service = findServiceByName(name);
    if (service == _registeredServices.end()) {
        LOG_ERROR(DomainManager_impl, "Cannot unregister service '" << name << "' not registered with domain");
        throw CF::InvalidObjectReference("Service is not registered with domain");
    }

    // Refuse to unregister if the unregistering object is not the same as the
    // registered one.
    if (!service->service->_is_equivalent(unregisteringService)) {
        LOG_ERROR(DomainManager_impl, "Not unregistering service '" << name << "' because object does not match prior registration");
        throw CF::InvalidObjectReference("Unregistering service is not the same as registered object");
    }
    
    _local_unregisterService(service);
}


ossie::ServiceList::iterator DomainManager_impl::_local_unregisterService(ossie::ServiceList::iterator service)
{
    TRACE_ENTER(DomainManager_impl)
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    // Disconnect any connections involving this service.
    _connectionManager.serviceUnregistered(service->name);
    try {
        db.store("CONNECTIONS", _connectionManager.getConnections());
    } catch (ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting change to device manager connections");
    }
    
    std::string serviceName(service->name);
    std::string serviceId(service->serviceId);

    // Remove the service from the internal list.
    service = _registeredServices.erase(service);

    if (_applications.length() > 0) {
        std::vector<Application_impl*> appsToRelease;

        PortableServer::POA_var dm_poa = ossie::corba::RootPOA()->find_POA("DomainManager", 0);
        PortableServer::POA_var poa = dm_poa->find_POA("Applications", 1);

        for (CORBA::ULong ii = 0; ii < _applications.length(); ++ii) {
            PortableServer::ServantBase* servant = poa->reference_to_servant(_applications[ii]);
            if (!servant) {
                LOG_DEBUG(DomainManager_impl, "No servant for application");
                continue;
            }
            Application_impl* app = dynamic_cast<Application_impl*>(servant);
            if (!app) {
                LOG_DEBUG(DomainManager_impl, "Application servant has wrong class type");
                continue;
            }

            if (app->checkConnectionDependency(ossie::Endpoint::SERVICENAME, serviceName)) {
                appsToRelease.push_back(app);
            }
        }

        LOG_DEBUG(DomainManager_impl, "Releasing " << appsToRelease.size() << " applications");
        for (std::vector<Application_impl*>::iterator app = appsToRelease.begin(); app != appsToRelease.end(); ++app) {
            LOG_DEBUG(DomainManager_impl, "Releasing " << ossie::corba::returnString((*app)->identifier()));
            (*app)->releaseObject();
        }
    }

    // Write the updated service list to the persistence store.
    try {
        db.store("SERVICES", _registeredServices);
    } catch (ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting change to services");
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
    ossie::sendObjectRemovedEvent(DomainManager_impl::__logger, _configuration.getID(), serviceId.c_str(), serviceName.c_str(),
                                  StandardEvent::SERVICE, proxy_consumer);

    return service;
}


void
DomainManager_impl::validate (const char* _profile)
{
    TRACE_ENTER(DomainManager_impl)

    if (_profile == NULL) {
        TRACE_EXIT(DomainManager_impl)
        return;
    }

// verify the application's SAD exists in DomainManager FileSystem
    LOG_TRACE(DomainManager_impl, "Validating that profile " << _profile << " exists");
    if (!_fileMgr->exists (_profile)) {
        string msg = "File ";
        msg += _profile;
        msg += " does not exist.";

        throw CF::FileException (CF::CF_ENOENT, msg.c_str());
    }
}


void
DomainManager_impl::validateSPD (const char* _spdProfile, int _cnt) throw (CF::DomainManager::ApplicationInstallationError)
{
    TRACE_ENTER(DomainManager_impl)

    if (_spdProfile == NULL) {
        TRACE_EXIT(DomainManager_impl)
        return;
    }

    /// \todo Figure out checks for xml conforming to dtd, through suitable exception if it doesn't. Possibly CF::InvalidProfile

    try {
        LOG_TRACE(DomainManager_impl, "Validating SPD " << _spdProfile);
        this->validate (_spdProfile);

        // check the filename ends with the extension given in the spec
        if ((strstr (_spdProfile, ".spd.xml")) == NULL)
            { LOG_ERROR(DomainManager_impl, "File " << _spdProfile << " should end with .spd.xml"); }
        LOG_TRACE(DomainManager_impl, "validating " << _spdProfile);

        SoftPkg spdParser;
        try {
            File_stream _spd(_fileMgr, _spdProfile);
            spdParser.load( _spd, _spdProfile );
            _spd.close();
        } catch (ossie::parser_error& ex) {
            LOG_ERROR(DomainManager_impl, "SPD file failed validation; parser error on file " << _spdProfile << "; " << ex.what());
            throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.what());
        } catch (CF::InvalidFileName ex) {
            LOG_ERROR(DomainManager_impl, "Failed to validate SPD due to invalid file name " << ex.msg);
            throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
        } catch (CF::FileException ex) {
            LOG_ERROR(DomainManager_impl, "Failed to validate SPD due to file exception" << ex.msg);
            throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
        } catch ( ... ) {
            LOG_ERROR(DomainManager_impl, "Unexpected error validating PRF " << _spdProfile);
            throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, "");
        }

        // query SPD for PRF
        if (spdParser.getPRFFile() != 0) {
            LOG_TRACE(DomainManager_impl, "validating " << spdParser.getPRFFile());
            try {
                this->validate (spdParser.getPRFFile ());

                // check the file name ends with the extension given in the spec
                if (spdParser.getPRFFile() && (strstr (spdParser.getPRFFile (), ".prf.xml")) == NULL) {
                    LOG_ERROR(DomainManager_impl, "File " << spdParser.getPRFFile() << " should end in .prf.xml.");
                }

                LOG_TRACE(DomainManager_impl, "Creating file stream")
                File_stream prfStream(_fileMgr, spdParser.getPRFFile());
                LOG_TRACE(DomainManager_impl, "Loading parser")
                Properties prfParser(prfStream);
                LOG_TRACE(DomainManager_impl, "Closing stream")
                prfStream.close();
            } catch (ossie::parser_error& ex ) {
                LOG_ERROR(DomainManager_impl, "Error validating PRF " << spdParser.getPRFFile() << ex.what());
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.what());
            } catch (CF::InvalidFileName ex) {
                LOG_ERROR(DomainManager_impl, "Failed to validate PRF due to invalid file name " << ex.msg);
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
            } catch (CF::FileException ex) {
                LOG_ERROR(DomainManager_impl, "Failed to validate PRF due to file exception" << ex.msg);
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
            } catch ( ... ) {
                LOG_ERROR(DomainManager_impl, "Unexpected error validating PRF " << spdParser.getPRFFile());
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, "");
            }
        } else {
            LOG_TRACE(DomainManager_impl, "No PRF file to validate")
        }

        if (spdParser.getSCDFile() != 0) {
            // query SPD for SCD
            LOG_TRACE(DomainManager_impl, "validating " << spdParser.getSCDFile());
            this->validate (spdParser.getSCDFile ());

            // Check the filename ends with  the extension given in the spec
            if ((strstr (spdParser.getSCDFile (), ".scd.xml")) == NULL)
                { LOG_ERROR(DomainManager_impl, "File " << spdParser.getSCDFile() << " should end with .scd.xml."); }

            try {
                File_stream _scd(_fileMgr, spdParser.getSCDFile());
                ComponentDescriptor scdParser (_scd);
                _scd.close();
            } catch (ossie::parser_error& ex) {
                LOG_ERROR(DomainManager_impl, "SCD file failed validation; parser error on file " << spdParser.getSCDFile() << "; " << ex.what());
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.what());
            } catch (CF::InvalidFileName ex) {
                LOG_ERROR(DomainManager_impl, "Failed to validate SCD due to invalid file name " << ex.msg);
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
            } catch (CF::FileException ex) {
                LOG_ERROR(DomainManager_impl, "Failed to validate SCD due to file exception" << ex.msg);
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
            } catch ( ... ) {
                LOG_ERROR(DomainManager_impl, "Unexpected error validating PRF " << spdParser.getSCDFile());
                throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, "");
            }
        } else if (spdParser.isScaCompliant()) {
            LOG_ERROR(DomainManager_impl, "SCA compliant component is missing SCD file reference");
            throw CF::DomainManager::ApplicationInstallationError(CF::CF_EBADF, "SCA compliant components require SCD file");
        } else {
            LOG_TRACE(DomainManager_impl, "No SCD file to validate")
        }

        /// \todo Figure out if this should go: this->validateSPD( spdParser.getSPDFile(), ++_cnt );

    } catch (CF::InvalidFileName& ex) {
        LOG_ERROR(DomainManager_impl, "Failed to validate SPD due to " << ex.msg);
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
    } catch (CF::FileException& ex) {
        LOG_ERROR(DomainManager_impl, "Failed to validate SPD due to " << ex.msg);
        throw CF::DomainManager::ApplicationInstallationError (CF::CF_EBADF, ex.msg);
    } catch (CF::DomainManager::ApplicationInstallationError& ex) {
        throw;
    } catch ( ... ) {
        LOG_ERROR(DomainManager_impl, "Unexpected error validating SPD " << _spdProfile);
        throw CF::DomainManager::ApplicationInstallationError ();
    }
}

CosEventChannelAdmin::EventChannel_ptr DomainManager_impl::getEventChannel(const std::string &name) {

    vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();

    while (_iter != _eventChannels.end()) {
        if ((*_iter).name == name) {
            return CosEventChannelAdmin::EventChannel::_duplicate((*_iter).channel);
        }
        _iter++;
    }

    _iter = _eventChannels.begin();
    while (_iter != _eventChannels.end()) {
        std::string::size_type pos = (*_iter).name.find(".",0);
        if (pos == std::string::npos) {
            _iter++;
            continue;
        }
        pos++;
        if (!(*_iter).name.compare(pos,(*_iter).name.size()-pos,name)) {
            return CosEventChannelAdmin::EventChannel::_duplicate((*_iter).channel);
        }
        _iter++;
    }

    return CosEventChannelAdmin::EventChannel::_nil();

}

bool DomainManager_impl::eventChannelExists(const std::string &name) {

    vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();

    while (_iter != _eventChannels.end()) {
        if ((*_iter).name == name) {
            return true;
        }
        _iter++;
    }
    _iter = _eventChannels.begin();
    while (_iter != _eventChannels.end()) {
        std::string::size_type pos = (*_iter).name.find(".",0);
        if (pos == std::string::npos) {
            _iter++;
            continue;
        }
        pos++;
        if (!(*_iter).name.compare(pos,(*_iter).name.size()-pos,name)) {
            return true;
        }
        _iter++;
    }

    return false;

}

unsigned int DomainManager_impl::incrementEventChannelConnections(const std::string &EventChannelName) {
    LOG_TRACE(DomainManager_impl, "Incrementing Event Channel " << EventChannelName);
    vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();

    while (_iter != _eventChannels.end()) {
        if ((*_iter).name == EventChannelName) {
            (*_iter).connectionCount++;
            LOG_TRACE(DomainManager_impl, "Event Channel " << EventChannelName<<" count: "<<(*_iter).connectionCount);
            return (*_iter).connectionCount;
        }
        _iter++;
    }

    LOG_TRACE(DomainManager_impl, "Event Channel " << EventChannelName<<" does not exist");
    return 0;
}

unsigned int DomainManager_impl::decrementEventChannelConnections(const std::string &EventChannelName) {
    LOG_TRACE(DomainManager_impl, "Decrementing Event Channel " << EventChannelName);
    vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();

    while (_iter != _eventChannels.end()) {
        if ((*_iter).name == EventChannelName) {
            (*_iter).connectionCount--;
            LOG_TRACE(DomainManager_impl, "Event Channel " << EventChannelName<<" count: "<<(*_iter).connectionCount);
            return (*_iter).connectionCount;
        }
        _iter++;
    }
    LOG_TRACE(DomainManager_impl, "Event Channel " << EventChannelName<<" does not exist");
    return 0;

}

CF::Resource_ptr DomainManager_impl::lookupComponentByInstantiationId(const std::string& identifier)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);
    DeviceList::iterator deviceNode = findDeviceById(identifier);
    if (deviceNode != _registeredDevices.end()) {
        return CF::Resource::_duplicate((*deviceNode)->device);
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
    LOG_TRACE(DomainManager_impl, "Resolving domainfinder type='" << type << "' name='" << name << "'");
    if (type == "filemanager") {
        return CF::FileManager::_duplicate(_fileMgr);
    } else if (type == "log") {
        LOG_WARN(DomainManager_impl, "No support for log in domainfinder element");
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
        LOG_WARN(DomainManager_impl, "No support for namingservice in domainfinder element");
    } else if (type == "servicename") {
        ServiceList::iterator serviceNode = findServiceByName(name);
        if (serviceNode != _registeredServices.end()) {
            LOG_TRACE(DomainManager_impl, "Found service " << name);
            return CORBA::Object::_duplicate(serviceNode->service);
        }
        LOG_WARN(DomainManager_impl, "No service found for servicename '" << name << "'");
    } else if (type == "servicetype") {
        ServiceList::iterator serviceNode = findServiceByType(name);
        if (serviceNode != _registeredServices.end()) {
            LOG_TRACE(DomainManager_impl, "Found service " << serviceNode->name << " supporting '" << name << "'");
            return CORBA::Object::_duplicate(serviceNode->service);
        }
        LOG_WARN(DomainManager_impl, "No service found for servicetype '" << name << "'");
    } else if (type == "domainmanager") {
        return _this();
    }
    return CORBA::Object::_nil();
}


void DomainManager_impl::catastrophicUnregisterDeviceManager (ossie::DeviceManagerList::iterator deviceManager)
{
    TRACE_ENTER(DomainManager_impl);
    boost::recursive_mutex::scoped_lock lock(stateAccess);

    // NOTE: Assume that the DeviceManager doesn't exist, so make no CORBA calls to it.

    // Release all devices associated with the DeviceManager that is being unregistered.
    LOG_TRACE(DomainManager_impl, "Finding devices for device manager " << deviceManager->identifier);
    for (DeviceList::iterator device = _registeredDevices.begin(); device != _registeredDevices.end(); ++device) {
        if ((*device)->devMgr.identifier == deviceManager->identifier) {
            LOG_TRACE(DomainManager_impl, "Releasing registered device " << (*device)->label);
            try {
                (*device)->device->releaseObject();
            } CATCH_LOG_WARN(DomainManager_impl, "Failed to release device " << (*device)->label);
        }
    }

    // Continue with the normal unregistration code path.
    _local_unregisterDeviceManager(deviceManager);

    TRACE_EXIT(DomainManager_impl);
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


void DomainManager_impl::parseDeviceProfile (ossie::DeviceNode& node)
{
    // TODO: Exception handling
    CF::FileSystem_var devMgrFS = node.devMgr.deviceManager->fileSys();

    // Parse and cache the device's SPD
    LOG_TRACE(DomainManager_impl, "Parsing SPD for device " << node.identifier);
    try {
        File_stream spd(devMgrFS, node.softwareProfile.c_str());
        node.spd.load(spd, node.softwareProfile);
    } catch (const ossie::parser_error& error) {
        LOG_WARN(DomainManager_impl, "Error parsing SPD for device " << node.identifier << ": " << error.what());
    } catch (...) {
        LOG_WARN(DomainManager_impl, "Unable to cache SPD for device " << node.identifier);
    }

    // Parse and cache the device's PRF, if it has one
    if (node.spd.getPRFFile()) {
        LOG_TRACE(DomainManager_impl, "Parsing PRF for device " << node.identifier);
        try {
            File_stream prf(devMgrFS, node.spd.getPRFFile());
            node.prf.load(prf);
        } catch (const ossie::parser_error& error) {
            LOG_WARN(DomainManager_impl, "Error parsing PRF for device " << node.identifier << ": " << error.what());
        } catch (...) {
            LOG_WARN(DomainManager_impl, "Unable to cache PRF for device " << node.identifier);
        }
    }

    // Check for an implementation-specific PRF
    for (std::vector<ossie::SPD::Implementation>::const_iterator impl = node.spd.getImplementations().begin(); impl != node.spd.getImplementations().end(); ++impl) {
        if (impl->getID() != node.implementationId) {
            continue;
        }
        if (impl->getPRFFile()) {
            LOG_TRACE(DomainManager_impl, "Parsing implementation-specific PRF for device " << node.identifier);
            try {
                File_stream prf_file(devMgrFS, impl->getPRFFile());
                node.prf.join(prf_file);
            } catch (const ossie::parser_error& error) {
                LOG_WARN(DomainManager_impl, "Error parsing implementation-specific PRF for device " << node.identifier << ": " << error.what());
            } catch (...) {
                LOG_WARN(DomainManager_impl, "Unable to cache implementation-specific PRF for device " << node.identifier);
            }
        }
    }

    // Override with values from the DCD
    LOG_TRACE(DomainManager_impl, "Parsing DCD overrides for device " << node.identifier);
    ossie::DeviceManagerConfiguration dcd;
    const std::string deviceManagerProfile = ossie::corba::returnString(node.devMgr.deviceManager->deviceConfigurationProfile());
    try {
        File_stream dcd_file(devMgrFS, deviceManagerProfile.c_str());
        dcd.load(dcd_file);
    } catch (const ossie::parser_error& error) {
        LOG_WARN(DomainManager_impl, "Error parsing DCD overrides for device " << node.identifier << ": " << error.what());
    } catch (...) {
        LOG_WARN(DomainManager_impl, "Unable to cache DCD overrides for device " << node.identifier);
    }


    const ComponentInstantiation* instantiation = findComponentInstantiation(dcd.getComponentPlacements(), node.identifier);
    if (instantiation) {
        node.prf.override(instantiation->properties);
    } else {
        LOG_WARN(DomainManager_impl, "Unable to find device " << node.identifier << " in DCD");
    }
}
