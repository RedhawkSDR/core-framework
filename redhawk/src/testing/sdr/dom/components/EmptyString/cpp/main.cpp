#include <iostream>
#include "ossie/ossieSupport.h"

#include "EmptyString.h"
int main(int argc, char* argv[])
{
    EmptyString_i* EmptyString_servant;
    Component::start_component(EmptyString_servant, argc, argv);
    return 0;
}
