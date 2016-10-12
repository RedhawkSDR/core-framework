#include <iostream>
#include "ossie/ossieSupport.h"

#include "comp.h"
int main(int argc, char* argv[])
{
    comp_i* comp_servant;
    Component::start_component(comp_servant, argc, argv);
    return 0;
}

