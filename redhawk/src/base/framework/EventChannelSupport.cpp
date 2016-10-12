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
#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <iostream>
#include <cstdio>
#include <cstddef>
#include <sstream>
#include <vector>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <omniORB4/omniURI.h>
#include "ossie/debug.h"
#include "ossie/EventChannelSupport.h"


namespace ossie {
void sendStateChangeEvent( LOGGER logger, const char* producerId, const char* sourceId,
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
            RH_WARN(logger, "Unable to send the following StateChangeEvent (the event service might not be running):\n  producer id:"<<producerId<<" source id:"<<sourceId<<" state change category:"<<stateChangeCategory<<" state change from/to: "<<stateChangeFrom<<"/"<<stateChangeTo);
        }
    }
}

void sendObjectAddedEvent( LOGGER logger, const char* producerId, const char* sourceId, const char* sourceName, 
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
            RH_WARN(logger, "Unable to send the following ObjectAddedEvent (the event service might not be running):\n  producer id:"<<producerId<<" source id:"<<sourceId<<" source name:"<<sourceName<<" source category: "<<sourceCategory);
        }
    }
}

void sendObjectRemovedEvent( LOGGER logger, const char* producerId, const char* sourceId, const char* sourceName, 
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
            RH_WARN(logger, "Unable to send the following ObjectRemovedEvent (the event service might not be running):\n  producer id:"<<producerId<<" source id:"<<sourceId<<" source name:"<<sourceName<<" source category: "<<sourceCategory);
        }
    }
}


namespace event {

CREATE_LOGGER(EventChannelSupport);

#define LNTRACE( lname, expression ) RH_TRACE( rh_logger::Logger::getLogger(lname), expression )
#define LNDEBUG( lname, expression ) RH_DEBUG( rh_logger::Logger::getLogger(lname), expression )
#define LNINFO( lname, expression )  RH_INFO( rh_logger::Logger::getLogger(lname), expression )
#define LNWARN( lname, expression )  RH_WARN( rh_logger::Logger::getLogger(lname), expression )
#define LNERROR( lname, expression ) RH_ERROR( rh_logger::Logger::getLogger(lname), expression )
#define LNFATAL( lname, expression ) RH_FATAL( rh_logger::Logger::getLogger(lname), expression )

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

  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////

   struct OrbContext;
   typedef OrbContext*  OrbPtr;
 
    //
    // Orb
    //
    // Context for access to ORB and common CORBA services
    //
    struct OrbContext {

      // orb instantiation
      CORBA::ORB_ptr                          orb;

      // root POA for to handle object requests
      PortableServer::POA_ptr                 rootPOA;

      // handle to naming service
      CosNaming::NamingContext_ptr            namingService;

      // handle to naming service
      CosNaming::NamingContextExt_ptr         namingServiceCtx;

      virtual ~OrbContext() {};

      OrbContext() {
	orb = ossie::corba::Orb();
	rootPOA = ossie::corba::RootPOA();
	namingService = ossie::corba::InitialNamingContext();
	namingServiceCtx = CosNaming::NamingContextExt::_nil();
	try {
	  CORBA::Object_ptr obj;
	  obj=orb->resolve_initial_references("NameService");
	  namingServiceCtx = CosNaming::NamingContextExt::_narrow(obj);
	}
	catch(...){
	};
      };

    };


  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////

  //
  // EventChannel Convenience Methods
  //

  //
  // (Taken from eventc.cc)
  //
  //
/**
   This requires libomniEvent.so....

  omniEvents::EventChannelFactory_ptr GetEventChannelFactory ( corba::OrbPtr &orb ) {
    
    CORBA::Object_var ecf_obj;
    omniEvents::EventChannelFactory_var ecf = omniEvents::EventChannelFactory::_nil();
    LNDEBUG( "GetEventChannelFactory", " Look up EventChannelFactory" );
    try {
      //ecf_obj = orb.namingServiceCtx->resolve_str(str2name("EventChannelFactory"));
      ecf_obj = orb->namingServiceCtx->resolve_str("EventChannelFactory");
      if (!CORBA::is_nil(ecf_obj)) {
	LNDEBUG( "GetEventChannelFactory",  " Narrow object to EventChannelFactory" );
	ecf = omniEvents::EventChannelFactory::_narrow(ecf_obj);
	LNDEBUG( "GetEventChannelFactory",  " Narrowed to ... EventChannelFactory" );
      }
    } catch (const CosNaming::NamingContext::NotFound&) {
      LNWARN( "GetEventChannelFactory", " No naming service entry for 'EventChannelFactory'" );
    } catch (const CORBA::Exception& e) {
      LNWARN( "GetEventChannelFactory",  " CORBA " << e._name() << ", exception looking up EventChannelFactory." );
    } 
    return ecf._retn();

  }

  CosLifeCycle::GenericFactory_ptr GetEventChannelFactory ( ) {
    
    Orb _orb;
    OrbPtr orb=&_orb;

    CORBA::Object_var ecf_obj;
    //omniEvents::EventChannelFactory_var ecf = omniEvents::EventChannelFactory::_nil();
    CosLifeCycle::GenericFactory_var  ecf;
    LNDEBUG( "GetEventChannelFactory", " Look up EventChannelFactory" );
    try {
      //ecf_obj = orb.namingServiceCtx->resolve_str(str2name("EventChannelFactory"));
      ecf_obj = orb->namingServiceCtx->resolve_str("EventChannelFactory");
      if (!CORBA::is_nil(ecf_obj)) {
	LNDEBUG( "GetEventChannelFactory",  " Narrow object to EventChannelFactory" );
	ecf = CosLifeCycle::GenericFactory::_narrow(ecf_obj);
	//ecf = omniEvents::EventChannelFactory::_narrow(ecf_obj);
	LNDEBUG( "GetEventChannelFactory",  " Narrowed to ... EventChannelFactory" );
      }
    } catch (const CosNaming::NamingContext::NotFound&) {
      LNWARN( "GetEventChannelFactory", " No naming service entry for 'EventChannelFactory'" );
    } catch (const CORBA::Exception& e) {
      LNWARN( "GetEventChannelFactory",  " CORBA " << e._name() << ", exception looking up EventChannelFactory." );
    } 
    return ecf._retn();

  }
**/

  //
  // GetEventChannel
  //
  // Will first lookup an event channel given the value of the name parameter... it will try to resolve the
  // name using different event channel resolution methods:
  // -) resolve if channel defined with InitRef method and resolve_initial_reference method
  // -) resolve as corbaname   corbaname::#channelname
  // -) resolve with  corbaloc
  //
  // If channel was not found and create==true then create the channel from the EventChannelFactory
  //
  CosEventChannelAdmin::EventChannel_ptr GetEventChannel ( const std::string& name, 
							   const bool create,
							   const std::string &host ) {
    LNDEBUG("GetEventChannel",  " : NamingService look up, Channel " << name );

    OrbContext _orb;
    OrbPtr     orb=&_orb;

    // return value if no event channel was found or error occured
    CosEventChannelAdmin::EventChannel_var event_channel = CosEventChannelAdmin::EventChannel::_nil();

    //
    // Look up event channel
    //   if no channel is found then try to lookup using InitRef
    //   if no channel is found then try to lookup using corbaname method
    //   if no channel is found then try to lookup using corbaloc method.
    // 
    // if all options fail then return nil if create== false
    //

    bool found=false;
    std::string tname;
    std::string nc_name("");

    //
    // try to resolve using channel name as InitRef  and resolve_initial_references
    //
    try {
      if ( found == false ) {
	LNDEBUG( "GetEventChannel", " : Trying InitRef Lookup " << name );
	CORBA::Object_var obj = orb->orb->resolve_initial_references(name.c_str());
	event_channel = CosEventChannelAdmin::EventChannel::_narrow(obj);
	found =true;
	LNDEBUG( "GetEventChannel", " : FOUND EXISTING, Channel " << name );
      } 
    }catch (const CORBA::Exception& e) {
      LNWARN( "GetEventChannel", "  Unable to lookup with InitRef:" << name << ",  CORBA RETURNED(" << e._name() << ")" );
    } 


    //
    // try to resolve with corbaname and string_to_object method
    //
    try {
      std::ostringstream os;
      if ( found == false ) {
	if ( name.find("corbaname") == std::string::npos ) {
	  if ( nc_name != "" )
	    os << "corbaname:rir:#"<< nc_name << "/" << name;
	  else
	    os << "corbaname:rir:#"<< name;
	}
	else
	  os << name;
	tname=os.str();
	LNDEBUG( "GetEventChannel", " : Trying corbaname resolution " << tname );
	CORBA::Object_var obj = obj=orb->orb->string_to_object(tname.c_str());
	event_channel = CosEventChannelAdmin::EventChannel::_narrow(obj);
	found =true;
	LNDEBUG( "GetEventChannel", " : FOUND EXISTING, Channel " << tname );
      } 

    }catch (const CORBA::Exception& e) {
      LNWARN( "GetEventChannel",  "  Unable to lookup with corbaname:  URI:" << tname << ",  CORBA RETURNED(" << e._name() << ")");
    }


    //
    // try to resolve with corbaloc method and string_to_object method
    //
    try {
      if ( found == false ) {	
	std::ostringstream os;
	//
	// last gasp... try the corbaloc method...corbaloc::host:11169/<channel name>
	// 
	os << "corbaloc::"<<host<<":11169/"<< name;
	tname=os.str();
	LNDEBUG( "GetEventChannel"," : Trying corbaloc resolution " << tname );
	CORBA::Object_var obj = orb->orb->string_to_object(tname.c_str());
	if ( !CORBA::is_nil(obj) ) {
	    event_channel = CosEventChannelAdmin::EventChannel::_narrow(obj);
	    found = true;
	    LNDEBUG( "GetEventChannel", " : FOUND EXISTING, Channel " << tname );
	}
	else {
	  LNDEBUG( "GetEventChannel", " : SEARCH FOR Channel " << tname << " FAILED");
	}
      } 
    }catch (const CORBA::Exception& e) {
      LNWARN( "GetEventChannel", "  Unable to lookup with corbaloc URI:" << tname << ", CORBA RETURNED(" << e._name() << ")" );
    }

    try{
      if ( !found && create ) {

	LNDEBUG( "GetEventChannel", " CREATE NEW CHANNEL " << name );
	event_channel = CreateEventChannel( name );
	if ( !CORBA::is_nil(event_channel) )
	  LNINFO( "GetEventChannel", " --- CREATED NEW CHANNEL ---" << name );
      }
    } catch (const CORBA::Exception& e) {
      LNERROR( "GetEventChannel", "  CORBA (" << e._name() << ") during event creation, channel " << name );
    }

    return event_channel._retn();
  }


  CosEventChannelAdmin::EventChannel_ptr GetEventChannel ( const std::string& name, 
							   const std::string &nc_name, 
							   const bool create,
							   const std::string &host )
  {

    // return value if no event channel was found or error occured
    CosEventChannelAdmin::EventChannel_var event_channel = CosEventChannelAdmin::EventChannel::_nil();
    // set orb context..
    OrbContext    _orb;
    OrbPtr orb = &_orb;
    //
    // Look up event channel in NamingService from root context...
    // if lookup fails then return nil if create== false,
    // else try and create a new EventChannel with name and nc_name 
    //

    bool found=false;
    std::string tname;

    try {
      //
      // Lookup in NamingService...
      //
      LNDEBUG("GetEventChannel",  " : NamingService look up, NC<"<<nc_name<<"> Channel " << name );
      std::string cname=name;
      if ( nc_name != "" )
	cname=nc_name+"/"+name;

      LNDEBUG("GetEventChannel",  " : NamingService look up : " << cname );
      CORBA::Object_var obj = orb->namingServiceCtx->resolve_str(cname.c_str());
      event_channel = CosEventChannelAdmin::EventChannel::_narrow(obj);

      LNDEBUG("GetEventChannel", " : FOUND EXISTING, Channel NC<"<<nc_name<<"> Channel " << name );
      found = true;
    } catch (const CosNaming::NamingContext::NotFound&) {
      LNWARN("GetEventChannel",  "  Unable to resolve event channel (" << name << ") in NamingService..." );
    } catch (const CORBA::Exception& e) {
      LNERROR("GetEventChannel", "  CORBA (" << e._name() << ") exception during event channel look up, CH:" << name );
    }


    try{
      if ( !found && create ) {

	LNDEBUG( "GetEventChannel", " CREATE NEW CHANNEL " << name );
	event_channel = CreateEventChannel( name, nc_name );
	if ( !CORBA::is_nil(event_channel) )
	  LNINFO( "GetEventChannel", " --- CREATED NEW CHANNEL ---" << name );
      }
    } catch (const CORBA::Exception& e) {
      LNERROR( "GetEventChannel", "  CORBA (" << e._name() << ") during event creation, channel " << name );
    }

    return event_channel._retn();

  }

  //
  // CreateEventChannel
  //
  // @param orb  context of the orb we are associated with 
  // @param name human readable path to the event channel being requested
  // @parm  bind bind the channel name to the object in the NamingService if channel was created
  //
  CosEventChannelAdmin::EventChannel_ptr CreateEventChannel (  const std::string& name, 
							      corba::NS_ACTION action  ) {
    return CreateEventChannel( name, "", action );
  }

  CosEventChannelAdmin::EventChannel_ptr CreateEventChannel ( const std::string& name, 
							      const std::string &nc_name, 
							      corba::NS_ACTION action  )
  {
    OrbContext  _orb;
    OrbPtr orb = &_orb;

    CosEventChannelAdmin::EventChannel_var event_channel = CosEventChannelAdmin::EventChannel::_nil();
    //omniEvents::EventChannelFactory_var event_channel_factory = GetEventChannelFactory( );
    CosLifeCycle::GenericFactory_var event_channel_factory = getEventChannelFactory( );
    LNDEBUG( "CreateEventChannel", " Request to create event channel:" << name << " bind action:" << action );

    if (CORBA::is_nil(event_channel_factory)) {
      LNERROR( "CreateEventChannel", "CHANNEL(CREATE): Could not find EventChannelFactory" );
      return event_channel._retn();
    }

    CosLifeCycle::Key key;
    key.length (1);
    key[0].id = CORBA::string_dup("EventChannel");
    key[0].kind = CORBA::string_dup("object interface");

    LNDEBUG( "CreateEventChannel", " action - event channel factory api" );
    if(!event_channel_factory->supports(key))
      {
	LNWARN( "CreateEventChannel", " EventChannelFactory does not support Event Channel Interface!" );
	return event_channel._retn();	
      }

    // 
    // Our EventChannels will always be created with InsName
    //
    LNINFO( "CreateEventChannel", " Request to create event channel:" << name.c_str() << " bind action:" << action );
    CosLifeCycle::Criteria criteria;
    criteria.length(2);
    criteria[0].name=CORBA::string_dup("InsName");
    criteria[0].value<<=name.c_str();
    criteria[1].name=CORBA::string_dup("CyclePeriod_ns");
    criteria[1].value<<=(CORBA::ULong)10;

    //
    // Create Event Channel Object.
    //
    LNDEBUG( "CreateEventChannel", " action - create EventChannel object" );

    CORBA::Object_var obj;
    try {
      obj =event_channel_factory->create_object(key, criteria);
    }
    catch (CosLifeCycle::CannotMeetCriteria& ex) /* create_object() */ {
      LNERROR( "CreateEventChannel", "create_object failed, channel: " << name << " reason: CannotMeetCriteria " );
    }
    catch (CosLifeCycle::InvalidCriteria& ex) /* create_object() */ {
      LNERROR( "CreateEventChannel", "create_object failed, channel: " << name << " reason: InvalidCriteria " );
      if(ex.invalid_criteria.length()>0) {
	int j;
	for (  j=0; (unsigned int)j < ex.invalid_criteria.length(); j++ ) { 
	  LNERROR( "CreateEventChannel", "--- Criteria Name: " << ex.invalid_criteria[j].name );
	  CORBA::ULong xx;
	  ex.invalid_criteria[j].value >>= xx;
	  LNERROR( "CreateEventChannel", "--- Criteria Value : " << xx );
	}
      }
    }
    catch( CORBA::Exception &ex ) {
      LNERROR( "CreateEventChannel", " create_object failed, channel:" << name << " reason: corba exception" );
    }

    if (CORBA::is_nil(obj)) {
      LNERROR( "CreateEventChannel", " Factory failed to create channel: " << name );
      return event_channel._retn();
    }

    try {
      LNDEBUG( "CreateEventChannel", " action - Narrow EventChannel" );
      event_channel = CosEventChannelAdmin::EventChannel::_narrow(obj);
    }
    catch( CORBA::Exception &ex ) {
      LNERROR( "CreateEventChannel", " Failed Narrow to EventChannel for:" << name  );
    }
    LNDEBUG( "CreateEventChannel", " created event channel " << name );
    try {

      if(!CORBA::is_nil(orb->namingService) && ( action == ossie::corba::NS_BIND) ) {
	ossie::corba::Bind( name, event_channel.in(), nc_name, true );
      }

    } 
    catch (const CosLifeCycle::InvalidCriteria& ex) {
      LNERROR( "CreateEventChannel", " CHANNEL: Invalid Criteria: " << ex._name() << " - for creating event channel " << name );
    } catch (const CORBA::Exception& ex) {
      LNERROR( "CreateEventChannel", " CHANNEL: CORBA " << ex._name() << " exception during create operation, CHANNEL:" << name );
    }


    LNDEBUG( "CreateEventChannel", " completed create event channel : " << name );
    return event_channel._retn();

  }


  //
  // DeleteEventChannel
  //
  // @param orb  context of the orb we are associated with 
  // @param name name of the event channel to delete
  // @parm  unbind perform an unbind operation with the NamingService if channel was deleted
  //
  // @returns 0 operation passed no issues or errors
  // @returns > 0 operation passed but issues were found but not a failure
  // @returns < 0 operation failed due to execeptions from the orb.
  //
  //
  //
  int DeleteEventChannel ( const std::string& name, corba::NS_ACTION action ) {
    int retval = 0;
    // return value if no event channel was found or error occured
    CosEventChannelAdmin::EventChannel_var event_channel = CosEventChannelAdmin::EventChannel::_nil();

    event_channel = GetEventChannel( name, false  );
    if ( CORBA::is_nil(event_channel) == true ) {
      LNDEBUG( "DeleteEventChannel", " Cannot find event channel name " << name << " to object, try to remove from naming context." );
      if ( ( action == ossie::corba::NS_UNBIND) &&  ossie::corba::Unbind( name ) == 0 ) {
	  LNINFO( "DeleteEventChannel", "Deregister EventChannel with the NamingService, CHANNEL:" << name );
	}
      retval=-1;
      return retval;
    }

    try {
      event_channel->destroy();
      LNINFO( "DeleteEventChannel", " Deleted event channel, CHANNEL: " << name );
    } 
    catch(CORBA::SystemException& ex) {
      // this will happen if channel is destroyed but 
      LNWARN( "DeleteEventChannel", " System exception occured, ex " << ex._name() );
      retval=-1;
    }
    catch(CORBA::Exception& ex) {
      LNWARN( "DeleteEventChannel", " CORBA exception occured, ex " << ex._name() );
      retval=-1;
    }

    try {
      if( (action == ossie::corba::NS_UNBIND) &&  ossie::corba::Unbind( name ) == 0 ) {
	LNINFO( "DeleteEventChannel", "Deregister EventChannel with the NamingService, CHANNEL:" << name );
      }

    } catch(CosNaming::NamingContext::InvalidName& ex) {
      LNWARN( "DeleteEventChannel", " Invalid name to unbind, name: " << name  );
    }
    catch(CosNaming::NamingContext::NotFound& ex) { // resolve
      LNWARN( "DeleteEventChannel", " Name not found, name: " << name  );
    }
    catch(CosNaming::NamingContext::CannotProceed& ex) { // resolve
      LNERROR( "DeleteEventChannel", " Cannot Process error, name: " << name );
      retval=-1;
    }

    return retval;
  }


int DeleteEventChannel (  const std::string& name, 
			   const std::string& nc_name, 
			   corba::NS_ACTION action )
  {
    
    int retval = 0;
    // return value if no event channel was found or error occured
    CosEventChannelAdmin::EventChannel_var event_channel = CosEventChannelAdmin::EventChannel::_nil();

    event_channel = GetEventChannel( name, nc_name, false  );
    if ( CORBA::is_nil(event_channel) == true ) {
      LNDEBUG( "DeleteEventChannel", " Cannot find event channel name " << name << " to object, try to remove from naming context." );
      if ( ( action == ossie::corba::NS_UNBIND) &&  ossie::corba::Unbind( name, nc_name ) == 0 ) {
	  LNINFO( "DeleteEventChannel", "Deregister EventChannel with the NamingService, CHANNEL:" << name );
	}
      retval=-1;
      return retval;
    }

    try {
      event_channel->destroy();
      LNINFO( "DeleteEventChannel", " Deleted event channel, CHANNEL: " << name );
    } 
    catch(CORBA::SystemException& ex) {
      // this will happen if channel is destroyed but 
      LNWARN( "DeleteEventChannel", " System exception occured, ex " << ex._name() );
      retval=-1;
    }
    catch(CORBA::Exception& ex) {
      LNWARN( "DeleteEventChannel", " CORBA exception occured, ex " << ex._name() );
      retval=-1;
    }

    try {
      if( (action == ossie::corba::NS_UNBIND) &&  ossie::corba::Unbind( name, nc_name ) == 0 ) {
	LNINFO( "DeleteEventChannel", "Deregister EventChannel with the NamingService, CHANNEL:" << name );
      }

    } catch(CosNaming::NamingContext::InvalidName& ex) {
      LNWARN( "DeleteEventChannel", " Invalid name to unbind, name: " << name  );
    }
    catch(CosNaming::NamingContext::NotFound& ex) { // resolve
      LNWARN( "DeleteEventChannel", " Name not found, name: " << name  );
    }
    catch(CosNaming::NamingContext::CannotProceed& ex) { // resolve
      LNERROR( "DeleteEventChannel", " Cannot Process error, name: " << name );
      retval=-1;
    }

    return retval;
  }


  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////

  //
  //  PushEventSupplier class implementation
  //
PushEventSupplier::PushEventSupplier( const std::string &channelName, 
				      Supplier          *inSupplier,
				      int                retries, 
				      int                retry_wait ) :
    name(channelName),
    nc_name(""),
    supplier(inSupplier),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init();
  }


  PushEventSupplier::PushEventSupplier( const std::string &channelName, 
					const std::string &ncName, 
					Supplier          *inSupplier,
					int                retries, 
					int                retry_wait ) :
    name(channelName),
    nc_name(ncName),
    supplier(inSupplier),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init();
  }

PushEventSupplier::PushEventSupplier(   const std::string &channelName, 
					int               retries, 
					int               retry_wait ) :
    name(channelName),
    nc_name(""),
    supplier(NULL),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init();
  }

  PushEventSupplier::PushEventSupplier( const std::string &channelName, 
					const std::string &ncName, 
					int                retries, 
					int                retry_wait ):
    name(channelName),
    nc_name(ncName),
    supplier(NULL),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init();
  }

  void PushEventSupplier::_init( ) 
  {
    LNDEBUG("PushEventSupplier", " GetEventChannel " << name );
    channel = GetEventChannel( name, nc_name, true );
   
    if ( CORBA::is_nil(channel) == true ) {
      LNERROR("PushEventSupplier", " Channel resource not available, channel " << name );
      return;
    }

    int tries=retries;
    do
      {
	try {
	  supplier_admin = channel->for_suppliers ();
	  break;
	}
	catch (CORBA::COMM_FAILURE& ex) {
	}
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while ( tries );

    if ( CORBA::is_nil(supplier_admin) ) return;
    
    LNDEBUG("PushEventSupplier", "Obtained SupplierAdmin." );

    tries=retries;
    do {
      try {
	proxy_for_consumer = supplier_admin->obtain_push_consumer ();
	break;
      }
      catch (CORBA::COMM_FAILURE& ex) {
      }
      if ( retry_wait > 0 ) {
	boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
      } else {
	boost::this_thread::yield();
      }
      tries--;
    } while ( tries );

    LNDEBUG("PushEventSupplier", "Obtained ProxyPushConsumer." );
    if ( CORBA::is_nil(proxy_for_consumer) ) return;

    if ( supplier == NULL ) {      
      LNDEBUG("PushEventSupplier", "Create Local Supplier Object." );
      supplier = new PushEventSupplier::Supplier();
    }

    CosEventComm::PushSupplier_var sptr =CosEventComm::PushSupplier::_nil();
    sptr = supplier->_this();

    // now attach supplier to the proxy
    do {
      try {
	proxy_for_consumer->connect_push_supplier(sptr.in());
      }
      catch (CORBA::BAD_PARAM& ex) {
	LNERROR("PushEventSupplier", "Caught BAD_PARAM " );
	break;
      }
      catch (CosEventChannelAdmin::AlreadyConnected& ex) {
	break;
      }
      catch (CORBA::COMM_FAILURE& ex) {
	LNERROR("PushEventSupplier",  "Caught COMM_FAILURE Exception "  << 
		"connecting Push Supplier! Retrying..." );
      }
      if ( retry_wait > 0 ) {
	boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
      } else {
	boost::this_thread::yield();
      }
      tries--;
    } while ( tries );


    LNDEBUG("PushEventSupplier",  "Connected Push Supplier." );

  };
    
  PushEventSupplier::~PushEventSupplier( ) {

    LNDEBUG("PushEventSupplier", "DTOR - START." );
    CORBA::ORB_ptr orb = ossie::corba::Orb();
    int tries = retries;
    if ( CORBA::is_nil(proxy_for_consumer) == false && 
         CORBA::is_nil(orb) == false ) {
     // Disconnect - retrying on Comms Failure.
     do {
        try {
           proxy_for_consumer->disconnect_push_consumer();
           break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
	  LNERROR("PushEventSupplier",  "Caught COMM_FAILURE Exception "
		  << "disconnecting Push Supplier! Retrying..." );
        }
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while(tries);
      LNDEBUG("PushEventSupplier", "ProxyPushConsumer disconnected." );
      
    }
    
    if ( supplier ) {
      supplier->_remove_ref();
    }

    LNDEBUG("PushEventSupplier", "DTOR - END." );

  }


  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////

  //
  //  PushEventConsumer class implementation
  //
  PushEventConsumer::PushEventConsumer( const std::string   &channelName, 
					Consumer            *inConsumer,
					const int            retries, 
					const int            retry_wait ):
    name(channelName),
    nc_name(""),
    consumer(inConsumer),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init();
  }

  PushEventConsumer::PushEventConsumer( const std::string   &channelName, 
					const std::string   &ncName, 
					Consumer            *inConsumer,
					const int            retries, 
					const int            retry_wait ):
    name(channelName),
    nc_name(ncName),
    consumer(inConsumer),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init();
  }


  PushEventConsumer::PushEventConsumer( const std::string   &channelName, 
					const int retries, 
					const int retry_wait ):
    name(channelName),
    nc_name(""),
    consumer(NULL),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init();
  }


  PushEventConsumer::PushEventConsumer( const std::string   &channelName, 
					const std::string   &ncName, 
					const int retries, 
					const int retry_wait ):
    name(channelName),
    nc_name(ncName),
    consumer(NULL),
    retries(retries),
    retry_wait(retry_wait)
  {
    _init();
  }


  void PushEventConsumer::_init( ) 
  {
    LNDEBUG("PushEventConsumer", " GetEventChannel " << name );
    try {
      //channel = corba::GetEventChannel( orb, name, nc_name, true );
      channel = GetEventChannel( name, nc_name, true );
    }
    catch(...){
        LNERROR("PushEventConsumer", " Channel " << name );
	return;
    }
    
    if ( CORBA::is_nil(channel) == true ) {
      LNERROR("PushEventConsumer", " Channel resource not available, channel " << name );
      return;
    }

    int tries=retries;
    do
      {
	try {
	  consumer_admin = channel->for_consumers ();
	  break;
	}
	catch (CORBA::COMM_FAILURE& ex) {
	}
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while ( tries );

    if ( CORBA::is_nil(consumer_admin) ) return;
    
    LNDEBUG("PushEventConsumer", "Obtained ConsumerAdmin." );

    tries=retries;
    do {
      try {
	proxy_for_supplier = consumer_admin->obtain_push_supplier ();
	break;
      }
      catch (CORBA::COMM_FAILURE& ex) {
      }
      if ( retry_wait > 0 ) {
	boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
      } else {
	boost::this_thread::yield();
      }
      tries--;
    } while ( tries );

    LNDEBUG("PushEventConsumer", "Obtained ProxyPushConsumer." );
    if ( CORBA::is_nil(proxy_for_supplier) ) return;

    if  ( consumer == NULL ) {
      consumer = new PushEventConsumer::CallbackConsumer(*this);
    }
    if ( consumer == NULL ) return;
    CosEventComm::PushConsumer_var sptr =CosEventComm::PushConsumer::_nil();
    sptr = consumer->_this();

    // now attach supplier to the proxy
    do {
      try {
	// connect the the consumer object to the supplier's proxy
	proxy_for_supplier->connect_push_consumer(sptr.in());
      }
      catch (CORBA::BAD_PARAM& ex) {
	LNERROR("PushEventConsumer", "Caught BAD_PARAM " );
	break;
      }
      catch (CosEventChannelAdmin::AlreadyConnected& ex) {
	break;
      }
      catch (CORBA::COMM_FAILURE& ex) {
	LNERROR("PushEventConsumer",  "Caught COMM_FAILURE Exception "  << 
		"connecting Push Consumer! Retrying..." );
      }
      if ( retry_wait > 0 ) {
	boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
      } else {
	boost::this_thread::yield();
      }
      tries--;
    } while ( tries );


    LNDEBUG("PushEventConsumer",  "Connected Push Consumer." );

  };
    
  PushEventConsumer::~PushEventConsumer( ) {

    LNDEBUG("PushEventConsumer", "DTOR - START." );
    CORBA::ORB_ptr orb = ossie::corba::Orb();
    int tries = retries;
    if ( CORBA::is_nil(proxy_for_supplier) == false && 
         CORBA::is_nil(orb) == false ) {
     // Disconnect - retrying on Comms Failure.
     do {
        try {
           proxy_for_supplier->disconnect_push_supplier();
           break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
	  LNERROR("PushEventConsumer",  "Caught COMM_FAILURE Exception "
		  << "disconnecting Push Consumer! Retrying..." );
        }
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while(tries);
      LNDEBUG("PushEventConsumer", "ProxyPushSupplier disconnected." );
      
    }

    if ( consumer ) {
      consumer->_remove_ref();
    }

    LNDEBUG("PushEventConsumer", "DTOR - END." );
  }

#if 0
  void   PushEventConsumer::detach() {

    LNDEBUG("PushEventConsumer", "DETTACH - START." );
    CORBA::ORB_ptr orb = ossie::corba::Orb();
    int tries = retries;
    if ( CORBA::is_nil(proxy_for_supplier) == false &&
         CORBA::is_nil(orb) == false ) {
      // Disconnect - retrying on Comms Failure.
      do {
        try {
	  proxy_for_supplier->disconnect_push_supplier();
	  break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
	  LNERROR("PushEventConsumer",  "Caught COMM_FAILURE Exception "
		  << "disconnecting Push Consumer! Retrying..." );
        }
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while(tries);
      LNDEBUG("PushEventConsumer", "ProxyPushSupplier disconnected." );
      
    }
    else {
      LNDEBUG("PushEventConsumer", "DETTACH - ProxyForSupplier is Nil." );    
    }
  }



  void   PushEventConsumer::attach() {

    LNDEBUG("PushEventConsumer", "ATTACH - START." );
    if ( consumer == NULL ) return;

    LNDEBUG("PushEventConsumer", "Register Consumer." );    
    CORBA::ORB_ptr orb = ossie::corba::Orb();
    CosEventComm::PushConsumer_var sptr = consumer->_this();
    int tries = retries;

    if ( CORBA::is_nil(proxy_for_supplier) == false && 
         CORBA::is_nil(orb) ==  false ) {
      // now attach supplier to the proxy
      do {
	try {
	  // connect the the consumer object to the supplier's proxy
	  proxy_for_supplier->connect_push_consumer(sptr.in());
	}
	catch (CORBA::BAD_PARAM& ex) {
	  LNERROR("PushEventConsumer", "Caught BAD_PARAM " );
	  break;
	}
	catch (CosEventChannelAdmin::AlreadyConnected& ex) {
	  LNDEBUG("PushEventConsumer", "ATTACH - Already Connected Consumer." );    
	  break;
	}
	catch (CORBA::COMM_FAILURE& ex) {
	  LNERROR("PushEventConsumer",  "Caught COMM_FAILURE Exception "  << 
		  "connecting Push Consumer! Retrying..." );
	}
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while ( tries );

    }
    else {
      LNDEBUG("PushEventConsumer", "ATTACH - ProxyForSupplier is Nil." );    
    }
  }

#endif

  //
  // used for boost shared pointer instantion when user
  // supplied callback is provided
  //
  struct null_deleter
  {
    void operator()(void const *) const
    {
    }
  };

  void   PushEventConsumer::setDataArrivedListener( DataArrivedListener *newListener ) {
    dataArrivedCB =  boost::shared_ptr< DataArrivedListener >(newListener, null_deleter());
  }

  void   PushEventConsumer::setDataArrivedListener( DataArrivedCallbackFn  newListener ) {
    dataArrivedCB =  boost::make_shared< StaticDataArrivedListener >( newListener );
  }




} // end of namespace event
} // end of namespace ossie
