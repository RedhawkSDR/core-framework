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


#ifndef DEVICE_MANAGER_IMPL_H
#define DEVICE_MANAGER_IMPL_H

#if ENABLE_EVENTS
#include <COS/CosEventChannelAdmin.hh>
#endif

#include "ossieSupport.h"

#include "PropertySet_impl.h"
#include "PortSupplier_impl.h"
#include "FileManager_impl.h"
#include "Properties.h"
#include "SoftPkg.h"
#include <string>
#include <map>
#include <sys/utsname.h>

#include <boost/thread/recursive_mutex.hpp>

class OSSIECF_API DeviceManager_impl;

class OSSIECF_API DeviceManager_impl: public virtual
    POA_CF::DeviceManager,
    public
    PropertySet_impl,
    public
    PortSupplier_impl
{
    ENABLE_LOGGING

public:
    DeviceManager_impl (const char*, const char*, const char*, const char*, struct utsname uname, bool *);

    // Run this after the constructor and the caller has created the object reference
    void post_constructor(CF::DeviceManager_var, const char*) throw (CORBA::SystemException, std::runtime_error);

    ~
    DeviceManager_impl ();
    char* deviceConfigurationProfile ()
        throw (CORBA::SystemException);

    CF::FileSystem_ptr fileSys ()
        throw (CORBA::SystemException);

    char* identifier ()
        throw (CORBA::SystemException);

    char* label ()
        throw (CORBA::SystemException);

    CF::DeviceSequence * registeredDevices ()
        throw (CORBA::SystemException);

    CF::DeviceManager::ServiceSequence * registeredServices ()
        throw (CORBA::SystemException);

    void registerDevice (CF::Device_ptr registeringDevice)
        throw (CF::InvalidObjectReference, CORBA::SystemException);

    void unregisterDevice (CF::Device_ptr registeredDevice)
        throw (CF::InvalidObjectReference, CORBA::SystemException);

    void shutdown ()
        throw (CORBA::SystemException);
    void _local_shutdown ()
        throw (CORBA::SystemException);

    void registerService (CORBA::Object_ptr registeringService, const char* name)
        throw (CF::InvalidObjectReference, CORBA::SystemException);

    void unregisterService (CORBA::Object_ptr registeredService, const char* name)
        throw (CF::InvalidObjectReference, CORBA::SystemException);

    char* getComponentImplementationId (const char* componentInstantiationId)
        throw (CORBA::SystemException);
    char* _local_getComponentImplementationId (const char* componentInstantiationId)
        throw (CORBA::SystemException);

    void childExited (pid_t pid, int status);

    bool allChildrenExited ();

private:
    DeviceManager_impl ();   // No default constructor
    DeviceManager_impl(DeviceManager_impl&);  // No copying

    struct DeviceNode {
        std::string identifier;
        std::string label;
        std::string IOR;
        CF::Device_var device;
        pid_t pid;
    };

    typedef std::vector<DeviceNode*> DeviceList;

    // Devices that are registered with this DeviceManager.
    DeviceList _registeredDevices;

    // Devices that were launched by this DeviceManager, but are either waiting for
    // registration or process termination.
    DeviceList _pendingDevices;

    // Properties
    std::string logging_config_uri;
    StringProperty* logging_config_prop;

// read only attributes
    struct utsname _uname;
    std::string _identifier;
    std::string _label;
    std::string _deviceConfigurationProfile;
    std::string _fsroot;
    std::string _cacheroot;

    std::string _domainName;
    std::string _domainManagerName;
    CosNaming::Name_var base_context;
    CosNaming::NamingContext_var rootContext;
    CosNaming::NamingContext_var devMgrContext;
    CF::FileSystem_var _fileSys;
    CF::DeviceManager_var myObj;
    CF::DeviceManager::ServiceSequence _registeredServices;

    ossie::SPD::Implementation _devManImpl;

    enum DevMgrAdmnType {
        DEVMGR_REGISTERED,
        DEVMGR_UNREGISTERED,
        DEVMGR_SHUTTING_DOWN,
        DEVMGR_SHUTDOWN
    };
    
    std::string HOSTNAME;

    DevMgrAdmnType _adminState;
    CF::DomainManager_var _dmnMgr;

    bool deviceIsRegistered (CF::Device_ptr);
    bool serviceIsRegistered (const char*);
    void getDomainManagerReference(const std::string&);

    boost::recursive_mutex registeredDevicesmutex;  // this mutex is used for synchronizing _registeredDevices and _pendingDevices
    void increment_registeredDevices(CF::Device_ptr registeringDevice);
    bool decrement_registeredDevices(CF::Device_ptr registeredDevice);
    //bool _base_decrement_registeredDevices(CF::Device_ptr registeredDevice);
    void clean_registeredDevices();

    const ossie::SPD::Implementation* locateMatchingDeviceImpl(const ossie::SoftPkg& devSpd, const ossie::SPD::Implementation* deployonImpl);
    bool checkProcessorAndOs(const ossie::SPD::Implementation& devMan, const std::vector<const ossie::Property*>& props);
    void deleteFileSystems();
    void makeDirectory(std::string path);
    std::string getIORfromID(const char* instanceid);
    std::string deviceMgrIOR;
    std::string fileSysIOR;
    bool *_internalShutdown;

    boost::mutex componentImplMapmutex;  // this mutex is used for synchronizing _registeredDevices, labelTable, and identifierTable
    std::map<std::string, std::string> _componentImplMap;

#if ENABLE_EVENTS
    CosEventChannelAdmin::EventChannel_var IDM_channel;
    std::string IDM_IOR;
#endif

};

#endif                                            /* __DEVICEMANAGER_IMPL__ */

