#include <iostream>
#include "ossie/ossieSupport.h"

#include "Property_CPP.h"
int main(int argc, char* argv[])
{
    Property_CPP_i* Property_CPP_servant;
    Component::start_component(Property_CPP_servant, argc, argv);
    return 0;
}

