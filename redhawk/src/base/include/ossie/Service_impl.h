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


#ifndef SERVICE_IMPL_H
#define SERVICE_IMPL_H

#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include "CF/cf.h"
#include "ossie/ossieSupport.h"
#include "ossie/debug.h"
#include "ossie/CorbaUtils.h"
#include "ossie/logging/loghelpers.h"
#include "ossie/Containers.h"
#include "ossie/Autocomplete.h"
#include <signal.h>

class Service_impl
{
    ENABLE_LOGGING

public:
    template<class T>
    static void start_service(T** servPtr, struct sigaction sa, int argc, char* argv[]) {
        char* devMgr_ior = 0;
        char* name = 0;
        const char* logging_config_uri = 0;
        int debug_level = -1; // use log config uri as log level context 
	std::string logcfg_uri("");
	std::string dpath("");
	std::string sname("");
        
        std::map<std::string, char*> execparams;
                
        for (int i = 0; i < argc; i++) {
            
            if (strcmp("DEVICE_MGR_IOR", argv[i]) == 0) {
                devMgr_ior = argv[++i];
            } else if (strcmp("SERVICE_NAME", argv[i]) == 0) {
                name = argv[++i];
		sname=name;
            } else if (strcmp("LOGGING_CONFIG_URI", argv[i]) == 0) {
                logging_config_uri = argv[++i];
            } else if (strcmp("DEBUG_LEVEL", argv[i]) == 0) {
                debug_level = atoi(argv[++i]);
            } else if (strcmp("DOM_PATH", argv[i]) == 0) {
                dpath = argv[++i];
            } else if (i > 0) {  // any other argument besides the first one is part of the execparams
                std::string paramName = argv[i];
                execparams[paramName] = argv[++i];
            }
        }
                       

        // The ORB must be initialized before configuring logging, which may use
        // CORBA to get its configuration file. Devices do not need persistent IORs.
        ossie::corba::CorbaInit(argc, argv);

	// check if logging config URL was specified...
	if ( logging_config_uri ) logcfg_uri=logging_config_uri;

	// setup logging context for a servie
	ossie::logging::ResourceCtxPtr ctx( new ossie::logging::ServiceCtx( sname, dpath ) );

	// configure the logging library
	ossie::logging::Configure(logcfg_uri, debug_level, ctx);

        if ((devMgr_ior == 0) || (name == 0)) {
            LOG_FATAL(Service_impl, "Per SCA specification, DEVICE_MGR_IOR and SERVICE_NAME must be provided");
            exit(-1);
        }

        LOG_DEBUG(Service_impl, "Name = " << name << " IOR = " << devMgr_ior)

        // Associate SIGINT to signal_catcher interrupt handler
        if( sigaction( SIGINT, &sa, NULL ) == -1 ) {
            LOG_FATAL(Service_impl, "SIGINT association failed");
            exit(EXIT_FAILURE);
        }

        // Associate SIGQUIT to signal_catcher interrupt handler
        if( sigaction( SIGQUIT, &sa, NULL ) == -1 ) {
            LOG_FATAL(Service_impl, "SIGQUIT association failed");
            exit(EXIT_FAILURE);
        }

        // Associate SIGTERM to signal_catcher interrupt handler
        if( sigaction( SIGTERM, &sa, NULL ) == -1 ) {
            LOG_FATAL(Service_impl, "SIGTERM association failed");
            exit(EXIT_FAILURE);
        }

        /* Ignore SIGInterrupt because when you CTRL-C the node
        booter we don't want the device to die, and it's the shells responsibility
        to send CTRL-C to all foreground processes (even children) */
        signal(SIGINT, SIG_IGN);
        
        *servPtr = new T(devMgr_ior, name);
        PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(*servPtr);
        (*servPtr)->resolveDeviceManager();
        (*servPtr)->registerServiceWithDevMgr();
        (*servPtr)->run();
        (*servPtr)->terminateService();
        (*servPtr)->_remove_ref();
	
        ossie::corba::OrbShutdown(true);
    }

    // Get the file path for the logging configuration file
    static std::string getLogConfig(const char* devmgr_ior, const char* log_config, std::string& devmgr_label) {
        // connect to the device manager and copy the log config file to the local directory

        std::string _local_logconfig_path;

        // connect to device manager
        CF::DeviceManager_ptr _devMgr_ptr = CF::DeviceManager::_nil();
        CORBA::Object_var _devMgr_obj = ossie::corba::Orb()->string_to_object(devmgr_ior);
        if (CORBA::is_nil(_devMgr_obj)) {
            std::cout << "ERROR:Service_impl:getLogConfig - Invalid device manager IOR: " << devmgr_ior << std::endl;
            return _local_logconfig_path;
        }

        _devMgr_ptr = CF::DeviceManager::_narrow(_devMgr_obj);
        if (CORBA::is_nil(_devMgr_ptr)) {
            std::cout << "ERROR:Service_impl:getLogConfig - Could not narrow device manager IOR: " << devmgr_ior << std::endl;
            return _local_logconfig_path;
        }

        // store the dev manager's label
        devmgr_label = _devMgr_ptr->label();

        // copy the file to memory
        CF::File_var logFile;
        CF::OctetSequence_var logFileData;
        try {
            logFile = _devMgr_ptr->fileSys()->open(log_config, true);
            unsigned int logFileSize = logFile->sizeOf();
            logFile->read(logFileData, logFileSize);
        } catch ( ... ) {
            std::cout << "ERROR:Service_impl:getLogConfig - Could not copy file to local memory. File name: " << log_config << std::endl;
            return _local_logconfig_path;
        }

        // get the log config file name from the path
        std::string tmp_log_config = log_config;
        std::string::size_type slash_loc = tmp_log_config.find_last_of("/");
        if (slash_loc != std::string::npos) {
            _local_logconfig_path = tmp_log_config.substr(slash_loc + 1);
        }

        // write the file to local directory
        std::fstream _local_logconfig;
        std::ios_base::openmode _local_logconfig_mode = std::ios::in | std::ios::out | std::ios::trunc;
        try {
            _local_logconfig.open(_local_logconfig_path.c_str(), _local_logconfig_mode);
            if (!_local_logconfig.is_open()) {
                std::cout << "ERROR:Service_impl:getLogConfig - Could not open log file on local system. File name: " << _local_logconfig_path << std::endl;
                throw;
            }

            _local_logconfig.write((const char*)logFileData->get_buffer(), logFileData->length());
            if (_local_logconfig.fail()) {
                std::cout << "ERROR:Service_impl:getLogConfig - Could not write log file on local system. File name: " << _local_logconfig_path << std::endl;
                throw;
            }
            _local_logconfig.close();
        } catch ( ... ) {
            std::cout << "ERROR:Service_impl:getLogConfig - Could not copy file to local system. File name: " << _local_logconfig_path << std::endl;
            _local_logconfig_path.clear();  // so calling function knows not to use value
            return _local_logconfig_path;
        }

        return _local_logconfig_path;
    }

protected:
    // Enumerated type for comparison
    enum AnyComparisonType {
        FIRST_BIGGER,
        SECOND_BIGGER,
        BOTH_EQUAL,
        POSITIVE,
        NEGATIVE,
        ZERO,
        UNKNOWN
    };
    CF::DeviceManager_ptr _deviceManager;
    bool initialConfiguration;
    CF::Properties originalCap;
    // Service instance name
    std::string _name;

    // Determine whether the contents of the argument are 0
    Service_impl::AnyComparisonType compareAnyToZero (CORBA::Any& first);
    // Determine whether the contents of different arguments
    Service_impl::AnyComparisonType compareAnys (CORBA::Any& first, CORBA::Any& second);

public:
    Service_impl (char*, char*);
    Service_impl(); // Code that tries to use this constructor will not work
    Service_impl(Service_impl&); // No copying
    ~Service_impl ();
    virtual void resolveDeviceManager ();
    virtual void registerServiceWithDevMgr ();
    virtual void run ();
    virtual void halt ();
    // Function that is called as the Service terminates
    virtual void terminateService ();
    
    // Return the container with a reference to the Device Manager hosting the Service
    redhawk::DeviceManagerContainer* getDeviceManager() {
        return this->_devMgr;
    }
    // Return the container with a reference to the Domain Manager that hosts the Service
    redhawk::DomainManagerContainer* getDomainManager() {
        return this->_domMgr;
    }

protected:
    std::string _devMgr_ior;
    omni_mutex component_running_mutex;
    omni_condition component_running;

private:
    void initResources(char*, char*);
    redhawk::DeviceManagerContainer *_devMgr;
    redhawk::DomainManagerContainer *_domMgr;
};


#endif

