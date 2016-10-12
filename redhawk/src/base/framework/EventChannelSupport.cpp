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
#include "ossie/CorbaUtils.h"
#include "ossie/EventChannelSupport.h"


namespace ossie {


namespace events {


  EventChannelFactory_ptr getEventChannelFactory ()
  {
    RH_NL_TRACE_ENTER(EventChannelSupport)

    // Attempt to locate the OmniEvents event channel factory to create the event channels.
    // First, check for an initial reference in the omniORB configuration; if it cannot be
    // resolved in this manner, look it up via the naming service.
    RH_NL_TRACE("EventChannelSupport", "Locating EventChannelFactory via initial reference");
    CORBA::Object_var factoryObj;
    try {
      factoryObj = ossie::corba::Orb()->resolve_initial_references("EventService");
    } catch (const CORBA::ORB::InvalidName&) {
      RH_NL_TRACE("EventChannelSupport", "No initial reference for 'EventService'");
    } catch (const CORBA::Exception& e) {
      RH_NL_WARN("EventChannelSupport", "CORBA " << e._name() << " exception locating EventChannelFactory via initial reference");
    }

    if (CORBA::is_nil(factoryObj)) {
      RH_NL_TRACE("EventChannelSupport", "Looking up EventChannelFactory in NameService");
      CosNaming::Name_var factoryName = ossie::corba::stringToName("EventChannelFactory");

      try {
        factoryObj = ossie::corba::InitialNamingContext()->resolve(factoryName);
      } catch (const CosNaming::NamingContext::NotFound&) {
        RH_NL_TRACE("EventChannelSupport", "No naming service entry for 'EventChannelFactory'");
      } catch (const CORBA::Exception& e) {
        RH_NL_WARN("EventChannelSupport", "CORBA " << e._name() << " exception looking up EventChannelFactory in name service");
      }
    }

    //CosLifeCycle::GenericFactory_var factory;
    EventChannelFactory_var factory;
    if (!CORBA::is_nil(factoryObj)) {
      try {
        if (!factoryObj->_non_existent()) {
          factory = CosLifeCycle::GenericFactory::_narrow(factoryObj);
        }
      } catch (const CORBA::TRANSIENT&) {
        RH_NL_WARN("EventChannelSupport", "Could not contact EventChannelFactory");
      } 
    }
    RH_NL_TRACE_EXIT(EventChannelSupport);
    return factory._retn();
  }

  EventChannel_ptr connectToEventChannel (CosNaming::NamingContext_ptr context, const std::string& name)
  {
    RH_NL_TRACE_ENTER(EventChannelSupport);
    CosNaming::Name_var boundName = ossie::corba::stringToName(name);
    try {
      CORBA::Object_var obj = context->resolve(boundName);
      EventChannel_var ret = EventChannel::_nil();
       ret =  ossie::events::EventChannel::_narrow(obj);
       RH_NL_TRACE("EventChannelSupport", "Existing event channel " << name << " found");
      return ret;
    } catch (const CosNaming::NamingContext::NotFound&) {
      // The channel does not exist and can be safely created.
    } catch (const CORBA::Exception& e) {
      RH_NL_WARN("EventChannelSupport", "CORBA (" << e._name() << ") exception connecting to event channel " << name <<". Continue without connecting to the channel (the event service might not be running)");
    }

    RH_NL_TRACE_EXIT(EventChannelSupport);
    return EventChannel::_nil();
  }

  EventChannel_ptr createEventChannel (const std::string& name)
  {
    RH_NL_TRACE_ENTER(EventChannelSupport);

    EventChannelFactory_ptr factory = getEventChannelFactory();
    if (CORBA::is_nil(factory)) {
      RH_NL_WARN("EventChannelSupport", "Event channel " << name << " not created");
      RH_NL_TRACE_EXIT(EventChannelSupport);
      return EventChannel::_nil();
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

    EventChannel_var eventChannel;

    RH_NL_TRACE("EventChannelSupport", "Creating event channel " << name);
    RH_NL_INFO("EventChannelSupport", "Creating event channel " << name);
    try {
      CORBA::Object_var obj = factory->create_object(key, criteria);
      eventChannel = ossie::events::EventChannel::_narrow(obj);
    } catch (const CosLifeCycle::InvalidCriteria&) {
      RH_NL_WARN("EventChannelSupport", "Invalid Criteria for creating event channel " << name);
    } catch (const CORBA::Exception& ex) {
      RH_NL_WARN("EventChannelSupport", "CORBA " << ex._name() << " exception creating event channel " << name);
    }

    RH_NL_TRACE_EXIT(EventChannelSupport);
    return eventChannel._retn();
  }


  ////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////

  //
  // EventChannel Convenience Methods
  //

  //
  // (Taken from eventc.cc)
  //
  //
  /*
     This requires libomniEvent.so....

     omniEvents::EventChannelFactory_ptr GetEventChannelFactory ( corba::OrbPtr &orb ) {
    
     CORBA::Object_var ecf_obj;
     omniEvents::EventChannelFactory_var ecf = omniEvents::EventChannelFactory::_nil();
     RH_NL_DEBUG( "GetEventChannelFactory", " Look up EventChannelFactory" );
     try {
     //ecf_obj = orb.namingServiceCtx->resolve_str(str2name("EventChannelFactory"));
     ecf_obj = orbCtx.namingServiceCtx->resolve_str("EventChannelFactory");
     if (!CORBA::is_nil(ecf_obj)) {
     RH_NL_DEBUG( "GetEventChannelFactory",  " Narrow object to EventChannelFactory" );
     ecf = omniEvents::EventChannelFactory::_narrow(ecf_obj);
     RH_NL_DEBUG( "GetEventChannelFactory",  " Narrowed to ... EventChannelFactory" );
     }
     } catch (const CosNaming::NamingContext::NotFound&) {
     RH_NL_WARN( "GetEventChannelFactory", " No naming service entry for 'EventChannelFactory'" );
     } catch (const CORBA::Exception& e) {
     RH_NL_WARN( "GetEventChannelFactory",  " CORBA " << e._name() << ", exception looking up EventChannelFactory." );
     } 
     return ecf._retn();

     }

     CosLifeCycle::GenericFactory_ptr GetEventChannelFactory ( ) {
    
     Orb orbCtx;
     OrbPtr orb=&orbCtx;

     CORBA::Object_var ecf_obj;
     //omniEvents::EventChannelFactory_var ecf = omniEvents::EventChannelFactory::_nil();
     CosLifeCycle::GenericFactory_var  ecf;
     RH_NL_DEBUG( "GetEventChannelFactory", " Look up EventChannelFactory" );
     try {
     //ecf_obj = orb.namingServiceCtx->resolve_str(str2name("EventChannelFactory"));
     ecf_obj = orbCtx.namingServiceCtx->resolve_str("EventChannelFactory");
     if (!CORBA::is_nil(ecf_obj)) {
     RH_NL_DEBUG( "GetEventChannelFactory",  " Narrow object to EventChannelFactory" );
     ecf = CosLifeCycle::GenericFactory::_narrow(ecf_obj);
     //ecf = omniEvents::EventChannelFactory::_narrow(ecf_obj);
     RH_NL_DEBUG( "GetEventChannelFactory",  " Narrowed to ... EventChannelFactory" );
     }
     } catch (const CosNaming::NamingContext::NotFound&) {
     RH_NL_WARN( "GetEventChannelFactory", " No naming service entry for 'EventChannelFactory'" );
     } catch (const CORBA::Exception& e) {
     RH_NL_WARN( "GetEventChannelFactory",  " CORBA " << e._name() << ", exception looking up EventChannelFactory." );
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
  EventChannel_ptr GetEventChannel ( const std::string& name, 
							   const bool create,
							   const std::string &host ) {
    RH_NL_DEBUG("GetEventChannel",  " : NamingService look up, Channel " << name );

    ossie::corba::OrbContext orbCtx;

    // return value if no event channel was found or error occured
    //CosEventChannelAdmin::EventChannel_var event_channel = CosEventChannelAdmin::EventChannel::_nil();
    EventChannel_var event_channel = EventChannel::_nil();

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
	RH_NL_DEBUG( "GetEventChannel", " : Trying InitRef Lookup " << name );
	CORBA::Object_var obj = orbCtx.orb->resolve_initial_references(name.c_str());
        if ( CORBA::is_nil(obj) == false ){
	  //CosEventChannelAdmin::EventChannel_var tchannel = CosEventChannelAdmin::EventChannel::_narrow(obj);
          //event_channel = EventChannel::_unchecked_narrow(tchannel);
          event_channel = EventChannel::_narrow(obj);
          found =true;
          RH_NL_DEBUG( "GetEventChannel", " : FOUND EXISTING, Channel " << name );
        }
      } 
    }catch (const CORBA::Exception& e) {
      RH_NL_DEBUG( "GetEventChannel", "  Unable to lookup with InitRef:" << name << ",  CORBA RETURNED(" << e._name() << ")" );
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
	RH_NL_DEBUG( "GetEventChannel", " : Trying corbaname resolution " << tname );
	CORBA::Object_var obj = obj=orbCtx.orb->string_to_object(tname.c_str());
        if ( CORBA::is_nil(obj) == false ){
	  //CosEventChannelAdmin::EventChannel_var tchannel = CosEventChannelAdmin::EventChannel::_narrow(obj);
	  //          event_channel = EventChannel::_unchecked_narrow(tchannel);
          event_channel = ossie::events::EventChannel::_narrow(obj);
          found =true;
          RH_NL_DEBUG( "GetEventChannel", " : FOUND EXISTING, Channel " << tname );
        }
      } 

    }catch (const CORBA::Exception& e) {
      RH_NL_DEBUG( "GetEventChannel",  "  Unable to lookup with corbaname:  URI:" << tname << ",  CORBA RETURNED(" << e._name() << ")");
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
	RH_NL_DEBUG( "GetEventChannel"," : Trying corbaloc resolution " << tname );
	CORBA::Object_var obj = orbCtx.orb->string_to_object(tname.c_str());
	if ( !CORBA::is_nil(obj) ) {
          //CosEventChannelAdmin::EventChannel_var tchannel = CosEventChannelAdmin::EventChannel::_narrow(obj);
          //event_channel = EventChannel::_unchecked_narrow(tchannel);
          event_channel = ossie::events::EventChannel::_narrow(obj);
          found = true;
          RH_NL_DEBUG( "GetEventChannel", " : FOUND EXISTING, Channel " << tname );
	}
	else {
	  RH_NL_DEBUG( "GetEventChannel", " : SEARCH FOR Channel " << tname << " FAILED");
	}
      } 
    }catch (const CORBA::Exception& e) {
      if ( !create ) {
        RH_NL_WARN( "GetEventChannel", "  Unable to lookup with corbaloc URI:" << tname << ", CORBA RETURNED(" << e._name() << ")" );
      }
    }

    try{
      if ( !found && create ) {

	RH_NL_DEBUG( "GetEventChannel", " CREATE NEW CHANNEL " << name );
	event_channel = CreateEventChannel( name, ossie::corba::NS_NOBIND);
	if ( !CORBA::is_nil(event_channel) )
	  RH_NL_INFO( "GetEventChannel", " --- CREATED NEW CHANNEL ---" << name );
      }
    } catch (const CORBA::Exception& e) {
      RH_NL_ERROR( "GetEventChannel", "  CORBA (" << e._name() << ") during event creation, channel " << name );
    }

    RH_NL_DEBUG( "GetEventChannel", "  RETURN: channel " << name  << " found " << found );
    return event_channel._retn();
  }


 EventChannel_ptr GetEventChannel ( const std::string& name, 
							   const std::string &nc_name, 
							   const bool create,
							   const std::string &host )
  {

    // return value if no event channel was found or error occured
    EventChannel_var event_channel = EventChannel::_nil();
    // set orb context..
    ossie::corba::OrbContext    orbCtx;
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
      RH_NL_DEBUG("GetEventChannel",  " : NamingService look up, NC<"<<nc_name<<"> Channel " << name );
      std::string cname=name;
      if ( nc_name != "" )
	cname=nc_name+"/"+name;

      RH_NL_DEBUG("GetEventChannel",  " : NamingService look up : " << cname );
      CORBA::Object_var obj = orbCtx.namingServiceCtx->resolve_str(cname.c_str());
        if ( CORBA::is_nil(obj) == false ){
	  // CosEventChannelAdmin::EventChannel_var tchannel = CosEventChannelAdmin::EventChannel::_narrow(obj);
          //event_channel = EventChannel::_unchecked_narrow(tchannel);
          event_channel = ossie::events::EventChannel::_narrow(obj);
          found =true;
          RH_NL_INFO( "GetEventChannel", " : FOUND EXISTING, CHANNEL " << name  << " FROM CONTEXT: " << nc_name );
        }
    } catch (const CosNaming::NamingContext::NotFound&) {
      RH_NL_DEBUG("GetEventChannel",  "  Unable to resolve event channel (" << name << ") in NamingService..." );
    } catch (const CORBA::Exception& e) {
      RH_NL_ERROR("GetEventChannel", "  CORBA (" << e._name() << ") exception during event channel look up, CH:" << name );
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
	os << "corbaloc::"<<host<<":11169/";
        if ( nc_name != "" ) { 
          os << nc_name << "." << name;
        }
        else {
          os << name;
        }
	tname=os.str();
	RH_NL_DEBUG( "GetEventChannel"," : Trying corbaloc resolution " << tname );
	CORBA::Object_var obj = orbCtx.orb->string_to_object(tname.c_str());
	if ( !CORBA::is_nil(obj) ) {
          event_channel = ossie::events::EventChannel::_narrow(obj);
          found = true;
          RH_NL_DEBUG( "GetEventChannel", " : FOUND EXISTING, Channel " << tname );

          try {
            if(!CORBA::is_nil(orbCtx.namingService) and nc_name != "" ) {
              RH_NL_DEBUG( "GetEventChannel", " : Trying to bind EXISTING Event Channel " << tname );
              ossie::corba::Bind( name, event_channel.in(), nc_name, true );
            }
            
          } 
          catch (const CORBA::Exception& ex) {
            RH_NL_ERROR( "CreateEventChannel", " CHANNEL: CORBA " << ex._name() << " exception during create operation, CHANNEL:" << name );
          }

	}
	else {
	  RH_NL_DEBUG( "GetEventChannel", " : SEARCH FOR Channel " << tname << " FAILED");
	}
      } 
    }catch (const CORBA::Exception& e) {
      if ( !create ) {
        RH_NL_WARN( "GetEventChannel", "  Unable to lookup with corbaloc URI:" << tname << ", CORBA RETURNED(" << e._name() << ")" );
      }
    }



    try{
      if ( !found && create ) {

	RH_NL_DEBUG( "GetEventChannel", " CREATE NEW CHANNEL " << name );
	event_channel = CreateEventChannel( name, nc_name);
	if ( !CORBA::is_nil(event_channel) )
	  RH_NL_INFO( "GetEventChannel", " --- CREATED NEW CHANNEL ---" << name << " ASSIGNED TO CONTEXT:" << nc_name);
      }
    } catch (const CORBA::Exception& e) {
      RH_NL_ERROR( "GetEventChannel", "  CORBA (" << e._name() << ") during event creation, channel " << name );
    }

    return event_channel._retn();

  }

  //
  // CreateEventChannel
  //
  // @param orb  context of the orb we are associated with 
  // @param name human readable path to the event channel being requested
  // @parm  action bind the channel name to the object in the NamingService if channel was created
  //
  EventChannel_ptr CreateEventChannel (  const std::string& name, 
                                                               corba::NS_ACTION action  ) {
    return CreateEventChannel( name, "", action );
  }

  EventChannel_ptr CreateEventChannel ( const std::string& name, 
							      const std::string &nc_name, 
							      corba::NS_ACTION action  )
  {
    ossie::corba::OrbContext  orbCtx;

    EventChannel_var event_channel =  EventChannel::_nil();
    EventChannelFactory_var event_channel_factory = getEventChannelFactory( );
    RH_NL_DEBUG( "CreateEventChannel", " Request to create event 44channel:" << name << " bind action:" << action );

    if (CORBA::is_nil(event_channel_factory)) {
      RH_NL_ERROR( "CreateEventChannel", "CHANNEL(CREATE): Could not find EventChannelFactory" );
      return event_channel._retn();
    }

    CosLifeCycle::Key key;
    key.length (1);
    key[0].id = CORBA::string_dup("EventChannel");
    key[0].kind = CORBA::string_dup("object interface");

    RH_NL_DEBUG( "CreateEventChannel", " action - event channel factory api" );
    if(!event_channel_factory->supports(key))
      {
	RH_NL_WARN( "CreateEventChannel", " EventChannelFactory does not support Event Channel Interface!" );
	return event_channel._retn();	
      }

    // 
    // Our EventChannels will always be created with InsName
    //
    std::string tname(name);
    if ( nc_name != "" ) { tname=nc_name+"."+name;  }
    RH_NL_INFO( "CreateEventChannel", " Request to create event channel:" << tname << " bind action:" << action );
    CosLifeCycle::Criteria criteria;
    criteria.length(1);
    criteria[0].name=CORBA::string_dup("InsName");
    criteria[0].value<<=tname.c_str();

    //
    // Create Event Channel Object.
    //
    RH_NL_DEBUG( "CreateEventChannel", " action - create EventChannel object" );

    CORBA::Object_var obj;
    try {
      obj =event_channel_factory->create_object(key, criteria);
    }
    catch (CosLifeCycle::CannotMeetCriteria& ex) /* create_object() */ {
      RH_NL_ERROR( "CreateEventChannel", "create_object failed, channel: " << name << " reason: CannotMeetCriteria " );
    }
    catch (CosLifeCycle::InvalidCriteria& ex) /* create_object() */ {
      RH_NL_ERROR( "CreateEventChannel", "create_object failed, channel: " << name << " reason: InvalidCriteria " );
      if(ex.invalid_criteria.length()>0) {
	int j;
	for (  j=0; (unsigned int)j < ex.invalid_criteria.length(); j++ ) { 
	  RH_NL_ERROR( "CreateEventChannel", "--- Criteria Name: " << ex.invalid_criteria[j].name );
	  CORBA::ULong xx;
	  ex.invalid_criteria[j].value >>= xx;
	  RH_NL_ERROR( "CreateEventChannel", "--- Criteria Value : " << xx );
	}
      }
    }
    catch( CORBA::Exception &ex ) {
      RH_NL_ERROR( "CreateEventChannel", " create_object failed, channel:" << name << " reason: corba exception" );
    }

    if (CORBA::is_nil(obj)) {
      RH_NL_ERROR( "CreateEventChannel", " Factory failed to create channel: " << name );
      return event_channel._retn();
    }

    try {
      RH_NL_DEBUG( "CreateEventChannel", " action - Narrow EventChannel" );
      event_channel = ossie::events::EventChannel::_narrow(obj);
    }
    catch( CORBA::Exception &ex ) {
      RH_NL_ERROR( "CreateEventChannel", " Failed Narrow to EventChannel for:" << name  );
    }
    RH_NL_DEBUG( "CreateEventChannel", " created event channel " << name );
    try {

      if(!CORBA::is_nil(orbCtx.namingService) && ( action == ossie::corba::NS_BIND) ) {
	ossie::corba::Bind( name, event_channel.in(), nc_name, true );
      }

    } 
    catch (const CosLifeCycle::InvalidCriteria& ex) {
      RH_NL_ERROR( "CreateEventChannel", " CHANNEL: Invalid Criteria: " << ex._name() << " - for creating event channel " << name );
    } catch (const CORBA::Exception& ex) {
      RH_NL_ERROR( "CreateEventChannel", " CHANNEL: CORBA " << ex._name() << " exception during create operation, CHANNEL:" << name );
    }


    RH_NL_DEBUG( "CreateEventChannel", " completed create event channel : " << name );
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
      RH_NL_DEBUG( "DeleteEventChannel", " Cannot find event channel name " << name << " to object, try to remove from naming context." );
      if ( ( action == ossie::corba::NS_UNBIND) &&  ossie::corba::Unbind( name ) == 0 ) {
        RH_NL_INFO( "DeleteEventChannel", "Deregister EventChannel with the NamingService, CHANNEL:" << name );
      }
      retval=-1;
      return retval;
    }

    try {
      event_channel->destroy();
      RH_NL_INFO( "DeleteEventChannel", " Deleted event channel, CHANNEL: " << name );
    } 
    catch(CORBA::SystemException& ex) {
      // this will happen if channel is destroyed but 
      RH_NL_WARN( "DeleteEventChannel", " System exception occured, ex " << ex._name() );
      retval=-1;
    }
    catch(CORBA::Exception& ex) {
      RH_NL_WARN( "DeleteEventChannel", " CORBA exception occured, ex " << ex._name() );
      retval=-1;
    }

    try {
      if( (action == ossie::corba::NS_UNBIND) &&  ossie::corba::Unbind( name ) == 0 ) {
	RH_NL_INFO( "DeleteEventChannel", "Deregister EventChannel with the NamingService, CHANNEL:" << name );
      }

    } catch(CosNaming::NamingContext::InvalidName& ex) {
      RH_NL_WARN( "DeleteEventChannel", " Invalid name to unbind, name: " << name  );
    }
    catch(CosNaming::NamingContext::NotFound& ex) { // resolve
      RH_NL_WARN( "DeleteEventChannel", " Name not found, name: " << name  );
    }
    catch(CosNaming::NamingContext::CannotProceed& ex) { // resolve
      RH_NL_ERROR( "DeleteEventChannel", " Cannot Process error, name: " << name );
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
      RH_NL_DEBUG( "DeleteEventChannel", " Cannot find event channel name " << name << " to object, try to remove from naming context." );
      if ( ( action == ossie::corba::NS_UNBIND) &&  ossie::corba::Unbind( name, nc_name ) == 0 ) {
        RH_NL_INFO( "DeleteEventChannel", "Deregister EventChannel with the NamingService, CHANNEL:" << name );
      }
      retval=-1;
      return retval;
    }

    try {
      event_channel->destroy();
      RH_NL_INFO( "DeleteEventChannel", " Deleted event channel, CHANNEL: " << name );
    } 
    catch(CORBA::SystemException& ex) {
      // this will happen if channel is destroyed but 
      RH_NL_WARN( "DeleteEventChannel", " System exception occured, ex " << ex._name() );
      retval=-1;
    }
    catch(CORBA::Exception& ex) {
      RH_NL_WARN( "DeleteEventChannel", " CORBA exception occured, ex " << ex._name() );
      retval=-1;
    }

    try {
      if( (action == ossie::corba::NS_UNBIND) &&  ossie::corba::Unbind( name, nc_name ) == 0 ) {
	RH_NL_INFO( "DeleteEventChannel", "Deregister EventChannel with the NamingService, CHANNEL:" << name );
      }

    } catch(CosNaming::NamingContext::InvalidName& ex) {
      RH_NL_WARN( "DeleteEventChannel", " Invalid name to unbind, name: " << name  );
    }
    catch(CosNaming::NamingContext::NotFound& ex) { // resolve
      RH_NL_WARN( "DeleteEventChannel", " Name not found, name: " << name  );
    }
    catch(CosNaming::NamingContext::CannotProceed& ex) { // resolve
      RH_NL_ERROR( "DeleteEventChannel", " Cannot Process error, name: " << name );
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
    RH_NL_DEBUG("PushEventSupplier", " GetEventChannel " << name );
    channel = GetEventChannel( name, nc_name, true );
   
    if ( CORBA::is_nil(channel) == true ) {
      RH_NL_ERROR("PushEventSupplier", " Channel resource not available, channel " << name );
      return;
    }

    CosEventChannelAdmin::SupplierAdmin_var     supplier_admin;
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
    
    RH_NL_TRACE("PushEventSupplier", "Obtained SupplierAdmin." );

    tries=retries;
    do {
      try {
	proxy = supplier_admin->obtain_push_consumer ();
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

    RH_NL_TRACE("PushEventSupplier", "Obtained ProxyPushConsumer." );
    if ( CORBA::is_nil(proxy) ) return;

    if ( supplier == NULL ) {      
      RH_NL_DEBUG("PushEventSupplier", "Create Local Supplier Object." );
      supplier = new PushEventSupplier::Supplier();
    }

    EventPublisherSupplier_var sptr = EventPublisherSupplier::_nil();
    sptr = supplier->_this();

    // now attach supplier to the proxy
    do {
      try {
	proxy->connect_push_supplier(sptr.in());
      }
      catch (CORBA::BAD_PARAM& ex) {
	RH_NL_ERROR("PushEventSupplier", "Caught BAD_PARAM " );
	break;
      }
      catch (CosEventChannelAdmin::AlreadyConnected& ex) {
	break;
      }
      catch (CORBA::COMM_FAILURE& ex) {
	RH_NL_ERROR("PushEventSupplier",  "Caught COMM_FAILURE Exception "  << 
		"connecting Push Supplier! Retrying..." );
      }
      if ( retry_wait > 0 ) {
	boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
      } else {
	boost::this_thread::yield();
      }
      tries--;
    } while ( tries );


    RH_NL_DEBUG("PushEventSupplier",  "Connected Push Supplier." );

  };
    
  PushEventSupplier::~PushEventSupplier( ) {

    RH_NL_DEBUG("PushEventSupplier", "DTOR - START." );
    CORBA::ORB_ptr orb = ossie::corba::Orb();
    int tries = retries;
    if ( CORBA::is_nil(proxy) == false && 
         CORBA::is_nil(orb) == false ) {
      // Disconnect - retrying on Comms Failure.
      do {
        try {
          proxy->disconnect_push_consumer();
          break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
	  RH_NL_ERROR("PushEventSupplier",  "Caught COMM_FAILURE Exception "
		  << "disconnecting Push Supplier! Retrying..." );
        }
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while(tries);
      RH_NL_DEBUG("PushEventSupplier", "ProxyPushConsumer disconnected." );
      
    }
    
    if ( supplier ) {
      supplier->_remove_ref();
    }

    RH_NL_DEBUG("PushEventSupplier", "DTOR - END." );

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
    RH_NL_DEBUG("PushEventConsumer", " GetEventChannel " << name );
    try {
      //channel = corba::GetEventChannel( orb, name, nc_name, true );
      channel = GetEventChannel( name, nc_name, true );
    }
    catch(...){
      RH_NL_ERROR("PushEventConsumer", " Channel " << name );
      return;
    }
    
    if ( CORBA::is_nil(channel) == true ) {
      RH_NL_ERROR("PushEventConsumer", " Channel resource not available, channel " << name );
      return;
    }

    int tries=retries;
    CosEventChannelAdmin::ConsumerAdmin_var     consumer_admin;
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
    
    RH_NL_TRACE("PushEventConsumer", "Obtained ConsumerAdmin." );

    tries=retries;
    do {
      try {
	proxy = consumer_admin->obtain_push_supplier ();
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

    RH_NL_TRACE("PushEventConsumer", "Obtained ProxyPushConsumer." );
    if ( CORBA::is_nil(proxy) ) return;

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
	proxy->connect_push_consumer(sptr.in());
      }
      catch (CORBA::BAD_PARAM& ex) {
	RH_NL_ERROR("PushEventConsumer", "Caught BAD_PARAM " );
	break;
      }
      catch (CosEventChannelAdmin::AlreadyConnected& ex) {
	break;
      }
      catch (CORBA::COMM_FAILURE& ex) {
	RH_NL_ERROR("PushEventConsumer",  "Caught COMM_FAILURE Exception "  << 
		"connecting Push Consumer! Retrying..." );
      }
      if ( retry_wait > 0 ) {
	boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
      } else {
	boost::this_thread::yield();
      }
      tries--;
    } while ( tries );


    RH_NL_DEBUG("PushEventConsumer",  "Connected Push Consumer." );

  };
    
  PushEventConsumer::~PushEventConsumer( ) {

    RH_NL_DEBUG("PushEventConsumer", "DTOR - START." );
    CORBA::ORB_ptr orb = ossie::corba::Orb();
    int tries = retries;
    if ( CORBA::is_nil(proxy) == false && 
         CORBA::is_nil(orb) == false ) {
      // Disconnect - retrying on Comms Failure.
      do {
        try {
          proxy->disconnect_push_supplier();
          break;
        }
        catch (CORBA::COMM_FAILURE& ex) {
	  RH_NL_ERROR("PushEventConsumer",  "Caught COMM_FAILURE Exception "
		  << "disconnecting Push Consumer! Retrying..." );
        }
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while(tries);
      RH_NL_DEBUG("PushEventConsumer", "ProxyPushSupplier disconnected." );
      
    }

    if ( consumer ) {
      consumer->_remove_ref();
    }

    RH_NL_DEBUG("PushEventConsumer", "DTOR - END." );
  }

#if 0
  void   PushEventConsumer::detach() {

    RH_NL_DEBUG("PushEventConsumer", "DETTACH - START." );
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
	  RH_NL_ERROR("PushEventConsumer",  "Caught COMM_FAILURE Exception "
		  << "disconnecting Push Consumer! Retrying..." );
        }
	if ( retry_wait > 0 ) {
	  boost::this_thread::sleep( boost::posix_time::microseconds( retry_wait*1000 ) );
	} else {
	  boost::this_thread::yield();
	}
	tries--;
      } while(tries);
      RH_NL_DEBUG("PushEventConsumer", "ProxyPushSupplier disconnected." );
      
    }
    else {
      RH_NL_DEBUG("PushEventConsumer", "DETTACH - ProxyForSupplier is Nil." );    
    }
  }



  void   PushEventConsumer::attach() {

    RH_NL_DEBUG("PushEventConsumer", "ATTACH - START." );
    if ( consumer == NULL ) return;

    RH_NL_DEBUG("PushEventConsumer", "Register Consumer." );    
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
	  RH_NL_ERROR("PushEventConsumer", "Caught BAD_PARAM " );
	  break;
	}
	catch (CosEventChannelAdmin::AlreadyConnected& ex) {
	  RH_NL_DEBUG("PushEventConsumer", "ATTACH - Already Connected Consumer." );    
	  break;
	}
	catch (CORBA::COMM_FAILURE& ex) {
	  RH_NL_ERROR("PushEventConsumer",  "Caught COMM_FAILURE Exception "  << 
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
      RH_NL_DEBUG("PushEventConsumer", "ATTACH - ProxyForSupplier is Nil." );    
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
