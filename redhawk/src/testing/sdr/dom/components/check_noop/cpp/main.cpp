#include <iostream>
#include "ossie/ossieSupport.h"

#include "check_noop.h"
extern "C" {
    Resource_impl* make_component(const std::string& uuid, const std::string& identifier)
    {
        return new check_noop_i(uuid.c_str(), identifier.c_str());
    }
}

