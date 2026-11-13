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


#ifndef __ALLOCATIONMANAGER__IMPL
#define __ALLOCATIONMANAGER__IMPL

#include <string>
#include <list>
#include <sstream>

#include <ossie/CF/cf.h>
#include <ossie/debug.h>

class DomainManager_impl;

class AllocationManager_impl: public virtual POA_CF::AllocationManager
{
    ENABLE_LOGGING
    
    public:
        AllocationManager_impl (DomainManager_impl* domainManager);
        ~AllocationManager_impl ();
        
        CF::AllocationManager::AllocationResponseSequence* allocate(const CF::AllocationManager::AllocationRequestSequence &requests);
        
        /* Allocates a set of dependencies only inside the local Domain */
        CF::AllocationManager::AllocationResponseSequence* allocateLocal(const CF::AllocationManager::AllocationRequestSequence &requests, const char* domainName);
        
        /* Deallocates a set of allocations */
        void deallocate(const CF::AllocationManager::allocationIDSequence &allocationIDs);
        
        /* Returns all current allocations on all Domains */
        CF::AllocationManager::AllocationStatusSequence* allocations(const CF::AllocationManager::allocationIDSequence &allocationIDs);
        
        /* Returns all current allocations that were made through the Allocation Manager that have not been deallocated */
        CF::AllocationManager::AllocationStatusSequence* localAllocations(const CF::AllocationManager::allocationIDSequence &allocationIDs);

        /* Lists up to 'count' devices within the given scope (local or all Domains). If there are more remaining, the out iterator can be used to fetch additional allocations. */
        void listDevices(CF::AllocationManager::DeviceScopeType deviceScope, CORBA::ULong count, CF::AllocationManager::DeviceLocationSequence_out deviceLocations, CF::DeviceLocationIterator_out iterator);
        
        /* Lists up to 'count' current allocations within the given scope (local or all Domains). If there are more remaining, the out iterator can be used to fetch additional allocations. */
        void listAllocations(CF::AllocationManager::AllocationScopeType allocScope, CORBA::ULong count, CF::AllocationManager::AllocationStatusSequence_out allocs, CF::AllocationStatusIterator_out ai);

        /* Returns all devices in all Domains that can be seen by any Allocation Manager seen by the local Allocation Manager */
        CF::AllocationManager::DeviceLocationSequence* allDevices();
        
        /* Returns all devices after policy is applied by any Allocation Manager seen by the local Allocation Manager */
        CF::AllocationManager::DeviceLocationSequence* authorizedDevices();
        
        /* Returns all devices that are located within the local Domain */
        CF::AllocationManager:: DeviceLocationSequence* localDevices();
        
        /* Returns all devices that are located within the local Domain */
        CF::DomainManager_ptr domainMgr();

        /* Allocates a set of dependencies for deployment; not part of the CORBA API */
        ossie::AllocationResult allocateDeployment(const std::string& requestID,
                                                   const CF::Properties& allocationProperties,
                                                   ossie::DeviceList& devices,
                                                   const std::string& sourceID,
                                                   const std::vector<std::string>& processorDeps,
                                                   const std::vector<ossie::SPD::NameVersionPair>& osDeps,
                                                   const CF::Properties& deviceRequires);

        /* Deallocates a set of allocations */
        template <class Iterator>
        void deallocate(Iterator first, const Iterator end)
        {
            CF::AllocationManager::allocationIDSequence invalidAllocations;
            invalidAllocations.length(0);

            boost::recursive_mutex::scoped_lock lock(allocationAccess);
            for (; first != end; ++first) {
                const std::string allocationId(*first);
                if (!deallocateSingle(allocationId)) {
                    LOG_TRACE(AllocationManager_impl, "Invalid allocation ID " << allocationId);
                    ossie::corba::push_back(invalidAllocations, allocationId.c_str());
                }
            }

            this->_domainManager->updateLocalAllocations(this->_allocations);
            this->_domainManager->updateRemoteAllocations(this->_remoteAllocations);
            if (invalidAllocations.length() != 0) {
                throw CF::AllocationManager::InvalidAllocationId(invalidAllocations);
            }
        }
        
        bool hasListenerAllocation(const CF::Properties& requestedProperties);

        // Local interface for persistance support
        void restoreLocalAllocations(const ossie::AllocationTable& localAllocations);
        void restoreRemoteAllocations(const ossie::RemoteAllocationTable& remoteAllocations);

        void restoreAllocations(ossie::AllocationTable& ref_allocations, std::map<std::string, CF::AllocationManager_var> &ref_remoteAllocations);

        void setLogger(rh_logger::LoggerPtr logptr) {
            _allocMgrLog = logptr;
        };

    private:
        CF::AllocationManager::AllocationResponseSequence* allocateDevices(const CF::AllocationManager::AllocationRequestSequence &requests, ossie::DeviceList& devices, const std::string& domainName);

        std::pair<ossie::AllocationType*,ossie::DeviceList::iterator> allocateRequest(const std::string& requestID,
                                                                                      const CF::Properties& allocationProperties,
                                                                                      ossie::DeviceList& devices,
                                                                                      const std::string& sourceID,
                                                                                      const std::vector<std::string>& processorDeps,
                                                                                      const std::vector<ossie::SPD::NameVersionPair>& osDeps,
                                                                                      const std::string& domainName,
                                                                                      const CF::Properties& deviceRequires = CF::Properties() );

        bool checkDeviceMatching(ossie::Properties& _prf, CF::Properties& externalProps, const CF::Properties& dependencyPropertiesFromComponent, const std::vector<std::string>& processorDeps, const std::vector<ossie::SPD::NameVersionPair>& osDeps);

        bool checkMatchingProperty(const ossie::Property* property, const CF::DataType& dependency);
        bool checkPartitionMatching( ossie::DeviceNode& node,
                                     const redhawk::PropertyMap& devicerequires );

        redhawk::PropertyMap getDeviceRequiredProperties( ossie::DeviceNode& node );


        bool allocateDevice(const CF::Properties& requestedProperties,
                            ossie::DeviceNode& device,
                            CF::Properties& allocatedProperties,
                            const std::vector<std::string>& processorDeps,
                            const std::vector<ossie::SPD::NameVersionPair>& osDeps,
                            const CF::Properties& deviceRequires = CF::Properties() );
        void partitionProperties(const CF::Properties& properties, std::vector<CF::Properties>& outProps);

        bool completeAllocations(CF::Device_ptr device, const std::vector<CF::Properties>& duplicates);

        bool deallocateSingle(const std::string& allocationID);
        bool deallocateLocal(const std::string& allocationID);
        bool deallocateRemote(const std::string& allocationID);

        DomainManager_impl* _domainManager;
        ossie::AllocationTable _allocations;
        ossie::RemoteAllocationTable _remoteAllocations;
        void unfilledRequests(CF::AllocationManager::AllocationRequestSequence &requests, const CF::AllocationManager::AllocationResponseSequence &result);
        rh_logger::LoggerPtr _allocMgrLog;

    protected:
        boost::recursive_mutex allocationAccess;
        
};                  /* END CLASS DEFINITION AllocationManager */
#endif              /* __ALLOCATIONMANAGER__IMPL */
