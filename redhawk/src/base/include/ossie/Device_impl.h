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


#ifndef DEVICE_IMPL_H
#define DEVICE_IMPL_H

#include <string>
#include <iostream>
#include <fstream>
#include <map>

#if ENABLE_EVENTS
#include <COS/CosEventComm.hh>
#include <COS/CosEventChannelAdmin.hh>
#endif

#include "Resource_impl.h"
#include "CF/cf.h"
#include "ossie/debug.h"
#include "ossie/CorbaUtils.h"
#include <signal.h>

class OSSIECF_API Device_impl;

#if ENABLE_EVENTS
class OSSIECF_API IDM_Channel_Supplier_i : virtual public POA_CosEventComm::PushSupplier
{

public:
    IDM_Channel_Supplier_i (Device_impl *_dev);
    void disconnect_push_supplier ();

private:
    Device_impl *_device;
    
};
#endif


class OSSIECF_API Device_impl: public virtual POA_CF::Device, public Resource_impl
{
    ENABLE_LOGGING

public:
    template<class T>
    static void start_device(T** devPtr, struct sigaction sa, int argc, char* argv[]) {
        char* devMgr_ior = 0;
        char* id = 0;
        char* label = 0;
        char* profile = 0;
        char* idm_channel_ior = 0;
        char* composite_device = 0;
        const char* logging_config_uri = 0;
        int debug_level = 3; // Default level is INFO.
        
        std::map<std::string, char*> execparams;
                
        for (int i = 0; i < argc; i++) {
            
            if (strcmp("DEVICE_MGR_IOR", argv[i]) == 0) {
                devMgr_ior = argv[++i];
            } else if (strcmp("PROFILE_NAME", argv[i]) == 0) {
                profile = argv[++i];
            } else if (strcmp("DEVICE_ID", argv[i]) == 0) {
                id = argv[++i];
            } else if (strcmp("DEVICE_LABEL", argv[i]) == 0) {
                label = argv[++i];
            } else if (strcmp("IDM_CHANNEL_IOR", argv[i]) == 0) {
                idm_channel_ior = argv[++i];
            } else if (strcmp("COMPOSITE_DEVICE_IOR", argv[i]) == 0) {
                composite_device = argv[++i];
            } else if (strcmp("LOGGING_CONFIG_URI", argv[i]) == 0) {
                logging_config_uri = argv[++i];
            } else if (strcmp("DEBUG_LEVEL", argv[i]) == 0) {
                debug_level = atoi(argv[++i]);
            } else if (i > 0) {  // any other argument besides the first one is part of the execparams
                std::string paramName = argv[i];
                execparams[paramName] = argv[++i];
            }
        }
                       

        // The ORB must be initialized before configuring logging, which may use
        // CORBA to get its configuration file. Devices do not need persistent IORs.
        ossie::corba::CorbaInit(argc, argv);

        // configure logging
          ossie::configureLogging(logging_config_uri, debug_level);

        if ((devMgr_ior == 0) || (id == 0) || (profile == 0) || (label == 0)) {
            LOG_FATAL(Device_impl, "Per SCA specification SR:478, DEVICE_MGR_IOR, PROFILE_NAME, DEVICE_ID, and DEVICE_LABEL must be provided");
            exit(-1);
        }

        LOG_DEBUG(Device_impl, "Identifier = " << id << "Label = " << label << " Profile = " << profile << " IOR = " << devMgr_ior)

        /** Ignore SIGInterrupt because when you CTRL-C the node
        booter we don't want the device to die, and it's the shells responsibility
        to send CTRL-C to all foreground processes (even children) */
        //signal(SIGINT, SIG_IGN);
        // Associate SIGINT to signal_catcher interrupt handler
        if( sigaction( SIGINT, &sa, NULL ) == -1 ) {
            LOG_FATAL(Device_impl, "SIGINT association failed");
            exit(EXIT_FAILURE);
        }

        // Associate SIGQUIT to signal_catcher interrupt handler
        if( sigaction( SIGQUIT, &sa, NULL ) == -1 ) {
            LOG_FATAL(Device_impl, "SIGQUIT association failed");
            exit(EXIT_FAILURE);
        }

        // Associate SIGTERM to signal_catcher interrupt handler
        if( sigaction( SIGTERM, &sa, NULL ) == -1 ) {
            LOG_FATAL(Device_impl, "SIGTERM association failed");
            exit(EXIT_FAILURE);
        }

        if (composite_device != 0) {
            // The AggregateDevice version of the constructor implicitly activates the new device,
            // which may be an unsafe behavior.
            *devPtr = new T(devMgr_ior, id, label, profile, composite_device);
        } else {
            *devPtr = new T(devMgr_ior, id, label, profile);
            PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(*devPtr);
        }
        
        // setting all the execparams passed as argument, this method resides in the Resource_impl class
        (*devPtr)->setExecparamProperties(execparams);

#if ENABLE_EVENTS
        if (idm_channel_ior) {
            try {
                CORBA::Object_var IDM_channel_obj = ossie::corba::Orb()->string_to_object(idm_channel_ior);
                if (CORBA::is_nil(IDM_channel_obj)) {
                    LOG_ERROR(Device_impl, "Invalid IDM channel IOR: " << idm_channel_ior);
                } else {
                    CosEventChannelAdmin::EventChannel_var idm_channel = CosEventChannelAdmin::EventChannel::_narrow(IDM_channel_obj);
                    (*devPtr)->connectSupplierToIncomingEventChannel(idm_channel);
                }
            }
            CATCH_LOG_WARN(Device_impl, "Unable to connect to IDM channel");
        }
#endif

        (*devPtr)->run();
        LOG_DEBUG(Device_impl, "Goodbye!");
        (*devPtr)->_remove_ref();
        ossie::corba::OrbShutdown(true);
    }

    static std::string getLogConfig(const char* devmgr_ior, const char* log_config, std::string& devmgr_label) {
        // connect to the device manager and copy the log config file to the local directory

        std::string _local_logconfig_path;

        // connect to device manager
        CF::DeviceManager_ptr _devMgr_ptr = CF::DeviceManager::_nil();
        CORBA::Object_var _devMgr_obj = ossie::corba::Orb()->string_to_object(devmgr_ior);
        if (CORBA::is_nil(_devMgr_obj)) {
            std::cout << "ERROR:Device_impl:getLogConfig - Invalid device manager IOR: " << devmgr_ior << std::endl;
            return _local_logconfig_path;
        }

        _devMgr_ptr = CF::DeviceManager::_narrow(_devMgr_obj);
        if (CORBA::is_nil(_devMgr_ptr)) {
            std::cout << "ERROR:Device_impl:getLogConfig - Could not narrow device manager IOR: " << devmgr_ior << std::endl;
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
            std::cout << "ERROR:Device_impl:getLogConfig - Could not copy file to local memory. File name: " << log_config << std::endl;
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
                std::cout << "ERROR:Device_impl:getLogConfig - Could not open log file on local system. File name: " << _local_logconfig_path << std::endl;
                throw;
            }

            _local_logconfig.write((const char*)logFileData->get_buffer(), logFileData->length());
            if (_local_logconfig.fail()) {
                std::cout << "ERROR:Device_impl:getLogConfig - Could not write log file on local system. File name: " << _local_logconfig_path << std::endl;
                throw;
            }
            _local_logconfig.close();
        } catch ( ... ) {
            std::cout << "ERROR:Device_impl:getLogConfig - Could not copy file to local system. File name: " << _local_logconfig_path << std::endl;
            _local_logconfig_path.clear();  // so calling function knows not to use value
            return _local_logconfig_path;
        }

        return _local_logconfig_path;
    }

protected:
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
    CF::Device::AdminType _adminState;
    CF::Device::UsageType _usageState;
    CF::Device::OperationalType _operationalState;
    CF::AggregateDevice_ptr _aggregateDevice;
    std::string _softwareProfile;
    std::string _label;
    std::string _compositeDev_ior;

#if ENABLE_EVENTS
    void connectSupplierToIncomingEventChannel (CosEventChannelAdmin::EventChannel_ptr idmChannel);;

    CosEventChannelAdmin::EventChannel_var IDM_channel;
    CosEventChannelAdmin::ProxyPushConsumer_var proxy_consumer;
#endif

    //AggregateDevice _compositeDevice;
    bool initialConfiguration;
    CF::Properties originalCap;
    void setUsageState (CF::Device::UsageType newUsageState);
    Device_impl::AnyComparisonType compareAnyToZero (CORBA::Any& first);
    Device_impl::AnyComparisonType compareAnys (CORBA::Any& first, CORBA::Any& second);
    void deallocate (CORBA::Any& deviceCapacity, const CORBA::Any& resourceRequest);
    bool allocate (CORBA::Any& deviceCapacity, const CORBA::Any& resourceRequest);

public:
    Device_impl (char*, char*, char*, char*);
    Device_impl (char*, char*, char*, char*, char*);
    Device_impl (char*, char*, char*, char*, CF::Properties& capacities);
    Device_impl (char*, char*, char*, char*, CF::Properties& capacities, char*);
    
    
    ~Device_impl ();
    void releaseObject () throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

    char* label () throw (CORBA::SystemException);
    char* softwareProfile () throw (CORBA::SystemException);
    CF::Device::UsageType usageState ()throw (CORBA::SystemException);
    CF::Device::AdminType adminState ()throw (CORBA::SystemException);
    CF::Device::OperationalType operationalState ()throw (CORBA::SystemException);
    CF::AggregateDevice_ptr compositeDevice ()throw (CORBA::SystemException);
    void setAdminState (CF::Device::AdminType _adminType);
    void adminState (CF::Device::AdminType _adminType) throw (CORBA::SystemException);
    void deallocateCapacity (const CF::Properties& capacities) throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CORBA::SystemException);
    CORBA::Boolean allocateCapacity (const CF::Properties& capacities) throw (CF::Device::InvalidState, CF::Device::InvalidCapacity, CF::Device::InsufficientCapacity, CORBA::SystemException);
    bool isUnlocked ();
    bool isLocked ();
    bool isDisabled ();
    bool isBusy ();
    bool isIdle ();
    void configure (const CF::Properties& configProperties) throw (CF::PropertySet::PartialConfiguration, CF::PropertySet::InvalidConfiguration, CORBA::SystemException);

    virtual void run ();
    virtual void halt ();

    Device_impl(); // Code that tries to use this constructor will not work
    Device_impl(Device_impl&); // No copying

protected:
    std::string _devMgr_ior;
private:
    friend class IDM_Channel_Supplier_i;
    void initResources(char*, char*, char*, char*);
};


#endif

