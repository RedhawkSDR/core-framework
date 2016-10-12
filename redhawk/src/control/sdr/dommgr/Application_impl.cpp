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
                                           ossie::SoftPkgList  &softpkgList,
                                           std::map<std::string, std::vector<ossie::AllocPropsInfo> >& allocPropsTable)
{
    TRACE_ENTER(Application_impl)
    _fileTable = fileTable;
    _softpkgList = softpkgList;
    _allocPropsTable = allocPropsTable;
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

    try {
        LOG_TRACE(Application_impl, "Calling stop on assembly controller");

        // Stop the components in the reverse order they were started
        for (int i = (int)(_appStartSeq.size()-1); i >= 0; i--){
            std::string msg = "Calling stop for ";
            msg = msg.append(ossie::corba::returnString(_appStartSeq[i]->identifier()));
            LOG_TRACE(Application_impl, msg);

            _appStartSeq[i]-> stop();
        }

        assemblyController->stop ();
    } catch( CF::Resource::StopError& se ) {
        LOG_ERROR(Application_impl, "Stop failed with CF::Resource::StopError")
        throw;
    } CATCH_THROW_LOG_ERROR(Application_impl, "Stop failed", CF::Resource::StopError(CF::CF_ESRCH, "Object might not exist"))
}

void Application_impl::configure (const CF::Properties& configProperties)
throw (CF::PropertySet::PartialConfiguration, CF::PropertySet::InvalidConfiguration, CORBA::SystemException)
{
    if (CORBA::is_nil(assemblyController)) { return; }

    LOG_TRACE(Application_impl, "Calling configure on assembly controller")
    assemblyController->configure(configProperties);
}

void Application_impl::query (CF::Properties& configProperties)
throw (CF::UnknownProperties, CORBA::SystemException)
{
    if (CORBA::is_nil(assemblyController)) { return; }

    LOG_TRACE(Application_impl, "Calling query on assembly controller")
    assemblyController->query(configProperties);
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

    // Search thru all waveform components
    //  - unbind from NS
    //  - release each component
    //  - unload and deallocate
    for (unsigned int i = 0; i < appComponentImplementations.length (); i++) {

        std::string id(appComponentImplementations[i].componentId);

        LOG_DEBUG(Application_impl, "Releasing component " << id)

        if (_componentNames.find(id) != _componentNames.end()) {
            std::string componentName = _componentNames[id];
            CORBA::Object_var _rscObj = CORBA::Object::_nil ();

            while (CORBA::is_nil (_rscObj)) {
                try {
                    _rscObj = ossie::corba::objectFromName(componentName);
                } catch( CORBA::SystemException& se ) {
                    LOG_INFO(Application_impl, "[Application::releaseObject] \"orb->get_object_from_name\" failed with find the component in the naming service with name " << componentName << ". Continuing the App release")
                    break;
                } catch( ... ) {
                    LOG_INFO(Application_impl, "[Application::releaseObject] \"orb->get_object_from_name\" failed with Unknown Exception for " << componentName << ". Continuing the App release")
                    break;
                }
            }

            // Unbind the component from the naming context. This assumes that the component is
            // bound into the waveform context, and its name inside of the context follows the
            // last slash in the fully-qualified name.
            std::string shortName = componentName.substr(componentName.rfind('/')+1);
            LOG_TRACE(Application_impl, "Unbinding component " << shortName);
            CosNaming::Name_var componentBindingName = ossie::corba::stringToName(shortName);
            try {
                _WaveformContext->unbind(componentBindingName);
            } CATCH_LOG_ERROR(Application_impl, "Unable to unbind component")
            
            // call releaseObject on the component
            CF::ResourceFactory_var _rscFac = 0;
            try {
                _rscFac = CF::ResourceFactory::_narrow (_rscObj);
            } CATCH_LOG_DEBUG(Application_impl, "narrow on resource object failed. Continuing the App release") 

            if (!CORBA::is_nil(_rscFac)) {
                LOG_DEBUG(Application_impl, "about to release resource factory object")
                try {
                    // TODO determine what is correct here , since localFileName doesn't seem right
                    //_rscFac->releaseResource (localFileName.c_str());
                } CATCH_LOG_WARN(Application_impl, "releaseResource failed for " << componentName << ". Continuing the App release")
            } else if (!CORBA::is_nil(_rscObj)) {
                LOG_DEBUG(Application_impl, "about to release Object")
                try {
                    CF::Resource_var _rsc = CF::Resource::_narrow (_rscObj);
                    unsigned long timeout = 3; // seconds
                    omniORB::setClientCallTimeout(_rsc, timeout * 1000);
                    _rsc->releaseObject ();
                    LOG_DEBUG(Application_impl, "returned from releaseObject")
                } CATCH_LOG_WARN(Application_impl, "releaseObject failed for: " << componentName << ". Continuing the App release")
            }
            
            LOG_DEBUG(Application_impl, "app->releaseObject finished releasing this component")
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

#if 0
        //  Removes soft package dependencies when applications are released...This can potentially slow
        // down deployments because we are cleaning up files that could be used again....e.g. the
        // same waveform with dependencies is started and stopped and started..
        //
        ossie::SoftPkgList::iterator pkg = _softpkgList.begin();
        for ( ;  pkg != _softpkgList.end(); pkg++ ) {
          try {
            if ( ossie::corba::objectExists(pkg->first) ) {
              CF::LoadableDevice_ptr loadDev = CF::LoadableDevice::_narrow(pkg->first);
              if ( CORBA::is_nil(loadDev) == false ) {
                LOG_DEBUG(Application_impl, "Unload soft package dependency:" << pkg->second);
                loadDev->unload(pkg->second.c_str());
              }
              else {
                throw -1;
              }
            }
            else {
              throw -1;
            }
          }
          catch(...) {
            // issue warning the unload failed for soft pkg unload
            LOG_WARN(Application_impl, "Unable to unload soft package dependency:" << pkg->second);
          }
          

        }
#endif
        
        // deallocate capacity
        LOG_DEBUG(Application_impl, "Determining if we need to deallocate capacities")
        try {
            LOG_DEBUG(Application_impl, "Entering deallocation function " << _allocPropsTable.count(id))
            if (_allocPropsTable.count(id) > 0) {
                LOG_DEBUG(Application_impl, "Entering deallocation function " << _allocPropsTable[id].size())
                for (unsigned int devCount = 0; devCount < _allocPropsTable[id].size(); devCount++) {
                    if (_allocPropsTable[id][devCount].properties.length() > 0) {
                        // first check to see if device still exists
                        if (!ossie::corba::objectExists(_allocPropsTable[id][devCount].device)) {
                            LOG_WARN(Application_impl, "Not deallocating capacity on device " << 
                                ossie::corba::returnString(_allocPropsTable[id][devCount].device->identifier()) <<
                                " because it no longer exists");
                            continue;
                        }
                        // deallocate capacity
                        LOG_DEBUG(Application_impl, "deallocating on device " <<
                                ossie::corba::returnString(_allocPropsTable[id][devCount].device->identifier()));
                        _allocPropsTable[id][devCount].device->deallocateCapacity(_allocPropsTable[id][devCount].properties);
                        LOG_DEBUG(Application_impl, "Finished deallocating")
                    } else {
                        LOG_DEBUG(Application_impl, "No capacity to deallocate")
                    }
                }
            }
        } catch ( ... ) {
            LOG_WARN(Application_impl, "Deallocation on component " << id << " failed on device " << _devId << ". Continuing the App release")
            continue;
        }

        LOG_DEBUG(Application_impl, "Next component")
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
#if ENABLE_EVENTS
    ossie::sendObjectRemovedEvent(Application_impl::__logger, _identifier.c_str(), _identifier.c_str(), appName.c_str(), 
        StandardEvent::APPLICATION, _domainManager->proxy_consumer);
#endif

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


bool Application_impl::checkConnectionDependency (Endpoint::DependencyType type, const std::string& identifier) const
{
    for (std::vector<ConnectionNode>::const_iterator connection = _connections.begin(); connection != _connections.end(); ++connection) {
        if (connection->checkDependency(type, identifier)) {
            return true;
        }
    }
    
    return false;
}
