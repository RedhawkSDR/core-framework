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

#include <COS/CosEventComm.hh>
#include <COS/CosEventChannelAdmin.hh>

#include "Resource_impl.h"
#include "CF/cf.h"
#include "ossie/debug.h"
#include "ossie/CorbaUtils.h"
#include <signal.h>

class Device_impl;

class IDM_Channel_Supplier_i : virtual public POA_CosEventComm::PushSupplier
{

public:
    IDM_Channel_Supplier_i (Device_impl *_dev);
    void disconnect_push_supplier ();

private:
    Device_impl *_device;
    
};


class Device_impl: public virtual POA_CF::Device, public Resource_impl
{
    ENABLE_LOGGING

public:
    template<class T>
    static void start_device(T** devPtr, struct sigaction sa, int argc, char* argv[]) {
        start_device(boost::bind(&Device_impl::make_device<T>,boost::ref(*devPtr),_1,_2,_3,_4,_5), sa, argc, argv);
    }

    static std::string getLogConfig(const char* devmgr_ior, const char* log_config, std::string& devmgr_label);

    Device_impl (char*, char*, char*, char*);
    Device_impl (char*, char*, char*, char*, char*);
    Device_impl (char*, char*, char*, char*, CF::Properties& capacities);
    Device_impl (char*, char*, char*, char*, CF::Properties& capacities, char*);
    
    
    ~Device_impl ();
    void releaseObject () throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

    char* label () throw (CORBA::SystemException);
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
    std::string _label;
    std::string _compositeDev_ior;

    void connectSupplierToIncomingEventChannel (CosEventChannelAdmin::EventChannel_ptr idmChannel);;

    CosEventChannelAdmin::EventChannel_var IDM_channel;
    CosEventChannelAdmin::ProxyPushConsumer_var proxy_consumer;

    //AggregateDevice _compositeDevice;
    bool initialConfiguration;
    CF::Properties originalCap;
    void setUsageState (CF::Device::UsageType newUsageState);
    Device_impl::AnyComparisonType compareAnyToZero (CORBA::Any& first);
    Device_impl::AnyComparisonType compareAnys (CORBA::Any& first, CORBA::Any& second);
    void deallocate (CORBA::Any& deviceCapacity, const CORBA::Any& resourceRequest);
    bool allocate (CORBA::Any& deviceCapacity, const CORBA::Any& resourceRequest);

    void updateUsageState ();

    template <typename T>
    void setAllocationImpl (const std::string& id, typename PropertyWrapper<T>::Allocator allocator,
                        typename PropertyWrapper<T>::Deallocator deallocator)
    {
        useNewAllocation = true;
        try {
            PropertyWrapper<T>* wrapper = getAllocationPropertyById<T>(id);
            wrapper->setAllocator(allocator); 
            wrapper->setDeallocator(deallocator); 
       } catch (const std::invalid_argument& error) {
            LOG_WARN(Device_impl, "Cannot set allocation implementation: " << error.what());
        }
    }

    template <typename T>
    void setAllocationImpl (const std::string& id, bool (*alloc)(const T&), void (*dealloc)(const T&))
    {
        typename PropertyWrapper<T>::Allocator allocator = alloc;
        typename PropertyWrapper<T>::Deallocator deallocator = dealloc;
        setAllocationImpl<T>(id, allocator, deallocator);
    }

    template <class C, typename T>
    void setAllocationImpl (const std::string& id, C* target, bool (C::*alloc)(const T&),
                        void (C::*dealloc)(const T&))
    {
        typename PropertyWrapper<T>::Allocator allocator;
        allocator = boost::bind(alloc, target, _1);
        typename PropertyWrapper<T>::Deallocator deallocator;
        deallocator = boost::bind(dealloc, target, _1);
        setAllocationImpl<T>(id, allocator, deallocator);
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

    std::string _devMgr_ior;

private:
    friend class IDM_Channel_Supplier_i;

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
};


#endif

