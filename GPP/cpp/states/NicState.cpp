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
#include "NicState.h"
#include "../utils/FileReader.h"
#include "../utils/IOError.h"

#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>

#include <iostream>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

NicState::NicState( const std::string& interface )
{
    data_.interface = interface;
    data_.device = interface;
    extract_device_and_vlan_from_interface();
    
    bind_data_to_file( data_.rx_bytes, nic_file_path("/statistics/rx_bytes") );
    bind_data_to_file( data_.rx_compressed, nic_file_path("/statistics/rx_compressed") );
    bind_data_to_file( data_.rx_crc_errors, nic_file_path("/statistics/rx_crc_errors") );
    bind_data_to_file( data_.rx_dropped, nic_file_path("/statistics/rx_dropped") );
    bind_data_to_file( data_.rx_errors, nic_file_path("/statistics/rx_errors") );
    bind_data_to_file( data_.rx_packets, nic_file_path("/statistics/rx_packets") );
    bind_data_to_file( data_.tx_bytes, nic_file_path("/statistics/tx_bytes") );
    bind_data_to_file( data_.tx_compressed, nic_file_path("/statistics/tx_compressed") );
    bind_data_to_file( data_.tx_dropped, nic_file_path("/statistics/tx_dropped") );
    bind_data_to_file( data_.tx_errors, nic_file_path("/statistics/tx_errors") );
    bind_data_to_file( data_.tx_packets, nic_file_path("/statistics/tx_packets") );
    bind_data_to_file( data_.tx_queue_len, nic_file_path("/tx_queue_len") );
    bind_data_to_file( data_.mac_address, nic_file_path("/address") );
    bind_data_to_file( data_.speed, nic_file_path("/speed") );
    bind_data_to_file( data_.mtu, nic_file_path("/mtu") );
    bind_data_to_file( data_.state, nic_file_path("/operstate") );
}

void 
NicState::extract_device_and_vlan_from_interface()
{
    std::string::size_type pos = data_.interface.find(".");
    if( pos != std::string::npos )
    {
        data_.device = data_.interface.substr(0, pos);
        data_.vlan = data_.interface.substr(pos+1);
    }
}

std::string 
NicState::nic_file_path( const std::string& suffix ) const
{
    return "/sys/class/net/" + data_.interface + suffix;
}

template<typename T> 
void 
NicState::bind_data_to_file( T& data, const std::string& filename )
{
    update_functions.push_back( boost::bind(&NicState::extract_file_contents<T>, 
                                            this, 
                                            boost::ref(data),
                                            filename ) );
}

template<typename T>
void NicState::extract_file_contents( T& data, const std::string& filename )
{
    try
    {
        std::stringstream str( FileReader::ReadFile( filename ) );
        str >> data;
    }
    catch( const IOError& e )
    {
        data = T();
    }
}

void
NicState::update_state()
{
    for( size_t i=0; i<update_functions.size(); ++i )
    {
        update_functions[i]();
    }
    
    if( data_.v4_address.empty() && data_.v6_address.empty() )
    {
        update_addresses();
    }
}

void
NicState::update_addresses()
{
    data_.v4_address = "INVALID";
    data_.v6_address = "INVALID";
    
    char addr[INET6_ADDRSTRLEN];
    char netmask[INET6_ADDRSTRLEN];
    struct ifaddrs *ifaddr, *ifa;
    void* sin_addr;
    void* sin_netmask;

    if (getifaddrs(&ifaddr) == -1) 
    {
        perror("getifaddrs");
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
    {
        if (ifa->ifa_addr == NULL)
            continue;  

        if( ifa->ifa_name == data_.interface )
        {
            data_.flags = ifa->ifa_flags;
            
            if(ifa->ifa_addr->sa_family==AF_INET)
            {
                sin_addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
                sin_netmask = &((struct sockaddr_in*)ifa->ifa_netmask)->sin_addr;
                
                if(inet_ntop(ifa->ifa_addr->sa_family, sin_addr, addr, sizeof(addr)) == NULL )
                {
                    perror("inet_ntop");
                }
                if(inet_ntop(ifa->ifa_addr->sa_family, sin_netmask, netmask, sizeof(netmask)) == NULL )
                {
                    perror("inet_ntop");
                }
                
                data_.v4_address = addr;
                data_.v4_netmask = netmask;
            }
            else if(ifa->ifa_addr->sa_family==AF_INET6)
            {
                sin_addr = &((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr;
                sin_netmask = &((struct sockaddr_in6*)ifa->ifa_netmask)->sin6_addr;
                
                if(inet_ntop(ifa->ifa_addr->sa_family, sin_addr, addr, sizeof(addr)) == NULL )
                {
                    perror("inet_ntop");
                }
                if(inet_ntop(ifa->ifa_addr->sa_family, sin_netmask, netmask, sizeof(netmask)) == NULL )
                {
                    perror("inet_ntop");
                }
                
                data_.v6_address = addr;
                data_.v6_netmask = netmask;
                data_.v6_scope_id = ((struct sockaddr_in6*)ifa->ifa_addr)->sin6_scope_id;
            }
        }
    }

    freeifaddrs(ifaddr);
}


