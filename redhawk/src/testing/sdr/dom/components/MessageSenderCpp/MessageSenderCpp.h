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

#ifndef MESSAGERECEIVERCPP__H
#define MESSAGERECEIVERCPP__H

#include <ossie/MessageInterface.h>

#include <string>

#include <ossie/Resource_impl.h>
#include <ossie/debug.h>
#include "struct_props.h"

class extendedMessageSupplier : public MessageSupplierPort {
    public:
        extendedMessageSupplier(std::string port_name) : MessageSupplierPort(port_name) {
        };

        void sendMessage(test_message_struct message) {
            CF::Properties outProps;
            CORBA::Any data;
            outProps.length(1);
            outProps[0].id = CORBA::string_dup(message.getName().c_str());
            outProps[0].value <<= message;
            data <<= outProps;
            push(data);
        };

        void sendMessages(std::vector<test_message_struct> messages) {
            CF::Properties outProps;
            CORBA::Any data;
            outProps.length(messages.size());
            for (unsigned int i=0; i<messages.size(); i++) {
                outProps[i].id = CORBA::string_dup(messages[i].getName().c_str());
                outProps[i].value <<= messages[i];
            }
            data <<= outProps;
            push(data);
        };
};

class MessageSenderCpp : public Resource_impl
{
    ENABLE_LOGGING;

public:
    MessageSenderCpp (const char* uuid, const char* label);
    ~MessageSenderCpp (void);
 
    CORBA::Object_ptr getPort (const char*);
 
    void releaseObject (void);

    void start();

private:
    extendedMessageSupplier* message_out;
};

#endif
