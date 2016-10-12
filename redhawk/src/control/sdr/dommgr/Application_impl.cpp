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

#include <ossie/debug.h>
#include <ossie/CorbaUtils.h>
#include <ossie/EventChannelSupport.h>

#include "Application_impl.h"
#include "DomainManager_impl.h"
#include "AllocationManager_impl.h"
#include "connectionSupport.h"

PREPARE_LOGGING(Application_impl);

using namespace ossie;

Application_impl::Application_impl (const char* _id, const char* _name, const char* _profile, DomainManager_impl* domainManager, const std::string& waveformContextName,
                                    CosNaming::NamingContext_ptr WaveformContext) :
    _identifier(_id)
{
    _domainManager = domainManager;
    _domainName = _domainManager->getDomainManagerName();

    appName = _name;
    sadProfile = _profile;
    _waveformContextName = waveformContextName;
    _WaveformContext = CosNaming::NamingContext::_duplicate(WaveformContext);

    this->appComponentDevices.length(0);
    this->appComponentImplementations.length(0);
    this->appComponentNamingContexts.length(0);
    this->appComponentProcessIds.length(0);
    
    release_already_called = false;
};

void Application_impl::populateApplication(CF::Resource_ptr _controller,
                                           std::vector<ossie::DeviceAssignmentInfo>&  _devSeq,
                                           CF::Application::ComponentElementSequence* _implSeq, 
                                           std::vector<CF::Resource_ptr> _startSeq,
                                           CF::Application::ComponentElementSequence* _ncSeq,
                                           CF::Application::ComponentProcessIdSequence* _pidSeq,
                                           std::vector<ConnectionNode>& connections,
                                           std::map<std::string, std::string>& fileTable,
                                           std::vector<std::string> allocationIDs)
{
    TRACE_ENTER(Application_impl)
    _fileTable = fileTable;
    _connections = connections;
    _componentDevices = _devSeq;
    _appStartSeq = _startSeq;
    
    _registeredComponents.length(0);

    LOG_DEBUG(Application_impl, "Getting File Manager reference")

    LOG_DEBUG(Application_impl, "Creating device sequence")
    if (_devSeq.size() != 0) {
        this->appComponentDevices.length (_devSeq.size());

        for (unsigned i = 0; i < _devSeq.size(); i++) {
            appComponentDevices[i] = _devSeq[i].deviceAssignment;
        }
    }

    LOG_DEBUG(Application_impl, "Creating implementation sequence")
    if (_implSeq != NULL) {
        this->appComponentImplementations.length (_implSeq->length ());

        for (unsigned int i = 0; i < _implSeq->length (); i++) {
            appComponentImplementations[i] = (*_implSeq)[i];
        }
    }

    LOG_DEBUG(Application_impl, "Creating naming context sequence")
    if (_ncSeq != NULL) {
        this->appComponentNamingContexts.length (_ncSeq->length ());

        for (unsigned int i = 0; i < _ncSeq->length (); i++) {
            appComponentNamingContexts[i] = (*_ncSeq)[i];
            _componentNames[static_cast<const char*>((*_ncSeq)[i].componentId)] = static_cast<const char*>((*_ncSeq)[i].elementId);
        }
    }

    LOG_DEBUG(Application_impl, "Creating process id sequence")
    if (_pidSeq != NULL) {
        this->appComponentProcessIds.length (_pidSeq->length ());

        for (unsigned int i = 0; i < _pidSeq->length (); i++) {
            appComponentProcessIds[i] = (*_pidSeq)[i];
            _pidTable[static_cast<const char*>((*_pidSeq)[i].componentId)] = (*_pidSeq)[i].processId;
        }
    } 

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
    appComponentDevices.release();
    appComponentImplementations.release();
    appComponentNamingContexts.release();
    appComponentProcessIds.release();
    _registeredComponents.release();
    TRACE_EXIT(Application_impl)
};

char* Application_impl::identifier () throw (CORBA::SystemException)
{
    return CORBA::string_dup(_identifier.c_str());
}

CORBA::Boolean Application_impl::started () throw (CORBA::SystemException)
{
    if (CORBA::is_nil(assemblyController)) { return false; }
    return assemblyController->started();
}

void Application_impl::start ()
throw (CORBA::SystemException, CF::Resource::StartError)
{
    if (CORBA::is_nil(assemblyController)) { return; }

    try {
        LOG_TRACE(Application_impl, "Calling start on assembly controller")
        assemblyController->start ();

        // Start the rest of the components
        for (unsigned int i = 0; i < _appStartSeq.size(); i++){
            std::string msg = "Calling start for ";
            msg = msg.append(ossie::corba::returnString(_appStartSeq[i]->identifier()));
            LOG_TRACE(Application_impl, msg)

            _appStartSeq[i]-> start();
        }
    } catch( CF::Resource::StartError& se ) {
        LOG_ERROR(Application_impl, "Start failed with CF:Resource::StartError")
        throw;
    } CATCH_THROW_LOG_ERROR(Application_impl, "Start failed", CF::Resource::StartError())
}


void Application_impl::stop ()
throw (CORBA::SystemException, CF::Resource::StopError)
{
    if (CORBA::is_nil(assemblyController)) { return; }

    unsigned long timeout = 3; // seconds
    try {
        LOG_TRACE(Application_impl, "Calling stop on assembly controller");

        // Stop the components in the reverse order they were started
        for (int i = (int)(_appStartSeq.size()-1); i >= 0; i--){
            std::string msg = "Calling stop for ";
            msg = msg.append(ossie::corba::returnString(_appStartSeq[i]->identifier()));
            LOG_TRACE(Application_impl, msg);

            omniORB::setClientCallTimeout(_appStartSeq[i], timeout * 1000);
            _appStartSeq[i]-> stop();
        }

        omniORB::setClientCallTimeout(assemblyController, timeout * 1000);
        assemblyController->stop ();
    } catch( CF::Resource::StopError& se ) {
        LOG_ERROR(Application_impl, "Stop failed with CF::Resource::StopError")
        throw;
    } CATCH_THROW_LOG_ERROR(Application_impl, "Stop failed", CF::Resource::StopError(CF::CF_ESRCH, "Object might not exist"))
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
        for (std::map<std::string, std::pair<std::string, CF::Resource_ptr> >::const_iterator prop = _properties.begin();
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
        //TODO: list all properties in 'up' as part of the error message
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
    TRACE_ENTER(Application_impl)

    // make sure releaseObject hasn't already been called
    boost::mutex::scoped_lock lock(releaseObjectLock);
    if (release_already_called) {
        LOG_DEBUG(Application_impl, "skipping release because release has already been called")
        return;
    } else {
        release_already_called = true;
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
    
    // Break all connections in the application
    ConnectionManager::disconnectAll(_connections, _domainManager);
    LOG_DEBUG(Application_impl, "app->releaseObject finished disconnecting ports");

    // Release all resources
    // Before releasing the components, all executed processes should be terminated,
    // all software components unloaded, and all capacities deallocated
    // search thru all waveform components
    // unload and deallocate capacity

    for (CORBA::ULong ii = 0; ii < _registeredComponents.length(); ++ii) {
        LOG_DEBUG(Application_impl, "Releasing component '" << _registeredComponents[ii].identifier << "'");
        try {
            CF::Resource_var resource = CF::Resource::_narrow(_registeredComponents[ii].componentObject);
            unsigned long timeout = 3; // seconds
            omniORB::setClientCallTimeout(resource, timeout * 1000);
            resource->releaseObject();
        } CATCH_LOG_WARN(Application_impl, "releaseObject failed for component '" << _registeredComponents[ii].identifier << "'");
    }

    // Search thru all waveform components
    //  - unbind from NS
    //  - release each component
    //  - unload and deallocate
    for (unsigned int i = 0; i < appComponentImplementations.length (); i++) {

        std::string id(appComponentImplementations[i].componentId);

        if (_componentNames.find(id) != _componentNames.end()) {
            std::string componentName = _componentNames[id];

            // Unbind the component from the naming context. This assumes that the component is
            // bound into the waveform context, and its name inside of the context follows the
            // last slash in the fully-qualified name.
            std::string shortName = componentName.substr(componentName.rfind('/')+1);
            LOG_TRACE(Application_impl, "Unbinding component " << shortName);
            CosNaming::Name_var componentBindingName = ossie::corba::stringToName(shortName);
            try {
                _WaveformContext->unbind(componentBindingName);
            } CATCH_LOG_ERROR(Application_impl, "Unable to unbind component")
        }
        

        // unload component
        LOG_DEBUG(Application_impl, "Unloading and terminating components")

        CORBA::Object_ptr _devObj = CORBA::Object::_nil ();
        std::string _devId;
        // find the DeviceAssignmentInfo instance associated with this component instance
        for (unsigned int i = 0; i < _componentDevices.size(); i++) {
            try {
                if (id == ossie::corba::returnString(_componentDevices[i].deviceAssignment.componentId)) {
                    _devObj = _componentDevices[i].device;
                    _devId = ossie::corba::returnString(_componentDevices[i].deviceAssignment.assignedDeviceId);
                    if (!ossie::corba::objectExists(_devObj)) {
                        LOG_WARN(Application_impl, "Not deallocating capacity on device " << 
                            _componentDevices[i].deviceAssignment.assignedDeviceId << " because it no longer exists");
                        continue;
                    }
                } else {
                    continue;
                }
            } CATCH_LOG_WARN(Application_impl, "Unable to retrieve device ID for deallocation or process termination. Continuing the App release")

            // unload component is launched on Loadable Device
            std::string localFileName = _fileTable[id];
            bool loadable = false;
            if (!localFileName.empty()) {
                CF::LoadableDevice_var loadDev;
                // see if device is Loadable
                try {
                    loadDev = CF::LoadableDevice::_narrow(_devObj);
                    CORBA::is_nil( loadDev ) ? loadable = false : loadable = true;
                } catch( CORBA::Exception& ex ) {
                    LOG_DEBUG(Application_impl, "Not a Loadable device...moving on")
                    loadable = false;
                }
                // if Loadable, do the unload
                if (loadable) {
                    try {
                        LOG_DEBUG(Application_impl, "Unloading: " << localFileName)
                        loadDev->unload(localFileName.c_str());

                    } catch (...) {
                        LOG_INFO(Application_impl, "loadDev->unload failed");
                        continue;
                    }
                }
            }

            // terminate component if launched on Executable Device
            if (loadable) {
                CF::ExecutableDevice_var execDev;
                bool executable = false;
                try {
                    execDev = CF::ExecutableDevice::_narrow (_devObj);
                    CORBA::is_nil( execDev ) ? executable = false : executable = true;
                } catch( ... ) {
                    LOG_DEBUG(Application_impl, "Not an executable device...moving on")
                    executable = false;
                }
                if (executable) {
                    try {
                        LOG_DEBUG(Application_impl, "Terminating " << _pidTable[id] << " on " << ossie::corba::returnString(execDev->label()));
                        execDev ->terminate ( _pidTable[id] );
                    } catch ( ... ) {
                        LOG_WARN(Application_impl, "Call to terminate failed. Continuing the App release")
                        continue;
                    }
                    break;
                }
            }
        }
        LOG_DEBUG(Application_impl, "Next component")
    }

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
    DNContextname.length(2);
    DNContextname[0].id = CORBA::string_dup(_domainName.c_str());
    DNContextname[1].id = CORBA::string_dup(_waveformContextName.c_str());
    try {
        ossie::corba::InitialNamingContext()->unbind(DNContextname);
    } catch (const CosNaming::NamingContext::NotFound&) {
        // Someone else has removed the naming context; this is a non-fatal condition.
        LOG_WARN(Application_impl, "Naming context has already been removed");
    } CATCH_LOG_ERROR(Application_impl, "Unbind context failed with CORBA::SystemException")
    
    // Destroy the waveform context; it should be empty by this point, assuming all
    // of the components were properly unbound.
    LOG_TRACE(Application_impl, "Destroying application naming context " << _waveformContextName);
    try {
        _WaveformContext->destroy();
    } catch (const CosNaming::NamingContext::NotEmpty&) {
        const char* error = "Application naming context not empty";
        LOG_ERROR(Application_impl, error);
        CF::StringSequence message;
        message.length(1);
        message[0] = CORBA::string_dup(error);
        throw CF::LifeCycle::ReleaseError(message);
    }
    _WaveformContext = CosNaming::NamingContext::_nil();


    // Send an event with the Application releaseObject
    ossie::sendObjectRemovedEvent(Application_impl::__logger, _identifier.c_str(), _identifier.c_str(), appName.c_str(), 
        StandardEvent::APPLICATION, _domainManager->proxy_consumer);

    // Deactivate this servant from the POA.
    PortableServer::POA_var dm_poa = ossie::corba::RootPOA()->find_POA("DomainManager", 0);
    app_poa = dm_poa->find_POA("Applications", 0);
    PortableServer::ObjectId_var oid = app_poa->servant_to_id(this);
    app_poa->deactivate_object(oid);

    TRACE_EXIT(Application_impl)
}

char* Application_impl::name ()
throw (CORBA::SystemException)
{
    return CORBA::string_dup(appName.c_str());
}


char* Application_impl::profile ()
throw (CORBA::SystemException)
{
    return CORBA::string_dup(sadProfile.c_str());
}

char* Application_impl::softwareProfile ()
throw (CORBA::SystemException)
{
    return CORBA::string_dup(sadProfile.c_str());
}


CF::Application::ComponentProcessIdSequence* Application_impl::componentProcessIds ()
throw (CORBA::SystemException)
{
    CF::Application::ComponentProcessIdSequence_var result = \
                                                             new CF::Application::ComponentProcessIdSequence(appComponentProcessIds);
    return result._retn();
}

CF::Components* Application_impl::registeredComponents ()
{
    CF::Components_var ret_registeredComponents = new CF::Components(_registeredComponents);

    return ret_registeredComponents._retn();
}

void Application_impl::registerComponent (CF::ComponentType &component)
{
    unsigned int compLength = _registeredComponents.length();
    _registeredComponents.length(compLength+1);
    _registeredComponents[compLength].identifier = CORBA::string_dup(component.identifier);
    _registeredComponents[compLength].softwareProfile = CORBA::string_dup(component.softwareProfile);
    _registeredComponents[compLength].type = component.type;
    _registeredComponents[compLength].componentObject = CORBA::Object::_duplicate(component.componentObject);

    return;
}

CF::Application::ComponentElementSequence* Application_impl::componentNamingContexts ()
throw (CORBA::SystemException)
{
    CF::Application::ComponentElementSequence_var result = new CF::Application::ComponentElementSequence(appComponentNamingContexts);
    return result._retn();
}


CF::Application::ComponentElementSequence* Application_impl::componentImplementations ()
throw (CORBA::SystemException)
{
    CF::Application::ComponentElementSequence_var result = new CF::Application::ComponentElementSequence(appComponentImplementations);
    return result._retn();
}


CF::DeviceAssignmentSequence* Application_impl::componentDevices ()
throw (CORBA::SystemException)
{
    CF::DeviceAssignmentSequence_var result = new CF::DeviceAssignmentSequence(appComponentDevices);
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

    _properties[externalId] = std::pair<std::string, CF::Resource_ptr>(propId, comp);
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

std::string Application_impl::getExternalPropertyId(std::string compIdIn, std::string propIdIn)
{
    for (std::map<std::string, std::pair<std::string, CF::Resource_ptr> >::const_iterator prop = _properties.begin();
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
