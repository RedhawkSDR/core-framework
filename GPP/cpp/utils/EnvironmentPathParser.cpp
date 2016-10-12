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
#include "EnvironmentPathParser.h"

#include <boost/algorithm/string.hpp>

/**
 * STL algorithm predicate for detecting empty string.
 */
struct IsEmptyString
{
    bool operator()( const std::string& str ) const
    {
        return str.empty();
    }
};

EnvironmentPathParser::EnvironmentPathParser( const std::string& path )
{
    from_string(path);
}

EnvironmentPathParser::EnvironmentPathParser( const char* path )
{
    if( path )
    {
        from_string(path);
    }
}

void
EnvironmentPathParser::merge_front( const std::string& path )
{
    if( std::find(paths.begin(), paths.end(), path) == paths.end() )
    {
        paths.insert( paths.begin(), path );
    }
}

void
EnvironmentPathParser::from_string( const std::string& path )
{
    boost::split( paths, path, boost::is_any_of(std::string(":")), boost::algorithm::token_compress_on );
    strip_empty_paths();
}

void 
EnvironmentPathParser::strip_empty_paths()
{
    paths.erase( std::remove_if(paths.begin(), paths.end(), IsEmptyString()), paths.end() );
}

std::string
EnvironmentPathParser::to_string() const
{
    return boost::join( paths, ":" );
}
