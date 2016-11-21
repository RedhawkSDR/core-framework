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
#include <ossie/logging/LogConfigUriResolver.h>
#include <iostream>
#include <sstream>


class EmptyLogConfigUri : public ossie::logging::LogConfigUriResolver {

public:

  EmptyLogConfigUri() {};

  virtual ~EmptyLogConfigUri(){};

  std::string get_uri( const std::string &path ) { 

#ifdef DEBUG_TEST
    std::string sdrroot("");
    if ( ::getenv("SDRROOT")){
      sdrroot = ::getenv("SDRROOT");
    }
    if ( path.find("dev:") != std::string::npos ) {
      std::ostringstream os;
      os << "file://" << sdrroot << "/dom/logcfg/device.log.cfg";      
      return std::string(os.str());
    }
    if ( path.find("svc:") != std::string::npos ) {
      std::ostringstream os;
      os << "file://" << sdrroot << "/dom/logcfg/serviceq.log.cfg";      
      return std::string(os.str());
    }
    if ( path.find("rsc:") != std::string::npos ) {
      std::ostringstream os;
      os << "file://" << sdrroot << "/dom/logcfg/comp.log.cfg";      
      return std::string(os.str());
    }
#endif
    return std::string(""); 
  };

};


MAKE_FACTORY(EmptyLogConfigUri);
