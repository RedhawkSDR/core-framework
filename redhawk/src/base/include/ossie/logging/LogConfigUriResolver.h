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
