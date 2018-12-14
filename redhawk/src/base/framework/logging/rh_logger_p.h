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

#ifdef HAVE_LOG4CXX
#include <log4cxx/logger.h>
#include <log4cxx/hierarchy.h>
#include <log4cxx/level.h>
#include <log4cxx/logstring.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/helpers/messagebuffer.h>
#endif

#include <ossie/logging/rh_logger.h>
#include "rh_logger_stdout.h"

namespace rh_logger {

#ifdef HAVE_LOG4CXX

  log4cxx::LevelPtr ConvertRHLevelToLog4 ( rh_logger::LevelPtr rh_level );
  rh_logger::LevelPtr ConvertLog4ToRHLevel ( log4cxx::LevelPtr l4_level );

  class L4Hierarchy : public log4cxx::Hierarchy {
  public:
      L4Hierarchy(const std::string &name) : log4cxx::Hierarchy() {
          _name = name;
      };
      void resetConfiguration() {
          log4cxx::Hierarchy::resetConfiguration();
      };
      std::string _name;
  };

  class L4Logger : public Logger {
  private:
    //typedef boost::shared_ptr< L4Hierarchy > L4HierarchyPtr;
    //typedef boost::shared_ptr< log4cxx::Hierarchy > L4HierarchyPtr;
    typedef log4cxx::helpers::ObjectPtrT<L4Hierarchy> L4HierarchyPtr;

  public:

    static  LoggerPtr  getRootLogger( );
    static  LoggerPtr  getLogger( const std::string &name );
    LoggerPtr getInstanceLogger( const std::string &name );
    static  LoggerPtr  getLogger( const char *name );
    static  LoggerPtr  getLogger( const std::string &name, bool newroot );

    virtual ~L4Logger() {}
    
    L4Logger( const std::string &name, L4HierarchyPtr hierarchy );

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

    std::vector<std::string> getNamedLoggers();

    bool isLoggerInHierarchy(const std::string& search_name);

    void* getUnderlyingLogger();

    L4HierarchyPtr getRootHierarchy() {
        return _rootHierarchy;
    };

    void setHierarchy(L4HierarchyPtr hierarchy) {
        _rootHierarchy = hierarchy;
    };

    void configureLogger(const std::string &configuration, bool root_reset=false, int level=-1);

  private:

    log4cxx::LoggerPtr  l4logger;

    typedef boost::shared_ptr< L4Logger > L4LoggerPtr;

    static L4LoggerPtr _rootLogger;

    log4cxx::LoggerPtr _instanceRootLogger;
    L4HierarchyPtr _rootHierarchy;
    log4cxx::PropertyConfigurator prop_conf;

    uint32_t               _error_count;
  };

#endif

};   // end of rh_logger namespace

#endif
