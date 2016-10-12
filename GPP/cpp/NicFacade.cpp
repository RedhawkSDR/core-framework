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
#include "NicFacade.h"

#include "states/NicState.h"
#include "statistics/NicAccumulator.h"
#include "NicAllocator.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include <net/if.h>

#if BOOST_FILESYSTEM_VERSION < 3
#define BOOST_PATH_STRING(x) (x)
#else
#define BOOST_PATH_STRING(x) (x).string()
#endif

const int MBIT_PER_MB = 8;
const double MBIT_PER_BIT = 1e-6;

NicFacade::NicFacade( const double& max_throughput_percent, 
                      const std::vector<std::string>& nic_interface_regexes,
                      std::vector<std::string>& filtered_nic_interfaces_reporting_data,
                      std::vector<interfaces_struct>& reporting_data,
                      std::vector<nic_metrics_struct_struct>& nic_metrics_reporting_data,
                      std::vector<nic_allocation_status_struct_struct>& nic_allocation_status_reporting_data ):
nic_interface_filter_(nic_interface_regexes, nic_states_, filtered_nic_states_),
filtered_nic_interfaces_reporting_data_(filtered_nic_interfaces_reporting_data),
reporting_data_(reporting_data),
nic_metrics_reporting_data_(nic_metrics_reporting_data),
nic_allocation_status_reporting_data_(nic_allocation_status_reporting_data)
{
  // find all interface devices for this host...
  initialize();

  // build allocator list for the nics that we would be watching.
  nic_allocator_ = boost::shared_ptr< NicAllocator >(new NicAllocator(filtered_nic_states_, max_throughput_percent, boost::bind(&NicFacade::get_throughput_by_device_bps, this, _1) ));

}

void 
NicFacade::initialize()
{
  std::vector<std::string> interfaces( poll_nic_interfaces() );
  for( std::vector<std::string>::const_iterator i=interfaces.begin(); i!=interfaces.end(); ++i )
    {
      const std::string& parsedInterface( *i );
                
      boost::shared_ptr<NicState> nic_state = get_or_insert_nic_state( parsedInterface );

      if( !has_nic_accumulator(nic_state->get_device()) )
        {
          RH_NL_DEBUG( "GPP", __FUNCTION__ << ": Adding NicAccumulator (" << nic_state->get_device() << ")" );
          add_nic_accumulator( nic_state );
            
          //            // Add new NicThroughputThresholdMonitor
          //(dynamicGPP_i, __FUNCTION__ << ": Adding interface (" << nic_state->get_device() << ")" );
          //            addThresholdMonitor( new NicThroughputThresholdMonitor(_identifier, nic_state->get_device(), &thresholds.nic_usage, boost::bind(&dynamicGPP_i::getThroughputByInterface, this, nic_state->get_device()) ) );
        }
    }
    
  nic_interface_filter_.filter();
}

std::vector<std::string> 
NicFacade::poll_nic_interfaces() const
{
  std::vector<std::string> interfaces;
  boost::filesystem::path p("/sys/class/net");
  boost::filesystem::directory_iterator end_iter;

  for(boost::filesystem::directory_iterator iter(p); iter != end_iter; ++iter) 
    {
      if(boost::filesystem::is_directory(iter->status())) 
        {
          std::ostringstream tmp;
          tmp << BOOST_PATH_STRING(iter->path());
          boost::filesystem::path test_file( tmp.str() + "/statistics/rx_bytes" );

          if(boost::filesystem::is_regular_file(test_file)) 
            {
              interfaces.push_back( BOOST_PATH_STRING(iter->path().filename()) );
            }
        }
    }
    
  return interfaces;
}

boost::shared_ptr<NicState>
NicFacade::get_or_insert_nic_state( const std::string& interface )
{
    std::map<std::string, boost::shared_ptr<NicState> >::iterator i = nic_states_.find(interface);
    if( nic_states_.end() == i )
    {
      RH_NL_DEBUG( "GPP", __FUNCTION__ << ": Adding NicState (" << interface << ")" );
      i = nic_states_.insert( std::make_pair(interface, new NicState(interface)) ).first;
    }
    return i->second;
}

bool
NicFacade::has_nic_accumulator( const std::string& device ) const
{
    return nic_accumulators_.find(device) != nic_accumulators_.end();
}
        
void 
NicFacade::add_nic_accumulator( const NicStatePtr &nic_state )
{
  NicAccumulatorPtr nic_accumulator( new NicAccumulator(nic_state) );
  BOOST_ASSERT( !nic_accumulator->get_device().empty() );
  nic_accumulators_.insert( std::make_pair(nic_accumulator->get_device(), nic_accumulator) );
}

bool
NicFacade::allocate_capacity( const nic_allocation_struct& alloc )
{
    bool success = nic_allocator_->allocate_capacity(alloc);
    write_nic_allocation_status_reporting_data();
    return success;
}
void 
NicFacade::deallocate_capacity( const nic_allocation_struct& alloc )
{
    nic_allocator_->deallocate_capacity(alloc);
    write_nic_allocation_status_reporting_data();
}


void 
NicFacade::update_state()
{
    for( NicStates::iterator i=nic_states_.begin(); i!=nic_states_.end(); ++i )
    {
        i->second->update_state();
    }
}

void
NicFacade::compute_statistics()
{
    for( NicAccumulators::iterator i=nic_accumulators_.begin(); i!=nic_accumulators_.end(); ++i )
    {
        i->second->compute_statistics();
    }
}

void 
NicFacade::report()
{
  // update state counters
  update_state();

  // update stats
  compute_statistics();

  // update 
    write_filtered_nic_interfaces_reporting_data();
    write_reporting_data();
    write_nic_metrics_reporting_data();
    write_nic_allocation_status_reporting_data();
}

void
NicFacade::write_filtered_nic_interfaces_reporting_data()
{
    filtered_nic_interfaces_reporting_data_.resize( filtered_nic_states_.size() );
    std::vector<std::string>::iterator nic_interface(filtered_nic_interfaces_reporting_data_.begin());
    
    NicStates::const_iterator i;
    for( i=filtered_nic_states_.begin(); i!=filtered_nic_states_.end(); ++i, ++nic_interface )
    {
        *nic_interface = i->first;
    }
}

void
NicFacade::write_reporting_data()
{
    reporting_data_.resize(nic_accumulators_.size());
    std::vector<interfaces_struct>::iterator nic(reporting_data_.begin());
    
    NicAccumulators::const_iterator i;
    for( i=nic_accumulators_.begin(); i!=nic_accumulators_.end(); ++i, ++nic )
    {
        boost::shared_ptr<NicAccumulator> nic_accumulator( i->second );
        
        nic->interface = nic_accumulator->get_device();
        nic->throughput = nic_accumulator->get_throughput_MB_per_sec();
        nic->vlans = nic_accumulator->get_vlans_string();
    }
}

void
NicFacade::write_nic_metrics_reporting_data()
{
    std::time_t t;
    time(&t);
    std::string time_str( asctime(gmtime(&t)) );
    
    nic_metrics_reporting_data_.resize( nic_states_.size() );
    std::vector<nic_metrics_struct_struct>::iterator report(nic_metrics_reporting_data_.begin());
    
    NicStates::const_iterator i;
    for( i=nic_states_.begin(); i!=nic_states_.end(); ++i, ++report )
    {
        boost::shared_ptr<NicState> nic_state( i->second );
        
        report->interface = nic_state->get_interface();
        report->mac_address = nic_state->get_mac_address();
        report->rate = nic_state->get_speed_mbit_per_sec();
        report->ipv4_address = nic_state->get_v4_address();
        report->ipv4_netmask = nic_state->get_v4_netmask();
        report->ipv4_broadcast = nic_state->get_v4_broadcast();
        report->ipv6_address = nic_state->get_v6_address();
        report->ipv6_netmask = nic_state->get_v6_netmask();
        report->ipv6_scope = boost::lexical_cast<std::string>(nic_state->get_v6_scope_id());
        report->flags = flags_to_str(nic_state->get_flags());
        report->module;
        report->mtu = boost::lexical_cast<std::string>(nic_state->get_mtu());
        report->state = nic_state->get_state();
        report->rx_bytes = boost::lexical_cast<std::string>(nic_state->get_rx_bytes());
        report->rx_compressed = boost::lexical_cast<std::string>(nic_state->get_rx_compressed());
        report->rx_crc_errors = boost::lexical_cast<std::string>(nic_state->get_rx_crc_errors());
        report->rx_dropped = boost::lexical_cast<std::string>(nic_state->get_rx_dropped());
        report->rx_errors = boost::lexical_cast<std::string>(nic_state->get_rx_errors());
        report->rx_packets = boost::lexical_cast<std::string>(nic_state->get_rx_packets());
        report->tx_bytes = boost::lexical_cast<std::string>(nic_state->get_tx_bytes());
        report->tx_compressed = boost::lexical_cast<std::string>(nic_state->get_tx_compressed());
        report->tx_dropped = boost::lexical_cast<std::string>(nic_state->get_tx_dropped());
        report->tx_errors = boost::lexical_cast<std::string>(nic_state->get_tx_errors());
        report->tx_packets = boost::lexical_cast<std::string>(nic_state->get_tx_packets());
        report->tx_queue_len = boost::lexical_cast<std::string>(nic_state->get_tx_queue_len());
        report->vlans = nic_state->get_vlan();
        report->multicast_support = (nic_state->get_flags() & IFF_MULTICAST) != 0;
        report->rate_allocated = nic_allocator_->get_allocated_device_throughput(nic_state->get_device()) * MBIT_PER_BIT;
        report->time_string_utc = time_str;
        report->time = t;
        report->current_throughput =  get_throughput_by_device(nic_state->get_device()) * MBIT_PER_MB;
    }
}

void
NicFacade::write_nic_allocation_status_reporting_data()
{
    nic_allocation_status_reporting_data_.resize( nic_allocator_->get_allocations().size() );
    std::vector<nic_allocation_status_struct_struct>::iterator report(nic_allocation_status_reporting_data_.begin());
    
    NicAllocator::Allocations::const_iterator i;
    for( i=nic_allocator_->get_allocations().begin(); i!=nic_allocator_->get_allocations().end(); ++i, ++report )
    {
        const nic_allocation_struct& alloc( i->second );
        
        report->identifier = alloc.identifier;
        report->data_rate = alloc.data_rate;
        report->data_size = alloc.data_size;
        report->multicast_support = alloc.multicast_support;
        report->ip_addressable = alloc.ip_addressable;
        report->interface = alloc.interface;
    }    
}


std::string 
NicFacade::flags_to_str( unsigned int flags ) const
{
    std::string str;
    
    if( flags & IFF_UP ) str += "UP ";
    if( flags & IFF_BROADCAST ) str += "BROADCAST ";
    if( flags & IFF_DEBUG ) str += "DEBUG ";
    if( flags & IFF_LOOPBACK ) str += "LOOPBACK ";
    if( flags & IFF_POINTOPOINT ) str += "POINTOPOINT ";
    if( flags & IFF_RUNNING ) str += "RUNNING ";
    if( flags & IFF_NOARP ) str += "NOARP ";
    if( flags & IFF_PROMISC ) str += "PROMISC ";
    if( flags & IFF_NOTRAILERS ) str += "NOTRAILERS ";
    if( flags & IFF_ALLMULTI ) str += "ALLMULTI ";
    if( flags & IFF_MASTER ) str += "MASTER ";
    if( flags & IFF_SLAVE ) str += "SLAVE ";
    if( flags & IFF_MULTICAST ) str += "MULTICAST ";
    if( flags & IFF_PORTSEL ) str += "PORTSEL ";
    if( flags & IFF_AUTOMEDIA ) str += "AUTOMEDIA ";
    if( flags & IFF_DYNAMIC ) str += "DYNAMIC ";
    
    return str;
}


std::vector<std::string> 
NicFacade::get_devices() const
{
    std::vector<std::string> devices;
    NicAccumulators::const_iterator i;
    for( i=nic_accumulators_.begin(); i!=nic_accumulators_.end(); ++i )
    {
        devices.push_back(i->first);
    }
    return devices;
}

float 
NicFacade::get_throughput_by_device( const std::string& device ) const
{
    NicAccumulators::const_iterator i = nic_accumulators_.find( device );
    if( nic_accumulators_.end() != i )
        return i->second->get_throughput_MB_per_sec();
    else
        return 0;
}

double 
NicFacade::get_throughput_by_device_bps( const std::string& device ) const
{
    return get_throughput_by_device(device) * MBIT_PER_MB * 1024*1024;
}

