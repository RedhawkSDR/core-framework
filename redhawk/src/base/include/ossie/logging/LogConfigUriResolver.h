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
#ifndef __RH_LOG_CONFIG_RESOLVER_H__
#define __RH_LOG_CONFIG_RESOLVER_H__
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>
#include <string>

namespace ossie {

  namespace logging {

class LogConfigUriResolver  {
 public:

  virtual ~LogConfigUriResolver() {};

  //
  // create a logging configuration base on single path element
  //  
  // Resource Path:
  //    DomainManager   dommgr:<domain>
  //    DeviceManger    devmgr:<domain>/<node name>
  //       Device       dev:<domain>/<node name>/<device>
  //       Service      svc:<domain>/<node name>/<ServiceName>
  //    Waveform        app:<domain>/<app_factory_name>/<app_instance_id>  
  //       component    rsc:<domain>/<app_instance_id>/<comp_instance_id>
  //       
  //
  //
  virtual std::string get_uri( const std::string &path ) = 0;


 protected:

 LogConfigUriResolver()
  {};


 private:
  LogConfigUriResolver( const LogConfigUriResolver &src) {};

};

 typedef boost::shared_ptr<LogConfigUriResolver>  LogConfigUriResolverPtr;
 typedef LogConfigUriResolverPtr   (*LogConfigFactory)();

  };

};



// the class factories

#define MAKE_FACTORY(CLAZZ) \
\
  class CLAZZ;  \
\
extern "C" { \
  ossie::logging::LogConfigUriResolverPtr logcfg_factory() {	\
    return ossie::logging::LogConfigUriResolverPtr( new CLAZZ() );	\
  }; \
\
  void destroy( ossie::logging::LogConfigUriResolverPtr &p) {	\
  p.reset(); \
}; \
};

#endif
