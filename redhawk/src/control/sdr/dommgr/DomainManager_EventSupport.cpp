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
#include <ossie/Events.h>
#include "DomainManager_EventSupport.h"
#include "DomainManager_impl.h"

using namespace ossie;


//
//  A Publisher Interface for Domain Event Channels
//

DOM_Publisher::DOM_Publisher( ossie::events::EventChannel_ptr           channel ) :
  Publisher(channel)
{}

DOM_Publisher::~DOM_Publisher() {};


//
//  A Subcriber Interface for Domain Event Channels
//

DOM_Subscriber::DOM_Subscriber(  ossie::events::EventChannel_ptr  channel ):
  Subscriber( channel )
{}

DOM_Subscriber::~DOM_Subscriber() {}


//
//  Create Subscriber Interface for an EventChannel
//
DOM_Subscriber_ptr DomainManager_impl::subscriber( const std::string &cname )
{
  DOM_Subscriber_ptr ret;

  try {
    ossie::events::EventChannel_var evt_channel;
    RH_NL_DEBUG("DomainManager","Requesting Event Channel::" << cname);
    if ( _eventChannelMgr->isChannel( cname ) ) {
      evt_channel =  ossie::events::EventChannel::_duplicate( _eventChannelMgr->findChannel( cname ) );
    }
    else {  
      evt_channel = createEventChannel(cname);
    }
  
    if (CORBA::is_nil(evt_channel)) {
      throw -1;
    }
    
    ret = DOM_Subscriber_ptr( new DOM_Subscriber( evt_channel ) );

  }
  catch(...){
    RH_NL_WARN("DomainManager", "Unable to establish Subscriber interface to event channel:"<< cname);
  }

  return ret;
}


//
//  Create Publisher Interface for an EventChannel
//
DOM_Publisher_ptr DomainManager_impl::publisher( const std::string &cname )
{
  DOM_Publisher_ptr ret;

  try {
    RH_NL_DEBUG("DomainManager","Requesting Event Channel::" << cname);
    ossie::events::EventChannel_var evt_channel;
    if ( _eventChannelMgr->isChannel( cname ) ) {
      evt_channel = ossie::events::EventChannel::_duplicate( _eventChannelMgr->findChannel( cname ) );
    }
    else {  
      evt_channel = createEventChannel(cname);
    }
    if (CORBA::is_nil(evt_channel)) {
      throw -1;
    }
    
    RH_NL_DEBUG("DomainManager","Create DomainManager Publisher Object for:" << cname);
    ret = DOM_Publisher_ptr( new DOM_Publisher( evt_channel ) );
  }
  catch(...){
    RH_NL_WARN("DomainManager","Unable to establish Publisher interface to event channel:" << cname);
  }

  return ret;
}


//
//  send and ObjectRemovedEvent message to the ODM Channel
//
void DomainManager_impl::sendRemoveEvent( const std::string &prod_id,
                                          const std::string &source_id,
                                          const std::string &source_name,
                                          StandardEvent::SourceCategoryType sourceCategory) {
  if ( !_odm_publisher ) return;

  redhawk::events::DomainEventWriter ewriter(_odm_publisher);
  redhawk::events::DomainStateEvent  evt;
  evt.prod_id = prod_id;
  evt.source_id = source_id;
  evt.source_name = source_name;
  evt.category = sourceCategory;
  ewriter.sendRemovedEvent( evt );
}



//
//  send and ObjectAddedEvent message to the IDM Channel
//
void DomainManager_impl::sendAddEvent( const std::string &prod_id,
                                       const std::string &source_id,
                                       const std::string &source_name,
                                       CORBA::Object_ptr obj,
                                       StandardEvent::SourceCategoryType sourceCategory) {
  
  if ( !_odm_publisher ) return;

  redhawk::events::DomainEventWriter ewriter(_odm_publisher);
  redhawk::events::DomainStateEvent  evt;
  evt.prod_id = prod_id;
  evt.source_id = source_id;
  evt.source_name = source_name;
  evt.obj = obj;
  evt.category = sourceCategory;
  ewriter.sendAddedEvent( evt );

}

void DomainManager_impl::sendResourceStateChange( const std::string &source_id,
                                        const std::string &source_name,
                                        const ExtendedEvent::ResourceStateChangeType stateChangeFrom, 
                                        const ExtendedEvent::ResourceStateChangeType stateChangeTo) {

  redhawk::events::DomainEventWriter ewriter(_odm_publisher);
  redhawk::events::ResourceStateChangeEvent  evt;
  evt.source_id = source_id;
  evt.source_name = source_name;
  evt.from = stateChangeFrom;
  evt.to = stateChangeTo;
  ewriter.sendResourceStateChange( evt );
}



void DomainManager_impl::handleIDMChannelMessages( const CORBA::Any &msg ) {

  const StandardEvent::AbnormalComponentTerminationEventType *termMsg;
  if ( msg >>= termMsg ) {
    LOG_WARN(DomainManager_impl, "Abormal Component Termination, Reporting Device: " << termMsg->deviceId << " Application/Component " << 
             termMsg->applicationId << "/" << termMsg->componentId );
  }

}


void DomainManager_impl::idmTerminationMessages( const redhawk::events::ComponentTerminationEvent &termMsg ) {
  LOG_WARN(DomainManager_impl, "Abormal Component Termination, Reporting Device: " << termMsg.device_id << " Application/Component " << 
             termMsg.application_id << "/" << termMsg.component_id );
}


void DomainManager_impl::establishDomainManagementChannels( const std::string &dburi ) {

    // Create Outgoing Domain Management (ODM) event channel
    if ( _eventChannelMgr ){

      if ( !dburi.empty() ) {
	LOG_INFO(DomainManager_impl, "Restoring event channels file:" << dburi);
	restoreEventChannels(dburi);
      }

      LOG_TRACE(DomainManager_impl, "Establishing Domain Event Channels");

      //
      // Create ODM Channel Publisher
      //
      std::string cname;
      try {
        cname = redhawk::events::ODM_Channel_Spec;
        _odm_publisher =  publisher( cname );
        if ( _odm_publisher ) {
          LOG_INFO(DomainManager_impl, "Domain Channel: " << cname << " created.");
        }
        else {
          throw -1;
        }
      }
      catch(...) {
	LOG_WARN(DomainManager_impl, "ODM Channel create FAILED, Disabling outgoing events");
      }

      //
      // Create IDM Channel Subscriber
      //
      try{
        cname = redhawk::events::IDM_Channel_Spec;
        DOM_Subscriber_ptr  idmSubscriber = subscriber( cname  );
        if ( idmSubscriber ) {
          LOG_INFO(DomainManager_impl, "Domain Channel: " << cname << " created.");
          _idm_reader.setTerminationListener( this, &DomainManager_impl::idmTerminationMessages );
          _idm_reader.subscribe( subscriber( cname  ) );
        }
        else {
          throw -1;
        }
      }
      catch(...) {
	LOG_WARN(DomainManager_impl, "IDM Channel create FAILED, Disabling incoming events");
      }

      LOG_DEBUG(DomainManager_impl, "Completed Creating Domain Event Channels");
    }
    else {
      LOG_WARN(DomainManager_impl, "No EventChannelManager, Disabling event channel management operations.");
    }
}


void DomainManager_impl::disconnectDomainManagementChannels() {

    if ( _eventChannelMgr ) {
      RH_NL_DEBUG("DomainManager", "Disconnect Domain Mananagment Event Channels. " );
      try {
        if ( _odm_publisher ) {
          _odm_publisher->disconnect();
          _odm_publisher.reset();
        }
      }
      catch(...){ 
        RH_NL_ERROR("DomainManager", "Error disconnecting from  ODM Channel. ");
      }

      try {
        _idm_reader.unsubscribe();
      }
      catch(...){ 
        RH_NL_ERROR("DomainManager", "Error disconnecting from IDM Channel. ");
      }

      // reset channel objects used by persistence module
      try{
        std::vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();
        for (;_iter != _eventChannels.end(); _iter++) {
             try {
               (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
             } catch ( ... ) {
             }
        }
        _eventChannels.clear();
        

        RH_NL_DEBUG("DomainManager", "Terminating EventChannelManager, but do not destroy channels " );
        _eventChannelMgr->terminate( false );

      }
      catch(...){ 
        RH_NL_ERROR("DomainManager", "Error disconnecting from all event channels. ");
      }

    }

    RH_NL_DEBUG("DomainManager", "Completed disconnectDomainManagementChannels" );

}



::ossie::events::EventChannel_ptr DomainManager_impl::createEventChannel (const std::string& cname)
{
    TRACE_ENTER(DomainManager_impl);

    ::ossie::events::EventChannel_var eventChannel = ::ossie::events::EventChannel::_nil();
    if ( _eventChannelMgr ) {

      try {
        RH_NL_DEBUG( "DomainManager", "Request event channel: " << cname  << " from EventChannelManager" );
        eventChannel = _eventChannelMgr->create( cname );
      }
      catch( const CF::EventChannelManager::ServiceUnavailable &) {
        RH_NL_ERROR( "DomainManager", "Service unvailable, Unable to create event channel: " << cname );
      }
      catch( const CF::EventChannelManager::InvalidChannelName &) {
        RH_NL_ERROR( "DomainManager", "Invalid Channel Name, Unable to create event channel: " << cname );
      }	
      catch( const CF::EventChannelManager::ChannelAlreadyExists &) {
        RH_NL_ERROR( "DomainManager", "Channel already exists, Unable to create event channel: " << cname );
      }	
      catch( const CF::EventChannelManager::OperationNotAllowed &) {
        RH_NL_ERROR( "DomainManager", "Operation not allowed, Unable to create event channel: " << cname );
      }	
      catch( const CF::EventChannelManager::OperationFailed &) {
        RH_NL_ERROR( "DomainManager", "Operation failed, Unable to create event channel: " << cname );
      }	
      catch( ... ) {
        RH_NL_ERROR( "DomainManager", "Unable to create event channel: " << cname );
      }
    }

    // RESOLVE --- move to list and persistance to ECM
    if (ossie::corba::objectExists(eventChannel)) {
      bool channelAlreadyInList = false;
      std::vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();
      std::string tmpName = cname;
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
          tmpNode.boundName = _eventChannelMgr->getFQN(cname);
          tmpNode.channel = ossie::events::EventChannel::_duplicate(eventChannel.in());
          _eventChannels.push_back(tmpNode);
        }
    }


    try {
        db.store("EVENT_CHANNELS", _eventChannels);
    } catch (const ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting change to event channels");
    }
    TRACE_EXIT(DomainManager_impl);
    return eventChannel._retn();
}


void DomainManager_impl::destroyEventChannel (const std::string& name)
{
    TRACE_ENTER(DomainManager_impl);
    if ( _eventChannelMgr ) {

      try {
        RH_NL_DEBUG("DomainManager", "Releasing channel: " << name );
        _eventChannelMgr->release(name);


      }
      catch(...){ 
        RH_NL_ERROR("DomainManager", "Error trying to release channel: " << name );
      }

    }

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

    (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
    _eventChannels.erase(_iter);

    try {
        db.store("EVENT_CHANNELS", _eventChannels);
    } catch (const ossie::PersistenceException& ex) {
        LOG_ERROR(DomainManager_impl, "Error persisting change to event channels");
    }

    TRACE_EXIT(DomainManager_impl);
}


void DomainManager_impl::destroyEventChannels()
{
    if ( _eventChannelMgr ) {
      RH_NL_DEBUG("DomainManager", "Delete Domain Mananagment Event Channels. " );
      try {
        RH_NL_DEBUG("DomainManager", "Disconnect ODM CHANNEL. " );
        if ( _odm_publisher ) {
          _odm_publisher->disconnect();
          _odm_publisher.reset();
        }
      }
      catch(...){ 
        RH_NL_ERROR("DomainManager", "Error Destroying ODM Channel. ");
      }

      try {
        RH_NL_DEBUG("DomainManager", "Disconnect IDM CHANNEL. " );
        _idm_reader.unsubscribe();
      }
      catch(...){ 
        RH_NL_ERROR("DomainManager", "Error Destroying IDM Channel. ");
      }

      try{
        std::vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();
        for (;_iter != _eventChannels.end(); _iter++) {
             try {
               (*_iter).channel = CosEventChannelAdmin::EventChannel::_nil();
               ossie::corba::Unbind( (*_iter).name, rootContext );
             } catch ( ... ) {
             }
        }
        _eventChannels.clear();
        

        RH_NL_DEBUG("DomainManager", "Terminating all event channels within EventChannelManager" );
        //boost::this_thread::sleep( boost::posix_time::milliseconds( 3000 ) );
        _eventChannelMgr->terminate();

      }
      catch(...){ 
        RH_NL_ERROR("DomainManager", "Error terminating all event channels. ");
      }

    }

    RH_NL_DEBUG("DomainManager", "Completed destroyEventChannels" );
}

CosEventChannelAdmin::EventChannel_ptr DomainManager_impl::getEventChannel(const std::string &name) {

  CosEventChannelAdmin::EventChannel_ptr ret = CosEventChannelAdmin::EventChannel::_nil();
  if ( _eventChannelMgr ) {
    try {
      ret = ossie::events::EventChannel::_duplicate( _eventChannelMgr->findChannel( name ) );
    }
    catch(...){
      // consume any exceptions
    };
  }
  return ret;
}



bool DomainManager_impl::eventChannelExists(const std::string &name) {
  
  if ( _eventChannelMgr ) {
    return _eventChannelMgr->isChannel( name );
  }
  return false;

}



unsigned int DomainManager_impl::incrementEventChannelConnections(const std::string &EventChannelName) {
    LOG_TRACE(DomainManager_impl, "Incrementing Event Channel " << EventChannelName);
    std::vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();

    while (_iter != _eventChannels.end()) {
        if ((*_iter).name == EventChannelName) {
            (*_iter).connectionCount++;
            LOG_TRACE(DomainManager_impl, "Event Channel " << EventChannelName<<" count: "<<(*_iter).connectionCount);
            return (*_iter).connectionCount;
        }
        _iter++;
    }

    LOG_TRACE(DomainManager_impl, "Event Channel " << EventChannelName<<" does not exist");
    return 0;
}

unsigned int DomainManager_impl::decrementEventChannelConnections(const std::string &EventChannelName) {
    LOG_TRACE(DomainManager_impl, "Decrementing Event Channel " << EventChannelName);
    std::vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();

    while (_iter != _eventChannels.end()) {
        if ((*_iter).name == EventChannelName) {
            (*_iter).connectionCount--;
            LOG_TRACE(DomainManager_impl, "Event Channel " << EventChannelName<<" count: "<<(*_iter).connectionCount);
            return (*_iter).connectionCount;
        }
        _iter++;
    }
    LOG_TRACE(DomainManager_impl, "Event Channel " << EventChannelName<<" does not exist");
    return 0;

}



