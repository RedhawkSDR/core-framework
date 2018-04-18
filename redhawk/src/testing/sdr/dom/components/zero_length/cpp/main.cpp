#include <iostream>
#include "ossie/ossieSupport.h"

#include "zero_length.h"
int main(int argc, char* argv[])
{
    zero_length_i* zero_length_servant;
    Component::start_component(zero_length_servant, argc, argv);
    return 0;
}

