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

#include <iostream>
#include <string>
#include <cctype>
#include <exception>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/signalfd.h>

#include <sys/resource.h>
#include <sys/time.h>

#include <boost/algorithm/string.hpp>
#include <boost/asio/ip/host_name.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/thread.hpp>

#include <ossie/CF/cf.h>
#include <ossie/CorbaUtils.h>
#include <ossie/ossieSupport.h>
#include <ossie/debug.h>
#include <ossie/logging/loghelpers.h>
#include <ossie/SimpleThread.h>

#include "DeviceManager_impl.h"

namespace fs = boost::filesystem;

static DeviceManager_impl* DeviceManager_servant = 0;
static bool internalShutdown_devMgr;
static int sig_fd=-1;


CREATE_LOGGER(DeviceManager);




// There's a known bug in log4cxx where it can segfault if it is
//  used during a program exit (so don't use the logger in the signal catcher)
static void shutdown (void)
{
    static bool shuttingDown = false;
    if (shuttingDown) {
        return;
    }

    shuttingDown = true;
    if (DeviceManager_servant) {
        if (!internalShutdown_devMgr) {
            DeviceManager_servant->shutdown();
        }
    }
}


static void child_exit (int sig)
{
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
      if (  DeviceManager_servant) {
        DeviceManager_servant->childExited(pid, status);
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


/*
  sigprocessor
  
  Called from controlling SimpleThread object that will check the file descriptor created by
  signalfd. Each iteration will check the file descriptor for watched signals that are blocked.
  This allows for io operations

*/

int sigprocessor(void ) {

  // Check if any children died....
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(sig_fd, &readfds);
  struct timeval tv = {0, 50};
  int retval=SimpleThread::NOOP;

  std::string logname("DeviceManagerLoader");

  if ( sig_fd > -1 ) {

    // don't care about writefds and exceptfds:
    select(sig_fd+1, &readfds, NULL, NULL, &tv);
    if (FD_ISSET(sig_fd, &readfds)) {

      retval=SimpleThread::NORMAL;
      struct signalfd_siginfo si;
      ssize_t s = read(sig_fd, &si, sizeof(struct signalfd_siginfo));
      if (s != sizeof(struct signalfd_siginfo)){
        RH_NL_ERROR(logname, "SIGCHLD handling error ...");
      }
 
      // check for SIGCHLD
      if ( si.ssi_signo == SIGCHLD) {
          RH_NL_TRACE(logname, "SignalChild is active....pid:." << si.ssi_pid);
          // Only concerned with children that exited; the status will be reported by
          // the DeviceManager's child handler
          switch (si.ssi_code) {
          case CLD_EXITED:
          case CLD_KILLED:
          case CLD_DUMPED:
              child_exit(si.ssi_signo);
              break;
          }
      }

      // check if we need to exit...
      if ( si.ssi_signo == SIGINT ||si.ssi_signo == SIGQUIT || 
           si.ssi_signo == SIGTERM ) {
        RH_NL_INFO(logname, "DeviceManager received signal (INT or QUIT or TERM) for proces: " << si.ssi_pid);
        shutdown();
      }
    }

    // need to handle last child died was captured..so we can release control back to main
    if (DeviceManager_servant) {
      if (internalShutdown_devMgr &&  
          DeviceManager_servant->allChildrenExited() &&
          DeviceManager_servant->isShutdown() ) {
        RH_NL_DEBUG(logname, "Release the ORB, control back to main.cpp" );
        // devmgr is done with using orb.... release orb so control goes back to main
        ossie::corba::OrbShutdown(false);
        retval=SimpleThread::FINISH;   // stop us too
      }
    }

  }

  return retval;
}


int main(int argc, char* argv[])
{
    // parse command line options
    std::string dcdFile;
    std::string sdrRoot;
    std::string spdFile;
    std::string sdrCache;
    std::string logfile_uri;
    std::string domainName;
    int debugLevel = -1;
    int initialDebugLevel = -1;
    std::string dpath("");
    std::string cpuBlackList("");
    std::string node_name("DEVICE_MANAGER");
    bool        useLogCfgResolver=false;

    std::map<std::string, char*>execparams;

    raise_limit(RLIMIT_NPROC, "process");
    raise_limit(RLIMIT_NOFILE, "file descriptor");

    std::string logname("DeviceManagerLoader");

    for (int ii = 1; ii < argc; ++ii) {
        std::string param = argv[ii];
        std::string pupper = boost::algorithm::to_upper_copy(param);
        if (pupper == "USELOGCFG") {
          useLogCfgResolver=true;
          continue;
        }
        if (++ii >= argc) {
            std::cerr << "ERROR: No value given for " << param << std::endl;
            exit(EXIT_FAILURE);
        }

        if (param == "DCD_FILE") {
            dcdFile = argv[ii];
        } else if (param == "SDRROOT") {
            sdrRoot = argv[ii];
        } else if (param == "SPD") {
            spdFile = argv[ii];
        } else if (param == "SDRCACHE") {
            sdrCache = argv[ii];
        } else if (param == "DOMAIN_NAME") {
            domainName = argv[ii];
        } else if (param == "LOGGING_CONFIG_URI") {
            logfile_uri = argv[ii];
        } else if (pupper == "NOLOGCFG") {
          useLogCfgResolver=false;
        } else if (pupper == "CPUBLACKLIST") {
          cpuBlackList=argv[ii];
        } else if (param == "DEBUG_LEVEL") {
            debugLevel = atoi(argv[ii]);
            if (debugLevel > 5) {
                std::cout<<"Logging level "<<debugLevel<<" invalid. Lowering to 5"<<std::endl;
                debugLevel = 5;
            }
            initialDebugLevel = debugLevel;
        } else if (ii > 0 ) {
            execparams[param] = argv[ii];
        }
    }

    if (dcdFile.empty() || domainName.empty()) {
        std::cerr << "ERROR: DCD_FILE and DOMAIN_NAME must be provided" << std::endl;
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

    fs::path devRootPath = sdrRootPath / "dev";
    if (!fs::is_directory(devRootPath)) {
        std::cerr << "ERROR: Invalid Device Manager File System Root " << devRootPath << std::endl;
        exit(EXIT_FAILURE);
    }


    fs::path dcdPath = devRootPath / dcdFile;
    std::ifstream dcdStream(dcdPath.string().c_str());
    if (!dcdStream) {
        std::cerr << "Could not read DCD file " << dcdFile << std::endl;
        exit(EXIT_FAILURE);
    }
    ossie::DeviceManagerConfiguration dcd;
    try {
        dcd.load(dcdStream);
    } catch (const ossie::parser_error& ex) {
        std::cerr << "Failed to parse DCD file " << dcdFile << ". The XML parser returned the following error: " << ex.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    dcdStream.close();
    
    node_name = dcd.getName();

    std::ostringstream os;
    os << domainName << "/" << node_name;
    dpath= os.str();
    
    //
    // setup logging context for this DeviceManager
    //
    ossie::logging::ResourceCtxPtr ctx( new ossie::logging::DeviceMgrCtx(node_name, domainName, dpath ) );

    std::string logcfg_uri=logfile_uri;
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
	  std::string fpath((char*)fs::path(devRootPath / path).string().c_str());
	  logcfg_uri = "file://" + fpath;
        }
    }


    //
    // apply logging settings to the library
    //
    ossie::logging::Configure(logcfg_uri, debugLevel, ctx);
    execparams["LOGGING_CONFIG_URI"] = const_cast<char *>(logfile_uri.c_str());

    ///////////////////////////////////////////////////////////////////////////
    // NO LOG_ STATEMENTS ABOVE THIS POINT
    ///////////////////////////////////////////////////////////////////////////

    // Create signal handler to catch system interrupts SIGINT and SIGQUIT
    // Install signal handler to properly handle SIGCHLD signals
    // we are using signalfd to remove io restriction when handling signals
    int err;
    sigset_t  sigset;
    err=sigemptyset(&sigset);
    err = sigaddset(&sigset, SIGINT);
    if ( err ) {
      RH_NL_ERROR(logname, "sigaction(SIGINT): " << strerror(errno));
      exit(EXIT_FAILURE);
    }

    err = sigaddset(&sigset, SIGQUIT);
    if ( err ) {
      RH_NL_ERROR(logname, "sigaction(SIGQUIT): " << strerror(errno));
      exit(EXIT_FAILURE);
    }

    err = sigaddset(&sigset, SIGTERM);
    if ( err ) {
      RH_NL_ERROR(logname, "sigaction(SIGTERM): " << strerror(errno));
      exit(EXIT_FAILURE);
    }
    err = sigaddset(&sigset, SIGCHLD);
    if ( err ) {
      RH_NL_ERROR(logname, "sigaction(SIGCHLD): " << strerror(errno));
      exit(EXIT_FAILURE);
    }

    // We must block the signals in order for signalfd to receive them 
    err = sigprocmask(SIG_BLOCK, &sigset, NULL);
    // Create the signalfd
    sig_fd = signalfd(-1, &sigset, SFD_NONBLOCK | SFD_CLOEXEC);
    if ( sig_fd == -1 ) {
      RH_NL_ERROR(logname, "signalfd failed: " << strerror(errno));
      exit(EXIT_FAILURE);
    }

    // Start CORBA. Persistence is not currently supported in the DeviceManager.
    CORBA::ORB_ptr orb = ossie::corba::OrbInit(argc, argv, false);

    // Temporarily deactivate the root POA manager.
    PortableServer::POA_var root_poa = ossie::corba::RootPOA();
    PortableServer::POAManager_var mgr = root_poa->the_POAManager();
    mgr->hold_requests(1);

    // Install our own adapter to create POAs as needed.
    ossie::corba::POACreator *activator_servant = new ossie::corba::POACreator();
    PortableServer::AdapterActivator_var activator = activator_servant->_this();
    root_poa->the_activator(activator);
    activator->_remove_ref();

    // Re-activate the root POA manager.
    mgr->activate();

    // Figure out what architecture we are on
    // Map i686 to SCA x86
    struct utsname un;
    if (uname(&un) != 0) {
        RH_NL_ERROR(logname, "Unable to determine system information: " << strerror(errno));
        exit (0);
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

    // Locate the physical location for the Device Manager's cache.
    std::string devMgrCache;
    if (!sdrCache.empty()) {
        // Get location from command line
        if (sdrCache[sdrCache.length()-1] == '/') {
            devMgrCache = sdrCache.substr(0, sdrCache.length() - 1);
        } else {
            devMgrCache = sdrCache;
        }
    } else if (getenv("SDRCACHE") != NULL) {
        // Get it from the env variable second.
        std::string sdrCacheEnv = getenv("SDRCACHE");

        if (sdrCacheEnv[sdrCacheEnv.length()-1] == '/') {
            devMgrCache = sdrCacheEnv.substr(0, sdrCacheEnv.length() - 1);
        } else {
            devMgrCache = sdrCacheEnv;
        }
    } else {
        // Get relative to fsDevRoot.
        devMgrCache = devRootPath.string();
    }

    RH_NL_INFO(logname, "Starting Device Manager with " << dcdFile);
    RH_NL_DEBUG(logname, "Root of DeviceManager FileSystem set to " << devRootPath);
    RH_NL_DEBUG(logname, "DevMgr cache set to " << devMgrCache);
    RH_NL_DEBUG(logname, "Domain Name set to " << domainName);

    SimpleThread sigthread( sigprocessor );
    int pstage=-1;
    try {
      try {
        DeviceManager_servant = new DeviceManager_impl(dcdFile.c_str(),
                                                       devRootPath.string().c_str(),
                                                       devMgrCache.c_str(),
                                                       (logfile_uri.empty()) ? NULL : logfile_uri.c_str(),
                                                       un,
                                                       useLogCfgResolver,
                                                       cpuBlackList.c_str(),
                                                       &internalShutdown_devMgr,
                                                       spdFile,
                                                       initialDebugLevel
                                                       );
        DeviceManager_servant->saveLoggingContext(logfile_uri, initialDebugLevel, ctx);

        DeviceManager_servant->setExecparamProperties(execparams);
        pstage=0;

        // start signal catching thread
        sigthread.start();

        // Activate the DeviceManager servant into its own POA, giving the POA responsibility
        // for its deletion.
        PortableServer::POA_var devmgr_poa = root_poa->find_POA("DeviceManager", 1);
        PortableServer::ObjectId_var oid = devmgr_poa->activate_object(DeviceManager_servant);
        
        
        
        // finish initializing the Device Manager
        try {
          pstage++;
          DeviceManager_servant->postConstructor(domainName.c_str());
        } catch (const CORBA::Exception& ex) {
          RH_NL_FATAL(logname, "Startup failed with CORBA::" << ex._name() << " exception");
          shutdown();
          throw;
        } catch (const std::runtime_error& e) {
          RH_NL_FATAL(logname, "Startup failed: " << e.what() );
          shutdown();
          throw;
        } catch (...) {
          RH_NL_FATAL(logname, "Startup failed; unknown exception");
          shutdown();
          throw;
        }

        pstage++;
        RH_NL_INFO(logname, "Starting ORB!");
        orb->run();

        pstage++;
        RH_NL_INFO(logname, "Goodbye!");
      } catch (const CORBA::Exception& ex) {
        RH_NL_ERROR(logname, "Terminated with CORBA::" << ex._name() << " exception");
        throw;
 
      } catch (const std::exception& ex) {
        RH_NL_ERROR(logname, "Terminated with exception: " << ex.what());
        throw;
      }
    }catch(...) {
    }
    
    // check if we ran.... this should 
    if ( pstage > 1 ) {
      while (not DeviceManager_servant->allChildrenExited()) {
        printf("waiting for child processes to stop \n");
        usleep(10000);  // sleep 10 ms to wait for all children to exit
      }
    }

    sigthread.stop();

    if ( pstage > 0 ) {
        int refcnt= DeviceManager_servant->_refcount_value();
        // we are done, remove extra references, poa holds 1
        while ( refcnt > 1 ) {
            DeviceManager_servant->_refcount_value();
            refcnt--;
        }
    }
    else {
      delete DeviceManager_servant;
      DeviceManager_servant = 0;
    }

    RH_NL_DEBUG(logname, "Farewell!")
    ossie::corba::OrbShutdown(true);
    ossie::logging::Terminate();

    exit( pstage>1?0:-1);
}
