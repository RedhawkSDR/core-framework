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
#include "NicInterfaceFilter.h"

#include <boost/regex.hpp>
#include <string>
#include <iostream>

NicInterfaceFilterRegex::NicInterfaceFilterRegex( const NicInterfaceRegexes& nic_interface_regexes ):
nic_interface_regexes_(nic_interface_regexes),
regex_(new boost::regex())
{
    compile_regex();
}

bool
NicInterfaceFilterRegex::match( const std::string& nic )
{
    if( is_regex_out_of_date() )
        compile_regex();
    
    return boost::regex_match( nic, *regex_ );
}

bool
NicInterfaceFilterRegex::is_regex_out_of_date() const
{
    return nic_interface_regexes_ != cached_nic_interface_regexes_;
}

void
NicInterfaceFilterRegex::compile_regex()
{
    std::string regex_str;
    
    NicInterfaceRegexes::const_iterator i = nic_interface_regexes_.begin();
    while( i!=nic_interface_regexes_.end() )
    {
        regex_str += *i;
        ++i;
        if( i!=nic_interface_regexes_.end() )
            regex_str += "|";
    }
    
    regex_->assign(regex_str);
    
    cached_nic_interface_regexes_ = nic_interface_regexes_;
}


NicInterfaceFilter::NicInterfaceFilter( const NicInterfaceRegexes& nic_interface_regexes, 
                                        const NicStates& nic_states,
                                        NicStates& filtered_nic_states ):
filter_regex_(nic_interface_regexes),
nic_states_(nic_states),
filtered_nic_states_(filtered_nic_states)
{
    filter();
}

void 
NicInterfaceFilter::filter()
{
    filtered_nic_states_.clear();
    for( NicStates::const_iterator i=nic_states_.begin(); i!=nic_states_.end(); ++i )
    {
        if( filter_regex_.match(i->first) )
            filtered_nic_states_.insert( std::make_pair(i->first, i->second) );
    }
}
