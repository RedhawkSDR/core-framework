/*#
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of REDHAWK core.
 * 
 * REDHAWK core is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * 
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 #*/
/*{% block license %}*/
/*# Allow child templates to include license #*/
/*{% endblock %}*/
/*{% block includes %}*/
#include <iostream>
#include "ossie/ossieSupport.h"

#include "${component.userclass.header}"
/*{% endblock %}*/
/*{% block main %}*/
/*{% if component is device %}*/

${component.userclass.name} *devicePtr;

void signal_catcher(int sig)
{
    // IMPORTANT Don't call exit(...) in this function
    // issue all CORBA calls that you need for cleanup here before calling ORB shutdown
    if (devicePtr) {
        devicePtr->halt();
    }
}
/*{% endif %}*/
/*{% if not component is device and component.impl.module %}*/
extern "C" {
    Resource_impl* make_component(const std::string& uuid, const std::string& identifier)
    {
        return new ${component.userclass.name}(uuid.c_str(), identifier.c_str());
    }
}
/*{% else %}*/
int main(int argc, char* argv[])
{
/*{% if component is device %}*/
    struct sigaction sa;
    sa.sa_handler = signal_catcher;
    sa.sa_flags = 0;
    devicePtr = 0;

    Device_impl::start_device(&devicePtr, sa, argc, argv);
/*{% else %}*/
/*{%   set servant = component.basename + '_servant' %}*/
    ${component.userclass.name}* ${servant};
    Component::start_component(${servant}, argc, argv);
/*{% endif %}*/
    return 0;
}
/*{% endif %}*/
/*{% endblock %}*/
