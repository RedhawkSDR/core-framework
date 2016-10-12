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


#ifndef APPLICATION_H
#define APPLICATION_H

#include <vector>
#include <string>
#include <map>

#include <ossie/CF/cf.h>
#include <ossie/debug.h>

#include "applicationSupport.h"
#include "connectionSupport.h"

class DomainManager_impl;

class Application_impl : public virtual POA_CF::Application
{
    ENABLE_LOGGING
    friend class DomainManager_impl;

protected:
    CF::DeviceAssignmentSequence appComponentDevices;
    CF::Application::ComponentElementSequence appComponentImplementations;
    CF::Application::ComponentElementSequence appComponentNamingContexts;
    CF::Application::ComponentProcessIdSequence appComponentProcessIds;
    CF::Resource_var assemblyController;

public:

    Application_impl (const char* _id, const char* _name, const char* _profile, DomainManager_impl* domainManager, const std::string& waveformContextName,
                      CosNaming::NamingContext_ptr WaveformContext);
    
    void populateApplication (CF::Resource_ptr _assemblyController,
                              std::vector<ossie::DeviceAssignmentInfo>& _devSequence,
                              CF::Application::ComponentElementSequence* _implSequence,
                              std::vector<CF::Resource_ptr> _startSeq,
                              CF::Application::ComponentElementSequence* _namingCtxSequence,
                              CF::Application::ComponentProcessIdSequence* _procIdSequence,
                              std::vector<ossie::ConnectionNode>& connections,
                              std::map<std::string, std::string>& fileTable,
                              ossie::SoftPkgList& softpkgList,
                              std::map<std::string, std::vector< ossie::AllocPropsInfo > >& allocPropsTable);

    ~Application_impl ();

    char* identifier () throw (CORBA::SystemException);
    CORBA::Boolean started ()
        throw (CORBA::SystemException);
    void start ()
        throw (CF::Resource::StartError, CORBA::SystemException);
    void stop ()
        throw (CF::Resource::StopError, CORBA::SystemException);

    /// The core framework provides an implementation for this method.
    void configure (const CF::Properties& configProperties)
        throw (CF::PropertySet::PartialConfiguration,
           CF::PropertySet::InvalidConfiguration, CORBA::SystemException);

    /// The core framework provides an implementation for this method.
    void query (CF::Properties& configProperties)
        throw (CF::UnknownProperties, CORBA::SystemException);

    void initialize ()
        throw (CF::LifeCycle::InitializeError, CORBA::SystemException);
        
    void releaseObject ()
        throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);
        
    CORBA::Object_ptr getPort (const char*)
        throw (CORBA::SystemException, CF::PortSupplier::UnknownPort);
        
    void runTest (CORBA::ULong, CF::Properties&)
        throw (CORBA::SystemException, CF::UnknownProperties, CF::TestableObject::UnknownTest);
    
    char* profile () throw (CORBA::SystemException);
    
    char* name () throw (CORBA::SystemException);
    
    CF::DeviceAssignmentSequence * componentDevices ()
        throw (CORBA::SystemException);
        
    CF::Application::ComponentElementSequence * componentImplementations ()
        throw (CORBA::SystemException);
        
    CF::Application::ComponentElementSequence * componentNamingContexts ()
        throw (CORBA::SystemException);
        
    CF::Application::ComponentProcessIdSequence * componentProcessIds ()
        throw (CORBA::SystemException);
    
    CF::Components * registeredComponents ();
    
    void registerComponent (CF::ComponentType &component);
    
    const ossie::AppConnectionManager& getConnectionManager();

    void addExternalPort (const std::string&, CORBA::Object_ptr);

    /** Implements the ConnectionManager functions
     *  - Makes this class compatible with the ConnectionManager
     */
    // ComponentLookup interface
    CF::Resource_ptr lookupComponentByInstantiationId(const std::string& identifier);

    // DeviceLookup interface
    CF::Device_ptr lookupDeviceThatLoadedComponentInstantiationId(const std::string& componentId);
    CF::Device_ptr lookupDeviceUsedByComponentInstantiationId(const std::string& componentId, const std::string& usesId);

#if ENABLE_EVENTS
    CosEventChannelAdmin::ProxyPushConsumer_var proxy_consumer;
#endif

    // Event Channel objects
    void createEventChannels (void);
    void connectToOutgoingEventChannel (void);
    std::string getEventChannelIOR();

    // Returns true if any connections in this application depend on the given object, false otherwise
    bool checkConnectionDependency (ossie::Endpoint::DependencyType type, const std::string& identifier) const;

private:
    Application_impl (); // No default constructor
    Application_impl(Application_impl&);  // No copying

    const std::string _identifier;

    std::string sadProfile;
    std::string _domainName;
    std::string appName;
    std::vector<ossie::DeviceAssignmentInfo> _componentDevices;
    std::map<std::string, std::string> _componentNames;
    std::vector<ossie::ConnectionNode> _connections;
    std::vector<CF::Resource_ptr> _appStartSeq;
    std::map<std::string, std::string> _fileTable;
    ossie::SoftPkgList                  _softpkgList;
    std::map<std::string, unsigned long> _pidTable;
    std::map<std::string, std::vector<ossie::AllocPropsInfo> >_allocPropsTable;
    std::auto_ptr<ossie::AppConnectionManager> connectionManager;
    DomainManager_impl* _domainManager;
    std::string _waveformContextName;
    CosNaming::NamingContext_var _WaveformContext;

    CF::Components _registeredComponents;

    std::map<std::string, CORBA::Object_var> _ports;

    bool release_already_called;
    boost::mutex releaseObjectLock;

    PortableServer::POA_var app_poa;

};
#endif
