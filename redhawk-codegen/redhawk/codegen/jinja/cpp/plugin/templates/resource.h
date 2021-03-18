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
//% set includeGuard = className.upper() + '_IMPL_H'
#ifndef ${includeGuard}
#define ${includeGuard}

#include "${className}_base.h"

class ${className} : public ${className}_base
{
    ENABLE_LOGGING;

public:
    ${className} (std::string &IOR, std::string &id);
    virtual ~${className} (void);

    void run() {
        this->start();
        plugin_running.wait();
    }

protected:

    void registerPlugin() {
        extern char *program_invocation_short_name;

        plugin_registration_struct plugin_reg_message;
        plugin_reg_message.id = _id;
        plugin_reg_message.name = program_invocation_short_name;
        plugin_reg_message.description = "none";
        plugin_reg_message.metric_port = ::ossie::corba::objectToString(message_in->_this());
        this->message_out->sendMessage(plugin_reg_message);
    };

    void serviceFunction ();

    void updateThreshold(const std::string& messageId, const plugin_set_threshold_struct& msgData);

    /*/// Message structure definition for plugin_registration
    plugin_registration_struct plugin_registration;
    /// Message structure definition for plugin_heartbeat
    plugin_heartbeat_struct plugin_heartbeat;
    /// Message structure definition for plugin_message
    plugin_message_struct plugin_message;*/

private:
    ${className}() {};
};

#endif // ${includeGuard}
