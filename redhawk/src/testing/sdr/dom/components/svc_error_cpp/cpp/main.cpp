#include <iostream>
#include "ossie/ossieSupport.h"

#include "svc_error_cpp.h"
int main(int argc, char* argv[])
{
    svc_error_cpp_i* svc_error_cpp_servant;
    Component::start_component(svc_error_cpp_servant, argc, argv);
    return 0;
}

