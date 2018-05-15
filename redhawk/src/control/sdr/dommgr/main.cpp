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

#include <string>
#include <cctype>
#include <stdexcept>
#include <signal.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/time.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <ossie/CF/cf.h>
#include <ossie/CorbaUtils.h>
#include <ossie/ossieSupport.h>
#include <ossie/debug.h>
#include <ossie/logging/loghelpers.h>

#include "DomainManager_impl.h"

#if ENABLE_BDB_PERSISTENCE || ENABLE_GDBM_PERSISTENCE || ENABLE_SQLITE_PERSISTENCE
#define ENABLE_PERSISTENCE
#endif

namespace fs = boost::filesystem;
using namespace std;

static DomainManager_impl* DomainManager_servant = 0;
static int received_signal = 0;

CREATE_LOGGER(DomainManager);

// There's a known bug in log4cxx where it can segfault if it is
//  used during a program exit (so don't use the logger in the signal catcher)
// System Signal Interrupt Handler will allow proper ORB shutdown
void signal_catcher( int sig )
{
    static int shuttingDown = 0;

    // IMPORTANT Don't call exit(...) in this function
    // issue all CORBA calls that you need for cleanup here before calling ORB shutdown
    if ((( sig == SIGINT ) || (sig == SIGQUIT) || (sig == SIGTERM)) && (shuttingDown == 0)) {
        received_signal = sig;
        shuttingDown = 1;
        if (DomainManager_servant) {
            DomainManager_servant->halt();
        }
    }
}

void usr_signal_catcher( int sig )
{
    if (sig == SIGUSR1) {
        if (DomainManager_servant) {
            DomainManager_servant->closeAllOpenFileHandles();
        }
    }
}

static void raise_limit(int resource, const char* name, const rlim_t DEFAULT_MAX=1024)
{
    struct rlimit limit;
    if (getrlimit(resource, &limit) == 0) {
        if (limit.rlim_max < DEFAULT_MAX) {
            std::cerr << "The current " << name << " maximum for this user is unusually low: "
                      << limit.rlim_max << ", and at least " << DEFAULT_MAX << " was expected" << std::endl;
        }
        if (limit.rlim_cur < limit.rlim_max) {
            limit.rlim_cur = limit.rlim_max;
            if (setrlimit(resource, &limit)) {
                std::cerr << "Unable to change " << name << " soft limit to the maximum allowed for this user ("
                          << limit.rlim_max << ")" << std::endl;
            }
        }
    }
}


#if _HAMMER_TEST_
static CORBA::Boolean CommFailureHandler(void* pCookie, CORBA::ULong nRetries, const CORBA::COMM_FAILURE& ex)
{
    std::cerr << std::endl << "CommFailure handler called. Minor: " << ex.minor() << "Retries = " << nRetries << std::endl << std::endl;
   return ((nRetries < 1) ? 1 : 0);
}
#endif


int old_main(int argc, char* argv[])
{
    // parse command line options
    string dmdFile("");
    string sdrRoot("");
    string logfile_uri("");
    string db_uri("");
    string domainName("");
    int debugLevel = -1;
    int initialDebugLevel = -1;
    std::string dpath("");
    std::string name_binding("DomainManager");
    bool  useLogCfgResolver = false;
    bool  bindToDomain=false;

    // If "--nopersist" is asserted, turn off persistent IORs.
    bool enablePersistence = false;
#ifdef ENABLE_PERSISTENCE
    enablePersistence = true;
#endif
    bool endPoint = false;
    
    raise_limit(RLIMIT_NPROC, "process");
    raise_limit(RLIMIT_NOFILE, "file descriptor");

    // If "--force-rebind" is asserted, this instance will replace any existing name binding
    // for the DomainManager.
    bool forceRebind = false;

    std::map<std::string, char*>execparams;

    for (int ii = 1; ii < argc; ++ii) {
        std::string param = argv[ii];
        std::string pupper = boost::algorithm::to_upper_copy(param);
        if (pupper == "USELOGCFG") {
          useLogCfgResolver=true;
          continue;
        }
        if (pupper == "BINDAPPS") {
          bindToDomain=true;
          continue;
        }
        if (++ii >= argc) {
            std::cerr << "ERROR: No value given for " << param << std::endl;
            return(EXIT_FAILURE);
        }

        if (param == "DMD_FILE") {
            dmdFile = argv[ii];
        } else if (param == "SDRROOT") {
            sdrRoot = argv[ii];
        } else if (param == "DOMAIN_NAME") {
            domainName = argv[ii];
        } else if (param == "LOGGING_CONFIG_URI") {
            logfile_uri = argv[ii];
        } else if (param == "DB_URL") {
            db_uri = argv[ii];
        } else if (param == "DEBUG_LEVEL") {
            debugLevel = atoi(argv[ii]);
            if (debugLevel > 5) {
                std::cout<<"Logging level "<<debugLevel<<" invalid. Lowering to 5"<<std::endl;
                debugLevel = 5;
            }
            initialDebugLevel = debugLevel;
        } else if (param == "PERSISTENCE") {
            string value = argv[ii];
            std::transform(value.begin(), value.begin(), value.end(), ::tolower);
#ifdef ENABLE_PERSISTENCE
            enablePersistence = (value == "true");
#endif
        } else if (param == "FORCE_REBIND") {
            string value = argv[ii];
            std::transform(value.begin(), value.begin(), value.end(), ::tolower);
            forceRebind = (value == "true");
        } else if (param == "-ORBendPoint") {
            endPoint = true;
        } else if (param == "SPD") {
            // The SPD value is ignored at present
        } else if ( ii > 0 ) { // any other argument besides the first one is part of the execparams
            execparams[param] = argv[ii];
        }
    }

    if (dmdFile.empty() || domainName.empty()) {
        std::cerr << "ERROR: DMD_FILE and DOMAIN_NAME must be provided" << std::endl;
        return(EXIT_FAILURE);
    }

#ifdef ENABLE_PERSISTENCE
    if (enablePersistence and db_uri.empty()) {
        std::cerr << "ERROR: PERSISTENCE requires DB_URI" << std::endl;
        return(EXIT_FAILURE);
    }
#endif

    // We have to have a real SDRROOT
    fs::path sdrRootPath;
    if (!sdrRoot.empty()) {
        sdrRootPath = sdrRoot;
    } else if (getenv("SDRROOT") != NULL) {
        sdrRootPath = getenv("SDRROOT");
    } else {
        // Fall back to CWD
        sdrRootPath = fs::initial_path();
    }

    // Verify the path exists
    if (!fs::is_directory(sdrRootPath)) {
        std::cerr << "ERROR: Invalid SDRROOT " << sdrRootPath << std::endl;
        return(EXIT_FAILURE);
    }

    fs::path domRootPath = sdrRootPath / "dom";
    if (!fs::is_directory(domRootPath)) {
        std::cerr << "ERROR: Invalid Domain Manager File System Root " << domRootPath << std::endl;
        return(EXIT_FAILURE);
    }

    dpath= domRootPath.string();

    std::string logname("DomainManagerLoader");
    // setup logging context for a component resource
    std::string logcfg_uri = logfile_uri;
    ossie::logging::DomainCtx *ctx_=new ossie::logging::DomainCtx( name_binding, domainName, dpath );
    ctx_->configure( logcfg_uri, debugLevel, logfile_uri );
    ossie::logging::ResourceCtxPtr ctx(ctx_);

    // configure the  logging library
    // This log statement is exempt from the "NO LOG STATEMENTS" warning below
    if ( logfile_uri == "") {
      RH_NL_INFO(logname, "Loading DEFAULT logging configuration. " );
    }
    else {
      RH_NL_INFO(logname, "Loading log configuration from uri:" << logfile_uri);
    }

#if ! defined ENABLE_PERSISTENCE
    if  (!db_uri.empty()) {
        // reset db_uri to empty... to force ignore of restore operations
        db_uri.clear();
    }
#endif

    ///////////////////////////////////////////////////////////////////////////
    // NO LOG_ STATEMENTS ABOVE THIS POINT
    ///////////////////////////////////////////////////////////////////////////

    // Create signal handler to catch system interrupts SIGINT and SIGQUIT
    struct sigaction sa;
    sa.sa_handler = signal_catcher;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    struct sigaction fp_sa;
    fp_sa.sa_handler = usr_signal_catcher;
    fp_sa.sa_flags = 0;
    sigemptyset(&fp_sa.sa_mask);

    // Associate SIGUSR1 to signal_catcher interrupt handler
    if (sigaction(SIGUSR1, &fp_sa, NULL) == -1) {
        RH_NL_ERROR(logname, "sigaction(SIGUSR1): " << strerror(errno));
        return(EXIT_FAILURE);
    }

    // Associate SIGINT to signal_catcher interrupt handler
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        RH_NL_ERROR(logname, "sigaction(SIGINT): " << strerror(errno));
        return(EXIT_FAILURE);
    }

    // Associate SIGQUIT to signal_catcher interrupt handler
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        RH_NL_ERROR(logname, "sigaction(SIGQUIT): " << strerror(errno));
        return(EXIT_FAILURE);
    }

    // Associate SIGTERM to signal_catcher interrupt handler
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        RH_NL_ERROR(logname, "sigaction(SIGTERM): " << strerror(errno));
        return(EXIT_FAILURE);
    }

    // Start CORBA. Only ask for persistence if "--nopersist" was not asserted, which implies
    // using the same port.
    ossie::corba::ORBProperties orbProperties;
    // If a user-specified ORB endpoint is present, do not override it. This
    // allows multiple persistent endpoints on the same machine.
    if (enablePersistence && !endPoint) {
        orbProperties.push_back(std::make_pair("endPoint", "giop:tcp::5678"));
    }
    // Limit the lifetime of idle client connections to 10 seconds, to avoid
    // leaving large numbers of file descriptors open. The DomainManager calls
    // initialize() and configure() on each component in an application, then
    // never talks to them again; in heavy utilization, this can cause hard-to-
    // diagnose failures.
    orbProperties.push_back(std::make_pair("outConScanPeriod", "10"));
    CORBA::ORB_ptr orb = ossie::corba::OrbInit(argc, argv, orbProperties, enablePersistence);

#if _HAMMER_TEST_
    omniORB::installCommFailureExceptionHandler(NULL, CommFailureHandler);
#endif

    PortableServer::POA_ptr root_poa = PortableServer::POA::_nil();
    try {
        // Install an adaptor to automatically create our own POAs.
        root_poa = ossie::corba::RootPOA();
    } catch ( CORBA::INITIALIZE& ex ) {
        RH_NL_FATAL(logname, "Failed to initialize the POA. Is there a Domain Manager already running?");
        return(EXIT_FAILURE);
    }
    ossie::corba::POACreator *activator_servant = new ossie::corba::POACreator();
    PortableServer::AdapterActivator_var activator = activator_servant->_this();
    root_poa->the_activator(activator);
    activator->_remove_ref();

    // Activate the root POA manager.
    PortableServer::POAManager_var mgr = root_poa->the_POAManager();
    mgr->activate();

    // Figure out what architecture we are on
    // Map i686 to SCA x86
    struct utsname un;
    if (uname(&un) != 0) {
        RH_NL_FATAL(logname, "Unable to determine system information: " << strerror(errno));
        return(EXIT_FAILURE);
    }
    if (strcmp("i686", un.machine) == 0) {
        strcpy(un.machine, "x86");
    }
    RH_NL_DEBUG(logname, "Machine " << un.machine);
    RH_NL_DEBUG(logname, "Version " << un.release);
    RH_NL_DEBUG(logname, "OS " << un.sysname);
    struct rlimit limit;
    if (getrlimit(RLIMIT_NPROC, &limit) == 0) {
        RH_NL_DEBUG(logname, "Process limit " << limit.rlim_cur);
    }
    if (getrlimit(RLIMIT_NOFILE, &limit) == 0) {
        RH_NL_DEBUG(logname, "File descriptor limit " << limit.rlim_cur);
    }

    // Create Domain Manager servant and object
    RH_NL_INFO(logname, "Starting Domain Manager");
    RH_NL_DEBUG(logname, "Root of DomainManager FileSystem set to " << domRootPath);
    RH_NL_DEBUG(logname, "DMD path set to " << dmdFile);
    RH_NL_DEBUG(logname, "Domain Name set to " << domainName);
    if ( bindToDomain ) { RH_NL_INFO(logname, "Binding applications to the domain." ); }

    try {
        DomainManager_servant = new DomainManager_impl(dmdFile.c_str(),
                                                       domRootPath.string().c_str(),
                                                       domainName.c_str(),
                                                       (db_uri.empty()) ? NULL : db_uri.c_str(),
                                                       (logfile_uri.empty()) ? NULL : logfile_uri.c_str(),
                                                       useLogCfgResolver,
                                                       bindToDomain,
                                                       enablePersistence,
                                                       initialDebugLevel
                                                       );

        // set logging level for the DomainManager's logger
        if ( DomainManager_servant ) {
          DomainManager_servant->saveLoggingContext( logfile_uri, debugLevel, ctx );
        }

    } catch (const CORBA::Exception& ex) {
        RH_NL_ERROR(logname, "Terminated with CORBA::" << ex._name() << " exception");
        return(-1);
    } catch (const std::exception& ex) {
        RH_NL_ERROR(logname, "Terminated with exception: " << ex.what());
        return(-1);
    } catch (...) {
        RH_NL_ERROR(logname, "Terminated with unknown exception");
        return(EXIT_FAILURE);
    }

    // Pass any execparams on to the DomainManager
    DomainManager_servant->setExecparamProperties(execparams);

    // If a database URI was given, attempt to restore the state. Most common errors should
    // be handled gracefully, so any exception that escapes to this level probably indicates
    // a severe problem.
    // NB: This step must occur before activating the servant. Because this is almost
    //     exclusively used with persistent IORs, a client might already hold (or be able
    //     to get) a reference to the DomainManager, but calls may fail due to the
    //     unpredictable state.
    if (!db_uri.empty()) {
        try {
            DomainManager_servant->restoreState(db_uri);
        } catch (const CORBA::Exception& ex) {
            RH_NL_FATAL(logname, "Unable to restore state: CORBA::" << ex._name());
            return(EXIT_FAILURE);
        } catch (const std::exception& ex) {
            RH_NL_FATAL(logname, "Unable to restore state: " << ex.what());
            return(EXIT_FAILURE);
        } catch (...) {
            RH_NL_FATAL(logname, "Unrecoverable error restoring state");
            return(EXIT_FAILURE);
        }
    }

    try {
        // Activate the DomainManager servant into its own POA, giving the POA responsibility
        // for its deletion.
        RH_NL_DEBUG(logname, "Activating DomainManager into POA");
        PortableServer::POA_var dommgr_poa = root_poa->find_POA("DomainManager", 1);
        PortableServer::ObjectId_var oid = ossie::corba::activatePersistentObject(dommgr_poa, DomainManager_servant, DomainManager_servant->getFullDomainManagerName());

        // Bind the DomainManager object to its full name (e.g. "DomainName/DomainName") in the NameService.
        RH_NL_DEBUG(logname, "Binding DomainManager to NamingService name " << DomainManager_servant->getFullDomainManagerName());
        CF::DomainManager_var DomainManager_obj = DomainManager_servant->_this();
        CosNaming::Name_var name = ossie::corba::stringToName(DomainManager_servant->getFullDomainManagerName());
        try {
            // Attempt to bind the name. The DomainManager_impl constructor cleans up any non-existing entries, so
            // this will only fail if there is an active object already bound to the name.
            ossie::corba::InitialNamingContext()->bind(name, DomainManager_obj);
        } catch (const CosNaming::NamingContext::AlreadyBound&) {
            if (forceRebind) {
                // Forcibly replace the existing name binding.
                RH_NL_INFO(logname, "Replacing existing name binding " << DomainManager_servant->getFullDomainManagerName());
                ossie::corba::InitialNamingContext()->rebind(name, DomainManager_obj);
            } else {
                RH_NL_FATAL(logname, "A DomainManager is already running as " << DomainManager_servant->getFullDomainManagerName());
                return(-1);
            }
        }

        CORBA::String_var ior_str = orb->object_to_string(DomainManager_obj);
        RH_NL_DEBUG(logname, ior_str);

        DomainManager_servant->_remove_ref();

        RH_NL_INFO(logname, "Starting ORB!");
        DomainManager_servant->run();

        RH_NL_DEBUG(logname, "Shutting down DomainManager");
        DomainManager_servant->shutdown(received_signal);

        RH_NL_INFO(logname, "Requesting ORB shutdown");
        ossie::corba::OrbShutdown(true);
        RH_NL_INFO(logname, "Farewell!");
        ossie::logging::Terminate();            //no more logging....
    } catch (const CORBA::Exception& ex) {
        RH_NL_FATAL(logname, "Terminated with CORBA::" << ex._name() << " exception");
        return(-1);
    } catch (const std::exception& ex) {
        RH_NL_FATAL(logname, "Terminated with exception: " << ex.what());
        return(-1);
    } catch (...) {
        RH_NL_FATAL(logname, "Terminated with unknown exception");
        DomainManager_servant->shutdown(-1);
        
        RH_NL_INFO(logname, "ORB shutdown.... short startup..");
        ossie::corba::OrbShutdown(true);
        return(-1);
    }

    return 0;
}

int main(int argc, char* argv[]) {
  int status = old_main(argc, argv);
  if ( status < 0 ) ossie::logging::Terminate(); 
  exit(status);
}
