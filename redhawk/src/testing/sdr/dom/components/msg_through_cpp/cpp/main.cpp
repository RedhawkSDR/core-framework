#include <iostream>
#include "ossie/ossieSupport.h"

#include "msg_through_cpp.h"
int main(int argc, char* argv[])
{
    msg_through_cpp_i* msg_through_cpp_servant;
    Component::start_component(msg_through_cpp_servant, argc, argv);
    return 0;
}

