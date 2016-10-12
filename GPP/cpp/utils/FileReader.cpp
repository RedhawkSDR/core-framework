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
#include "FileReader.h"
#include "IOError.h"

#include <sstream>
#include <fstream>

std::string 
FileReader::ReadFile( const std::string& filename )
{
    return GetImpl()->read_file(filename);
}

std::string
FileReader::read_file( const std::string& filename )
{
    std::ifstream fstr(filename.c_str());
    
    if( !fstr.good() )
    {
        std::stringstream errstr;
        errstr << "Error opening file for reading (" << filename << ")";
        throw IOError( errstr.str() );
    }
  
    try
    {
        return std::string((std::istreambuf_iterator<char>(fstr)), std::istreambuf_iterator<char>());
    }
    catch( const std::exception& e )
    {
        std::stringstream errstr;
        errstr << "Error reading file (" << filename << " msg=\"" << e.what() << "\")";
        throw IOError( errstr.str() );
    }
}

