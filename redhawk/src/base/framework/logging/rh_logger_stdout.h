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
    LoggerPtr getInstanceLogger( const std::string &name );
    static  LoggerPtr  getLogger( const char *name );

    virtual ~StdOutLogger() {}
    
    StdOutLogger( const std::string &name );

    StdOutLogger( const char *name );

    void setLevel ( const LevelPtr &newLevel );

    void handleLogEvent( const LevelPtr &lvl, const std::string &msg ) ;

    void handleLogEvent( const LevelPtr &lvl, const std::string &msg, const spi::LocationInfo &location ) ;

    const LevelPtr&  getEffectiveLevel() const;

    std::vector<std::string> getNamedLoggers();

    bool isLoggerInHierarchy(const std::string& search_name);

    void* getUnderlyingLogger();

    virtual void configureLogger(const std::string &configuration, bool root_reset=false, int level=-1);

  protected:


  private:

    typedef boost::shared_ptr< StdOutLogger > StdOutLoggerPtr;

    static StdOutLoggerPtr   _rootLogger;

    std::ostream   &_os;

  };

};

#endif
