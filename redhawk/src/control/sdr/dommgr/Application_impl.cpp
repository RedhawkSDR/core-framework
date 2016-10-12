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

#include <ossie/debug.h>
#include <ossie/CorbaUtils.h>
#include <ossie/EventChannelSupport.h>

#include "Application_impl.h"
#include "DomainManager_impl.h"
#include "AllocationManager_impl.h"
#include "ApplicationRegistrar.h"
#include "connectionSupport.h"
#include "FakeApplication.h"

PREPARE_LOGGING(Application_impl);

using namespace ossie;

namespace {
    CF::Application::ComponentElementType to_impl_element(const ossie::ApplicationComponent& component)
    {
        CF::Application::ComponentElementType result;
        result.componentId = component.identifier.c_str();
        result.elementId = component.implementationId.c_str();
        return result;
    }

    bool has_naming_context(const ossie::ApplicationComponent& component)
    {
        return !component.namingContext.empty();
    }

    CF::Application::ComponentElementType to_name_element(const ossie::ApplicationComponent& component)
    {
        CF::Application::ComponentElementType result;
        result.componentId = component.identifier.c_str();
        result.elementId = component.namingContext.c_str();
        return result;
    }

    CF::Application::ComponentProcessIdType to_pid_type(const ossie::ApplicationComponent& component)
    {
        CF::Application::ComponentProcessIdType result;
        result.componentId = component.identifier.c_str();
        result.processId = component.processId;
        return result;
    }

    bool is_registered(const ossie::ApplicationComponent& component)
    {
        return !CORBA::is_nil(component.componentObject);
    }

    CF::ComponentType to_component_type(const ossie::ApplicationComponent& component)
    {
        CF::ComponentType result;
        result.identifier = component.identifier.c_str();
        result.softwareProfile = component.softwareProfile.c_str();
        result.type = CF::APPLICATION_COMPONENT;
        result.componentObject = CORBA::Object::_duplicate(component.componentObject);
        return result;
    }

    template <class Sequence, class Iterator, class Function>
    void convert_sequence(Sequence& out, Iterator begin, const Iterator end, Function func)
    {
        for (; begin != end; ++begin) {
            ossie::corba::push_back(out, func(*begin));
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
            if (pred(*begin)) {
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
                                    CosNaming::NamingContext_ptr waveformContext, bool aware, CosNaming::NamingContext_ptr DomainContext) :
    _identifier(id),
    _sadProfile(profile),
    _appName(name),
    _domainManager(domainManager),
    _waveformContextName(waveformContextName),
    _waveformContext(CosNaming::NamingContext::_duplicate(waveformContext)),
    _started(false),
    _isAware(aware),
    _fakeProxy(0),
    _domainContext(CosNaming::NamingContext::_duplicate(DomainContext)),
    _releaseAlreadyCalled(false)
{
    _registrar = new ApplicationRegistrar_impl(waveformContext, this);
    if (!_isAware) {
        _fakeProxy = new FakeApplication(this);
    }
};

void Application_impl::populateApplication(CF::Resource_ptr _controller,
                                           std::vector<ossie::DeviceAssignmentInfo>&  _devSeq,
                                           std::vector<CF::Resource_var> _startSeq,
                                           std::vector<ConnectionNode>& connections,
                                           std::vector<std::string> allocationIDs)
{
    TRACE_ENTER(Application_impl)
    _connections = connections;
    _componentDevices = _devSeq;
    _appStartSeq = _startSeq;

    LOG_DEBUG(Application_impl, "Creating allocation sequence");
    this->_allocationIDs = allocationIDs;

    LOG_DEBUG(Application_impl, "Assigning the assembly controller")
    // Assume _controller is NIL implies that the assembly controller component is Non SCA-Compliant
    if (CORBA::is_nil(_controller)) {
        LOG_INFO(Application_impl, "Assembly controller is non SCA-compliant");
    } else {
        assemblyController = CF::Resource::_duplicate(_controller);
    }
    TRACE_EXIT(Application_impl)
}

Application_impl::~Application_impl ()
{
    TRACE_ENTER(Application_impl)
    TRACE_EXIT(Application_impl)
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

void Application_impl::start ()
throw (CORBA::SystemException, CF::Resource::StartError)
{
    if (CORBA::is_nil(assemblyController) and (_appStartSeq.size() == 0)) {
        throw(CF::Resource::StartError(CF::CF_ENOTSUP, "No assembly controller and no Components with startorder set"));
        return;
    }

    try {
        omniORB::setClientCallTimeout(assemblyController, 0);       
        LOG_TRACE(Application_impl, "Calling start on assembly controller")
        assemblyController->start ();

        // Start the rest of the components
        for (unsigned int i = 0; i < _appStartSeq.size(); i++){
            std::string msg = "Calling start for ";
            msg = msg.append(ossie::corba::returnString(_appStartSeq[i]->identifier()));
            LOG_TRACE(Application_impl, msg)

            omniORB::setClientCallTimeout(_appStartSeq[i], 0);       
            _appStartSeq[i]-> start();
        }
    } catch( CF::Resource::StartError& se ) {
        LOG_ERROR(Application_impl, "Start failed with CF:Resource::StartError")
        throw;
    } CATCH_THROW_LOG_ERROR(Application_impl, "Start failed", CF::Resource::StartError())
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


bool Application_impl::stopComponent (CF::Resource_ptr component)
{
    std::string identifier;
    try {
        identifier = ossie::corba::returnString(component->identifier());
    } catch (const CORBA::SystemException& ex) {
        LOG_ERROR(Application_impl, "CORBA::" << ex._name() << " getting component identifier");
        return false;
    } catch (...) {
        LOG_ERROR(Application_impl, "Unknown exception getting component identifier");
        return false;
    }
    LOG_TRACE(Application_impl, "Calling stop for " << identifier);
    const unsigned long timeout = 3; // seconds
    omniORB::setClientCallTimeout(component, timeout * 1000);
    try {
        component->stop();
        return true;
    } catch (const CF::Resource::StopError& error) {
        LOG_ERROR(Application_impl, "Failed to stop " << identifier << "; CF::Resource::StopError '" << error.msg << "'");
    } CATCH_LOG_ERROR(Application_impl, "Failed to stop " << identifier);
    return false;
}

void Application_impl::stop ()
throw (CORBA::SystemException, CF::Resource::StopError)
{
    if (CORBA::is_nil(assemblyController) and (_appStartSeq.size() == 0)) {
        throw(CF::Resource::StopError(CF::CF_ENOTSUP, "No assembly controller and no Components with startorder set"));
        return;
    }

    int failures = 0;
    // Stop the components in the reverse order they were started
    for (int i = (int)(_appStartSeq.size()-1); i >= 0; i--){
        if (!stopComponent(_appStartSeq[i])) {
            failures++;
        }
    }

    LOG_TRACE(Application_impl, "Calling stop on assembly controller");
    if (!stopComponent(assemblyController)) {
        failures++;
    }
    if (failures > 0) {
        std::ostringstream oss;
        oss << failures << " component(s) failed to stop";
        const std::string message = oss.str();
        LOG_ERROR(Application_impl, "Stopping " << _identifier << "; " << message);
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


void Application_impl::initializeProperties (const CF::Properties& configProperties)
throw (CF::PropertySet::PartialConfiguration, CF::PropertySet::InvalidConfiguration, CORBA::SystemException)
{
}

void Application_impl::configure (const CF::Properties& configProperties)
throw (CF::PropertySet::PartialConfiguration, CF::PropertySet::InvalidConfiguration, CORBA::SystemException)
{
    int validProperties = 0;
    CF::Properties invalidProperties;

    // Creates a map from componentIdentifier -> (rsc_ptr, ConfigPropSet)
    // to allow for one batched configure call per component

    CF::Properties acProps;
    const std::string acId = ossie::corba::returnString(assemblyController->identifier());
    std::map<std::string, std::pair<CF::Resource_ptr, CF::Properties> > batch;
    batch[acId] = std::pair<CF::Resource_ptr, CF::Properties>(assemblyController, acProps);

    // Loop through each passed external property, mapping it with its respective resource
    for (unsigned int i = 0; i < configProperties.length(); ++i) {
        // Gets external ID for property mapping
        const std::string extId(configProperties[i].id);

        if (_properties.count(extId)) {
            // Gets the component and its internal property id
            const std::string propId = _properties[extId].first;
            CF::Resource_ptr comp = _properties[extId].second;

            if (CORBA::is_nil(comp)) {
                LOG_ERROR(Application_impl, "Unable to retrieve component for external property: " << extId);
                int count = invalidProperties.length();
                invalidProperties.length(count + 1);
                invalidProperties[count].id = CORBA::string_dup(extId.c_str());
                invalidProperties[count].value = configProperties[i].value;
            } else {
                // Key used for map
                const std::string compId = ossie::corba::returnString(comp->identifier());

                LOG_TRACE(Application_impl, "Configure external property: " << extId << " on "
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
        } else if (!CORBA::is_nil(assemblyController)) {
            // Properties that are not external get batched with assembly controller
            LOG_TRACE(Application_impl, "Calling configure on assembly controller for property: " << configProperties[i].id);
            int count = batch[acId].second.length();
            batch[acId].second.length(count + 1);
            batch[acId].second[count].id = configProperties[i].id;
            batch[acId].second[count].value = configProperties[i].value;
        } else {
            LOG_ERROR(Application_impl, "Unable to retrieve assembly controller for external property: " << extId);
            int count = invalidProperties.length();
            invalidProperties.length(count + 1);
            invalidProperties[count].id = CORBA::string_dup(extId.c_str());
            invalidProperties[count].value = configProperties[i].value;
        }
    }

    // -Loop through each component with a property that needs to be configured and make configure call
    // -Catch any errors
    for (std::map<std::string, std::pair<CF::Resource_ptr, CF::Properties> >::const_iterator comp = batch.begin();
            comp != batch.end(); ++comp) {
        int propLength = comp->second.second.length();
        try {
            comp->second.first->configure(comp->second.second);
            validProperties += propLength;
        } catch (CF::PropertySet::InvalidConfiguration e) {
            // Add invalid properties to return list
            for (unsigned int i = 0; i < e.invalidProperties.length(); ++i) {
                int count = invalidProperties.length();
                invalidProperties.length(count + 1);
                invalidProperties[count].id = CORBA::string_dup(e.invalidProperties[i].id);
                invalidProperties[count].value = e.invalidProperties[i].value;
            }
        } catch (CF::PropertySet::PartialConfiguration e) {
            // Add invalid properties to return list
            for (unsigned int i = 0; i < e.invalidProperties.length(); ++i) {
                int count = invalidProperties.length();
                invalidProperties.length(count + 1);
                invalidProperties[count].id = CORBA::string_dup(e.invalidProperties[i].id);
                invalidProperties[count].value = e.invalidProperties[i].value;
            }
            validProperties += propLength - e.invalidProperties.length();
        }
    }

    // Throw appropriate exception if any configure errors were handled
    if (invalidProperties.length () > 0) {
        if (validProperties > 0) {
            throw CF::PropertySet::PartialConfiguration(invalidProperties);
        } else {
            throw CF::PropertySet::InvalidConfiguration("No matching external properties found", invalidProperties);
        }
    }
}


void Application_impl::query (CF::Properties& configProperties)
throw (CF::UnknownProperties, CORBA::SystemException)
{
    CF::Properties invalidProperties;

    // Creates a map from componentIdentifier -> (rsc_ptr, ConfigPropSet)
    // to allow for one batched query call per component
    const std::string acId = ossie::corba::returnString(assemblyController->identifier());
    std::map<std::string, std::pair<CF::Resource_ptr, CF::Properties> > batch;

    // For queries of zero length, return all external properties
    if (configProperties.length() == 0) {
        LOG_TRACE(Application_impl, "Query all external and assembly controller properties");

        configProperties.length(0);

        // Loop through each external property and add it to the batch with its respective component
        for (std::map<std::string, std::pair<std::string, CF::Resource_var> >::const_iterator prop = _properties.begin();
                prop != _properties.end(); ++prop) {
            // Gets the property mapping info
            std::string extId = prop->first;
            std::string propId = prop->second.first;
            CF::Resource_ptr comp = prop->second.second;

            if (CORBA::is_nil(comp)) {
                LOG_ERROR(Application_impl, "Unable to retrieve component for external property: " << extId);
                int count = invalidProperties.length();
                invalidProperties.length(count + 1);
                invalidProperties[count].id = CORBA::string_dup(extId.c_str());
                invalidProperties[count].value = CORBA::Any();
            } else {
                // Key used for map
                const std::string compId = ossie::corba::returnString(comp->identifier());

                LOG_TRACE(Application_impl, "Query external property: " << extId << " on "
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
                for (unsigned int i = 0; i < e.invalidProperties.length(); ++i) {
                    // Add invalid properties to return list
                    int count = invalidProperties.length();
                    invalidProperties.length(count + 1);
                    invalidProperties[count].id = CORBA::string_dup(e.invalidProperties[i].id);
                    invalidProperties[count].value = e.invalidProperties[i].value;
                }
            }
        }

        // Query Assembly Controller properties
        CF::Properties tempProp;
        try {
            assemblyController->query(tempProp);
        } catch (CF::UnknownProperties e) {
            int count = invalidProperties.length();
            invalidProperties.length(count + e.invalidProperties.length());
            for (unsigned int i = 0; i < e.invalidProperties.length(); ++i) {
                LOG_ERROR(Application_impl, "Invalid assembly controller property name: " << e.invalidProperties[i].id);
                invalidProperties[count + i] = e.invalidProperties[i];
            }
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
        batch[acId] = std::pair<CF::Resource_ptr, CF::Properties>(assemblyController, acProps);

        for (unsigned int i = 0; i < configProperties.length(); ++i) {
            // Gets external ID for property mapping
            const std::string extId(configProperties[i].id);

            if (_properties.count(extId)) {
                // Gets the component and its property id
                const std::string propId = _properties[extId].first;
                CF::Resource_ptr comp = _properties[extId].second;

                if (CORBA::is_nil(comp)) {
                    LOG_ERROR(Application_impl, "Unable to retrieve component for external property: " << extId);
                    int count = invalidProperties.length();
                    invalidProperties.length(count + 1);
                    invalidProperties[count].id = CORBA::string_dup(extId.c_str());
                    invalidProperties[count].value = configProperties[i].value;
                } else {
                    // Key used for map
                    std::string compId = ossie::corba::returnString(comp->identifier());

                    LOG_TRACE(Application_impl, "Query external property: " << extId << " on "
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
            } else if (!CORBA::is_nil(assemblyController)) {
                // Properties that are not external get batched with assembly controller
                LOG_TRACE(Application_impl, "Calling query on assembly controller for property: "
                        << configProperties[i].id);
                int count = batch[acId].second.length();
                batch[acId].second.length(count + 1);
                batch[acId].second[count].id = configProperties[i].id;
                batch[acId].second[count].value = configProperties[i].value;
            } else {
                LOG_ERROR(Application_impl, "Unable to retrieve assembly controller for external property: " << extId);
                int count = invalidProperties.length();
                invalidProperties.length(count + 1);
                invalidProperties[count].id = CORBA::string_dup(extId.c_str());
                invalidProperties[count].value = configProperties[i].value;
            }
        }

        // -Loop through each component with a property that needs to be queried and make query() call
        // -Catch any errors
        for (std::map<std::string, std::pair<CF::Resource_ptr, CF::Properties> >::iterator comp = batch.begin();
                comp != batch.end(); ++comp) {
            try {
                comp->second.first->query(comp->second.second);
            } catch (CF::UnknownProperties e) {
                for (unsigned int i = 0; i < e.invalidProperties.length(); ++i) {
                    // Add invalid properties to return list
                    int count = invalidProperties.length();
                    invalidProperties.length(count + 1);
                    invalidProperties[count].id = CORBA::string_dup(e.invalidProperties[i].id);
                    invalidProperties[count].value = e.invalidProperties[i].value;
                }
            }
        }

        // Loops through requested property IDs to find value that was returned from internal call
        for (unsigned int i = 0; i < configProperties.length(); ++i) {
            const std::string extId(configProperties[i].id);
            std::string propId;
            std::string compId;

            // Checks if property ID is external or AC property
            if (_properties.count(extId)) {
                propId = _properties[extId].first;
                compId = ossie::corba::returnString(_properties[extId].second->identifier());
            } else {
                propId = extId;
                compId = ossie::corba::returnString(assemblyController->identifier());
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

    if (invalidProperties.length () != 0) {
        throw CF::UnknownProperties(invalidProperties);
    }

    LOG_TRACE(Application_impl, "Query returning " << configProperties.length() <<
            " external and assembly controller properties");
}


char *Application_impl::registerPropertyListener( CORBA::Object_ptr listener, const CF::StringSequence &prop_ids, const CORBA::Float interval)
  throw(CF::UnknownProperties, CF::InvalidObjectReference)
{

  SCOPED_LOCK( releaseObjectLock );

  LOG_TRACE(Application_impl, "Number of Properties to Register: " << prop_ids.length() );
  typedef   std::map< CF::Resource_ptr, std::vector< std::string > > CompRegs;
  CompRegs comp_regs;

  for (unsigned int i = 0; i < prop_ids.length(); ++i) {
        // Gets external ID for property mapping
        const std::string extId(prop_ids[i]);

        if (_properties.count(extId)) {
          CF::Resource_ptr comp = _properties[extId].second;          
          std::string prop_id = _properties[extId].first;          
          LOG_TRACE(Application_impl, "  ---> Register ExternalID: " << extId << " Comp/Id " << 
                ossie::corba::returnString(comp->identifier()) << "/" << prop_id);
          comp_regs[ comp ].push_back( prop_id ); 
          
        } else if (!CORBA::is_nil(assemblyController)) {
          comp_regs[ assemblyController ].push_back( extId ) ;
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
      LOG_TRACE(Application_impl, "Component-->PropertyChangeRegistryRegistry  comp/id " << 
                ossie::corba::returnString(reg_iter->first->identifier()) << "/" << reg_id );
      pc_recs.push_back( PropertyChangeRecord( reg_id, reg_iter->first ) );

    }

  }
  catch (...) {
    
    LOG_WARN(Application_impl, "PropertyChangeListener registration failed against Application: " << _identifier );
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
        LOG_WARN(Application_impl, "Unregister PropertyChangeListener operation failed. app/reg_id: " << _identifier << "/" << reg_id );
      }
    }
    
    _propertyChangeRegistrations.erase( reg_id );
    
  }

}

void Application_impl::initialize ()
throw (CORBA::SystemException, CF::LifeCycle::InitializeError)
{
    if (CORBA::is_nil(assemblyController)) { return; }

    try {
        LOG_TRACE(Application_impl, "Calling initialize on assembly controller")
        assemblyController->initialize ();
    } catch( CF::LifeCycle::InitializeError& ie ) {
        LOG_ERROR(Application_impl, "Initialize failed with CF::LifeCycle::InitializeError")
        throw;
    } CATCH_THROW_LOG_ERROR(Application_impl, "Initialize failed", CF::LifeCycle::InitializeError())
}


CORBA::Object_ptr Application_impl::getPort (const char* _id)
throw (CORBA::SystemException, CF::PortSupplier::UnknownPort)
{
    const std::string identifier = _id;
    if (_ports.count(identifier)) {
        return CORBA::Object::_duplicate(_ports[identifier]);
    } else {
        LOG_ERROR(Application_impl, "Get port failed with unknown port " << _id)
        throw(CF::PortSupplier::UnknownPort());
    }
}

CF::PortSet::PortInfoSequence* Application_impl::getPortSet ()
{
    CF::PortSet::PortInfoSequence_var retval = new CF::PortSet::PortInfoSequence();
    std::vector<CF::PortSet::PortInfoSequence_var> comp_portsets;
    for (ossie::ComponentList::iterator _component_iter=this->_components.begin(); _component_iter!=this->_components.end(); _component_iter++) {
        try {
            CF::Resource_ptr comp = CF::Resource::_narrow(_component_iter->componentObject);
            comp_portsets.push_back(comp->getPortSet());
        } catch ( ... ) {
            // failed to get the port set from the component
        }
    }
    for (std::map<std::string, CORBA::Object_var>::iterator _port_val=_ports.begin(); _port_val!=_ports.end(); _port_val++) {
        for (std::vector<CF::PortSet::PortInfoSequence_var>::iterator comp_portset=comp_portsets.begin(); comp_portset!=comp_portsets.end(); comp_portset++) {
            for (unsigned int i=0; i<(*comp_portset)->length(); i++) {
                try {
                    if (_port_val->second->_is_equivalent((*comp_portset)[i].obj_ptr)) {
                        CF::PortSet::PortInfoType info;
                        info.obj_ptr = (*comp_portset)[i].obj_ptr;
                        info.name = (*comp_portset)[i].name;
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
    if (CORBA::is_nil(assemblyController)) {
        LOG_ERROR(Application_impl, "Run test called with non SCA compliant assembly controller");
        throw CF::TestableObject::UnknownTest();
    }

    try {
        LOG_TRACE(Application_impl, "Calling runTest on assembly controller")
        assemblyController->runTest (_testId, _props);
    } catch( CF::UnknownProperties& up ) {
        // It would be helpful to list all properties in 'up' as part of the error message
        LOG_ERROR(Application_impl, "Run test failed with CF::UnknownProperties for Test ID " << _testId)
        throw;
    } catch( CF::TestableObject::UnknownTest& ) {
        LOG_ERROR(Application_impl, "Run test failed with CF::TestableObject::UnknownTest for Test ID " << _testId)
        throw;
    } CATCH_RETHROW_LOG_ERROR(Application_impl, "Run test failed for Test ID " << _testId)
}


void Application_impl::releaseObject ()
throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
  TRACE_ENTER(Application_impl);
      
  try {
    // Make sure releaseObject hasn't already been called, but only hold the
    // lock long enough to check to prevent a potential priority inversion with
    // the domain's stateAccess mutex
    {
        boost::mutex::scoped_lock lock(releaseObjectLock);

        if (_releaseAlreadyCalled) {
            LOG_DEBUG(Application_impl, "skipping release because release has already been called");
            return;
        } else {
            _releaseAlreadyCalled = true;
        }
    }
    
    LOG_DEBUG(Application_impl, "Releasing application");

    // remove application from DomainManager's App Sequence
    try {
        _domainManager->removeApplication(_identifier);
    } catch (CF::DomainManager::ApplicationUninstallationError& ex) {
        LOG_ERROR(Application_impl, ex.msg);
    }

    // Stop all components on the application
    try {
        this->stop();
    } catch ( ... ) {
        // error happened while stopping. Ignore the error and continue tear-down
        LOG_TRACE(Application_impl, "Error occurred while stopping the application during tear-down. Ignoring the error and continuing")
    }
    
    assemblyController = CF::Resource::_nil ();
    
    try {
      // Break all connections in the application
      ConnectionManager::disconnectAll(_connections, _domainManager);
      LOG_DEBUG(Application_impl, "app->releaseObject finished disconnecting ports");
    } CATCH_LOG_ERROR(Application_impl, "Failure during disconnect operation");

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
    for (ossie::ComponentList::iterator ii = _components.begin(); ii != _components.end(); ++ii) {

        const std::string id = ii->identifier;

        if (!ii->namingContext.empty()) {
            std::string componentName = ii->namingContext;

            // Unbind the component from the naming context. This assumes that the component is
            // bound into the waveform context, and its name inside of the context follows the
            // last slash in the fully-qualified name.
            std::string shortName = componentName.substr(componentName.rfind('/')+1);
            LOG_TRACE(Application_impl, "Unbinding component " << shortName);
            CosNaming::Name_var componentBindingName = ossie::corba::stringToName(shortName);
            try {
                _waveformContext->unbind(componentBindingName);
            } CATCH_LOG_ERROR(Application_impl, "Unable to unbind component")
        }
        LOG_DEBUG(Application_impl, "Next component")
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
        LOG_ERROR(Application_impl, err.str());
    }

    // Unbind the application's naming context using the fully-qualified name.
    LOG_TRACE(Application_impl, "Unbinding application naming context " << _waveformContextName);
    CosNaming::Name DNContextname;
    DNContextname.length(1);
    std::string domainName = _domainManager->getDomainManagerName();
    DNContextname[0].id = CORBA::string_dup(_waveformContextName.c_str());
    try {
      if ( CORBA::is_nil(_domainContext) ==  false ) {
        _domainContext->unbind(DNContextname);
      }
    } catch (const CosNaming::NamingContext::NotFound&) {
        // Someone else has removed the naming context; this is a non-fatal condition.
        LOG_WARN(Application_impl, "Naming context has already been removed");
    } CATCH_LOG_ERROR(Application_impl, "Unbind context failed with CORBA::SystemException")

    // Destroy the waveform context; it should be empty by this point, assuming all
    // of the components were properly unbound.
    LOG_TRACE(Application_impl, "Destroying application naming context " << _waveformContextName);
    try {
        _waveformContext->destroy();
    } catch (const CosNaming::NamingContext::NotEmpty&) {
        const char* error = "Application naming context not empty";
        LOG_ERROR(Application_impl, error);
        CF::StringSequence message;
        message.length(1);
        message[0] = CORBA::string_dup(error);
        throw CF::LifeCycle::ReleaseError(message);
    } CATCH_LOG_ERROR(Application_impl, "Destory waveform context: " << _waveformContextName );
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
    LOG_ERROR(Application_impl, errstr.str());
    CF::StringSequence message;
    message.length(1);
    message[0] = CORBA::string_dup(errstr.str().c_str());
    throw CF::LifeCycle::ReleaseError(message);
  }

  TRACE_EXIT(Application_impl);
}

void Application_impl::releaseComponents()
{
    for (ossie::ComponentList::iterator ii = _components.begin(); ii != _components.end(); ++ii) {
        if (CORBA::is_nil(ii->componentObject)) {
            // Ignore components that never registered
            continue;
        }

        LOG_DEBUG(Application_impl, "Releasing component '" << ii->identifier << "'");
        try {
            CF::Resource_var resource = CF::Resource::_narrow(ii->componentObject);
            unsigned long timeout = 3; // seconds
            omniORB::setClientCallTimeout(resource, timeout * 1000);
            resource->releaseObject();
        } CATCH_LOG_WARN(Application_impl, "releaseObject failed for component '" << ii->identifier << "'");
    }
}

void Application_impl::terminateComponents()
{
    // Terminate any components that were executed on devices
    for (ossie::ComponentList::iterator ii = _components.begin(); ii != _components.end(); ++ii) {
        const unsigned long pid = ii->processId;
        if (pid == 0) {
            continue;
        }

        LOG_DEBUG(Application_impl, "Terminating component '" << ii->identifier << "' pid " << pid);

        CF::ExecutableDevice_var device = ossie::corba::_narrowSafe<CF::ExecutableDevice>(ii->assignedDevice);
        if (CORBA::is_nil(device)) {
            LOG_WARN(Application_impl, "Cannot find device to terminate component " << ii->identifier);
        } else {
            try {
                device->terminate(ii->processId);
            } CATCH_LOG_WARN(Application_impl, "Unable to terminate process " << pid);
        }
    }
}

void Application_impl::unloadComponents()
{
    // Terminate any components that were executed on devices
    for (ossie::ComponentList::iterator ii = _components.begin(); ii != _components.end(); ++ii) {
        if (ii->loadedFiles.empty()) {
            continue;
        }

        LOG_DEBUG(Application_impl, "Unloading " << ii->loadedFiles.size() << " file(s) for component '"
                  << ii->identifier << "'");
        
        CF::LoadableDevice_var device = ossie::corba::_narrowSafe<CF::LoadableDevice>(ii->assignedDevice);
        if (CORBA::is_nil(device)) {
            LOG_WARN(Application_impl, "Cannot find device to unload files for component " << ii->identifier);
            continue;
        }

        for (std::vector<std::string>::iterator file = ii->loadedFiles.begin(); file != ii->loadedFiles.end();
             ++file) {
            LOG_TRACE(Application_impl, "Unloading file " << *file);
            try {
                device->unload(file->c_str());
            } CATCH_LOG_WARN(Application_impl, "Unable to unload file " << *file);
        }
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
    convert_sequence(result, _components, to_pid_type);
    return result._retn();
}

CF::Components* Application_impl::registeredComponents ()
{
    CF::Components_var result = new CF::Components();
    convert_sequence_if(result, _components, to_component_type, is_registered);
    return result._retn();
}

CF::ApplicationRegistrar_ptr Application_impl::appReg (void)
{
    return _registrar->_this();
}

CF::Application::ComponentElementSequence* Application_impl::componentNamingContexts ()
throw (CORBA::SystemException)
{
    CF::Application::ComponentElementSequence_var result = new CF::Application::ComponentElementSequence();
    convert_sequence_if(result, _components, to_name_element, has_naming_context);
    return result._retn();
}


CF::Application::ComponentElementSequence* Application_impl::componentImplementations ()
throw (CORBA::SystemException)
{
    CF::Application::ComponentElementSequence_var result = new CF::Application::ComponentElementSequence();
    convert_sequence(result, _components, to_impl_element);
    return result._retn();
}


CF::DeviceAssignmentSequence* Application_impl::componentDevices ()
throw (CORBA::SystemException)
{
    CF::DeviceAssignmentSequence_var result = new CF::DeviceAssignmentSequence();
    std::vector<ossie::DeviceAssignmentInfo>::const_iterator begin = _componentDevices.begin();
    const std::vector<ossie::DeviceAssignmentInfo>::const_iterator end = _componentDevices.end();
    for (; begin != end; ++begin) {
        ossie::corba::push_back(result, begin->deviceAssignment);
    }
    return result._retn();
}


void Application_impl::addExternalPort (const std::string& identifier, CORBA::Object_ptr port)
{
    if (_ports.count(identifier)) {
        throw std::runtime_error("Port name " + identifier + " is already in use");
    }

    _ports[identifier] = CORBA::Object::_duplicate(port);
}

void Application_impl::addExternalProperty (const std::string& propId, const std::string& externalId, CF::Resource_ptr comp)
{
    if (_properties.count(externalId)) {
        throw std::runtime_error("External Property name " + externalId + " is already in use");
    }

    _properties[externalId] = std::pair<std::string, CF::Resource_var>(propId, CF::Resource::_duplicate(comp));
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
    for (ossie::ComponentList::iterator ii = _components.begin(); ii != _components.end(); ++ii) {
        if (is_registered(*ii)) {
            identifiers.erase(ii->identifier);
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
      LOG_DEBUG(Application_impl, "Waiting for components....APP:" << _identifier << "  list " << identifiers.size() );
        if (!_registrationCondition.timed_wait(lock, end)) {
            break;
        }
    }
    return identifiers.empty();
}

CF::Application_ptr Application_impl::getComponentApplication ()
{
    if (_isAware) {
        return _this();
    } else {
        return _fakeProxy->_this();
    }
}

CF::DomainManager_ptr Application_impl::getComponentDomainManager ()
{
    if (_isAware) {
        return _domainManager->_this();
    } else {
        return CF::DomainManager::_nil();
    }
}

void Application_impl::registerComponent (CF::Resource_ptr resource)
{
    const std::string componentId = ossie::corba::returnString(resource->identifier());
    const std::string softwareProfile = ossie::corba::returnString(resource->softwareProfile());


    boost::mutex::scoped_lock lock(_registrationMutex);
    ossie::ApplicationComponent* comp = findComponent(componentId);

    if (!comp) {
        LOG_WARN(Application_impl, "Unexpected component '" << componentId
                 << "' registered with application '" << _appName << "'");
        _components.push_back(ossie::ApplicationComponent());
        comp = &(_components.back());
        comp->identifier = componentId;
        comp->softwareProfile = softwareProfile;
        comp->processId = 0;
    } else if (softwareProfile != comp->softwareProfile) {
        // Mismatch between expected and reported SPD path
        LOG_WARN(Application_impl, "Component '" << componentId << "' software profile " << softwareProfile
                 << " does not match expected profile " << comp->softwareProfile);
        comp->softwareProfile = softwareProfile;
    }

    LOG_TRACE(Application_impl, "REGISTERING Component '" << componentId << "' software profile " << softwareProfile << " pid:" << comp->processId );
    comp->componentObject = CORBA::Object::_duplicate(resource);
    _registrationCondition.notify_all();
}

std::string Application_impl::getExternalPropertyId(std::string compIdIn, std::string propIdIn)
{
    for (std::map<std::string, std::pair<std::string, CF::Resource_var> >::const_iterator prop = _properties.begin();
            prop != _properties.end(); ++prop) {
        // Gets the property mapping info
        std::string extId = prop->first;
        std::string propId = prop->second.first;
        // Gets the Resource identifier
        std::string compId = ossie::corba::returnString(prop->second.second->identifier());

        if (compId == compIdIn && propId == propIdIn) {
            return extId;
        }
    }

    return "";
}

ossie::ApplicationComponent* Application_impl::findComponent(const std::string& identifier)
{
    for (ossie::ComponentList::iterator ii = _components.begin(); ii != _components.end(); ++ii) {
        if (identifier == ii->identifier) {
            return &(*ii);
        }
    }

    return 0;
}

void Application_impl::addComponent(const std::string& identifier, const std::string& profile)
{
    if (findComponent(identifier)) {
        LOG_ERROR(Application_impl, "Component '" << identifier << "' is already registered");
        return;
    }
    LOG_DEBUG(Application_impl, "Adding component '" << identifier << "' with profile " << profile);
    ossie::ApplicationComponent component;
    component.identifier = identifier;
    component.softwareProfile = profile;
    component.processId = 0;
    _components.push_back(component);
}

void Application_impl::setComponentPid(const std::string& identifier, unsigned long pid)
{
    ossie::ApplicationComponent* component = findComponent(identifier);
    if (!component) {
        LOG_ERROR(Application_impl, "Setting process ID for unknown component '" << identifier << "'");
    } else {
        component->processId = pid;
    }
}

void Application_impl::setComponentNamingContext(const std::string& identifier, const std::string& name)
{
    ossie::ApplicationComponent* component = findComponent(identifier);
    if (!component) {
        LOG_ERROR(Application_impl, "Setting naming context for unknown component '" << identifier << "'");
    } else {
        component->namingContext = name;
    }
}

void Application_impl::setComponentImplementation(const std::string& identifier, const std::string& implementationId)
{
    ossie::ApplicationComponent* component = findComponent(identifier);
    if (!component) {
        LOG_ERROR(Application_impl, "Setting implementation for unknown component '" << identifier << "'");
    } else {
        component->implementationId = implementationId;
    }
}

void Application_impl::setComponentDevice(const std::string& identifier, CF::Device_ptr device)
{
    ossie::ApplicationComponent* component = findComponent(identifier);
    if (!component) {
        LOG_ERROR(Application_impl, "Setting device for unknown component '" << identifier << "'");
    } else {
        component->assignedDevice = CF::Device::_duplicate(device);
    }
}

void Application_impl::addComponentLoadedFile(const std::string& identifier, const std::string& fileName)
{
    ossie::ApplicationComponent* component = findComponent(identifier);
    if (!component) {
        LOG_ERROR(Application_impl, "Adding loaded file for unknown component '" << identifier << "'");
    } else {
        component->loadedFiles.push_back(fileName);
    }
}
