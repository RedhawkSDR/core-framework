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
#include <set>

#include <boost/thread/condition_variable.hpp>

#include <ossie/CF/cf.h>
#include <ossie/debug.h>
#include <ossie/Logging_impl.h>

#include "Deployment.h"
#include "ApplicationDeployment.h"
#include "PersistenceStore.h"
#include "connectionSupport.h"
#include "ApplicationComponent.h"

class DomainManager_impl;
class ApplicationRegistrar_impl;
class FakeApplication;

class Application_impl : public virtual POA_CF::Application, public Logging_impl
{
    ENABLE_LOGGING
    friend class DomainManager_impl;

public:

    Application_impl (const std::string& id, const std::string& name, const std::string& profile,
                      DomainManager_impl* domainManager, const std::string& waveformContextName,
                      CosNaming::NamingContext_ptr waveformContext, bool aware,
                      float stopTimeout, CosNaming::NamingContext_ptr DomainContext);
    
    void populateApplication (const CF::DeviceAssignmentSequence& deviceAssignments,
                              std::vector<ossie::ConnectionNode>& connections,
                              std::vector<std::string> allocationIDs);

    void setStartOrder(const std::vector<std::string>& startOrder);

    ~Application_impl ();

    static PortableServer::ObjectId* Activate(Application_impl* application);

    /**
     * @throw CORBA::SystemException
     */
    char* identifier ();
    CORBA::Boolean started ();
    /**
     * @throw CF::Resource::StartError
     * @throw CORBA::SystemException
     */
    void start ();
    /**
     * @throw CF::Resource::StartError
     * @throw CORBA::SystemException
     */
    void stop ();

    /**
     * @throw CF::Resource::StopError
     * @throw CORBA::SystemException
     */
    void local_stop (float timeout);

    /**
     * The core framework provides an implementation for this method.
     * 
     * @throw CF::PropertySet::PartialConfiguration
     * @throw CF::PropertySet::InvalidConfiguration
     * @throw CORBA::SystemException
     */
    void initializeProperties (const CF::Properties& configProperties);

    /**
     * The core framework provides an implementation for this method.
     * 
     * @throw CF::PropertySet::PartialConfiguration
     * @throw CF::PropertySet::InvalidConfiguration
     * @throw CORBA::SystemException
     */
    void configure (const CF::Properties& configProperties);

    /**
     * The core framework provides an implementation for this method.
     * 
     * @throw CF::UnknownProperties
     * @throw CORBA::SystemException
     */
    void query (CF::Properties& configProperties);

    /**
     * The core framework provides an implementation for this method.
     * 
     * @throw CF::Application::InvalidMetric
     * @throw CORBA::SystemException
     */
    CF::Properties* metrics (const CF::StringSequence& components, const CF::StringSequence& attributes);

    /**
     * @throw CF::UnknownProperties
     * @throw CF::InvalidObjectReference
     */
    char *registerPropertyListener( CORBA::Object_ptr listener, const CF::StringSequence &prop_ids, const CORBA::Float interval);

    /**
     * @throw CF::InvalidIdentifier
     */
    void unregisterPropertyListener( const char *reg_id );

    /**
     * @throw CF::LifeCycle::InitializeError
     * @throw CORBA::SystemException
     */
    void initialize ();
        
    /**
     * @throw CF::LifeCycle::ReleaseError
     * @throw CORBA::SystemException
     */
    void releaseObject ();
        
    /**
     * @throw CF::PortSupplier::UnknownPort
     * @throw CORBA::SystemException
     */
    CORBA::Object_ptr getPort (const char*);

    CF::PortSet::PortInfoSequence* getPortSet ();
        
    /**
     * @throw CF::UnknownProperties
     * @throw CF::TestableObject::UnknownTest
     * @throw CORBA::SystemException
     */
    void runTest (CORBA::ULong, CF::Properties&);
    
    /**
     * @throw CORBA::SystemException
     */
    char* profile ();
    
    /**
     * @throw CORBA::SystemException
     */
    char* softwareProfile ();
    
    /**
     * @throw CORBA::SystemException
     */
    char* name ();
    
    /**
     * @throw CORBA::SystemException
     */
    bool aware ();
    
    /**
     * @throw CORBA::SystemException
     */
    CORBA::Float stopTimeout ();

    /**
     * @throw CORBA::SystemException
     */
    void stopTimeout (CORBA::Float timeout);
    
    /**
     * @throw CORBA::SystemException
     */
    CF::DeviceAssignmentSequence * componentDevices ();
        
    /**
     * @throw CORBA::SystemException
     */
    CF::Application::ComponentElementSequence * componentImplementations ();
        
    /**
     * @throw CORBA::SystemException
     */
    CF::Application::ComponentElementSequence * componentNamingContexts ();
        
    /**
     * @throw CORBA::SystemException
     */
    CF::Application::ComponentProcessIdSequence * componentProcessIds ();
    
    CF::Components * registeredComponents ();
    
    CF::ApplicationRegistrar_ptr appReg (void);

    void setAssemblyController (const std::string& assemblyControllerRef);
    redhawk::ApplicationComponent* getAssemblyController();

    const std::string& getIdentifier() const;
    const std::string& getName() const;
    const std::string& getProfile() const;

    void addExternalPort (const std::string&, CORBA::Object_ptr);
    void addExternalProperty (const std::string&, const std::string&, const std::string &access, CF::Resource_ptr);

    // Returns true if any connections in this application depend on the given object, false otherwise
    bool checkConnectionDependency (ossie::Endpoint::DependencyType type, const std::string& identifier) const;
    
    void _cleanupActivations();

    // Set component state
    redhawk::ApplicationComponent* addComponent(const std::string& componentId, const std::string& softwareProfile);
    redhawk::ApplicationComponent* addCluster(const std::string& componentId, const std::string& softwareProfile);
    redhawk::ApplicationComponent* addComponent(const boost::shared_ptr<redhawk::GeneralDeployment> deployment);
    redhawk::ApplicationComponent* addContainer(const boost::shared_ptr<redhawk::GeneralDeployment> container);
    redhawk::ApplicationComponent* addCluster(const boost::shared_ptr<redhawk::GeneralDeployment> deployment);

    redhawk::ApplicationComponent* getLastComponent();

    void releaseComponents();
    void terminateComponents();
    void unloadComponents();

    void componentTerminated(const std::string& componentId, const std::string& deviceId);
    
    bool waitForCluster(std::set<std::string>& identifiers, int timeout);
    bool waitForComponents(std::set<std::string>& identifiers, int timeout);

    CF::Application_ptr getComponentApplication();
    CF::DomainManager_ptr getComponentDomainManager();

    redhawk::ApplicationComponent* getComponent(const std::string& identifier);

    /**
     * set the log level for one of the loggers on a component on the waveform
     * 
     * @throw CF::UnknownIdentifier
     */
    void setLogLevel( const char *logger_id, const CF::LogLevel newLevel );

    /**
     * get the log level from one of the loggers on a component on the waveform
     * 
     * @throw CF::UnknownIdentifier
     */
    CF::LogLevel getLogLevel( const char *logger_id );

    // retrieves the list of named loggers from all the components associated with the waveform
    CF::StringSequence* getNamedLoggers();

    // reset the loggers on all components on the waveform
    void resetLog();

    std::string GetStdoutFromCommand(std::string cmd);
    void poll(redhawk::ApplicationComponent& ii);
    bool isReleasing() {
        return _isReleasing;
    };

    redhawk::ApplicationComponent* findComponent(const std::string& identifier);

private:

    Application_impl (); // No default constructor
    Application_impl(Application_impl&);  // No copying

    struct PropertyChangeRecord {
      std::string               reg_id;
      CF::Resource_ptr          comp;

      PropertyChangeRecord( const std::string &id, CF::Resource_ptr inComp ) : reg_id(id), comp(inComp) {};
      PropertyChangeRecord( const PropertyChangeRecord &src ) { reg_id = src.reg_id; comp = src.comp; };
      ~PropertyChangeRecord() {};
    };

    typedef  std::vector< PropertyChangeRecord >                 PropertyChangeRecords;
    typedef  std::map< std::string, PropertyChangeRecords >      PropertyChangeRegistry;

    std::map<std::string, redhawk::PropertyMap> measuredDevices;
    bool haveAttribute(std::vector<std::string> &atts, std::string att);
    redhawk::PropertyMap measureComponent(redhawk::ApplicationComponent &component);
    redhawk::PropertyMap filterAttributes(redhawk::PropertyMap &attributes, std::vector<std::string> &filter);

    void registerComponent(CF::Resource_ptr resource);

    bool _checkRegistrations(std::set<std::string>& identifiers);
    bool _checkPodRegistrations(std::set<std::string>& identifiers);

    void _checkComponentConnections(redhawk::ApplicationComponent* component);

    redhawk::ApplicationComponent* _assemblyController;
    const std::string _identifier;
    const std::string _sadProfile;
    const std::string _appName;
    bool _isReleasing;
    CF::DeviceAssignmentSequence _componentDevices;
    std::vector<ossie::ConnectionNode> _connections;
    std::vector<redhawk::ApplicationComponent*> _startOrder;
    std::vector<std::string> _allocationIDs;
    DomainManager_impl* _domainManager;
    const std::string _waveformContextName;
    CosNaming::NamingContext_var _waveformContext;
    bool _started;
    const bool _isAware;
    float _stopTimeout;
    FakeApplication* _fakeProxy;
    
    ApplicationRegistrar_impl* _registrar;

    typedef std::list<redhawk::ApplicationComponent> ComponentList;
    typedef std::list<redhawk::ApplicationComponent> ClusterList;
    ComponentList _components;
    ClusterList _cluster;
    CosNaming::NamingContext_var _domainContext;

    boost::mutex _registrationMutex;
    boost::condition_variable _registrationCondition;

    std::map<std::string, CORBA::Object_var> _ports;
    std::map<std::string, ossie::externalPropertyType> _properties;

    bool _releaseAlreadyCalled;
    boost::mutex releaseObjectLock;
    boost::mutex metricsLock;

    PropertyChangeRegistry   _propertyChangeRegistrations;

    // Returns externalpropid if one exists based off of compId and
    // internal propId, returns empty string if no external prop exists
    std::string getExternalPropertyId(std::string compId, std::string propId);

    friend class ApplicationRegistrar_impl;
};

//Method to poll for k8s pod status
//void poll(redhawk::ApplicationComponent& ii);
//std::string GetStdoutFromCommand(std::string cmd);
#endif
