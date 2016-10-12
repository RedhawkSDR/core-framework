#include <iostream>
#include "ossie/ossieSupport.h"

#include "prop_trigger_timing.h"
int main(int argc, char* argv[])
{
    prop_trigger_timing_i* prop_trigger_timing_servant;
    Component::start_component(prop_trigger_timing_servant, argc, argv);
    return 0;
}

