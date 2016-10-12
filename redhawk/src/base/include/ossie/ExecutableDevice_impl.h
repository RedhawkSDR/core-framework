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


class ExecutableDevice_impl: public virtual
    POA_CF::ExecutableDevice,
    public
    LoadableDevice_impl
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
                                                  const CF::Properties& parameters) throw (CF::ExecutableDevice::ExecuteFail,
                                                          CF::InvalidFileName, CF::ExecutableDevice::InvalidOptions,
                                                          CF::ExecutableDevice::InvalidParameters,
                                                          CF::ExecutableDevice::InvalidFunction, CF::Device::InvalidState,
                                                          CORBA::SystemException);

    void terminate (CF::ExecutableDevice::ProcessID_Type processId) throw
    (CF::Device::InvalidState, CF::ExecutableDevice::InvalidProcess,
     CORBA::SystemException);

    void configure (const CF::Properties& configProperties)
    throw (CF::PropertySet::PartialConfiguration,
           CF::PropertySet::InvalidConfiguration, CORBA::SystemException);

    ExecutableDevice_impl();
    ExecutableDevice_impl(ExecutableDevice_impl&);
private:


    CF::ExecutableDevice::ProcessID_Type PID;
};

#endif

