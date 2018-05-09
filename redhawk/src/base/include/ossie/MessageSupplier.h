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
#include "internal/message_traits.h"

/************************************************************************************
  Message producer
************************************************************************************/
class MessageSupplierPort : public redhawk::UsesPort
{

public:
    MessageSupplierPort (const std::string& name);
    virtual ~MessageSupplierPort (void);

    /**
     * @brief  Sends pre-serialized messages.
     * @param data          Messages serialized to a CORBA::Any.
     * @param connectionID  Target connection (default: all).
     * @throw redhawk::InvalidConnectionId  If @p connectionId is not empty and
     *                                      does not match any connection
     */
    void push(const CORBA::Any& data, const std::string& connectionId=std::string());

    /**
     * @brief  Sends a single message.
     * @param message       Message to send.
     * @param connectionID  Target connection (default: all).
     * @throw redhawk::InvalidConnectionId  If @p connectionId is not empty and
     *                                      does not match any connection
     */
    template <typename Message>
    void sendMessage(const Message& message, const std::string& connectionId=std::string())
    {
        const Message* begin(&message);
        const Message* end(&begin[1]);
        sendMessages(begin, end, connectionId);
    }

    /**
     * @brief  Sends a sequence of messages.
     * @param messages      Container of messages to send.
     * @param connectionID  Target connection (default: all).
     * @throw redhawk::InvalidConnectionId  If @p connectionId is not empty and
     *                                      does not match any connection
     */
    template <class Sequence>
    void sendMessages(const Sequence& messages, const std::string& connectionId=std::string())
    {
        sendMessages(messages.begin(), messages.end(), connectionId);
    }

    /**
     * @brief  Sends a sequence of messages.
     * @param first         Iterator to first message.
     * @param last          Iterator to one past last message.
     * @param connectionID  Target connection (default: all).
     * @throw redhawk::InvalidConnectionId  If @p connectionId is not empty and
     *                                      does not match any connection
     */
    template <typename Iterator>
    void sendMessages(Iterator first, Iterator last, const std::string& connectionId=std::string())
    {
        boost::mutex::scoped_lock lock(updatingPortsLock);
        _checkConnectionId(connectionId);
        _beginMessageQueue(std::distance(first, last), connectionId);
        for (; first != last; ++first) {
            _queueMessage(*first, connectionId);
        }
        _sendMessageQueue(connectionId);
    }

    std::string getRepid() const;

protected:
    virtual void _validatePort(CORBA::Object_ptr object);
    virtual redhawk::UsesTransport* _createTransport(CORBA::Object_ptr object, const std::string& connectionId);

    template <class Message>
    inline void _queueMessage(const Message& message, const std::string& connectionId)
    {
        // Use the traits class to abstract differences between message classes
        // based on the REDHAWK version with which they were generated
        typedef ::redhawk::internal::message_traits<Message> traits;
        const std::string messageId = traits::getId(message);
        const char* format = traits::format();
        _queueMessage(messageId, format, &message, &traits::serialize, connectionId);
    }

    typedef void (*SerializerFunc)(CORBA::Any&,const void*);

    void _beginMessageQueue(size_t count, const std::string& connectionId);
    void _queueMessage(const std::string& msgId, const char* format, const void* msgData,
                       SerializerFunc serializer, const std::string& connectionId);
    void _sendMessageQueue(const std::string& connectionId);

    bool _isConnectionSelected(const std::string& connectionId, const std::string& targetId);
    void _checkConnectionId(const std::string& connectionId);

    class MessageTransport;
    class CorbaTransport;
    class LocalTransport;

    typedef redhawk::UsesPort::TransportIteratorAdapter<MessageTransport> TransportIterator;
};

#endif // MESSAGESUPPLIER_H
