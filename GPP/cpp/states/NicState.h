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
#ifndef NIC_STATE_H_
#define NIC_STATE_H_

#include <string>
#include <vector>
#include <stdint.h>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include "State.h"

class NicState;

typedef boost::shared_ptr< NicState > NicStatePtr;


struct NicStateData
{
    NicStateData():
    rx_bytes(0),
    rx_compressed(0),
    rx_crc_errors(0),
    rx_dropped(0),
    rx_errors(0),
    rx_packets(0),
    tx_bytes(0),
    tx_compressed(0),
    tx_dropped(0),
    tx_errors(0),
    tx_packets(0),
    tx_queue_len(0),
    speed(0),
    flags(0),
    v6_scope_id(0),
    mtu(0)
    {}
    
    std::string interface;
    std::string device;
    std::string vlan;
    uint64_t rx_bytes;
    uint64_t rx_compressed;
    uint64_t rx_crc_errors;
    uint64_t rx_dropped;
    uint64_t rx_errors;
    uint64_t rx_packets;
    uint64_t tx_bytes;
    uint64_t tx_compressed;
    uint64_t tx_dropped;
    uint64_t tx_errors;
    uint64_t tx_packets;
    uint64_t tx_queue_len;
    uint64_t speed;
    unsigned int flags;
    std::string mac_address;
    std::string v4_address;
    std::string v4_netmask;
    std::string v4_broadcast;
    std::string v6_address;
    std::string v6_netmask;
    uint32_t v6_scope_id;
    unsigned int mtu;
    std::string state;
};

class NicState : public State
{
public:
    NicState( const std::string& interface );

    void update_state();

    std::string get_interface() const { return data_.interface; }
    std::string get_device() const { return data_.device; }
    std::string get_vlan() const { return data_.vlan; }
    uint64_t get_speed_mbit_per_sec() const { return data_.speed; }
    uint64_t get_rx_bytes() const { return data_.rx_bytes; }
    uint64_t get_rx_compressed() const { return data_.rx_compressed; }
    uint64_t get_rx_crc_errors() const { return data_.rx_crc_errors; }
    uint64_t get_rx_dropped() const { return data_.rx_dropped; }
    uint64_t get_rx_errors() const { return data_.rx_errors; }
    uint64_t get_rx_packets() const { return data_.rx_packets; }
    uint64_t get_tx_bytes() const { return data_.tx_bytes; }
    uint64_t get_tx_compressed() const { return data_.tx_compressed; }
    uint64_t get_tx_dropped() const { return data_.tx_dropped; }
    uint64_t get_tx_errors() const { return data_.tx_errors; }
    uint64_t get_tx_packets() const { return data_.tx_packets; }
    uint64_t get_tx_queue_len() const { return data_.tx_queue_len; }
    unsigned int get_flags() const { return data_.flags; }
    std::string get_mac_address() const { return data_.mac_address; }
    std::string get_v4_address() const { return data_.v4_address; }
    std::string get_v4_netmask() const { return data_.v4_netmask; }
    std::string get_v4_broadcast() const { return data_.v4_broadcast; }
    std::string get_v6_address() const { return data_.v6_address; }
    std::string get_v6_netmask() const { return data_.v6_netmask; }
    uint32_t get_v6_scope_id() const { return data_.v6_scope_id; }
    unsigned int get_mtu() const { return data_.mtu; }
    std::string get_state() const { return data_.state; }

private:
    void extract_device_and_vlan_from_interface();
    std::string nic_file_path( const std::string& suffix ) const;
    template<typename T> void bind_data_to_file( T& data, const std::string& filename );
    template<typename T> void extract_file_contents( T& data, const std::string& filename );
    void update_addresses();

protected:
    NicStateData data_;
    
private:
    typedef boost::function< void() > StateVariableUpdateFunction;
    std::vector<StateVariableUpdateFunction> update_functions;
};



#endif
