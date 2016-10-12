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

//#include <ossie/CF/ExtendedEvent.h>

#include "MessageSenderCpp.h"


PREPARE_LOGGING(MessageSenderCpp);

MessageSenderCpp::MessageSenderCpp (const char *uuid, const char *label) :
    Resource_impl(uuid, label)
{
    message_out = new extendedMessageSupplier(std::string("message_out"));
    PortableServer::ObjectId_var oid = ossie::corba::RootPOA()->activate_object(message_out);
    message_out->_remove_ref();
}

MessageSenderCpp::~MessageSenderCpp (void)
{

}

CORBA::Object_ptr MessageSenderCpp::getPort (const char* name) throw (CF::PortSupplier::UnknownPort, CORBA::SystemException)
{
    if (strcmp(name, "message_out") != 0) {
        throw CF::PortSupplier::UnknownPort();
    }

    return message_out->_this();
}

void MessageSenderCpp::releaseObject (void) throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    PortableServer::POA_ptr root_poa = ossie::corba::RootPOA();
    PortableServer::ObjectId_var oid = root_poa->servant_to_id(message_out);
    root_poa->deactivate_object(oid);

    Resource_impl::releaseObject();
}

void MessageSenderCpp::start (void) throw (CF::Resource::StartError)
{
    test_message_struct tmp;
    tmp.item_float = 1.0;
    tmp.item_string = std::string("some string");
    message_out->sendMessage(tmp);
    std::vector<test_message_struct> messages;
    messages.resize(2);
    messages[0].item_float = 2.0;
    messages[0].item_string = std::string("another string");
    messages[1].item_float = 3.0;
    messages[1].item_string = std::string("yet another string");
    message_out->sendMessages(messages);
}
