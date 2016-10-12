#include <iostream>
#include "ossie/ossieSupport.h"

#include "C1.h"
int main(int argc, char* argv[])
{
    C1_i* C1_servant;
    Component::start_component(C1_servant, argc, argv);
    return 0;
}

