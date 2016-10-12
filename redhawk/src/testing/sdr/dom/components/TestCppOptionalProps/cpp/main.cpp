#include <iostream>
#include "ossie/ossieSupport.h"

#include "TestCppOptionalProps.h"
int main(int argc, char* argv[])
{
    TestCppOptionalProps_i* TestCppOptionalProps_servant;
    Component::start_component(TestCppOptionalProps_servant, argc, argv);
    return 0;
}

