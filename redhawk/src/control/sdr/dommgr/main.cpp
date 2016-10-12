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

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#include <ossie/CF/cf.h>
#include <ossie/CorbaUtils.h>
#include <ossie/ossieSupport.h>
#include <ossie/debug.h>
#include <ossie/logging/loghelpers.h>

#include "DomainManager_impl.h"

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

int main(int argc, char* argv[])
{
    // parse command line options
    string dmdFile("");
    string sdrRoot("");
    string logfile_uri("");
    string db_uri("");
    string domainName("");
    int debugLevel = 3;
    std::string dpath("");
    std::string name_binding("DomainManager");

    // If "--nopersist" is asserted, turn off persistent IORs.
    bool enablePersistence = false;
#if ENABLE_BDB_PERSISTENCE || ENABLE_GDBM_PERSISTENCE || ENABLE_SQLITE_PERSISTENCE
    enablePersistence = true;
#endif
    
    raise_limit(RLIMIT_NPROC, "process");
    raise_limit(RLIMIT_NOFILE, "file descriptor");

    // If "--force-rebind" is asserted, this instance will replace any existing name binding
    // for the DomainManager.
    bool forceRebind = false;

    std::map<std::string, char*>execparams;

    for (int ii = 1; ii < argc; ++ii) {
        std::string param = argv[ii];
        if (++ii >= argc) {
            std::cerr << "ERROR: No value given for " << param << std::endl;
            exit(EXIT_FAILURE);
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
        } else if (param == "PERSISTENCE") {
            string value = argv[ii];
            std::transform(value.begin(), value.begin(), value.end(), ::tolower);
#if ENABLE_BDB_PERSISTENCE || ENABLE_GDBM_PERSISTENCE || ENABLE_SQLITE_PERSISTENCE
            enablePersistence = (value == "true");
#endif
        } else if (param == "FORCE_REBIND") {
            string value = argv[ii];
            std::transform(value.begin(), value.begin(), value.end(), ::tolower);
            forceRebind = (value == "true");
        } else if (param == "-ORBendPoint") {
        } else if ( ii > 0 ) { // any other argument besides the first one is part of the execparams
            execparams[param] = argv[ii];
        }
    }

    if (dmdFile.empty() || domainName.empty()) {
        std::cerr << "ERROR: DMD_FILE and DOMAIN_NAME must be provided" << std::endl;
        exit(EXIT_FAILURE);
    }

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
        exit(EXIT_FAILURE);
    }

    fs::path domRootPath = sdrRootPath / "dom";
    if (!fs::is_directory(domRootPath)) {
        std::cerr << "ERROR: Invalid Domain Manager File System Root " << domRootPath << std::endl;
        exit(EXIT_FAILURE);
    }

    std::ostringstream os;
    os << domainName << "/" << domainName;
    dpath= os.str();

    // setup logging context for a component resource
    ossie::logging::ResourceCtxPtr ctx( new ossie::logging::DomainCtx(name_binding, domainName, dpath ) );

    std::string logcfg_uri = logfile_uri;
    if ( !logfile_uri.empty() ) {
        // Determine the scheme, if any.  This isn't a full fledged URI parser so we can
        // get tripped up on complex URIs.  We should probably incorporate a URI parser
        // library for this sooner rather than later
        std::string scheme;
        fs::path path;

        std::string::size_type colonIdx = logfile_uri.find(":"); // Find the scheme separator
        if (colonIdx == std::string::npos) {

            scheme = "file";
            path = logfile_uri;
            // Make the path absolute
            fs::path logfile_path(path);
            if (! logfile_path.is_complete()) {
                // Get the root path so we can resolve relative paths
                fs::path root = fs::initial_path();
                logfile_path = fs::path(root / path);
            }
            path = logfile_path;
            logfile_uri = "file://" + path.string();

        } else {

            scheme = logfile_uri.substr(0, colonIdx);
            colonIdx += 1;
            if ((logfile_uri.at(colonIdx + 1) == '/') && (logfile_uri.at(colonIdx + 2) == '/')) {
                colonIdx += 2;
            }
            path = logfile_uri.substr(colonIdx, logfile_uri.length() - colonIdx);
	}

        if (scheme == "file") {
	  std::string fpath((char*)path.string().c_str());
	  logcfg_uri = "file://" + fpath;	  
	}
	if (scheme == "sca") {
	  std::string fpath((char*)fs::path(domRootPath / path).string().c_str());
	  logcfg_uri = "file://" + fpath;
        }
    }

    // configure the  logging library
    ossie::logging::Configure(logcfg_uri, debugLevel, ctx);
    // This log statement is exempt from the "NO LOG STATEMENTS" warning below
    if ( logfile_uri == "") {
      LOG_INFO(DomainManager, "Loading DEFAULT logging configuration. " );
    }
    else {
      LOG_INFO(DomainManager, "Loading log configuration from uri:" << logfile_uri);
    }

#if 0 
    // test logger configuration....
    LOG_FATAL(DomainManager, "FATAL MESSAGE " );
    LOG_ERROR(DomainManager, "ERROR MESSAGE " );
    LOG_WARN(DomainManager, "WARN MESSAGE " );
    LOG_INFO(DomainManager, "INFO MESSAGE " );
    LOG_DEBUG(DomainManager, "DEBUG MESSAGE " );
    LOG_TRACE(DomainManager, "TRACE MESSAGE " );
    std::cout << " END OF TEST LOGGER MESSAGES " << std::endl;
#endif

    ///////////////////////////////////////////////////////////////////////////
    // NO LOG_ STATEMENTS ABOVE THIS POINT
    ///////////////////////////////////////////////////////////////////////////

    // Create signal handler to catch system interrupts SIGINT and SIGQUIT
    struct sigaction sa;
    sa.sa_handler = signal_catcher;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    // Associate SIGINT to signal_catcher interrupt handler
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        LOG_ERROR(DomainManager, "sigaction(SIGINT): " << strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Associate SIGQUIT to signal_catcher interrupt handler
    if (sigaction(SIGQUIT, &sa, NULL) == -1) {
        LOG_ERROR(DomainManager, "sigaction(SIGQUIT): " << strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Associate SIGTERM to signal_catcher interrupt handler
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        LOG_ERROR(DomainManager, "sigaction(SIGTERM): " << strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Start CORBA. Only ask for persistence if "--nopersist" was not asserted, which implies
    // using the same port.
    CORBA::ORB_ptr orb = ossie::corba::OrbInit(argc, argv, enablePersistence);

    PortableServer::POA_ptr root_poa = PortableServer::POA::_nil();
    try {
        // Install an adaptor to automatically create our own POAs.
        root_poa = ossie::corba::RootPOA();
    } catch ( CORBA::INITIALIZE& ex ) {
        LOG_FATAL(DomainManager, "Failed to initialize the POA. Is there a Domain Manager already running?");
        exit(EXIT_FAILURE);
    }
    ossie::corba::POACreator activator_servant;
    PortableServer::AdapterActivator_var activator = activator_servant._this();
    root_poa->the_activator(activator);

    // Activate the root POA manager.
    PortableServer::POAManager_var mgr = root_poa->the_POAManager();
    mgr->activate();

    // Figure out what architecture we are on
    // Map i686 to SCA x86
    struct utsname un;
    if (uname(&un) != 0) {
        LOG_FATAL(DomainManager, "Unable to determine system information: " << strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (strcmp("i686", un.machine) == 0) {
        strcpy(un.machine, "x86");
    }
    LOG_DEBUG(DomainManager, "Machine " << un.machine);
    LOG_DEBUG(DomainManager, "Version " << un.release);
    LOG_DEBUG(DomainManager, "OS " << un.sysname);
    struct rlimit limit;
    if (getrlimit(RLIMIT_NPROC, &limit) == 0) {
        LOG_DEBUG(DomainManager, "Process limit " << limit.rlim_cur);
    }
    if (getrlimit(RLIMIT_NOFILE, &limit) == 0) {
        LOG_DEBUG(DomainManager, "File descriptor limit " << limit.rlim_cur);
    }

    try {
        // Create Domain Manager servant and object
        LOG_INFO(DomainManager, "Starting Domain Manager");
        LOG_DEBUG(DomainManager, "Root of DomainManager FileSystem set to " << domRootPath);
        LOG_DEBUG(DomainManager, "DMD path set to " << dmdFile);
        LOG_DEBUG(DomainManager, "Domain Name set to " << domainName);
        try {
            DomainManager_servant = new DomainManager_impl(dmdFile.c_str(),
                                                           domRootPath.string().c_str(),
                                                           domainName.c_str(),
                                                           (logfile_uri.empty()) ? NULL : logfile_uri.c_str(),
                                                           (db_uri.empty()) ? NULL : db_uri.c_str()
                                                           );
            DomainManager_servant->setExecparamProperties(execparams);

        } catch (const CORBA::Exception& ex) {
            LOG_ERROR(DomainManager, "Terminated with CORBA::" << ex._name() << " exception");
            exit(-1);
        } catch (const std::exception& ex) {
            LOG_ERROR(DomainManager, "Terminated with exception: " << ex.what());
            exit(-1);
        } catch( ... ) {
            LOG_ERROR(DomainManager, "Terminated with unknown exception");
            exit(EXIT_FAILURE);
        }

        // Activate the DomainManager servant into its own POA, giving the POA responsibility
        // for its deletion.
        LOG_DEBUG(DomainManager, "Activating DomainManager into POA");
        PortableServer::POA_var dommgr_poa = root_poa->find_POA("DomainManager", 1);
        PortableServer::ObjectId_var oid = ossie::corba::activatePersistentObject(dommgr_poa, DomainManager_servant, DomainManager_servant->getFullDomainManagerName());

        // If a database URI was given, attempt to restore the state. Most common errors should
        // be handled gracefully, so any exception that escapes to this level probably indicates
        // a severe problem.
        // TODO: Determine whether this step can occur before activating the servant. Because this
        // is almost exclusively used with persistent IORs, a client might already hold (or be able
        // to get) a reference to the DomainManager, but calls may fail due to the unpredictable
        // state.
        if (!db_uri.empty()) {
            try {
                DomainManager_servant->restoreState(db_uri);
            } CATCH_RETHROW_LOG_ERROR(DomainManager, "Unrecoverable error restoring state");
        }

        // Bind the DomainManager object to its full name (e.g. "DomainName/DomainName") in the NameService.
        LOG_DEBUG(DomainManager, "Binding DomainManager to NamingService name " << DomainManager_servant->getFullDomainManagerName());
        CF::DomainManager_var DomainManager_obj = DomainManager_servant->_this();
        CosNaming::Name_var name = ossie::corba::stringToName(DomainManager_servant->getFullDomainManagerName());
        try {
            // Attempt to bind the name. The DomainManager_impl constructor cleans up any non-existing entries, so
            // this will only fail if there is an active object already bound to the name.
            ossie::corba::InitialNamingContext()->bind(name, DomainManager_obj);
        } catch (const CosNaming::NamingContext::AlreadyBound&) {
            if (forceRebind) {
                // Forcibly replace the existing name binding.
                LOG_INFO(DomainManager, "Replacing existing name binding " << DomainManager_servant->getFullDomainManagerName());
                ossie::corba::InitialNamingContext()->rebind(name, DomainManager_obj);
            } else {
                LOG_FATAL(DomainManager, "A DomainManager is already running as " << DomainManager_servant->getFullDomainManagerName());
                exit(-1);
            }
        }

        CORBA::String_var ior_str = orb->object_to_string(DomainManager_obj);
        LOG_DEBUG(DomainManager, ior_str);

        DomainManager_servant->_remove_ref();

        LOG_INFO(DomainManager, "Starting ORB!");
        DomainManager_servant->run();

        LOG_DEBUG(DomainManager, "Shutting down DomainManager");
        DomainManager_servant->shutdown(received_signal);

        LOG_INFO(DomainManager, "Requesting ORB shutdown");
        ossie::corba::OrbShutdown(true);

        LOG_INFO(DomainManager, "Goodbye!");
    } catch (const CORBA::Exception& ex) {
        LOG_FATAL(DomainManager, "Terminated with CORBA::" << ex._name() << " exception");
        exit(-1);
    } catch (const std::exception& ex) {
        LOG_FATAL(DomainManager, "Terminated with exception: " << ex.what());
        exit(-1);
    } catch (...) {
        LOG_FATAL(DomainManager, "Terminated with unknown exception");
        exit(-1);
    }
    LOG_DEBUG(DomainManager, "Farewell!")
}
