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
#ifndef NIC_INTERFACE_FILTER_H_
#define NIC_INTERFACE_FILTER_H_
#include <string>
#include <vector>
#include <map>
#include <boost/regex_fwd.hpp>
#include <boost/shared_ptr.hpp>
#include "states/NicState.h"


typedef std::vector<std::string> NicInterfaceRegexes;

class NicInterfaceFilterRegex
{
public:
    NicInterfaceFilterRegex( const NicInterfaceRegexes& nic_interface_regexes );
    
    bool match( const std::string& nic );
    
private:
    bool is_regex_out_of_date() const;
    void compile_regex();
    
private:
    const NicInterfaceRegexes& nic_interface_regexes_;
    NicInterfaceRegexes cached_nic_interface_regexes_;
    boost::shared_ptr<boost::regex> regex_;
};


class NicInterfaceFilter
{
public:
    typedef std::map< std::string, NicStatePtr > NicStates;
    
public:
    NicInterfaceFilter( const NicInterfaceRegexes& nic_interface_regexes, 
                        const NicStates& nic_states,
                        NicStates& filtered_nic_states );
    
    void filter();
    
private:
    NicInterfaceFilterRegex filter_regex_;
    const NicStates& nic_states_;
    NicStates& filtered_nic_states_;
};

#endif
