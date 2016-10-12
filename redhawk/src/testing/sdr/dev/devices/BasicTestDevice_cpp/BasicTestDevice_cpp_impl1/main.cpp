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


#include "BasicTestDevice_cpp_impl1.h"

BasicTestDevice_cpp_impl1_i *devicePtr;

CREATE_LOGGER(TestBasicTestDevice_cpp_impl1Device)

void signal_catcher(int sig)
{
    // IMPORTANT Don't call exit(...) in this function
    // issue all CORBA calls that you need for cleanup here before calling ORB shutdown
    LOG_DEBUG(TestBasicTestDevice_cpp_impl1Device, "Terminate signal " << sig << " received")
    if (devicePtr) {
        devicePtr->halt();
    } else {
        LOG_DEBUG(TestBasicTestDevice_cpp_impl1Device, "Device not instantiated")
    }
    LOG_DEBUG(TestBasicTestDevice_cpp_impl1Device, "Device halted")
}

int main(int argc, char *argv[])
{
    struct sigaction sa;
    sa.sa_handler = signal_catcher;
    sa.sa_flags = 0;

    Device_impl::start_device(&devicePtr, sa, argc, argv);
}
