#include <iostream>
#include "ossie/ossieSupport.h"

#include "slow_stop_cpp.h"
int main(int argc, char* argv[])
{
    slow_stop_cpp_i* slow_stop_cpp_servant;
    Component::start_component(slow_stop_cpp_servant, argc, argv);
    return 0;
}

