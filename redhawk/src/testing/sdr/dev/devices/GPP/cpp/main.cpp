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
  std::vector<std::string> gpp_argv(argv, argv+argc);
  gpp_argv.push_back("USESIGFD");

  std::vector<char*> new_gpp_argv(gpp_argv.size()+1, NULL);
  for (std::size_t i = 0; i < gpp_argv.size(); ++i) {
    new_gpp_argv[i] = const_cast<char*> (gpp_argv[i].c_str());
  }

  struct sigaction sa;
  sa.sa_handler = signal_catcher;
  sa.sa_flags = 0;
  devicePtr = 0;

  Device_impl::start_device(&devicePtr, sa,gpp_argv.size(), &new_gpp_argv[0]);

  return 0;
}

