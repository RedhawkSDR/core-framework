#include <iostream>
#include "ossie/ossieSupport.h"

#include "src.h"
int main(int argc, char* argv[])
{
    src_i* src_servant;
    Component::start_component(src_servant, argc, argv);
    return 0;
}

