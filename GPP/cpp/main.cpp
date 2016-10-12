/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK GPP.
 *
 * REDHAWK GPP is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK GPP is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include <iostream>
#include <string>
#include <vector>
#include "ossie/ossieSupport.h"

#include "GPP.h"

GPP_i *devicePtr;

void signal_catcher(int sig)
{
    // IMPORTANT Don't call exit(...) in this function
    // issue all CORBA calls that you need for cleanup here before calling ORB shutdown
    if (devicePtr) {
        devicePtr->halt();
    }
}
int main(int argc, char* argv[])
{
  //
  // Install signal handler for processing SIGCHLD through
  // signal file descriptor to avoid whitelist/blacklist function calls
  //
  // add command line arg, to setup signalfd in start_device
  std::vector<std::string> gpp_argv(argv, argv+argc);
  gpp_argv.push_back("USESIGFD");

  std::vector<char*> new_gpp_argv(gpp_argv.size()+1, NULL);
  for (std::size_t i = 0; i < gpp_argv.size(); ++i) {
    new_gpp_argv[i] = const_cast<char*> (gpp_argv[i].c_str());
  }

  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = signal_catcher;
  sa.sa_flags = 0;
  devicePtr = 0;

  Device_impl::start_device(&devicePtr, sa,gpp_argv.size(), &new_gpp_argv[0]);

  return 0;
}

