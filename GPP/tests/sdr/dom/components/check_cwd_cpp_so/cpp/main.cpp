#include <iostream>
#include "ossie/ossieSupport.h"

#include "check_cwd_cpp_so.h"
extern "C" {
    Resource_impl* make_component(const std::string& uuid, const std::string& identifier)
    {
        return new check_cwd_cpp_so_i(uuid.c_str(), identifier.c_str());
    }
}

