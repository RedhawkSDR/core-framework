#include <iostream>
#include "ossie/ossieSupport.h"

#include "timeprop_cpp.h"
int main(int argc, char* argv[])
{
    timeprop_cpp_i* timeprop_cpp_servant;
    Component::start_component(timeprop_cpp_servant, argc, argv);
    return 0;
}

