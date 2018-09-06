#include <iostream>
#include "ossie/ossieSupport.h"

#include "StreamWriteCheck.h"
int main(int argc, char* argv[])
{
    StreamWriteCheck_i* StreamWriteCheck_servant;
    Component::start_component(StreamWriteCheck_servant, argc, argv);
    return 0;
}

