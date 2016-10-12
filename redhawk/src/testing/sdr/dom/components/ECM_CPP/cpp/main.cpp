#include <iostream>
#include "ossie/ossieSupport.h"

#include "ECM_CPP.h"
int main(int argc, char* argv[])
{
    ECM_CPP_i* ECM_CPP_servant;
    Component::start_component(ECM_CPP_servant, argc, argv);
    return 0;
}

