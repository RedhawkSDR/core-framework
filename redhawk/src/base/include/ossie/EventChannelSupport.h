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

#ifndef OSSIE_EVENTCHANNELSUPPORT_H
#define OSSIE_EVENTCHANNELSUPPORT_H

#if ENABLE_EVENTS

#include <omniORB4/CORBA.h>

#include <COS/CosEventChannelAdmin.hh>
#include <COS/CosLifeCycle.hh>

#include "CF/StandardEvent.h"

#ifndef HAVE_LOG4CXX

class log4cxx
{
    public:
        static std::string __logger;
        typedef std::string LoggerPtr;
};

#define LOG4CXX_WARN(classname, expression) ;

#endif

namespace ossie {

    void sendStateChangeEvent(log4cxx::LoggerPtr logger, const char* producerId, const char* sourceId,
        StandardEvent::StateChangeCategoryType stateChangeCategory, 
        StandardEvent::StateChangeType stateChangeFrom, 
        StandardEvent::StateChangeType stateChangeTo,
        CosEventChannelAdmin::ProxyPushConsumer_ptr _proxy_consumer);

    void sendObjectAddedEvent(log4cxx::LoggerPtr logger, const char* producerId, const char* sourceId, const char* sourceName, 
        CORBA::Object_ptr sourceIOR, StandardEvent::SourceCategoryType sourceCategory, 
        CosEventChannelAdmin::ProxyPushConsumer_ptr _proxy_consumer);

    void sendObjectRemovedEvent(log4cxx::LoggerPtr logger, const char* producerId, const char* sourceId, const char* sourceName, 
        StandardEvent::SourceCategoryType sourceCategory, CosEventChannelAdmin::ProxyPushConsumer_ptr _proxy_consumer);

    namespace event {
        CosEventChannelAdmin::EventChannel_ptr connectToEventChannel (CosNaming::NamingContext_ptr context, const std::string& name);
        CosEventChannelAdmin::EventChannel_ptr createEventChannel (const std::string& name);
        CosLifeCycle::GenericFactory_ptr getEventChannelFactory ();
    }
}

#endif

#endif // OSSIE_EVENTCHANNELSUPPORT_H
