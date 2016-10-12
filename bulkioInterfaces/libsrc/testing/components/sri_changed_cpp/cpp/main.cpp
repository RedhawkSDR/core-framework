#include <iostream>
#include "ossie/ossieSupport.h"

#include "sri_changed_cpp.h"
int main(int argc, char* argv[])
{
    sri_changed_cpp_i* sri_changed_cpp_servant;
    Resource_impl::start_component(sri_changed_cpp_servant, argc, argv);
    return 0;
}

