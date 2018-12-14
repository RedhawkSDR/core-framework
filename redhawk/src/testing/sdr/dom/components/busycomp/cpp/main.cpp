#include <iostream>
#include "ossie/ossieSupport.h"

#include "busycomp.h"
extern "C" {
    Resource_impl* make_component(const std::string& uuid, const std::string& identifier)
    {
        return new busycomp_i(uuid.c_str(), identifier.c_str());
    }
}

