#include <iostream>
#include "ossie/ossieSupport.h"

#include "base_programmable.h"

base_programmable_i *devicePtr;

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
    struct sigaction sa;
    sa.sa_handler = signal_catcher;
    sa.sa_flags = 0;
    devicePtr = 0;

    Device_impl::start_device(&devicePtr, sa, argc, argv);
    return 0;
}

