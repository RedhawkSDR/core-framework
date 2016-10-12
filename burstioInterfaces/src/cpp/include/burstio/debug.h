/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK burstioInterfaces.
 *
 * REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef BURSTIO_DEBUG_H
#define BURSTIO_DEBUG_H

#include <ossie/debug.h>

namespace burstio {

#ifdef NO_RH_LOGGER

#if HAVE_LOG4CXX
    typedef log4cxx::LoggerPtr LoggerPtr;
#else
    typedef std::string LoggerPtr;
#endif

    inline LoggerPtr getChildLogger (LoggerPtr parent, const std::string& name)
    {
#if HAVE_LOG4CXX
        std::string log_name;
        parent->getName(log_name);
        log_name += "." + name;
        return log4cxx::Logger::getLogger(log_name);
#else
        return parent + "." + name;
#endif
    }

#else 
    typedef rh_logger::LoggerPtr LoggerPtr;

    inline LoggerPtr getChildLogger (LoggerPtr parent, const std::string& name)
    {
        std::string log_name;
        parent->getName(log_name);
        log_name += "." + name;
        return rh_logger::Logger::getLogger(log_name);
    }

#endif

}


#define ENABLE_INSTANCE_LOGGING                 \
    private:                                    \
        static LoggerPtr __classlogger;         \
        LoggerPtr __logger;

#endif // BURSTIO_DEBUG_H
