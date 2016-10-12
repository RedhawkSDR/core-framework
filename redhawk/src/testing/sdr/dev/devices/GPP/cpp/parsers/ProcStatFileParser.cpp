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
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "ProcStatFileParser.h"
#include "ParserExceptions.h"
#include "utils/FileReader.h"
void
ProcStatFileParser::Parse( ProcStatFileData& data )
{
  std::stringstream file_str( FileReader::ReadFile( "/proc/stat" ) );
  GetImpl()->parse( file_str, data );
}

void 
ProcStatFileParser::parse( std::istream& istr, ProcStatFileData& data )
{
	reset_fields(data);
	parse_fields(istr,data);
	validate_fields(data);
}

void
ProcStatFileParser::reset_fields(ProcStatFileData& data)
{
    data = ProcStatFileData();
}

void
ProcStatFileParser::parse_fields(std::istream& istr, ProcStatFileData& data)
{
	std::string line;
	do
	{
		std::getline( istr, line );
		std::vector<std::string> values;
		boost::split( values, line, boost::is_any_of(std::string(" ")), boost::algorithm::token_compress_on );

		try
		{
			parse_line( values, data );
		}
		catch( const boost::bad_lexical_cast& )
		{
			throw ParserExceptions::ParseError( "Error parsing /proc/stat line (" + line + ")" );
		}
	}while( !istr.eof() );
}


void
ProcStatFileParser::parse_line(const std::vector<std::string>& values, ProcStatFileData& data)
{
	if( values[0] == "cpu" )
	{
		for( size_t i=1; i<values.size() && i<=data.cpu_jiffies.size(); ++i )
		{
			data.cpu_jiffies[i-1] = boost::lexical_cast<size_t>( values[i] );
		}
	}
	else if( values[0] == "btime" )
	{
        data.os_start_time = boost::lexical_cast<size_t>( values[1] );
	}
}

void
ProcStatFileParser::validate_fields(const ProcStatFileData& data) const
{
	validate_field( 0 != data.os_start_time, "empty btime field" );
}

void
ProcStatFileParser::validate_field( bool success, const std::string& message ) const
{
	if( !success )
	{
		throw ParserExceptions::ParseError( "Error validating /proc/stat, " + message );
	}
}

