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
#include <fstream>
#include <vector>
#include <algorithm>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <errno.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <sys/time.h>
#include <libgen.h>

#include "ossie/ExecutableDevice_impl.h"
#include "ossie/prop_helpers.h"
#include "ossie/affinity.h"
#include "logging/rh_logger_stdout.h"

PREPARE_CF_LOGGING(ExecutableDevice_impl)

/* ExecutableDevice_impl ****************************************
    - constructor 1: no capacities defined
****************************************************************** */
ExecutableDevice_impl::ExecutableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl):
    LoadableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl)
{
}



/* ExecutableDevice_impl ************************************************
    - constructor 2: capacities defined
******************************************************************** */
ExecutableDevice_impl::ExecutableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl,
                                              CF::Properties capacities):
    LoadableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl, capacities)
{
}

/* ExecutableDevice_impl ****************************************
    - constructor 1: no capacities defined
****************************************************************** */
ExecutableDevice_impl::ExecutableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl, 
                                              char* composite_ior):
    LoadableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl, composite_ior)
{
}


/* ExecutableDevice_impl ************************************************
    - constructor 2: capacities defined
******************************************************************** */
ExecutableDevice_impl::ExecutableDevice_impl (char* devMgr_ior, char* id, char* lbl, char* sftwrPrfl,
                                              CF::Properties capacities, char* composite_ior):
    LoadableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl, capacities, composite_ior)
{
}

std::string ExecutableDevice_impl::component_name_from_profile_name(const std::string& profile_name)
{
    std::string component_name = profile_name;

    // Strip ".spd.xml" from end of profile_name
    std::size_t pos = component_name.find(".spd.xml");
    if( pos != std::string::npos )
    {
        component_name = component_name.substr( 0, pos );
    }

    // Strip directory path
    pos = component_name.find_last_of( "/" );
    if( pos != std::string::npos )
    {
        component_name = component_name.substr( pos+1 );
    }

    return component_name;
}

std::string ExecutableDevice_impl::get_component_name_from_exec_params(const CF::Properties& parameters)
{
    for (CORBA::ULong i = 0; i < parameters.length(); ++i) {
        if (ossie::corba::returnString(parameters[i].id) == std::string("PROFILE_NAME"))
            return component_name_from_profile_name( ossie::any_to_string(parameters[i].value) );
    }
    
    LOG_ERROR(ExecutableDevice_impl, __FUNCTION__ << ": Could not extract component name from exec params" );
    throw CF::ExecutableDevice::InvalidParameters(parameters);
}

CF::ExecutableDevice::ProcessID_Type ExecutableDevice_impl::executeLinked (const char* name, const CF::Properties& options, const CF::Properties& parameters, const CF::StringSequence& deps) throw (CORBA::SystemException, CF::Device::InvalidState, CF::ExecutableDevice::InvalidFunction, CF::ExecutableDevice::InvalidParameters, CF::ExecutableDevice::InvalidOptions, CF::InvalidFileName, CF::ExecutableDevice::ExecuteFail)
{
    boost::recursive_mutex::scoped_lock lock;
    try
    {
        lock = boost::recursive_mutex::scoped_lock(load_execute_lock);
    }
    catch( const boost::thread_resource_error& e )
    {
        std::stringstream errstr;
        errstr << "Error acquiring lock (errno=" << e.native_error() << " msg=\"" << e.what() << "\")";
        LOG_ERROR(ExecutableDevice_impl, __FUNCTION__ << ": " << errstr.str() );
        throw CF::Device::InvalidState(errstr.str().c_str());
    }
    
    boost::shared_ptr<envState> initial_env(new envState());
    this->initialState.set();
    
    std::vector<sharedLibraryStorage> selected_paths;
    for (unsigned int i=0; i<deps.length(); i++) {
        std::string dep = ossie::corba::returnString(deps[i]);
        if (this->sharedPkgs.find(dep) == this->sharedPkgs.end()) {
            // it is not a loaded package
        } else {
            selected_paths.push_back(this->sharedPkgs[dep]);
        }
    }
    update_selected_paths(selected_paths);

    std::vector<std::string> prepend_args;
    CF::ExecutableDevice::ProcessID_Type pid = execute(name, options, parameters);
    return pid;
}

CF::ExecutableDevice::ProcessID_Type ExecutableDevice_impl::execute (const char* name, const CF::Properties& options, const CF::Properties& parameters) throw (CORBA::SystemException, CF::Device::InvalidState, CF::ExecutableDevice::InvalidFunction, CF::ExecutableDevice::InvalidParameters, CF::ExecutableDevice::InvalidOptions, CF::InvalidFileName, CF::ExecutableDevice::ExecuteFail)
{
    boost::recursive_mutex::scoped_lock lock;
    try
    {
        lock = boost::recursive_mutex::scoped_lock(load_execute_lock);
    }
    catch( const boost::thread_resource_error& e )
    {
        std::stringstream errstr;
        errstr << "Error acquiring lock (errno=" << e.native_error() << " msg=\"" << e.what() << "\")";
        LOG_ERROR(ExecutableDevice_impl, __FUNCTION__ << ": " << errstr.str() );
        throw CF::Device::InvalidState(errstr.str().c_str());
    }
    
    std::vector<std::string> prepend_args;
    CF::ExecutableDevice::ProcessID_Type pid = do_execute(name, options, parameters, prepend_args);
    return pid;
}



void ExecutableDevice_impl::set_resource_affinity( const CF::Properties& options, const pid_t rsc_pid, const char*rsc_name, const redhawk::affinity::CpuList &blacklist ) {

   //
   // check if affinity namespaced options exists...
   //   
   try {
     if ( redhawk::affinity::has_affinity( options ) ) {
         LOG_DEBUG(ExecutableDevice_impl, "Has Affinity....ExecDevice/Resource:" << label() << "/" << rsc_name);
       if ( redhawk::affinity::is_disabled() ) {
         LOG_WARN(ExecutableDevice_impl, "Resource has affinity directives but processing disabled, ExecDevice/Resource:" << 
                  label() << "/" << rsc_name);
       }
       else {
         LOG_DEBUG(ExecutableDevice_impl, "Calling set resource affinity....ExecDevice/Resource:" <<
                  label() << "/" << rsc_name);
         redhawk::affinity::set_affinity( options, rsc_pid, blacklist );
       }
     }
     else {
         LOG_TRACE(ExecutableDevice_impl, "No Affinity Found....ExecDevice/Resource:" << label() << "/" << rsc_name);
     }
   }
   catch( redhawk::affinity::AffinityFailed &e) {
     LOG_WARN(ExecutableDevice_impl, "AFFINITY REQUEST FAILED: " << e.what() );
     throw;
   }


}

/* execute *****************************************************************
    - executes a process on the device
************************************************************************* */
CF::ExecutableDevice::ProcessID_Type ExecutableDevice_impl::do_execute (const char* name, const CF::Properties& options, const CF::Properties& parameters, const std::vector<std::string> prepend_args) throw (CORBA::SystemException, CF::Device::InvalidState, CF::ExecutableDevice::InvalidFunction, CF::ExecutableDevice::InvalidParameters, CF::ExecutableDevice::InvalidOptions, CF::InvalidFileName, CF::ExecutableDevice::ExecuteFail)
{
    CF::Properties invalidOptions;
    std::string path;
    char* tmp;

    // throw and error if name does not begin with a /
    if (strncmp(name, "/", 1) != 0)
        throw CF::InvalidFileName(CF::CF_EINVAL, "Filename must be absolute");
    if (isLocked())
        throw CF::Device::InvalidState("System is locked down");
    if (isDisabled())
        throw CF::Device::InvalidState("System is disabled");

    //process options and throw InvalidOptions errors if they are not ULong
    for (CORBA::ULong i = 0; i < options.length(); ++i) {
        if (options[i].id == CF::ExecutableDevice::PRIORITY_ID) {
            CORBA::TypeCode_var atype = options[i].value.type();
            if (atype->kind() != CORBA::tk_ulong) {
                invalidOptions.length(invalidOptions.length() + 1);
                invalidOptions[invalidOptions.length() - 1].id = options[i].id;
                invalidOptions[invalidOptions.length() - 1].value
                        = options[i].value;
            } else
                LOG_WARN(ExecutableDevice_impl, "Received a PRIORITY_ID execute option...ignoring.")
            }
        if (options[i].id == CF::ExecutableDevice::STACK_SIZE_ID) {
            CORBA::TypeCode_var atype = options[i].value.type();
            if (atype->kind() != CORBA::tk_ulong) {
                invalidOptions.length(invalidOptions.length() + 1);
                invalidOptions[invalidOptions.length() - 1].id = options[i].id;
                invalidOptions[invalidOptions.length() - 1].value
                        = options[i].value;
            } else
                LOG_WARN(ExecutableDevice_impl, "Received a STACK_SIZE_ID execute option...ignoring.")
            }
    }

    if (invalidOptions.length() > 0) {
        throw CF::ExecutableDevice::InvalidOptions(invalidOptions);
    }

    // retrieve current working directory
    tmp = getcwd(NULL, 200);
    if (tmp != NULL) {
        path = std::string(tmp);
        free(tmp);
    }

    // append relative path of the executable
    path.append(name);

    // check file existence
    if (access(path.c_str(), F_OK) == -1) {
        std::string errMsg = "File could not be found " + path;
        throw CF::InvalidFileName(CF::CF_EINVAL,
                CORBA::string_dup(errMsg.c_str()));
    }

    // change permissions to 7--
    if (chmod(path.c_str(), S_IRWXU) != 0) {
        LOG_ERROR(ExecutableDevice_impl, "Unable to change permission on executable");
        throw CF::ExecutableDevice::ExecuteFail(CF::CF_EACCES,
                "Unable to change permission on executable");
    }

    // assemble argument list
    std::vector<std::string> args = prepend_args;
    if (getenv("VALGRIND")) {
        char* valgrind = getenv("VALGRIND");
        if (strlen(valgrind) == 0) {
            // Assume that valgrind is somewhere on the path
            args.push_back("valgrind");
        } else {
            // Environment variable is path to valgrind executable
            args.push_back(valgrind);
        }
        // Put the log file in the cache next to the component entrypoint;
        // include the pid to avoid clobbering existing files
        std::string logFile = "--log-file=";
        char* name_temp = strdup(path.c_str());
        logFile += dirname(name_temp);
        free(name_temp);
        logFile += "/valgrind.%p.log";
        args.push_back(logFile);
    }
    args.push_back(path);

    LOG_DEBUG(ExecutableDevice_impl, "Building param list for process " << path);
    for (CORBA::ULong i = 0; i < parameters.length(); ++i) {
        LOG_DEBUG(ExecutableDevice_impl, "id=" << ossie::corba::returnString(parameters[i].id) << " value=" << ossie::any_to_string(parameters[i].value));
        CORBA::TypeCode_var atype = parameters[i].value.type();
        args.push_back(ossie::corba::returnString(parameters[i].id));
        args.push_back(ossie::any_to_string(parameters[i].value));
    }

    LOG_DEBUG(ExecutableDevice_impl, "Forking process " << path);

    std::vector<char*> argv(args.size() + 1, NULL);
    for (std::size_t i = 0; i < args.size(); ++i) {
        // const_cast because execv does not modify values in argv[].
        // See:  http://pubs.opengroup.org/onlinepubs/9699919799/functions/exec.html
        argv[i] = const_cast<char*> (args[i].c_str());
    }

    rh_logger::LevelPtr  lvl = ExecutableDevice_impl::__logger->getLevel();

    // fork child process
    int pid = fork();

    if (pid == 0) {

      int num_retries = 5;
      int returnval = 0;

      //
      // log4cxx will cause dead locks between fork and execv, use the stdout logger object, this will only replace the new process'
      // ExecutableDevice's logger object till execv envoked.
      //
      ExecutableDevice_impl::__logger = rh_logger::StdOutLogger::getRootLogger();
      ExecutableDevice_impl::__logger->setLevel(lvl);
      // set affinity logger method so we do not use log4cxx during affinity processing routine
      redhawk::affinity::set_affinity_logger( ExecutableDevice_impl::__logger ) ;
      LOG_DEBUG(ExecutableDevice_impl, " Calling set resource affinity....exec:" << name << " options=" << options.length());

      // set affinity preference before exec
      try {
        LOG_DEBUG(ExecutableDevice_impl, " Calling set resource affinity....exec:" << name << " options=" << options.length());
        set_resource_affinity( options, getpid(), name );
      }
      catch( redhawk::affinity::AffinityFailed &ex ) {
        LOG_WARN(ExecutableDevice_impl, "Unable to satisfy affinity request for: " << name << " Reason: " << ex.what() );
        errno=EPERM<<2;
        returnval=-1;
        ossie::corba::OrbShutdown(true);
        exit(returnval);
      }
      catch( ... ) {
        LOG_WARN(ExecutableDevice_impl,  "Unhandled exception during affinity processing for resource: " << name  );
        ossie::corba::OrbShutdown(true);
        exit(returnval);
      }

      // reset mutex in child...
      pthread_mutex_init(load_execute_lock.native_handle(),0);
      
      // set the forked component as the process group leader
      setpgid(getpid(), 0);
      
      // Run executable
      while(true)
        {
          if (strcmp(argv[0], "valgrind") == 0) {
              // Find valgrind in the path
              returnval = execvp(argv[0], &argv[0]);
          } else {
              returnval = execv(argv[0], &argv[0]);
          }

          num_retries--;
          if( num_retries <= 0 || errno!=ETXTBSY)
                break;

          // Only retry on "text file busy" error
          LOG_WARN(ExecutableDevice_impl, "execv() failed, retrying... (cmd=" << path << " msg=\"" << strerror(errno) << "\" retries=" << num_retries << ")");
          usleep(100000);
        }

        if( returnval ) {
            LOG_ERROR(ExecutableDevice_impl, "Error when calling execv() (cmd=" << path << " errno=" << errno << " msg=\"" << strerror(errno) << "\")");
            ossie::corba::OrbShutdown(true);
        }

        LOG_DEBUG(ExecutableDevice_impl, "Exiting FAILED subprocess:" << returnval );
        exit(returnval);
    }
    else if (pid < 0 ){
        LOG_ERROR(ExecutableDevice_impl, "Error forking child process (errno: " << errno << " msg=\"" << strerror(errno) << "\")" );
        switch (errno) {
            case E2BIG:
                throw CF::ExecutableDevice::ExecuteFail(CF::CF_E2BIG,
                        "Argument list too long");
            case EACCES:
                throw CF::ExecutableDevice::ExecuteFail(CF::CF_EACCES,
                        "Permission denied");
            case ENAMETOOLONG:
                throw CF::ExecutableDevice::ExecuteFail(CF::CF_ENAMETOOLONG,
                        "File name too long");
            case ENOENT:
                throw CF::ExecutableDevice::ExecuteFail(CF::CF_ENOENT,
                        "No such file or directory");
            case ENOEXEC:
                throw CF::ExecutableDevice::ExecuteFail(CF::CF_ENOEXEC,
                        "Exec format error");
            case ENOMEM:
                throw CF::ExecutableDevice::ExecuteFail(CF::CF_ENOMEM,
                        "Out of memory");
            case ENOTDIR:
                throw CF::ExecutableDevice::ExecuteFail(CF::CF_ENOTDIR,
                        "Not a directory");
            case EPERM:
                throw CF::ExecutableDevice::ExecuteFail(CF::CF_EPERM,
                        "Operation not permitted");
            default:
                throw CF::ExecutableDevice::ExecuteFail(CF::CF_NOTSET,
                        "ERROR ON FORK");
        }
    }

    LOG_DEBUG(ExecutableDevice_impl, "Execute success: name:" << name << " : "<< path);

    return pid;
}


/* terminate ***********************************************************
    - terminates a process on the device
******************************************************************* */
void
ExecutableDevice_impl::terminate (CF::ExecutableDevice::ProcessID_Type processId) throw (CORBA::SystemException, CF::ExecutableDevice::InvalidProcess, CF::Device::InvalidState)
{
    std::vector< std::pair< int, float > > _signals;
    _signals.push_back(std::make_pair(SIGINT, 2));
    _signals.push_back(std::make_pair(SIGQUIT, 2));
    _signals.push_back(std::make_pair(SIGTERM, 2));
    _signals.push_back(std::make_pair(SIGKILL, 0.5));
// validate device state
    if (isLocked () || isDisabled ()) {
        printf ("Cannot terminate. System is either LOCKED or DISABLED.");
        throw (CF::Device::
               InvalidState
               ("Cannot terminate. System is either LOCKED or DISABLED."));
    }

  // go ahead and terminate the process
  pid_t pgroup = getpgid(processId);
  bool processes_dead = false;
  for (std::vector< std::pair< int, float > >::iterator _signal=_signals.begin();!processes_dead &&_signal!=_signals.end();_signal++) {
    int retval = killpg(pgroup, _signal->first);
    LOG_TRACE(ExecutableDevice_impl,"Intitial Process Termination pid/group " << processId << "/" << pgroup << "   RET= " << retval);
    if ( retval == -1 && errno == EPERM ) {
      LOG_ERROR(ExecutableDevice_impl,"Error sending pid/group " << processId << "/" <<  pgroup);
      continue;
    }
    if ( retval == -1 && errno == ESRCH )  { 
      LOG_TRACE(ExecutableDevice_impl,"Process group is dead " << processId << "/" <<  pgroup);
     processes_dead = true; 
      continue;
    }
    struct timeval tv;
    gettimeofday(&tv, NULL);
    double now = tv.tv_sec + (((float)tv.tv_usec)/1e6) ;
    double end_time = now + _signal->second;
    int cnt=0;
    while (!processes_dead && (retval != -1) and ( now < end_time )) {
      retval = killpg(pgroup, 0);
      LOG_TRACE(ExecutableDevice_impl,"Terminating Process.... (loop:" << cnt++ << " signal:" << _signal->first << ") pid/group " << processId << "/" << pgroup << "   RET= " << retval);
      if (retval == -1 and (errno == ESRCH)) {
        LOG_TRACE(ExecutableDevice_impl,"Process group terminated  " << processId << "/" <<  pgroup);
        processes_dead = true;
        continue;
      }

      usleep(100000);
      gettimeofday(&tv, NULL);
      now = tv.tv_sec + (((double)tv.tv_usec)/1e6);
    }
  }
}

void  ExecutableDevice_impl::configure (const CF::Properties& capacities)
throw (CF::PropertySet::PartialConfiguration, CF::PropertySet::
       InvalidConfiguration, CORBA::SystemException)
{
    Device_impl::configure(capacities);
}
