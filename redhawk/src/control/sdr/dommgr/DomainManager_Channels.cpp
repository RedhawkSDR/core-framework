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

#include <string>
#include <vector>

#include <ossie/debug.h>
#include <ossie/CorbaUtils.h>
#include <ossie/EventChannelSupport.h>

#include "DomainManager_impl.h"

using namespace ossie;

class IDM_Channel_Consumer_i: public virtual POA_CosEventComm::PushConsumer
{
public:
    IDM_Channel_Consumer_i (DomainManager_impl *_dmn)
    {
        TRACE_ENTER(DomainManager_impl);
        _dmnMgr = _dmn;
        TRACE_EXIT(DomainManager_impl);
    }


    void push (const CORBA::Any& _any) throw (CORBA::SystemException, CosEventComm::Disconnected)
    {
        TRACE_ENTER(DomainManager_impl);
        CORBA::ULong tmp = 0;
        float tmp2 = 0.0;
        _any >>= tmp;
        _any >>= tmp2;

        TRACE_EXIT(DomainManager_impl);
    }


    void disconnect_push_consumer () throw (CORBA::SystemException)
    {
        TRACE_ENTER(DomainManager_impl);
        TRACE_EXIT(DomainManager_impl);
    }

private:
    DomainManager_impl * _dmnMgr;

};

class ODM_Channel_Supplier_i : public virtual POA_CosEventComm::PushSupplier
{
public:
    ODM_Channel_Supplier_i (DomainManager_impl *_dmn)
    {
        TRACE_ENTER(DomainManager_impl);
        _dmnMgr = _dmn;
        TRACE_EXIT(DomainManager_impl);
    }

    void disconnect_push_supplier ()
    {
        TRACE_ENTER(DomainManager_impl);
        TRACE_EXIT(DomainManager_impl);
    }

private:
    DomainManager_impl * _dmnMgr;
};


CosEventChannelAdmin::EventChannel_ptr DomainManager_impl::createEventChannel (const std::string& name)
{
    TRACE_ENTER(DomainManager_impl);

    // Try to connect to the event channel if it already exists. If it doesn't, or it's in the
    // NameService but non-existent (in the CORBA sense), recreate it.
    CosEventChannelAdmin::EventChannel_var eventChannel = ossie::event::connectToEventChannel(rootContext, name);
    if (!ossie::corba::objectExists(eventChannel)) {
        // Give the channel a unique (one per domain) name, which is used by the EventService
        // to generate a persistent object ID. This is independent of the name bound to the
        // NameService below.
        eventChannel = ossie::event::createEventChannel(_domainName + "." + name);

        if (!CORBA::is_nil(eventChannel)) {
            LOG_TRACE(DomainManager_impl, "Binding event channel " << name << " into domain naming context");
            CosNaming::Name_var cosName = ossie::corba::stringToName(name);
            try {
                // Use rebind to force the naming service to replace any existing object with the same name.
                rootContext->rebind(cosName, eventChannel.in());
            } catch ( ... ) {
                LOG_WARN(DomainManager_impl, "Failed to bind event channel " << name << " to naming context");
                eventChannel->destroy();
                eventChannel = CosEventChannelAdmin::EventChannel::_nil();
            }
        }
    }
    if (ossie::corba::objectExists(eventChannel)) {
        bool channelAlreadyInList = false;
        std::vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();
        std::string tmpName = _domainName + "." + name;
        while (_iter != _eventChannels.end()) {
            if ((*_iter).name == tmpName) {
                channelAlreadyInList = true;
                break;
            }
            _iter++;
        }
        if (not channelAlreadyInList) {
            ossie::EventChannelNode tmpNode;
            tmpNode.connectionCount = 0;
            tmpNode.name = tmpName;
            tmpNode.boundName = name;
            tmpNode.channel = CosEventChannelAdmin::EventChannel::_duplicate(eventChannel);
            _eventChannels.push_back(tmpNode);
        }
    }

    try {
        db.store("EVENT_CHANNELS", _eventChannels);
    } catch ( ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting change to event channels");
    }
    TRACE_EXIT(DomainManager_impl);
    return eventChannel._retn();
}


void DomainManager_impl::destroyEventChannel (const std::string& name)
{
    TRACE_ENTER(DomainManager_impl);

    std::vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();

    while (_iter != _eventChannels.end()) {
        if ((*_iter).name == name) {
            break;
        }
        _iter++;
    }
    if (_iter == _eventChannels.end()) {
        return;
    }
    CosNaming::Name_var channel_name = ossie::corba::stringToName((*_iter).boundName);
    rootContext->unbind(channel_name);
    (*_iter).channel->destroy();
    (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();

    _eventChannels.erase(_iter);

    try {
        db.store("EVENT_CHANNELS", _eventChannels);
    } catch ( ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting change to event channels");
    }

    TRACE_EXIT(DomainManager_impl);
}


void DomainManager_impl::createEventChannels (void)
{
    TRACE_ENTER(DomainManager_impl);

    std::string nameODMChannel = "ODM_Channel";
    std::string nameIDMChannel = "IDM_Channel";
    
    CosEventChannelAdmin::EventChannel_var ODM_channel;
    CosEventChannelAdmin::EventChannel_var IDM_channel;

    // Check for existing event channels.
    try {
        ODM_channel = ossie::event::connectToEventChannel(rootContext, _domainName + "." + nameODMChannel);
    } catch (...) {
        LOG_WARN(DomainManager_impl, "Ignoring unexpected exception checking for existing ODM channel");
    }
    if (!CORBA::is_nil(ODM_channel)) {
        ossie::EventChannelNode tmpNode;
        tmpNode.connectionCount = 0;
        tmpNode.name = _domainName + "." + nameODMChannel;
        _eventChannels.push_back(tmpNode);
        (*(_eventChannels.end()-1)).channel = CosEventChannelAdmin::EventChannel::_duplicate(ODM_channel);
    }
    try {
        IDM_channel = ossie::event::connectToEventChannel(rootContext, _domainName + "." + nameIDMChannel);
    } catch (...) {
        LOG_WARN(DomainManager_impl, "Ignoring unexpected exception checking for existing IDM channel");
    }
    if (!CORBA::is_nil(IDM_channel)) {
        ossie::EventChannelNode tmpNode;
        tmpNode.connectionCount = 0;
        tmpNode.name = _domainName + "." + nameIDMChannel;
        _eventChannels.push_back(tmpNode);
        (*(_eventChannels.end()-1)).channel = CosEventChannelAdmin::EventChannel::_duplicate(IDM_channel);
    }

    // If both event channels already existed, return early.
    if (!CORBA::is_nil(ODM_channel) && !CORBA::is_nil(IDM_channel)) {
        LOG_DEBUG(DomainManager_impl, "Reconnected to existing event channels");
        TRACE_EXIT(DomainManager_impl);
        return;
    }

    // Try to create the ODM channel if it was not found.
    if (CORBA::is_nil(ODM_channel)) {
        ODM_channel = createEventChannel(nameODMChannel);
    }

    // Try to create the IDM channel if it was not found.
    if (CORBA::is_nil(IDM_channel)) {
        IDM_channel = createEventChannel(nameIDMChannel);
    }

    TRACE_EXIT(DomainManager_impl);
}

void DomainManager_impl::destroyEventChannels()
{
    std::vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();

    while (_iter != _eventChannels.end()) {
        try {
            CosNaming::Name_var channel_name = ossie::corba::stringToName((*_iter).boundName);
            rootContext->unbind(channel_name);
            (*_iter).channel->destroy();
            (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
        } catch ( ... ) {
        }
        _eventChannels.erase(_iter);
        _iter = _eventChannels.begin();
    }
}


void DomainManager_impl::connectToOutgoingEventChannel (void)
{
    std::string nameODMChannel = _domainName+".ODM_Channel";

    std::vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();

    while (_iter != _eventChannels.end()) {
        if ((*_iter).name == nameODMChannel) {
            break;
        }
        _iter++;
    }

    if (_iter == _eventChannels.end()) {
        return;
    }

    CosEventChannelAdmin::SupplierAdmin_var supplier_admin;
    unsigned int number_tries;
    unsigned int maximum_tries = 10;

    number_tries = 0;
    while (true)
    {
        try {
            supplier_admin = (*_iter).channel->for_suppliers ();
            if (CORBA::is_nil(supplier_admin))
            {
                (*_iter).channel->destroy();
                (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
                _eventChannels.erase(_iter);
                return;
            }
            break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
            if (number_tries == maximum_tries) {
                (*_iter).channel->destroy();
                (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
                _eventChannels.erase(_iter);
                return;
            }
            usleep(1000);   // wait 1 ms
            number_tries++;
            continue;
        }
    }

    proxy_consumer = CosEventChannelAdmin::ProxyPushConsumer::_nil();
    number_tries = 0;
    while (true)
    {
        try {
            proxy_consumer = supplier_admin->obtain_push_consumer ();
            if (CORBA::is_nil(proxy_consumer))
            {
                (*_iter).channel->destroy();
                (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
                _eventChannels.erase(_iter);
                return;
            }
            break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
            if (number_tries == maximum_tries) {
                (*_iter).channel->destroy();
                (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
                _eventChannels.erase(_iter);
                return;
            }
            usleep(1000);   // wait 1 ms
            number_tries++;
            continue;
        }
    }

    //
    // Connect Push Supplier - retrying on Comms Failure.
    ODM_Channel_Supplier_i* supplier_servant = new ODM_Channel_Supplier_i(this);
    PortableServer::POA_var event_poa = poa->find_POA("EventChannels", 1);
    PortableServer::ObjectId_var oid = event_poa->activate_object(supplier_servant);
    CosEventComm::PushSupplier_var sptr = supplier_servant->_this();
    supplier_servant->_remove_ref();
    number_tries = 0;
    while (true)
    {
        try {
            proxy_consumer->connect_push_supplier(sptr.in());
            break;
        }
        catch (CORBA::BAD_PARAM& ex) {
            (*_iter).channel->destroy();
            (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
            _eventChannels.erase(_iter);
            return;
        }
        catch (CosEventChannelAdmin::AlreadyConnected& ex) {
            break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
            if (number_tries == maximum_tries) {
                (*_iter).channel->destroy();
                (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
                _eventChannels.erase(_iter);
                return;
            }
            usleep(1000);   // wait 1 ms
            number_tries++;
            continue;
        }
    }
}


void DomainManager_impl::connectToIncomingEventChannel (void)
{
    std::string nameIDMChannel = _domainName+".IDM_Channel";

    std::vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();

    while (_iter != _eventChannels.end()) {
        if ((*_iter).name == nameIDMChannel) {
            break;
        }
        _iter++;
    }

    if (_iter == _eventChannels.end()) {
        return;
    }

    CosEventChannelAdmin::ConsumerAdmin_var consumer_admin;
    unsigned int number_tries;
    unsigned int maximum_tries = 10;

    number_tries = 0;
    while (true)
    {
        try {
            consumer_admin = (*_iter).channel->for_consumers ();
            if (CORBA::is_nil (consumer_admin))
            {
                (*_iter).channel->destroy();
                (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
                _eventChannels.erase(_iter);
                return;
            }
            break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
            if (number_tries == maximum_tries) {
                (*_iter).channel->destroy();
                (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
                _eventChannels.erase(_iter);
                return;
            }
            usleep(1000);   // wait 1 ms
            number_tries++;
            continue;
        }
    }

    number_tries = 0;
    CosEventChannelAdmin::ProxyPushSupplier_var proxy_supplier;
    while (true)
    {
        try {
            proxy_supplier = consumer_admin->obtain_push_supplier ();
            if (CORBA::is_nil (proxy_supplier))
            {
                (*_iter).channel->destroy();
                (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
                _eventChannels.erase(_iter);
                return;
            }
            break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
            if (number_tries == maximum_tries) {
                (*_iter).channel->destroy();
                (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
                _eventChannels.erase(_iter);
                return;
            }
            usleep(1000);   // wait 1 ms
            number_tries++;
            continue;
        }
    }

    number_tries = 0;
    IDM_Channel_Consumer_i* consumer_servant = new IDM_Channel_Consumer_i(this);
    PortableServer::POA_var event_poa = poa->find_POA("EventChannels", 1);
    PortableServer::ObjectId_var oid = event_poa->activate_object(consumer_servant);
    CosEventComm::PushConsumer_var consumer = consumer_servant->_this();
    consumer_servant->_remove_ref();
    while (true)
    {
        try {
            proxy_supplier->connect_push_consumer(consumer);
            break;
        }
        catch (CORBA::BAD_PARAM& ex) {
            (*_iter).channel->destroy();
            (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
            _eventChannels.erase(_iter);
            return;
        }
        catch (CosEventChannelAdmin::AlreadyConnected& ex) {
            break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
            if (number_tries == maximum_tries) {
                (*_iter).channel->destroy();
                (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
                _eventChannels.erase(_iter);
                return;
            }
            usleep(1000);   // wait 1 ms
            number_tries++;
            continue;
        }
    }
}
