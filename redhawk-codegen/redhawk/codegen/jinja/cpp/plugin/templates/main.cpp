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
#include <iostream>
#include <string>
#include <map>
#include <boost/scoped_ptr.hpp>

#include <ossie/ossieSupport.h>
#include <Logging_impl.h>
#include <Port_impl.h>
#include <LifeCycle_impl.h>
#include <PortSet_impl.h>
#include <PropertySet_impl.h>
#include <TestableObject_impl.h>
#include <ossie/logging/loghelpers.h>
#include <ossie/prop_helpers.h>
#include <ossie/Containers.h>
#include <ossie/PropertyMap.h>
#include <ossie/MessageInterface.h>

#include "${component.name}.h"
#include "struct_props.h"

int main(int argc, char* argv[])
{
    ossie::corba::CorbaInit(argc, argv);

    std::string ior;
    std::string id;
    if (argc < 3) {
        MessageConsumerPort* _consumer = new MessageConsumerPort("consumer");
        ior = ::ossie::corba::objectToString(_consumer->_this());
        id = "the id";
    } else {
        ior = argv[1];
        id = argv[2];
    }

    ${component.name}* my_plugin = new ${component.name}(ior, id);
    my_plugin->start();
    while (true) {
        usleep(1e6);
    }
}
