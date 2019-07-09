#include <iostream>
#include "ossie/ossieSupport.h"

#include "stream_snk.h"
extern "C" {
    Resource_impl* make_component(const std::string& uuid, const std::string& identifier)
    {
        return new stream_snk_i(uuid.c_str(), identifier.c_str());
    }
}

