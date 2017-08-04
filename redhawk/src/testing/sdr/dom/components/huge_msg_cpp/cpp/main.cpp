#include <iostream>
#include "ossie/ossieSupport.h"

#include "huge_msg_cpp.h"
int main(int argc, char* argv[])
{
    huge_msg_cpp_i* huge_msg_cpp_servant;
    Component::start_component(huge_msg_cpp_servant, argc, argv);
    return 0;
}

