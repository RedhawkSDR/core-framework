#include <iostream>
#include "ossie/ossieSupport.h"

#include "TestCppPropsRange.h"
int main(int argc, char* argv[])
{
    TestCppPropsRange_i* TestCppPropsRange_servant;
    Component::start_component(TestCppPropsRange_servant, argc, argv);
    return 0;
}

