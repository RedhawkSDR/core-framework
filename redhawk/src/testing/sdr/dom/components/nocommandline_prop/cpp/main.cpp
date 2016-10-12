#include <iostream>
#include "ossie/ossieSupport.h"

#include "nocommandline_prop.h"
int main(int argc, char* argv[])
{
    nocommandline_prop_i* nocommandline_prop_servant;
    Component::start_component(nocommandline_prop_servant, argc, argv);
    return 0;
}

