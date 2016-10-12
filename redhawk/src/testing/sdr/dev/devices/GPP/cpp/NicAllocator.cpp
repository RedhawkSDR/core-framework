/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK GPP.
 *
 * REDHAWK GPP is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK GPP is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include "NicAllocator.h"
#include "states/NicState.h"

#include <boost/algorithm/string.hpp>

#include <sstream>

#include <arpa/inet.h>
#include <net/if.h>

static const int64_t MBITS_TO_BITS = 1000000;

NicAllocator::NicAllocator( const AllocatableNics& allocatable_nics,
                            const double& max_throughput_percent,
                            const CurrentThroughputFunction& current_throughput):
allocatable_nics_(allocatable_nics),
max_throughput_percent_(max_throughput_percent),
current_throughput_(current_throughput)
{
  //std::cout << " NicAllocator allocatable nics....." << allocatable_nics.size() << std::endl;
  AllocatableNics::const_iterator iter=allocatable_nics.begin();
  //for(; iter!=allocatable_nics.end(); iter++ ) std::cout << " Allocatable Nic " << iter->second->get_interface() << std::endl;
}

bool
NicAllocator::allocate_capacity( const nic_allocation_struct& input_alloc )
{
    //    std::cout << "Checking  Allocation identifier " << input_alloc.identifier << std::endl;
    validate_allocation(input_alloc);
    //    std::cout << "Validate  Allocation identifier " << input_alloc.identifier << std::endl;    
    // Lookup and merge any previous allocation
    nic_allocation_struct alloc;
    double delta_alloc_throughput(get_throughput(input_alloc));
    //std::cout << "Validate  Allocation identifier " << input_alloc.identifier << std::endl;    
    Allocations::iterator alloc_lookup = allocations_.find(input_alloc.identifier);
    if( alloc_lookup != allocations_.end() )
    {
        alloc = alloc_lookup->second;
        delta_alloc_throughput -= get_throughput(alloc);
    }

    MergeAllocationStructs(alloc, input_alloc);
    
    AllocatableNics::const_iterator iter;
    for( iter=allocatable_nics_.begin(); iter!=allocatable_nics_.end(); ++iter )
    {
        const NicState& nic( *iter->second );
        DeviceThroughputCapacity& nic_throughput_capacity( lookup_nic_throughput_capacity( nic ) );
        double current_throughput = current_throughput_( nic.get_device() );

        if( allocatable_by_is_addressable(alloc, nic) &&
            allocatable_by_multicast_support(alloc, nic) &&
            allocatable_by_interface(alloc, nic) &&
            allocatable_by_data_rate(delta_alloc_throughput, nic_throughput_capacity, current_throughput) )
        {        
            // Updated allocation map
            alloc.interface = nic.get_interface();
            allocations_[alloc.identifier] = alloc;

            // Update allocated capacity
            nic_throughput_capacity.allocated_throughput += delta_alloc_throughput;
            return true;
        }
    }
    
    return false;
}

void
NicAllocator::validate_allocation( const nic_allocation_struct& alloc ) const
{
    if( alloc.identifier.empty() )
    {
        std::stringstream errstr;
        errstr << "Invalid allocation (identifier=\"\")";
        throw InvalidAllocation(errstr.str());
    }
    
    if( !alloc.ip_addressable.empty() )
    {
        (void)GetV4AddressValue(alloc.ip_addressable);
    }
    
    if( !alloc.multicast_support.empty() )
    {
        if( !boost::iequals(alloc.multicast_support, "true") &&
            !boost::iequals(alloc.multicast_support, "false") )
        {
            std::stringstream errstr;
            errstr << "Invalid allocation (multicast_support=\"" << alloc.multicast_support << "\")";
            throw InvalidAllocation(errstr.str());
        }
    }
}

void
NicAllocator::MergeAllocationStructs( nic_allocation_struct& merged, const nic_allocation_struct& updated )
{
    merged.identifier = updated.identifier;
    merged.data_rate = updated.data_rate;
    merged.data_size = updated.data_size;
    merged.multicast_support = updated.multicast_support;
    merged.ip_addressable = updated.ip_addressable;
    
    if( merged.interface.empty() )
    {
        merged.interface = updated.interface;
    }
}

DeviceThroughputCapacity& 
NicAllocator::lookup_nic_throughput_capacity(const NicState& nic)
{
    DeviceThroughputCapacity& nic_throughput_capacity( device_throughput_capacities_[nic.get_device()] );
    if( nic_throughput_capacity.device.empty() )
    {
        nic_throughput_capacity.device = nic.get_device();
        nic_throughput_capacity.maximum_throughput = nic.get_speed_mbit_per_sec() * MBITS_TO_BITS;
    }
    return nic_throughput_capacity;
}

bool 
NicAllocator::allocatable_by_is_addressable( const nic_allocation_struct& alloc, const NicState& nic ) const
{
    if( alloc.ip_addressable.empty() )
        return true;
    
    return IsAddressableV4( alloc.ip_addressable, 
                            nic.get_v4_address(), 
                            nic.get_v4_netmask() );
}

bool 
NicAllocator::allocatable_by_multicast_support( const nic_allocation_struct& alloc, const NicState& nic ) const
{
    if( alloc.multicast_support.empty() )
        return true;
    
    bool alloc_multicast_support = boost::iequals(alloc.multicast_support, "true");
    bool nic_multicast_support = (nic.get_flags() & IFF_MULTICAST) != 0;
    
    return alloc_multicast_support == nic_multicast_support;
}

bool
NicAllocator::allocatable_by_interface( const nic_allocation_struct& alloc, const NicState& nic ) const
{
    if( alloc.interface.empty() )
        return true;
    
    return alloc.interface == nic.get_interface();
}

bool
NicAllocator::allocatable_by_data_rate( double delta_alloc_throughput, const DeviceThroughputCapacity& capacity, double current_throughput ) const
{
  RH_NL_TRACE( "GPP", __FUNCTION__ << ": { delta_alloc_throughput: " << delta_alloc_throughput << ", current_throughput: " << current_throughput << ", capacity: { device: " << capacity.device << ", maximum_throughput: " << capacity.maximum_throughput << ", allocated_throughput: " << capacity.allocated_throughput << " } }" );
    double requested_throughput = capacity.allocated_throughput + delta_alloc_throughput;
    double adjusted_max_throughput = capacity.maximum_throughput * max_throughput_percent_ / 100.0;
    
    return std::max(current_throughput, requested_throughput) <= adjusted_max_throughput;
}

bool
NicAllocator::IsAddressableV4( const std::string& addr1, const std::string& addr2, const std::string& netmask )
{
    //std::cout << __FUNCTION__ << ": { addr1: " << addr1 << ", addr2: " << addr2 << ", netmask: " << netmask << " }" << std::endl;
    uint32_t v4_address1 = GetV4AddressValue(addr1);
    uint32_t v4_address2 = GetV4AddressValue(addr2);
    uint32_t v4_netmask = GetV4AddressValue(netmask);

    return (v4_address1 & v4_netmask) == (v4_address2 & v4_netmask);
}

uint32_t
NicAllocator::GetV4AddressValue( const std::string& address )
{
    //std::cout << __FUNCTION__ << ": { address: " << address << " }" << std::endl;
    struct in_addr addr;
    const int s = inet_pton( AF_INET, address.c_str(), &addr );

    if( s <= 0 )
    {
        std::stringstream errstr;
        if( s == 0 )
        {
            errstr << "Address not in valid IPv4 presentation format (address=" << address << ")";
        }
        else
        {
            errstr << "Error during inet_pton (address=" << address << " msg=\"" << strerror(EAFNOSUPPORT) << "\")";
        }
        throw InvalidAllocation(errstr.str());
    }
    
    return ntohl(addr.s_addr);
}

void
NicAllocator::deallocate_capacity( const nic_allocation_struct& alloc )
{
    validate_allocation(alloc);
    
    Allocations::iterator iter = allocations_.find(alloc.identifier);
    if( iter != allocations_.end() )
    {
        AllocatableNics::const_iterator nic_iter = allocatable_nics_.find(iter->second.interface);
        if( nic_iter != allocatable_nics_.end() )
        {
            const NicState& nic( *nic_iter->second );
            DeviceThroughputCapacity& nic_throughput_capacity( lookup_nic_throughput_capacity( nic ) );

            nic_throughput_capacity.allocated_throughput -= get_throughput(iter->second);
        }
        allocations_.erase(iter);
    }
    else
    {
        std::stringstream errstr;
        errstr << "Unknown allocation (identifier=\"" << alloc.identifier << "\")";
        throw InvalidAllocation(errstr.str());
    }
}

double
NicAllocator::get_allocated_device_throughput(const std::string& device) const
{
    DeviceThroughputCapacities::const_iterator i = device_throughput_capacities_.find(device);
    if( i != device_throughput_capacities_.end() )
        return i->second.allocated_throughput;
    else
        return 0;
}
