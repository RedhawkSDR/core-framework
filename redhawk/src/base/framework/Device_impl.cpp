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
#include <string.h>

#include "ossie/Device_impl.h"
#include "ossie/CorbaUtils.h"

#if ENABLE_EVENTS
#include "ossie/EventChannelSupport.h"
#endif

PREPARE_LOGGING(Device_impl)


void Device_impl::initResources (char* devMgr_ior, char* _id, 
                          char* lbl, char* sftwrPrfl)
{
    _label = lbl;
    _softwareProfile = sftwrPrfl;
    _devMgr_ior = devMgr_ior;
    _compositeDev_ior = "";

    _aggregateDevice = CF::AggregateDevice::_nil();
    _usageState = CF::Device::IDLE;
    _operationalState = CF::Device::ENABLED;
    _adminState = CF::Device::UNLOCKED;
    initialConfiguration = true;
}                          


Device_impl::Device_impl (char* devMgr_ior, char* _id, char* lbl, char* sftwrPrfl) : Resource_impl(_id)
{
    LOG_TRACE(Device_impl, "Constructing Device")
    initResources(devMgr_ior, _id, lbl, sftwrPrfl);
    LOG_TRACE(Device_impl, "Done Constructing Device")
}


Device_impl::Device_impl (char* devMgr_ior, char* _id, char* lbl, char* sftwrPrfl,
                          CF::Properties& capacities) : Resource_impl(_id)
{
    LOG_TRACE(Device_impl, "Constructing Device")
    initResources(devMgr_ior, _id, lbl, sftwrPrfl);
    configure (capacities);
    LOG_TRACE(Device_impl, "Done Constructing Device")
}

Device_impl::Device_impl (char* devMgr_ior, char* _id, char* lbl, char* sftwrPrfl,
                          CF::Properties& capacities, char* compositeDev_ior) : Resource_impl(_id)
{

    LOG_TRACE(Device_impl, "Constructing Device")
    CORBA::Object_var _aggDev_obj = ossie::corba::Orb()->string_to_object(_compositeDev_ior.c_str());
    if (CORBA::is_nil(_aggDev_obj)) {
        LOG_ERROR(Device_impl, "Invalid composite device IOR: " << _compositeDev_ior);
    } else {
        _aggregateDevice = CF::AggregateDevice::_narrow(_aggDev_obj);
        _aggregateDevice->addDevice(this->_this());
    }

    initResources(devMgr_ior, _id, lbl, sftwrPrfl);
    configure (capacities);
    LOG_TRACE(Device_impl, "Done Constructing Device")
}

Device_impl::Device_impl (char* devMgr_ior, char* _id, char* lbl, char* sftwrPrfl,
                          char* compositeDev_ior) : Resource_impl(_id)
{

    LOG_TRACE(Device_impl, "Constructing Device")
    CORBA::Object_var _aggDev_obj = ossie::corba::Orb()->string_to_object(_compositeDev_ior.c_str());
    if (CORBA::is_nil(_aggDev_obj)) {
        LOG_ERROR(Device_impl, "Invalid composite device IOR: " << _compositeDev_ior);
    } else {
        _aggregateDevice = CF::AggregateDevice::_narrow(_aggDev_obj);
        _aggregateDevice->addDevice(this->_this());
    }

    initResources(devMgr_ior, _id, lbl, sftwrPrfl);
    LOG_TRACE(Device_impl, "Done Constructing Device")
}


void  Device_impl::run ()
{
    _deviceManager = CF::DeviceManager::_nil();
    CORBA::Object_var obj = ossie::corba::Orb()->string_to_object(_devMgr_ior.c_str());
    if (CORBA::is_nil(obj)) {
        LOG_ERROR(Device_impl, "Invalid device manager IOR");
        exit(-1);
    }
    _deviceManager = CF::DeviceManager::_narrow(obj);
    if (CORBA::is_nil(_deviceManager)) {
        LOG_ERROR(Device_impl, "Could not narrow device manager IOR");
        exit(-1);
    }

    _deviceManager->registerDevice(this->_this());
 
    Resource_impl::run(); // This won't return until halt is called
}

void  Device_impl::halt ()
{
    LOG_DEBUG(Device_impl, "Halting Device")
    if (not _adminState == CF::Device::LOCKED) {
        return;
    }
    Resource_impl::halt();
}

void
Device_impl::releaseObject ()
throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // SR:419
    LOG_DEBUG(Device_impl, "Receive releaseObject call");
    if (_adminState == CF::Device::UNLOCKED) {
        LOG_DEBUG(Device_impl, "Releasing Device")
        setAdminState(CF::Device::SHUTTING_DOWN);

        // SR:418
        // TODO Release aggregate devices if more than one exists
        if (!CORBA::is_nil(_aggregateDevice)) {
            try {
                _aggregateDevice->removeDevice(this->_this());
            } catch (...) {
            }
        }

        // SR:420
        setAdminState(CF::Device::LOCKED);
        try {
            // SR:422
            LOG_DEBUG(Device_impl, "Unregistering Device")
            _deviceManager->unregisterDevice(this->_this());
        } catch (...) {
            // SR:423
            throw CF::LifeCycle::ReleaseError();
        }
        LOG_DEBUG(Device_impl, "Done Releasing Device")
    }
    
    PortableServer::POA_ptr root_poa = ossie::corba::RootPOA();
    PortableServer::ObjectId_var oid = root_poa->servant_to_id(this);
    root_poa->deactivate_object(oid);
}


Device_impl::~Device_impl ()
{
}


/* Alternate implementation*/
CORBA::Boolean Device_impl::allocateCapacity (const CF::Properties& capacities)
throw (CORBA::SystemException, CF::Device::InvalidCapacity, CF::Device::InvalidState, CF::Device::InsufficientCapacity)
{
    LOG_TRACE(Device_impl, "in allocateCapacity");

    CF::Properties currentCapacities;

    bool extraCap = false;  /// Flag to check remaining extra capacity to allocate
    bool foundProperty;     /// Flag to indicate if the requested property was found

    if (capacities.length() == 0) {
        // Nothing to do, return
        LOG_TRACE(Device_impl, "no capacities to configure.");
        return true;
    }

    // Verify that the device is in a valid state
    if (!isUnlocked () || isDisabled ()) {
        LOG_WARN(Device_impl, "Cannot allocate capacity: System is either LOCKED, SHUTTING DOWN, or DISABLED.");
        throw (CF::Device::InvalidState("Cannot allocate capacity. System is either LOCKED, SHUTTING DOWN or DISABLED."));
    }
    if (!isBusy ()) {
        // The try is just a formality in this case
        try {
            // Get all properties currently in device
            query (currentCapacities);
        } catch (CF::UnknownProperties) {
        }

        /* Look in propertySet for the properties requested */
        for (unsigned i = 0; i < capacities.length (); i++) {
            foundProperty = false;

            for (unsigned j = 0; j < currentCapacities.length (); j++) {
                LOG_TRACE(Device_impl, "Comparing IDs: " << capacities[i].id << ", " << currentCapacities[j].id );
                if (strcmp (capacities[i].id, currentCapacities[j].id) == 0) {
                    // Verify that both values have the same type
                    if (!ossie::corba::isValidType (currentCapacities[j].value, capacities[i].value)) {
                        LOG_ERROR(Device_impl, "Cannot allocate capacity: Incorrect data type.");
                        throw (CF::Device::InvalidCapacity("Cannot allocate capacity. Incorrect Data Type.", capacities));
                    } else {
                        // Check for sufficient capacity and allocate it
                        if (!allocate (currentCapacities[j].value, capacities[i].value)) {
                            LOG_ERROR(Device_impl, "Cannot allocate capacity: Insufficient capacity.");
                            return false;
                        }
                        CORBA::Short capValue;
                        currentCapacities[j].value >>= capValue;
                        LOG_TRACE(Device_impl, "Device Capacity ID: " << currentCapacities[j].id << ", New Capacity: " << capValue);
                    }

                    foundProperty = true;     // Report that the requested property was found
                    break;
                }
            }

            if (!foundProperty) {
                LOG_ERROR(Device_impl, "Cannot allocate capacity: Invalid property ID: " << capacities[i].id);
                throw (CF::Device::InvalidCapacity("Cannot allocate capacity. Invalid property ID", capacities));
            }
        }

        // Check for remaining capacity.
        for (unsigned i = 0; i < currentCapacities.length (); i++) {
            if (compareAnyToZero (currentCapacities[i].value) == POSITIVE) {
                extraCap = true;

                // No need to keep going. if after allocation there is any capacity available, the device is ACTIVE.
                break;
            }
        }

        // Store new capacities, here is when the allocation takes place
        configure (currentCapacities);

        /* Update usage state */
        if (!extraCap) {
            setUsageState (CF::Device::BUSY);
        } else {
            setUsageState (CF::Device::ACTIVE);   /* Assumes it allocated something. Not considering zero allocations */
        }

        return true;
    } else {
        /* Not sure */
        LOG_WARN(Device_impl, "Cannot allocate capacity: System is BUSY");
        return false;
    }
}


void Device_impl::deallocateCapacity (const CF::Properties& capacities)
throw (CORBA::SystemException, CF::Device::InvalidCapacity, CF::Device::InvalidState)
{
    CF::Properties currentCapacities;

    bool totalCap = true;                         /* Flag to check remaining extra capacity to allocate */
    bool foundProperty;                           /* Flag to indicate if the requested property was found */
    AnyComparisonType compResult;

    /* Verify that the device is in a valid state */
    if (isLocked () || isDisabled ()) {
        LOG_WARN(Device_impl, "Cannot deallocate capacity. System is either LOCKED or DISABLED.");
        throw (CF::Device::InvalidState("Cannot deallocate capacity. System is either LOCKED or DISABLED."));
        return;
    }

    /* Now verify that there is capacity currently being used */
    if (!isIdle ()) {
        query (currentCapacities);

        /* Look in propertySet for the properties requested */
        for (unsigned i = 0; i < capacities.length (); i++) {
            foundProperty = false;

            for (unsigned j = 0; j < currentCapacities.length (); j++) {
                if (strcmp (capacities[i].id, currentCapacities[j].id) == 0) {

                    // Verify that both values have the same type
                    if (!ossie::corba::isValidType (currentCapacities[j].value, capacities[i].value)) {
                        LOG_WARN(Device_impl, "Cannot deallocate capacity. Incorrect Data Type.");
                        throw (CF::Device::InvalidCapacity("Cannot deallocate capacity. Incorrect Data Type.", capacities));
                    } else {
                        deallocate (currentCapacities[j].value, capacities[i].value);
                    }

                    foundProperty = true;     /* Report that the requested property was found */
                    break;
                }
            }

            if (!foundProperty) {
                LOG_WARN(Device_impl, "Cannot deallocate capacity. Invalid property ID");
                throw (CF::Device::InvalidCapacity("Cannot deallocate capacity. Invalid property ID",   capacities));
            }
        }

        // Check for exceeding dealLocations and back-to-total capacity
        for (unsigned i = 0; i < currentCapacities.length (); i++) {
            for (unsigned j = 0; j < originalCap.length (); j++) {
                if (strcmp (currentCapacities[i].id, originalCap[j].id) == 0) {
                    compResult = compareAnys (currentCapacities[i].value, originalCap[j].value);

                    if (compResult == FIRST_BIGGER) {
                        LOG_WARN(Device_impl, "Cannot deallocate capacity. New capacity would exceed original bound.");
                        throw (CF::Device::InvalidCapacity("Cannot deallocate capacity. New capacity would exceed original bound.", capacities));
                    } else if (compResult == SECOND_BIGGER) {
                        totalCap = false;
                        break;
                    }
                }
            }
        }

        /* Write new capacities */
        configure (currentCapacities);

        /* Update usage state */
        if (!totalCap) {
            setUsageState (CF::Device::ACTIVE);
        } else {
            setUsageState (CF::Device::IDLE);     /* Assumes it allocated something. Not considering zero allocations */
        }

        return;
    } else {
        /* Not sure */
        throw (CF::Device::InvalidCapacity ("Cannot deallocate capacity. System is IDLE.", capacities));
        return;
    }
}


bool Device_impl::allocate (CORBA::Any& deviceCapacity, const CORBA::Any& resourceRequest)
{
    CORBA::TypeCode_var tc1 = deviceCapacity.type ();
    CORBA::TypeCode_var tc2 = resourceRequest.type ();

    switch (tc1->kind ()) {
    case CORBA::tk_ulong: {
        CORBA::ULong devCapac, rscReq;
        deviceCapacity >>= devCapac;
        resourceRequest >>= rscReq;

        if (rscReq <= devCapac) {
            devCapac -= rscReq;
            deviceCapacity <<= devCapac;
            return true;
        } else {
            return false;
        }
    }

    case CORBA::tk_long: {
        CORBA::Long devCapac, rscReq;
        deviceCapacity >>= devCapac;
        resourceRequest >>= rscReq;

        if (rscReq <= devCapac) {
            devCapac -= rscReq;
            deviceCapacity <<= devCapac;
            return true;
        } else {
            return false;
        }
    }

    case CORBA::tk_short: {
        CORBA::Short devCapac, rscReq;
        deviceCapacity >>= devCapac;
        resourceRequest >>= rscReq;

        if (rscReq <= devCapac) {
            devCapac -= rscReq;
            deviceCapacity <<= devCapac;
            return true;
        } else {
            return false;
        }
    }

    default:
        return false;
    }

    //Should never reach this point
    return false;
}


void Device_impl::deallocate (CORBA::Any& deviceCapacity, const CORBA::Any& resourceRequest)
{
    CORBA::TypeCode_var tc1 = deviceCapacity.type ();
    CORBA::TypeCode_var tc2 = resourceRequest.type ();

    switch (tc1->kind ()) {
    case CORBA::tk_ulong: {
        CORBA::ULong devCapac, rscReq;
        deviceCapacity >>= devCapac;
        resourceRequest >>= rscReq;
        devCapac += rscReq;
        deviceCapacity <<= devCapac;
        break;
    }

    case CORBA::tk_long: {
        CORBA::Long devCapac, rscReq;
        deviceCapacity >>= devCapac;
        resourceRequest >>= rscReq;
        devCapac += rscReq;
        deviceCapacity <<= devCapac;
        break;
    }

    case CORBA::tk_short: {
        CORBA::Short devCapac, rscReq;
        deviceCapacity >>= devCapac;
        resourceRequest >>= rscReq;
        devCapac += rscReq;
        deviceCapacity <<= devCapac;
        break;
    }

    default:
        break;
    }

    return;
}


// compareAnys function compares both Any type inputs
// returns FIRST_BIGGER if the first argument is bigger
// returns SECOND_BIGGER is the second argument is bigger
// and BOTH_EQUAL if they are equal
Device_impl::AnyComparisonType Device_impl::compareAnys (CORBA::Any& first,
                                                         CORBA::Any& second)
{
    CORBA::TypeCode_var tc1 = first.type ();
    CORBA::TypeCode_var tc2 = second.type ();

    switch (tc1->kind ()) {
    case CORBA::tk_ulong: {
        CORBA::ULong frst, scnd;
        first >>= frst;
        second >>= scnd;

        if (frst > scnd) {
            return FIRST_BIGGER;
        } else if (frst == scnd) {
            return BOTH_EQUAL;
        } else {
            return SECOND_BIGGER;
        }
    }

    case CORBA::tk_long: {
        CORBA::Long frst, scnd;
        first >>= frst;
        second >>= scnd;

        if (frst > scnd) {
            return FIRST_BIGGER;
        } else if (frst == scnd) {
            return BOTH_EQUAL;
        } else {
            return SECOND_BIGGER;
        }
    }

    case CORBA::tk_short: {
        CORBA::Short frst, scnd;
        first >>= frst;
        second >>= scnd;

        if (frst > scnd) {
            return FIRST_BIGGER;
        } else if (frst == scnd) {
            return BOTH_EQUAL;
        } else {
            return SECOND_BIGGER;
        }
    }

    default:
        return UNKNOWN;
    }

    return UNKNOWN;
}


// compareAnyToZero function compares the any type input to zero
// returns POSITIVE if the first argument is bigger
// returns NEGATIVE is the second argument is bigger
// and ZERO if they are equal
Device_impl::AnyComparisonType Device_impl::compareAnyToZero (CORBA::Any& first)
{
    CORBA::TypeCode_var tc1 = first.type ();

    switch (tc1->kind ()) {
    case CORBA::tk_ulong: {
        CORBA::ULong frst;
        first >>= frst;

        if (frst > 0) {
            return POSITIVE;
        } else if (frst == 0) {
            return ZERO;
        } else {
            return NEGATIVE;
        }
    }

    case CORBA::tk_long: {
        CORBA::Long frst;
        first >>= frst;

        if (frst > 0) {
            return POSITIVE;
        } else if (frst == 0) {
            return ZERO;
        } else {
            return NEGATIVE;
        }
    }

    case CORBA::tk_short: {
        CORBA::Short frst;
        first >>= frst;

        if (frst > 0) {
            return POSITIVE;
        } else if (frst == 0) {
            return ZERO;
        } else {
            return NEGATIVE;
        }
    }

    default:
        return UNKNOWN;
    }

    return UNKNOWN;
}


void Device_impl::setUsageState (CF::Device::UsageType newUsageState)
{
    /* Keep a copy of the actual usage state */
#if ENABLE_EVENTS
    StandardEvent::StateChangeType current_state = StandardEvent::BUSY;
    StandardEvent::StateChangeType new_state = StandardEvent::BUSY;
    switch (_usageState) {
        case CF::Device::IDLE:
            current_state = StandardEvent::IDLE;
            break;
        case CF::Device::ACTIVE:
            current_state = StandardEvent::ACTIVE;
            break;
        case CF::Device::BUSY:
            current_state = StandardEvent::BUSY;
            break;
    }
    switch (newUsageState) {
        case CF::Device::IDLE:
            new_state = StandardEvent::IDLE;
            break;
        case CF::Device::ACTIVE:
            new_state = StandardEvent::ACTIVE;
            break;
        case CF::Device::BUSY:
            new_state = StandardEvent::BUSY;
            break;
    }
    ossie::sendStateChangeEvent(Device_impl::__logger, _identifier.c_str(), _identifier.c_str(), StandardEvent::USAGE_STATE_EVENT, 
        current_state, new_state, proxy_consumer);
#endif
    _usageState = newUsageState;
}

void Device_impl::setAdminState (CF::Device::AdminType new_adminState)
{
#if ENABLE_EVENTS
    /* Keep a copy of the actual usage state */
    StandardEvent::StateChangeType current_state = StandardEvent::UNLOCKED;
    StandardEvent::StateChangeType new_state = StandardEvent::UNLOCKED;
    switch (_adminState) {
        case CF::Device::LOCKED:
            current_state = StandardEvent::LOCKED;
            break;
        case CF::Device::UNLOCKED:
            current_state = StandardEvent::UNLOCKED;
            break;
        case CF::Device::SHUTTING_DOWN:
            current_state = StandardEvent::SHUTTING_DOWN;
            break;
    }
    switch (new_adminState) {
        case CF::Device::LOCKED:
            new_state = StandardEvent::LOCKED;
            break;
        case CF::Device::UNLOCKED:
            new_state = StandardEvent::UNLOCKED;
            break;
        case CF::Device::SHUTTING_DOWN:
            new_state = StandardEvent::SHUTTING_DOWN;
            break;
    }
    ossie::sendStateChangeEvent(Device_impl::__logger, _identifier.c_str(), _identifier.c_str(), StandardEvent::ADMINISTRATIVE_STATE_EVENT, 
        current_state, new_state, proxy_consumer);
#endif
    _adminState = new_adminState;
}


void Device_impl::adminState (CF::Device::AdminType new_adminState)
throw (CORBA::SystemException)
{
    setAdminState(new_adminState);
    _adminState = new_adminState;
}


bool Device_impl::isUnlocked ()
{
    if (_adminState == CF::Device::UNLOCKED) {
        return true;
    } else {
        return false;
    }
}


bool Device_impl::isLocked ()
{
    if (_adminState == CF::Device::LOCKED) {
        return true;
    } else {
        return false;
    }
}


bool Device_impl::isDisabled ()
{
    if (_operationalState == CF::Device::DISABLED) {
        return true;
    } else {
        return false;
    }
}


bool Device_impl::isBusy ()
{
    if (_usageState == CF::Device::BUSY) {
        return true;
    } else {
        return false;
    }
}


bool Device_impl::isIdle ()
{
    if (_usageState == CF::Device::IDLE) {
        return true;
    } else {
        return false;
    }
}


char* Device_impl::label ()
throw (CORBA::SystemException)
{
    return CORBA::string_dup(_label.c_str());
}


char* Device_impl::softwareProfile ()
throw (CORBA::SystemException)
{
    return CORBA::string_dup(_softwareProfile.c_str());
}


CF::Device::UsageType Device_impl::usageState ()
throw (CORBA::SystemException)
{
    return _usageState;
}


CF::Device::AdminType Device_impl::adminState ()
throw (CORBA::SystemException)
{
    return _adminState;
}


CF::Device::OperationalType Device_impl::operationalState ()
throw (CORBA::SystemException)
{
    return _operationalState;
}


CF::AggregateDevice_ptr Device_impl::compositeDevice ()
throw (CORBA::SystemException)
{
    return _aggregateDevice;
}


void  Device_impl::configure (const CF::Properties& capacities)
throw (CF::PropertySet::PartialConfiguration, CF::PropertySet::
       InvalidConfiguration, CORBA::SystemException)
{
    
    if (initialConfiguration) {
        initialConfiguration = false;

        originalCap.length (capacities.length ());

        for (unsigned int i = 0; i < capacities.length (); i++) {
            
            originalCap[i].id = CORBA::string_dup (capacities[i].id);
            originalCap[i].value = capacities[i].value;
        }
    }

    PropertySet_impl::configure(capacities);
}

#if ENABLE_EVENTS
IDM_Channel_Supplier_i::IDM_Channel_Supplier_i (Device_impl *_dev)
{
    TRACE_ENTER(Device_impl)

    _device = _dev;

    TRACE_EXIT(Device_impl)
}


void IDM_Channel_Supplier_i::disconnect_push_supplier ()
{
    TRACE_ENTER(Device_impl)

    TRACE_EXIT(Device_impl)
}


void Device_impl::connectSupplierToIncomingEventChannel(CosEventChannelAdmin::EventChannel_ptr idm_channel)
{
    TRACE_ENTER(Device_impl);

    IDM_channel = CosEventChannelAdmin::EventChannel::_duplicate(idm_channel);

    CosEventChannelAdmin::SupplierAdmin_var supplier_admin;
    unsigned int number_tries;
    unsigned int maximum_tries = 10;

    number_tries = 0;
    while (true)
    {
        try {
            supplier_admin = IDM_channel->for_suppliers ();
            if (CORBA::is_nil(supplier_admin))
            {
                IDM_channel = CosEventChannelAdmin::EventChannel::_nil();
                return;
            }
            break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
            if (number_tries == maximum_tries) {
                IDM_channel = CosEventChannelAdmin::EventChannel::_nil();
                return;
            }
            usleep(1000);   // wait 1 ms
            number_tries++;
            continue;
        }
    }

    proxy_consumer = CosEventChannelAdmin::ProxyPushConsumer::_nil();
    number_tries = 0;
    while (true)
    {
        try {
            proxy_consumer = supplier_admin->obtain_push_consumer ();
            if (CORBA::is_nil(proxy_consumer))
            {
                IDM_channel = CosEventChannelAdmin::EventChannel::_nil();
                return;
            }
            break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
            if (number_tries == maximum_tries) {
                IDM_channel = CosEventChannelAdmin::EventChannel::_nil();
                return;
            }
            usleep(1000);   // wait 1 ms
            number_tries++;
            continue;
        }
    }

    //
    // Connect Push Supplier - retrying on Comms Failure.
    PortableServer::POA_var root_poa = PortableServer::POA::_narrow(ossie::corba::RootPOA());

    IDM_Channel_Supplier_i* supplier_servant = new IDM_Channel_Supplier_i(this);

    PortableServer::ObjectId_var oid = root_poa->activate_object(supplier_servant);

    CosEventComm::PushSupplier_var sptr = supplier_servant->_this();
    
    supplier_servant->_remove_ref();
    number_tries = 0;
    while (true)
    {
        try {
            proxy_consumer->connect_push_supplier(sptr.in());
            break;
        }
        catch (CORBA::BAD_PARAM& ex) {
            IDM_channel = CosEventChannelAdmin::EventChannel::_nil();
            return;
        }
        catch (CosEventChannelAdmin::AlreadyConnected& ex) {
            break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
            if (number_tries == maximum_tries) {
                IDM_channel = CosEventChannelAdmin::EventChannel::_nil();
                return;
            }
            usleep(1000);   // wait 1 ms
            number_tries++;
            continue;
        }
    }

}

#endif
