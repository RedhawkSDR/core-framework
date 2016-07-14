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
#include "ossie/Component.h"

PREPARE_CF_LOGGING(Resource_impl)

Resource_impl::Resource_impl (const char* _uuid) :
    _identifier(_uuid),
    _started(false),
    component_running_mutex(),
    component_running(&component_running_mutex),
    _domMgr(new redhawk::DomainManagerContainer()),
    _initialized(false)
{
}


Resource_impl::Resource_impl (const char* _uuid, const char *label) :
    _identifier(_uuid),
    naming_service_name(label),
    _started(false),
    component_running_mutex(),
    component_running(&component_running_mutex),
    _domMgr(new redhawk::DomainManagerContainer()),
    _initialized(false)
{
}

Resource_impl::~Resource_impl ()
{
}


void Resource_impl::setAdditionalParameters(std::string& softwareProfile, std::string &application_registrar_ior, std::string &nic)
{
    _softwareProfile = softwareProfile;
    CORBA::ORB_ptr orb = ossie::corba::Orb();
    CORBA::Object_var applicationRegistrarObject = CORBA::Object::_nil();
    try {
      RH_NL_TRACE("Resource", "narrow to Registrar object:" << application_registrar_ior );
      applicationRegistrarObject = orb->string_to_object(application_registrar_ior.c_str());
    } catch ( ... ) {
      RH_NL_WARN("Resource", "No  Registrar... create empty container");
      setDomainManager(CF::DomainManager::_nil());
      return;
    }
    CF::ApplicationRegistrar_ptr applicationRegistrar = ossie::corba::_narrowSafe<CF::ApplicationRegistrar>(applicationRegistrarObject);
    if (!CORBA::is_nil(applicationRegistrar)) {
      RH_NL_TRACE("Resource", "Get DomainManager from Registrar object:" << application_registrar_ior );
      CF::DomainManager_var dm=applicationRegistrar->domMgr();
      setDomainManager(dm);
      return;
    }

    RH_NL_TRACE("Resource", "Resolve DeviceManager...");
    CF::DeviceManager_var devMgr = ossie::corba::_narrowSafe<CF::DeviceManager>(applicationRegistrarObject);
    if (!CORBA::is_nil(devMgr)) {
      RH_NL_TRACE("Resource", "Resolving DomainManager from DeviceManager...");
        CF::DomainManager_var dm=devMgr->domMgr();
        setDomainManager(dm);
        return;
    }

    RH_NL_DEBUG("Resource", "All else failed.... use empty container");
    setDomainManager(CF::DomainManager::_nil());
}

redhawk::DomainManagerContainer* Resource_impl::getDomainManager()
{
    return _domMgr.get();
}

void Resource_impl::setDomainManager(CF::DomainManager_ptr domainManager)
{
    _domMgr.reset(new redhawk::DomainManagerContainer(domainManager));
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
    PortableServer::POA_ptr root_poa = ossie::corba::RootPOA();
    PortableServer::ObjectId_var oid = root_poa->servant_to_id(this);
    root_poa->deactivate_object(oid);

    component_running.signal();

    _resourceReleased(this);
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

const std::string& Resource_impl::getIdentifier() const
{
    return _identifier;
}

void Resource_impl::setCurrentWorkingDirectory(std::string& cwd) {
    this->currentWorkingDirectory = cwd;
}

std::string& Resource_impl::getCurrentWorkingDirectory() {
    return this->currentWorkingDirectory;
}

const std::string& Resource_impl::getDeploymentRoot() const
{
    return _deploymentRoot;
}

void Resource_impl::setCommandLineProperty(const std::string& id, const redhawk::Value& value)
{
    if (id == "PROFILE_NAME") {
        _softwareProfile = value.toString();
    } else if (id == "RH::DEPLOYMENT_ROOT") {
        _deploymentRoot = value.toString();
    } else {
        PropertySet_impl::setCommandLineProperty(id, value);
    }
}

Resource_impl* Resource_impl::create_component(Resource_impl::ctor_type ctor, const CF::Properties& properties)
{
    const redhawk::PropertyMap& parameters = redhawk::PropertyMap::cast(properties);

    std::string identifier;
    std::string name_binding;
    std::string application_registrar_ior;
    redhawk::PropertyMap cmdlineProps;
    for (redhawk::PropertyMap::const_iterator prop = parameters.begin(); prop != parameters.end(); ++prop) {
        const std::string id = prop->getId();
        if (id == "COMPONENT_IDENTIFIER") {
            identifier = prop->getValue().toString();
        } else if (id == "NAME_BINDING") {
            name_binding = prop->getValue().toString();
        } else if (id == "NAMING_CONTEXT_IOR") {
            application_registrar_ior = prop->getValue().toString();
        } else if (id == "DEBUG_LEVEL") {
            // If DEBUG_LEVEL is in the parameters, this component is being
            // created from a shared library entry point, not the command line;
            // logging has already been set up by the component host.
            // TODO: Revisit as part of logging update
        } else {
            cmdlineProps.push_back(*prop);
        }
    }

    LOG_TRACE(Resource_impl, "Creating component with identifier '" << identifier << "'");
    Resource_impl* resource = ctor(identifier, name_binding);

    // Initialize command line properties, which can include special properties
    // like PROFILE_NAME.
    for (redhawk::PropertyMap::const_iterator prop = cmdlineProps.begin(); prop != cmdlineProps.end(); ++prop) {
        resource->setCommandLineProperty(prop->getId(), prop->getValue());
    }

    // Activate the component servant.
    LOG_TRACE(Resource_impl, "Activating component object");
    PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(resource);
    CF::Resource_var resource_obj = resource->_this();

    // Get the application naming context and bind the component into it.
    if (!application_registrar_ior.empty()) {
        CORBA::Object_var applicationRegistrarObject = ossie::corba::stringToObject(application_registrar_ior);
        CF::ApplicationRegistrar_var applicationRegistrar = ossie::corba::_narrowSafe<CF::ApplicationRegistrar>(applicationRegistrarObject);
        if (!CORBA::is_nil(applicationRegistrar)) {
            LOG_TRACE(Resource_impl, "Registering with application using name '" << name_binding << "'");
            // Register with the application
            applicationRegistrar->registerComponent(name_binding.c_str(), resource_obj);

            // Set up the DomainManager container
            CF::DomainManager_var domainManager = applicationRegistrar->domMgr();
            resource->setDomainManager(domainManager);

            // If it inherits from the Component class, set up the Application
            // container as well
            Component* component = dynamic_cast<Component*>(resource);
            if (component) {
                CF::Application_var application = applicationRegistrar->app();
                component->setApplication(application);
            }
        } else {
            LOG_TRACE(Resource_impl, "Binding component to naming context with name '" << name_binding << "'");
            // the registrar is not available (because the invoking infrastructure only uses the name service)
            CosNaming::NamingContext_var applicationContext = ossie::corba::_narrowSafe<CosNaming::NamingContext>(applicationRegistrarObject);
            ossie::corba::bindObjectToContext(resource_obj, applicationContext, name_binding);
        }
    }

    return resource;
}

static Resource_impl* main_component = 0;
static void sigint_handler(int signum)
{
    main_component->halt();
}

void Resource_impl::start_component(Resource_impl::ctor_type ctor, int argc, char* argv[])
{
    // Scan the arguments for NAME_BINDING, setting the thread/process name
    // based on the name. If this isn't done prior to initializing CORBA, the
    // ORB creates some threads that will get the original process name, and
    // any threads they create, and so on.
    for (size_t index = 1; index < argc; ++index) {
        if (strcmp("NAME_BINDING", argv[index]) == 0) {
            if (++index < argc) {
                std::string value = argv[index];
                value = value.substr(0, 15);
                pthread_setname_np(pthread_self(), value.c_str());
            }
            break;
        }
    }

    // The ORB must be initialized before anything that might depend on CORBA,
    // such as PropertyMap and logging configuration
    ossie::corba::CorbaInit(argc, argv);

    // Parse command line arguments.
    int debug_level = -1; // use log config uri as log level context
    std::string logcfg_uri;
    std::string dpath("");
    bool skip_run = false;
    redhawk::PropertyMap cmdlineProps;
    for (int i = 1; i < argc; i++) {
        if (strcmp("SKIP_RUN", argv[i]) == 0) {
            skip_run = true;
        } else {
            const std::string name = argv[i++];
            std::string value;
            if (i < argc) {
                value = argv[i];
            } else {
                std::cerr << "No value given for " << name << std::endl;
            }
            if (name ==  "LOGGING_CONFIG_URI") {
                logcfg_uri = value;
            } else if (name == "DEBUG_LEVEL") {
                debug_level = atoi(value.c_str());
            } else if (name == "DOM_PATH") {
                dpath = value;
            } else {  // any other argument is part of the cmdlineProps
                cmdlineProps[name] = value;
            }
        }
    }

    if (!cmdlineProps.contains("NAMING_CONTEXT_IOR")) {
        std::cout<<std::endl<<"usage: "<<argv[0]<<" [execparams]"<<std::endl<<std::endl;
        std::cout<<"The set of execparams is defined in the .prf for the component"<<std::endl;
        std::cout<<"They are provided as arguments pairs ID VALUE, for example:"<<std::endl;
        std::cout<<"     "<<argv[0]<<" INT_PARAM 5 STR_PARAM ABCDED"<<std::endl<<std::endl;
        exit(-1);
    }

    // setup logging context for a component resource
    std::string component_identifier = cmdlineProps.get("COMPONENT_IDENTIFIER", "").toString();
    std::string name_binding = cmdlineProps.get("NAME_BINDING", "").toString();
    ossie::logging::ResourceCtxPtr ctx( new ossie::logging::ComponentCtx(name_binding, component_identifier, dpath ) );

    if (!skip_run) {
        // configure the  logging library
        ossie::logging::Configure(logcfg_uri, debug_level, ctx);
    }

    // Create the servant.
    Resource_impl* resource = create_component(ctor, cmdlineProps);

    if ( !skip_run ) {
        // assign the logging context to the resource to support logging interface
        resource->saveLoggingContext( logcfg_uri, debug_level, ctx );
    }

    std::string pathAndFile = argv[0];
    unsigned lastSlash      = pathAndFile.find_last_of("/");
    std::string cwd         = pathAndFile.substr(0, lastSlash);
    resource->setCurrentWorkingDirectory(cwd);

    if (skip_run){
        return;
    }

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

    redhawk::events::Manager::Terminate();

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
