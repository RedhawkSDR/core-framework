#include <iostream>
#include "ossie/ossieSupport.h"

#include "check_cwd_cpp.h"
int main(int argc, char* argv[])
{
    check_cwd_cpp_i* check_cwd_cpp_servant;
    Component::start_component(check_cwd_cpp_servant, argc, argv);
    return 0;
}

