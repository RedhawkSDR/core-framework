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
    addPort("message_in", message_in);
    
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
    delete message_in;
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
