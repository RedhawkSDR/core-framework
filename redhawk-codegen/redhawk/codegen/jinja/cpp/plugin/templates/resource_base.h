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
//% set className = component.name + '_base'
//% set includeGuard = className.upper() + '_IMPL_H'
#ifndef ${includeGuard}
#define ${includeGuard}

#include <ossie/MessageInterface.h>

#include <string>

#include <ossie/Resource_impl.h>
#include <ossie/debug.h>
#include "struct_props.h"

class GPPMetricSupplier : public MessageSupplierPort {
    ENABLE_LOGGING;

    public:
        GPPMetricSupplier(std::string port_name) : MessageSupplierPort(port_name) {
        };

        void send(plugin_registration_struct &in_plugin_registration) {
            CF::Properties outProps;
            CORBA::Any data;
            outProps.length(1);
            outProps[0].id = CORBA::string_dup("plugin::registration");
            outProps[0].value <<= in_plugin_registration;
            data <<= outProps;
            push(data);
        };

        void send(plugin_message_struct &in_plugin_message) {
            CF::Properties outProps;
            CORBA::Any data;
            outProps.length(1);
            outProps[0].id = CORBA::string_dup("plugin::message");
            outProps[0].value <<= in_plugin_message;
            data <<= outProps;
            push(data);
        };

        void send(plugin_heartbeat_struct &in_plugin_heartbeat) {
            CF::Properties outProps;
            CORBA::Any data;
            outProps.length(1);
            outProps[0].id = CORBA::string_dup("plugin::heartbeat");
            outProps[0].value <<= in_plugin_heartbeat;
            data <<= outProps;
            push(data);
        };
};

class ${className}
{
    ENABLE_LOGGING;

public:
    ${className} (std::string &IOR, std::string &id);
    ~${className} (void);
    ${className}() {};

    virtual void start ();
    virtual void run () {};

protected:

    void connectPlugin(std::string &IOR, std::string &id) {
        ossie::logging::ConfigureDefault();
        message_out = new GPPMetricSupplier(std::string("metrics_out"));
        PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(message_out);
        message_out->_remove_ref();
        CORBA::Object_ptr object = ::ossie::corba::stringToObject(IOR);
        message_out->connectPort(object, "metrics_out_connection");

        message_in = new MessageConsumerPort("threshold_control");
        message_in->registerMessage("plugin::update_threshold", this, &${className}::updateThreshold);

        _id = id;
    };

    virtual void updateThreshold(const std::string& messageId, const plugin_update_threshold_struct& msgData) {};

    GPPMetricSupplier* message_out;
    MessageConsumerPort* message_in;
    boost::thread* _thread;
    std::string _id;
};

#endif // ${includeGuard}
