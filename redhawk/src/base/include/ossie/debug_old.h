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


#ifndef OLD_DEBUG_H
#define OLD_DEBUG_H


/*
 * README
 *
 * The goal of this file is to ensure a consistent logging approach
 * throughout OSSIE.  The use of the LOG_ statements is preferred over
 * the old DEBUG() macro.  For tracing entry and exiting of functions,
 * use TRACE_ENTRY and TRACE_EXIT.
 */
#if HAVE_LOG4CXX
#include<log4cxx/logger.h>
#include<log4cxx/propertyconfigurator.h>
#include <log4cxx/logstring.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/logmanager.h>
#include <fstream>

/*
 * Place the ENABLE_LOGGING macro in a class to enable the log statements for the class.
 */
#define ENABLE_LOGGING \
    private: \
      static log4cxx::LoggerPtr __logger;

/*
 * Place PREPARE_LOGGING at the top of a class .cpp file to setup the logger for
 * classname.
 */
#define PREPARE_LOGGING(classname) \
    log4cxx::LoggerPtr classname::__logger(log4cxx::Logger::getLogger(#classname));

/*
 * Use PREPARE_ALT_LOGGING at the top of a class .cpp file to have classname 
 * use logger loggername.
 * This is useful allowing port_impl logging messages to use the main component
 * logger 
 */
#define PREPARE_ALT_LOGGING(classname, loggername) \
    log4cxx::LoggerPtr classname::__logger(log4cxx::Logger::getLogger(#loggername));

#define CREATE_LOGGER(classname) \
    class classname \
    { \
        public: \
          static log4cxx::LoggerPtr __logger; \
    }; \
    log4cxx::LoggerPtr classname::__logger(log4cxx::Logger::getLogger(#classname));

class LoggingConfigurator
{
public:
    static void configure() {
        configure(3);
    }

    static void configure(int debugLevel) {
        // Don't use BasicConfigurator::configure() here because it produces ugly logs (IMHO)
        // that regular users won't like...power users can always set more advanced patterns
        // by using the -log4cxx option
        log4cxx::LogManager::getLoggerRepository()->setConfigured(true);
        log4cxx::LoggerPtr root = log4cxx::Logger::getRootLogger();
#ifdef ENABLE_TRACE
        // Put file/line numbers in each log output
        log4cxx::LayoutPtr layout(new log4cxx::PatternLayout("%p:%c - %m [%F:%L]%n"));
#else
        log4cxx::LayoutPtr layout(new log4cxx::PatternLayout("%p:%c - %m%n"));
#endif
        log4cxx::AppenderPtr appender(new log4cxx::ConsoleAppender(layout));
        root->addAppender(appender);

        if (debugLevel == 0) {
            root->setLevel(log4cxx::Level::getFatal());
        } else if (debugLevel == 1) {
            root->setLevel(log4cxx::Level::getError());
        } else if (debugLevel == 2) {
            root->setLevel(log4cxx::Level::getWarn());
        } else if (debugLevel == 3) {
            root->setLevel(log4cxx::Level::getInfo());
        } else if (debugLevel == 4) {
            root->setLevel(log4cxx::Level::getDebug());
        } else if (debugLevel >= 5) {
            root->setLevel(log4cxx::Level::getAll());
        }
    }

    static void configure(const char* filename) {
        log4cxx::PropertyConfigurator::configure(filename);
    }

    static void configure(const char* filename, const char* nameoverride) {
        // this function overrides the pattern __NAME__ with nameoverride in the input config file
        std::ifstream in_configfile(filename);
        std::string in_str, out_str;
        std::string name_pat = "__NAME__";
        std::string::size_type idx = 0;

        // do the override
        while( getline(in_configfile, in_str) ) {
            idx = in_str.find(name_pat);
            while (idx != std::string::npos) {
                in_str = in_str.substr(0, idx) + nameoverride + "." + in_str.substr(idx + name_pat.length(), in_str.length());
                idx = in_str.find(name_pat);
            }
            out_str += in_str + "\n";
        }
        in_configfile.close();

        // overwrite the old file
        std::fstream out_configfile(filename, std::ios::out | std::ios::trunc);
        out_configfile.write(out_str.c_str(), out_str.length());
        out_configfile.close();

        // now configure
        log4cxx::PropertyConfigurator::configure(filename);
    }

    static int getLevel() {
        log4cxx::LoggerPtr root = log4cxx::Logger::getRootLogger();
        log4cxx::LevelPtr level = root->getLevel();
        if (log4cxx::Level::getFatal()->equals(level)) {
            return 0;
        } else if (log4cxx::Level::getError()->equals(level)) {
            return 1;
        } else if (log4cxx::Level::getWarn()->equals(level)) {
            return 2;
        } else if (log4cxx::Level::getInfo()->equals(level)) {
            return 3;
        } else if (log4cxx::Level::getDebug()->equals(level)) {
            return 4;
        } else if (log4cxx::Level::getAll()->equals(level)) {
            return 5;
        }
        return 3;
    }
};

#define LOG_TRACE(classname, expression) LOG4CXX_TRACE(classname::__logger, expression)
#define LOG_DEBUG(classname, expression) LOG4CXX_DEBUG(classname::__logger, expression)
#define LOG_INFO(classname, expression)  LOG4CXX_INFO(classname::__logger, expression)
#define LOG_WARN(classname, expression)  LOG4CXX_WARN(classname::__logger, expression)
#define LOG_ERROR(classname, expression) LOG4CXX_ERROR(classname::__logger, expression)
#define LOG_FATAL(classname, expression) LOG4CXX_FATAL(classname::__logger, expression)

#else
#include<iostream>
#include<fstream>
#include<string>
#include<cstring>

/*
 * Although technically you could implement this
 * with blank #defines, doing this ensures that
 * any future use of logging will remain
 * compatible with LOG4CXX.  If we didn't
 * do this, logging statements could be
 * created that won't work with LOG4CXX down
 * the road.
 */
#define ENABLE_LOGGING \
    private: \
      static std::string __logger;

#define PREPARE_LOGGING(classname) \
    std::string classname::__logger(#classname);

/*
 * Use PREPARE_ALT_LOGGING at the top of a class .cpp file to have classname 
 * use logger loggername.
 * This is useful allowing port_impl logging messages to use the main component
 * logger 
 */
#define PREPARE_ALT_LOGGING(classname, loggername) \
    log4cxx::LoggerPtr classname::__logger(log4cxx::Logger::getLogger(#loggername));

#define CREATE_LOGGER(classname) \
    class classname \
    { \
        public: \
          static std::string __logger; \
    }; \
   std::string classname::__logger(#classname);

class LoggingConfigurator
{
public:
    static unsigned int ossieDebugLevel;

    static void configure() {
        configure(3);
    }

    static void configure(int debugLevel) {
        ossieDebugLevel = debugLevel;
    }

    static void configure(const char* filename) {
        configure(filename, "");
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
};

#ifdef ENABLE_TRACE
#define _LOG(level, levelname, logger, msg) \
      if (LoggingConfigurator::ossieDebugLevel >= level) \
        std::cout << #levelname << ":" << logger << " - " << msg << " [" << __FILE__ << ":" << __LINE__ << "]"<< std::endl;
#else
#define _LOG(level, levelname, logger, msg) \
      if (LoggingConfigurator::ossieDebugLevel >= level) \
        std::cout << #levelname << ":" << logger << " - " << msg << std::endl;
#endif


#define LOG_TRACE(classname, expression)  _LOG(5, TRACE, classname::__logger, expression)
#define LOG_DEBUG(classname, expression)  _LOG(4, DEBUG, classname::__logger, expression)
#define LOG_INFO(classname, expression)   _LOG(3, INFO,  classname::__logger, expression)
#define LOG_WARN(classname, expression)   _LOG(2, WARN,  classname::__logger, expression)
#define LOG_ERROR(classname, expression)  _LOG(1, ERROR, classname::__logger, expression)
#define LOG_FATAL(classname, expression)  _LOG(0, FATAL, classname::__logger, expression)

#endif

/*
* Provide a standardized mechanism for  TRACING into and out of functions so
* they all look similar.  Unlike LOG_TRACE, these are only included
* in the code if DEBUG is defined
*/
#ifdef ENABLE_TRACE
#define TRACE_ENTER(classname) \
    LOG_TRACE(classname, "Entering " << #classname << "." << __PRETTY_FUNCTION__ << " [" << __FILE__ << ":" << __LINE__ << "]")

#define TRACE_EXIT(classname) \
    LOG_TRACE(classname, "Exiting " << #classname << "." << __PRETTY_FUNCTION__ << " [" << __FILE__ << ":" << __LINE__ << "]")

#else
#define TRACE_ENTER(classname)
#define TRACE_EXIT(classname)

#endif

/*
* Provide standardized exception handling for common calls.
*/
#define CATCH_LOG_EXCEPTION(classname, expression, levelname) \
    catch( std::exception& ex ) { \
        LOG_##levelname(classname, expression << "; std::exception info: " << ex.what()) \
    } \
    catch( CORBA::Exception& ex ) { \
        LOG_##levelname(classname, expression << "; CORBA::Exception name: " << ex._name()) \
    } \
    catch( ... ) { \
        LOG_##levelname(classname, expression << "; unknown exception") \
    }

#define CATCH_LOG_TRACE(classname, expression) CATCH_LOG_EXCEPTION(classname, expression, TRACE)
#define CATCH_LOG_DEBUG(classname, expression) CATCH_LOG_EXCEPTION(classname, expression, DEBUG)
#define CATCH_LOG_INFO(classname, expression) CATCH_LOG_EXCEPTION(classname, expression, INFO)
#define CATCH_LOG_WARN(classname, expression) CATCH_LOG_EXCEPTION(classname, expression, WARN)
#define CATCH_LOG_ERROR(classname, expression) CATCH_LOG_EXCEPTION(classname, expression, ERROR)
#define CATCH_LOG_FATAL(classname, expression) CATCH_LOG_EXCEPTION(classname, expression, FATAL)

/*
* Provide standardized exception handling for catching and throwing a new exception.
*/
#define CATCH_THROW_LOG_EXCEPTION(classname, expression, levelname, newexception) \
    catch( std::exception& ex ) { \
        LOG_##levelname(classname, expression << "; std::exception info: " << ex.what()) \
        throw(newexception); \
    } \
    catch( CORBA::Exception& ex ) { \
        LOG_##levelname(classname, expression << "; CORBA::Exception name: " << ex._name()) \
        throw(newexception); \
    } \
    catch( ... ) { \
        LOG_##levelname(classname, expression << "; unknown exception") \
        throw(newexception); \
    }

#define CATCH_THROW_LOG_TRACE(classname, expression, newexception) CATCH_THROW_LOG_EXCEPTION(classname, expression, TRACE, newexception)
#define CATCH_THROW_LOG_DEBUG(classname, expression, newexception) CATCH_THROW_LOG_EXCEPTION(classname, expression, DEBUG, newexception)
#define CATCH_THROW_LOG_INFO(classname, expression, newexception) CATCH_THROW_LOG_EXCEPTION(classname, expression, INFO, newexception)
#define CATCH_THROW_LOG_WARN(classname, expression, newexception) CATCH_THROW_LOG_EXCEPTION(classname, expression, WARN, newexception)
#define CATCH_THROW_LOG_ERROR(classname, expression, newexception) CATCH_THROW_LOG_EXCEPTION(classname, expression, ERROR, newexception)
#define CATCH_THROW_LOG_FATAL(classname, expression, newexception) CATCH_THROW_LOG_EXCEPTION(classname, expression, FATAL, newexception)

/*
* Provide standardized exception handling for catching and rethrowing.
*/
#define CATCH_RETHROW_LOG_EXCEPTION(classname, expression, levelname) \
    catch( std::exception& ex ) { \
        LOG_##levelname(classname, expression << "; std::exception info: " << ex.what()) \
        throw; \
    } \
    catch( CORBA::Exception& ex ) { \
        LOG_##levelname(classname, expression << "; CORBA::Exception name: " << ex._name()) \
        throw; \
    } \
    catch( ... ) { \
        LOG_##levelname(classname, expression << "; unknown exception") \
        throw; \
    }

#define CATCH_RETHROW_LOG_TRACE(classname, expression) CATCH_RETHROW_LOG_EXCEPTION(classname, expression, TRACE)
#define CATCH_RETHROW_LOG_DEBUG(classname, expression) CATCH_RETHROW_LOG_EXCEPTION(classname, expression, DEBUG)
#define CATCH_RETHROW_LOG_INFO(classname, expression) CATCH_RETHROW_LOG_EXCEPTION(classname, expression, INFO)
#define CATCH_RETHROW_LOG_WARN(classname, expression) CATCH_RETHROW_LOG_EXCEPTION(classname, expression, WARN)
#define CATCH_RETHROW_LOG_ERROR(classname, expression) CATCH_RETHROW_LOG_EXCEPTION(classname, expression, ERROR)
#define CATCH_RETHROW_LOG_FATAL(classname, expression) CATCH_RETHROW_LOG_EXCEPTION(classname, expression, FATAL)


/*
 * Provide a backwards compatible macro.
 * THIS MACRO SHOULD BE AVOIDED FOR ALL NEW DEVELOPMENT
 */
#define DEBUG(level, classname, expression) LOG_DEBUG(classname, expression)



#ifdef LOCAL_DEBUG_ON
#define STDOUT_DEBUG(x)    std::cout << x << std::endl
#else
#define STDOUT_DEBUG(x)    
#endif

#endif
