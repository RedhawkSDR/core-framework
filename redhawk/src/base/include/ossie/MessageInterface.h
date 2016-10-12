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

#include "CF/ExtendedEvent.h"
#include "CF/cf.h"
#include "CorbaUtils.h"
#include "Port_impl.h"

#include <COS/CosEventChannelAdmin.hh>



/************************************************************************************
  Message consumer
************************************************************************************/

class MessageConsumerPort;

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

class MessageConsumerPort : public Port_Provides_base_impl, public virtual POA_ExtendedEvent::MessageEvent {

public:
    MessageConsumerPort (std::string port_name);
    virtual ~MessageConsumerPort (void) { };

    template <class Class, class MessageStruct>
    void registerMessage (const std::string& id, Class* target, void (Class::*func)(const std::string&, const MessageStruct&))
    {
        callbacks_[id] = new MemberCallback<Class, MessageStruct>(*target, func);
    }

    template <class Class>
    void registerMessage (Class* target, void (Class::*func)(const std::string&, const CORBA::Any&))
    {
        generic_callbacks_.push_back(new GenericMemberCallback<Class>(*target, func));
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
    

protected:
    void addSupplier (const std::string& connectionId, CosEventComm::PushSupplier_ptr supplier);

    CosEventComm::PushSupplier_ptr removeSupplier (const std::string& connectionId);
    
    boost::mutex portInterfaceAccess;
    std::map<std::string, Consumer_i*> consumers;
    std::map<std::string, CosEventChannelAdmin::EventChannel_ptr> _connections;
    
    SupplierAdmin_i *supplier_admin;
    
    /**
     * Abstract interface for message callbacks.
     */
    class MessageCallback
    {
    public:
        virtual void operator() (const std::string& value, const CORBA::Any& data) = 0;
        virtual ~MessageCallback () { }

    protected:
        MessageCallback () { }
    };


    /**
     * Concrete class for member function property change callbacks.
     */
    template <class Class, class M>
    class MemberCallback : public MessageCallback
    {
    public:
        typedef void (Class::*MemberFn)(const std::string&, const M&);

        virtual void operator() (const std::string& value, const CORBA::Any& data)
        {
            M message;
            if (data >>= message) {
                (target_.*func_)(value, message);
            }
        }

    protected:
        // Only allow MessageConsumerPort to instantiate this class.
        MemberCallback (Class& target, MemberFn func) :
            target_(target),
            func_(func)
        {
        }

        friend class MessageConsumerPort;

        Class& target_;
        MemberFn func_;
    };
    
    template <class Class>
    class GenericMemberCallback : public MessageCallback
    {
    public:
        typedef void (Class::*MemberFn)(const std::string&, const CORBA::Any&);

        virtual void operator() (const std::string& value, const CORBA::Any& data)
        {
            (target_.*func_)(value, data);
        }

    protected:
        // Only allow MessageConsumerPort to instantiate this class.
        GenericMemberCallback (Class& target, MemberFn func) :
            target_(target),
            func_(func)
        {
        }

        friend class MessageConsumerPort;

        Class& target_;
        MemberFn func_;
    };

    typedef std::map<std::string, MessageCallback*> CallbackTable;
    typedef std::vector<MessageCallback*> GenericCallbackTable;
    CallbackTable callbacks_;
    GenericCallbackTable generic_callbacks_;

    typedef std::map<std::string, CosEventComm::PushSupplier_var> SupplierTable;
    SupplierTable suppliers_;
};


/************************************************************************************
  Message producer
************************************************************************************/

class MessageSupplierPort : public Port_Uses_base_impl, public virtual POA_CF::Port {

public:
    MessageSupplierPort (std::string port_name);
    virtual ~MessageSupplierPort (void);

    // CF::Port methods
    void connectPort(CORBA::Object_ptr connection, const char* connectionId);
    void disconnectPort(const char* connectionId);

    void push(const CORBA::Any& data);

    CosEventChannelAdmin::ProxyPushConsumer_ptr removeConsumer(std::string consumer_id);
    void extendConsumers(std::string consumer_id, CosEventChannelAdmin::ProxyPushConsumer_ptr proxy_consumer);

    template <typename Message>
    void sendMessage(const Message& message) {
        const Message* begin(&message);
        const Message* end(&begin[1]);
        sendMessages(begin, end);
    }

    template <class Sequence>
    void sendMessages(const Sequence& messages) {
        sendMessages(messages.begin(), messages.end());
    }
    
    template <typename Iterator>
    void sendMessages(Iterator first, Iterator last)
    {
        CF::Properties properties;
        properties.length(std::distance(first, last));
        for (CORBA::ULong ii = 0; first != last; ++ii, ++first) {
            // Workaround for older components whose structs have a non-const,
            // non-static member function getId(): determine the type of value
            // pointed to by the iterator, and const_cast the dereferenced
            // value; this ensures that it works for both bare pointers and
            // "true" iterators
            typedef typename std::iterator_traits<Iterator>::value_type value_type;
            properties[ii].id = const_cast<value_type&>(*first).getId().c_str();
            properties[ii].value <<= *first;
        }
        CORBA::Any data;
        data <<= properties;
        push(data);
    }

protected:
    boost::mutex portInterfaceAccess;
    std::map<std::string, CosEventChannelAdmin::ProxyPushConsumer_var> consumers;
    std::map<std::string, CosEventChannelAdmin::EventChannel_ptr> _connections;

};

#endif // MESSAGEINTERFACE_H
