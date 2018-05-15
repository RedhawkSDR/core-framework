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
#include <signal.h>
#include <sys/signalfd.h>

#include "ossie/Device_impl.h"
#include "ossie/CorbaUtils.h"
#include "ossie/Events.h"

//
// Helper class for performing cleanup when an allocation partially succeeds
//
class DeallocationHelper {
public:
    DeallocationHelper (Device_impl* device):
        device(device),
        capacities()
    {

    }

    // Destructor deallocates all registered allocations
    ~DeallocationHelper ()
    {
        try {
            if (capacities.length() > 0) {
                device->deallocateCapacity(capacities);
            }
        } catch (...) {
            // Exceptions cannot propgate out of a dtor
        }
    }

    // Add the given capacity to the list of successful allocations
    void add (const CF::DataType& capacity)
    {
        ossie::corba::push_back(capacities, capacity);
    }

    // Mark that the allocation was successful, so that the capacities are not
    // deallocated at destruction
    void clear ()
    {
        capacities.length(0);
    }

private:
    Device_impl* device;
    CF::Properties capacities;
};

PREPARE_CF_LOGGING(Device_impl)

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
    sig_fd=-1;

    useNewAllocation = false;
    this->_devMgr = NULL;
    setLogger(this->_baseLog->getChildLogger("Device", "system"));
}

Device_impl::Device_impl (char* devMgr_ior, char* _id, char* lbl, char* sftwrPrfl) : Resource_impl(_id, lbl)
{
    initResources(devMgr_ior, _id, lbl, sftwrPrfl);
    RH_TRACE(_deviceLog, "Done Constructing Device")
}


Device_impl::Device_impl (char* devMgr_ior, char* _id, char* lbl, char* sftwrPrfl,
                          CF::Properties& capacities) : Resource_impl(_id, lbl)
{
    initResources(devMgr_ior, _id, lbl, sftwrPrfl);
    configure (capacities);
    RH_TRACE(_deviceLog, "Done Constructing Device")
}

Device_impl::Device_impl (char* devMgr_ior, char* _id, char* lbl, char* sftwrPrfl,
                          CF::Properties& capacities, char* compositeDev_ior) : Resource_impl(_id, lbl)
{

    initResources(devMgr_ior, _id, lbl, sftwrPrfl);
    _compositeDev_ior = compositeDev_ior;
    CORBA::Object_var _aggDev_obj = ossie::corba::Orb()->string_to_object(_compositeDev_ior.c_str());
    if (CORBA::is_nil(_aggDev_obj)) {
        RH_ERROR(_deviceLog, "Invalid composite device IOR: " << _compositeDev_ior);
    } else {
        _aggregateDevice = CF::AggregateDevice::_narrow(_aggDev_obj);
        _aggregateDevice->addDevice(this->_this());
    }

    configure (capacities);
    RH_TRACE(_deviceLog, "Done Constructing Device")
}

Device_impl::Device_impl (char* devMgr_ior, char* _id, char* lbl, char* sftwrPrfl,
                          char* compositeDev_ior) : Resource_impl(_id, lbl)
{

    initResources(devMgr_ior, _id, lbl, sftwrPrfl);
    _compositeDev_ior = compositeDev_ior;
    CORBA::Object_var _aggDev_obj = ossie::corba::Orb()->string_to_object(_compositeDev_ior.c_str());
    if (CORBA::is_nil(_aggDev_obj)) {
        RH_ERROR(_deviceLog, "Invalid composite device IOR: " << _compositeDev_ior);
    } else {
        _aggregateDevice = CF::AggregateDevice::_narrow(_aggDev_obj);
        _aggregateDevice->addDevice(this->_this());
    }


    RH_TRACE(_deviceLog, "Done Constructing Device")
}

const CF::DeviceManager_ptr Device_impl::getDeviceManager() const {
  if ( _devMgr ) return _devMgr->getRef();
  return CF::DeviceManager::_nil();
}

void Device_impl::setLogger(rh_logger::LoggerPtr logptr)
{
    _deviceLog = logptr;
}


void  Device_impl::postConstruction (std::string &profile, 
                                     std::string &registrar_ior, 
                                     const std::string &idm_channel_ior,
                                     const std::string &nic,
                                     const int sigfd )
{

  // signalfd for processing sigXX events in service function, removes deadlock issue with io operations
  sig_fd = sigfd;   
  
  // resolves Domain and Device Manger relationships
  setAdditionalParameters(profile, registrar_ior, nic);

  // establish IDM Channel connectivity
  connectIDMChannel( idm_channel_ior );

  // register ourself with my DeviceManager
  _deviceManager->registerDevice(this->_this());

  // setup original capacity values cache
  RH_TRACE(_deviceLog, "postConstructor: Saving original capacities... ");
  PropertySet_impl::PropertyMap::iterator pi = propTable.begin();
  for( ; pi != propTable.end(); pi++ ) {
      PropertyInterface *p = pi->second;
      if ( p and p->isAllocatable() ) {
          CF::DataType res;
          res.id = p->id.c_str();
          p->getValue(res.value);
          RH_TRACE(_deviceLog, "postConstructor: Saving allocation ID: " << p->id);
          bool found = false;
          for ( unsigned int j=0; j < originalCap.length(); j++) {
              if ( strcmp(p->id.c_str(), originalCap[j].id) == 0 ) {
                  RH_TRACE(_deviceLog, "Override value for allocation ID: " << p->id);
                  originalCap[j].value = res.value;
                  found = true;
              }
          }
          if ( !found ) {
              ossie::corba::push_back( originalCap, res );
          }
      }

  }

}



void  Device_impl::setAdditionalParameters ( std::string &profile, 
                                             std::string &registrar_ior,
                                             const std::string &nic )
{
  _devMgr_ior = registrar_ior;
  _deviceManager = CF::DeviceManager::_nil();
  CORBA::Object_var obj = ossie::corba::Orb()->string_to_object(_devMgr_ior.c_str());
  if (CORBA::is_nil(obj)) {
    RH_ERROR(_deviceLog, "Invalid device manager IOR");
    exit(-1);
  }
  _deviceManager = CF::DeviceManager::_narrow(obj);
  if (CORBA::is_nil(_deviceManager)) {
    RH_ERROR(_deviceLog, "Could not narrow device manager IOR");
    exit(-1);
  }

  this->_devMgr = new redhawk::DeviceManagerContainer(_deviceManager);

  // Set up domain awareness
  CF::DomainManager_var domainManager = _deviceManager->domMgr();
  setDomainManager(domainManager);
}


void  Device_impl::run ()
{
  Resource_impl::run(); // This won't return until halt is called
}

void  Device_impl::halt ()
{
    RH_DEBUG(_deviceLog, "Halting Device")
    Resource_impl::halt();
}

void
Device_impl::releaseObject ()
throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // SR:419
    RH_DEBUG(_deviceLog, "Receive releaseObject call");
    if (_adminState == CF::Device::UNLOCKED) {
        RH_DEBUG(_deviceLog, "Releasing Device")
        setAdminState(CF::Device::SHUTTING_DOWN);

        // SR:418
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
            RH_DEBUG(_deviceLog, "Unregistering Device")
            _deviceManager->unregisterDevice(this->_this());
        } catch (...) {
            // SR:423
            throw CF::LifeCycle::ReleaseError();
        }
        RH_DEBUG(_deviceLog, "Done Releasing Device")
    }

    RH_DEBUG(_deviceLog, "Clean up IDM_CHANNEL. DEV-ID:"  << _identifier );
    if ( idm_publisher )  idm_publisher.reset();
    delete this->_devMgr;
    this->_devMgr=NULL;
    Resource_impl::releaseObject();
}

Device_impl::~Device_impl ()
{

  RH_TRACE(_deviceLog, "DTOR START  DEV-ID:"  << _identifier );

  RH_DEBUG(_deviceLog, "Clean up event channel allocations");
  if ( idm_publisher ) idm_publisher.reset();
  
  if (this->_devMgr != NULL) {
      delete this->_devMgr;
   }

  RH_TRACE(_deviceLog, "DTOR END  DEV-ID:"  << _identifier );

}

/* Alternate implementation*/
CORBA::Boolean Device_impl::allocateCapacity (const CF::Properties& capacities)
throw (CORBA::SystemException, CF::Device::InvalidCapacity, CF::Device::InvalidState, CF::Device::InsufficientCapacity)
{
    RH_TRACE(_deviceLog, "in allocateCapacity");

    if (capacities.length() == 0) {
        // Nothing to do, return
        RH_TRACE(_deviceLog, "no capacities to configure.");
        return true;
    }

    // Verify that the device is in a valid state
    if (!isUnlocked() || isDisabled()) {
        const char* invalidState;
        if (isLocked()) {
            invalidState = "LOCKED";
        } else if (isDisabled()) {
            invalidState = "DISABLED";
        } else {
            invalidState = "SHUTTING_DOWN";
        }
        RH_DEBUG(_deviceLog, "Cannot allocate capacity: System is " << invalidState);
        throw CF::Device::InvalidState(invalidState);
    }

    if (useNewAllocation) {
        return allocateCapacityNew(capacities);
    } else {
        return allocateCapacityLegacy(capacities);
    }
}

void Device_impl::validateCapacities (const CF::Properties& capacities)
{
    CF::Properties unknownProperties;
    CF::Properties nonAllocProperties;
    for (size_t ii = 0; ii < capacities.length(); ++ii) {
        const CF::DataType& capacity = capacities[ii];
        const std::string id = static_cast<const char*>(capacity.id);
        PropertyInterface* property = getPropertyFromId(id);
        if (!property) {
            ossie::corba::push_back(unknownProperties, capacity);
        } else if (!property->isAllocatable()) {
            ossie::corba::push_back(nonAllocProperties, capacity);
        }
    }

    if (unknownProperties.length() > 0) {
        throw CF::Device::InvalidCapacity("Unknown properties", unknownProperties);
    } else if (nonAllocProperties.length() > 0) {
        throw CF::Device::InvalidCapacity("Not allocatable", nonAllocProperties);
    }
}

bool Device_impl::allocateCapacityLegacy (const CF::Properties& capacities)
{
    RH_TRACE(_deviceLog, "Using legacy capacity allocation");
    
    typedef std::pair< CF::DataType, CF::DataType >         Allocation;
    std::vector< Allocation > allocations;
    CF::Properties currentCapacities;

    {
        SCOPED_LOCK(propertySetAccess);
        validateCapacities(capacities);
    }

    bool extraCap = false;  // Flag to check remaining extra capacity to allocate
    bool foundProperty;     // Flag to indicate if the requested property was found

    if (!isBusy ()) {
        // The try is just a formality in this case
        try {
            // Get all properties currently in device
            query (currentCapacities);
        } catch (CF::UnknownProperties) {
        }

        SCOPED_LOCK(propertySetAccess);
        /* Look in propertySet for the properties requested */
        for (unsigned i = 0; i < capacities.length (); i++) {
            foundProperty = false;

            for (unsigned j = 0; j < currentCapacities.length (); j++) {
                RH_TRACE(_deviceLog, "Comparing IDs: " << capacities[i].id << ", " << currentCapacities[j].id );
                if (strcmp (capacities[i].id, currentCapacities[j].id) == 0) {
                    // Verify that both values have the same type
                    if (!ossie::corba::isValidType (currentCapacities[j].value, capacities[i].value)) {
                        RH_ERROR(_deviceLog, "Cannot allocate capacity: Incorrect data type.");
                        throw (CF::Device::InvalidCapacity("Cannot allocate capacity. Incorrect Data Type.", capacities));
                    } else {
                        // Check for sufficient capacity and allocate it
                        if (!allocate (currentCapacities[j].value, capacities[i].value)) {
                            RH_ERROR(_deviceLog, "Cannot allocate capacity: Insufficient capacity.");
                            return false;
                        }
                        Allocation a( capacities[i], currentCapacities[j] );
                        allocations.push_back( a );
                        RH_TRACE(_deviceLog, "Device Allocation Capacity against, ID: " << capacities[i].id );
                    }

                    foundProperty = true;     // Report that the requested property was found
                    break;
                }
            }

            if (!foundProperty) {
                RH_ERROR(_deviceLog, "Cannot allocate capacity: Invalid property ID: " << capacities[i].id);
                throw (CF::Device::InvalidCapacity("Cannot allocate capacity. Invalid property ID", capacities));
            }
        }

        // Check for remaining capacity.
        for (unsigned i = 0; i < allocations.size(); i++) {
            if (compareAnyToZero (allocations[i].second.value) == POSITIVE) {
                extraCap = true;

                // No need to keep going. if after allocation there is any capacity available, the device is ACTIVE.
                break;
            }
        }

        // apply allocations to properties..
        CF::Properties invalidProperties;
        DeallocationHelper cleanup(this);

        for (CORBA::ULong ii = 0; ii < allocations.size(); ++ii) {
            CF::DataType request = allocations[ii].first;
            CF::DataType new_alloc = allocations[ii].second;
            PropertyInterface* property = getPropertyFromId((const char*)request.id);
            RH_TRACE(_deviceLog, "Allocatable property: " << property->id);
            try {
                    std::vector<std::string>::iterator kind = property->kinds.begin();
                    bool sendEvent = false;
                    bool eventType = false;
                    if (propertyChangePort != NULL) {
                        // searching for event type
                        while (kind != property->kinds.end()) {
                            if (!kind->compare("event")) {
                                // it is of event type
                                eventType = true;
                                break;
                            }
                            kind++;
                        }
                        if (eventType) {
                            if (property->compare(new_alloc.value)) {
                                // the incoming value is different from the current value
                                sendEvent = true;
                            }
                        }
                    }
                    property->setValue(new_alloc.value);
                    executePropertyCallback(property->id);
                    if (sendEvent) {
                        // sending the event
                        propertyChangePort->sendPropertyEvent(property->id);
                    }

                    // just in case.. should not happen
                    cleanup.add(request);
            } catch (std::exception& e) {
                RH_ERROR(_deviceLog, "Setting property " << property->id << ", " << property->name << " failed.  Cause: " << e.what());
                ossie::corba::push_back(invalidProperties,request);
            } catch (CORBA::Exception& e) {
                RH_ERROR(_deviceLog, "Setting property " << property->id << " failed.  Cause: " << e._name());
                ossie::corba::push_back(invalidProperties,request);
            }
        }

        if (invalidProperties.length () > 0) {
            throw (CF::Device::InvalidCapacity("Cannot allocate capacity", invalidProperties ));
        }

        cleanup.clear();

        /* Update usage state */
        if (!extraCap) {
            setUsageState (CF::Device::BUSY);
        } else {
            setUsageState (CF::Device::ACTIVE);   /* Assumes it allocated something. Not considering zero allocations */
        }

        return true;
    } else {
        /* Not sure */
        RH_WARN(_deviceLog, "Cannot allocate capacity: System is BUSY");
        return false;
    }
}

bool Device_impl::allocateCapacityNew (const CF::Properties& capacities)
{
    RH_TRACE(_deviceLog, "Using callback-based capacity allocation");

    validateCapacities(capacities);

    DeallocationHelper allocations(this);

    for (size_t ii = 0; ii < capacities.length(); ++ii) {
        const CF::DataType& capacity = capacities[ii];
        const std::string id = static_cast<const char*>(capacity.id);
        PropertyInterface* property = getPropertyFromId(id);
        RH_TRACE(_deviceLog, "Allocating property '" << id << "'");
        try {
            if (property->allocate(capacity.value)) {
                allocations.add(capacity);
            } else {
                RH_DEBUG(_deviceLog, "Cannot allocate capacity. Insufficent capacity for property '" << id << "'");
                return false;
            }
        } catch (const ossie::not_implemented_error& ex) {
            RH_WARN(_deviceLog, "No allocation implementation for property '" << id << "'");
            return false;
        }
    }

    // All allocations were successful, clear 
    allocations.clear();

    updateUsageState();
    return true;
}

void Device_impl::deallocateCapacity (const CF::Properties& capacities)
throw (CORBA::SystemException, CF::Device::InvalidCapacity, CF::Device::InvalidState)
{
    // Verify that the device is in a valid state
    if (isLocked() || isDisabled()) {
        const char* invalidState;
        if (isLocked()) {
            invalidState = "LOCKED";
        } else {
            invalidState = "DISABLED";
        }
        RH_DEBUG(_deviceLog, "Cannot deallocate capacity: System is " << invalidState);
        throw CF::Device::InvalidState(invalidState);
    }

    if (useNewAllocation) {
        deallocateCapacityNew(capacities);
    } else {
        deallocateCapacityLegacy(capacities);
    }
}

void Device_impl::deallocateCapacityLegacy (const CF::Properties& capacities)
{
    RH_TRACE(_deviceLog, "Using legacy capacity deallocation");

    typedef std::pair< CF::DataType, PropertyInterface * >         Allocation;
    std::vector< Allocation > deallocations;
    CF::Properties overCaps;
    CF::Properties invalidProps;

    bool totalCap = true;                         /* Flag to check remaining extra capacity to allocate */
    AnyComparisonType compResult;

    {
        SCOPED_LOCK(propertySetAccess);
        validateCapacities(capacities);
    }

    /* Now verify that there is capacity currently being used */
    if (!isIdle ()) {
        SCOPED_LOCK(propertySetAccess);
        /* Look in propertySet for the properties requested */
        for (unsigned i = 0; i < capacities.length (); i++) {
            CF::DataType request = capacities[i];
            std::string pid = (const char*)capacities[i].id;
            PropertyInterface *property = getPropertyFromId(pid);
            CORBA::Any new_value;
            property->getValue(new_value);
            if (!ossie::corba::isValidType (new_value, capacities[i].value)) {
                RH_WARN(_deviceLog, "Cannot deallocate capacity. Incorrect Data Type.");
                throw (CF::Device::InvalidCapacity("Cannot deallocate capacity. Incorrect Data Type.", capacities));
            } else {
                deallocate (new_value, capacities[i].value);
                // check that we can stay within original bounds
                bool _apply=true;
                for (unsigned ii = 0; ii < originalCap.length (); ii++) {
                    RH_TRACE(_deviceLog, "Testing max value for allocation ID: " << originalCap[ii].id);
                    if (strcmp (pid.c_str(), originalCap[ii].id) == 0) {
                        compResult = compareAnys (new_value, originalCap[ii].value);
                        if (compResult == FIRST_BIGGER) {
                            RH_WARN(_deviceLog, "Cannot deallocate capacity, allocation ID: " << pid << ", New capacity would exceed original bound.");
                            ossie::corba::push_back(overCaps, capacities[i]);
                            _apply = false;
                        }
                    }
                }
                if (_apply) {
                    RH_TRACE(_deviceLog, "(deallocation) Allocatable property : " << property->id);
                    try {
                        std::vector<std::string>::iterator kind = property->kinds.begin();
                        bool sendEvent = false;
                        bool eventType = false;
                        if (propertyChangePort != NULL) {
                            // searching for event type
                            while (kind != property->kinds.end()) {
                                if (!kind->compare("event")) {
                                    // it is of event type
                                    eventType = true;
                                    break;
                                }
                                kind++;
                            }
                            if (eventType) {
                                if (property->compare(new_value)) {
                                    // the incoming value is different from the current value
                                    sendEvent = true;
                                }
                            }
                        }
                        property->setValue(new_value);
                        executePropertyCallback(property->id);
                        if (sendEvent) {
                            // sending the event
                            propertyChangePort->sendPropertyEvent(property->id);
                        }

                    } catch (std::exception& e) {
                        RH_ERROR(_deviceLog, "Setting property " << property->id << ", " << property->name << " failed.  Cause: " << e.what());
                        ossie::corba::push_back(invalidProps,request);
                    } catch (CORBA::Exception& e) {
                        RH_ERROR(_deviceLog, "Setting property " << property->id << " failed.  Cause: " << e._name());
                        ossie::corba::push_back(invalidProps,request);
                    }

                    //currentCapacities[j] = new_value;
                    Allocation a( capacities[i], property );
                    deallocations.push_back( a );
                }

            }
        }

        // Check for exceeding deallocations and back-to-total capacity
        for (unsigned i = 0; i < deallocations.size(); i++) {
            for (unsigned j = 0; j < originalCap.length (); j++) {
                if (strcmp (deallocations[i].second->id.c_str(), originalCap[j].id) == 0) {
                    CORBA::Any new_value;
                    deallocations[i].second->getValue(new_value);
                    compResult = compareAnys (new_value, originalCap[j].value);
                    if (compResult == SECOND_BIGGER) {
                        totalCap = false;
                        break;
                    }
                }
            }
        }

        /* Update usage state */
        if (!totalCap) {
            setUsageState (CF::Device::ACTIVE);
        } else {
            setUsageState (CF::Device::IDLE);     /* Assumes it allocated something. Not considering zero allocations */
        }

        if (invalidProps.length () > 0) {
            throw (CF::Device::InvalidCapacity("Invalid capacity allocation Ids", invalidProps ));
        }

        if (overCaps.length () > 0) {
            throw (CF::Device::InvalidCapacity("Following properties exceeded original bounds", overCaps ));
        }

        return;
    } else {
        /* Not sure */
        throw (CF::Device::InvalidCapacity ("Cannot deallocate capacity. System is IDLE.", capacities));
        return;
    }
}

void Device_impl::deallocateCapacityNew (const CF::Properties& capacities)
{
    RH_TRACE(_deviceLog, "Using callback-based capacity deallocation");

    validateCapacities(capacities);

    CF::Properties invalidProps;
    CF::Properties overCaps;

    for (size_t ii = 0; ii < capacities.length(); ++ii) {
        const CF::DataType& capacity = capacities[ii];
        const std::string id = static_cast<const char*>(capacity.id);
        PropertyInterface* property = getPropertyFromId(id);
        RH_TRACE(_deviceLog, "Deallocating property (new method) '" << id << "'");
        try {
            property->deallocate(capacity.value);
        } catch (const ossie::not_implemented_error& ex) {
            RH_WARN(_deviceLog, "No deallocation implementation for property '" << id << "'");
        } catch (const std::exception& ex) {
            ossie::corba::push_back(invalidProps, capacity);
        }
    }

    if (overCaps.length () > 0) {
        throw (CF::Device::InvalidCapacity("Following properties exceeded original bounds", overCaps ));
    }

    if ( invalidProps.length() > 0 ) {
        throw CF::Device::InvalidCapacity("Error occurred during deallocation for the following properties", invalidProps);
    }
    updateUsageState();
}

void Device_impl::updateUsageState ()
{
    // Default implementation does nothing
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

void Device_impl::sendStateChange( StandardEvent::StateChangeType &stateChangeFrom, 
                                   StandardEvent::StateChangeType &stateChangeTo,
                                   StandardEvent::StateChangeCategoryType stateCategory ) {

  std::string producerId = _identifier;
  std::string sourceId = _identifier;
  if ( idm_publisher ) {
    redhawk::events::SendStateChangeEvent( producerId.c_str(), 
                                      sourceId.c_str(),
                                      stateCategory,
                                      stateChangeFrom,
                                      stateChangeTo,
                                      idm_publisher );
  }
  else {
    RH_WARN(_deviceLog, "Unable to publish state change, DEV-ID:"  << _identifier );
  }

}


void Device_impl::setUsageState (CF::Device::UsageType newUsageState)
{
    /* Keep a copy of the actual usage state */
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
    if ( current_state != new_state ) 
      sendStateChange( current_state, new_state, StandardEvent::USAGE_STATE_EVENT );
    _usageState = newUsageState;
}

void Device_impl::setAdminState (CF::Device::AdminType new_adminState)
{
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
    sendStateChange( current_state, new_state, StandardEvent::ADMINISTRATIVE_STATE_EVENT );
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
    return CF::AggregateDevice::_duplicate(_aggregateDevice);
}


void  Device_impl::configure (const CF::Properties& capacities)
throw (CF::PropertySet::PartialConfiguration, CF::PropertySet::
       InvalidConfiguration, CORBA::SystemException)
{
    if (initialConfiguration) {
        initialConfiguration = false;

        PropertySet_impl::PropertyMap::iterator pi = propTable.begin();
        for( ; pi != propTable.end(); pi++ ) {
            PropertyInterface *p = pi->second;
            if ( p and p->isAllocatable() ) {
                CF::DataType res;
                res.id = p->id.c_str();
                p->getValue(res.value);
                RH_TRACE(_deviceLog, "Saving value for allocation ID: " << p->id);
                bool found = false;
                for ( unsigned int j=0; j < originalCap.length(); j++) {
                    if ( strcmp(p->id.c_str(), originalCap[j].id) == 0 ) {
                        RH_TRACE(_deviceLog, "Override value for allocation ID: " << p->id);
                        originalCap[j].value = res.value;
                        found = true;
                    }
                }
                if ( !found ) {
                    ossie::corba::push_back( originalCap, res );
                }
            }

        }


        for (unsigned int i = 0; i < capacities.length (); i++) {
            bool found = false;
            unsigned int j=0;
            for ( ; j < originalCap.length(); j++ ) {
                if ( strcmp(capacities[i].id, originalCap[j].id) == 0 ) {
                    RH_TRACE(_deviceLog, "Override original value for allocation ID: " <<capacities[i].id);
                    originalCap[j].value = capacities[i].value;
                    found=true;
                    break;
                }
            }
            if ( !found ) {
                RH_TRACE(_deviceLog, "Saving original value for allocation ID: " <<capacities[i].id);
                ossie::corba::push_back( originalCap, capacities[i]);
            }
        }
    }
    else {
        for (unsigned int i = 0; i < capacities.length (); i++) {
            // if allocation property is configurable... then change the originalCap
            const std::string id = static_cast<const char*>(capacities[i].id);
            PropertyInterface *p = getPropertyFromId(id);
            if ( p and p->isAllocatable() && p->isConfigurable() ) {
                for ( unsigned int j=0 ; j < originalCap.length(); j++ ) {
                    if ( strcmp(capacities[i].id, originalCap[j].id) == 0 ) {
                        originalCap[j].value = capacities[i].value;
                        break;
                    }
                }
            }
        }
    }

    PropertySet_impl::configure(capacities);
}

void Device_impl::connectIDMChannel( const std::string &idm_channel_ior ) {

  
  if ( idm_channel_ior.size() > 0 and idm_channel_ior != "" ) {
    try {
      CORBA::Object_var IDM_channel_obj = ossie::corba::Orb()->string_to_object(idm_channel_ior.c_str());
      if (CORBA::is_nil(IDM_channel_obj)) {
        RH_ERROR(_deviceLog, "Invalid IDM channel IOR: " << idm_channel_ior << "  DEV-ID:" << _identifier );
      } else {
        ossie::events::EventChannel_var idm_channel = ossie::events::EventChannel::_narrow(IDM_channel_obj);
        idm_publisher = redhawk::events::PublisherPtr(new redhawk::events::Publisher( idm_channel ));
      }
    }
    CATCH_RH_WARN(_deviceLog, "Unable to connect to IDM channel");
  }
  else {

    try {
      RH_DEBUG(_deviceLog, "Getting EventManager.... DEV-ID:" << _identifier );
      redhawk::events::ManagerPtr evt_mgr = redhawk::events::Manager::GetManager( this );
      
      if ( evt_mgr ) {
        RH_INFO(_deviceLog, "DEV-ID:" << _identifier << " Requesting IDM CHANNEL " << redhawk::events::IDM_Channel_Spec  );
        idm_publisher = evt_mgr->Publisher( redhawk::events::IDM_Channel_Spec );
      
        if (idm_publisher == NULL ) throw -1;
      }
    }
    catch(...) { 
      RH_WARN(_deviceLog, "Unable to connect to Domain's IDM Channel,  DEV-ID:" << _identifier );
    }  
  }

}

void Device_impl::start_device(Device_impl::ctor_type ctor, struct sigaction sa, int argc, char* argv[])
{
    char* devMgr_ior = 0;
    char* id = 0;
    char* label = 0;
    char* profile = 0;
    //char* idm_channel_ior = 0;
    std::string idm_channel_ior("");
    char* composite_device = 0;
    const char* logging_config_uri = 0;
    int debug_level = -1; // use log level from configuration file 
    std::string logcfg_uri("");
    std::string log_dpath("");
    std::string log_id("");
    std::string log_label("");
    bool skip_run = false;
    bool enablesigfd=false;
        
    for (int index = 1; index < argc; ++index) {
        if (std::string(argv[index]) == std::string("-i")) {
            std::cout<<"Interactive mode (-i) no longer supported. Please use the sandbox to run Components/Devices/Services outside the scope of a Domain"<<std::endl;
            exit(-1);
        }
    }
    
    std::map<std::string, char*> execparams;
                
    for (int i = 0; i < argc; i++) {
            
        if (strcmp("DEVICE_MGR_IOR", argv[i]) == 0) {
            devMgr_ior = argv[++i];
        } else if (strcmp("PROFILE_NAME", argv[i]) == 0) {
            profile = argv[++i];
        } else if (strcmp("DEVICE_ID", argv[i]) == 0) {
            id = argv[++i];
            log_id = id;
        } else if (strcmp("DEVICE_LABEL", argv[i]) == 0) {
            label = argv[++i];
            log_label = label;
        } else if (strcmp("IDM_CHANNEL_IOR", argv[i]) == 0) {
            idm_channel_ior = argv[++i];
        } else if (strcmp("COMPOSITE_DEVICE_IOR", argv[i]) == 0) {
            composite_device = argv[++i];
        } else if (strcmp("LOGGING_CONFIG_URI", argv[i]) == 0) {
            logging_config_uri = argv[++i];
        } else if (strcmp("DEBUG_LEVEL", argv[i]) == 0) {
            debug_level = atoi(argv[++i]);
        } else if (strcmp("DOM_PATH", argv[i]) == 0) {
            log_dpath = argv[++i];
        } else if (strcmp("USESIGFD", argv[i]) == 0){
            enablesigfd = true;
        } else if (strcmp("SKIP_RUN", argv[i]) == 0){
            skip_run = true;
            i++;             // skip flag has bogus argument need to skip over so execparams is processed correctly
        } else if (i > 0) {  // any other argument besides the first one is part of the execparams
            std::string paramName = argv[i];
            execparams[paramName] = argv[++i];
        }
    }

    std::string logname = log_label+".system.Device";
    // signal assist with processing SIGCHLD events for executable devices..
    int sig_fd=-1;
    if ( enablesigfd  ){
      int err;
      sigset_t  sigset;
      err=sigemptyset(&sigset);
      err=sigaddset(&sigset, SIGCHLD);
      /* We must block the signals in order for signalfd to receive them */
      err = sigprocmask(SIG_BLOCK, &sigset, NULL);
      if ( err != 0 ) {
        RH_NL_FATAL(logname, "Failed to create signalfd for SIGCHLD");
        exit(EXIT_FAILURE);
      }
      /* Create the signalfd */
      sig_fd = signalfd(-1, &sigset,0);
      if ( sig_fd == -1 ) {
        RH_NL_FATAL(logname, "Failed to create signalfd for SIGCHLD");
        exit(EXIT_FAILURE);
      }
    }

    // The ORB must be initialized before configuring logging, which may use
    // CORBA to get its configuration file. Devices do not need persistent IORs.
    ossie::corba::CorbaInit(argc, argv);

    // check if logging config URL was specified...
    if ( logging_config_uri ) logcfg_uri=logging_config_uri;

    // setup logging context for this resource
    ossie::logging::ResourceCtxPtr ctx( new ossie::logging::DeviceCtx( log_label, log_id, log_dpath ) );

    // configure logging
    if (!skip_run){
        // configure the logging library 
        ossie::logging::Configure(logcfg_uri, debug_level, ctx);
    }

    if ((devMgr_ior == 0) || (id == 0) || (profile == 0) || (label == 0)) {
        RH_NL_FATAL(logname, "Per SCA specification SR:478, DEVICE_MGR_IOR, PROFILE_NAME, DEVICE_ID, and DEVICE_LABEL must be provided");
        exit(-1);
    }

    RH_NL_DEBUG(logname, "Identifier = " << id << "Label = " << label << " Profile = " << profile << " IOR = " << devMgr_ior);

    // Associate SIGINT to signal_catcher interrupt handler
    if( sigaction( SIGINT, &sa, NULL ) == -1 ) {
        RH_NL_FATAL(logname, "SIGINT association failed");
        exit(EXIT_FAILURE);
    }

    // Associate SIGQUIT to signal_catcher interrupt handler
    if( sigaction( SIGQUIT, &sa, NULL ) == -1 ) {
        RH_NL_FATAL(logname, "SIGQUIT association failed");
        exit(EXIT_FAILURE);
    }

    // Associate SIGTERM to signal_catcher interrupt handler
    if( sigaction( SIGTERM, &sa, NULL ) == -1 ) {
        RH_NL_FATAL(logname, "SIGTERM association failed");
        exit(EXIT_FAILURE);
    }

    /* Ignore SIGInterrupt because when you CTRL-C the node
        booter we don't want the device to die, and it's the shells responsibility
        to send CTRL-C to all foreground processes (even children) */
    signal(SIGINT, SIG_IGN);

    Device_impl* device = ctor(devMgr_ior, id, label, profile, composite_device);
    
    if ( !skip_run ) {
        // assign logging context to the resource..to support logging interface
        device->saveLoggingContext( logcfg_uri, debug_level, ctx );
    }

    // setting all the execparams passed as argument, this method resides in the Resource_impl class
    device->setExecparamProperties(execparams);

    //perform post construction operations for the device
    std::string tmp_devMgr_ior = devMgr_ior;
    std::string tmp_profile = profile;
    std::string nic = "";
    try {
      device->postConstruction( tmp_profile, tmp_devMgr_ior, idm_channel_ior, nic, sig_fd);
    }
    catch( CF::InvalidObjectReference &ex ) {
      RH_NL_FATAL(logname, "Device " << label << ", Failed initialization and registration, terminating execution");
      if ( device ) device->_remove_ref();
      ossie::logging::Terminate();
      ossie::corba::OrbShutdown(true);
      exit(EXIT_FAILURE);
    } catch ( CORBA::SystemException &ex ) {
        std::ostringstream eout;
        eout<<"CORBA::"<<ex._name()<<"(";
        const char* minor = ex.NP_minorString();
        if (minor)
            eout<<minor;
        else {
            eout << "0x" << std::hex << ex.minor();
        }
        eout << ", CORBA::";
        switch (ex.completed()) {
            case CORBA::COMPLETED_YES:
                eout << "COMPLETED_YES";
                break;
            case CORBA::COMPLETED_NO:
                eout << "COMPLETED_NO";
                break;
            default:
                eout << "COMPLETED_MAYBE";
        }
        eout << ")";
        RH_NL_FATAL(logname, "Unable to complete Device construction: "<<eout.str());
        if ( device ) device->_remove_ref();
        ossie::logging::Terminate();
        ossie::corba::OrbShutdown(true);
        exit(EXIT_FAILURE);
    } catch ( ... ) {
        RH_NL_FATAL(logname, "device fatal failure");
        if ( device ) device->_remove_ref();
        ossie::logging::Terminate();
        ossie::corba::OrbShutdown(true);
        exit(EXIT_FAILURE);
    }

    if (skip_run) {
        return;
    }    
    device->run();
    RH_NL_DEBUG(logname, "Goodbye!");
    device->_remove_ref();
    ossie::logging::Terminate();
    ossie::corba::OrbShutdown(true);
}

std::string Device_impl::getLogConfig(const char* devmgr_ior, const char* log_config, std::string& devmgr_label)
{
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
