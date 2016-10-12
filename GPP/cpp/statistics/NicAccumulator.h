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
#ifndef NIC_ACCUMULATOR_H_
#define	NIC_ACCUMULATOR_H_

#include <stdint.h>
#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>

#include "Statistics.h"
#include "states/NicState.h"

class NicAccumulator;
typedef boost::shared_ptr< NicAccumulator > NicAccumulatorPtr;


class NicAccumulator : public Statistics
{
public:
    typedef boost::function< double() > CurrentTimeFunction;
    typedef std::vector<std::string> Vlans;

public:
    NicAccumulator();
    NicAccumulator( const NicStatePtr &nicState );
    
    void add_nic( const NicStatePtr& nic_state );

    void compute_statistics();

    std::string get_device() const;
    const Vlans& get_vlans() const { return vlans_; }
    std::string get_vlans_string() const { return vlans_string_; }
    double get_throughput_MB_per_sec() const { return throughput_; }

    void set_current_time_function( CurrentTimeFunction current_time );
    static double CurrentTime();

private:
    void validate_device( const std::string& device ) const;
    void add_vlan( const std::string& vlan );

private:
    typedef std::vector<boost::shared_ptr<const NicState> > NicStates;
    
    uint64_t prev_rx_bytes_;
    uint64_t prev_tx_bytes_;
    double prev_time_;
    double throughput_;

    NicStates nic_states_;
    Vlans vlans_;
    std::string vlans_string_;

    CurrentTimeFunction current_time_;
};


typedef boost::shared_ptr< NicAccumulator >  NicAccumulatorPtr;

#endif	

