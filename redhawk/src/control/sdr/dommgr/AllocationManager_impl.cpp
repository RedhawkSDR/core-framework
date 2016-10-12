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


#include <string>
#include <set>

#include <ossie/CF/WellKnownProperties.h>

#include "AllocationManager_impl.h"
#include "ossie/debug.h"
#include "ossie/CorbaUtils.h"
#include "ossie/ossieSupport.h"

namespace {
    static inline CF::AllocationManager::AllocationStatusType convertAllocTableEntry(const ossie::AllocationTable::value_type& entry)
    {
        CF::AllocationManager::AllocationStatusType status;
        status.allocationID = entry.first.c_str();
        status.requestingDomain = entry.second.requestingDomain.c_str();
        status.allocationProperties = entry.second.allocationProperties;
        status.allocatedDevice = CF::Device::_duplicate(entry.second.allocatedDevice);
        status.allocationDeviceManager = CF::DeviceManager::_duplicate(entry.second.allocationDeviceManager);
        return status;
    }
}

PREPARE_LOGGING(AllocationManager_impl);

AllocationManager_impl::AllocationManager_impl (DomainManager_impl* domainManager) :
    _domainManager(domainManager),
    _allocations()
{
}

AllocationManager_impl::~AllocationManager_impl ()
{
}

void AllocationManager_impl::unfilledRequests(CF::AllocationManager::AllocationRequestSequence &requests, const CF::AllocationManager::AllocationResponseSequence &result)
{
    if (requests.length() == 0) {
        return;
    }

    std::vector<unsigned int> unfilled_request_idx;
    unfilled_request_idx.resize(0);
    for (unsigned int req_idx=0; req_idx<result.length(); req_idx++) {
        std::string req_request_id = ossie::corba::returnString(requests[req_idx].requestID);
        bool found_match = false;
        for (unsigned int res_idx=0; res_idx<requests.length(); res_idx++) {
            std::string res_request_id = ossie::corba::returnString(result[res_idx].requestID);
            if (res_request_id == req_request_id) {
                found_match = true;
                break;
            }
        }
        if (not found_match) {
            unfilled_request_idx.push_back(req_idx);
        }
    }
    std::sort(unfilled_request_idx.begin(),unfilled_request_idx.end(),std::greater<unsigned int>());
    for (std::vector<unsigned int>::iterator ur_itr = unfilled_request_idx.begin(); ur_itr != unfilled_request_idx.end(); ur_itr++) {
        for (unsigned int idx=(*ur_itr); idx<requests.length()-1; idx++) {
            requests[idx] = requests[idx+1];
        }
        requests.length(requests.length()-1);
    }
    return;
}

CF::AllocationManager::AllocationResponseSequence* AllocationManager_impl::allocate(const CF::AllocationManager::AllocationRequestSequence &requests) throw (CF::AllocationManager::AllocationError)
{
    TRACE_ENTER(AllocationManager_impl)
    boost::recursive_mutex::scoped_lock lock(allocationAccess);
    
    // try to fulfill the request locally
    const std::string domainName = this->_domainManager->getDomainManagerName();

    CF::AllocationManager::AllocationResponseSequence_var result = this->allocateLocal(requests, domainName.c_str());

    if (result->length() != requests.length()) {
        const ossie::DomainManagerList remoteDomains = this->_domainManager->getRegisteredRemoteDomainManagers();
        ossie::DomainManagerList::const_iterator remoteDomains_itr = remoteDomains.begin();

        CF::AllocationManager::AllocationRequestSequence remaining_requests;
        remaining_requests.length(requests.length());
        for (unsigned ridx=0;ridx<requests.length();ridx++) {
            remaining_requests[ridx] = requests[ridx];
        }

        while ((remoteDomains_itr != remoteDomains.end()) and (result->length() != requests.length())) {
            unfilledRequests(remaining_requests, result);
            CORBA::String_var domain_name = this->_domainManager->name();
            CF::AllocationManager_var allocationMgr = remoteDomains_itr->domainManager->allocationMgr();
            CF::AllocationManager::AllocationResponseSequence_var new_result = allocationMgr->allocateLocal(remaining_requests, domain_name);
            for (unsigned int idx=0; idx<new_result->length(); idx++) {
                ossie::corba::push_back(result, new_result[idx]);
                ossie::RemoteAllocationType allocation;
                allocation.allocationID = new_result[idx].allocationID;
                allocation.allocatedDevice = CF::Device::_duplicate(new_result[idx].allocatedDevice);
                allocation.allocationDeviceManager = CF::DeviceManager::_duplicate(new_result[idx].allocationDeviceManager);
                allocation.allocationProperties = new_result[idx].allocationProperties;
                allocation.requestingDomain = domainName;
                allocation.allocationManager = CF::AllocationManager::_duplicate(allocationMgr);
                _remoteAllocations[allocation.allocationID] = allocation;
            }
            remoteDomains_itr++;
        }

        // allocateLocal updates the database, so update only if remote allocations were needed
        this->_domainManager->updateRemoteAllocations(this->_remoteAllocations);
    }

    TRACE_EXIT(AllocationManager_impl)
    return result._retn();
}

/* Allocates a set of dependencies only inside the local Domain */
CF::AllocationManager::AllocationResponseSequence* AllocationManager_impl::allocateLocal(const CF::AllocationManager::AllocationRequestSequence &requests, const char* domainName) throw (CF::AllocationManager::AllocationError)
{
    TRACE_ENTER(AllocationManager_impl);

    CF::AllocationManager::AllocationResponseSequence* results;
    if (requests.length() > 0) {
        ossie::DeviceList registeredDevices = this->_domainManager->getRegisteredDevices();
        results = allocateDevices(requests, registeredDevices, domainName);
    } else {
        results = new CF::AllocationManager::AllocationResponseSequence();
    }

    TRACE_EXIT(AllocationManager_impl);
    return results;
}

CF::AllocationManager::AllocationResponseSequence* AllocationManager_impl::allocateDevices(const CF::AllocationManager::AllocationRequestSequence &requests, ossie::DeviceList& devices, const std::string& domainName)
{
    LOG_TRACE(AllocationManager_impl, "Servicing " << requests.length() << " allocation request(s)");
    CF::AllocationManager::AllocationResponseSequence_var response = new CF::AllocationManager::AllocationResponseSequence();

    typedef std::list<ossie::AllocationType*> LocalAllocationList;
    LocalAllocationList local_allocations;

    for (unsigned int request_idx=0; request_idx<requests.length(); request_idx++) {
        const CF::AllocationManager::AllocationRequestType& request = requests[request_idx];
        const std::string requestID(request.requestID);
        LOG_TRACE(AllocationManager_impl, "Allocation request " << requestID
                  << " contains " << request.allocationProperties.length() << " properties");

        // Get device identifiers, and ensure that no device references are nil
        std::vector<std::string> requestedDeviceIDs;
        for (unsigned int device_idx = 0; device_idx < request.requestedDevices.length(); ++device_idx) {
            CF::Device_ptr device = request.requestedDevices[device_idx];
            if (!CORBA::is_nil(device)) {
                requestedDeviceIDs.push_back(ossie::corba::returnString(device->identifier()));
            }
        }
        if (requestedDeviceIDs.size() != request.requestedDevices.length()) {
            // At least one requested device was nil
            continue;
        }

        // If a requested device list was given, skip devices not in list
        ossie::DeviceList requestedDevices = devices;
        if (!requestedDeviceIDs.empty()) {
            for (ossie::DeviceList::iterator node = requestedDevices.begin(); node != requestedDevices.end(); ++node) {
                if (std::find(requestedDeviceIDs.begin(), requestedDeviceIDs.end(), (*node)->identifier) == requestedDeviceIDs.end()) {
                    node = requestedDevices.erase(node);
                }
            }
        }

        std::pair<ossie::AllocationType*,ossie::DeviceList::iterator> result = allocateRequest(requestID, request.allocationProperties, requestedDevices, std::vector<std::string>(), std::vector<ossie::SPD::NameVersionPair>(), domainName);
        if (result.first) {
            local_allocations.push_back(result.first);
            ossie::AllocationType* allocation(result.first);
            const std::string requestID(request.requestID);
            ossie::corba::push_back(response, ossie::assembleResponse(requestID, allocation->allocationID, allocation->allocationProperties, allocation->allocatedDevice, allocation->allocationDeviceManager));
        }
    }

    // Update the database
    boost::recursive_mutex::scoped_lock lock(allocationAccess);
    for (LocalAllocationList::iterator alloc = local_allocations.begin(); alloc != local_allocations.end(); ++alloc) {
        this->_allocations[(*alloc)->allocationID] = **alloc;
        delete *alloc;
    }

    if (response->length() != 0) {
        this->_domainManager->updateLocalAllocations(this->_allocations);
    }
    return response._retn();
}

std::pair<ossie::AllocationType*,ossie::DeviceList::iterator> AllocationManager_impl::allocateRequest(const std::string& requestID, const CF::Properties& dependencyProperties, ossie::DeviceList& devices, const std::vector<std::string>& processorDeps, const std::vector<ossie::SPD::NameVersionPair>& osDeps, const std::string& domainName)
{
    for (ossie::DeviceList::iterator iter = devices.begin(); iter != devices.end(); ++iter) {
        boost::shared_ptr<ossie::DeviceNode> node = *iter;
        CF::Properties allocatedProperties;
        if (allocateDevice(dependencyProperties, *node, allocatedProperties, processorDeps, osDeps)) {
            ossie::AllocationType* allocation = new ossie::AllocationType();
            allocation->allocationID = ossie::generateUUID();
            allocation->allocatedDevice = CF::Device::_duplicate(node->device);
            allocation->allocationDeviceManager = CF::DeviceManager::_duplicate(node->devMgr.deviceManager);
            allocation->allocationProperties = allocatedProperties;
            allocation->requestingDomain = domainName;
            return std::make_pair(allocation, iter);
        }
    }
    return std::make_pair((ossie::AllocationType*)0, devices.end());
}

ossie::AllocationResult AllocationManager_impl::allocateDeployment(const std::string& requestID, const CF::Properties& allocationProperties, ossie::DeviceList& devices, const std::vector<std::string>& processorDeps, const std::vector<ossie::SPD::NameVersionPair>& osDeps)
{
    const std::string domainName = this->_domainManager->getDomainManagerName();
    std::pair<ossie::AllocationType*,ossie::DeviceList::iterator> result = allocateRequest(requestID, allocationProperties, devices, processorDeps, osDeps, domainName);
    if (result.first) {
        // Update the allocation table, including the persistence store
        const std::string allocationID = result.first->allocationID;
        boost::recursive_mutex::scoped_lock lock(allocationAccess);
        this->_allocations[allocationID] = *(result.first);
        this->_domainManager->updateLocalAllocations(this->_allocations);

        // Delete the temporary
        delete result.first;

        return ossie::AllocationResult(allocationID, *result.second);
    }
    return std::make_pair(std::string(), boost::shared_ptr<ossie::DeviceNode>());
}

bool AllocationManager_impl::allocateDevice(const CF::Properties& requestedProperties, ossie::DeviceNode& node, CF::Properties& allocatedProperties, const std::vector<std::string>& processorDeps, const std::vector<ossie::SPD::NameVersionPair>& osDeps)
{
    if (!ossie::corba::objectExists(node.device)) {
        LOG_WARN(AllocationManager_impl, "Not using device for uses_device allocation " << node.identifier << " because it no longer exists");
        return false;
    }
    try {
        if (node.device->usageState() == CF::Device::BUSY) {
            return false;
        }
    } catch ( ... ) {
        // bad device reference or device in an unusable state
        LOG_WARN(AllocationManager_impl, "Unable to verify state of device " << node.identifier);
        return false;
    }

    LOG_TRACE(AllocationManager_impl, "Allocating against device " << node.identifier);

    // Determine whether or not the device in question has the required matching properties
    CF::Properties allocProps;
    if (!checkDeviceMatching(node.prf, allocProps, requestedProperties, processorDeps, osDeps)) {
        LOG_TRACE(AllocationManager_impl, "Matching failed");
        return false;
    }

    // If there are no external properties to allocate, the allocation is
    // already successful
    if (allocProps.length() == 0) {
        LOG_TRACE(AllocationManager_impl, "Allocation requires no capacity from device");
        return true;
    }

    // If there are duplicates in the allocation sequence, break up the allocation into multiple calls
    std::vector<CF::Properties> allocations;
    partitionProperties(allocProps, allocations);

    LOG_TRACE(AllocationManager_impl, "Allocating " << allocProps.length() << " properties ("
              << allocations.size() << " calls)");
    try {
        if (!this->completeAllocations(node.device, allocations)) {
            LOG_TRACE(AllocationManager_impl, "Device lacks sufficient capacity");
            return false;
        }
    } catch (const CF::Device::InvalidCapacity& e) {
        LOG_TRACE(AllocationManager_impl, "Device reported invalid capacity");
        return false;
    } catch (const CF::Device::InsufficientCapacity& e) {
        LOG_TRACE(AllocationManager_impl, "Device reported insufficient capacity");
        return false;
    }

    // Transfer ownership of the allocated properties to the caller
    ossie::corba::move(allocatedProperties, allocProps);
    LOG_TRACE(AllocationManager_impl, "Allocation successful");
    return true;
}

void AllocationManager_impl::partitionProperties(const CF::Properties& properties, std::vector<CF::Properties>& outProps)
{
    std::set<std::string> identifiers;
    size_t start = 0;
    for (size_t index = 0; index < properties.length(); ++index) {
        const std::string propertyID(properties[index].id);
        if (identifiers.count(propertyID) > 0) {
            // Duplicate property, partition at this point
            outProps.push_back(ossie::corba::slice(properties, start, index));

            start = index;
            identifiers.clear();
        }
        identifiers.insert(propertyID);
    }

    // Copy remaining partition
    if (start < properties.length()) {
        outProps.push_back(ossie::corba::slice(properties, start));
    }
}

bool AllocationManager_impl::completeAllocations(CF::Device_ptr device, const std::vector<CF::Properties>& allocations)
{
    for (size_t ii = 0; ii < allocations.size(); ++ii) {
        try {
            if (device->allocateCapacity(allocations[ii])) {
                // Allocation succeeded, try next
                continue;
            }
        } CATCH_LOG_WARN(AllocationManager_impl, "Device allocation raised an exception");

        // An allocation failed; backtrack and deallocate any prior successes
        bool warned = false;
        for (size_t undo = ii; undo > 0; --undo) {
            try {
                device->deallocateCapacity(allocations[undo-1]);
            } catch (...) {
                if (!warned) {
                    // If a symmetric deallocateCapacity failes, the device is
                    // probably in a bad state; only warn about it once
                    LOG_WARN(AllocationManager_impl, "Device deallocation on cleanup raised an exception");
                    warned = true;
                }
            }
        }
        return false;
    }
    return true;
}

bool AllocationManager_impl::checkMatchingProperty(const ossie::Property* property, const CF::DataType& dependency)
{
    // Only attempt matching for simple properties
    const ossie::SimpleProperty* simpleProp = dynamic_cast<const ossie::SimpleProperty*>(property);
    if (!simpleProp) {
        LOG_ERROR(AllocationManager_impl, "Invalid action '" << property->getAction()
                  << "' for non-simple property " << property->getID());
        return false;
    }

    // Convert the input Any to the property's data type via string; if it came
    // from the ApplicationFactory, it's already a string, but a remote request
    // could be of any type
    const CF::DataType allocProp = ossie::convertPropertyToDataType(simpleProp);
    const CORBA::Any depValue = ossie::convertAnyToPropertyType(dependency.value, simpleProp);

    std::string action = simpleProp->getAction();
    LOG_TRACE(AllocationManager_impl, "Matching " << simpleProp->getID() << " '" << simpleProp->getValue()
              << "' " << action << " '" << ossie::any_to_string(dependency.value) << "'");

    // Per section D.4.1.1.7 the allocation property is on the left side of the action
    // and the dependency value is on the right side of the action
    return ossie::compare_anys(allocProp.value, depValue, action);
}

bool AllocationManager_impl::checkDeviceMatching(ossie::Properties& prf, CF::Properties& externalProperties, const CF::Properties& dependencyProperties, const std::vector<std::string>& processorDeps, const std::vector<ossie::SPD::NameVersionPair>& osDeps)
{
    // Check for a matching processor, which only happens in deployment
    if (!processorDeps.empty()) {
        if (!ossie::checkProcessor(processorDeps, prf.getAllocationProperties())) {
            LOG_TRACE(AllocationManager_impl, "Device did not match requested processor");
            return false;
        } else {
            LOG_TRACE(AllocationManager_impl, "Matched processor name");
        }
    }

    // Likewise, check for OS name/version
    if (!osDeps.empty()) {
        if (!ossie::checkOs(osDeps, prf.getAllocationProperties())) {
            LOG_TRACE(AllocationManager_impl, "Device did not match requested OS name/version");
            return false;
        } else {
            LOG_TRACE(AllocationManager_impl, "Matched OS name/version");
        }
    }

    int matches = 0;

    for (unsigned int index = 0; index < dependencyProperties.length(); ++index) {
        const CF::DataType& dependency = dependencyProperties[index];
        const std::string propId(dependency.id);
        const ossie::Property* property = prf.getAllocationProperty(propId);

        if (!property) {
            LOG_TRACE(AllocationManager_impl, "Device has no property " << propId);
            return false;
        } else if (property->isExternal()) {
            // Collect properties with an action of "external" for a later
            // allocateCapacity() call
            LOG_TRACE(AllocationManager_impl, "Adding external property " << propId);
            ossie::corba::push_back(externalProperties, ossie::convertDataTypeToPropertyType(dependency, property));
        } else {
            // Evaluate matching properties right now
            if (!checkMatchingProperty(property, dependency)) {
                return false;
            } else {
                ++matches;
            }   
        }
    }

    LOG_TRACE(AllocationManager_impl, "Matched " << matches << " properties");
    return true;
}

/* Deallocates a set of allocations */
void AllocationManager_impl::deallocate(const CF::AllocationManager::allocationIDSequence &allocationIDs) throw (CF::AllocationManager::InvalidAllocationId)
{
    TRACE_ENTER(AllocationManager_impl);
    if (allocationIDs.length() > 0) {
        deallocate(allocationIDs.get_buffer(), allocationIDs.get_buffer() + allocationIDs.length());
    }
    TRACE_EXIT(AllocationManager_impl);
}

/* Returns all current allocations on all Domains */
CF::AllocationManager::AllocationStatusSequence* AllocationManager_impl::allocations(const CF::AllocationManager::allocationIDSequence &allocationIDs) throw (CF::AllocationManager::InvalidAllocationId)
{
    TRACE_ENTER(AllocationManager_impl)
    boost::recursive_mutex::scoped_lock lock(allocationAccess);
    
    CF::AllocationManager::AllocationStatusSequence_var result = new CF::AllocationManager::AllocationStatusSequence();

    if (allocationIDs.length() == 0) {
        // Empty allocation ID list, return all allocations

        // Start with the local allocations
        result = localAllocations(allocationIDs);

        // Add the remote allocations to the end of the results
        size_t offset = result->length();
        result->length(offset + _remoteAllocations.size());
        ossie::RemoteAllocationTable::const_iterator start = _remoteAllocations.begin();
        const ossie::RemoteAllocationTable::const_iterator end = _remoteAllocations.end();
        for (; start != end; ++offset, ++start) {
            result[offset] = convertAllocTableEntry(*start);
        }
    } else {
        // Caller provided a list of requested allocation IDs
        CF::AllocationManager::allocationIDSequence invalid_ids;
        result->length(allocationIDs.length());
        for (size_t ii = 0; ii < allocationIDs.length(); ++ii) {
            // Try to find the allocation ID locally
            const std::string alloc_id(allocationIDs[ii]);
            ossie::AllocationTable::const_iterator alloc = _allocations.find(alloc_id);
            if (alloc != _allocations.end()) {
                // Valid local allocation, convert into CORBA sequence element
                result[ii] = convertAllocTableEntry(*alloc);
            } else {
                // Not found locally, try remote
                ossie::RemoteAllocationTable::const_iterator ralloc = _remoteAllocations.find(alloc_id);
                if (ralloc != _remoteAllocations.end()) {
                    result[ii] = convertAllocTableEntry(*ralloc);
                } else {
                    // Allocation was not found, mark ID as invalid
                    ossie::corba::push_back(invalid_ids, allocationIDs[ii]);
                }
            }
        }

        // If one or more allocation IDs was invalid, throw an exception
        if (invalid_ids.length() > 0) {
            throw CF::AllocationManager::InvalidAllocationId(invalid_ids);
        }
    }
    
    TRACE_EXIT(AllocationManager_impl)
    return result._retn();
}

/* Returns all current allocations that were made through the Allocation Manager that have not been deallocated */
CF::AllocationManager::AllocationStatusSequence* AllocationManager_impl::localAllocations(const CF::AllocationManager::allocationIDSequence &allocationIDs) throw (CF::AllocationManager::InvalidAllocationId)
{
    TRACE_ENTER(AllocationManager_impl)
    boost::recursive_mutex::scoped_lock lock(allocationAccess);

    CF::AllocationManager::AllocationStatusSequence_var result = new CF::AllocationManager::AllocationStatusSequence();
    if (allocationIDs.length() == 0) {
        // Empty allocation ID list, return all allocations
        result->length(_allocations.size());
        ossie::AllocationTable::const_iterator start = _allocations.begin();
        const ossie::AllocationTable::const_iterator end = _allocations.end();
        for (int ii = 0; start != end; ++ii, ++start) {
            result[ii] = convertAllocTableEntry(*start);
        }
    } else {
        // Caller provided a list of requested allocation IDs
        CF::AllocationManager::allocationIDSequence invalid_ids;
        result->length(allocationIDs.length());
        for (size_t ii = 0; ii < allocationIDs.length(); ++ii) {
            ossie::AllocationTable::const_iterator alloc = _allocations.find(std::string(allocationIDs[ii]));
            if (alloc != _allocations.end()) {
                // Valid allocation ID, convert into CORBA sequence element
                result[ii] = convertAllocTableEntry(*alloc);
            } else {
                // Allocation was not found, mark ID as invalid
                ossie::corba::push_back(invalid_ids, allocationIDs[ii]);
            }
        }

        // If one or more allocation IDs was invalid, throw an exception
        if (invalid_ids.length() > 0) {
            throw CF::AllocationManager::InvalidAllocationId(invalid_ids);
        }
    }
    
    TRACE_EXIT(AllocationManager_impl)
    return result._retn();
}

/* Returns all devices in all Domains that can be seen by any Allocation Manager seen by the local Allocation Manager */
CF::AllocationManager::DeviceLocationSequence* AllocationManager_impl::allDevices()
{
    TRACE_ENTER(AllocationManager_impl)
    boost::recursive_mutex::scoped_lock lock(allocationAccess);

    // Start with local devices
    CF::AllocationManager::DeviceLocationSequence_var result = localDevices();

    // Add local devices from remote domains
    const ossie::DomainManagerList remoteDomains = this->_domainManager->getRegisteredRemoteDomainManagers();
    const ossie::DomainManagerList::const_iterator end = remoteDomains.end();
    for (ossie::DomainManagerList::const_iterator start = remoteDomains.begin(); start != end; ++start) {
        CF::AllocationManager_var allocationMgr = start->domainManager->allocationMgr();
        CF::AllocationManager::DeviceLocationSequence_var remoteDevices = allocationMgr->localDevices();
        LOG_TRACE(AllocationManager_impl, "Adding " << remoteDevices->length() << " device(s) from domain '"
                  << ossie::corba::returnString(start->domainManager->name()) << "' to list");
        ossie::corba::extend(result, remoteDevices);
    }
    LOG_TRACE(AllocationManager_impl, result->length() << " total device(s)");
    
    TRACE_EXIT(AllocationManager_impl)
    return result._retn();
}

/* Returns all devices after policy is applied by any Allocation Manager seen by the local Allocation Manager */
CF::AllocationManager::DeviceLocationSequence* AllocationManager_impl::authorizedDevices()
{
    TRACE_ENTER(AllocationManager_impl)
    boost::recursive_mutex::scoped_lock lock(allocationAccess);

    // Default implementation has no policy engine; return all local devices
    CF::AllocationManager::DeviceLocationSequence_var result = localDevices();
    
    TRACE_EXIT(AllocationManager_impl)
    return result._retn();
}

/* Returns all devices that are located within the local Domain */
CF::AllocationManager::DeviceLocationSequence* AllocationManager_impl::localDevices()
{
    TRACE_ENTER(AllocationManager_impl)
    boost::recursive_mutex::scoped_lock lock(allocationAccess);

    // Get a point-in-time copy of the domain's devices
    const ossie::DeviceList registeredDevices = _domainManager->getRegisteredDevices();
    const std::string domainName = _domainManager->getDomainManagerName();

    // Copy the devices in the local device list into CORBA sequence
    CF::AllocationManager::DeviceLocationSequence_var result = new CF::AllocationManager::DeviceLocationSequence();
    result->length(registeredDevices.size());
    ossie::DeviceList::const_iterator start = registeredDevices.begin();
    const ossie::DeviceList::const_iterator end = registeredDevices.end();
    for (int ii = 0; start != end; ++ii, ++start) {
        result[ii].domainName = domainName.c_str();
        result[ii].pools.length(0);
        result[ii].devMgr = CF::DeviceManager::_duplicate((*start)->devMgr.deviceManager);
        result[ii].dev = CF::Device::_duplicate((*start)->device);
    }
    LOG_TRACE(AllocationManager_impl, result->length() << " local device(s)");

    TRACE_EXIT(AllocationManager_impl)
    return result._retn();
}

/* Returns a link to the local Domain */
CF::DomainManager_ptr AllocationManager_impl::domainMgr()
{
    TRACE_ENTER(AllocationManager_impl);

    TRACE_EXIT(AllocationManager_impl);
    return _domainManager->_this();
}

void AllocationManager_impl::restoreLocalAllocations (const ossie::AllocationTable& localAllocations)
{
    _allocations = localAllocations;
}

void AllocationManager_impl::restoreRemoteAllocations (const ossie::RemoteAllocationTable& remoteAllocations)
{
    _remoteAllocations = remoteAllocations;
}

void AllocationManager_impl::restoreAllocations (ossie::AllocationTable &ref_allocations, std::map<std::string, CF::AllocationManager_var> &ref_remoteAllocations)
{
    // NB: This function remains for backwards-compatibility between 1.10.1 and
    //     1.10.0 databases; remove for 1.11
    _allocations = ref_allocations;
    
    _remoteAllocations.clear();
    std::map<std::string, CF::AllocationManager_var>::const_iterator start = ref_remoteAllocations.begin();
    const std::map<std::string, CF::AllocationManager_var>::const_iterator end = ref_remoteAllocations.end();
    for (; start != end; ++start) {
        // Contact the remote AllocationManager to get the full state for each
        // allocation
        LOG_TRACE(AllocationManager_impl, "Restoring allocation '" << start->first << "'");
        CF::AllocationManager::allocationIDSequence alloc_ids;
        alloc_ids.length(1);
        alloc_ids[0] = start->first.c_str();
        CF::AllocationManager::AllocationStatusSequence_var result;
        try {
            result = start->second->localAllocations(alloc_ids);
        } catch (const CORBA::Exception& ex) {
            LOG_ERROR(AllocationManager_impl, "Unable to restore allocation '" << start->first << "': CORBA::"
                      << ex._name());
            continue;
        }

        ossie::RemoteAllocationType allocation;
        allocation.allocationID = result[0].allocationID;
        allocation.allocatedDevice = CF::Device::_duplicate(result[0].allocatedDevice);
        allocation.allocationDeviceManager = CF::DeviceManager::_duplicate(result[0].allocationDeviceManager);
        allocation.allocationProperties = result[0].allocationProperties;
        allocation.requestingDomain = _domainManager->getDomainManagerName();
        allocation.allocationManager = CF::AllocationManager::_duplicate(start->second);
        _remoteAllocations[allocation.allocationID] = allocation;
    }

    // Update the persistence store with the new database format
    _domainManager->updateLocalAllocations(_allocations);
    _domainManager->updateRemoteAllocations(_remoteAllocations);
}

/* Deallocates a single allocation (assumes lock is held) */
bool AllocationManager_impl::deallocateSingle(const std::string& allocationID)
{
    if (deallocateLocal(allocationID)) {
        return true;
    } else if (deallocateRemote(allocationID)) {
        return true;
    } else {
        return false;
    }
}

bool AllocationManager_impl::deallocateLocal(const std::string& allocationID)
{
    ossie::AllocationTable::iterator alloc = this->_allocations.find(allocationID);
    if (alloc == this->_allocations.end()) {
        return false;
    }

    const ossie::AllocationType& localAlloc = alloc->second;
    std::vector<CF::Properties> allocations;
    partitionProperties(localAlloc.allocationProperties, allocations);
    LOG_TRACE(AllocationManager_impl, "Deallocating " << localAlloc.allocationProperties.length()
              << " properties (" << allocations.size() << " calls) for local allocation " << allocationID);
    if (!ossie::corba::objectExists(localAlloc.allocatedDevice)) {
        LOG_WARN(AllocationManager_impl, "Not deallocating capacity a device because it no longer exists");
    } else {
        bool warned = false;
        for (size_t index = 0; index < allocations.size(); ++index) {
            try {
                localAlloc.allocatedDevice->deallocateCapacity(allocations[index]);
            } catch (...) {
                if (!warned) {
                    // If a symmetric deallocateCapacity failes, the device is
                    // probably in a bad state; only warn about it once
                    LOG_WARN(AllocationManager_impl, "Deallocation raised an exception");
                    warned = true;
                }
            }
        }
    }
    this->_allocations.erase(alloc);
    return true;
}

bool AllocationManager_impl::deallocateRemote(const std::string& allocationID)
{
    ossie::RemoteAllocationTable::iterator alloc = this->_remoteAllocations.find(allocationID);
    if (alloc == this->_remoteAllocations.end()) {
        return false;
    }
     
    LOG_TRACE(AllocationManager_impl, "Deallocating remote allocation " << allocationID);
    CF::AllocationManager::allocationIDSequence allocations;
    allocations.length(1);
    allocations[0] = allocationID.c_str();
    try {
        alloc->second.allocationManager->deallocate(allocations);
    } catch (const CF::AllocationManager::InvalidAllocationId&) {
        // Although the remote AllocationManager disagrees, the allocation ID
        // was valid on this side and should be removed
    } catch (...) {
        // Some other failure occurred; remove the allocation from the table
        // and continue
        LOG_WARN(AllocationManager_impl, "Remote deallocation " << allocationID << " failed");
    }
    this->_remoteAllocations.erase(alloc);
    return true;
}
