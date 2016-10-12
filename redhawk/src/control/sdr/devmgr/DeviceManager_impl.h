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

#include <string>
#include <map>

#include <boost/thread/recursive_mutex.hpp>

#include <COS/CosEventChannelAdmin.hh>

#include <ossie/ComponentDescriptor.h>
#include <ossie/ossieSupport.h>
#include <ossie/DeviceManagerConfiguration.h>
#include <ossie/PropertySet_impl.h>
#include <ossie/PortSupplier_impl.h>
#include <ossie/FileManager_impl.h>
#include <ossie/Properties.h>
#include <ossie/SoftPkg.h>

#include <dirent.h>

class DeviceManager_impl: 
    public virtual POA_CF::DeviceManager,
    public PropertySet_impl,
    public PortSupplier_impl
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

    struct ServiceNode{
        std::string identifier;
        std::string label;
        std::string IOR;
        CORBA::Object_ptr service;
        pid_t pid;
    };

    typedef std::vector<DeviceNode*> DeviceList;
    typedef std::vector<ServiceNode*> ServiceList;

    // Devices that are registered with this DeviceManager.
    DeviceList _registeredDevices;
    ServiceList _registeredServices;

    // Devices that were launched by this DeviceManager, but are either waiting for
    // registration or process termination.
    DeviceList _pendingDevices;
    ServiceList _pendingServices;

    // Properties
    std::string     logging_config_uri;
    StringProperty* logging_config_prop;
    std::string     HOSTNAME;
    std::string logging_uri;
    
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
    bool checkWriteAccess(std::string &path);

    enum DevMgrAdmnType {
        DEVMGR_REGISTERED,
        DEVMGR_UNREGISTERED,
        DEVMGR_SHUTTING_DOWN,
        DEVMGR_SHUTDOWN
    };
    
    DevMgrAdmnType        _adminState;
    CF::DomainManager_var _dmnMgr;

    void killPendingDevices(int signal, int timeout);
    void abort();

    void getDevManImpl(
        const ossie::SPD::Implementation*& devManImpl,
        ossie::SoftPkg&                    devmgrspdparser);
    
    void checkDeviceConfigurationProfile();
    void parseDCDProfile(
        ossie::DeviceManagerConfiguration& DCDParser,
        const char*                        overrideDomainName);

    void resolveNamingContext();
    void bindNamingContext();

    void parseSpd(
        const ossie::DeviceManagerConfiguration& DCDParser,
        ossie::SoftPkg&                          devmgrspdparser);

    void getDomainManagerReferenceAndCheckExceptions();

    bool storeCommonDevicePrfLocation(
        ossie::SoftPkg&    SPDParser,
        ossie::Properties& deviceProperties);

    bool storeImplementationSpecificDevicePrfLocation(
        ossie::SoftPkg&                    SPDParser,
        ossie::Properties&                 deviceProperties,
        const ossie::SPD::Implementation*& matchedDeviceImpl);

    bool getCodeFilePath(
        std::string&                       codeFilePath,
        const ossie::SPD::Implementation*& matchedDeviceImpl,
        ossie::SoftPkg&                    SPDParser,
        FileSystem_impl*&                  fs_servant);

    bool joinDevicePropertiesFromPRFs(
        ossie::SoftPkg&                    SPDParser,
        ossie::Properties&                 deviceProperties,
        const ossie::SPD::Implementation*& matchedDeviceImpl);

    void registerDeviceManagerWithDomainManager(
        CF::DeviceManager_var& my_object_var);

    void getCompositeDeviceIOR(
        std::string&                                  compositeDeviceIOR, 
        const std::vector<ossie::ComponentPlacement>& componentPlacements,
        const ossie::ComponentPlacement&              componentPlacementInst);

    bool loadDeviceProperties (
        const ossie::SoftPkg& softpkg, 
        const ossie::SPD::Implementation& deviceImpl, 
        ossie::Properties& deviceProperties);

    bool joinPRFProperties (
        const std::string& prfFile, 
        ossie::Properties& properties);
    
    void getOverloadprops(
        std::map<std::string, std::string>&           overloadprops, 
        const std::vector<ossie::ComponentProperty*>& instanceprops,
        const ossie::Properties&                      deviceProperties);

    bool loadScdToParser(
        ossie::ComponentDescriptor& scdParser, 
        const ossie::SoftPkg&       _SPDParser);
    
    DeviceNode* getDeviceNode(const pid_t pid);

    bool getDeviceOrService(
        std::string& type, 
        const ossie::ComponentDescriptor& scdParser);

    void createDeviceCacheLocation(
        std::string&                         devcache,
        std::string&                         usageName, 
        const ossie::ComponentInstantiation& instantiation);

    void createDeviceExecStatement(
        const char*                                   new_argv[], 
        const ossie::ComponentPlacement&              componentPlacement,
        const std::string&                            componentType,
        std::map<std::string, std::string>*           pOverloadprops,
        const std::string&                            codeFilePath,
        ossie::DeviceManagerConfiguration&            DCDParser,
        const ossie::ComponentInstantiation&          instantiation,
        const std::string&                            usageName,
        const std::vector<ossie::ComponentPlacement>& componentPlacements,
        const std::string&                            compositeDeviceIOR,
        const std::vector<ossie::ComponentProperty*>& instanceprops) ;

    void createDeviceThreadAndHandleExceptions(
        const ossie::ComponentPlacement&              componentPlacement,
        const std::string&                            componentType,
        std::map<std::string, std::string>*           pOverloadprops,
        const std::string&                            codeFilePath,
        const ossie::SoftPkg&                         SPDParser,
        ossie::DeviceManagerConfiguration&            DCDParser,
        const ossie::ComponentInstantiation&          instantiation,
        const std::vector<ossie::ComponentPlacement>& componentPlacements,
        const std::string&                            compositeDeviceIOR,
        const std::vector<ossie::ComponentProperty*>& instanceprops);

    void createDeviceThread(
        const ossie::ComponentPlacement&              componentPlacement,
        const std::string&                            componentType,
        std::map<std::string, std::string>*           pOverloadprops,
        const std::string&                            codeFilePath,
        const ossie::SoftPkg&                         SPDParser,
        ossie::DeviceManagerConfiguration&            DCDParser,
        const ossie::ComponentInstantiation&          instantiation,
        const std::string&                            devcache,
        const std::string&                            usageName,
        const std::vector<ossie::ComponentPlacement>& componentPlacements,
        const std::string&                            compositeDeviceIOR,
        const std::vector<ossie::ComponentProperty*>& instanceprops);

    typedef std::list<std::pair<std::string,std::string> > ExecparamList;

    ExecparamList createDeviceExecparams(
        const ossie::ComponentPlacement&              componentPlacement,
        const std::string&                            componentType,
        std::map<std::string, std::string>*           pOverloadprops,
        const std::string&                            codeFilePath,
        ossie::DeviceManagerConfiguration&            DCDParser,
        const ossie::ComponentInstantiation&          instantiation,
        const std::string&                            usageName,
        const std::string&                            compositeDeviceIOR,
        const std::vector<ossie::ComponentProperty*>& instanceprops);

    bool loadSPD(
        ossie::SoftPkg&                    SPDParser,
        ossie::DeviceManagerConfiguration& DCDParser,
        const ossie::ComponentPlacement&   componentPlacement);

    void recordComponentInstantiationId(
        const ossie::ComponentInstantiation& instantiation,
        const ossie::SPD::Implementation*&   matchedDeviceImpl);

    bool deviceIsRegistered (CF::Device_ptr);
    bool serviceIsRegistered (const char*);
    void getDomainManagerReference(const std::string&);

    // this mutex is used for synchronizing _registeredDevices, _pendingDevices, and _registeredServices
    boost::recursive_mutex registeredDevicesmutex;  
    boost::condition_variable_any pendingDevicesEmpty;
    void increment_registeredDevices(CF::Device_ptr registeringDevice);
    void increment_registeredServices(CORBA::Object_ptr registeringService, 
                                      const char* name);
    bool decrement_registeredDevices(CF::Device_ptr registeredDevice);
    bool decrement_registeredServices(CORBA::Object_ptr registeringService, 
                                      const char* name);
    void clean_registeredDevices();
    void clean_registeredServices();
    void clean_externalServices();

    void local_unregisterService(CORBA::Object_ptr service, const std::string& name);
    void local_unregisterDevice(CF::Device_ptr device, const std::string& name);

    const ossie::SPD::Implementation* locateMatchingDeviceImpl(
        const ossie::SoftPkg&             devSpd, 
        const ossie::SPD::Implementation* deployonImpl);

    bool checkProcessorAndOs(
        const ossie::SPD::Implementation&          devMan, 
        const std::vector<const ossie::Property*>& props);

    void deleteFileSystems();
    bool makeDirectory(std::string path);
    std::string getIORfromID(const char* instanceid);
    std::string deviceMgrIOR;
    std::string fileSysIOR;
    bool *_internalShutdown;

    bool skip_fork;

    // this mutex is used for synchronizing _registeredDevices, labelTable, and identifierTable
    boost::mutex componentImplMapmutex;  

    std::map<std::string, std::string> _componentImplMap;

    CosEventChannelAdmin::EventChannel_var IDM_channel;
    std::string IDM_IOR;

};

#endif                                            /* __DEVICEMANAGER_IMPL__ */

