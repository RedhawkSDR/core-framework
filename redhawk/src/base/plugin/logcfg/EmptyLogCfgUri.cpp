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
