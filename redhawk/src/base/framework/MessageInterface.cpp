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

#include "ossie/MessageInterface.h"
#include <iostream>

PREPARE_CF_LOGGING(MessageConsumerPort)

Consumer_i::Consumer_i(MessageConsumerPort *_parent) {
    parent = _parent;
}

// CosEventComm::PushConsumer methods
void Consumer_i::push(const CORBA::Any& data) {
    CF::Properties* temp;
    if (!(data >>= temp)) {
        return;
    }
    CF::Properties& props = *temp;
    for (CORBA::ULong ii = 0; ii < props.length(); ++ii) {
        const std::string id = static_cast<const char*>(props[ii].id);
        parent->fireCallback(id, props[ii].value);
    }
};

void Consumer_i::connect_push_supplier(CosEventComm::PushSupplier_ptr push_supplier) {
};

void Consumer_i::disconnect_push_consumer() {
};

SupplierAdmin_i::SupplierAdmin_i(MessageConsumerPort *_parent) {
    parent = _parent;
    instance_counter = 0;
};
        
CosEventChannelAdmin::ProxyPushConsumer_ptr SupplierAdmin_i::obtain_push_consumer() {
    bool addedConsumer = false;
    CosEventChannelAdmin::ProxyPushConsumer_ptr tmp_consumer;
    while (not addedConsumer) {
        instance_counter++;
        std::ostringstream instance_str;
        instance_str<<instance_counter;
        tmp_consumer = parent->extendConsumers(instance_str.str());
        if (!CORBA::is_nil(tmp_consumer))
            addedConsumer = true;
    }
    return tmp_consumer;
};
        
CosEventChannelAdmin::ProxyPullConsumer_ptr SupplierAdmin_i::obtain_pull_consumer() {
    return CosEventChannelAdmin::ProxyPullConsumer::_nil();
};
    
MessageConsumerPort::MessageConsumerPort(std::string port_name) : Port_Provides_base_impl(port_name) {
    supplier_admin = new SupplierAdmin_i(this);
}

    // CF::Port methods
void MessageConsumerPort::connectPort(CORBA::Object_ptr connection, const char* connectionId) {
    CosEventChannelAdmin::EventChannel_var channel = ossie::corba::_narrowSafe<CosEventChannelAdmin::EventChannel>(connection);
    if (CORBA::is_nil(channel)) {
        throw CF::Port::InvalidPort(0, "The object provided did not narrow to a CosEventChannelAdmin::EventChannel type");
    }
        
    CosEventChannelAdmin::ConsumerAdmin_var consumer_admin = channel->for_consumers();
    CosEventChannelAdmin::ProxyPushSupplier_var proxy_supplier = consumer_admin->obtain_push_supplier();
    addSupplier(connectionId, proxy_supplier);
    CosEventChannelAdmin::ProxyPushConsumer_var tmp_consumer = extendConsumers(connectionId);
    proxy_supplier->connect_push_consumer(tmp_consumer);
    
};

void MessageConsumerPort::disconnectPort(const char* connectionId) {
    CosEventComm::PushSupplier_var supplier = removeSupplier(connectionId);
    if (CORBA::is_nil(supplier)) {
        return;
    }
    supplier->disconnect_push_supplier();
};
    
CosEventChannelAdmin::ConsumerAdmin_ptr MessageConsumerPort::for_consumers() {
    return CosEventChannelAdmin::ConsumerAdmin::_nil();
};
    
CosEventChannelAdmin::SupplierAdmin_ptr MessageConsumerPort::for_suppliers() {
    return supplier_admin->_this();
};
    
void MessageConsumerPort::destroy() {
};

Consumer_i* MessageConsumerPort::removeConsumer(std::string consumer_id) {
    // whoever makes this call needs to deactivate the object and then delete it, otherwise
    //  you will have a memory leak. We should probably change the storage from a class
    //  pointer to a CORBA one so that we can user _var scope exit for memory management
    boost::mutex::scoped_lock lock(portInterfaceAccess);
    std::map<std::string, Consumer_i*>::iterator consumer = consumers.find(consumer_id);
    if (consumer == consumers.end()) {
        return NULL;
    }
    Consumer_i* Consumer_p = consumer->second;
    consumers.erase(consumer_id);
    return Consumer_p;
};
    
CosEventChannelAdmin::ProxyPushConsumer_ptr MessageConsumerPort::extendConsumers(std::string consumer_id) {
    boost::mutex::scoped_lock lock(portInterfaceAccess);
    consumers[std::string(consumer_id)] = new Consumer_i(this);
    return consumers[std::string(consumer_id)]->_this();
};
    
void MessageConsumerPort::addSupplier (const std::string& connectionId, CosEventComm::PushSupplier_ptr supplier) {
    boost::mutex::scoped_lock lock(portInterfaceAccess);
    suppliers_[connectionId] = CosEventComm::PushSupplier::_duplicate(supplier);
};

CosEventComm::PushSupplier_ptr MessageConsumerPort::removeSupplier (const std::string& connectionId) {
    boost::mutex::scoped_lock lock(portInterfaceAccess);
    SupplierTable::iterator iter = suppliers_.find(connectionId);
    if (iter == suppliers_.end()) {
        return CosEventComm::PushSupplier::_nil();
    }
    CosEventComm::PushSupplier_var supplier = iter->second;
    suppliers_.erase(iter);
    return supplier._retn();
};

void MessageConsumerPort::fireCallback (const std::string& id, const CORBA::Any& data) {
    CallbackTable::iterator callback = callbacks_.find(id);
    if (callback != callbacks_.end()) {
        (*callback->second)(id, data);
    } else {
        if (generic_callbacks_.empty()) {
            std::string warning = "no callbacks registered for messages with id: "+id+".";
        
            if (callbacks_.size() == 0) {
                warning += " No callbacks are registered";
            } else if (callbacks_.size() == 1) {
                warning += " The only registered callback is for message with id: "+callbacks_.begin()->first;
            } else { 
                warning += " The available message callbacks are for messages with any of the following id: ";
                for (callback = callbacks_.begin();callback != callbacks_.end(); callback++) {
                    warning += callback->first+" ";
                }
            }
            LOG_WARN(MessageConsumerPort,warning);
        }
    }

    // Invoke the callback for those messages that are generic
    generic_callbacks_(id, data);
};

std::string MessageConsumerPort::getRepid() const 
{
	return "IDL:ExtendedEvent/MessageEvent:1.0";
}

std::string MessageConsumerPort::getDirection() const 
{
	return "Bidir";
}

MessageSupplierPort::MessageSupplierPort (std::string port_name) :
    Port_Uses_base_impl(port_name)
{
}

MessageSupplierPort::~MessageSupplierPort (void)
{
}

void MessageSupplierPort::connectPort(CORBA::Object_ptr connection, const char* connectionId)
{
    boost::mutex::scoped_lock lock(portInterfaceAccess);
    this->active = true;
    CosEventChannelAdmin::EventChannel_var channel = ossie::corba::_narrowSafe<CosEventChannelAdmin::EventChannel>(connection);
    if (CORBA::is_nil(channel)) {
        throw CF::Port::InvalidPort(0, "The object provided did not narrow to a CosEventChannelAdmin::EventChannel type");
    }
    CosEventChannelAdmin::SupplierAdmin_var supplier_admin = channel->for_suppliers();
    CosEventChannelAdmin::ProxyPushConsumer_ptr proxy_consumer = supplier_admin->obtain_push_consumer();
    proxy_consumer->connect_push_supplier(CosEventComm::PushSupplier::_nil());
    extendConsumers(connectionId, proxy_consumer);
}

void MessageSupplierPort::disconnectPort(const char* connectionId)
{
    boost::mutex::scoped_lock lock(portInterfaceAccess);
    CosEventChannelAdmin::ProxyPushConsumer_var consumer = removeConsumer(connectionId);
    if (CORBA::is_nil(consumer)) {
        return;
    }
    consumer->disconnect_push_consumer();
    if (this->consumers.empty()) {
        this->active = false;
    }
}

void MessageSupplierPort::push(const CORBA::Any& data)
{
    boost::mutex::scoped_lock lock(portInterfaceAccess);
    std::map<std::string, CosEventChannelAdmin::ProxyPushConsumer_var>::iterator connection = consumers.begin();
    while (connection != consumers.end()) {
        try {
            (connection->second)->push(data);
        } catch ( ... ) {
        }
        connection++;
    }
}

CosEventChannelAdmin::ProxyPushConsumer_ptr MessageSupplierPort::removeConsumer(std::string consumer_id)
{
    std::map<std::string, CosEventChannelAdmin::ProxyPushConsumer_var>::iterator connection = consumers.find(consumer_id);
    if (connection == consumers.end()) {
        return CosEventChannelAdmin::ProxyPushConsumer::_nil();
    }
    CosEventChannelAdmin::ProxyPushConsumer_var consumer = connection->second;
    consumers.erase(connection);
    return consumer._retn();
}

void MessageSupplierPort::extendConsumers(std::string consumer_id, CosEventChannelAdmin::ProxyPushConsumer_ptr proxy_consumer)
{
    consumers[std::string(consumer_id)] = proxy_consumer;
}

std::string MessageSupplierPort::getRepid() const 
{
	return "IDL:ExtendedEvent/MessageEvent:1.0";
}
