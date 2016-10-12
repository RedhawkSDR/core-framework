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

#include <signal.h>

#include "ossie/Resource_impl.h"
#include "ossie/Events.h"

PREPARE_CF_LOGGING(Resource_impl)

Resource_impl::Resource_impl (const char* _uuid) :
    _identifier(_uuid),
    _started(false),
    component_running_mutex(),
    component_running(&component_running_mutex),
    _domMgr(NULL),
    _initialized(false)
{
}


Resource_impl::Resource_impl (const char* _uuid, const char *label) :
    _identifier(_uuid),
    naming_service_name(label),
    _started(false),
    component_running_mutex(),
    component_running(&component_running_mutex),
    _domMgr(NULL),
    _initialized(false)
{
}

Resource_impl::~Resource_impl () {
  if (this->_domMgr != NULL)
    delete this->_domMgr;


};



void Resource_impl::setAdditionalParameters(std::string &softwareProfile, std::string &application_registrar_ior, std::string &nic)
{
    _softwareProfile = softwareProfile;
    CORBA::ORB_ptr orb = ossie::corba::Orb();
    CORBA::Object_var applicationRegistrarObject = CORBA::Object::_nil();
    try {
      RH_NL_TRACE("Resource", "narrow to Registrar object:" << application_registrar_ior );
      applicationRegistrarObject = orb->string_to_object(application_registrar_ior.c_str());
    } catch ( ... ) {
      RH_NL_WARN("Resource", "No  Registrar... create empty container");
      this->_domMgr = new redhawk::DomainManagerContainer();
      return;
    }
    CF::ApplicationRegistrar_ptr applicationRegistrar = ossie::corba::_narrowSafe<CF::ApplicationRegistrar>(applicationRegistrarObject);
    if (!CORBA::is_nil(applicationRegistrar)) {
      RH_NL_TRACE("Resource", "Get DomainManager from Registrar object:" << application_registrar_ior );
      CF::DomainManager_var dm=applicationRegistrar->domMgr();
      this->_domMgr = new redhawk::DomainManagerContainer(dm);
      return;
    }

    RH_NL_TRACE("Resource", "Resolve DeviceManager...");
    CF::DeviceManager_var devMgr = ossie::corba::_narrowSafe<CF::DeviceManager>(applicationRegistrarObject);
    if (!CORBA::is_nil(devMgr)) {
      RH_NL_TRACE("Resource", "Resolving DomainManager from DeviceManager...");
        CF::DomainManager_var dm=devMgr->domMgr();
        this->_domMgr = new redhawk::DomainManagerContainer(dm);
        return;
    }

    RH_NL_DEBUG("Resource", "All else failed.... use empty container");
    this->_domMgr = new redhawk::DomainManagerContainer();
}


void Resource_impl::constructor ()
{
}


void Resource_impl::start () throw (CORBA::SystemException, CF::Resource::StartError)
{
    startPorts();
    _started = true;
}


void Resource_impl::stop () throw (CORBA::SystemException, CF::Resource::StopError)
{
    stopPorts();
    _started = false;
}

char* Resource_impl::identifier () throw (CORBA::SystemException)
{
    return CORBA::string_dup(_identifier.c_str());
}

char* Resource_impl::softwareProfile ()
throw (CORBA::SystemException)
{
    return CORBA::string_dup(_softwareProfile.c_str());
}

CORBA::Boolean Resource_impl::started () throw (CORBA::SystemException)
{
    return _started;
}



void Resource_impl::initialize () throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
  startPropertyChangeMonitor(_identifier);
  if (!_initialized) {
      _initialized = true;
      try {
          constructor();
      } catch (const std::exception& exc) {
          LOG_ERROR(Resource_impl, "initialize(): " << exc.what());
          CF::StringSequence messages;
          ossie::corba::push_back(messages, exc.what());
          throw CF::LifeCycle::InitializeError(messages);
      }
  }
}

void Resource_impl::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    releasePorts();
    stopPropertyChangeMonitor();
    redhawk::events::Manager::Terminate();
    PortableServer::POA_ptr root_poa = ossie::corba::RootPOA();
    PortableServer::ObjectId_var oid = root_poa->servant_to_id(this);
    root_poa->deactivate_object(oid);

    component_running.signal();
}

void Resource_impl::run() {
    // Start handling CORBA requests
    LOG_TRACE(Resource_impl, "handling CORBA requests");
    component_running.wait();
    LOG_TRACE(Resource_impl, "leaving run()");
}

void Resource_impl::halt() {
    LOG_DEBUG(Resource_impl, "Halting component")

    LOG_TRACE(Resource_impl, "Sending device running signal");
    component_running.signal();
    LOG_TRACE(Resource_impl, "Done sending device running signal");
}

void Resource_impl::setCurrentWorkingDirectory(std::string& cwd) {
    this->currentWorkingDirectory = cwd;
}

std::string& Resource_impl::getCurrentWorkingDirectory() {
    return this->currentWorkingDirectory;
}

static Resource_impl* main_component = 0;
static void sigint_handler(int signum)
{
    main_component->halt();
}

void Resource_impl::start_component(Resource_impl::ctor_type ctor, int argc, char* argv[])
{
    std::string application_registrar_ior;
    std::string component_identifier;
    std::string name_binding;
    std::string profile = "";
    std::string nic = "";
    const char* logging_config_uri = 0;
    int debug_level = -1; // use log config uri as log level context
    bool standAlone = false;
    std::map<std::string, char*> execparams;
    std::string logcfg_uri("");
    std::string dpath("");
    bool skip_run = false;

    // Parse execparams.
    for (int i=0; i < argc; i++) {
        if (strcmp("NAMING_CONTEXT_IOR", argv[i]) == 0) {
            application_registrar_ior = argv[++i];
        } else if (strcmp("NIC", argv[i]) == 0) {
            nic = argv[++i];
        } else if (strcmp("PROFILE_NAME", argv[i]) == 0) {
            profile = argv[++i];
        } else if (strcmp("COMPONENT_IDENTIFIER", argv[i]) == 0) {
            component_identifier = argv[++i];
        } else if (strcmp("NAME_BINDING", argv[i]) == 0) {
            name_binding = argv[++i];
        } else if (strcmp("LOGGING_CONFIG_URI", argv[i]) == 0) {
            logging_config_uri = argv[++i];
        } else if (strcmp("DEBUG_LEVEL", argv[i]) == 0) {
            debug_level = atoi(argv[++i]);
        } else if (strcmp("DOM_PATH", argv[i]) == 0) {
            dpath = argv[++i];
        } else if (strcmp("-i", argv[i]) == 0) {
            standAlone = true;
        } else if (strcmp("SKIP_RUN", argv[i]) == 0) {
            skip_run = true;
        } else if (i > 0) {  // any other argument besides the first one is part of the execparams
            std::string paramName = argv[i];
            execparams[paramName] = argv[++i];
        }
    }

    if (standAlone) {
        if (component_identifier.empty()) {
            component_identifier = ossie::generateUUID();
        }
        if (name_binding.empty()) {
            name_binding = "";
        }
    } else {
        if (application_registrar_ior.empty()) {
            std::cout<<std::endl<<"usage: "<<argv[0]<<" [options] [execparams]"<<std::endl<<std::endl;
            std::cout<<"The set of execparams is defined in the .prf for the component"<<std::endl;
            std::cout<<"They are provided as arguments pairs ID VALUE, for example:"<<std::endl;
            std::cout<<"     "<<argv[0]<<" INT_PARAM 5 STR_PARAM ABCDED"<<std::endl<<std::endl;
            std::cout<<"Options:"<<std::endl;
            std::cout<<"     -i,--interactive           Run the component in interactive test mode"<<std::endl<<std::endl;
            exit(-1);
        }
    }

    // The ORB must be initialized before configuring logging, which may use
    // CORBA to get its configuration file.
    CORBA::ORB_ptr orb = ossie::corba::CorbaInit(argc, argv);

    // check if logging config URL was specified...
    if ( logging_config_uri ) logcfg_uri=logging_config_uri;

    // setup logging context for a component resource
    ossie::logging::ResourceCtxPtr ctx( new ossie::logging::ComponentCtx(name_binding, component_identifier, dpath ) );

    if (!skip_run) {
        // configure the  logging library
        ossie::logging::Configure(logcfg_uri, debug_level, ctx);
    }


    // Create the servant.
    LOG_TRACE(Resource_impl, "Creating component with identifier '" << component_identifier << "'");
    Resource_impl* resource = ctor(component_identifier, name_binding);

    resource->setAdditionalParameters(profile, application_registrar_ior, nic);

    if ( !skip_run ) {
        // assign the logging context to the resource to support logging interface
        resource->saveLoggingContext( logcfg_uri, debug_level, ctx );
    }

    // setting all the execparams passed as argument, this method resides in the Resource_impl class
    resource->setExecparamProperties(execparams);

    std::string pathAndFile = argv[0];
    unsigned lastSlash      = pathAndFile.find_last_of("/");
    std::string cwd         = pathAndFile.substr(0, lastSlash);
    resource->setCurrentWorkingDirectory(cwd);

    // Activate the component servant.
    LOG_TRACE(Resource_impl, "Activating component object");
    PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(resource);
    CF::Resource_var resource_obj = resource->_this();

    // Get the application naming context and bind the component into it.
    if (!application_registrar_ior.empty()) {
        LOG_TRACE(Resource_impl, "Binding component to application context with name '" << name_binding << "'");
        CORBA::Object_var applicationRegistrarObject = orb->string_to_object(application_registrar_ior.c_str());
        CF::ApplicationRegistrar_var applicationRegistrar = ossie::corba::_narrowSafe<CF::ApplicationRegistrar>(applicationRegistrarObject);
        if (!CORBA::is_nil(applicationRegistrar)) {
            applicationRegistrar->registerComponent(name_binding.c_str(), resource_obj);
        } else {
            // the registrar is not available (because the invoking infrastructure only uses the name service)
            CosNaming::NamingContext_var applicationContext = ossie::corba::_narrowSafe<CosNaming::NamingContext>(applicationRegistrarObject);
            ossie::corba::bindObjectToContext(resource_obj, applicationContext, name_binding);
        }
    } else {
        if (standAlone) {
            std::cout<<orb->object_to_string(resource_obj)<<std::endl;
        }
    }

    if (!skip_run){
        // Store away a reference to the main component and establish a handler for
        // SIGINT that will break out of run()
        main_component = resource;
        struct sigaction sa;
        sa.sa_handler = &sigint_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGINT, &sa, NULL);

        LOG_TRACE(Resource_impl, "Entering component run loop");
        resource->run();
        LOG_TRACE(Resource_impl, "Component run loop terminated");

        // Ignore SIGINT from here on out to ensure that the ORB gets shut down
        // properly
        sa.sa_handler = SIG_IGN;
        sigemptyset(&sa.sa_mask);
        sigaction(SIGINT, &sa, NULL);
        main_component = 0;

        LOG_TRACE(Resource_impl, "Deleting component");
        resource->_remove_ref();
        LOG_TRACE(Resource_impl, "Shutting down ORB");

        ossie::logging::Terminate();

        ossie::corba::OrbShutdown(true);
    }
}
