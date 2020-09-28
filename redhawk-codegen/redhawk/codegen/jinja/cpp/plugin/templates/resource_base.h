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

class ${className}
{
    ENABLE_LOGGING;

public:
    ${className} (std::string &IOR, std::string &id);
    virtual ~${className} (void);
    ${className}();

    virtual void start ();
    virtual void halt ();
    virtual void serviceFunction () {};

protected:

    void connectPlugin(std::string &IOR, std::string &id) {
        ossie::logging::ConfigureDefault();
        message_out = new MessageSupplierPort(std::string("metrics_out"));
        PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(message_out);
        message_out->_remove_ref();
        CORBA::Object_ptr object = ::ossie::corba::stringToObject(IOR);
        message_out->connectPort(object, "metrics_out_connection");

        message_in = new MessageConsumerPort("threshold_control");
        message_in->registerMessage("plugin::set_threshold", this, &${className}::updateThreshold);

        _id = id;
    };

    virtual void updateThreshold(const std::string& messageId, const plugin_set_threshold_struct& msgData) {};

    MessageSupplierPort* message_out;
    omni_mutex plugin_running_mutex;
    omni_condition plugin_running;
    MessageConsumerPort* message_in;
    boost::thread* _thread;
    bool _shutdown;
    std::string _id;

};

#endif // ${includeGuard}
