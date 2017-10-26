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

#ifndef MESSAGESUPPLIER_H
#define MESSAGESUPPLIER_H

#include <string>

#include <boost/thread/mutex.hpp>

#include <COS/CosEventChannelAdmin.hh>

#include "UsesPort.h"

/************************************************************************************
  Message producer
************************************************************************************/
class MessageSupplierPort : public redhawk::UsesPort
{

public:
    MessageSupplierPort (const std::string& name);
    virtual ~MessageSupplierPort (void);

    void push(const CORBA::Any& data);

    // Send a single message
    template <typename Message>
    void sendMessage(const Message& message) {
        const Message* begin(&message);
        const Message* end(&begin[1]);
        sendMessages(begin, end);
    }

    // Send a sequence of messages
    template <class Sequence>
    void sendMessages(const Sequence& messages) {
        sendMessages(messages.begin(), messages.end());
    }
    
    // Send a set of messages from an iterable set
    template <typename Iterator>
    void sendMessages(Iterator first, Iterator last)
    {
        boost::mutex::scoped_lock lock(updatingPortsLock);
        _beginMessageQueue(std::distance(first, last));
        for (; first != last; ++first) {
            _queueMessage(*first);
        }
        _sendMessageQueue();
    }

    std::string getRepid() const;

protected:
    virtual void _validatePort(CORBA::Object_ptr object);
    virtual redhawk::UsesTransport* _createTransport(CORBA::Object_ptr object, const std::string& connectionId);

    template <class Message>
    inline void _queueMessage(const Message& message)
    {
        // Workaround for older components whose structs have a non-const,
        // non-static member function getId(): const_cast the value
        const std::string messageId = const_cast<Message&>(message).getId();
        const char* format = ::internal::message_traits<Message>::format();
        _queueMessage(messageId, format, &message, &MessageSupplierPort::_serializeMessage<Message>);
    }

    template <class Message>
    static void _serializeMessage(CORBA::Any& any, const void* data)
    {
        any <<= *(reinterpret_cast<const Message*>(data));
    }

    typedef void (*SerializerFunc)(CORBA::Any&,const void*);

    void _beginMessageQueue(size_t count);
    void _queueMessage(const std::string& msgId, const char* format, const void* msgData, SerializerFunc serializer);
    void _sendMessageQueue();

    class MessageTransport;
    class CorbaTransport;
    class LocalTransport;

    typedef redhawk::UsesPort::TransportIteratorAdapter<MessageTransport> TransportIterator;
};

#endif // MESSAGESUPPLIER_H
