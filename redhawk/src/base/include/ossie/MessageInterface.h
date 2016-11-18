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

#ifndef MESSAGEINTERFACE_H
#define MESSAGEINTERFACE_H

#include <map>
#include <string>
#include <vector>
#include <iterator>

#include <boost/utility/enable_if.hpp>

#include "CF/ExtendedEvent.h"
#include "CF/QueryablePort.h"
#include "CF/cf.h"
#include "CorbaUtils.h"
#include "Port_impl.h"
#include "callback.h"

#include <COS/CosEventChannelAdmin.hh>



/************************************************************************************
  Message consumer
************************************************************************************/

class MessageConsumerPort;

#ifdef BEGIN_AUTOCOMPLETE_IGNORE
    /**
     * \cond INTERNAL
     */
class Consumer_i : public virtual POA_CosEventChannelAdmin::ProxyPushConsumer {
    public:
        Consumer_i(MessageConsumerPort *_parent);
        // CosEventComm::PushConsumer methods
        void push(const CORBA::Any& data);

        void connect_push_supplier(CosEventComm::PushSupplier_ptr push_supplier);

        void disconnect_push_consumer();
    
    protected:
        MessageConsumerPort *parent;

};

class SupplierAdmin_i : public virtual POA_CosEventChannelAdmin::SupplierAdmin {
    public:
        SupplierAdmin_i(MessageConsumerPort *_parent);
        
        CosEventChannelAdmin::ProxyPushConsumer_ptr obtain_push_consumer();
        
        CosEventChannelAdmin::ProxyPullConsumer_ptr obtain_pull_consumer();
        
    
    protected:
        MessageConsumerPort *parent;
        unsigned int instance_counter;
    
};
    /**
     * \endcond
     */
#endif

namespace internal {
    template <class T>
    struct has_format
    {
        typedef ::boost::type_traits::no_type no_type;
        typedef ::boost::type_traits::yes_type yes_type;
        template <typename U, U> struct type_check;

        template <typename U>
        static yes_type& check(type_check<const char* (*)(), &U::getFormat>*);

        template <typename>
        static no_type& check(...);

        static bool const value = (sizeof(check<T>(0)) == sizeof(yes_type));
    };

    template <class T, class Enable=void>
    struct message_traits
    {
        static const char* format()
        {
            return "";
        }
    };

    template <class T>
    struct message_traits<T, typename boost::enable_if<has_format<T> >::type>
    {
        static const char* format()
        {
            return T::getFormat();
        }
    };
}

class MessageConsumerPort : public Port_Provides_base_impl
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
, public virtual POA_ExtendedEvent::MessageEvent
#endif
{
    ENABLE_LOGGING

public:
    MessageConsumerPort (std::string port_name);
    virtual ~MessageConsumerPort (void);

    /*
     * Register a callback function
     * @param id The message id that this callback is intended to support
     * @param target A pointer to the object that owns the callback function
     * @param func The function that implements the callback
     */
    template <class Class, class MessageStruct>
    void registerMessage (const std::string& id, Class* target, void (Class::*func)(const std::string&, const MessageStruct&))
    {
        const char* format = ::internal::message_traits<MessageStruct>::format();
        callbacks_[id] = new MessageCallbackImpl<MessageStruct>(format, boost::bind(func, target, _1, _2));
    }

    template <class Target, class Func>
    void registerMessage (Target target, Func func)
    {
        generic_callbacks_.add(target, func);
    }

    // CF::Port methods
    void connectPort(CORBA::Object_ptr connection, const char* connectionId);

    void disconnectPort(const char* connectionId);
    
    CosEventChannelAdmin::ConsumerAdmin_ptr for_consumers();
    
    CosEventChannelAdmin::SupplierAdmin_ptr for_suppliers();
    
    void destroy();
    
    CosEventChannelAdmin::ProxyPushConsumer_ptr extendConsumers(std::string consumer_id);
    
    Consumer_i* removeConsumer(std::string consumer_id);
    
    void fireCallback (const std::string& id, const CORBA::Any& data);

	std::string getRepid() const;

	std::string getDirection() const;
    

protected:
    friend class MessageSupplierPort;

    void addSupplier (const std::string& connectionId, CosEventComm::PushSupplier_ptr supplier);

    CosEventComm::PushSupplier_ptr removeSupplier (const std::string& connectionId);

    bool hasGenericCallbacks();
    void dispatchGeneric(const std::string& id, const CORBA::Any& data);
    
    boost::mutex portInterfaceAccess;
    std::map<std::string, Consumer_i*> consumers;
    std::map<std::string, CosEventChannelAdmin::EventChannel_ptr> _connections;
    
    SupplierAdmin_i *supplier_admin;
    
    /*
     * Abstract untyped interface for message callbacks.
     */
    class MessageCallback
    {
    public:
        virtual void dispatch (const std::string& value, const CORBA::Any& data) = 0;
        virtual void dispatch (const std::string& value, const void* data) = 0;
        virtual ~MessageCallback () { }

        bool isCompatible (const char* format)
        {
            if (_format.empty()) {
                // Message type has no format descriptor, assume that it cannot
                // be passed via void*
                return false;
            }
            // The format descriptors must be identical, otherwise go through
            // CORBA::Any
            return _format == format;
        }

    protected:
        MessageCallback(const std::string& format) :
            _format(format)
        {
        }

        const std::string _format;
    };


    /*
     * Concrete typed class for message callbacks.
     */
    template <class Message>
    class MessageCallbackImpl : public MessageCallback
    {
    public:
        typedef boost::function<void (const std::string&, const Message&)> CallbackFunc;

        MessageCallbackImpl (const std::string& format, CallbackFunc func) :
            MessageCallback(format),
            func_(func)
        {
        }

        virtual void dispatch (const std::string& value, const CORBA::Any& data)
        {
            Message message;
            if (data >>= message) {
                func_(value, message);
            }
        }

        virtual void dispatch (const std::string& value, const void* data)
        {
            const Message* message = reinterpret_cast<const Message*>(data);
            func_(value, *message);
        }

    private:
        CallbackFunc func_;
    };

    typedef std::map<std::string, MessageCallback*> CallbackTable;
    CallbackTable callbacks_;

    MessageCallback* getMessageCallback(const std::string& msgId);

    ossie::notification<void (const std::string&, const CORBA::Any&)> generic_callbacks_;

    typedef std::map<std::string, CosEventComm::PushSupplier_var> SupplierTable;
    SupplierTable suppliers_;
};


/************************************************************************************
  Message producer
************************************************************************************/
#include "UsesPort.h"
class MessageSupplierPort : public redhawk::UsesPort
{

public:
    MessageSupplierPort (std::string port_name);
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
    virtual redhawk::BasicTransport* _createTransport(CORBA::Object_ptr object, const std::string& connectionId);
    virtual void _transportDisconnected(redhawk::BasicTransport* transport);

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

    typedef boost::function<void(CORBA::Any&,const void*)> SerializerFunc;

    void _beginMessageQueue(size_t count);
    void _queueMessage(const std::string& msgId, const char* format, const void* msgData, SerializerFunc serializer);
    void _sendMessageQueue();

    class MessageTransport;
    class RemoteTransport;
    class LocalTransport;
};

#endif // MESSAGEINTERFACE_H
