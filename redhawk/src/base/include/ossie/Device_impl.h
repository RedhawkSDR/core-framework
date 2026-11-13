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

#include <signal.h>
#include <string>
#include <iostream>
#include <fstream>
#include <map>

#include "Resource_impl.h"
#include "CF/cf.h"
#include "ossie/debug.h"
#include "ossie/Events.h"
#include "ossie/CorbaUtils.h"
#include "ossie/Autocomplete.h"

class Device_impl;

class Device_impl:
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
    public virtual POA_CF::Device, 
#endif
    public Resource_impl
{
    ENABLE_LOGGING

public:

    // Return the Log configuration file
    static std::string getLogConfig(const char* devmgr_ior, const char* log_config, std::string& devmgr_label);
    Device_impl (char*, char*, char*, char*);
    Device_impl (char*, char*, char*, char*, char*);
    Device_impl (char*, char*, char*, char*, CF::Properties& capacities);
    Device_impl (char*, char*, char*, char*, CF::Properties& capacities, char*);
    ~Device_impl ();

    template<class T>
    static void start_device(T** devPtr, struct sigaction sa, int argc, char* argv[]) {
        start_device(boost::bind(&Device_impl::make_device<T>,boost::ref(*devPtr),_1,_2,_3,_4,_5), sa, argc, argv);
    }
    virtual void halt ();
    void releaseObject ();
    char* label ();
    CF::Device::UsageType usageState ();
    CF::Device::AdminType adminState ();
    CF::Device::OperationalType operationalState ();
    CF::AggregateDevice_ptr compositeDevice ();
    void adminState (CF::Device::AdminType _adminType);
    void deallocateCapacity (const CF::Properties& capacities);
    CORBA::Boolean allocateCapacity (const CF::Properties& capacities);
    void configure (const CF::Properties& configProperties);
    // resolve domain awareness
    void setAdditionalParameters(std::string &softwareProfile, std::string &application_registrar_ior, const std::string &nic);
    virtual void run ();
    const CF::DeviceManager_ptr getDeviceManager() const ;

    // Returns true if the Device is in an unlocked state
    bool isUnlocked ();
    // Returns true if the Device is in a locked state
    bool isLocked ();
    // Returns true if the Device is a disabled state
    bool isDisabled ();
    // Returns true if the Device is a busy state
    bool isBusy ();
    // Returns true if the Device is an idle state
    bool isIdle ();
    // Set admin state (LOCKED, SHUTTING_DOWN, UNLOCKED)
    void setAdminState (CF::Device::AdminType _adminType);

    void setLogger(rh_logger::LoggerPtr logptr);

protected:
    // Admin state (LOCKED, SHUTTING_DOWN, UNLOCKED)
    CF::Device::AdminType _adminState;
    // Admin state (IDLE, ACTIVE, BUSY)
    CF::Device::UsageType _usageState;
    // Admin state (ENABLED, DISABLED)
    CF::Device::OperationalType _operationalState;
    // Pointer to this child's parent (CF::AggregateDevice::nil otherwise)
    CF::AggregateDevice_ptr _aggregateDevice;
    // Device label
    std::string _label;
    // String pointer to this child's parent (empty string otherwise
    std::string _compositeDev_ior;

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
    redhawk::events::PublisherPtr  idm_publisher;
    int                            sig_fd;
    //
    // call after device has been created and assigned exec params
    //
    virtual void  postConstruction( std::string &softwareProfile,
                                    std::string &registrar_ior,
                                    const std::string &idm_channel_ior="",
                                    const std::string &nic="",
                                    const int  sigfd=-1 );
    // resolve domain context for this device, what domain and device manager am I associated with
    void  resolveDomainContext();

    //
    // Support for publishing state changes
    //
    void sendStateChange( StandardEvent::StateChangeType &fromState, 
                          StandardEvent::StateChangeType &toState,
                          StandardEvent::StateChangeCategoryType category );
    void connectIDMChannel( const std::string &idm_ior="" );
    bool initialConfiguration;
    CF::Properties originalCap;
    void deallocate (CORBA::Any& deviceCapacity, const CORBA::Any& resourceRequest);
    bool allocate (CORBA::Any& deviceCapacity, const CORBA::Any& resourceRequest);
    Device_impl::AnyComparisonType compareAnyToZero (CORBA::Any& first);
    Device_impl::AnyComparisonType compareAnys (CORBA::Any& first, CORBA::Any& second);
    std::string _devMgr_ior;

    // Change the value of _usageState
    void setUsageState (CF::Device::UsageType newUsageState);

    // Function that is called when the usage state for the Device should be re-evaluated
    void updateUsageState ();

    template <typename T>
    void setAllocationImpl (const std::string& id, bool (*alloc)(const T&), void (*dealloc)(const T&))
    {
        useNewAllocation = true;
        try {
            PropertyWrapper<T>* wrapper = getAllocationPropertyById<T>(id);
            wrapper->setAllocator(alloc);
            wrapper->setDeallocator(dealloc);
        } catch (const std::exception& error) {
            LOG_WARN(Device_impl, "Cannot set allocation implementation: " << error.what());
        }
    }

    template <class C, typename T>
    void setAllocationImpl (const std::string& id, C* target, bool (C::*alloc)(const T&),
                            void (C::*dealloc)(const T&))
    {
        useNewAllocation = true;
        try {
            PropertyWrapper<T>* wrapper = getAllocationPropertyById<T>(id);
            wrapper->setAllocator(target, alloc);
            wrapper->setDeallocator(target, dealloc);
        } catch (const std::exception& error) {
            LOG_WARN(Device_impl, "Cannot set allocation implementation: " << error.what());
        }
    }

    template <typename Alloc, typename Dealloc>
    void setAllocationImpl (const char* id, Alloc alloc, Dealloc dealloc)
    {
        setAllocationImpl(std::string(id), alloc, dealloc);
    }

    template <typename Target, typename Alloc, typename Dealloc>
    void setAllocationImpl (const char* id, Target target, Alloc alloc, Dealloc dealloc)
    {
        setAllocationImpl(std::string(id), target, alloc, dealloc);
    }
    
    template <typename T>
    PropertyWrapper<T>* getAllocationPropertyById (const std::string& id)
    {
        PropertyWrapper<T>* property = getPropertyWrapperById<T>(id);
        if (!property->isAllocatable()) {
            throw std::invalid_argument("Property '" + id + "' is not allocatable");
        }
        return property;
    }


    // Associate a function callback with an allocation request against a property
    template <typename T, typename Alloc, typename Dealloc>
    void setAllocationImpl (T& value, Alloc alloc, Dealloc dealloc)
    {
        useNewAllocation = true;
        try {
            PropertyWrapper<T>* wrapper = getPropertyWrapper(value);
            wrapper->setAllocator(alloc);
            wrapper->setDeallocator(dealloc);
        } catch (const std::exception& error) {
            LOG_WARN(Device_impl, "Cannot set allocation implementation: " << error.what());
        }
    }

    // Associate a function callback on a target object (usually this) with an allocation request against a property
    template <typename T, typename Target, typename Alloc, typename Dealloc>
    void setAllocationImpl (T& value, Target target, Alloc alloc, Dealloc dealloc)
    {
        useNewAllocation = true;
        try {
            PropertyWrapper<T>* wrapper = getPropertyWrapper(value);
            wrapper->setAllocator(target, alloc);
            wrapper->setDeallocator(target, dealloc);
        } catch (const std::exception& error) {
            LOG_WARN(Device_impl, "Cannot set allocation implementation: " << error.what());
        }
    }

    // Return a container with a pointer to the Device Manager hosting this Device
    redhawk::DeviceManagerContainer* getDeviceManager() {
        return this->_devMgr;
    }

    rh_logger::LoggerPtr _deviceLog;

private:
    // Adapter template function for device constructors. This is the only part of
    // device creation that requires type-specific knowledge.
    template <class T>
    static Device_impl* make_device(T*& device, char* devMgrIOR, char* identifier, char* label,
                                    char* profile, char* compositeDeviceIOR)
    {
        if (compositeDeviceIOR) {
            // The AggregateDevice version of the constructor implicitly activates the new device,
            // which may be an unsafe behavior.
            device = new T(devMgrIOR, identifier, label, profile, compositeDeviceIOR);
        } else {
            device = new T(devMgrIOR, identifier, label, profile);
            PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(device);
        }
        return device;
    }
    // Generic implementation of start_device, taking a function pointer to
    // a component constructor (via make_device).
    typedef boost::function<Device_impl* (char*, char*, char*, char*, char*)> ctor_type;
    static void start_device(ctor_type ctor, struct sigaction sa, int argc, char* argv[]);
    void initResources(char*, char*, char*, char*);
    // Check for valid allocation properties
    void validateCapacities (const CF::Properties& capacities);
    // New per-property callback-based capacity management
    bool useNewAllocation;
    bool allocateCapacityNew (const CF::Properties& capacities);
    void deallocateCapacityNew (const CF::Properties& capacities);
    // Legacy capacity management
    bool allocateCapacityLegacy (const CF::Properties& capacities);
    void deallocateCapacityLegacy (const CF::Properties& capacities);
    // container to the Device Manager
    redhawk::DeviceManagerContainer *_devMgr;


 private:
    Device_impl(); // Code that tries to use this constructor will not work
    Device_impl(Device_impl&); // No copying

};


#endif

