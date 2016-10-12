#include <iostream>
#include <cstddef>
#include <sstream>
#include <vector>
#include <uuid/uuid.h>
#include <boost/thread/thread.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <omniORB4/omniURI.h>
#include "ossie/debug.h"
#include "ossie/CorbaIterator.h"
#ifndef CPPUNIT_TEST
#include "DomainManager_impl.h"
#endif
#include "EventChannelManager.h"


#define ECM_TRACE( fname, expression ) RH_NL_TRACE( "EventChannelManager", fname << "--"<< expression )
#define ECM_DEBUG( fname, expression ) RH_NL_DEBUG( "EventChannelManager", fname << "--"<< expression )
#define ECM_INFO( fname, expression )  RH_NL_INFO( "EventChannelManager", fname << "--"<< expression )
#define ECM_WARN( fname, expression )  RH_NL_WARN( "EventChannelManager", fname << "--"<< expression )
#define ECM_ERROR( fname, expression ) RH_NL_ERROR( "EventChannelManager", fname << "--" << expression )
#define ECM_FATAL( fname, expression ) RH_NL_FATAL( "EventChannelManager", fname << "--"<< expression )



typedef  ossie::corba::Iterator< CF::EventChannelManager::EventChannelInfo,
                                   CF::EventChannelManager::EventChannelInfo_out,
                                   CF::EventChannelManager::EventChannelInfoList,
                                   CF::EventChannelManager::EventChannelInfoList_out,
                                   CF::EventChannelInfoIterator,
                                   POA_CF::EventChannelInfoIterator >   EventChannelInfoIter;

typedef  ossie::corba::Iterator< CF::EventChannelManager::EventRegistrant,
                                   CF::EventChannelManager::EventRegistrant_out,
                                   CF::EventChannelManager::EventRegistrantList,
                                   CF::EventChannelManager::EventRegistrantList_out,
                                   CF::EventRegistrantIterator,
                                   POA_CF::EventRegistrantIterator >   EventRegistrantIter;


#ifdef CPPUNIT_TEST
  EventChannelManager::EventChannelManager ( const bool use_fqn,
                                             const bool enableNS,
                                             const bool allow_es ) :
    EventChannelManagerBase(),
    _event_channel_factory(),
    _use_naming_service( enableNS ),
    _use_fqn(use_fqn),
    _allow_es_resolve(allow_es),
    _default_poll_period(20000000)
  {
    ECM_DEBUG("CTOR", "Creating EventChannel Manager ");

    ECM_DEBUG("CTOR", "Initializing EventChannel Manager ... ");
    try {
       _getEventChannelFactory();  
      ECM_DEBUG("CTOR", "Establish Event Channel Factory Reference ... ");
    }
    catch(...){
    }

  }
#else
  EventChannelManager::EventChannelManager (DomainManager_impl * domainManager, 
                                            const bool use_fqn,
                                            const bool enableNS,
                                            const bool allow_es ) :
    EventChannelManagerBase(),
    _domainManager(domainManager),
    _event_channel_factory(),
    _use_naming_service( enableNS ),
    _use_fqn(use_fqn),
    _allow_es_resolve(allow_es),
    _default_poll_period(20000000)
  {
    ECM_TRACE("CTOR", "Creating EventChannel Manager ");

    if ( domainManager ) _domain_context = domainManager->getDomainManagerName();

    ECM_TRACE("CTOR", "Initializing EventChannel Manager ... ");
    try {
       _getEventChannelFactory();  
       ECM_TRACE("CTOR", "Establish Event Channel Factory Reference ... ");
    }
    catch(...){
    }

  }
#endif


EventChannelManager::~EventChannelManager () {
  ECM_TRACE("DTOR", "DTOR ENTER");
  // call terminate to clean up resources
  terminate();
  ECM_TRACE("DTOR", "DTOR EXIT");
}


void EventChannelManager::_initialize () {

}

  //
  // Public interface for DomainManager to use
  //

void EventChannelManager::terminate ( const bool destroyChannels ) {

  ECM_DEBUG("terminate", " Terminate START" );
  SCOPED_LOCK(_mgrlock);

  ECM_DEBUG("terminate", "Event Channel Registry Size:" << _channels.size() );
  ChannelRegistrationTable::iterator  iter = _channels.begin();
  for ( ; iter != _channels.end(); iter++ ) {
    try {
      ECM_DEBUG("terminate", "Removing Channel: " << iter->first );
      ECM_DEBUG("terminate", "    ChannelRecord: name:" << iter->second.channel_name );
      ECM_DEBUG("terminate", "    ChannelRecord: fqn:" << iter->second.fqn );
      ECM_DEBUG("terminate", "    ChannelRecord: autoRelease:" << iter->second.autoRelease );
      ECM_DEBUG("terminate", "    ChannelRecord: release:" << iter->second.release );
      ECM_DEBUG("terminate", "    ChannelRecord: registrants:" << iter->second.registrants.size());
      ECM_DEBUG("terminate", "    ChannelRecord: channel:" << iter->second.channel);
      //if ( CORBA::is_nil(iter->second.channel) == true ) {
      if ( ossie::corba::objectExists(iter->second.channel) == false ) {
	ECM_DEBUG("terminate", " Channel is NIL : " << iter->first );
      }
      else {
	ECM_DEBUG("terminate", " Destroy EventChannel PRE: " << iter->first );
        if ( destroyChannels ) {
          iter->second.channel->destroy();
        }
	ECM_DEBUG("terminate", " Destroy EventChannel POST: " << iter->first );
      }
      iter->second.channel = ossie::events::EventChannel::_nil();
    }
    catch(CORBA::OBJECT_NOT_EXIST){
      ECM_ERROR("Terminate", "Remove Channel FAILED, CHANNEL:" << iter->first << " REASON: Object does not exists");
    }
    catch(...){
      ECM_ERROR("Terminate","Remove Channel FAILED, CHANNEL:" << iter->first << " Possible legacy channels in event service ");
    }
  }

  _channels.clear();
  ECM_DEBUG("terminate", " Terminate COMPLETED " );

}

const ossie::events::EventChannel_ptr EventChannelManager::findChannel( const std::string &channel_name )
  {
    SCOPED_LOCK(_mgrlock);
    ossie::events::EventChannel_ptr ret = ossie::events::EventChannel::_nil();
    
    // get the event channel factory... throws ServiceUnavailable
    _getEventChannelFactory();

    //
    // check if a registration record exists for this channel .. 
    //
    ChannelRegistrationPtr creg = _getChannelRegistration( channel_name );
    if ( creg ) {
      ret = creg->channel;
    }

    return ret;
  }


  bool EventChannelManager::isChannel( const std::string &channel_name )
  {
    SCOPED_LOCK(_mgrlock);
    return _channelExists(channel_name );
  }

  std::string EventChannelManager::getFQN( const std::string &cname, const std::string &nc_name  ) {
    SCOPED_LOCK(_mgrlock);
    return _getFQN( cname, nc_name );
  }

  void EventChannelManager::setPollingPeriod( const int64_t  period ) {
     SCOPED_LOCK(_mgrlock);
    _default_poll_period = period;
  }

  void EventChannelManager::markForRegistrations( const char *channel_name) 
    throw ( CF::EventChannelManager::ChannelDoesNotExist, 
	    CF::EventChannelManager::OperationFailed, 
	    CF::EventChannelManager::OperationNotAllowed,
	    CF::EventChannelManager::ServiceUnavailable )
  {
     SCOPED_LOCK(_mgrlock);
    // get the event channel factory... throws ServiceUnavailable if factory is not resolved
    _getEventChannelFactory();

    std::string cname(channel_name);
    ChannelRegistrationPtr reg = _getChannelRegistration( cname );

    // channel registration entry does not exists 
    if ( reg == NULL ) {
      throw (CF::EventChannelManager::ChannelDoesNotExist());
    }

    reg->autoRelease = true;
    ECM_DEBUG( "markForRegistrationr",  " EventChannel: " << cname << " marked for autoRelease" );
  }


  void EventChannelManager::release( const std::string &channel_name) 
    throw ( CF::EventChannelManager::ChannelDoesNotExist, 
            CF::EventChannelManager::RegistrationsExists, 
	    CF::EventChannelManager::OperationFailed, 
	    CF::EventChannelManager::OperationNotAllowed,
	    CF::EventChannelManager::ServiceUnavailable ) {
     SCOPED_LOCK(_mgrlock);
    _release(channel_name.c_str() );
  }


  void EventChannelManager::release( const char *channel_name) 
    throw ( CF::EventChannelManager::ChannelDoesNotExist, 
            CF::EventChannelManager::RegistrationsExists, 
	    CF::EventChannelManager::OperationFailed, 
	    CF::EventChannelManager::OperationNotAllowed,
	    CF::EventChannelManager::ServiceUnavailable ) {
    SCOPED_LOCK(_mgrlock);
    _release(channel_name);
  }


  void EventChannelManager::_release( const std::string &channel_name) 
    throw ( CF::EventChannelManager::ChannelDoesNotExist, 
            CF::EventChannelManager::RegistrationsExists, 
	    CF::EventChannelManager::OperationFailed, 
	    CF::EventChannelManager::OperationNotAllowed,
	    CF::EventChannelManager::ServiceUnavailable )
  {

    // get the event channel factory... throws ServiceUnavailable if factory is not resolved
    _getEventChannelFactory();

    ECM_DEBUG( "release", " Delete event channel: " << channel_name );    
    std::string cname(channel_name);

    ECM_DEBUG( "release", " Check registration for event channel: " << channel_name );    
    ChannelRegistrationPtr reg = _getChannelRegistration( cname );

    // channel registration entry does not exists 
    if ( reg == NULL ) {
      ECM_DEBUG( "release", " Registration DOES NOT EXISTS event channel: " << channel_name );    
      throw (CF::EventChannelManager::ChannelDoesNotExist());
    }

    // check if anyone is still registered
    if ( reg->nregistrants() > 0 ) {
      ECM_DEBUG( "release", " Registrations still exists: " << channel_name );    
      // mark channel for deletion when registrations are emptied
      reg->autoRelease = true;
      reg->release = true;
      throw (CF::EventChannelManager::RegistrationsExists());
    }

    //
    // if naming service registry is used then unbind the object 
    //
    if ( _use_naming_service ) {
      ECM_DEBUG( "release", " Remove channel from NamingService: " << channel_name );    
      ossie::corba::Unbind( cname, _domain_context );        }

    // destroy the event channel and delete the registration
    try {
      ECM_DEBUG( "release", " Delete the registration: " << channel_name );    
      // delete the registration
      _deleteChannelRegistration( cname );
    } 
    catch(CORBA::SystemException& ex) {
      // this will happen if channel is destroyed but 
      ECM_WARN("release", " Event Channel RELEASE operation, System exception occured, ex " << ex._name() );
      throw (CF::EventChannelManager::OperationFailed());
    }
    catch(CORBA::Exception& ex) {
      ECM_WARN("release", " Event Channel RELEASE operation, CORBA exception occured, ex " << ex._name() );
      throw (CF::EventChannelManager::OperationFailed());
    }
    

    ECM_DEBUG( "release", " Released EventChannel: " << cname );

  }


ossie::events::EventChannel_ptr EventChannelManager::create( const std::string &channel_name) 
    throw ( CF::EventChannelManager::ChannelAlreadyExists, 
	    CF::EventChannelManager::OperationFailed, 
	    CF::EventChannelManager::OperationNotAllowed,
	    CF::EventChannelManager::ServiceUnavailable )
  {
    SCOPED_LOCK(_mgrlock);
    return _create( channel_name );
    
  }



  ossie::events::EventChannel_ptr EventChannelManager::create( const char *channel_name) 
    throw ( CF::EventChannelManager::ChannelAlreadyExists, 
	    CF::EventChannelManager::OperationFailed, 
	    CF::EventChannelManager::OperationNotAllowed,
	    CF::EventChannelManager::ServiceUnavailable )
  {
    SCOPED_LOCK(_mgrlock);
    return _create( channel_name );
    
  }


  ossie::events::EventChannel_ptr EventChannelManager::createForRegistrations( const char *channel_name) 
    throw ( CF::EventChannelManager::ChannelAlreadyExists, 
	    CF::EventChannelManager::OperationFailed, 
	    CF::EventChannelManager::OperationNotAllowed,
	    CF::EventChannelManager::ServiceUnavailable )
  {
    SCOPED_LOCK(_mgrlock);
    return _create( channel_name, true );
    
  }


  ossie::events::EventChannel_ptr EventChannelManager::_create( const std::string &channel_name, const bool autoRelease) 
    throw ( CF::EventChannelManager::ChannelAlreadyExists, 
	    CF::EventChannelManager::OperationFailed, 
	    CF::EventChannelManager::OperationNotAllowed,
	    CF::EventChannelManager::ServiceUnavailable )
  {
    // get the event channel factory... throws ServiceUnavailable if factory is not resolved
    _getEventChannelFactory();

    //
    // validate channel name...
    //
    if ( _validateChannelName( channel_name ) == false ) {
      throw ( CF::EventChannelManager::InvalidChannelName());
    }

    //
    // check if channel name is already registered
    //
    if ( _channelExists( channel_name ) ) {
      ECM_DEBUG("create", "Event channel: "<< channel_name << " exists, in the local domain");
      throw (CF::EventChannelManager::ChannelAlreadyExists());
    }

    //
    // check if channel name is already exists in the event service
    //
    ossie::events::EventChannel_var event_channel = ossie::events::EventChannel::_nil();
    std::string   cname(channel_name);
    std::string   fqn = _getFQN(cname);
    bool          require_ns = false;           // if channel exists and use_nameing_service is enabled
    event_channel = _resolve_es( cname, fqn );

    // if we found a matching channel
    if ( !CORBA::is_nil(event_channel) ) {

      // if naming service disabled then throw
      if ( _use_naming_service == false && _allow_es_resolve == false ) {
        ECM_WARN("EventChannelManager", "Event Channel: "<< channel_name << " exists in EventService!");
        throw (CF::EventChannelManager::ChannelAlreadyExists());
      }
      else {
        if ( _allow_es_resolve == false ) {
          // set next search method to require use of NS
          require_ns=true;
          ECM_DEBUG("EventChannelManager", "Event Channel: "<< channel_name << " exists,  requiring NamingService resolution.");
        }
        else {
          ECM_DEBUG("EventChannelManager", "Event Channel: "<< channel_name << " exists,  using EventService resolution.");
        }
      }
    }
    
    //
    // try and resolve with naming service (if enabled)
    //
    if ( require_ns ) {
      ECM_TRACE( "_createChannel", " Checking NamingService for:" << fqn );
      event_channel = _resolve_ns( cname, fqn, _domain_context );

      // if NamingService is enable and we require its use, but channel evaluation failed
      if ( CORBA::is_nil(event_channel) && require_ns) {
        throw (CF::EventChannelManager::OperationFailed());
      }        
    }
  
    //
    // Create an event channel for use in the domain
    // this throws OperationFailed or OperationNotAllowed
    //
    if ( CORBA::is_nil(event_channel) ) {    
      event_channel = _createChannel( cname, fqn, _domain_context, require_ns );  
    }

    if ( CORBA::is_nil(event_channel) == true ) {
      ECM_ERROR("EventChannelManager", "Create Event Channel failed.  channel: "<< channel_name );
      throw (CF::EventChannelManager::OperationFailed());      
    }

    ECM_TRACE("create", 
            "ADD Channel Registration, Event Channel: "<< channel_name << " fqn:" << fqn );
    ChannelRegistrationPtr reg  __attribute__((unused)) = _addChannelRegistration( channel_name, fqn, event_channel, autoRelease ); 

    //
    // return pointer the channel... we maintain a separate copy
    //
    ECM_TRACE("create", "Completed create Event Channel: "<< channel_name );
    return event_channel._retn();
  }



void EventChannelManager::restore( ossie::events::EventChannel_ptr savedChannel, 
                                   const std::string &channel_name, 
                                   const std::string &fqn_name) 
    throw ( CF::EventChannelManager::ChannelAlreadyExists, 
	    CF::EventChannelManager::OperationFailed, 
	    CF::EventChannelManager::OperationNotAllowed,
	    CF::EventChannelManager::ServiceUnavailable )
  {
    // get the event channel factory... throws ServiceUnavailable if factory is not resolved
    _getEventChannelFactory();

    //
    // validate channel name...
    //
    if ( _validateChannelName( channel_name ) == false ) {
      throw ( CF::EventChannelManager::InvalidChannelName());
    }

    //
    // check if channel name is already registered
    //
    if ( _channelExists( channel_name ) ) {
      ECM_DEBUG("restore", "Event channel: "<< channel_name << " exists, in the local domain");
      throw (CF::EventChannelManager::ChannelAlreadyExists());
    }

    //
    // check if channel name is already exists in the event service
    //
    ossie::events::EventChannel_var event_channel = ossie::events::EventChannel::_nil();
    std::string   cname(channel_name);
    std::string fqn;
    if ( fqn_name.empty() )  {
      fqn = _getFQN(cname);
    }
    else {
      fqn = fqn_name;
    }
    bool          require_ns = false;           // if channel exists and use_naming_service is enabled

    //
    // try and resolve with naming service (if enabled)
    //
    if ( _use_naming_service &&  CORBA::is_nil(event_channel) ) {
      ECM_TRACE( "restore", " Rebind Exiting Channel to:" << fqn );
      if ( ossie::corba::Bind( fqn, savedChannel, _domain_context ) != 0 ) {
        ECM_TRACE( "restore", " Checking NamingService for:" << fqn );
        event_channel = _resolve_ns( cname, fqn, _domain_context );
        // if NamingService is enable and we require its use, but channel evaluation failed
        if ( CORBA::is_nil(event_channel) && require_ns) {
          throw (CF::EventChannelManager::OperationFailed());
        }        
      }
      else {
        event_channel = ossie::events::EventChannel::_duplicate(savedChannel);
      }
    }

    //
    // Create an event channel for use in the domain
    // this throws OperationFailed or OperationNotAllowed
    //
    if ( CORBA::is_nil(event_channel) ) {    
      event_channel = _createChannel( cname, fqn, _domain_context, require_ns );  
    }

    if ( CORBA::is_nil(event_channel) == true ) {
      ECM_ERROR("EventChannelManager", "Create Event Channel failed.  channel: "<< channel_name );
      throw (CF::EventChannelManager::OperationFailed());      
    }

    ECM_DEBUG("restore", 
            "ADD Channel Registration, Event Channel: "<< channel_name << " fqn:" << fqn );
    ChannelRegistrationPtr reg  __attribute__((unused)) = _addChannelRegistration( channel_name, fqn, event_channel, false ); 

    //
    // return pointer the channel... we maintain a separate copy
    //
    ECM_DEBUG("restore", "Completed create Event Channel: "<< channel_name );
  }



  ossie::events::EventChannelReg_ptr EventChannelManager::registerResource( const ossie::events::EventRegistration &request)  
    throw ( CF::EventChannelManager::InvalidChannelName, 
	    CF::EventChannelManager::RegistrationAlreadyExists,
	    CF::EventChannelManager::OperationFailed, 
	    CF::EventChannelManager::OperationNotAllowed,
	    CF::EventChannelManager::ServiceUnavailable )
  {

    ECM_DEBUG("registerResource", "REQUEST REGISTRATION , REG-ID:" << request.reg_id << " CHANNEL:" << request.channel_name );
    SCOPED_LOCK(_mgrlock);

    // get the event channel factory... throws ServiceUnavailable
    _getEventChannelFactory();

    //
    // check if registration exists
    //
    std::string regid(request.reg_id.in());
    std::string channel_name(request.channel_name.in());
    if (regid.empty()) {
      regid = _generateRegId();
      while (_regIdExists( channel_name, regid ) == true) {
        regid = _generateRegId();
      }
    }
    if (  _regIdExists( channel_name, regid ) == true ) {
      throw ( CF::EventChannelManager::RegistrationAlreadyExists());
    }

    //
    // validate channel name...
    //
    if ( _validateChannelName( channel_name ) == false ) {
      throw ( CF::EventChannelManager::InvalidChannelName());
    }
    //
    // check if a registration record exists for this channel .. 
    //
    ChannelRegistrationPtr creg = _getChannelRegistration( channel_name );
    if ( creg ) {
      // channel is marked for deletion..... 
      if (creg->release ) throw ( CF::EventChannelManager::OperationNotAllowed());    
    }
    else {

      // need to create a channel....
      try {
        ossie::events::EventChannel_var ch  __attribute__((unused)) = _create(channel_name);         // adds to list of channel registrations automatically
      }
      catch( CF::EventChannelManager::ChannelAlreadyExists ){
        ECM_ERROR("registerResource", "REGISTRATION ERROR, REG-ID:" << regid << " CHANNEL:" << channel_name  << " Channel exists in EventService" );
        throw (CF::EventChannelManager::OperationFailed());    
      }
      creg = _getChannelRegistration( channel_name );
      if ( creg == NULL ) throw ( CF::EventChannelManager::OperationFailed());    
    }

    ossie::events::EventChannelReg_ptr reg = new ossie::events::EventChannelReg();
    reg->reg.channel_name = CORBA::string_dup(channel_name.c_str());
    reg->reg.reg_id = CORBA::string_dup(regid.c_str());
    reg->channel = ossie::events::EventChannel::_duplicate(creg->channel);

    std::string ior = ossie::corba::objectToString( reg->channel );
    ECM_DEBUG("register", "NEW REGISTRATION FOR:" << ior );
    creg->registrants.insert( RegRecord( regid, ior ) );

    ECM_DEBUG("registerResource", "NEW REGISTRATION REG-ID:" << regid << " CHANNEL:" << channel_name );

    //
    // Release memory to the caller....they will clean up
    //
    return reg;
  }




  /*
     Unregister from an event channel and invalidates the context
  */
  void EventChannelManager::unregister( const ossie::events::EventRegistration &reg ) 
    throw ( CF::EventChannelManager::ChannelDoesNotExist,
	    CF::EventChannelManager::RegistrationDoesNotExist,
	    CF::EventChannelManager::ServiceUnavailable ) 
  {
    SCOPED_LOCK(_mgrlock);

    ECM_DEBUG("unregister", "REQUEST TO UNREGISTER, REG-ID:" << reg.reg_id  << " CHANNEL:" << reg.channel_name);
    // get the event channel factory... throw ServiceUnavailable
    _getEventChannelFactory();

    _unregister(reg);

    ECM_DEBUG("unregister", "UNREGISTER COMPLETED, REG-ID:" << reg.reg_id  << " CHANNEL:" << reg.channel_name);
  }


  /*
     Unregister from an event channel and invalidates the context
  */
  void EventChannelManager::_unregister( const ossie::events::EventRegistration &reg ) 
    throw ( CF::EventChannelManager::ChannelDoesNotExist,
	    CF::EventChannelManager::RegistrationDoesNotExist,
	    CF::EventChannelManager::ServiceUnavailable ) 
  {
    ECM_DEBUG("unregister", "START:  ID:" << reg.reg_id );
    //
    // check channel and registration existence
    //
    std::string regid(reg.reg_id);
    std::string cname(reg.channel_name);

    ECM_DEBUG("unregister", "CHECK REGISTRATION,  ID:" << regid<< " CHANNEL:" << cname);
    _regExists( cname, regid );

    // get the registration record for this channel 
    ECM_DEBUG("unregister", "GET REGISTRATION RECORD, ID:" << reg.reg_id );
    ChannelRegistrationPtr creg = _getChannelRegistration( cname );
    if (creg) {
      //  search for registration entry and remove registration from the list,
      RegIdList::iterator itr = creg->registrants.find(regid);
      if ( itr != creg->registrants.end() )  {
        ECM_DEBUG("unregister", "ERASE REGISTRATION RECORD, REG-ID:" << reg.reg_id );        
        creg->registrants.erase(itr);
      }

      if ( creg->registrants.size()==0 and creg->autoRelease ) {
        ECM_DEBUG("EventChannelManager", "NO MORE REGISTRATIONS, (AUTO-RELEASE IS ON), DELETING CHANNEL:" << cname  );        
        _deleteChannelRegistration(cname);
      }
    }

    ECM_DEBUG("unregister", "UNREGISTER END:  REG-ID:" << reg.reg_id );
  }



  void EventChannelManager::listChannels( CORBA::ULong how_many, 
                                          CF::EventChannelManager::EventChannelInfoList_out elist, 
                                          CF::EventChannelInfoIterator_out eiter) {


    SCOPED_LOCK(_mgrlock);
    uint64_t size = _channels.size();
    ECM_TRACE( "listChannels", " listChannel context " << this << ", how_many " << how_many << ", size " << size );

    // create copy of entire table...
    ossie::events::EventChannelInfoList* all = new ossie::events::EventChannelInfoList(size);
    all->length(size);

    ChannelRegistrationTable::iterator iter =  _channels.begin();
    for ( int i=0; iter != _channels.end() ; iter++,i++ ){
      std::string cname(iter->first.c_str());
      (*all)[i].channel_name = CORBA::string_dup(cname.c_str());
      (*all)[i].reg_count = iter->second.registrants.size();
      ECM_DEBUG("listChannels", " list channel context: (" << i << ") channel_name: " <<  iter->first <<
              " N registrants: " << iter->second.registrants.size() );
    }

    eiter = EventChannelInfoIter::list( how_many, elist, all );

  }

  void EventChannelManager::listRegistrants( const char* channel_name, 
                                             CORBA::ULong how_many, 
                                             CF::EventChannelManager::EventRegistrantList_out rlist, 
                                             CF::EventRegistrantIterator_out riter) {

    SCOPED_LOCK(_mgrlock);
    ChannelRegistrationPtr reg = _getChannelRegistration( channel_name );
    if ( reg == 0 ) {
      // no results
      // result of this call
      riter = CF::EventRegistrantIterator::_nil();
      rlist->length(0);
      return;
    }      

    // get number of registrants to 
    uint64_t size = reg->nregistrants();
    ECM_TRACE( "listRegistrants", " list channel registrants context " << this << ", how_many " << how_many << ", size " << size );

    // create copy of entire table...
    ossie::events::EventRegistrantList* all = new ossie::events::EventRegistrantList(size);
    all->length(size);

    RegIdList::iterator iter =  reg->registrants.begin();
    for ( int i=0; iter != reg->registrants.end() ; iter++,i++ ){
      std::string cname(channel_name);
      (*all)[i].channel_name = CORBA::string_dup(cname.c_str());
      (*all)[i].reg_id = CORBA::string_dup( iter->first.c_str() );
      ECM_DEBUG("listRegistrants", " Registrant : (" << i << ") regid: " <<  iter->first );
    }

    riter = EventRegistrantIter::list( how_many, rlist, all );
  }


  //
  // Private Methods Section
  //

  //
  // Returns fully qualified channel name that was registred with the underlying EventService
  //
  std::string EventChannelManager::_getFQN( const std::string &cname, const std::string &nc_name  ) {

    // return a fully qualified channel name that can be used outside of the domain
    std::string fqn(cname);
    if ( _use_fqn ) {
#ifndef CPPUNIT_TEST
      if ( _domainManager ) {
        fqn = _domainManager->getDomainManagerName() + "." + cname;
      }
      else if ( nc_name != "" ) {
	fqn = nc_name + "." + cname;
      }
#else
      if ( nc_name != "" ) {
	fqn = nc_name + "." + cname;
      }
#endif
      else {
        fqn = "_ANONYMOUS_." + cname;
      }
    }
    else if ( nc_name != "" ) {
	fqn = nc_name + "." + cname;
    }


    ECM_TRACE("_getFQN", "CHANNEL:" << cname << " FQN:" << fqn);
    return fqn; 

  }


void EventChannelManager::_getEventChannelFactory ()
    throw  ( CF::EventChannelManager::ServiceUnavailable )
  {

    ECM_TRACE("_getEventChannelFactory", " .. Checking ORB Context");
    if ( CORBA::is_nil(_orbCtx.orb) == true ) {
      ECM_TRACE("_getEventChannelFactory", " ... ORB Context is invalid...");
      throw (CF::EventChannelManager::ServiceUnavailable() );
    }

    if ( CORBA::is_nil(_event_channel_factory )  ) {

      // Attempt to locate the OmniEvents event channel factory to create the event channels.
      // First, check for an initial reference in the omniORB configuration; if it cannot be
      // resolved in this manner, look it up via the naming service.
      CORBA::Object_var factoryObj;
      ECM_TRACE("_getEventChannelFactory", " ... Get EventChannelFactory...");
      try {
	  factoryObj = _orbCtx.namingServiceCtx->resolve_str("EventChannelFactory");
      } catch (const CosNaming::NamingContext::NotFound&) {
	ECM_DEBUG("_getEventChannelFactory",  "No naming service entry for 'EventChannelFactory'");
      } catch (const CORBA::Exception& e) {
	ECM_WARN( "_getEventChannel", "CORBA " << e._name() << " exception looking up EventChannelFactory in name service");
      }

      if (!CORBA::is_nil(factoryObj)) {
	try {
	  if (!factoryObj->_non_existent()) {
	    _event_channel_factory = CosLifeCycle::GenericFactory::_narrow(factoryObj);
            ECM_TRACE("_getEventChannelFactory", "Resolved EventChannelFactory in NameService");
	  }
	} catch (const CORBA::TRANSIENT&) {
	  ECM_WARN( "_getEventChannelEvent", "Could not contact EventChannelFactory");
	} 
      }
    }

    if ( CORBA::is_nil(_event_channel_factory) ) {
      ECM_TRACE( "_getEventChannelEvent", "EventChannelFactory unavailable.");
      throw (CF::EventChannelManager::ServiceUnavailable() );
    }

  }



  ossie::events::EventChannel_ptr EventChannelManager::_createChannel( const std::string &cname, 
                                                                      const std::string &fqn, 
                                                                      const std::string &nc_name,
                                                                      const bool require_ns ) 
    throw ( CF::EventChannelManager::ChannelAlreadyExists, 
	    CF::EventChannelManager::OperationFailed, 
	    CF::EventChannelManager::OperationNotAllowed,
	    CF::EventChannelManager::ServiceUnavailable )
  {
    // resolve factory.... this throws.... ServiceUnavailable
    _getEventChannelFactory();

    //
    // if we are at this point then prior methods are responsible for checking if channel already exits
    //
    ossie::events::EventChannel_var  event_channel = ossie::events::EventChannel::_nil();

    CosLifeCycle::Key key;
    key.length(1);
    key[0].id = "EventChannel";
    key[0].kind = "object interface";

    ECM_TRACE( "_createChannel", " Check EventChannelFactory API" );
    if(!_event_channel_factory->supports(key)) {
      ECM_WARN( "Create Event Channel", " EventChannelFactory does not support Event Channel Interface!" );
      throw (CF::EventChannelManager::OperationNotAllowed());
    }

    std::string insName = fqn;   // create channel based on fully qualified name

    CosLifeCycle::Criteria criteria;
    criteria.length(2);
    criteria[0].name = CORBA::string_dup("InsName");
    criteria[0].value <<=insName.c_str();
    criteria[1].name = CORBA::string_dup("CyclePeriod_ns");
    criteria[1].value <<= (CORBA::ULong)_default_poll_period;

    //
    // Create Event Channel Object.
    //
    ECM_TRACE( "_createChannel", " Create CHANNEL:" << cname << " AS FQN:"  << fqn );

    CORBA::Object_var obj;
    try {
      obj =_event_channel_factory->create_object(key, criteria);
    }
    catch (CosLifeCycle::CannotMeetCriteria& ex) /* create_object() */ {
      ECM_ERROR( "Create Event Channel", "Create failed, CHANNEL: " << cname << " REASON: CannotMeetCriteria " );
      throw (CF::EventChannelManager::OperationFailed());
    }
    catch (CosLifeCycle::InvalidCriteria& ex) /* create_object() */ {
      ECM_ERROR( "Create Event Channel", "Create failed, CHANNEL: " << cname << " REASON: InvalidCriteria " );
      if(ex.invalid_criteria.length()>0) {
	int j;
	for (  j=0; (unsigned int)j < ex.invalid_criteria.length(); j++ ) { 
	  ECM_TRACE( "_createChannel", "--- Criteria Name: " << ex.invalid_criteria[j].name );
	  if ( j == 0 ) {
	    const char * xx;
	    ex.invalid_criteria[j].value >>= xx;
	    ECM_TRACE( "_createChannel", "--- Criteria Value : " << xx );
	  }
	  else {
	    CORBA::ULong xx;
	    ex.invalid_criteria[j].value >>= xx;
	    ECM_TRACE( "_createChannel", "--- Criteria Value : " << xx );
	  }
	}
      }
    
      throw (CF::EventChannelManager::OperationFailed());
    }
    catch( CORBA::Exception &ex ) {
      ECM_ERROR( "Create Event Channel", " Create failed, CHANNEL:" << cname << " REASON: corba exception" );
      throw CF::EventChannelManager::OperationFailed();
    }

    if (CORBA::is_nil(obj)) {
      ECM_ERROR( "Create Event Channel", " Create failed, CHANNEL:" << cname << " REASON: Factory failed to create channel");
      throw CF::EventChannelManager::OperationFailed();
    }

    try {
      ECM_TRACE( "_createChannel", " action - Narrow EventChannel" );
      event_channel = ossie::events::EventChannel::_narrow(obj);
    }
    catch( CORBA::Exception &ex ) {
      ECM_ERROR( "Create Event Channel", " Create failed, CHANNEL:" << cname << " REASON: Failed to narrow to EventChannel");
    }
    ECM_TRACE( "_createChannel", " created event channel " << cname );

    if ( _use_naming_service ){
      try {
        // bind channel using channel name and not fqn name
      	ossie::corba::Bind( cname, event_channel.in(), nc_name, false );
      } 
      catch (const CORBA::Exception& ex) {
	ECM_ERROR( "Create Event Channel", " Bind Failed, CHANNEL:" << cname << " REASON: CORBA " << ex._name() );
      }
    }

    ECM_TRACE( "_createChannel", " completed create event channel : " << cname );
    return event_channel._retn();
  }


ossie::events::EventChannel_ptr EventChannelManager::_resolve_ns( const std::string &cname, 
                                                                 const std::string &fqn, 
                                                                 const std::string &nc_name)  {
  ECM_DEBUG("_resolve",  " : resolve event channel with NamingService cname/fqn... " << cname << "/" << fqn );

  // return value if no event channel was found or error occured
  ossie::events::EventChannel_var event_channel = ossie::events::EventChannel::_nil();
  bool found = false;

  //
  // try to resolve using channel name as InitRef  and resolve_initial_references
  //
  try {
    ECM_TRACE( "_resolve_ns", " : Trying InitRef Lookup " << fqn);
    CORBA::Object_var obj = _orbCtx.orb->resolve_initial_references(fqn.c_str());
    if ( CORBA::is_nil(obj) == false ){
      event_channel = ossie::events::EventChannel::_narrow(obj);
      found =true;
      ECM_TRACE( "_resolve_ns", " : FOUND EXISTING (InitRef), Channel " << cname );
    }
  }catch (const CORBA::Exception& e) {
    ECM_DEBUG( "_resolve_ns", "  Unable to lookup with InitRef:" << fqn << ",  CORBA RETURNED(" << e._name() << ")" );
  }

  /*
   * try and resolve channel as follows..
   *      get context for nc_name... resolve( fully qualified name (domain.channel) )
   *      get context for nc_name... resolve( channel name )
   *      get context for root context resolve( fully qualified name (domain.channel) )
   */

  if ( !found ) {
    CosNaming::Name_var boundName = ossie::corba::stringToName(fqn);
    try {
      CosNaming::NamingContext_ptr context = ossie::corba::ResolveNamingContextPath( nc_name );
      if ( !CORBA::is_nil(context) ) {
        CORBA::Object_var obj = context->resolve(boundName);
        ECM_TRACE( "_resolve_ns", " : FOUND EXISTING (NamingService - domain/domain.channel), Channel/FQN " << cname  << "/" << fqn );
        event_channel = ossie::events::EventChannel::_narrow(obj);
        found = true;
      }
    } catch (const CosNaming::NamingContext::NotFound&) {
      // The channel does not exist and can be safely created.
    } catch (const CORBA::Exception& e) {
      ECM_WARN("Name Resolution", " Naming Service failed for CHANNEL:" << cname << " REASON: CORBA (" << e._name() << ")" );
    } catch (...) {
      ECM_DEBUG("Name Resolution", " Naming Service failed ... (NamingService - domain/domain.channel) Channel/FQN " << cname  << "/" << fqn );
    }
  }

  if ( !found ) {
    CosNaming::Name_var boundName = ossie::corba::stringToName(cname);
    try {
      CosNaming::NamingContext_ptr context = ossie::corba::ResolveNamingContextPath( nc_name );
      if ( !CORBA::is_nil(context) ) {
        CORBA::Object_var obj = context->resolve(boundName);
        ECM_TRACE( "_resolve_ns", " : FOUND EXISTING (NamingService - domaincontext/channel ), Channel/FQN " << cname  << "/" << fqn );
        event_channel = ossie::events::EventChannel::_narrow(obj);
        found = true;
      }
    } catch (const CosNaming::NamingContext::NotFound&) {
      // The channel does not exist and can be safely created.
    } catch (const CORBA::Exception& e) {
      ECM_WARN("Name Resolution", " Naming Service failed for CHANNEL:" << cname << " REASON: CORBA (" << e._name() << ")" );
    } catch (...) {
      ECM_DEBUG("Name Resolution", " Naming Service failed ... (NamingService - domaincontext/channel) Channel/FQN " << cname  << "/" << fqn );
    }
  }


  if ( !found ) {
    CosNaming::Name_var boundName = ossie::corba::stringToName(fqn);
    try {
      CosNaming::NamingContext_ptr context = ossie::corba::ResolveNamingContextPath("");
      if ( !CORBA::is_nil(context) ) {
        CORBA::Object_var obj = context->resolve(boundName);
        ECM_TRACE( "_resolve_ns", " : FOUND EXISTING (NamingService - root context/dommain.channel), Channel/FQN " << cname  << "/" << fqn );
        event_channel = ossie::events::EventChannel::_narrow(obj);
        found = true;
      }
    } catch (const CosNaming::NamingContext::NotFound&) {
      // The channel does not exist and can be safely created.
    } catch (const CORBA::Exception& e) {
      ECM_WARN("Name Resolution", " Naming Service failed for CHANNEL:" << cname << " REASON: CORBA (" << e._name() << ")" );
    } catch (...) {
      ECM_WARN("Name Resolution", " Naming Service failed for CHANNEL:" << cname  );
    }
  }


  return event_channel._retn();
}



ossie::events::EventChannel_ptr  EventChannelManager::_resolve_es ( const std::string& cname, 
                                                                   const std::string& fqn, 
                                                                   bool suppress ) {

  ECM_DEBUG("_resolve_es",  " : resolve event channel ... " << cname );

  // return value if no event channel was found or error occured
  ossie::events::EventChannel_var event_channel = ossie::events::EventChannel::_nil();

  std::string tname;
  //
  // try to resolve with corbaloc method and string_to_object method
  //
  try {
    std::ostringstream os;
    //
    // last gasp... try the corbaloc method...corbaloc::host:11169/<channel name>
    // 
    os << "corbaloc::localhost:11169/"<< fqn;
    tname=os.str();
    ECM_TRACE( "_resolve_es"," : Trying corbaloc resolution " << tname );
    CORBA::Object_var obj = _orbCtx.orb->string_to_object(tname.c_str());
    if ( !CORBA::is_nil(obj) ) {
      event_channel = ossie::events::EventChannel::_narrow(obj);
      if ( CORBA::is_nil(event_channel) == false ){
        ECM_TRACE( "_resolve_es", " : FOUND EXISTING (corbaloc), Channel " << tname );
      }
      else {
        ECM_TRACE( "_resolve_es", " : RESOLVE FAILED VIA (corbaloc), Channel " << tname );
      }
    }
    else {
      ECM_TRACE( "_resolve_es", " : SEARCH FOR Channel " << tname << " FAILED");
    }
  }catch (const CORBA::Exception& e) {
    if (!suppress)  ECM_WARN( "Event Service Lookup", 
			      "  Unable to lookup with corbaloc URI:" << tname << ", CHANNEL:" << cname << " REASON: CORBA RETURNED(" << e._name() << ")" );
  }

  return event_channel._retn();
}


  /*
     Check the registration database exists for the specified channel
     
     @returns false - channel or registration was not found
     @returns true - registration id was found for specified channel
  */
bool EventChannelManager::_regIdExists( const std::string &cname, const std::string &regid ) {

  bool retval = false;
  if ( regid.size() > 0  ) {
    ChannelRegistrationPtr reg = _getChannelRegistration( cname );
    if ( reg ) {
      if ( reg->registrants.find(regid) != reg->registrants.end() ) retval = true;
    }
  }
  return retval;
}


  /*
     Check the registration database if a specified publisher is registered
     
     @returns false - channel or registration was not found
     @returns true -  publisher registration id was found for the specified channel
  */
  bool EventChannelManager::_regExists( const std::string &cname, const std::string &regid ) 
    throw ( CF::EventChannelManager::ChannelDoesNotExist,
            CF::EventChannelManager::RegistrationDoesNotExist )
  {

    bool retval = false;
    if ( regid.size() > 0  ) {
      ChannelRegistrationPtr reg = _getChannelRegistration( cname );
      if ( reg ) {
	if ( reg->registrants.find(regid) != reg->registrants.end() ) {
          retval = true;
        }
        else {
          throw ( CF::EventChannelManager::RegistrationDoesNotExist());
        }
      }
      else {
        throw ( CF::EventChannelManager::ChannelDoesNotExist());
      }
    }
    else {
      throw ( CF::EventChannelManager::RegistrationDoesNotExist());
    }

    return retval;
  }


  /*
     check if channel name is registered
  */
  bool EventChannelManager::_channelExists( const std::string &cname ) {
    
    // if channel name is qualified then look at FQN first then channel name
    std::string::size_type pos = cname.find(".");
    int count=0;
    if ( pos != std::string::npos ) {
      // search for fully qualified name in registration list
      ChannelRegistrationTable::iterator   itr = _channels.begin();
      for(; itr != _channels.end() ; itr++ ) {
        if ( itr->second.fqn.compare(cname) == 0 ) {
          count++;
          break;
        }
      }
    }
    
    // just search for non qualified channel name
    if ( count == 0 ) {
      count = _channels.count( cname );
    }
    
    return ( count > 0 );
  }

  /*
   */
  std::string EventChannelManager::_generateRegId() 
  {
    uuid_t id;
    uuid_generate(id);

    // Per the man page, UUID strings are 36 characters plus the '\0' terminator.
    char strbuf[37];
    uuid_unparse(id, strbuf);

    return std::string(strbuf);
  }


/*
 */
  bool EventChannelManager::_validateChannelName( const std::string &cname ) {
    if ( cname.find(":") == std::string::npos ) {
      return true;
    }
    else {
      return false;
    }
  }     


  /*
   */
  EventChannelManager::ChannelRegistrationPtr  EventChannelManager::_getChannelRegistration( const std::string &cname ) {

    std::string::size_type pos = cname.find(".");
    ChannelRegistrationPtr ret=NULL;

    if ( pos != std::string::npos ) {
      // search registration base on fully qualified name
      ChannelRegistrationTable::iterator   itr = _channels.begin();
      for( ; itr != _channels.end(); itr++ ) {
        if ( itr->second.fqn.compare(cname) == 0 ) {
          ret = &(itr->second);
          break;
        }
      }
    }

    if ( ret == NULL ) {
      ChannelRegistrationTable::iterator   itr = _channels.find( cname );
      if ( itr != _channels.end() ){
        ret = &(itr->second);
      }
    }

    return ret;
  }


EventChannelManager::ChannelRegistrationPtr EventChannelManager::_addChannelRegistration( const std::string &cname,
                                                                                          const std::string &fqn,
                                                                                          ossie::events::EventChannel_ptr channel,
                                                                                          bool autoRelease ) 
{

  ECM_TRACE("_addChannelRegistration", "Created ChannelRegistrationRecord, Event Channel/FQN : "<< cname << "/" << fqn );
  ECM_TRACE("_addChannelRegistration", "Created ChannelRegistrationRecord, Event Channel : "<< channel << "/" << autoRelease );

  ChannelRegistrationTable::iterator   itr = _channels.find( cname );
  ChannelRegistrationPtr ret=NULL;
  if ( itr == _channels.end() ){
    // Insert a new registration
    _channels[cname] = ChannelRegistration();
    _channels[cname].channel_name = cname;           // name known to the domain
    _channels[cname].fqn = fqn;                      // internal name registered with EventService
    _channels[cname].channel = ossie::events::EventChannel::_duplicate(channel);
    _channels[cname].release = false;
    _channels[cname].autoRelease = autoRelease;
    ret = &(_channels[cname]);
    ECM_TRACE("_addChannelRegistration", "Created ChannelRegistrationRecord, Event Channel/FQN : "<< cname << "/" << fqn );
    ECM_TRACE("addChannelRegistration", "    ChannelRecord: name:" << _channels[cname].channel_name );
    ECM_TRACE("addChannelRegistration", "    ChannelRecord: fqn:" << _channels[cname].fqn );
    ECM_TRACE("addChannelRegistration", "    ChannelRecord: autoRelease:" << _channels[cname].autoRelease );
    ECM_TRACE("addChannelRegistration", "    ChannelRecord: release:" << _channels[cname].release );
    ECM_TRACE("addChannelRegistration", "    ChannelRecord: registrants:" << _channels[cname].registrants.size());
    ECM_TRACE("addChannelRegistration", "    ChannelRecord: channel:" << _channels[cname].channel);
    ECM_TRACE("_addChannelRegistration", "Registration Table Size: "<< _channels.size() );
  }
  return ret;
}




  /*
   
   */
  void EventChannelManager::_deleteChannelRegistration( const std::string &cname ) {
    ChannelRegistrationTable::iterator   itr = _channels.find( cname );
    if ( itr != _channels.end() ){
      ChannelRegistrationPtr reg = &(itr->second);
      if ( reg ) {
        if ( CORBA::is_nil( reg->channel) == false ){
          ECM_DEBUG("_deleteChannelRegistration", "Calling Destroy, Channel/EventChannel: "<< cname << "/" << reg->fqn);
          try {
            reg->channel->destroy();
          }
          catch(...){
            ECM_DEBUG("_deleteChannelRegistration", "Exception during destroy  EventService.. channel/EventChannel: "<< cname << "/" << reg->fqn);
          }
          reg->channel = ossie::events::EventChannel::_nil();
          ECM_DEBUG("_deleteChannelRegistration", "Destory Completed, Channel/EventChannel: "<< cname << "/" << reg->fqn);
        }
      }

      // remove the channel registration entry
      ECM_DEBUG("_deleteChannelRegistration", "Deleting Registration for EventChannel: "<< cname );
      _channels.erase(itr);
    }
    ECM_DEBUG("_deleteChannelRegistration", "Completed delete registration...EventChannel: "<< cname );
    return;
  }

