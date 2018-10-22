#include <iostream>
#include "ossie/ossieSupport.h"

#include "alloc_shm.h"
extern "C" {
    Resource_impl* make_component(const std::string& uuid, const std::string& identifier)
    {
        return new alloc_shm_i(uuid.c_str(), identifier.c_str());
    }
}

