#include <iostream>
#include "ossie/ossieSupport.h"

#include "stream_src.h"
extern "C" {
    Resource_impl* make_component(const std::string& uuid, const std::string& identifier)
    {
        return new stream_src_i(uuid.c_str(), identifier.c_str());
    }
}

