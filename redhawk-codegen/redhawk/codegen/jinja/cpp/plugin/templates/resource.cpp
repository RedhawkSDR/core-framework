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
//% set className = component.name
/**************************************************************************

    This is the ${artifactType} code. This file contains the child class where
    custom functionality can be added to the ${artifactType}. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include <iostream>
#include <iomanip>

#include "${className}.h"

PREPARE_LOGGING(${className})

${className}::${className}(std::string &IOR, std::string &id)
{
    connectPlugin(IOR, id);
}

${className}::~${className}(void)
{

}

void ${className}::run()
{
    while (true) {
        usleep(5e5);
        plugin_message_struct plugin_message;
        plugin_message.ok = false;
        plugin_message.plugin_id = _id;
        plugin_message.metric_name = CORBA::string_dup("colors");
        plugin_message.metric_timestamp = redhawk::time::utils::now();
        plugin_message.metric_reason = CORBA::string_dup("should be green but got an orange");
        plugin_message.metric_threshold_value = CORBA::string_dup("green");
        plugin_message.metric_recorded_value = CORBA::string_dup("orange");
        this->message_out->send(plugin_message);
    }
}

void ${className}::start()
{
    _thread = new boost::thread(&${className}::run, this);
}

void ${className}::updateThreshold(const std::string& messageId, const update_threshold_struct& msgData)
{
    // msgData.plugin_id
    // msgData.metric_name
    // msgData.metric_threshold_value
}
