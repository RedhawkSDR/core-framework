#include <iostream>
#include "ossie/ossieSupport.h"

#include "Oversized_framedata.h"
int main(int argc, char* argv[])
{
    Oversized_framedata_i* Oversized_framedata_servant;
    Resource_impl::start_component(Oversized_framedata_servant, argc, argv);
    return 0;
}

