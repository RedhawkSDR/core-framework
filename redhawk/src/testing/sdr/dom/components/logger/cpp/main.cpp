#include <iostream>
#include "ossie/ossieSupport.h"

#include "logger.h"
extern "C" {
    Resource_impl* make_component(const std::string& uuid, const std::string& identifier)
    {
        return new logger_i(uuid.c_str(), identifier.c_str());
    }
}

