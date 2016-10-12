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


#include "ossie/debug.h"
#include "ossie/CorbaUtils.h"
#include "ossie/EventChannelSupport.h"

#include <iostream>

namespace ossie {
void sendStateChangeEvent(log4cxx::LoggerPtr logger, const char* producerId, const char* sourceId,
    StandardEvent::StateChangeCategoryType stateChangeCategory, 
    StandardEvent::StateChangeType stateChangeFrom, 
    StandardEvent::StateChangeType stateChangeTo,
    CosEventChannelAdmin::ProxyPushConsumer_ptr _proxy_consumer)
{
    if (_proxy_consumer != CosEventChannelAdmin::ProxyPushConsumer::_nil()) {
        CORBA::Any outboundMessage;
        StandardEvent::StateChangeEventType new_message;
        new_message.producerId = CORBA::string_dup(producerId);
        new_message.sourceId = CORBA::string_dup(sourceId);
        new_message.stateChangeCategory = stateChangeCategory;
        new_message.stateChangeFrom = stateChangeFrom;
        new_message.stateChangeTo = stateChangeTo;
        outboundMessage <<= new_message;
        try {
            _proxy_consumer->push(outboundMessage);
        } catch ( ... ) {
            LOG4CXX_WARN(logger, "Unable to send the following StateChangeEvent (the event service might not be running):\n  producer id:"<<producerId<<" source id:"<<sourceId<<" state change category:"<<stateChangeCategory<<" state change from/to: "<<stateChangeFrom<<"/"<<stateChangeTo);
        }
    }
}

void sendObjectAddedEvent(log4cxx::LoggerPtr logger, const char* producerId, const char* sourceId, const char* sourceName, 
    CORBA::Object_ptr sourceIOR, StandardEvent::SourceCategoryType sourceCategory, 
    CosEventChannelAdmin::ProxyPushConsumer_ptr _proxy_consumer)
{
    if (_proxy_consumer != CosEventChannelAdmin::ProxyPushConsumer::_nil()) {
        CORBA::Any outboundMessage;
        StandardEvent::DomainManagementObjectAddedEventType new_message;
        new_message.producerId = CORBA::string_dup(producerId);
        new_message.sourceId = CORBA::string_dup(sourceId);
        new_message.sourceName = CORBA::string_dup(sourceName);
        new_message.sourceCategory = sourceCategory;
        new_message.sourceIOR = CORBA::Object::_duplicate(sourceIOR);
        outboundMessage <<= new_message;
        try {
            _proxy_consumer->push(outboundMessage);
        } catch ( ... ) {
            LOG4CXX_WARN(logger, "Unable to send the following ObjectAddedEvent (the event service might not be running):\n  producer id:"<<producerId<<" source id:"<<sourceId<<" source name:"<<sourceName<<" source category: "<<sourceCategory);
        }
    }
}

void sendObjectRemovedEvent(log4cxx::LoggerPtr logger, const char* producerId, const char* sourceId, const char* sourceName, 
    StandardEvent::SourceCategoryType sourceCategory, CosEventChannelAdmin::ProxyPushConsumer_ptr _proxy_consumer)
{
    if (_proxy_consumer != CosEventChannelAdmin::ProxyPushConsumer::_nil()) {
        CORBA::Any outboundMessage;
        StandardEvent::DomainManagementObjectRemovedEventType new_message;
        new_message.producerId = CORBA::string_dup(producerId);
        new_message.sourceId = CORBA::string_dup(sourceId);
        new_message.sourceName = CORBA::string_dup(sourceName);
        new_message.sourceCategory = sourceCategory;
        outboundMessage <<= new_message;
        try {
            _proxy_consumer->push(outboundMessage);
        } catch ( ... ) {
            LOG4CXX_WARN(logger, "Unable to send the following ObjectRemovedEvent (the event service might not be running):\n  producer id:"<<producerId<<" source id:"<<sourceId<<" source name:"<<sourceName<<" source category: "<<sourceCategory);
        }
    }
}


namespace event {

CREATE_LOGGER(EventChannelSupport);

CosLifeCycle::GenericFactory_ptr getEventChannelFactory ()
{
    TRACE_ENTER(EventChannelSupport);

    // Attempt to locate the OmniEvents event channel factory to create the event channels.
    // First, check for an initial reference in the omniORB configuration; if it cannot be
    // resolved in this manner, look it up via the naming service.
    LOG_TRACE(EventChannelSupport, "Locating EventChannelFactory via initial reference");
    CORBA::Object_var factoryObj;
    try {
        factoryObj = ossie::corba::Orb()->resolve_initial_references("EventService");
    } catch (const CORBA::ORB::InvalidName&) {
        LOG_TRACE(EventChannelSupport, "No initial reference for 'EventService'");
    } catch (const CORBA::Exception& e) {
        LOG_WARN(EventChannelSupport, "CORBA " << e._name() << " exception locating EventChannelFactory via initial reference");
    }

    if (CORBA::is_nil(factoryObj)) {
        LOG_TRACE(EventChannelSupport, "Looking up EventChannelFactory in NameService");
        CosNaming::Name_var factoryName = ossie::corba::stringToName("EventChannelFactory");

        try {
            factoryObj = ossie::corba::InitialNamingContext()->resolve(factoryName);
        } catch (const CosNaming::NamingContext::NotFound&) {
            LOG_TRACE(EventChannelSupport, "No naming service entry for 'EventChannelFactory'");
        } catch (const CORBA::Exception& e) {
            LOG_WARN(EventChannelSupport, "CORBA " << e._name() << " exception looking up EventChannelFactory in name service");
        }
    }

    CosLifeCycle::GenericFactory_var factory;
    if (!CORBA::is_nil(factoryObj)) {
        try {
            if (!factoryObj->_non_existent()) {
                factory = CosLifeCycle::GenericFactory::_narrow(factoryObj);
            }
        } catch (const CORBA::TRANSIENT&) {
            LOG_WARN(EventChannelSupport, "Could not contact EventChannelFactory");
        } 
    }
    TRACE_EXIT(EventChannelSupport);
    return factory._retn();
}

CosEventChannelAdmin::EventChannel_ptr connectToEventChannel (CosNaming::NamingContext_ptr context, const std::string& name)
{
    TRACE_ENTER(EventChannelSupport);
    CosNaming::Name_var boundName = ossie::corba::stringToName(name);
    try {
        CORBA::Object_var obj = context->resolve(boundName);
        LOG_TRACE(EventChannelSupport, "Existing event channel " << name << " found");
        return CosEventChannelAdmin::EventChannel::_narrow(obj);
    } catch (const CosNaming::NamingContext::NotFound&) {
        // The channel does not exist and can be safely created.
    } catch (const CORBA::Exception& e) {
        LOG_WARN(EventChannelSupport, "CORBA (" << e._name() << ") exception connecting to event channel " << name <<". Continue without connecting to the channel (the event service might not be running)");
    }

    TRACE_EXIT(EventChannelSupport);
    return CosEventChannelAdmin::EventChannel::_nil();
}

CosEventChannelAdmin::EventChannel_ptr createEventChannel (const std::string& name)
{
    TRACE_ENTER(EventChannelSupport);

    CosLifeCycle::GenericFactory_var factory = getEventChannelFactory();
    if (CORBA::is_nil(factory)) {
        LOG_WARN(EventChannelSupport, "Event channel " << name << " not created");
        TRACE_EXIT(EventChannelSupport);
        return CosEventChannelAdmin::EventChannel::_nil();
    }

    CosLifeCycle::Key key;
    key.length(1);
    key[0].id = "EventChannel";
    key[0].kind = "object interface";

    std::string insName = name;
    CosLifeCycle::Criteria criteria;
    criteria.length(1);
    criteria[0].name = "InsName";
    criteria[0].value <<= insName.c_str();

    CosEventChannelAdmin::EventChannel_var eventChannel;

    LOG_TRACE(EventChannelSupport, "Creating event channel " << name);
    try {
        CORBA::Object_var obj = factory->create_object(key, criteria);
        eventChannel = CosEventChannelAdmin::EventChannel::_narrow(obj);
    } catch (const CosLifeCycle::InvalidCriteria&) {
        LOG_WARN(EventChannelSupport, "Invalid Criteria for creating event channel " << name);
    } catch (const CORBA::Exception& ex) {
        LOG_WARN(EventChannelSupport, "CORBA " << ex._name() << " exception creating event channel " << name);
    }

    TRACE_EXIT(EventChannelSupport);
    return eventChannel._retn();
}

}
}
