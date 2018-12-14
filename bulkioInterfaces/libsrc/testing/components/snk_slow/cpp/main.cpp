#include <iostream>
#include "ossie/ossieSupport.h"

#include "snk_slow.h"
extern "C" {
    Resource_impl* make_component(const std::string& uuid, const std::string& identifier)
    {
        return new snk_slow_i(uuid.c_str(), identifier.c_str());
    }
}

