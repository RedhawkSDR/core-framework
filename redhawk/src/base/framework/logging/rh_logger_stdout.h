#ifndef  RH_LOGGER_P_STDOUT_H
#define  RH_LOGGER_P_STDOUT_H

#include <values.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ossie/logging/rh_logger.h>

namespace rh_logger {

  class StdOutLogger : public Logger {

  public:

    static  LoggerPtr  getRootLogger( );
    static  LoggerPtr  getLogger( const std::string &name );
    static  LoggerPtr  getLogger( const char *name );

    virtual ~StdOutLogger() {}
    
    StdOutLogger( const std::string &name );

    StdOutLogger( const char *name );

    void setLevel ( const LevelPtr &newLevel );

    void handleLogEvent( const LevelPtr &lvl, const std::string &msg ) ;

    void handleLogEvent( const LevelPtr &lvl, const std::string &msg, const spi::LocationInfo &location ) ;

    const LevelPtr&  getEffectiveLevel() const;

  protected:


  private:

    typedef boost::shared_ptr< StdOutLogger > StdOutLoggerPtr;

    static StdOutLoggerPtr   _rootLogger;

    std::ostream   &_os;

  };

};

#endif
