/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef  RH_LOGGER_CFG_H
#define  RH_LOGGER_CFG_H

#include <values.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <boost/shared_ptr.hpp>
#include <boost/circular_buffer.hpp>
#include <stdint.h>


//
// This file implements the log4cxx configure methods used by the ossie::logging namespace. 
// This file is only include when HAVE_LOG4CXX is NOT defined. 
//

namespace rh_logger {

  namespace helpers {

    class InputStream {

    public:
      
      InputStream() {}

      virtual ~InputStream() {};
    };
    
    class StringInputStream : public InputStream {

    public:
      StringInputStream ( const std::string &cfg_data ) : _cfg_data(cfg_data) {};

      virtual ~StringInputStream() {};

    private:
      std::string _cfg_data;

    };
    
    typedef  boost::shared_ptr< InputStream >  InputStreamPtr;

    class Properties {

    public:

      Properties() {};
      
      void load( InputStreamPtr istr ) { 
	_istream.swap(istr);
      };

    private:
      
      InputStreamPtr   _istream;

    };

  };  // end helpers namespace


  class PropertyConfigurator 
  {

  public:

    static unsigned int ossieDebugLevel;

    static void configure() {
      configure( 3 );
    }

    static void configure(int debugLevel) {
      ossieDebugLevel = debugLevel;
    }

    static void configure(const char* filename) {
      configure( filename , "");
    }

    static void configure(const char* filename, const char* nameoverride) {
      // read in the file (which is in log4cxx format) and
      // parse out the line that contains log4j.rootLogger= level, [loggers]
      // ignore loggers and only read the level
      std::ifstream in(filename);
      if (! in) {
	std::perror(filename);
	return;
      }

      // A very rudimentary parser that isn't very forgiving
      char linebuffer[1024];
      std::string line;
      while (! in.eof() ) {
	in.getline(linebuffer, 1024);
	line.assign(linebuffer);
	if (line.find("log4j.rootLogger=") == 0) {
	  std::string value = line.substr(strlen("log4j.rootLogger="), line.size());
	  std::string::size_type first_non_ws = value.find_first_not_of(" ");
	  // we can use line.size() because substr uses the smaller of n and size() - pos
	  if (value.find("FATAL", first_non_ws) == 0) {
	    ossieDebugLevel = 0;
	  } else if (value.find("ERROR", first_non_ws) == 0) {
	    ossieDebugLevel = 1;
	  } else if (value.find("WARN", first_non_ws) == 0) {
	    ossieDebugLevel = 2;
	  } else if (value.find("INFO", first_non_ws) == 0) {
	    ossieDebugLevel = 3;
	  } else if (value.find("DEBUG", first_non_ws) == 0) {
	    ossieDebugLevel = 4;
	  } else if (value.find("ALL", first_non_ws) == 0) {
	    ossieDebugLevel = 5;
	  }
	  break;
	}
      }
      in.close();
    }

    static void configure( helpers::Properties &props ) {
      // RESOLVE 
    }

  };  // end of PropertyConfigurator


  namespace  xml {

    class DOMConfigurator 
    {
      
    public:

      static void configure( helpers::Properties &props ) {
	// RESOLVE 
      }

      static void configure( std::string &fname ) {
	// RESOLVE 
      }

    };  // end of DOMConfigurator

  }; // end xml namespace

  

  class  LogManager {
  public:
       static void         shutdown() {};
  };

  // 
  // Needs to be removed if we provide a more functional 
  //
  unsigned int PropertyConfigurator::ossieDebugLevel = 0;

};  // end of rh_logger namespace







#endif
