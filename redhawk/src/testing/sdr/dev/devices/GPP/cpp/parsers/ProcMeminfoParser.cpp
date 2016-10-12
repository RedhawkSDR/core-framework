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
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "ProcMeminfoParser.h"
#include "ParserExceptions.h"


#ifdef DEBUG_ON
#define DEBUG(x)            x
#else
#define DEBUG(x)            
#endif


ProcMeminfoParser::ProcMeminfoParser() :
  fname("/proc/meminfo" )
{
  std::ifstream input( fname.c_str() );
  if ( !input.good()) throw std::ifstream::failure("unable to open " + fname );
}


ProcMeminfoParser::ProcMeminfoParser( const std::string &fname ) :
  fname(fname)
{
  std::ifstream input( fname.c_str() );
  if ( !input.good()) throw std::ifstream::failure("unable to open " + fname );
}



ProcMeminfoParser::~ProcMeminfoParser() 
{
}


void   ProcMeminfoParser::parse( ProcMeminfo::Contents & data )
{

  try {
    std::ifstream procmeminfo_file( fname.c_str() );
    if ( !procmeminfo_file.good()) throw std::ifstream::failure("unable to open " + fname );
    DEBUG(std::cout << " opened up the file..."<< std::endl);

    std::string line;
    while (  std::getline( procmeminfo_file, line ) ) {
      std::vector<std::string> values;
      boost::split( values, line, boost::is_any_of(std::string(" ")), boost::algorithm::token_compress_on );
      DEBUG(std::cout << " line(tokens) " << values.size() << ":" << line << std::endl);

      // key:  value [unit]
      std::string key;
      ProcMeminfo::Counter     metric = 0;
      key = values[0].substr(0, values[0].find(":"));
        
      if ( values.size() >= 2 ) {
        try {
          metric = boost::lexical_cast<ProcMeminfo::Counter>( values[1] );
        }
        catch( boost::bad_lexical_cast ){
        }
      }

      // handle units
      ProcMeminfo::Counter  unit_m=1;
      if ( values.size() >= 3 ) {
          std::string units(values[2]);
          boost::to_upper(units);
          if ( units == "KB" ) unit_m = 1e3;
          if ( units == "MB" ) unit_m = 1e6;
          if ( units == "GB" ) unit_m = 1e9;
          if ( units == "TB" ) unit_m = 1e12;
      }
        
      metric = metric * unit_m;
        
      data[key]=metric;
    }

  }
  catch(...){
    throw;
  }  


}


