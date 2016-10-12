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

#include "ProcStatParser.h"
#include "ParserExceptions.h"


#ifdef DEBUG_ON
#define DEBUG(x)            x
#else
#define DEBUG(x)            
#endif


ProcStatParser::ProcStatParser() :
  fname("/proc/stat" )
{
  std::ifstream input( fname.c_str() );
  if ( !input.good()) throw std::ifstream::failure("unable to open " + fname );
}


ProcStatParser::ProcStatParser( const std::string &fname ) :
  fname(fname)
{
  std::ifstream input( fname.c_str() );
  if ( !input.good()) throw std::ifstream::failure("unable to open " + fname );
}



ProcStatParser::~ProcStatParser() 
{
}


void   ProcStatParser::parse( ProcStat::Contents & data )
{

  try {
    std::ifstream procstat_file( fname.c_str() );
    if ( !procstat_file.good()) throw std::ifstream::failure("unable to open " + fname );
    DEBUG(std::cout << " opened up the file..."<< std::endl);

    std::string line;
    data.time_stamp = time(NULL);
    while (  std::getline( procstat_file, line ) ) {
      std::vector<std::string> values;
      boost::split( values, line, boost::is_any_of(std::string(" ")), boost::algorithm::token_compress_on );
      DEBUG(std::cout << " line: " << line << std::endl);

      try {
        
        // handle different line types...
        
	if( boost::starts_with( values[0], "cpu" ) ) {
            ProcStat::CpuStat cstat;
            cstat.id = values[0];
            cstat.idx = -1;   // default to all
            try {
              if ( values[0].size() > 3 ) {
                std::string ids = values[0].substr(3);
                cstat.idx= boost::lexical_cast<int>( ids );
              }
            }
            catch( boost::bad_lexical_cast ){
            }
            // save off stat line for this cpu
            cstat.jiffies.resize( values.size()-1 );
            for( uint32_t i=1; i<values.size(); ++i )
            {
              try {
                cstat.jiffies[i-1] = boost::lexical_cast<ProcStat::Counter>( values[i] );
              }
              catch( boost::bad_lexical_cast ){
              }
            }

            std::stringstream ss;
            ss << boost::format("%-5s:%-2d ") % cstat.id % cstat.idx;
            for ( uint32_t j=0; j < cstat.jiffies.size(); j++ ) ss  << boost::format("%-7lld  ") % cstat.jiffies[j];
            DEBUG(std::cout << ss.str() << std::endl);

            if ( cstat.idx == -1 ) {
              data.all = cstat;
            }
            else {
              data.cpus.push_back(cstat);
            }


        
        }

	if( boost::starts_with( values[0], "intr" ) ) {
            ProcStat::CounterList stat_list;
            for( uint32_t i=1; i<values.size(); ++i )
            {
              try {
                stat_list.push_back( boost::lexical_cast<ProcStat::Counter>( values[i] ) );
              }
              catch( boost::bad_lexical_cast ){
              }
            }
            data.interrupts.swap( stat_list );
        }

	if( boost::starts_with( values[0], "softirq" ) ){
            ProcStat::CounterList stat_list;
            for( uint32_t i=1; i<values.size(); ++i )
            {
              try {
                stat_list.push_back( boost::lexical_cast<ProcStat::Counter>( values[i] ) );
              }
              catch( boost::bad_lexical_cast ){
              }
            }
            data.soft_irqs.swap( stat_list );
        }
          

        if( boost::starts_with( values[0], "btime" ) ) {        
          try {
            data.boot_time = boost::lexical_cast<ProcStat::Counter>( values[1] );
          }
          catch( const boost::bad_lexical_cast& )
            {
              throw ParserExceptions::ParseError( "Error parsing /proc/stat line (" + line + ")" );
            }
        }

	if( boost::starts_with( values[0], "ctxt" ) ) {        
          try {
            data.context_switches = boost::lexical_cast<ProcStat::Counter>( values[1] );
          }
          catch( const boost::bad_lexical_cast& )
            {
              throw ParserExceptions::ParseError( "Error parsing /proc/stat line (" + line + ")" );
            }
        }

	if( boost::starts_with( values[0], "processes" ) ){        
          try {
            data.processes_started = boost::lexical_cast<ProcStat::Counter>( values[1] );
          }
          catch( const boost::bad_lexical_cast& )
            {
              throw ParserExceptions::ParseError( "Error parsing /proc/stat line (" + line + ")" );
            }
        }


	if( boost::starts_with( values[0], "processes_running" ) ) {        
          try {
            data.processes_running = boost::lexical_cast<ProcStat::Counter>( values[1] );
          }
          catch( const boost::bad_lexical_cast& )
            {
              throw ParserExceptions::ParseError( "Error parsing /proc/stat line (" + line + ")" );
            }
        }

	if( boost::starts_with( values[0], "processes_blocked" ) ) {        
          try {
            data.processes_blocked = boost::lexical_cast<ProcStat::Counter>( values[1] );
          }
          catch( const boost::bad_lexical_cast& )
            {
              throw ParserExceptions::ParseError( "Error parsing /proc/stat line (" + line + ")" );
            }
        }


        }
      catch( const boost::bad_lexical_cast& )
        {
          throw ParserExceptions::ParseError( "Error parsing /proc/stat line (" + line + ")" );
        }
    }
    
  }
  catch(...){
    throw;
  }  


}


