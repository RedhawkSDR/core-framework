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

#ifndef EXECUTABLE_DEVICE_IMPL_H
#define EXECUTABLE_DEVICE_IMPL_H

#include <sys/types.h>

#include "LoadableDevice_impl.h"
#include "CF/cf.h"
#include "ossie/Autocomplete.h"


class ExecutableDevice_impl: 
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
    public virtual POA_CF::ExecutableDevice,
#endif
    public LoadableDevice_impl
{

    ENABLE_LOGGING

public:

    ExecutableDevice_impl (char*, char*, char*, char*);
    ExecutableDevice_impl (char*, char*, char*, char*, CF::Properties capacities);
    ExecutableDevice_impl (char*, char*, char*, char*, char*);
    ExecutableDevice_impl (char*, char*, char*, char*, CF::Properties capacities, char*);
    ~ExecutableDevice_impl () {
    };
    CF::ExecutableDevice::ProcessID_Type execute (const char* name, const CF::Properties& options,
                                                  const CF::Properties& parameters);
    void configure (const CF::Properties& configProperties);

    ExecutableDevice_impl();
    ExecutableDevice_impl(ExecutableDevice_impl&);
    // Execute a Component with the associated soft packages as modifiers for its environment
    CF::ExecutableDevice::ProcessID_Type executeLinked (const char* name, const CF::Properties& options,
                                                  const CF::Properties& parameters, const CF::StringSequence& deps);
    // Perform the actual Component forking
    CF::ExecutableDevice::ProcessID_Type do_execute (const char* name, const CF::Properties& options,
                                                  const CF::Properties& parameters, const std::vector<std::string> prepend_args);

    // Terminate a process
    void terminate (CF::ExecutableDevice::ProcessID_Type processId);

    void setLogger(rh_logger::LoggerPtr logptr);

protected:
    // Parse the command-line arguments to retrieve the name of the Component that is to be launched
    static std::string get_component_name_from_exec_params(const CF::Properties& params);
    // Retrieve the name of the Component from its profile
    static std::string component_name_from_profile_name(const std::string& profile_name);

    // process affinity options
    virtual void   set_resource_affinity( const CF::Properties& options,
                                          const pid_t rsc_pid,
                                          const char *rsc_name,
                                          const std::vector<int> &bl = std::vector<int>(0) );
    rh_logger::LoggerPtr _executabledeviceLog;

private:
    CF::ExecutableDevice::ProcessID_Type PID;

    void _init();
};

#endif

