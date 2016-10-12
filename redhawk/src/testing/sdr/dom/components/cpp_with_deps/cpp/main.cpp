#include <iostream>
#include "ossie/ossieSupport.h"

#include "cpp_with_deps.h"
int main(int argc, char* argv[])
{
    cpp_with_deps_i* cpp_with_deps_servant;
    Component::start_component(cpp_with_deps_servant, argc, argv);
    return 0;
}

