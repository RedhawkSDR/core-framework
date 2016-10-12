#include <iostream>
#include "ossie/ossieSupport.h"

#include "PropertyChange_C1.h"
int main(int argc, char* argv[])
{
    PropertyChange_C1_i* PropertyChange_C1_servant;
    Component::start_component(PropertyChange_C1_servant, argc, argv);
    return 0;
}

