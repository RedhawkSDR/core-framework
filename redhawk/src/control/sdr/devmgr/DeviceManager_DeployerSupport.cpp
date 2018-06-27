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

#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include <ossie/ossieSupport.h>
#include <ossie/debug.h>
#include <ossie/CorbaUtils.h>
#include <ossie/prop_utils.h>
#include <ossie/logging/loghelpers.h>
#include "DeviceManager_impl.h"
#include "rh_logger_stdout.h"

namespace fs = boost::filesystem;

using namespace ossie;


static bool checkPath(const std::string& envpath, const std::string& pattern, char delim=':')
{
    // First, check if the pattern is even in the input path
    std::string::size_type start = envpath.find(pattern);
    if (start == std::string::npos) {
        return false;
    }
    // Next, make sure that the pattern starts at a boundary--either at the
    // beginning, or immediately following a delimiter
    if ((start != 0) && (envpath[start-1] != delim)) {
        return false;
    }
    // Finally, make sure that the pattern ends at a boundary as well
    std::string::size_type end = start + pattern.size();
    return ((end == envpath.size()) || (envpath[end] == delim));
}

void DeviceManager_impl::createDeviceThreadAndHandleExceptions(
        const ossie::DevicePlacement&                 componentPlacement,
	local_spd::ProgramProfile                     *compProfile,
        const std::string&                            componentType,
        const std::string&                            codeFilePath,
        const ossie::ComponentInstantiation&          instantiation,
        const std::string&                            compositeDeviceIOR ){

    try {
	// proces any instance overrides from DCD componentproperties
	const ossie::ComponentPropertyList& overrideProps = instantiation.getProperties();
	for (unsigned int j = 0; j < overrideProps.size (); j++) {
	  RH_TRACE(this->_baseLog, "Override  Properties prop id " << overrideProps[j].getID());
	  compProfile->overrideProperty( overrideProps[j] );
	}

        std::string devcache; 
        std::string devcwd; 
        std::string usageName; 
        createDeviceCacheLocation(devcache, devcwd, usageName, compProfile, instantiation);

        // these variables will cleanup path and environment from package mods that might have failed
        ProcessEnvironment  restoreState;

        createDeviceThread(componentPlacement,
			   compProfile,
                           componentType,
                           codeFilePath,
                           instantiation,
                           devcache,
                           devcwd,
                           usageName,
                           compositeDeviceIOR );

    } catch (std::runtime_error& ex) {
        RH_ERROR(this->_baseLog, 
                  "The following runtime exception occurred: "<<ex.what()<<" while launching a Device")
        throw;
    } catch ( std::exception& ex ) {
        RH_ERROR(this->_baseLog, 
                  "The following standard exception occurred: "<<ex.what()<<" while launching a Device")
        throw;
    } catch ( const CORBA::Exception& ex ) {
        RH_ERROR(this->_baseLog, 
                  "The following CORBA exception occurred: "<<ex._name()<<" while launching a Device")
        throw;
    } catch ( ... ) {
        RH_TRACE(this->_baseLog, 
                  "Launching Device file failed with an unknown exception")
        throw;
    }
}


void DeviceManager_impl::createDeviceCacheLocation(
        std::string& devcache,
        std::string& devcwd,
        std::string& usageName, 
        local_spd::ProgramProfile            *compProfile,
        const ossie::ComponentInstantiation& instantiation )
{
    if (instantiation.getUsageName().empty()) {
        // no usage name was given, so create one. By definition, the instantiation id must be unique
        usageName = instantiation.instantiationId;
    } else {
        usageName = instantiation.getUsageName();
    }

    RH_DEBUG(this->_baseLog, "Getting Cache/Working Directories for: " << instantiation.instantiationId );
    redhawk::PropertyMap tmpProps = redhawk::PropertyMap::cast( compProfile->getNonNilConstructProperties() );
    std::string pcache;
    std::string pcwd;

    if (tmpProps.find("cacheDirectory")!=tmpProps.end()) {
        pcache = tmpProps["cacheDirectory"].toString();
    }
    if (tmpProps.find("workingDirectory")!=tmpProps.end()) {
        pcwd = tmpProps["workingDirectory"].toString();
    }
    
    if (pcache.empty()) {
        std::string baseDevCache = _cacheroot + "/." + _label;
        devcache = baseDevCache + "/" + usageName;
    } else {
        devcache = pcache;
    }
    
    devcwd = pcwd;

    // create device cache location
    bool retval = this->makeDirectory(devcache);
    if (not retval) {
        std::ostringstream emsg;
        emsg << "Unable to create the Device cache directory: " << devcache << " for device : " << usageName;
        RH_ERROR(this->_baseLog, emsg.str() );
        std::runtime_error( emsg.str().c_str() );
    }

    // create device cwd location if needed
    if (not devcwd.empty()) {
        retval = this->makeDirectory(devcwd);
        if (not retval) {
            std::ostringstream emsg;
            emsg << "Unable to create the Device working directory: " << devcache << " for device : " << usageName;
            RH_ERROR(this->_baseLog, emsg.str() );
            std::runtime_error( emsg.str().c_str() );
        }
    }
}


void DeviceManager_impl::createDeviceThread(
        const ossie::DevicePlacement&                 componentPlacement,
	local_spd::ProgramProfile                     *compProfile,
        const std::string&                            componentType,
        const std::string&                            codeFilePath,
        const ossie::ComponentInstantiation&          instantiation,
        const std::string&                            devcache,
        const std::string&                            devcwd,
        const std::string&                            usageName,
        const std::string&                            compositeDeviceIOR ){
 
    RH_DEBUG(this->_baseLog, "Launching " << componentType << " file " 
                                  << codeFilePath << " Usage name " 
                                  << instantiation.getUsageName());
    
    //get code type 
    const local_spd::ImplementationInfo *matchingImpl = compProfile->getSelectedImplementation();
    bool isSharedLibrary = (matchingImpl->getCodeType() == CF::LoadableDevice::SHARED_LIBRARY);
    
    // reset package modifications list
    sharedPkgs.clear();

    // Logic for persona devices
    // check is parent exists and if the code type is "SharedLibrary"
    if (componentPlacement.isCompositePartOf() && isSharedLibrary) {
   
        // Locate the parent device via the IOR
        CORBA::Object_var _aggDev_obj = ossie::corba::Orb()->string_to_object(compositeDeviceIOR.c_str());
        if (CORBA::is_nil(_aggDev_obj)) {
            std::ostringstream emsg;
            emsg << "Failed to deploy '" << usageName << "': Invalid composite device IOR: " << compositeDeviceIOR;
            RH_ERROR(this->_baseLog, emsg.str() );
            return;
        }

        // Attempt to narrow the parent device to an Executable Device
        CF::ExecutableDevice_var execDevice = ossie::corba::_narrowSafe<CF::ExecutableDevice>(_aggDev_obj);
        if (CORBA::is_nil(execDevice)) {
            std::ostringstream emsg;
            emsg <<"Failed to deploy '" << usageName <<
                "': DeployOnDevice must be an Executable Device:  Unable to narrow to Executable Device";
                RH_ERROR(this->_baseLog, emsg.str());
                return;
        }

        // load soft package dependencies
        loadDependencies( compProfile, execDevice, matchingImpl->getSoftPkgDependencies() );
        
        // all conditions are met for a persona    
        // Load shared library into device using load mechanism
        std::string execDevId = ossie::corba::returnString(execDevice->identifier());
        RH_DEBUG(this->_baseLog, "Loading '" << codeFilePath << "' to parent device: " << execDevId );
        execDevice->load(_fileSys, codeFilePath.c_str(), CF::LoadableDevice::SHARED_LIBRARY);
        RH_DEBUG(this->_baseLog, "Load complete on device: " << execDevId);
       
        const std::string realCompType = "device";

        ExecparamList execparams = createDeviceExecparams(componentPlacement,
							  compProfile,
                                                          realCompType,
                                                          codeFilePath,
                                                          instantiation,
                                                          usageName,
                                                          compositeDeviceIOR);
                
        // Pack execparams into CF::Properties to send to the parent
        CF::Properties personaProps;
        for (ExecparamList::iterator arg = execparams.begin(); arg != execparams.end(); ++arg) {
            RH_DEBUG(this->_baseLog, arg->first << "=\"" << arg->second << "\"");
            CF::DataType argument;
            argument.id = arg->first.c_str();
            argument.value <<= arg->second;
            ossie::corba::push_back(personaProps, argument);
        }
        CF::Properties options;
        options.length(0);

        CF::StringSequence dep_seq;
        std::vector<std::string> resolved_softpkg_deps = compProfile->getResolvedSoftPkgDependencies();
        dep_seq.length(resolved_softpkg_deps.size());
        for (unsigned int p=0;p!=dep_seq.length();p++) {
            dep_seq[p]=CORBA::string_dup(resolved_softpkg_deps[p].c_str());
        }

        // Tell parent device to execute shared library using execute mechanism
        RH_DEBUG(this->_baseLog, "Execute '" << codeFilePath << "' on parent device: " << execDevice->identifier());
        execDevice->executeLinked(codeFilePath.c_str(), options, personaProps, dep_seq);
        RH_DEBUG(this->_baseLog, "Execute complete");

    } else {

       
        ProcessEnvironment myenv( false /* == do not restore */ );

        std::vector< std::string > new_argv;

        createDeviceExecStatement(new_argv, 
                                  componentPlacement,
                                  compProfile,
                                  componentType,
                                  codeFilePath,
                                  instantiation,
                                  usageName,
                                  compositeDeviceIOR );

        // check that our executable is good
        if (access(codeFilePath.c_str(), R_OK | X_OK) == -1) {
            std::string errMsg = "Missing read or execute file permissions on file: " + codeFilePath;
            RH_ERROR(this->_baseLog, errMsg );
            return;
        }

        // convert std:string to char * for execv
        std::vector<char*> argv(new_argv.size() + 1, NULL);
        for (std::size_t i = 0; i < new_argv.size(); ++i) {
            RH_DEBUG(this->_baseLog, "ARG: " << i << " VALUE " << new_argv[i] );
            argv[i] = const_cast<char*> (new_argv[i].c_str());
        }

        loadDependencies( compProfile, CF::LoadableDevice::_nil(), matchingImpl->getSoftPkgDependencies() );

        setEnvironment( myenv, compProfile->getResolvedSoftPkgDependencies(), sharedPkgs );

        rh_logger::LevelPtr  lvl = DeviceManager_impl::__logger->getLevel();

        int pid = fork();
        if (pid > 0) {
            // parent process: pid is the process ID of the child
            RH_TRACE(this->_baseLog, "Resulting PID: " << pid);

            // Add the new device/service to the pending list. When it registers, the remaining
            // fields will be filled out and it will be moved to the registered list.
            if (componentType == "service") {
                ServiceNode* serviceNode = new ServiceNode;
                serviceNode->identifier = instantiation.getID();
                serviceNode->label = usageName;
                serviceNode->pid = pid;
                boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);
                _pendingServices.push_back(serviceNode);
            } else {
                DeviceNode* deviceNode = new DeviceNode;
                deviceNode->identifier = instantiation.getID();
                deviceNode->label      = usageName;
                deviceNode->pid        = pid;
                boost::recursive_mutex::scoped_lock lock(registeredDevicesmutex);
                _pendingDevices.push_back(deviceNode);
            }
        }
        else if (pid == 0) {
            // Child process

            rh_logger::LoggerPtr __logger = rh_logger::StdOutLogger::getRootLogger();
            __logger->setLevel(lvl);

            // apply environment changes for the process
            myenv.apply();

            int err;
            sigset_t  sigset;
            err=sigemptyset(&sigset);
            err = sigaddset(&sigset, SIGINT);
            if ( err ) {
                RH_ERROR(__logger, "sigaction(SIGINT): " << strerror(errno));
            }
            err = sigaddset(&sigset, SIGQUIT);
            if ( err ) {
                RH_ERROR(__logger, "sigaction(SIGQUIT): " << strerror(errno));
            }
            err = sigaddset(&sigset, SIGTERM);
            if ( err ) {
                RH_ERROR(__logger, "sigaction(SIGTERM): " << strerror(errno));
            }
            err = sigaddset(&sigset, SIGCHLD);
            if ( err ) {
                RH_ERROR(__logger, "sigaction(SIGCHLD): " << strerror(errno));
            }

            // We must unblock the signals for child processes
            err = sigprocmask(SIG_UNBLOCK, &sigset, NULL);

            //////////////////////////////////////////////////////////////
            // DO not put any LOG calls between the fork and the execv
            //////////////////////////////////////////////////////////////

            // switch to working directory
            if (not devcwd.empty()) {
                int retval = chdir(devcwd.c_str());
                if (retval) {
                    RH_ERROR(__logger, "Unable to change the current working directory to : " << devcwd);
                }
            } else {
                chdir(devcache.c_str());
            }

            // honor affinity requests
            try {
                CF::Properties options;
                options = getResourceOptions( instantiation );
                if ( redhawk::affinity::has_affinity(options) ){
                    if ( redhawk::affinity::is_disabled() ) {
                        RH_WARN(__logger, "Affinity processing is disabled, unable to apply AFFINITY properties for resource: " << usageName );
                    }
                    else {
                        RH_DEBUG(__logger, "Applying AFFINITY properties, resource: " << usageName );
                        redhawk::affinity::set_affinity( options, getpid(), cpu_blacklist );
                    }
                }
            }
            catch( redhawk::affinity::AffinityFailed &e) {
                RH_WARN(__logger, "AFFINITY REQUEST FAILED, RESOURCE: " << usageName << ", REASON: " << e.what() );
            }

            // now exec - we should not return from this
            if (strcmp(argv[0], "valgrind") == 0) {
                execvp(argv[0], &argv[0]);
            } else {
                execv(argv[0], &argv[0]);
            }

            RH_ERROR(__logger, new_argv[0] << " did not execute : " << strerror(errno));
            exit(-1);
        }
        else {
            // The system cannot support deployment of the device
            // The most likely cause is the operating system running out of 
            // threads, in which case the system is in bad shape.  Exit
            // with an error to allow the system to recover.
            RH_ERROR(this->_baseLog, "[DeviceManager::execute] Cannot create device thread: " << strerror(errno)); 
            throw;
        }
    }
}



/*
 * Populate new_argv, which is a list of character strings
 * to be passed to execv.
 */
void DeviceManager_impl::createDeviceExecStatement(
        std::vector< std::string >&                   new_argv, 
        const ossie::DevicePlacement&                 componentPlacement,
	local_spd::ProgramProfile*                    compProfile,
        const std::string&                            componentType,
        const std::string&                            codeFilePath,
        const ossie::ComponentInstantiation&          instantiation,
        const std::string&                            usageName,
        const std::string&                            compositeDeviceIOR ) {
    
    // DO not put any LOG calls in this method, as it is called beteen
    // fork() and execv().
    std::string logcfg_path("");
    deviceMgrIOR = ossie::corba::objectToString(myObj);
    if (getenv("VALGRIND")) {
        const char* valgrind = getenv("VALGRIND");
        if (strlen(valgrind) > 0) {
            // Environment variable is path to valgrind executable
            new_argv.push_back(valgrind);
        } else {
            // Assume that valgrind is somewhere on the path
            new_argv.push_back("valgrind");
        }
        // Put the log file in the current working directory, which will be the
        // device's cache directory; include the pid to avoid clobbering
        // existing files
        new_argv.push_back("--log-file=valgrind.%p.log");
    }
    // argv[0] program executable
    new_argv.push_back(codeFilePath);             

    if (componentType == "device") {
      new_argv.push_back("PROFILE_NAME");
      new_argv.push_back(compProfile->getSpdFileName());
      new_argv.push_back( "DEVICE_ID");
      new_argv.push_back(instantiation.getID());
      new_argv.push_back( "DEVICE_LABEL");
      new_argv.push_back(usageName);
      if (componentPlacement.isCompositePartOf()) {
          new_argv.push_back("COMPOSITE_DEVICE_IOR");
          new_argv.push_back(compositeDeviceIOR);
      }

      if (IDM_IOR.size() > 0 ) {
        new_argv.push_back("IDM_CHANNEL_IOR");
        new_argv.push_back(IDM_IOR.c_str());
      }
      logcfg_path= ossie::logging::GetDevicePath(_domainName, _label, usageName );
    } else if (componentType == "service") {
      logcfg_path= ossie::logging::GetServicePath(_domainName, _label, usageName );
      new_argv.push_back( "SERVICE_NAME");
      new_argv.push_back(usageName);
    }
    
    resolveLoggingConfiguration( usageName, new_argv, instantiation, logcfg_path );

    std::string dpath;
    dpath = _domainName + "/" + _label;
    new_argv.push_back("DOM_PATH");
    new_argv.push_back(dpath);
    RH_DEBUG(this->_baseLog, "DOM_PATH: arg: " << new_argv[new_argv.size()-2] << " value:"  << dpath);

    // check exec params... place all of them here.. allow for instance props to override the params...
    CF::Properties eParams = compProfile->getPopulatedExecParameters();
    for (CORBA::ULong i = 0; i < eParams.length(); ++i) {
      std::string p1;
      std::string p2;
      p1 = ossie::corba::returnString(eParams[i].id);
      p2 = ossie::any_to_string(eParams[i].value);      
      RH_TRACE(this->_baseLog, "createDevicExecStatement id= " << p1 << " value= " << p2 );
      CORBA::TypeCode_var  etype=eParams[i].value.type();
      RH_TRACE(this->_baseLog, " createDeviceExecParams id= " << p1 << "  type " << etype->kind() << " tk_bool " << CORBA::tk_boolean );
      if  ( etype->kind() == CORBA::tk_boolean ) {
          std::string v("");
          resolveBoolExec( p1, v, compProfile, instantiation);
          if ( v.size() == 0 ) continue;   // skip empty params
          p2=v;
      }

      // skiop already added params
      if ( p1 == "LOGGING_CONFIG_URI" || p1 == "DEBUG_LEVEL" ) continue;
      new_argv.push_back(p1);
      new_argv.push_back(p2);
    }

    // Move this to the end of the command-line to make it easier to read
    new_argv.push_back("DEVICE_MGR_IOR");
    new_argv.push_back(deviceMgrIOR);

}                    


// handle boolean exec params as user provided value
void   DeviceManager_impl::resolveBoolExec( const std::string&                      id,
                                            std::string&                            value,
                                            local_spd::ProgramProfile*              compProfile,
                                            const ossie::ComponentInstantiation&    instantiation ){

    const std::vector<const Property*>& eprop = compProfile->prf.getExecParamProperties();
    RH_TRACE(this->_baseLog, "resolveBoolExec exec params size " << eprop.size() );
    for (unsigned int j = 0; j < eprop.size(); j++) {
        std::string prop_id = eprop[j]->getID();
        if ( prop_id == id  ){
            RH_TRACE(this->_baseLog, "resolveBoolExec exec id == instantiation prop " << id <<  " = " << prop_id );
             const SimpleProperty* tmp = dynamic_cast<const SimpleProperty*>(eprop[j]);
             if (tmp) {
                 RH_TRACE(this->_baseLog, "Default exec boolean with PRF value: " << tmp->getValue());
                 if (tmp->getValue()) {
                     std::string v(tmp->getValue());
                     if ( v.size() != 0 ) {
                         value = tmp->getValue();
                     }
                 }
             }
        }
    }

    const std::vector<const Property*>& cprop = compProfile->prf.getConstructProperties();
    RH_TRACE(this->_baseLog, "resolveBoolExec ctor params size: " << cprop.size() );
    for (unsigned int j = 0; j < cprop.size(); j++) {
        std::string prop_id = cprop[j]->getID();
        if ( prop_id == id  ){
            RH_TRACE(this->_baseLog, "resolveBoolExec ctor id == instantiation prop " << id <<  " = " << prop_id );
            const SimpleProperty* tmp = dynamic_cast<const SimpleProperty*>(cprop[j]);
            if (tmp) {
                 RH_TRACE(this->_baseLog, "Default exec boolean with PRF value: " << tmp->getValue());
                 if (tmp->getValue()) {
                     std::string v(tmp->getValue());
                     if ( v.size() != 0 ) {
                         value = tmp->getValue();
                     }
                 }
             }
        }
    }

    // handle bool values to be actual provided overrides. corba stores as 0/1
    const ossie::ComponentPropertyList& overrideProps = instantiation.getProperties();
    for (unsigned int j = 0; j < overrideProps.size (); j++) {
        std::string prop_id = overrideProps[j].getID();
        if ( prop_id == id  ){
            RH_TRACE(this->_baseLog, "resolveBoolExec instance id == instantiation prop " << id <<  " = " << prop_id );
            const SimplePropertyRef* ref = dynamic_cast<const SimplePropertyRef*>(&overrideProps[j]);
            if ( ref ) {
                RH_TRACE(this->_baseLog, "Overriding exec boolean with instance value: " << ref->getValue());
                if (ref->getValue()) {
                     std::string v(ref->getValue());
                     value = v;
                }
            }
        }
    }



}


DeviceManager_impl::ExecparamList DeviceManager_impl::createDeviceExecparams(
        const ossie::DevicePlacement&                 componentPlacement,
	local_spd::ProgramProfile                     *compProfile,
        const std::string&                            componentType,
        const std::string&                            codeFilePath,
        const ossie::ComponentInstantiation&          instantiation,
        const std::string&                            usageName,
        const std::string&                            compositeDeviceIOR )
{
    // DO not put any LOG calls in this method, as it is called beteen
    // fork() and execv().
    ExecparamList execparams;
    std::string logcfg_path("");
    deviceMgrIOR = ossie::corba::objectToString(myObj);
    if (componentType == "device") {
        execparams.push_back(std::make_pair("PROFILE_NAME", compProfile->getSpdFileName()));
        execparams.push_back(std::make_pair("DEVICE_ID", instantiation.getID()));
        execparams.push_back(std::make_pair("DEVICE_LABEL", usageName));
        if (componentPlacement.isCompositePartOf()) {
            execparams.push_back(std::make_pair("COMPOSITE_DEVICE_IOR", compositeDeviceIOR));
        }
        if (!CORBA::is_nil(idm_registration->channel) && IDM_IOR.size() > 0 ) {
            execparams.push_back(std::make_pair("IDM_CHANNEL_IOR", IDM_IOR));
        }
	logcfg_path=ossie::logging::GetDevicePath( _domainName, _label, usageName );
    } else if (componentType == "service") {
      logcfg_path=ossie::logging::GetServicePath( _domainName, _label, usageName );
      execparams.push_back(std::make_pair("SERVICE_NAME", usageName));
    }

    // Try to resolve LOGGING_CONFIG_URI exec param.
    //  1) honor LOGGING_CONFIG_URI property
    //  2) check if log_cfg_resolver resolves device path
    //  3) use  devmgr's property value
    std::vector< std::string >  new_argv;
    resolveLoggingConfiguration( usageName, new_argv, instantiation, logcfg_path );
    for( std::vector< std::string >::iterator iter = new_argv.begin(); iter != new_argv.end(); ) {
        std::string p1 = iter->c_str();
        iter++;
        std::string p2="";
        if ( iter != new_argv.end() ){
            p2 = iter->c_str();
            iter++;
        }
        execparams.push_back(std::make_pair(p1,p2));
    }

    std::string dpath;
    dpath = _domainName + "/" + _label;
    execparams.push_back(std::make_pair("DOM_PATH", dpath));

    //
    // check exec params... place all of them here.. 
    //   instance overrides should already have been applied
    //
    CF::Properties eParams = compProfile->getPopulatedExecParameters();
    for (CORBA::ULong i = 0; i < eParams.length(); ++i) {
      std::string p1;
      std::string p2;
      p1 = ossie::corba::returnString(eParams[i].id);
      p2 = ossie::any_to_string(eParams[i].value);      
      RH_DEBUG(this->_baseLog, " createDeviceExecParams id= " << p1 << "  value= " << p2 );
      CORBA::TypeCode_var  etype=eParams[i].value.type();
      RH_DEBUG(this->_baseLog, " createDeviceExecParams id= " << p1 << "  type " << etype->kind() );
      if  ( etype->kind() == CORBA::tk_boolean ) {      
          std::string v("");
          resolveBoolExec( p1, v, compProfile, instantiation);
          if ( v.size() == 0 ) continue;   // skip empty params
          p2=v;
      }
      
      if ( p1 == "LOGGING_CONFIG_URI" || p1 == "DEBUG_LEVEL" ) continue;
      execparams.push_back(std::make_pair(p1,p2));
    }
    
    // Move this to the end of the command-line to make it easier to read
    execparams.push_back(std::make_pair("DEVICE_MGR_IOR", deviceMgrIOR));

    return execparams;
}


int DeviceManager_impl::resolveDebugLevel( const std::string &level_in ) {
    int  debug_level=-1;
    std::string dlevel = boost::to_upper_copy(level_in);
    rh_logger::LevelPtr rhlevel=ossie::logging::ConvertCanonicalLevelToRHLevel( dlevel );
    debug_level = ossie::logging::ConvertRHLevelToDebug( rhlevel );
    if ( dlevel.at(0) != 'I' and debug_level == 3 ) debug_level=-1;

    // test if number was provided.
    if ( debug_level == -1  ){
        char *p=NULL;
        int dl=strtol(dlevel.c_str(), &p, 10 );
        if ( p == 0 ) {
            // this will for check valid value and force to info for errant values
            rh_logger::LevelPtr rhlevel=ossie::logging::ConvertDebugToRHLevel( dl );
            debug_level = ossie::logging::ConvertRHLevelToDebug( rhlevel );
        }
    }

    return debug_level;
}


void DeviceManager_impl::resolveLoggingConfiguration( const std::string &                      usageName,
                                                      std::vector< std::string >&              new_argv,
                                                      const ossie::ComponentInstantiation&     instantiation,
                                                      const std::string &logcfg_path ) {
    // get logging info if available
    std::string logging_uri = "";
    int debug_level = -1;
    const ossie::ComponentPropertyList&      instanceprops = instantiation.getProperties();
    ossie::ComponentPropertyList::const_iterator iprops_iter;
    for (iprops_iter = instanceprops.begin(); iprops_iter != instanceprops.end(); iprops_iter++) {
        if ((strcmp(iprops_iter->getID().c_str(), "LOGGING_CONFIG_URI") == 0)
            && (dynamic_cast<const SimplePropertyRef*>(&(*iprops_iter)) != NULL)) {
          const SimplePropertyRef* simpleref = dynamic_cast<const SimplePropertyRef*>(&(*iprops_iter));
            logging_uri = simpleref->getValue();
            RH_DEBUG(this->_baseLog, "resolveLoggingConfig: property log config:" << logging_uri);
            continue;
        }

        if ((strcmp(iprops_iter->getID().c_str(), "LOG_LEVEL") == 0)
            && (dynamic_cast<const SimplePropertyRef*>(&(*iprops_iter)) != NULL)) {
          const SimplePropertyRef* simpleref = dynamic_cast<const SimplePropertyRef*>(&(*iprops_iter));
          debug_level = resolveDebugLevel(simpleref->getValue());
          RH_DEBUG(this->_baseLog, "resolveLoggingConfig: property log level:" << debug_level);
          continue;
        }
    }

    // check if logging configuration is part of component
    ossie::ComponentInstantiation::LoggingConfig log_config=instantiation.getLoggingConfig();
    if ( !log_config.first.empty()) {
        logging_uri = log_config.first;
        RH_DEBUG(this->_baseLog, "resolveLoggingConfig: loggingconfig log config:" << logging_uri);
    }
    // check if debug value provided
    if ( !log_config.second.empty() ) {
        debug_level = resolveDebugLevel( log_config.second );
        RH_DEBUG(this->_baseLog, "resolveLoggingConfig: loggingconfig debug_level:" << debug_level);
    }

    if ( getUseLogConfigResolver() ) {
        ossie::logging::LogConfigUriResolverPtr logcfg_resolver = ossie::logging::GetLogConfigUriResolver();
        if ( logcfg_resolver ) {
            std::string t_uri = logcfg_resolver->get_uri( logcfg_path );
            RH_DEBUG(this->_baseLog, "Resolve LOGGING_CONFIG_URI:  key:" << logcfg_path << " value <" << t_uri << ">" );
            if ( !t_uri.empty() ) logging_uri = t_uri;
        }
    }

    // if we still do not have a URI then assigne the DeviceManager's logging config uri
    if (logging_uri.empty() && !logging_config_prop->isNil()) {
      logging_uri = logging_config_uri;
    }

    if (!logging_uri.empty()) {
        if (logging_uri.substr(0, 4) == "sca:") {
          logging_uri += ("?fs=" + fileSysIOR);
        }
        new_argv.push_back("LOGGING_CONFIG_URI");
        new_argv.push_back(logging_uri);
        RH_DEBUG(this->_baseLog, "RSC: " << usageName << " LOGGING PARAM:VALUE " << new_argv[new_argv.size()-2] << ":" <<new_argv[new_argv.size()-1] );
    }
    
    if (debug_level == -1) {
        debug_level = _initialDebugLevel;
    }

    if (debug_level != -1) {
        // Convert the numeric level directly into its ASCII equivalent.
        std::string dlevel="";
        dlevel.push_back(char(0x30 + debug_level));
        new_argv.push_back( "DEBUG_LEVEL");
        new_argv.push_back(dlevel);
        RH_DEBUG(this->_baseLog, "DEBUG_LEVEL: arg: " << new_argv[new_argv.size()-2] << " value:"  << dlevel );
    }
}


void DeviceManager_impl::setEnvironment( ProcessEnvironment &env,
                                         const std::vector< std::string > &deps,
                                         const PackageMods  &pkgMods )
{

    RH_TRACE(this->_baseLog, "setEnvironment setting enviroment deps:" << deps.size() );

    // 
    // get modifications for each of the dependency files that were loaded
    //
    PackageModList mod_list;
    for (unsigned int i=0; i<deps.size(); i++) {
        std::string dep = deps[i];
        RH_TRACE(this->_baseLog, "setEnvironment looking for dep: " << dep );
        if ( pkgMods.find(dep) == pkgMods.end()) {
            // it is not a loaded package
        } else {
            RH_TRACE(this->_baseLog, "adding to Mod List dep: " << dep );
            mod_list.push_back(sharedPkgs[dep]);
        }
    }


    // 
    // apply modifications to the ProcessEnvironment proxy 
    //
    for (PackageModList::iterator p = mod_list.begin(); p != mod_list.end();p++) {
        const PackageMod &pkg = *p;
        PackageMod::PathModifications::const_iterator mod;
        for ( mod = pkg.modifications.begin(); mod!=pkg.modifications.end(); mod++) {
            
            std::string current_path;
            if (getenv(mod->first.c_str())) {
                current_path = env.getenv(mod->first);
            }
            std::vector< std::string >::const_iterator piter;
            for ( piter = mod->second.begin(); piter != mod->second.end(); piter++ ) {
                // Make sure that the current path is not already in the current path
                std::string path_mod=*piter;
                if (!checkPath(current_path, path_mod )) {
                    if (!current_path.empty()) {
                        path_mod += ":" + current_path;
                    }
                    RH_TRACE(this->_baseLog, "setEnvironment env: " << mod->first << " value:"  << path_mod );
                    env.setenv(mod->first, path_mod);
                    current_path = path_mod;
                }
            }
        };
    };

}


void DeviceManager_impl::loadDependencies( local_spd::ProgramProfile *component,
                                           CF::LoadableDevice_ptr device,
                                           const local_spd::SoftpkgInfoList & dependencies)
{
    for ( local_spd::SoftpkgInfoList::const_iterator dep = dependencies.begin(); dep != dependencies.end(); ++dep) {
        const local_spd::ImplementationInfo* implementation = (*dep)->getSelectedImplementation();
        if (!implementation) {
            std::ostringstream emsg;
            emsg << "No implementation selected for dependency " << (*dep)->getName();
            RH_ERROR(this->_baseLog, emsg.str());
            throw std::runtime_error( emsg.str().c_str() );
        }

        // Recursively load dependencies
        std::string pkgId = (*dep)->getName();
        RH_TRACE(this->_baseLog, "Loading dependencies for soft package " << pkgId );
        loadDependencies(component, device, implementation->getSoftPkgDependencies());

        // Determine absolute path of dependency's local file
        CF::LoadableDevice::LoadType codeType = implementation->getCodeType();
        fs::path codeLocalFile = fs::path(implementation->getLocalFileName());
        if (!codeLocalFile.has_root_directory()) {
            // Path is relative to SPD file location
            fs::path base_dir = fs::path((*dep)->getSpdFileName()).parent_path();
            codeLocalFile = base_dir / codeLocalFile;
        }
        codeLocalFile = codeLocalFile.normalize();
        if (codeLocalFile.has_leaf() && codeLocalFile.leaf() == ".") {
            codeLocalFile = codeLocalFile.branch_path();
        }

        const std::string fileName = codeLocalFile.string();
        RH_DEBUG(this->_baseLog, "Loading dependency local file " << fileName);
        try {
            if ( !CORBA::is_nil(device) ) {
                device->load(_fileSys, fileName.c_str(), codeType);
            }
            else {
                do_load( _local_dom_filesys, fileName, codeType );
            }
        } catch (...) {
            std::ostringstream emsg ;
            emsg << "Failure loading file " << fileName;
            RH_ERROR(this->_baseLog, emsg.str());
            throw std::runtime_error( emsg.str().c_str() );
        }

        component->addResolvedSoftPkgDependency(fileName);
    }
}



void DeviceManager_impl::do_load ( CF::FileSystem_ptr fs,
                                   const std::string &fileName,
                                   const CF::LoadableDevice::LoadType &loadKind)
{
    //
    // perform similar steps that a loadable device performs except we don't need
    // to cache the files..
    //

    // verify that the loadKind is supported (only executable is supported by this version)
    if ((loadKind != CF::LoadableDevice::EXECUTABLE) && (loadKind != CF::LoadableDevice::SHARED_LIBRARY)) {
        std::ostringstream emsg;
        emsg << "File: " << fileName << " is not CF::LoadableDevice::EXECUTABLE or CF::LoadableDevice::SHARED_LIBRARY";
        RH_ERROR(this->_baseLog, emsg.str() );
        throw std::runtime_error( emsg.str().c_str() );
    }

    // get actual path to file that should be loaded..
    std::string workingFileName;
    std::string sep("/");
    if ( fileName[0] =='/' )  sep="";
    workingFileName = _local_domroot + sep + fileName;

    // check if directory
    fs::path workpath(workingFileName);
    try {
        if (!fs::exists (workpath)) {
            std::ostringstream emsg;
            emsg << "File : " << workingFileName << " does not exist";
            RH_ERROR(this->_baseLog, emsg.str());
            throw std::runtime_error(emsg.str().c_str());
        }
    }
    catch(...){
        std::ostringstream emsg;
        emsg << "Unknown exception when testing file : " << workingFileName ;
        RH_ERROR(this->_baseLog, emsg.str());
        throw std::runtime_error(emsg.str().c_str());
    }
    
    if (loadKind == CF::LoadableDevice::SHARED_LIBRARY) {


        PackageMod env_changes(fileName);   // saves change set for environment
        bool CLibrary = false;
        bool PythonPackage = false;
        bool JavaJar = false;

        //
        // Check to see if it's a C library
        //
        RH_DEBUG(this->_baseLog, "do_load check read elf file: " << workingFileName );
        std::string command = "readelf -h ";
        command += workingFileName;
        command += std::string(" 2>&1"); // redirect stdout to /dev/null
        FILE *fileCheck = popen(command.c_str(), "r");
        int status = pclose(fileCheck);
        std::string currentPath = ossie::getCurrentDirName();

        if (!status) { 
            fs::path additionalPath(workingFileName);
            if ( additionalPath.has_parent_path() ) {
                additionalPath = additionalPath.parent_path();
            }
            env_changes.addLibPath(additionalPath.string());
            CLibrary = true;
        }

        // Check to see if it's a Python module
        if (!CLibrary) {
            RH_DEBUG(this->_baseLog, "do_load checking python module file: " << workingFileName );
            currentPath = ossie::getCurrentDirName();
            std::string::size_type lastSlash = workingFileName.find_last_of("/");
            if (lastSlash == std::string::npos) { // there are no slashes in the name
                std::string fileOrDirectoryName = workingFileName;
                std::string relativePath = "";
                std::string::size_type iext = fileOrDirectoryName.find_last_of(".");
                if (iext != std::string::npos) {
                    std::string extension = fileOrDirectoryName.substr(iext);
                    if ((extension == ".py") || (extension == ".pyc")) {
                        fileOrDirectoryName.erase(iext);
                    }
                }
                std::string command = "python -c \"import ";
                command += fileOrDirectoryName;
                command += std::string("\" 2>&1"); // redirect stdout and stderr to /dev/null
                RH_DEBUG(this->_baseLog, "do_load check python module (.py) cmd= " << command << 
                          " workingFileName: " << workingFileName <<
                          " currentDirectory: " << currentPath);
                FILE *fileCheck = popen(command.c_str(), "r");
                int status = pclose(fileCheck);
                if (!status) {
                    env_changes.addPythonPath(currentPath);
                    PythonPackage = true;
                }
            } else {
                // try to import module as directory 
                std::string fileOrDirectoryName = "";
                std::string relativePath = "";
                std::string relativeFileName = workingFileName;
                fileOrDirectoryName.assign(relativeFileName, lastSlash+1, relativeFileName.size()-lastSlash);
                std::string::size_type iext = fileOrDirectoryName.find_last_of(".");
                if (iext != std::string::npos) {
                    std::string extension = fileOrDirectoryName.substr(iext);
                    if ((extension == ".py") || (extension == ".pyc")) {
                        fileOrDirectoryName.erase(iext);
                    }
                }
                relativePath.assign(relativeFileName, 0, lastSlash);
                // change parent directory and try to import module
                if (chdir(relativePath.c_str())) {
                    // this is an invalid path
                } else {
                    std::string command = "python -c \"import ";
                    command += fileOrDirectoryName;
                    command += std::string("\" 2>&1"); // redirect stdout and stderr to /dev/null
                    RH_DEBUG(this->_baseLog, "cmd= " << command << 
                              " relativeFileName: " << relativeFileName <<
                              " relativePath: " << relativePath);
                    FILE *fileCheck = popen(command.c_str(), "r");
                    int status = pclose(fileCheck);
                    if (!status) {
                        // The import worked
                        env_changes.addPythonPath(relativePath);
                        PythonPackage = true;
                    }
                }
            }

            chdir(currentPath.c_str());

        }

        // Check to see if it's a Java package
        if (!CLibrary and !PythonPackage) {
            RH_DEBUG(this->_baseLog, "do_load trying JAVA test for : " << workingFileName );
            if ( ossie::helpers::is_jarfile( workingFileName ) == 0 ) {
                env_changes.addJavaPath(workingFileName);
                JavaJar = true;
            }
        }
    
        // It doesn't match anything, assume that it's a set of libraries
        if (!(CLibrary || PythonPackage || JavaJar)) {
            const std::string additionalPath = workingFileName;
            RH_DEBUG(this->_baseLog, "do_load Adding (PATH) Directory ")
            env_changes.addLibPath(additionalPath);
            env_changes.addJavaPath(additionalPath);
            env_changes.addOctavePath(additionalPath);
        }
        sharedPkgs[env_changes.pkgId] = env_changes;
    }
}


