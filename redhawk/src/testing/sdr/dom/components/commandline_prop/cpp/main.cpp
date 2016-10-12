#include <iostream>
#include "ossie/ossieSupport.h"

#include "commandline_prop.h"
int main(int argc, char* argv[])
{
    commandline_prop_i* commandline_prop_servant;
    Component::start_component(commandline_prop_servant, argc, argv);
    return 0;
}

