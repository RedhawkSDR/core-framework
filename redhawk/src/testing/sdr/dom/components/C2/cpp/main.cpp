#include <iostream>
#include "ossie/ossieSupport.h"

#include "C2.h"
int main(int argc, char* argv[])
{
    C2_i* C2_servant;
    Component::start_component(C2_servant, argc, argv);
    return 0;
}

