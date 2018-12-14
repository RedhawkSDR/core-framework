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
#include "Application_impl.h"

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
    RH_DEBUG(this->_baseLog, "Requesting Event Channel::" << cname);
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
    RH_WARN(this->_baseLog, "Unable to establish Subscriber interface to event channel:"<< cname);
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
    RH_DEBUG(this->_baseLog,"Requesting Event Channel::" << cname);
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
    
    RH_DEBUG(this->_baseLog,"Create DomainManager Publisher Object for:" << cname);
    ret = DOM_Publisher_ptr( new DOM_Publisher( evt_channel ) );
  }
  catch(...){
    RH_WARN(this->_baseLog,"Unable to establish Publisher interface to event channel:" << cname);
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

void DomainManager_impl::idmTerminationMessages(const redhawk::events::ComponentTerminationEvent& termMsg)
{
    boost::recursive_mutex::scoped_lock lock(stateAccess);
    Application_impl* application = findApplicationById(termMsg.application_id);
    if (!application) {
        ApplicationTable::iterator iter = _pendingApplications.find(termMsg.application_id);
        if (iter != _pendingApplications.end()) {
            application = iter->second;
        }
    }

    // Make the device identification as useful as possible by providing the
    // label first (if available) and then the unique identifier, which is
    // often a UUID
    std::string device_label;
    DeviceList::iterator device = findDeviceById(termMsg.device_id);
    if (device != _registeredDevices.end()) {
        device_label = "'" + (*device)->label + "' (" + termMsg.device_id + ")";
    } else {
        device_label = termMsg.device_id;
    }

    if (application) {
        application->componentTerminated(termMsg.component_id, device_label);
    } else {
        RH_WARN(this->_baseLog, "Abnormal Component Termination, Reporting Device: " << device_label
                 << " Application/Component " << termMsg.application_id << "/" << termMsg.component_id);
    }
}


void DomainManager_impl::establishDomainManagementChannels( const std::string &dburi ) {

    // Create Outgoing Domain Management (ODM) event channel
    if ( _eventChannelMgr ){

      if ( !dburi.empty() ) {
        RH_INFO(this->_baseLog, "Restoring event channel manager state");
        restorePubProxies(dburi);
        restoreSubProxies(dburi);
        restoreEventChannelRegistrations(dburi);
        RH_DEBUG(this->_baseLog, "Completed Restoring Event Channel Manager state");
	RH_INFO(this->_baseLog, "Restoring event channels file:" << dburi);
	restoreEventChannels(dburi);
      }
      

      RH_TRACE(this->_baseLog, "Establishing Domain Event Channels");

      //
      // Create ODM Channel Publisher
      //
      std::string cname;
      try {
        cname = redhawk::events::ODM_Channel_Spec;
        _odm_publisher =  publisher( cname );
        if ( _odm_publisher ) {
          RH_INFO(this->_baseLog, "Domain Channel: " << cname << " created.");
        }
        else {
          throw -1;
        }
      }
      catch(...) {
	RH_WARN(this->_baseLog, "ODM Channel create FAILED, Disabling outgoing events");
      }

      //
      // Create IDM Channel Subscriber
      //
      try{
        cname = redhawk::events::IDM_Channel_Spec;
        DOM_Subscriber_ptr  idmSubscriber = subscriber( cname  );
        if ( idmSubscriber ) {
          RH_INFO(this->_baseLog, "Domain Channel: " << cname << " created.");
          _idm_reader.setTerminationListener( this, &DomainManager_impl::idmTerminationMessages );
          _idm_reader.subscribe( subscriber( cname  ) );
        }
        else {
          throw -1;
        }
      }
      catch(...) {
	RH_WARN(this->_baseLog, "IDM Channel create FAILED, Disabling incoming events");
      }

      RH_DEBUG(this->_baseLog, "Completed Creating Domain Event Channels");
    }
    else {
      RH_WARN(this->_baseLog, "No EventChannelManager, Disabling event channel management operations.");
    }
}


void DomainManager_impl::disconnectDomainManagementChannels() {

    if ( _eventChannelMgr ) {
      RH_DEBUG(this->_baseLog, "Disconnect Domain Mananagment Event Channels. " );
      try {
        if ( _odm_publisher ) {
          _odm_publisher->disconnect();
          _odm_publisher.reset();
        }
      }
      catch(...){ 
        RH_ERROR(this->_baseLog, "Error disconnecting from  ODM Channel. ");
      }

      try {
        _idm_reader.unsubscribe();
      }
      catch(...){ 
        RH_ERROR(this->_baseLog, "Error disconnecting from IDM Channel. ");
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
        

        RH_DEBUG(this->_baseLog, "Terminating EventChannelManager, but do not destroy channels " );
        _eventChannelMgr->terminate( false );

      }
      catch(...){ 
        RH_ERROR(this->_baseLog, "Error disconnecting from all event channels. ");
      }

    }

    RH_DEBUG(this->_baseLog, "Completed disconnectDomainManagementChannels" );

}



::ossie::events::EventChannel_ptr DomainManager_impl::createEventChannel (const std::string& cname)
{
    ::ossie::events::EventChannel_var eventChannel = ::ossie::events::EventChannel::_nil();
    if ( _eventChannelMgr ) {

      try {
        RH_DEBUG(this->_baseLog, "Request event channel: " << cname  << " from EventChannelManager" );
        eventChannel = _eventChannelMgr->create( cname );
      }
      catch( const CF::EventChannelManager::ServiceUnavailable &) {
        RH_ERROR(this->_baseLog, "Service unvailable, Unable to create event channel: " << cname );
      }
      catch( const CF::EventChannelManager::InvalidChannelName &) {
        RH_ERROR(this->_baseLog, "Invalid Channel Name, Unable to create event channel: " << cname );
      }	
      catch( const CF::EventChannelManager::ChannelAlreadyExists &) {
        RH_ERROR(this->_baseLog, "Channel already exists, Unable to create event channel: " << cname );
      }	
      catch( const CF::EventChannelManager::OperationNotAllowed &) {
        RH_ERROR(this->_baseLog, "Operation not allowed, Unable to create event channel: " << cname );
      }	
      catch( const CF::EventChannelManager::OperationFailed &) {
        RH_ERROR(this->_baseLog, "Operation failed, Unable to create event channel: " << cname );
      }	
      catch( ... ) {
        RH_ERROR(this->_baseLog, "Unable to create event channel: " << cname );
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
        RH_ERROR(this->_baseLog, "Error persisting change to event channels");
    }
    return eventChannel._retn();
}


void DomainManager_impl::destroyEventChannel (const std::string& name)
{
    if ( _eventChannelMgr ) {

      try {
        RH_DEBUG(this->_baseLog, "Releasing channel: " << name );
        _eventChannelMgr->release(name);


      }
      catch(...){ 
        RH_ERROR(this->_baseLog, "Error trying to release channel: " << name );
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
        RH_ERROR(this->_baseLog, "Error persisting change to event channels");
    }
}


void DomainManager_impl::destroyEventChannels()
{
    if ( _eventChannelMgr ) {
      RH_DEBUG(this->_baseLog, "Delete Domain Mananagment Event Channels. " );
      try {
        RH_DEBUG(this->_baseLog, "Disconnect ODM CHANNEL. " );
        if ( _odm_publisher ) {
          _odm_publisher->disconnect();
          _odm_publisher.reset();
        }
      }
      catch(...){ 
        RH_ERROR(this->_baseLog, "Error Destroying ODM Channel. ");
      }

      try {
        RH_DEBUG(this->_baseLog, "Disconnect IDM CHANNEL. " );
        _idm_reader.unsubscribe();
      }
      catch(...){ 
        RH_ERROR(this->_baseLog, "Error Destroying IDM Channel. ");
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
        

        RH_DEBUG(this->_baseLog, "Terminating all event channels within EventChannelManager" );
        //boost::this_thread::sleep( boost::posix_time::milliseconds( 3000 ) );
        _eventChannelMgr->terminate();

      }
      catch(...){ 
        RH_ERROR(this->_baseLog, "Error terminating all event channels. ");
      }

    }

    RH_DEBUG(this->_baseLog, "Completed destroyEventChannels" );
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
    RH_TRACE(this->_baseLog, "Incrementing Event Channel " << EventChannelName);
    std::vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();

    while (_iter != _eventChannels.end()) {
        if ((*_iter).name == EventChannelName) {
            (*_iter).connectionCount++;
            RH_TRACE(this->_baseLog, "Event Channel " << EventChannelName<<" count: "<<(*_iter).connectionCount);
            return (*_iter).connectionCount;
        }
        _iter++;
    }

    RH_TRACE(this->_baseLog, "Event Channel " << EventChannelName<<" does not exist");
    return 0;
}

unsigned int DomainManager_impl::decrementEventChannelConnections(const std::string &EventChannelName) {
    RH_TRACE(this->_baseLog, "Decrementing Event Channel " << EventChannelName);
    std::vector < ossie::EventChannelNode >::iterator _iter = _eventChannels.begin();

    while (_iter != _eventChannels.end()) {
        if ((*_iter).name == EventChannelName) {
            (*_iter).connectionCount--;
            RH_TRACE(this->_baseLog, "Event Channel " << EventChannelName<<" count: "<<(*_iter).connectionCount);
            return (*_iter).connectionCount;
        }
        _iter++;
    }
    RH_TRACE(this->_baseLog, "Event Channel " << EventChannelName<<" does not exist");
    return 0;

}



