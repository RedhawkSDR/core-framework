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

#include <stdexcept>
#include <string>
#include <sstream>

#include <boost/foreach.hpp>

#include <ossie/debug.h>
#include <ossie/CorbaUtils.h>
#include <ossie/EventChannelSupport.h>
#include <ossie/PropertyMap.h>

#include "Application_impl.h"
#include "DomainManager_impl.h"
#include "AllocationManager_impl.h"
#include "ApplicationRegistrar.h"
#include "connectionSupport.h"
#include "FakeApplication.h"
#include "DeploymentExceptions.h"

PREPARE_CF_LOGGING(Application_impl);

using namespace ossie;

namespace {
    CF::Application::ComponentElementType to_impl_element(const redhawk::ApplicationComponent& component)
    {
        CF::Application::ComponentElementType result;
        result.componentId = component.getIdentifier().c_str();
        result.elementId = component.getImplementationId().c_str();
        return result;
    }

    CF::Application::ComponentElementType to_name_element(const redhawk::ApplicationComponent& component)
    {
        CF::Application::ComponentElementType result;
        result.componentId = component.getIdentifier().c_str();
        result.elementId = component.getNamingContext().c_str();
        return result;
    }

    CF::Application::ComponentProcessIdType to_pid_type(const redhawk::ApplicationComponent& component)
    {
        CF::Application::ComponentProcessIdType result;
        result.componentId = component.getIdentifier().c_str();
        result.processId = component.getProcessId();
        return result;
    }

    CF::ComponentType to_component_type(const redhawk::ApplicationComponent& component)
    {
        CF::ComponentType result;
        result.identifier = component.getIdentifier().c_str();
        result.softwareProfile = component.getSoftwareProfile().c_str();
        result.type = CF::APPLICATION_COMPONENT;
        result.componentObject = component.getComponentObject();
        return result;
    }

    template <class Sequence, class Iterator, class Function>
    void convert_sequence(Sequence& out, Iterator begin, const Iterator end, Function func)
    {
        for (; begin != end; ++begin) {
            if (begin->isVisible()) {
                ossie::corba::push_back(out, func(*begin));
            }
        }
    }

    template <class Sequence, class Container, class Function>
    void convert_sequence(Sequence& out, Container& in, Function func)
    {
        convert_sequence(out, in.begin(), in.end(), func);
    }

    template <class Sequence, class Iterator, class Function, class Predicate>
    void convert_sequence_if(Sequence& out, Iterator begin, const Iterator end, Function func, Predicate pred)
    {
        for (; begin != end; ++begin) {
            if (begin->isVisible() && pred(*begin)) {
                ossie::corba::push_back(out, func(*begin));
            }
        }
    }

    template <class Sequence, class Container, class Function, class Predicate>
    void convert_sequence_if(Sequence& out, Container& in, Function func, Predicate pred)
    {
        convert_sequence_if(out, in.begin(), in.end(), func, pred);
    }
}

Application_impl::Application_impl (const std::string& id, const std::string& name, const std::string& profile,
                                    DomainManager_impl* domainManager, const std::string& waveformContextName,
                                    CosNaming::NamingContext_ptr waveformContext, bool aware,
                                    float stopTimeout, CosNaming::NamingContext_ptr DomainContext) :
    Logging_impl(domainManager->getInstanceLogger("Application")),
    _assemblyController(0),
    _identifier(id),
    _sadProfile(profile),
    _appName(name),
    _domainManager(domainManager),
    _waveformContextName(waveformContextName),
    _waveformContext(CosNaming::NamingContext::_duplicate(waveformContext)),
    _started(false),
    _isAware(aware),
    _stopTimeout(stopTimeout),
    _fakeProxy(0),
    _domainContext(CosNaming::NamingContext::_duplicate(DomainContext)),
    _releaseAlreadyCalled(false)
{
    _registrar = new ApplicationRegistrar_impl(waveformContext, this);
    if (!_isAware) {
        _fakeProxy = new FakeApplication(this);
    }
};

void Application_impl::setAssemblyController(const std::string& assemblyControllerRef)
{
    RH_DEBUG(_baseLog, "Assigning the assembly controller")
    _assemblyController = findComponent(assemblyControllerRef);
    // Assume _controller is NIL implies that the assembly controller component is Non SCA-Compliant
    if (!_assemblyController || !_assemblyController->isResource()) {
        RH_INFO(_baseLog, "Assembly controller is non SCA-compliant");
        _assemblyController = 0;
    }
}

redhawk::ApplicationComponent* Application_impl::getAssemblyController()
{
    return _assemblyController;
}

void Application_impl::populateApplication(const CF::DeviceAssignmentSequence& assignedDevices,
                                           std::vector<ConnectionNode>& connections,
                                           std::vector<std::string> allocationIDs)
{
    _connections = connections;
    _componentDevices = assignedDevices;

    RH_DEBUG(_baseLog, "Creating allocation sequence");
    this->_allocationIDs = allocationIDs;
}

void Application_impl::setStartOrder(const std::vector<std::string>& startOrder)
{
    _startOrder.clear();
    BOOST_FOREACH(const std::string& componentId, startOrder) {
        redhawk::ApplicationComponent* component = findComponent(componentId);
        if (component) {
            _startOrder.push_back(component);
        } else {
            RH_WARN(_baseLog, "Invalid component '" << componentId << "' in start order");
        }
    }
}

Application_impl::~Application_impl ()
{
};

PortableServer::ObjectId* Application_impl::Activate(Application_impl* application)
{
    // The DomainManager POA must exist, but the  Applications POA might not
    // have been created yet.
    PortableServer::POA_var dm_poa = ossie::corba::RootPOA()->find_POA("DomainManager", 0);
    PortableServer::POA_var poa = dm_poa->find_POA("Applications", 1);
    PortableServer::ObjectId_var oid = ossie::corba::activatePersistentObject(poa, 
                                                                              application, 
                                                                              application->_identifier);

    const std::string registryId = application->_identifier + ".registry";
    PortableServer::ObjectId_var reg_oid = ossie::corba::activatePersistentObject(poa,
                                                                                  application->_registrar,
                                                                                  registryId);

    // If the application is not aware, activate the fake proxy
    if (!application->aware()) {
        const std::string proxyId = application->_identifier + ".fake";
        reg_oid = ossie::corba::activatePersistentObject(poa, application->_fakeProxy, proxyId);
    }

    return oid._retn();
}

char* Application_impl::identifier () throw (CORBA::SystemException)
{
    return CORBA::string_dup(_identifier.c_str());
}

CORBA::Boolean Application_impl::started () throw (CORBA::SystemException)
{
    return this->_started;
}

void Application_impl::setLogLevel( const char *logger_id, const CF::LogLevel newLevel ) throw (CF::UnknownIdentifier)
{
    BOOST_FOREACH(redhawk::ApplicationComponent component, _components) {
        if (not component.isRegistered() or not component.isVisible())
            continue;
        CF::Resource_var resource_ref = component.getResourcePtr();
        try {
            resource_ref->setLogLevel(logger_id, newLevel);
            return;
        } catch (const CF::UnknownIdentifier& ex) {
        }
    }
    throw (CF::UnknownIdentifier());
}

CF::LogLevel Application_impl::getLogLevel( const char *logger_id ) throw (CF::UnknownIdentifier)
{
    BOOST_FOREACH(redhawk::ApplicationComponent component, _components) {
        if (not component.isRegistered() or not component.isVisible())
            continue;
        CF::Resource_var resource_ref = component.getResourcePtr();
        try {
            CF::LogLevel level = resource_ref->getLogLevel(logger_id);
            return level;
        } catch (const CF::UnknownIdentifier& ex) {
        }
    }
    throw (CF::UnknownIdentifier());
}

CF::StringSequence* Application_impl::getNamedLoggers()
{
    CF::StringSequence_var retval = new CF::StringSequence();
    BOOST_FOREACH(redhawk::ApplicationComponent component, _components) {
        if (not component.isRegistered() or not component.isVisible())
            continue;
        CF::Resource_var resource_ref = component.getResourcePtr();
        CF::StringSequence_var component_logger_list = resource_ref->getNamedLoggers();
        for (unsigned int i=0; i<component_logger_list->length(); i++) {
            ossie::corba::push_back(retval, CORBA::string_dup(component_logger_list[i]));
        }
    }
    return retval._retn();
}

void Application_impl::resetLog()
{
    BOOST_FOREACH(redhawk::ApplicationComponent component, _components) {
        if (not component.isRegistered() or not component.isVisible())
            continue;
        CF::Resource_var resource_ref = component.getResourcePtr();
        resource_ref->resetLog();
    }
}

void Application_impl::start ()
throw (CORBA::SystemException, CF::Resource::StartError)
{
    // Make sure releaseObject hasn't already been called
    {
        boost::mutex::scoped_lock lock(releaseObjectLock);

        if (_releaseAlreadyCalled) {
            RH_DEBUG(_baseLog, "skipping start call because releaseObject has been called");
            return;
        }
    }

   if (!_assemblyController && _startOrder.empty()) {
        throw CF::Resource::StartError(CF::CF_ENOTSUP, "No assembly controller and no Components with startorder set");
    }

    try {
        if (_assemblyController) {
            RH_TRACE(_baseLog, "Calling start on assembly controller");
            _assemblyController->start();
        }

        // Start the rest of the components
        BOOST_FOREACH(redhawk::ApplicationComponent* component, _startOrder) {
            RH_TRACE(_baseLog, "Calling start for " << component->getIdentifier());
            component->start();
        }
    } catch (const CF::Resource::StartError& se) {
        RH_ERROR(_baseLog, "Failed to start application '" << _appName << "': " << se.msg);
        throw;
    }

    if (!this->_started) {
        this->_started = true;
        if (_domainManager ) {
            _domainManager->sendResourceStateChange( _identifier, 
                    this->_appName,
                    ExtendedEvent::STOPPED,
                    ExtendedEvent::STARTED);
        }
    }
}

void Application_impl::stop ()
throw (CORBA::SystemException, CF::Resource::StopError)
{
    // Make sure releaseObject hasn't already been called
    {
        boost::mutex::scoped_lock lock(releaseObjectLock);

        if (_releaseAlreadyCalled) {
            RH_DEBUG(_baseLog, "skipping stop call because releaseObject has been called");
            return;
        }
    }
    this->local_stop(this->_stopTimeout);
}

void Application_impl::local_stop (float timeout)
throw (CORBA::SystemException, CF::Resource::StopError)
{
    if (!_assemblyController && _startOrder.empty()) {
        throw CF::Resource::StopError(CF::CF_ENOTSUP, "No assembly controller and no Components with startorder set");
    }

    int failures = 0;
    // Stop the components in the reverse order they were started
    BOOST_REVERSE_FOREACH(redhawk::ApplicationComponent* component, _startOrder) {
        RH_TRACE(_baseLog, "Calling stop for " << component->getIdentifier());
        if (!component->stop(timeout)) {
            failures++;
        }
    }

    if (_assemblyController) {
        RH_TRACE(_baseLog, "Calling stop on assembly controller");
        if (!_assemblyController->stop(timeout)) {
            failures++;
        }
    }

    if (failures > 0) {
        std::ostringstream oss;
        oss << failures << " component(s) failed to stop";
        const std::string message = oss.str();
        RH_ERROR(_baseLog, "Stopping " << _identifier << "; " << message);
        throw CF::Resource::StopError(CF::CF_NOTSET, message.c_str());
    }
    if (this->_started) {
        this->_started = false;
        if (_domainManager ) {
            _domainManager->sendResourceStateChange( _identifier, 
                    this->_appName,
                    ExtendedEvent::STARTED,
                    ExtendedEvent::STOPPED);
        }
    }
}

CORBA::Float Application_impl::stopTimeout () throw (CORBA::SystemException) {
    return this->_stopTimeout;
}

void Application_impl::stopTimeout (CORBA::Float timeout) throw (CORBA::SystemException) {
    this->_stopTimeout = timeout;
}

void Application_impl::initializeProperties (const CF::Properties& configProperties)
throw (CF::PropertySet::PartialConfiguration, CF::PropertySet::InvalidConfiguration, CORBA::SystemException)
{
    // Make sure releaseObject hasn't already been called
    {
        boost::mutex::scoped_lock lock(releaseObjectLock);

        if (_releaseAlreadyCalled) {
            RH_DEBUG(_baseLog, "skipping initializeProperties call because releaseObject has been called");
            return;
        }
    }
}

void Application_impl::configure (const CF::Properties& configProperties)
throw (CF::PropertySet::PartialConfiguration, CF::PropertySet::InvalidConfiguration, CORBA::SystemException)
{
    redhawk::PropertyMap invalidProperties;

    // Creates a map from componentIdentifier -> (rsc_ptr, ConfigPropSet)
    // to allow for one batched configure call per component
    const std::string& acId = _assemblyController->getIdentifier();
    CF::Resource_var ac_resource = _assemblyController->getResourcePtr();
    std::map<std::string, std::pair<CF::Resource_ptr, CF::Properties> > batch;
    batch[acId] = std::pair<CF::Resource_ptr, CF::Properties>(ac_resource, CF::Properties());

    // Loop through each passed external property, mapping it with its respective resource
    for (unsigned int i = 0; i < configProperties.length(); ++i) {
        // Gets external ID for property mapping
        const std::string extId(configProperties[i].id);

        if (_properties.count(extId)) {
            // Gets the component and its internal property id
            const std::string propId = _properties[extId].property_id;
            CF::Resource_ptr comp = _properties[extId].component;

            if (CORBA::is_nil(comp)) {
                RH_ERROR(_baseLog, "Unable to retrieve component for external property: " << extId);
                invalidProperties.push_back(configProperties[i]);
            } else {
                // Key used for map
                const std::string compId = ossie::corba::returnString(comp->identifier());

                RH_TRACE(_baseLog, "Configure external property: " << extId << " on "
                        << compId << " (propid: " << propId << ")");

                // Adds property to component ID mapping
                // If one doesn't exist, adds it
                if (batch.count(compId)) {
                    int count = batch[compId].second.length();
                    batch[compId].second.length(count + 1);
                    batch[compId].second[count].id = propId.c_str();
                    batch[compId].second[count].value = configProperties[i].value;
                } else {
                    CF::Properties tempProp;
                    tempProp.length(1);
                    tempProp[0].id = propId.c_str();
                    tempProp[0].value = configProperties[i].value;
                    batch[compId] = std::pair<CF::Resource_ptr, CF::Properties>(comp, tempProp);
                }
            }
        } else if (_assemblyController) {
            // Properties that are not external get batched with assembly controller
            RH_TRACE(_baseLog, "Calling configure on assembly controller for property: " << configProperties[i].id);
            int count = batch[acId].second.length();
            batch[acId].second.length(count + 1);
            batch[acId].second[count].id = configProperties[i].id;
            batch[acId].second[count].value = configProperties[i].value;
        } else {
            RH_ERROR(_baseLog, "Unable to retrieve assembly controller for external property: " << extId);
            invalidProperties.push_back(configProperties[i]);
        }
    }

    // -Loop through each component with a property that needs to be configured and make configure call
    // -Catch any errors
    for (std::map<std::string, std::pair<CF::Resource_ptr, CF::Properties> >::const_iterator comp = batch.begin();
            comp != batch.end(); ++comp) {
        try {
            comp->second.first->configure(comp->second.second);
        } catch (CF::PropertySet::InvalidConfiguration e) {
            // Add invalid properties to return list
            invalidProperties.extend(e.invalidProperties);
        } catch (CF::PropertySet::PartialConfiguration e) {
            // Add invalid properties to return list
            invalidProperties.extend(e.invalidProperties);
        }
    }

    // Throw appropriate exception if any configure errors were handled
    if (!invalidProperties.empty()) {
        if (invalidProperties.size() < configProperties.length()) {
            throw CF::PropertySet::PartialConfiguration(invalidProperties);
        } else {
            throw CF::PropertySet::InvalidConfiguration("No matching external properties found", invalidProperties);
        }
    }
}


void Application_impl::query (CF::Properties& configProperties)
throw (CF::UnknownProperties, CORBA::SystemException)
{
    redhawk::PropertyMap invalidProperties;

    // Creates a map from componentIdentifier -> (rsc_ptr, ConfigPropSet)
    // to allow for one batched query call per component
    const std::string acId = _assemblyController->getIdentifier();
    std::map<std::string, std::pair<CF::Resource_ptr, CF::Properties> > batch;

    // For queries of zero length, return all external properties
    if (configProperties.length() == 0) {
        RH_TRACE(_baseLog, "Query all external and assembly controller properties");

        configProperties.length(0);

        // Loop through each external property and add it to the batch with its respective component
        for (std::map<std::string, externalPropertyType>::const_iterator prop = _properties.begin();
                prop != _properties.end(); ++prop) {
            // Gets the property mapping info
            std::string extId = prop->first;
            std::string propId = prop->second.property_id;
            CF::Resource_ptr comp = prop->second.component;

            if (prop->second.access == "writeonly")
                continue;

            if (CORBA::is_nil(comp)) {
                RH_ERROR(_baseLog, "Unable to retrieve component for external property: " << extId);
                invalidProperties.push_back(redhawk::PropertyType(extId));
            } else {
                // Key used for map
                const std::string compId = ossie::corba::returnString(comp->identifier());

                RH_TRACE(_baseLog, "Query external property: " << extId << " on "
                        << compId << " (propid: " << propId << ")");

                // Adds property to component ID mapping
                // If one doesn't exist, creates it
                if (batch.count(compId)) {
                    int count = batch[compId].second.length();
                    batch[compId].second.length(count + 1);
                    batch[compId].second[count].id = propId.c_str();
                } else {
                    CF::Properties tempProp;
                    tempProp.length(1);
                    tempProp[0].id = propId.c_str();
                    batch[compId] = std::pair<CF::Resource_ptr, CF::Properties>(comp, tempProp);
                }
            }
        }

        // -Loop through each component with an external property and make query() call
        // -Catch any errors
        for (std::map<std::string, std::pair<CF::Resource_ptr, CF::Properties> >::iterator comp = batch.begin();
                comp != batch.end(); ++comp) {
            try {
                comp->second.first->query(comp->second.second);

                // Adds each individual queried property
                for (unsigned int i = 0; i < comp->second.second.length(); ++i) {
                    // Gets the external property ID from the component ID and internal prop ID
                    std::string extId = getExternalPropertyId(comp->first, std::string(comp->second.second[i].id));
                    int count = configProperties.length();
                    configProperties.length(count + 1);
                    configProperties[count].id = CORBA::string_dup(extId.c_str());
                    configProperties[count].value = comp->second.second[i].value;
                }
            } catch (CF::UnknownProperties e) {
                invalidProperties.extend(e.invalidProperties);
            }
        }

        // Query Assembly Controller properties
        CF::Properties tempProp;
        try {
            CF::Resource_var ac_resource = _assemblyController->getResourcePtr();
            ac_resource->query(tempProp);
        } catch (CF::UnknownProperties e) {
            for (unsigned int i = 0; i < e.invalidProperties.length(); ++i) {
                RH_ERROR(_baseLog, "Invalid assembly controller property name: " << e.invalidProperties[i].id);
            }
            invalidProperties.extend(e.invalidProperties);
        }

        // Adds Assembly Controller properties
        for (unsigned int i = 0; i < tempProp.length(); ++i) {
            // Only add AC props that aren't already promoted as external
            if (this->getExternalPropertyId(acId, std::string(tempProp[i].id)) == "") {
                int count = configProperties.length();
                configProperties.length(count + 1);
                configProperties[count].id = CORBA::string_dup(tempProp[i].id);
                configProperties[count].value = tempProp[i].value;
            }
        }
    } else {
        // For queries of length > 0, return all requested pairs that are valid external properties
        // or are Assembly Controller Properties
        CF::Properties acProps;
        CF::Resource_var ac_resource = _assemblyController->getResourcePtr();
        batch[acId] = std::pair<CF::Resource_ptr, CF::Properties>(ac_resource, acProps);

        for (unsigned int i = 0; i < configProperties.length(); ++i) {
            // Gets external ID for property mapping
            const std::string extId(configProperties[i].id);

            if (_properties.count(extId)) {
                if (_properties[extId].access == "writeonly") {
                    LOG_ERROR(Application_impl, "Cannot read writeonly external property: " << extId);
                    int count = invalidProperties.length();
                    invalidProperties.length(count + 1);
                    invalidProperties[count].id = CORBA::string_dup(extId.c_str());
                    invalidProperties[count].value = configProperties[i].value;
                    continue;
                }

                // Gets the component and its property id
                const std::string propId = _properties[extId].property_id;
                CF::Resource_ptr comp = _properties[extId].component;

                if (CORBA::is_nil(comp)) {
                    RH_ERROR(_baseLog, "Unable to retrieve component for external property: " << extId);
                    invalidProperties.push_back(configProperties[i]);
                } else {
                    // Key used for map
                    std::string compId = ossie::corba::returnString(comp->identifier());

                    RH_TRACE(_baseLog, "Query external property: " << extId << " on "
                            << compId << " (propid: " << propId << ")");

                    // Adds property to component ID mapping
                    // If one doesn't exist, adds it
                    if (batch.count(compId)) {
                        int count = batch[compId].second.length();
                        batch[compId].second.length(count + 1);
                        batch[compId].second[count].id = propId.c_str();
                        batch[compId].second[count].value = configProperties[i].value;
                    } else {
                        CF::Properties tempProp;
                        tempProp.length(1);
                        tempProp[0].id = propId.c_str();
                        tempProp[0].value = configProperties[i].value;
                        batch[compId] = std::pair<CF::Resource_ptr, CF::Properties>(comp, tempProp);
                    }
                }
            } else if (_assemblyController) {
                // Properties that are not external get batched with assembly controller
                RH_TRACE(_baseLog, "Calling query on assembly controller for property: "
                        << configProperties[i].id);
                int count = batch[acId].second.length();
                batch[acId].second.length(count + 1);
                batch[acId].second[count].id = configProperties[i].id;
                batch[acId].second[count].value = configProperties[i].value;
            } else {
                RH_ERROR(_baseLog, "Unable to retrieve assembly controller for external property: " << extId);
                invalidProperties.push_back(configProperties[i]);
            }
        }

        // -Loop through each component with a property that needs to be queried and make query() call
        // -Catch any errors
        for (std::map<std::string, std::pair<CF::Resource_ptr, CF::Properties> >::iterator comp = batch.begin();
                comp != batch.end(); ++comp) {
            try {
                comp->second.first->query(comp->second.second);
            } catch (CF::UnknownProperties e) {
                invalidProperties.extend(e.invalidProperties);
            }
        }

        // Loops through requested property IDs to find value that was returned from internal call
        for (unsigned int i = 0; i < configProperties.length(); ++i) {
            const std::string extId(configProperties[i].id);
            std::string propId;
            std::string compId;

            // Checks if property ID is external or AC property
            if (_properties.count(extId)) {
                propId = _properties[extId].property_id;
                compId = ossie::corba::returnString(_properties[extId].component->identifier());
            } else {
                propId = extId;
                compId = _assemblyController->getIdentifier();
            }

            // Loops through batched query results finding requested property
            for (std::map<std::string, std::pair<CF::Resource_ptr, CF::Properties> >::const_iterator comp = batch.begin();
                    comp != batch.end(); ++comp) {
                for (unsigned int j = 0; j < comp->second.second.length(); ++j) {
                    const std::string currPropId(comp->second.second[j].id);
                    if (compId == comp->first && propId == currPropId) {
                        configProperties[i].value = comp->second.second[j].value;
                    }
                }
            }
        }
    }

    if (!invalidProperties.empty()) {
        throw CF::UnknownProperties(invalidProperties);
    }

    RH_TRACE(_baseLog, "Query returning " << configProperties.length() <<
            " external and assembly controller properties");
}


char *Application_impl::registerPropertyListener( CORBA::Object_ptr listener, const CF::StringSequence &prop_ids, const CORBA::Float interval)
  throw(CF::UnknownProperties, CF::InvalidObjectReference)
{

  SCOPED_LOCK( releaseObjectLock );
  if (_releaseAlreadyCalled) {
    RH_DEBUG(_baseLog, "skipping registerPropertyListener call because releaseObject has been called");
    std::string regid;
    return CORBA::string_dup(regid.c_str());
  }

  RH_TRACE(_baseLog, "Number of Properties to Register: " << prop_ids.length() );
  typedef   std::map< CF::Resource_ptr, std::vector< std::string > > CompRegs;
  CompRegs comp_regs;

  for (unsigned int i = 0; i < prop_ids.length(); ++i) {
        // Gets external ID for property mapping
        const std::string extId(prop_ids[i]);

        if (_properties.count(extId)) {
          CF::Resource_ptr comp = _properties[extId].component;
          std::string prop_id = _properties[extId].property_id;
          RH_TRACE(_baseLog, "  ---> Register ExternalID: " << extId << " Comp/Id " << 
                ossie::corba::returnString(comp->identifier()) << "/" << prop_id);
          comp_regs[ comp ].push_back( prop_id ); 
        } else if (_assemblyController) {
            CF::Resource_var ac_resource = _assemblyController->getResourcePtr();
            comp_regs[ac_resource].push_back(extId);
        }
  }

  CompRegs::iterator reg_iter = comp_regs.begin();
  PropertyChangeRecords   pc_recs;
  try {
    
    for ( ; reg_iter != comp_regs.end(); reg_iter++ ) {

      CF::StringSequence reg_ids;
      reg_ids.length( reg_iter->second.size() );
      for ( uint32_t i=0; i < reg_iter->second.size(); i++ ) reg_ids[i] = reg_iter->second[i].c_str();
      
      std::string reg_id = ossie::corba::returnString( reg_iter->first->registerPropertyListener( listener, reg_ids, interval ) );
      RH_TRACE(_baseLog, "Component-->PropertyChangeRegistryRegistry  comp/id " << 
                ossie::corba::returnString(reg_iter->first->identifier()) << "/" << reg_id );
      pc_recs.push_back( PropertyChangeRecord( reg_id, reg_iter->first ) );

    }

  }
  catch (...) {
    
    RH_WARN(_baseLog, "PropertyChangeListener registration failed against Application: " << _identifier );
    PropertyChangeRecords::iterator iter = pc_recs.begin(); 
    try {
      iter->comp->unregisterPropertyListener( iter->reg_id.c_str() );
    }
    catch(...) {
    }
    
    throw;
  }

  // save off registrations when we unregister
  std::string regid = ossie::generateUUID();
  _propertyChangeRegistrations[ regid ] = pc_recs;
  return CORBA::string_dup(regid.c_str() );
}

void Application_impl::unregisterPropertyListener( const char *reg_id ) 
  throw (CF::InvalidIdentifier)
{
  SCOPED_LOCK( releaseObjectLock );
  if (_releaseAlreadyCalled) {
    RH_DEBUG(_baseLog, "skipping unregisterPropertyListener call because releaseObject has been called");
    return;
  }

  if ( _propertyChangeRegistrations.count( reg_id ) == 0 )  {
    throw CF::InvalidIdentifier();
  }
  else {
    
    PropertyChangeRecords::iterator iter = _propertyChangeRegistrations[reg_id].begin();
    PropertyChangeRecords::iterator end = _propertyChangeRegistrations[reg_id].end();

    for( ; iter != end; iter++ ) {
      try {
        PropertyChangeRecord &rec = *iter;
        if ( CORBA::is_nil(rec.comp) == false ) {
          rec.comp->unregisterPropertyListener( rec.reg_id.c_str() ) ;
        }
      }
      catch(...){
        RH_WARN(_baseLog, "Unregister PropertyChangeListener operation failed. app/reg_id: " << _identifier << "/" << reg_id );
      }
    }
    
    _propertyChangeRegistrations.erase( reg_id );
    
  }

}

void Application_impl::initialize ()
throw (CORBA::SystemException, CF::LifeCycle::InitializeError)
{
    // Make sure releaseObject hasn't already been called
    {
        boost::mutex::scoped_lock lock(releaseObjectLock);

        if (_releaseAlreadyCalled) {
            RH_DEBUG(_baseLog, "skipping initialize call because releaseObject has been called");
            return;
        }
    }

    if (!_assemblyController) {
        return;
    }

    try {
        RH_TRACE(_baseLog, "Calling initialize on assembly controller");
        CF::Resource_var resource = _assemblyController->getResourcePtr();
        resource->initialize();
    } catch( CF::LifeCycle::InitializeError& ie ) {
        RH_ERROR(_baseLog, "Initialize failed with CF::LifeCycle::InitializeError")
        throw;
    } CATCH_THROW_RH_ERROR(_baseLog, "Initialize failed", CF::LifeCycle::InitializeError())
}


CORBA::Object_ptr Application_impl::getPort (const char* _id)
throw (CORBA::SystemException, CF::PortSupplier::UnknownPort)
{

    SCOPED_LOCK( releaseObjectLock );
    if (_releaseAlreadyCalled) {
        RH_DEBUG(_baseLog, "skipping getPort because release has already been called");
        return CORBA::Object::_nil();
    }

    const std::string identifier = _id;
    if (_ports.count(identifier)) {
        return CORBA::Object::_duplicate(_ports[identifier]);
    } else {
        RH_ERROR(_baseLog, "Get port failed with unknown port " << _id)
        throw(CF::PortSupplier::UnknownPort());
    }
}



CF::PortSet::PortInfoSequence* Application_impl::getPortSet ()
{
    SCOPED_LOCK( releaseObjectLock );
    CF::PortSet::PortInfoSequence_var retval = new CF::PortSet::PortInfoSequence();
    if (_releaseAlreadyCalled) {
        RH_DEBUG(_baseLog, "skipping getPortSet because release has alraeady been called");
        return retval._retn();
    }

    std::vector<CF::PortSet::PortInfoSequence_var> comp_portsets;
    for (ComponentList::iterator _component_iter=this->_components.begin(); _component_iter!=this->_components.end(); _component_iter++) {
        try {
            CF::Resource_var comp = _component_iter->getResourcePtr();
            comp_portsets.push_back(comp->getPortSet());
        } catch ( CORBA::COMM_FAILURE &ex ) {
            RH_ERROR(_baseLog, "Component getPortSet failed, application: " << _identifier << " comp:" << _component_iter->getIdentifier() << "/" << _component_iter->getNamingContext() );            
        } catch ( ... ) {
            RH_ERROR(_baseLog, "Unhandled exception during getPortSet, application: " << _identifier << " comp:" << _component_iter->getIdentifier() << "/" << _component_iter->getNamingContext() );            
        }
    }
    for (std::map<std::string, CORBA::Object_var>::iterator _port_val=_ports.begin(); _port_val!=_ports.end(); _port_val++) {
        for (std::vector<CF::PortSet::PortInfoSequence_var>::iterator comp_portset=comp_portsets.begin(); comp_portset!=comp_portsets.end(); comp_portset++) {
            for (unsigned int i=0; i<(*comp_portset)->length(); i++) {
                try {
                    if (_port_val->second->_is_equivalent((*comp_portset)[i].obj_ptr)) {
                        CF::PortSet::PortInfoType info;
                        info.obj_ptr = (*comp_portset)[i].obj_ptr;
                        info.name = _port_val->first.c_str();
                        info.repid =(*comp_portset)[i].repid;
                        info.description = (*comp_portset)[i].description;
                        info.direction = (*comp_portset)[i].direction;
                        ossie::corba::push_back(retval, info);
                    }
                } catch ( ... ) {
                    // unable to add port reference
                }
            }
        }
    }
    if (_ports.size() != retval->length()) {
        // some of the components are unreachable and the list is incomplete
        for (std::map<std::string, CORBA::Object_var>::iterator _port_val=_ports.begin(); _port_val!=_ports.end(); _port_val++) {
            bool foundPort = false;
            for (unsigned int i=0; i<retval->length(); i++) {
                std::string retvalPortName(retval[i].name);
                if (retvalPortName == _port_val->first) {
                    foundPort = true;
                    break;
                }
            }
            if (not foundPort) {
                CF::PortSet::PortInfoType info;
                info.obj_ptr = CORBA::Object::_nil();
                info.name = _port_val->first.c_str();
                info.repid = "";
                info.description = "";
                info.direction = "";
                ossie::corba::push_back(retval, info);
            }
        }
    }
	return retval._retn();
}


void Application_impl::runTest (CORBA::ULong _testId, CF::Properties& _props)
throw (CORBA::SystemException, CF::UnknownProperties, CF::TestableObject::UnknownTest)

{
    // Make sure releaseObject hasn't already been called
    {
        boost::mutex::scoped_lock lock(releaseObjectLock);

        if (_releaseAlreadyCalled) {
            RH_DEBUG(_baseLog, "skipping runTest call because releaseObject has been called");
            return;
        }
    }

    if (!_assemblyController) {
        RH_ERROR(_baseLog, "Run test called with non SCA compliant assembly controller");
        throw CF::TestableObject::UnknownTest();
    }

    try {
        RH_TRACE(_baseLog, "Calling runTest on assembly controller");
        CF::Resource_var resource = _assemblyController->getResourcePtr();
        resource->runTest(_testId, _props);
    } catch( CF::UnknownProperties& up ) {
        std::ostringstream eout;
        eout << "Run test failed with CF::UnknownProperties for Test ID " << _testId << " for properties: ";
        for (unsigned int i=0; i<up.invalidProperties.length(); i++) {
            eout << up.invalidProperties[i].id;
            if (i == up.invalidProperties.length()-1)
                eout << ".";
            else
                eout << ", ";
        }
        RH_ERROR(_baseLog, eout.str())
        throw;
    } catch( CF::TestableObject::UnknownTest& ) {
        RH_ERROR(_baseLog, "Run test failed with CF::TestableObject::UnknownTest for Test ID " << _testId)
        throw;
    } CATCH_RETHROW_RH_ERROR(_baseLog, "Run test failed for Test ID " << _testId)
}


void Application_impl::releaseObject ()
throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
  try {
    // Make sure releaseObject hasn't already been called, but only hold the
    // lock long enough to check to prevent a potential priority inversion with
    // the domain's stateAccess mutex
    {
        boost::mutex::scoped_lock lock(releaseObjectLock);

        if (_releaseAlreadyCalled) {
            RH_DEBUG(_baseLog, "skipping release because release has already been called");
            return;
        } else {
            _releaseAlreadyCalled = true;
        }
    }
    
    RH_DEBUG(_baseLog, "Releasing application");

    // remove application from DomainManager's App Sequence
    try {
        _domainManager->removeApplication(_identifier);
    } catch (CF::DomainManager::ApplicationUninstallationError& ex) {
        RH_ERROR(_baseLog, ex.msg);
    }

    // Stop all components on the application
    try {
        this->local_stop(DEFAULT_STOP_TIMEOUT);
    } catch ( ... ) {
        // error happened while stopping. Ignore the error and continue tear-down
        RH_TRACE(_baseLog, "Error occurred while stopping the application during tear-down. Ignoring the error and continuing")
    }
    
    try {
      // Break all connections in the application
      ConnectionManager::disconnectAll(_connections, _domainManager);
      RH_DEBUG(_baseLog, "app->releaseObject finished disconnecting ports");
    } CATCH_RH_ERROR(_baseLog, "Failure during disconnect operation");

    // Release all resources
    // Before releasing the components, all executed processes should be terminated,
    // all software components unloaded, and all capacities deallocated
    // search thru all waveform components
    // unload and deallocate capacity

    releaseComponents();

    // Search thru all waveform components
    //  - unbind from NS
    //  - release each component
    //  - unload and deallocate
    for (ComponentList::iterator ii = _components.begin(); ii != _components.end(); ++ii) {

        if (ii->hasNamingContext()) {
            const std::string& componentName = ii->getNamingContext();

            // Unbind the component from the naming context. This assumes that the component is
            // bound into the waveform context, and its name inside of the context follows the
            // last slash in the fully-qualified name.
            std::string shortName = componentName.substr(componentName.rfind('/')+1);
            RH_TRACE(_baseLog, "Unbinding component " << shortName);
            CosNaming::Name_var componentBindingName = ossie::corba::stringToName(shortName);
            try {
                _waveformContext->unbind(componentBindingName);
            } CATCH_RH_ERROR(_baseLog, "Unable to unbind component")
        }
        RH_DEBUG(_baseLog, "Next component")
    }

    terminateComponents();
    unloadComponents();

    // deallocate capacities
    try {
        this->_domainManager->_allocationMgr->deallocate(this->_allocationIDs.begin(), this->_allocationIDs.end());
    } catch (const CF::AllocationManager::InvalidAllocationId& iad) {
        std::ostringstream err;
        err << "Tried to deallocate invalid allocation IDs: ";
        for (size_t ii = 0; ii < iad.invalidAllocationIds.length(); ++ii) {
            if (ii > 0) {
                err << ", ";
            }
            err << iad.invalidAllocationIds[ii];
        }
        RH_ERROR(_baseLog, err.str());
    }

    // Unbind the application's naming context using the fully-qualified name.
    RH_TRACE(_baseLog, "Unbinding application naming context " << _waveformContextName);
    CosNaming::Name DNContextname;
    DNContextname.length(1);
    DNContextname[0].id = CORBA::string_dup(_waveformContextName.c_str());
    try {
      if ( CORBA::is_nil(_domainContext) ==  false ) {
        _domainContext->unbind(DNContextname);
      }
    } catch (const CosNaming::NamingContext::NotFound&) {
        // Someone else has removed the naming context; this is a non-fatal condition.
        RH_WARN(_baseLog, "Naming context has already been removed");
    } CATCH_RH_ERROR(_baseLog, "Unbind context failed with CORBA::SystemException")

    // Destroy the waveform context; it should be empty by this point, assuming all
    // of the components were properly unbound.
    RH_TRACE(_baseLog, "Destroying application naming context " << _waveformContextName);
    try {
        _waveformContext->destroy();
    } catch (const CosNaming::NamingContext::NotEmpty&) {
        const char* error = "Application naming context not empty";
        RH_ERROR(_baseLog, error);
        CF::StringSequence message;
        message.length(1);
        message[0] = CORBA::string_dup(error);
        throw CF::LifeCycle::ReleaseError(message);
    } CATCH_RH_ERROR(_baseLog, "Destory waveform context: " << _waveformContextName );
    _waveformContext = CosNaming::NamingContext::_nil();

    // send application removed event notification
    if (_domainManager ) {
        // Send an event with the Application releaseObject
      _domainManager->sendRemoveEvent( _identifier.c_str(), 
                                       _identifier.c_str(), 
                                       _appName.c_str(),
                                       StandardEvent::APPLICATION);
    }

    // Deactivate this servant from the POA.
    this->_cleanupActivations();

  }
  catch( boost::thread_resource_error &e)  {
    std::stringstream errstr;
    errstr << "Error acquiring lock (errno=" << e.native_error() << " msg=\"" << e.what() << "\")";
    RH_ERROR(_baseLog, errstr.str());
    CF::StringSequence message;
    message.length(1);
    message[0] = CORBA::string_dup(errstr.str().c_str());
    throw CF::LifeCycle::ReleaseError(message);
  }
}

void Application_impl::releaseComponents()
{
    for (ComponentList::iterator ii = _components.begin(); ii != _components.end(); ++ii) {
        if (ii->getChildren().empty()) {
            // Release "real" components first
            ii->releaseObject();
        }
    }

    for (ComponentList::iterator ii = _components.begin(); ii != _components.end(); ++ii) {
        if (!ii->getChildren().empty()) {
            // Release containers once all "real" components have been released
            ii->releaseObject();
        }
    }
}

void Application_impl::terminateComponents()
{
    // Terminate any components that were executed on devices
    for (ComponentList::iterator ii = _components.begin(); ii != _components.end(); ++ii) {
        ii->terminate();
    }
}

void Application_impl::unloadComponents()
{
    // Terminate any components that were executed on devices
    for (ComponentList::iterator ii = _components.begin(); ii != _components.end(); ++ii) {
        ii->unloadFiles();
    }
}

void Application_impl::_cleanupActivations()
{
    // Use the existance of the application registry as a sentinel for whether
    // the servants have been deactivated
    if (!_registrar) {
        return;
    }
    PortableServer::POA_var dm_poa = ossie::corba::RootPOA()->find_POA("DomainManager", 0);
    PortableServer::POA_var app_poa = dm_poa->find_POA("Applications", 0);

    // Deactivate the application registry, release our reference and reset the
    // local pointer
    PortableServer::ObjectId_var oid = app_poa->servant_to_id(_registrar);
    app_poa->deactivate_object(oid);
    _registrar->_remove_ref();
    _registrar = 0;

    // If we created a fake application proxy, deactivate and clean it up too
    if (_fakeProxy) {
        oid = app_poa->servant_to_id(_fakeProxy);
        app_poa->deactivate_object(oid);
        _fakeProxy->_remove_ref();
        _fakeProxy = 0;
    }

    // Release this application
    oid = app_poa->servant_to_id(this);
    app_poa->deactivate_object(oid);
}

char* Application_impl::name ()
throw (CORBA::SystemException)
{
    return CORBA::string_dup(_appName.c_str());
}

bool Application_impl::aware ()
throw (CORBA::SystemException)
{
    return _isAware;
}


char* Application_impl::profile ()
throw (CORBA::SystemException)
{
    return CORBA::string_dup(_sadProfile.c_str());
}

char* Application_impl::softwareProfile ()
throw (CORBA::SystemException)
{
    return CORBA::string_dup(_sadProfile.c_str());
}

CF::Application::ComponentProcessIdSequence* Application_impl::componentProcessIds ()
throw (CORBA::SystemException)
{
    CF::Application::ComponentProcessIdSequence_var result = new CF::Application::ComponentProcessIdSequence();
    // Make sure releaseObject hasn't already been called
    {
        boost::mutex::scoped_lock lock(releaseObjectLock);

        if (_releaseAlreadyCalled) {
            RH_DEBUG(_baseLog, "skipping componentProcessIds call because releaseObject has been called");
            return result._retn();
        }
    }
    convert_sequence(result, _components, to_pid_type);
    return result._retn();
}

CF::Components* Application_impl::registeredComponents ()
{
    CF::Components_var result = new CF::Components();
    // Make sure releaseObject hasn't already been called
    {
        boost::mutex::scoped_lock lock(releaseObjectLock);

        if (_releaseAlreadyCalled) {
            RH_DEBUG(_baseLog, "skipping registeredComponents call because releaseObject has been called");
            return result._retn();
        }
    }
    boost::mutex::scoped_lock lock(_registrationMutex);
    convert_sequence_if(result, _components, to_component_type,
                        std::mem_fun_ref(&redhawk::ApplicationComponent::isRegistered));
    return result._retn();
}

bool Application_impl::haveAttribute(std::vector<std::string> &atts, std::string att)
{
    if (std::find(atts.begin(), atts.end(), att) == atts.end()) {
        return false;
    }
    return true;
}

CF::Properties* Application_impl::metrics(const CF::StringSequence& components, const CF::StringSequence& attributes)
throw (CF::Application::InvalidMetric, CORBA::SystemException)
{
    CF::Properties_var result_ugly = new CF::Properties();
    // Make sure releaseObject hasn't already been called
    {
        boost::mutex::scoped_lock lock(releaseObjectLock);

        if (_releaseAlreadyCalled) {
            RH_DEBUG(_baseLog, "skipping metrics call because releaseObject has been called");
            return result_ugly._retn();
        }
    }

    boost::mutex::scoped_lock lock(metricsLock);
    measuredDevices.clear();
    std::vector<std::string> valid_attributes;
    valid_attributes.push_back("valid");
    valid_attributes.push_back("shared");
    valid_attributes.push_back("cores");
    valid_attributes.push_back("memory");
    valid_attributes.push_back("processes");
    valid_attributes.push_back("threads");
    valid_attributes.push_back("files");
    valid_attributes.push_back("componenthost");
    std::vector<std::string> mod_attributes;
    mod_attributes.resize(attributes.length());
    for (unsigned int _att=0; _att<attributes.length(); _att++) {
        mod_attributes[_att] = attributes[_att];
        if (not haveAttribute(valid_attributes, mod_attributes[_att])) {
            CF::StringSequence _components;
            CF::StringSequence _attributes;
            ossie::corba::push_back(_attributes, mod_attributes[_att].c_str());
            throw CF::Application::InvalidMetric(_components, _attributes);
        }
    }
    if (mod_attributes.size() == 0) {
        mod_attributes = valid_attributes;
    }

    std::map<std::string, redhawk::ApplicationComponent*> component_map;
    std::vector<std::string> component_list;
    for (ComponentList::iterator _component_iter=this->_components.begin(); _component_iter!=this->_components.end(); _component_iter++) {
        if (_component_iter->isVisible()) {
            component_list.push_back(_component_iter->getName());
            component_map[_component_iter->getName()] = &(*_component_iter);
        }
    }
    ossie::DeviceList _registeredDevices = _domainManager->getRegisteredDevices();
    redhawk::PropertyMap& result = redhawk::PropertyMap::cast(result_ugly);
    redhawk::PropertyMap measuredComponents;
    std::vector<std::string> mod_requests;
    if (components.length() == 0) {
        mod_requests.insert(mod_requests.begin(), component_list.begin(), component_list.end());
        mod_requests.push_back("application utilization");
    } else {
        mod_requests.resize(components.length());
        for (unsigned int _req=0; _req<components.length(); _req++) {
            mod_requests[_req] = components[_req];
        }
    }
    for (unsigned int _req=0; _req<mod_requests.size(); _req++) {
        std::string _request(mod_requests[_req]);
        if (_request == "application utilization") {
            bool valid = true;
            for (std::vector<std::string>::iterator _comp=component_list.begin();_comp!=component_list.end();_comp++) {
                measuredComponents[*_comp] = measureComponent(*component_map[*_comp]);
                if (not measuredComponents[*_comp].asProperties()["valid"].toBoolean())
                    valid = false;
            }
            redhawk::PropertyMap _util;
            if (valid) {
                if (haveAttribute(mod_attributes, "cores"))
                    _util["cores"] = (float)0;
                if (haveAttribute(mod_attributes, "memory"))
                    _util["memory"] = (float)0;
                if (haveAttribute(mod_attributes, "processes"))
                    _util["processes"] = (unsigned long)0;
                if (haveAttribute(mod_attributes, "threads"))
                    _util["threads"] = (unsigned long)0;
                if (haveAttribute(mod_attributes, "files"))
                    _util["files"] = (unsigned long)0;
                if (haveAttribute(mod_attributes, "valid"))
                    _util["valid"] = true;
                std::vector<std::string> already_measured;
                for (std::vector<std::string>::iterator _comp=component_list.begin();_comp!=component_list.end();_comp++) {
                    redhawk::PropertyMap _mC = measuredComponents[*_comp].asProperties();
                    if (haveAttribute(already_measured, _mC["componenthost"].toString()))
                        continue;
                    already_measured.push_back(_mC["componenthost"].toString());
                    if (haveAttribute(mod_attributes, "cores"))
                        _util["cores"] = _util["cores"].toFloat() + _mC["cores"].toFloat();
                    if (haveAttribute(mod_attributes, "memory"))
                        _util["memory"] = _util["memory"].toFloat() + _mC["memory"].toFloat();
                    if (haveAttribute(mod_attributes, "processes"))
                        _util["processes"] = _util["processes"].toULong() + _mC["processes"].toULong();
                    if (haveAttribute(mod_attributes, "threads"))
                        _util["threads"] = _util["threads"].toULong() + _mC["threads"].toULong();
                    if (haveAttribute(mod_attributes, "files"))
                        _util["files"] = _util["files"].toULong() + _mC["files"].toULong();
                }
                result[_request] = _util;
            } else {
                redhawk::PropertyMap _util;
                if (haveAttribute(mod_attributes, "valid"))
                    _util["valid"] = false;
                result[_request] = _util;
            }
        }
    }
    // find out if all components need to be queried
    for (unsigned int _req=0; _req<mod_requests.size(); _req++) {
        std::string _request(mod_requests[_req]);
        if (_request != "application utilization") {
            if (measuredComponents.size() != 0) {
                result[_request] = filterAttributes(measuredComponents[_request].asProperties(), mod_attributes);
            } else {
                bool found_component = false;
                for (std::vector<std::string>::iterator _comp=component_list.begin();_comp!=component_list.end();_comp++) {
                    if (_request == *_comp) {
                        redhawk::PropertyMap tmp = measureComponent(*component_map[*_comp]);
                        result[_request] = filterAttributes(tmp, mod_attributes);
                        found_component = true;
                        break;
                    }
                }
                if (not found_component) {
                    CF::StringSequence _components;
                    CF::StringSequence _attributes;
                    ossie::corba::push_back(_components, _request.c_str());
                    throw CF::Application::InvalidMetric(_components, _attributes);
                }
            }
        }
    }
    return result_ugly._retn();
}

redhawk::PropertyMap Application_impl::filterAttributes(redhawk::PropertyMap &attributes, std::vector<std::string> &filter)
{
    redhawk::PropertyMap retval;
    if (haveAttribute(filter, "cores"))
        retval["cores"] = attributes["cores"];
    if (haveAttribute(filter, "memory"))
        retval["memory"] = attributes["memory"];
    if (haveAttribute(filter, "valid"))
        retval["valid"] = attributes["valid"];
    if (haveAttribute(filter, "shared"))
        retval["shared"] = attributes["shared"];
    if (haveAttribute(filter, "processes"))
        retval["processes"] = attributes["processes"];
    if (haveAttribute(filter, "threads"))
        retval["threads"] = attributes["threads"];
    if (haveAttribute(filter, "files"))
        retval["files"] = attributes["files"];
    if (haveAttribute(filter, "componenthost"))
        retval["componenthost"] = attributes["componenthost"];
    return retval;
}

redhawk::PropertyMap Application_impl::measureComponent(redhawk::ApplicationComponent &component)
{
    redhawk::PropertyMap retval;
    retval["valid"] = false;
    if (component.getComponentHost() != NULL) {
        retval["shared"] = true;
    } else {
        retval["shared"] = false;
    }
    ossie::DeviceList _registeredDevices = _domainManager->getRegisteredDevices();
    for (ossie::DeviceList::iterator _dev=_registeredDevices.begin(); _dev!=_registeredDevices.end(); _dev++) {
        if (component.getAssignedDevice()->identifier == (*_dev)->identifier) {
            retval["valid"] = false;
            redhawk::PropertyMap query;
            if (measuredDevices.find(component.getAssignedDevice()->identifier) == measuredDevices.end()) {
                query["component_monitor"] = redhawk::Value();
                try {
                    (*_dev)->device->query(query);
                } catch ( ... ) {
                    RH_WARN(_baseLog, "Unable to query 'component_monitor' on "<<component.getAssignedDevice()->identifier);
                    continue;
                }
                measuredDevices[component.getAssignedDevice()->identifier] = query;
            } else {
                query = measuredDevices[component.getAssignedDevice()->identifier];
            }
            const redhawk::ValueSequence& values = query["component_monitor"].asSequence();
            std::string target_id = component.getIdentifier();
            if (retval["shared"].toBoolean())
                target_id = component.getComponentHost()->getIdentifier();
            if (values.size()!=0) {
                for (unsigned int i=0; i<values.size(); i++) {
                    const redhawk::PropertyMap& single(values[i].asProperties());
                    if (not single.contains("component_monitor::component_monitor::waveform_id")) {
                        RH_WARN(_baseLog, "Unable to query 'component_monitor' missing 'waveform_id' on "<<component.getAssignedDevice()->identifier);
                        continue;
                    }
                    if (single["component_monitor::component_monitor::waveform_id"].toString() != _identifier) {
                        continue;
                    }
                    if (not single.contains("component_monitor::component_monitor::component_id")) {
                        RH_WARN(_baseLog, "Unable to query 'component_monitor' missing 'component_id' on "<<component.getAssignedDevice()->identifier);
                        continue;
                    }
                    if (single["component_monitor::component_monitor::component_id"].toString() != target_id) {
                        continue;
                    }
                    if ((not single.contains("component_monitor::component_monitor::cores")) or
                        (not single.contains("component_monitor::component_monitor::mem_rss")) or
                        (not single.contains("component_monitor::component_monitor::num_processes")) or
                        (not single.contains("component_monitor::component_monitor::num_threads")) or
                        (not single.contains("component_monitor::component_monitor::num_files"))) {
                        RH_WARN(_baseLog, "Unable to query 'component_monitor' missing 'cores', 'mem_rss', 'num_processes', 'num_threads', or 'num_files' on "<<component.getAssignedDevice()->identifier);
                        continue;
                    }
                    retval["cores"] = single["component_monitor::component_monitor::cores"].toFloat();
                    retval["memory"] = single["component_monitor::component_monitor::mem_rss"].toFloat();
                    retval["processes"] = single["component_monitor::component_monitor::num_processes"].toULong();
                    retval["threads"] = single["component_monitor::component_monitor::num_threads"].toULong();
                    retval["files"] = single["component_monitor::component_monitor::num_files"].toULong();
                    retval["componenthost"] = target_id;
                    retval["valid"] = true;
                    break;
                }
            }
        }
    }
    return retval;
}

CF::ApplicationRegistrar_ptr Application_impl::appReg (void)
{
    return _registrar->_this();
}

CF::Application::ComponentElementSequence* Application_impl::componentNamingContexts ()
throw (CORBA::SystemException)
{
    CF::Application::ComponentElementSequence_var result = new CF::Application::ComponentElementSequence();
    // Make sure releaseObject hasn't already been called
    {
        boost::mutex::scoped_lock lock(releaseObjectLock);

        if (_releaseAlreadyCalled) {
            RH_DEBUG(_baseLog, "skipping componentNamingContexts call because releaseObject has been called");
            return result._retn();
        }
    }

    convert_sequence_if(result, _components, to_name_element,
                        std::mem_fun_ref(&redhawk::ApplicationComponent::hasNamingContext));
    return result._retn();
}


CF::Application::ComponentElementSequence* Application_impl::componentImplementations ()
throw (CORBA::SystemException)
{
    CF::Application::ComponentElementSequence_var result = new CF::Application::ComponentElementSequence();
    // Make sure releaseObject hasn't already been called
    {
        boost::mutex::scoped_lock lock(releaseObjectLock);

        if (_releaseAlreadyCalled) {
            RH_DEBUG(_baseLog, "skipping componentImplementations call because releaseObject has been called");
            return result._retn();
        }
    }
    convert_sequence(result, _components, to_impl_element);
    return result._retn();
}


CF::DeviceAssignmentSequence* Application_impl::componentDevices ()
throw (CORBA::SystemException)
{
    CF::DeviceAssignmentSequence_var result = new CF::DeviceAssignmentSequence();
    // Make sure releaseObject hasn't already been called
    {
        boost::mutex::scoped_lock lock(releaseObjectLock);

        if (_releaseAlreadyCalled) {
            RH_DEBUG(_baseLog, "skipping componentDevices call because releaseObject has been called");
            return result._retn();
        }
    }

    return new CF::DeviceAssignmentSequence(_componentDevices);
}

const std::string& Application_impl::getIdentifier() const
{
    return _identifier;
}

const std::string& Application_impl::getName() const
{
    return _appName;
}

const std::string& Application_impl::getProfile() const
{
    return _sadProfile;
}

void Application_impl::addExternalPort (const std::string& identifier, CORBA::Object_ptr port)
{
    if (_ports.count(identifier)) {
        throw std::runtime_error("Port name " + identifier + " is already in use");
    }

    _ports[identifier] = CORBA::Object::_duplicate(port);
}

void Application_impl::addExternalProperty (const std::string& propId, const std::string& externalId, const std::string& access, CF::Resource_ptr comp)
{
    if (_properties.count(externalId)) {
        throw std::runtime_error("External Property name " + externalId + " is already in use");
    }

    externalPropertyType external;
    external.property_id = propId;
    external.access = access;
    external.component = CF::Resource::_duplicate(comp);
    _properties.insert(std::pair<std::string, externalPropertyType>(externalId, external));
}

bool Application_impl::checkConnectionDependency (Endpoint::DependencyType type, const std::string& identifier) const
{
    for (std::vector<ConnectionNode>::const_iterator connection = _connections.begin(); connection != _connections.end(); ++connection) {
        if (connection->checkDependency(type, identifier)) {
            return true;
        }
    }
    
    return false;
}

bool Application_impl::_checkRegistrations (std::set<std::string>& identifiers)
{
    for (ComponentList::iterator ii = _components.begin(); ii != _components.end(); ++ii) {
        if (ii->isRegistered()) {
            identifiers.erase(ii->getIdentifier());
        } else if (ii->isTerminated()) {
            throw redhawk::ComponentTerminated(ii->getIdentifier());
        }
    }
    return identifiers.empty();
}

bool Application_impl::waitForComponents (std::set<std::string>& identifiers, int timeout)
{
    // Determine the current time, then add the timeout value to calculate when we should
    // stop retrying as an absolute time.
    boost::system_time end = boost::get_system_time() + boost::posix_time::seconds(timeout);

    boost::mutex::scoped_lock lock(_registrationMutex);
    while (!_checkRegistrations(identifiers)) {
      RH_DEBUG(_baseLog, "Waiting for components....APP:" << _identifier << "  list " << identifiers.size() );
        if (!_registrationCondition.timed_wait(lock, end)) {
            break;
        }
    }
    return identifiers.empty();
}

CF::Application_ptr Application_impl::getComponentApplication ()
{
    SCOPED_LOCK( releaseObjectLock );
    if (_isAware) {
        return _this();
    } else {
        return _fakeProxy->_this();
    }
}

CF::DomainManager_ptr Application_impl::getComponentDomainManager ()
{
    SCOPED_LOCK( releaseObjectLock );
    CF::DomainManager_var ret = CF::DomainManager::_nil();
    if (_isAware) {
        ret = _domainManager->_this();
    }

    return CF::DomainManager::_duplicate(ret);
}

redhawk::ApplicationComponent* Application_impl::getComponent(const std::string& identifier)
{
    redhawk::ApplicationComponent* component = findComponent(identifier);
    if (!component) {
        throw std::logic_error("unknown component '" + identifier + "'");
    }
    return component;
}

void Application_impl::registerComponent (CF::Resource_ptr resource)
{
    const std::string componentId = ossie::corba::returnString(resource->identifier());
    const std::string softwareProfile = ossie::corba::returnString(resource->softwareProfile());


    boost::mutex::scoped_lock lock(_registrationMutex);
    redhawk::ApplicationComponent* comp = findComponent(componentId);

    if (!comp) {
        RH_WARN(_baseLog, "Unexpected component '" << componentId
                 << "' registered with application '" << _appName << "'");
        comp = addComponent(componentId, softwareProfile);
    } else if (softwareProfile != comp->getSoftwareProfile()) {
        // Mismatch between expected and reported SPD path
        RH_WARN(_baseLog, "Component '" << componentId << "' software profile " << softwareProfile
                 << " does not match expected profile " << comp->getSoftwareProfile());
        comp->setSoftwareProfile(softwareProfile);
    }

    RH_TRACE(_baseLog, "REGISTERING Component '" << componentId << "' software profile " << softwareProfile << " pid:" << comp->getProcessId());
    comp->setComponentObject(resource);
    _registrationCondition.notify_all();
}

std::string Application_impl::getExternalPropertyId(std::string compIdIn, std::string propIdIn)
{
    for (std::map<std::string, externalPropertyType>::const_iterator prop = _properties.begin();
            prop != _properties.end(); ++prop) {
        // Gets the property mapping info
        std::string extId = prop->first;
        std::string propId = prop->second.property_id;
        // Gets the Resource identifier
        std::string compId = ossie::corba::returnString(prop->second.component->identifier());

        if (compId == compIdIn && propId == propIdIn) {
            return extId;
        }
    }

    return "";
}

redhawk::ApplicationComponent* Application_impl::findComponent(const std::string& identifier)
{
    for (ComponentList::iterator ii = _components.begin(); ii != _components.end(); ++ii) {
        if (identifier == ii->getIdentifier()) {
            return &(*ii);
        }
    }

    return 0;
}

redhawk::ApplicationComponent* Application_impl::addComponent(const std::string& componentId,
                                                              const std::string& softwareProfile)
{
    _components.push_back(redhawk::ApplicationComponent(componentId));
    redhawk::ApplicationComponent* component = &(_components.back());
    component->setSoftwareProfile(softwareProfile);
    component->setLogger(_baseLog);
    return component;
}

redhawk::ApplicationComponent* Application_impl::addContainer(const redhawk::ContainerDeployment* container)
{
    const std::string& identifier = container->getIdentifier();
    if (findComponent(identifier)) {
        throw std::logic_error("container '" + identifier + "' is already registered");
    }
    const std::string& profile = container->getSoftPkg()->getSPDFile();
    RH_DEBUG(_baseLog, "Adding container '" << identifier << "' with profile " << profile);
    redhawk::ApplicationComponent* component = addComponent(identifier, profile);
    component->setName(container->getInstantiation()->getID());
    component->setImplementationId(container->getImplementation()->getID());
    // Hide ComponentHost instances from the CORBA API
    component->setVisible(false);
    component->setAssignedDevice(container->getAssignedDevice());
    return component;
}

redhawk::ApplicationComponent* Application_impl::addComponent(const redhawk::ComponentDeployment* deployment)
{
    const std::string& identifier = deployment->getIdentifier();
    if (findComponent(identifier)) {
        throw std::logic_error("component '" + identifier + "' is already registered");
    }
    const std::string& profile = deployment->getSoftPkg()->getSPDFile();
    RH_DEBUG(_baseLog, "Adding component '" << identifier << "' with profile " << profile);
    redhawk::ApplicationComponent* component = addComponent(identifier, profile);
    component->setName(deployment->getInstantiation()->getID());
    component->setImplementationId(deployment->getImplementation()->getID());
    component->setAssignedDevice(deployment->getAssignedDevice());
    return component;
}

void Application_impl::componentTerminated(const std::string& componentId, const std::string& deviceId)
{
    boost::mutex::scoped_lock lock(_registrationMutex);
    redhawk::ApplicationComponent* component = findComponent(componentId);
    if (!component) {
        RH_WARN(_baseLog, "Unrecognized component '" << componentId << "' from application '" << _identifier
                 << "' terminated abnormally on device " << deviceId);
        return;
    }
    if (!component->getChildren().empty()) {
        RH_ERROR(_baseLog, "Component host from application '" << _appName
                  << "' containing " << component->getChildren().size()
                  << " component(s) terminated abnormally on device " << deviceId);
        BOOST_FOREACH(redhawk::ApplicationComponent* child, component->getChildren()) {
            _checkComponentConnections(child);
        }
    } else {
        RH_ERROR(_baseLog, "Component '" << component->getName()
                  << "' from application '" << _appName
                  << "' terminated abnormally on device " << deviceId);
        _checkComponentConnections(component);
    }
    component->setProcessId(0);
    _registrationCondition.notify_all();
}

void Application_impl::_checkComponentConnections(redhawk::ApplicationComponent* component)
{
    RH_DEBUG(_baseLog, "Checking for connections that depend on terminated component "
              << component->getIdentifier());
    const std::string& name = component->getName();
    int connection_count = 0;
    BOOST_FOREACH(ConnectionNode& connection, _connections) {
        if (connection.dependencyTerminated(ossie::Endpoint::COMPONENT, name)) {
            RH_TRACE(_baseLog, "Application '" << _appName << "' connection '"
                      << connection.identifier << "' depends on terminated component '"
                      << name << "'");
            connection_count++;
        }
    }
    if (connection_count > 0) {
        RH_DEBUG(_baseLog, "Application '" << _appName << "' has " << connection_count
                 << " connection(s) depending on terminated component '" << name << "'");
    }
}
