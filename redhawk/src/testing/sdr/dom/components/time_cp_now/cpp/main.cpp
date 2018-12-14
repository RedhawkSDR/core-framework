#include <iostream>
#include "ossie/ossieSupport.h"

#include "time_cp_now.h"
extern "C" {
    Resource_impl* make_component(const std::string& uuid, const std::string& identifier)
    {
        return new time_cp_now_i(uuid.c_str(), identifier.c_str());
    }
}

