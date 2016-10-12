/*
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
 */

#include <iostream>
#include <iomanip>

#include "MessageReceiverCpp.h"


PREPARE_LOGGING(MessageReceiverCpp);

MessageReceiverCpp::MessageReceiverCpp (const char *uuid, const char *label) :
    Resource_impl(uuid, label)
{
    message_in = new MessageConsumerPort("message_in");
    message_in->registerMessage("test_message", this, &MessageReceiverCpp::messageReceived);
    PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(message_in);
    message_in->_remove_ref();
    
    addProperty(received_messages,
                received_messages, 
               "received_messages",
               "received_messages",
               "readwrite",
               "null",
               "external",
               "configure");
}

MessageReceiverCpp::~MessageReceiverCpp (void)
{

}

CORBA::Object_ptr MessageReceiverCpp::getPort (const char* name) throw (CF::PortSupplier::UnknownPort, CORBA::SystemException)
{
    if (strcmp(name, "message_in") != 0) {
        throw CF::PortSupplier::UnknownPort();
    }

    return message_in->_this();
}

void MessageReceiverCpp::releaseObject (void) throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    PortableServer::POA_ptr root_poa = ossie::corba::RootPOA();
    PortableServer::ObjectId_var oid = root_poa->servant_to_id(message_in);
    root_poa->deactivate_object(oid);

    Resource_impl::releaseObject();
}

void MessageReceiverCpp::messageReceived (const std::string& id, const test_message_struct& msg)
{
    boost::mutex::scoped_lock lock(messageAccess);
    std::stringstream tmp;
    tmp << id << "," << msg.item_float << "," << msg.item_string;
    received_messages.push_back(tmp.str());
    std::cout << "Received message " << id << std::endl;
    std::cout << " item_float   = " << msg.item_float << std::endl;
    std::cout << " item_string = " << msg.item_string << std::endl;
}
