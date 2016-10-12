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

#include <string>

#include <ossie/Resource_impl.h>
#include <ossie/debug.h>
#include "struct_props.h"
#include <ossie/MessageInterface.h>

class MessageReceiverCpp : public Resource_impl
{
    ENABLE_LOGGING;

public:
    MessageReceiverCpp (const char* uuid, const char* label);
    ~MessageReceiverCpp (void);
 
    void messageReceived (const std::string&, const test_message_struct&);

private:
    MessageConsumerPort* message_in;
    std::vector<std::string> received_messages;
    boost::mutex messageAccess;
};

#endif
