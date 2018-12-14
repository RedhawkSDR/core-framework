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
#ifndef __RH_CF_DOMMGR_ECM__
#define __RH_CF_DOMMGR_ECM__
#include <set>
#include <COS/CosLifeCycle.hh>
#include <ossie/RedhawkDefs.h>
#include <ossie/EventTypes.h>
#include <ossie/debug.h>
#include <ossie/concurrent.h>
#include <ossie/CorbaUtils.h>




typedef  POA_CF::EventChannelManager       EventChannelManagerBase;
class DomainManager_impl;


class EventChannelManager: public virtual EventChannelManagerBase {

 private:
    
  friend  class EventChannelInfoIteratorImpl;

 public:

  //
  // CTOR to instantiate the EventChannelManager implementation
  //
#ifdef CPPUNIT_TEST
  EventChannelManager ( const bool useFQN=true, const bool enableNS=false, const bool allowES=false );
#else
  EventChannelManager (DomainManager_impl* domainManager, const bool useFQN=true, const bool enableNS=false, const bool allowES = false );
#endif

  virtual ~EventChannelManager();


  //
  // terminate
  //
  // Remove all event channel object and any registrations.
  //
  void      terminate( const bool destoryChannels = true );

  /*
     Return an Event Channel in the Domain associated with the Manager from the specified channel_name parameter.
     
     Event Channel names must be  unique across the entire Domain. Internally channel names will be prefixed with the 
     the Domain they are associated with to distinguish them from other Domain instances.

     Events created with this interface will remain active until the release method is called
     or the associated DomainManager is terminated.  Thus active registrations against the channel will
     become invalid, and operations against the channel will throw exceptions.
  */
  ossie::events::EventChannel_ptr create( const char *channel_name  )  
    throw ( CF::EventChannelManager::ChannelAlreadyExists, 
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );

  ossie::events::EventChannel_ptr create( const std::string &channel_name  )  
    throw ( CF::EventChannelManager::ChannelAlreadyExists, 
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );

  ossie::events::EventChannel_ptr get( const char *channel_name  )  
    throw ( CF::EventChannelManager::ChannelDoesNotExist, 
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );

  ossie::events::EventChannel_ptr get( const std::string &channel_name  )  
    throw ( CF::EventChannelManager::ChannelDoesNotExist, 
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );

  /*
     Return an Event Channel in the Domain associated with the Manager from the specified channel_name parameter.
     
     Event Channel names must be  unique across the entire Domain. Internally channel names will be prefixed with the 
     the Domain they are associated with to distinguish them from other Domain instances.

     Events created with this interface will remain active until the last registered user has unregistered or the
     associated Domain has terminated.

  */
  ossie::events::EventChannel_ptr createForRegistrations( const char *channel_name  )  
    throw ( CF::EventChannelManager::ChannelAlreadyExists, 
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );    

  void restore( ossie::events::EventChannel_ptr channel, 
                const std::string &channel_name, 
                const std::string &fqn_name )
    throw ( CF::EventChannelManager::ChannelAlreadyExists, 
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );    

  /*
        void setLogger(rh_logger::LoggerPtr logptr) {
            _allocMgrLog = logptr;
        };

     Force the removal of the event channel from the Domain
  */
  void forceRelease( const char *channel_name  ) 
    throw ( CF::EventChannelManager::ChannelDoesNotExist, 
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );

  /*
     Remove the event channel from the Domain
  */
  void release( const char *channel_name  ) 
    throw ( CF::EventChannelManager::ChannelDoesNotExist, 
            CF::EventChannelManager::RegistrationsExists, 
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );

  void release( const std::string &channel_name  ) 
    throw ( CF::EventChannelManager::ChannelDoesNotExist, 
            CF::EventChannelManager::RegistrationsExists, 
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );

  /*
     Mark an Event Channel for release on registrations so the Event Channel will remain active until all 
     registrants have unregistered.
  */

  void markForRegistrations( const char *channel_name )
    throw ( CF::EventChannelManager::ChannelDoesNotExist, 
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );
  /*
     Register an association with an Event Channel.  The EventChannel can then be
     used as input into the base library support package.
  
     Look up the registration against the current list for a match, if one exists then 
     throw RegistrationAlreadyExists
  
     Look for an existing Event Channel object being managed. If the Event Channel
     is not found then add a new Event Channel object 
  
  */

  ossie::events::EventChannelReg_ptr _registerResource( const ossie::events::EventRegistration &req)  
    throw ( CF::EventChannelManager::InvalidChannelName, 
            CF::EventChannelManager::RegistrationAlreadyExists,
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );

  ossie::events::EventChannelReg_ptr registerResource( const ossie::events::EventRegistration &req)  
    throw ( CF::EventChannelManager::InvalidChannelName, 
            CF::EventChannelManager::RegistrationAlreadyExists,
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );

  ossie::events::EventChannelReg_ptr registerConsumer( CosEventComm::PushConsumer_ptr consumer, const ossie::events::EventRegistration &req)  
    throw ( CF::EventChannelManager::InvalidChannelName, 
            CF::EventChannelManager::RegistrationAlreadyExists,
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );
  ossie::events::PublisherReg_ptr registerPublisher( const ossie::events::EventRegistration &req, CosEventComm::PushSupplier_ptr disconnectReceiver)  
    throw ( CF::EventChannelManager::InvalidChannelName, 
            CF::EventChannelManager::RegistrationAlreadyExists,
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );
  
  /*
     Unregister a publisher or subcriber from an event channel and invalidates the context
  */
  void unregister( const ossie::events::EventRegistration &reg )
    throw ( CF::EventChannelManager::ChannelDoesNotExist,
            CF::EventChannelManager::RegistrationDoesNotExist,
            CF::EventChannelManager::ServiceUnavailable );

  void listChannels( CORBA::ULong how_many, 
                     CF::EventChannelManager::EventChannelInfoList_out elist, 
                     CF::EventChannelInfoIterator_out eiter);

  void listRegistrants( const char* channel_name, 
                        CORBA::ULong how_many, 
                        CF::EventChannelManager::EventRegistrantList_out rlist, 
                        CF::EventRegistrantIterator_out riter);

  //
  // Extended EventChannelManager interface support for the DomainManager
  //
  std::string  getFQN( const std::string &cname, const std::string &nc_name="");

  // find channel via channel name or search FQN name.........
  const ossie::events::EventChannel_ptr findChannel( const std::string &channel_name );
  bool             isChannel( const std::string &channel_name );
  void             setPollingPeriod( const int64_t period );

  typedef  std::map< std::string, std::string >   RegIdList;
  struct ChannelRegistration {
    std::string                      channel_name;
    std::string                      fqn;
    ossie::events::EventChannel_var  channel;
    bool                             autoRelease;
    bool                             release;
    RegIdList                        registrants;

    int nregistrants( ) const { 
      return registrants.size();
    }
  };
  typedef std::map< std::string, ChannelRegistration >      ChannelRegistrationTable;
  
  typedef std::map<std::string, ossie::events::EventSubscriber_var> SubProxyMap;
  typedef std::map<std::string, ossie::events::EventPublisher_var> PubProxyMap;
  
  SubProxyMap getSubProxies() { return _subProxies;};
  PubProxyMap getPubProxies() { return _pubProxies;};
  ChannelRegistrationTable getChannelRegistrations() { return _channels;};
  void setSubProxies(SubProxyMap &_inval) { _subProxies = _inval;};
  void setPubProxies(PubProxyMap &_inval) { _pubProxies = _inval;};
  void setChannelRegistrations(ChannelRegistrationTable &_inval) { _channels = _inval; };

    void setLogger(rh_logger::LoggerPtr logptr) {
        _eventChannelMgrLog = logptr;
    };

  private:

  // type definitions
  typedef  std::pair< std::string, std::string >  RegRecord;
  typedef ChannelRegistration*                              ChannelRegistrationPtr;
  rh_logger::LoggerPtr _eventChannelMgrLog;

  // event channel manager state
  SubProxyMap _subProxies;
  PubProxyMap _pubProxies;
  //
  // Channel Registration database
  //
  ChannelRegistrationTable                         _channels;

  // configuration and default state information
#ifndef CPPUNIT_TEST
  // Handle to the Resource that owns us
  DomainManager_impl*                              _domainManager;
#endif
  // naming context directory to bind event channgels to...
  std::string                                      _domain_context;
  // orb context
  ossie::corba::OrbContext                         _orbCtx;
  //  Handle to factory interface to create EventChannels
  ossie::events::EventChannelFactory_var            _event_channel_factory;
  // if enabled, events will show up in the NamingService
  bool                                            _use_naming_service;
  // use fully qualified domain names when creating channels.
  bool                                           _use_fqn;
  // allow event service to resolve channels
  bool                                           _allow_es_resolve;
  // default polling period to assign to a channel
  int64_t                                        _default_poll_period;
  // synchronize access to member variables
  redhawk::Mutex                                   _mgrlock;


  void _initialize();



  void _unregister( const ossie::events::EventRegistration &reg ) 
    throw ( CF::EventChannelManager::ChannelDoesNotExist,
            CF::EventChannelManager::RegistrationDoesNotExist,
            CF::EventChannelManager::ServiceUnavailable );

  /*
     _getEventChannelFactory
  */
  void     _getEventChannelFactory() throw  ( CF::EventChannelManager::ServiceUnavailable );


  void _release( const std::string &channel_name) 
    throw ( CF::EventChannelManager::ChannelDoesNotExist, 
            CF::EventChannelManager::RegistrationsExists, 
	    CF::EventChannelManager::OperationFailed, 
	    CF::EventChannelManager::OperationNotAllowed,
	    CF::EventChannelManager::ServiceUnavailable );

  ossie::events::EventChannel_ptr _get( const std::string &channel_name )
    throw ( CF::EventChannelManager::ChannelDoesNotExist, 
	    CF::EventChannelManager::OperationFailed, 
	    CF::EventChannelManager::OperationNotAllowed,
	    CF::EventChannelManager::ServiceUnavailable );

  ossie::events::EventChannel_ptr _create( const std::string &channel_name, const bool autoRelease=false )  
    throw ( CF::EventChannelManager::ChannelAlreadyExists, 
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );

  /*
     _create

     Create and event channel in the Domain, if the registry is enabled then register the event channel
  */

  ossie::events::EventChannel_ptr        _createChannel( const std::string &channel_name, 
                                                        const std::string &fqn,
                                                        const std::string &nc_name="",
                                                        const bool require_ns = false )  
    throw ( CF::EventChannelManager::ChannelAlreadyExists, 
            CF::EventChannelManager::OperationFailed, 
            CF::EventChannelManager::OperationNotAllowed,
            CF::EventChannelManager::ServiceUnavailable );


  /*
     Resolve event channel via EventService resolution methods
   */
  ossie::events::EventChannel_ptr        _resolve_es( const std::string &cname,  
                                                     const std::string &fqn,
                                                     const bool suppress=true );

  /*
     Resolve the request to look up an EventChannel using the cname and nc_name
  */
  ossie::events::EventChannel_ptr        _resolve_ns( const std::string &cname,  
                                                     const std::string &fqn,
                                                     const std::string &nc_name="" );

  /*
     check if registration id exists for the specified channel
  */
  bool                                _regIdExists( const std::string &cname, const std::string &regid );


  /*
     check if registration id exists for the specified channel as a publisher
  */
  bool                                _regExists( const std::string &cname, const std::string &regid ) 
    throw ( CF::EventChannelManager::ChannelDoesNotExist,
            CF::EventChannelManager::RegistrationDoesNotExist );

  /*
     check if channel name exits in the local registration database 
  */
  bool                                _channelExists( const std::string &cname  );

  /*
   */
  std::string                         _generateRegId();


  std::string                         _getFQN( const std::string &cname, const std::string &nc_name="");

  /*
   */
  bool                                _validateChannelName( const std::string &cname );


  /*
       
   */
  ChannelRegistrationPtr              _addChannelRegistration( const std::string &cname, 
                                                               const std::string &fqn, 
                                                               ossie::events::EventChannel_ptr channel,
                                                               bool autoRelease=true);

  /*
       
   */
  ChannelRegistrationPtr              _getChannelRegistration( const std::string &cname );


  /*
       
   */
  void                                _deleteChannelRegistration( const std::string &cname );

};

#endif
