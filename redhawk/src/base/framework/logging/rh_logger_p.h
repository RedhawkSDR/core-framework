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
#ifndef  RH_LOGGER_P_H
#define  RH_LOGGER_P_H

#include <values.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <ossie/logging/rh_logger.h>
#include "rh_logger_stdout.h"

namespace rh_logger {

#ifdef HAVE_LOG4CXX

  log4cxx::LevelPtr ConvertRHLevelToLog4 ( rh_logger::LevelPtr rh_level );
  rh_logger::LevelPtr ConvertLog4ToRHLevel ( log4cxx::LevelPtr l4_level );

  class L4Logger : public Logger {

  public:

    static  LoggerPtr  getRootLogger( );
    static  LoggerPtr  getLogger( const std::string &name );
    static  LoggerPtr  getLogger( const char *name );

    virtual ~L4Logger() {}
    
    L4Logger( const std::string &name );

    L4Logger( const char *name );

    bool isFatalEnabled() const;
    bool isErrorEnabled() const;
    bool isWarnEnabled() const;
    bool isInfoEnabled() const;
    bool isDebugEnabled() const;
    bool isTraceEnabled() const;

    void setLevel ( const LevelPtr &newLevel );

    LevelPtr getLevel () const;

    void handleLogEvent( const LevelPtr &lvl, const std::string &msg ) ;

    void handleLogEvent( const LevelPtr &lvl, const std::string &msg, const spi::LocationInfo &location ) ;

    const LevelPtr&  getEffectiveLevel() const;

  private:

    typedef boost::shared_ptr< L4Logger > L4LoggerPtr;

    static L4LoggerPtr   _rootLogger;

    log4cxx::LoggerPtr  l4logger;
  };

#endif

};   // end of rh_logger namespace

#endif
