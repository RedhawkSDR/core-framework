#include <iostream>
#include "ossie/ossieSupport.h"

#include "BasicService_cpp.h"

BasicService_cpp_i *servicePtr;

void signal_catcher(int sig)
{
    // IMPORTANT Don't call exit(...) in this function
    // issue all CORBA calls that you need for cleanup here before calling ORB shutdown
    if (servicePtr) {
        servicePtr->halt();
    }
}

int main(int argc, char* argv[])
{
    struct sigaction sa;
    sa.sa_handler = signal_catcher;
    sa.sa_flags = 0;
    servicePtr = 0;

    Service_impl::start_service(&servicePtr, sa, argc, argv);
    return 0;
}
