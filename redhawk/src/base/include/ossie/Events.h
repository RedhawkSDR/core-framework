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

#ifndef __RH_CF_EVENTS_H__
#define __RH_CF_EVENTS_H__

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>
#include <deque>
#include <ossie/RedhawkDefs.h>
#include <ossie/EventTypes.h>
#include <ossie/debug.h>
#include <ossie/concurrent.h>
#include <ossie/CF/ExtendedEvent.h>

class Resource_impl;

REDHAWK_CPP_NAMESPACE_BEGIN

typedef   CORBA::Object_ptr             RH_Object;

/*

  redhawk.events -- Namespace for event processing within a REDHAWK domain.  

  Players:

     redhawk::events::Manager:
        Singleton pattern that allows a REDHAWK resource to interact with the Domain to acquire Event Channels for non standard usage patterns.
        Requiring access to event channels that are not provided to a running application via the FindBy mechnisim.

     redhawk::events::Publisher 
        This interface allows a REDHAWK resource to interact with an Event Channel as in a publisher role.  Resources will request a publisher
        using the Manager's Publisher method and the specified channel name.  Users of this interface can publish their events to the 
        underlying Event Channel using the push method.

        These object remain registered with the underlying Manager during their life cycle. The REDHAWK resource can relinquist registration
        by destorying this object.

     redhawk::events::Subscriber 
        This interface allows a REDHAWK resource to interact with an Event Channel as in a subscriber role.  Resources will request a subscriber
        using the Manager's Subscriber method and the specified channel name.   Subcribers can interact with the Event Channel to consume
        message events using 2 methods of acquistion:
          
           Non Callback Queues- Subscribers can use the queueing method to received data from the message queue at their convenience.  The 
                                getData method returns the oldest message on the event queue.

           Callback notification: - Subscribers can use a callback method to imediately receive event messages upon arrival to the underlying
                                    Event Channel.

        These object remain registered with the underlying Manager during their life cycle. The REDHAWK resource can relinquist registration
        by destorying this object.

     CF::EventChannel:
        Messaging interface that allows for a multicast like implmemtation of event message distribution.  Messages are provide in a 
        publisher/subscriber pattern and defined via IDL.
 */

namespace events {

  class Manager;
  class Publisher;
  class Subscriber;

  
  // Well known channel names for the Domain
  const std::string   IDM_Channel_Spec="IDM_Channel";
  const std::string   ODM_Channel_Spec="ODM_Channel";

  //
  // Top level API used by clients
  //
  typedef boost::shared_ptr<Publisher>   PublisherPtr;
  typedef boost::shared_ptr<Subscriber>  SubscriberPtr;
  typedef boost::shared_ptr<Manager>     ManagerPtr;

  class RegistrationExists: std::exception {
  };


  class RegistrationFailed: std::exception {
  };

  class OperationFailed: std::exception {
  };


  struct DomainStateEvent {
    std::string prod_id;
    std::string source_id;
    std::string source_name;
    StandardEvent::SourceCategoryType category;
    RH_Object obj;

  };

  struct ObjectStateChangeEvent {
    std::string prod_id;
    std::string source_id;
    StandardEvent::StateChangeCategoryType category;
    StandardEvent::StateChangeType    from;
    StandardEvent::StateChangeType    to;
  };

  struct ResourceStateChangeEvent {
    std::string source_id;
    std::string source_name;
    ExtendedEvent::ResourceStateChangeType    from;
    ExtendedEvent::ResourceStateChangeType    to;
  };

  struct ComponentTerminationEvent {
    std::string device_id;
    std::string application_id;
    std::string component_id;
  };

  void SendStateChangeEvent(const char* producerId, 
                            const char* sourceId,
                            StandardEvent::StateChangeCategoryType stateChangeCategory, 
                            StandardEvent::StateChangeType stateChangeFrom, 
                            StandardEvent::StateChangeType stateChangeTo,
			    PublisherPtr  publisher );

  void SendResourceStateChangeEvent(const char* sourceId, 
                            const char* sourceName,
                            ExtendedEvent::ResourceStateChangeType stateChangeFrom, 
                            ExtendedEvent::ResourceStateChangeType stateChangeTo,
			    PublisherPtr  publisher );

  void SendObjectAddEvent( const char* producerId, 
                           const char* sourceId, 
                           const char* sourceName, 
                           RH_Object src_obj,
                           StandardEvent::SourceCategoryType sourceCategory, 
                           PublisherPtr publisher);

  void SendObjectRemovedEvent( const char* producerId, 
                               const char* sourceId, 
                               const char* sourceName, 
                               StandardEvent::SourceCategoryType sourceCategory, 
                               PublisherPtr publisher);

  void SendComponentTermination( const char *device_id,
                                      const char *application_id,
                                      const char *component_id );

  class DomainEventWriter {
  public:
    DomainEventWriter( PublisherPtr pub );
    virtual ~DomainEventWriter(){};
    virtual void sendObjectStateChange(const char* producerId, 
				       const char* sourceId,
				       StandardEvent::StateChangeCategoryType stateChangeCategory, 
				       StandardEvent::StateChangeType stateChangeFrom, 
				       StandardEvent::StateChangeType stateChangeTo );
    virtual void sendObjectStateChange( const ObjectStateChangeEvent &evt );
    
    virtual void sendResourceStateChange(const char* sourceId, 
					const char* sourceName,
					ExtendedEvent::ResourceStateChangeType stateChangeFrom, 
					ExtendedEvent::ResourceStateChangeType stateChangeTo);
    virtual void sendResourceStateChange( const ResourceStateChangeEvent &evt );

    virtual void sendAddedEvent( const char* producerId, 
				 const char* sourceId, 
				 const char* sourceName, 
				 RH_Object src_obj,
				 StandardEvent::SourceCategoryType sourceCategory );
    virtual void sendAddedEvent( const DomainStateEvent  &evt);

    virtual void sendRemovedEvent( const char* producerId, 
				   const char* sourceId, 
				   const char* sourceName, 
				   StandardEvent::SourceCategoryType sourceCategory );
    virtual void sendRemovedEvent( const DomainStateEvent  &evt );

    virtual void sendComponentTermination( const char *deviceId,
                                           const char *applicationId,
                                           const char *componentId );
    virtual void sendComponentTermination( const ComponentTerminationEvent  &evt );

  private:
    PublisherPtr   pub;

  };


  class DomainEventReader {
  public:
    DomainEventReader( SubscriberPtr sub );
    DomainEventReader();
    virtual ~DomainEventReader(){
      unsubscribe();
    };

    template< typename T > 
    class ReaderListener {
    public:
      virtual void operator() ( const T & ) = 0;
      virtual ~ReaderListener() {};

    };

    template <class T, typename DT >
      class MemberCBListener : public ReaderListener<DT>
    {
    public:
      typedef boost::shared_ptr< MemberCBListener< T, DT > > SPtr;
      
      typedef void (T::*MemberFn)( const DT &data );

      static SPtr Create( T &target, MemberFn func ){
        return SPtr( new MemberCBListener(target, func ) );
      };

      virtual void operator() ( const DT &data )
      {
        (target_.*func_)(data);
      }

      MemberCBListener ( T& target,  MemberFn func) :
      target_(target),
        func_(func)
        {
        }
    private:
      T& target_;
      MemberFn func_;
    };


    typedef ReaderListener< ObjectStateChangeEvent >      ObjectStateListener;
    typedef ReaderListener< ResourceStateChangeEvent >    ResourceStateListener;
    typedef ReaderListener< ComponentTerminationEvent >   TerminationListener;
    typedef ReaderListener< DomainStateEvent >            AddRemoveListener;

    typedef boost::shared_ptr< ObjectStateListener >            ObjectStateListenerPtr;
    typedef boost::shared_ptr< ResourceStateListener >          ResourceStateListenerPtr;
    typedef boost::shared_ptr< TerminationListener >            TerminationListenerPtr;
    typedef boost::shared_ptr< AddRemoveListener >              AddRemoveListenerPtr;

    template< typename T > inline
      void setObjectListener(T &target, void (T::*func)( const ObjectStateChangeEvent &data )  ) {
      obj_cbs.push_back( boost::make_shared< MemberCBListener< T, ObjectStateChangeEvent > >( boost::ref(target), func ) );
    };

    template< typename T > inline
      void setObjectListener(T *target, void (T::*func)( const ObjectStateChangeEvent &data  )  ) {
      obj_cbs.push_back( boost::make_shared< MemberCBListener< T, ObjectStateChangeEvent > >( boost::ref(*target), func ));
    };

    template< typename T > inline
      void setResourceListener(T &target, void (T::*func)( const ResourceStateChangeEvent &data )  ) {
      rsc_cbs.push_back(boost::make_shared< MemberCBListener< T, ResourceStateChangeEvent > >( boost::ref(target), func ));
    };

    template< typename T > inline
      void setResourceListener(T *target, void (T::*func)( const ResourceStateChangeEvent &data  )  ) {
      rsc_cbs.push_back(boost::make_shared< MemberCBListener< T, ResourceStateChangeEvent > >( boost::ref(*target), func ));
    };

    template< typename T > inline
      void setTerminationListener(T &target, void (T::*func)( const ComponentTerminationEvent &data )  ) {
      term_cbs.push_back( boost::make_shared< MemberCBListener< T, ComponentTerminationEvent > >( boost::ref(target), func ));
    };

    template< typename T > inline
      void setTerminationListener(T *target, void (T::*func)( const ComponentTerminationEvent &data  )  ) {
      term_cbs.push_back( boost::make_shared< MemberCBListener< T, ComponentTerminationEvent > >( boost::ref(*target), func ));
    };

    template< typename T > inline
      void setAddRemoveListener(T &target, void (T::*func)( const  DomainStateEvent &data )  ) {
      addrm_cbs.push_back( boost::make_shared< MemberCBListener< T, DomainStateEvent > >( boost::ref(target), func ) );
    };

    template< typename T > inline
      void setAddRemoveListener(T *target, void (T::*func)( const  DomainStateEvent &data  )  ) {
      addrm_cbs.push_back( boost::make_shared< MemberCBListener< T, DomainStateEvent > >( boost::ref(*target), func ));
    };


    void setObjectListener(  ObjectStateListener *cb );
    void setResourceListener( ResourceStateListener *cb );
    void setTerminationListener( TerminationListener *cb );
    void setAddRemoveListener( AddRemoveListener *cb ); 

    void subscribe( SubscriberPtr sub );
    void subscribe( );
    void unsubscribe();

  private:

    typedef std::vector< ObjectStateListenerPtr >    ObjectListeners;
    typedef std::vector< ResourceStateListenerPtr >  ResourceListeners;
    typedef std::vector< TerminationListenerPtr >    TerminationListeners;
    typedef std::vector< AddRemoveListenerPtr >      AddRemoveListeners;

    typedef bool (DomainEventReader::*EventMsgHandler)( const CORBA::Any & msg );
    typedef std::vector< DomainEventReader::EventMsgHandler >     EventMsgHandlers;

    void  _eventMsgHandler( const CORBA::Any &msg  );
    bool  _objectMsgHandler( const CORBA::Any & msg );
    bool  _resourceMsgHandler( const CORBA::Any & msg );
    bool  _terminationMsgHandler( const CORBA::Any & msg );
    bool  _addRemoveMsgHandler( const CORBA::Any & msg );

    SubscriberPtr             sub;
    
    ObjectListeners           obj_cbs;
    ResourceListeners         rsc_cbs;
    TerminationListeners      term_cbs;
    AddRemoveListeners        addrm_cbs;
    
    EventMsgHandlers          msg_handlers;
    

  };



  class EM_Publisher;
  class EM_Subscriber;



  //
  // Provide a Resource common access to the EventChannelManager
  //
  class Manager {

  public:

    static  ManagerPtr GetManager( Resource_impl *obj );   // returns class singleton

    static  void        Terminate();                        // required by users to terminate before closing down their orb

    //
    // Clean up my resources with the Domain's EventChannelManager
    //
    virtual ~Manager();

    //
    // Request a Publisher on a specified Event Channel
    //
    PublisherPtr Publisher( const std::string &channelName, const std::string &registrationId="" );
 
    //
    // Request a Subscriber to a specified Event Channel
    //
    SubscriberPtr  Subscriber( const std::string &channelName, const std::string &registrationId="" );

  private:

    typedef std::vector< ossie::events::EventChannelReg >     Registrations;
    typedef std::vector< redhawk::events::Publisher * >       Publishers;
    typedef std::vector< redhawk::events::Subscriber * >      Subscribers;

    friend class EM_Publisher;
    friend class EM_Subscriber;

    void _deletePublisher( redhawk::events::Publisher *pub );
    void _deleteSubscriber( redhawk::events::Subscriber  *sub );
    void _unregister( const ossie::events::EventChannelReg &, redhawk::events::Publisher *pub  );
    void _unregister( const ossie::events::EventChannelReg &, redhawk::events::Subscriber *sub );
    void _unregister( const ossie::events::EventChannelReg & );
    void _terminate();


    Manager(  Resource_impl *obj );

    //static ManagerPtr              _Manager;

    Registrations                  _registrations;

    // allocated publishers and subscribers..
    Publishers                     _publishers;
    Subscribers                    _subscribers;

    CF::EventChannelManager_var    _ecm;

    Mutex                          _mgr_lock;

    bool                           _allow;
    
    static std::map<std::string, ManagerPtr> _managers;

    Resource_impl                  *_obj;
      
    std::string                    _obj_id;
    rh_logger::LoggerPtr           _resourceLog;
    rh_logger::LoggerPtr           _eventManagerLog;
  };


   
  
  // forward declarations for Disconnect interfaces



  class  Publisher  {

  public:

    // interface that handle disconnects from channel
    class Receiver;

  private:
    friend class Manager;
    friend class Receiver;


  public:    

    static const int DEFAULT_RETRIES=10;
    static const int DEFAULT_WAIT=10;


    //
    //  Publisher for an Event Channel
    //
    // @param channel    event channel returned from the Domain's EventChannelManager
    // @param pub   interface that is notified when a disconnect occurs
    // @param retries    number of retries to perform when trying to establish  publisher interface
    // @param retry_wait number of millisecs to wait between retries
    Publisher( ossie::events::EventChannel_ptr            channel);

    void setLogger(rh_logger::LoggerPtr logptr) {
        _publisherLog = logptr;
    };

    //
    //
    //
    virtual ~Publisher();
    
    //
    // Publish a CORBA Any object or a specific object subscribers.... 
    //
    template< typename T > 
      int     push( T &msg ) {
      int retval=0;
      try {
        CORBA::Any data;
        RH_NL_TRACE("Publisher", "Creating event message object for proxy.");
        data <<= msg;
        if (!CORBA::is_nil(proxy)) {
          proxy->push(data);
          RH_NL_TRACE("Publisher", "Message sent downstream......");
        }
        else{
          retval=-1;
        }
      }
      catch( CORBA::Exception& ex) {
        retval=-1;
      }

      RH_NL_TRACE("Publisher", "push(msg) retval" << retval );
      return retval;
    }

    template< typename T > 
      int     push( T *msg ) {
      int retval=0;
      try {
        CORBA::Any data;
        RH_NL_TRACE("Publisher", "Creating event message object for proxy.");
        data <<= msg;
        if (!CORBA::is_nil(proxy)) {
          proxy->push(data);
          RH_NL_TRACE("Publisher", "Message sent downstream......");
        }
        else{
          retval=-1;
        }
      }
      catch( CORBA::Exception& ex) {
        retval=-1;
      }
      RH_NL_TRACE("Publisher", "push(*msg) retval" << retval );
      return retval;
    }


    int     push( const std::string &msg );
    int     push( CORBA::Any &data );

    //
    // disconnect from the event channnel
    //
    // @returns  0  disconnect was successfull
    // @returns  -1 disconnect failed
    //
    virtual int disconnect(const int retries=DEFAULT_RETRIES, const int retry_wait=DEFAULT_WAIT );

    //
    // connect to the event channnel
    //
    // @returns  0  connection was successfull
    // @returns  -1 connection failed
    //
    virtual int connect(const int retries=DEFAULT_RETRIES, const int retry_wait=DEFAULT_WAIT );


  protected:


    // handle to the Event Channel ... 
    ossie::events::EventChannel_var           channel;

    // handle to object that publishes the event to the channel's consumers
    ossie::events::EventPublisher_var         proxy;

  private:

    // handle to object that responds to disconnect messages
    Receiver                                 *_disconnectReceiver;
    rh_logger::LoggerPtr _publisherLog;

  };


  //
  // Subscriber
  //
  // This class will perform the subscription portion of the a publisher/subscriber pattern 
  // over an EventChannel.   Users will request a registration for the event channel and then
  // 
  // 
  //
  class  Subscriber {

    friend class Manager;

  public:

    static const int DEFAULT_RETRIES=10;
    static const int DEFAULT_WAIT=10;

    //
    // Interface to handle received messages and disconnects
    class Receiver;

    //
    // Callback interface when data arrives event happens
    //
    typedef void   (*DataArrivedCallbackFn)( const CORBA::Any &data );

    //
    // Interface definition that will be notified when data arrives on a EventChannel
    //
    class DataArrivedListener {

    public:
      virtual void operator() ( const CORBA::Any &data ) = 0;
      virtual ~DataArrivedListener() {};

    };

    void setLogger(rh_logger::LoggerPtr logptr) {
        _subscriberLog = logptr;
    };

    typedef boost::shared_ptr< Subscriber::DataArrivedListener >    DataArrivedListenerPtr;

    /*
     * Allow for member functions to receive connect/disconnect notifications
     */
    template <class T>
      class MemberDataArrivedListener : public DataArrivedListener
    {
    public:
      typedef boost::shared_ptr< MemberDataArrivedListener< T > > SPtr;
      
      typedef void (T::*MemberFn)( const CORBA::Any &data );

      static SPtr Create( T &target, MemberFn func ){
        return SPtr( new MemberDataArrivedListener(target, func ) );
      };

      virtual void operator() ( const CORBA::Any &data )
      {
        (target_.*func_)(data);
      }

      // Only allow PropertySet_impl to instantiate this class.
      MemberDataArrivedListener ( T& target,  MemberFn func) :
      target_(target),
        func_(func)
        {
        }
    private:
      T& target_;
      MemberFn func_;
    };

    //
    // DTOR
    //
    virtual ~Subscriber();

    //
    // Attach callback listener when data arrives to Consumer object
    //
    template< typename T > inline
      void setDataArrivedListener(T &target, void (T::*func)( const CORBA::Any &data )  ) {
      dataArrivedCB =  boost::make_shared< MemberDataArrivedListener< T > >( boost::ref(target), func );
    };

    template< typename T > inline
      void setDataArrivedListener(T *target, void (T::*func)( const CORBA::Any &data  )  ) {
      dataArrivedCB =  boost::make_shared< MemberDataArrivedListener< T > >( boost::ref(*target), func );
    };

    void   setDataArrivedListener( DataArrivedListener *newListener );
    void   setDataArrivedListener( DataArrivedCallbackFn  newListener );


    template< typename MSG_TYPE>
      int getData( MSG_TYPE &ret_msg ) {
        
      int retval=-1;
      try{

        // check if callback method is enable.. it so then return
        if ( dataArrivedCB ) return retval;

        // check if data is available
        if ( events.size() < 1 ) return retval;
          
        CORBA::Any  rawdata =  events.front();
          
        MSG_TYPE tmsg;
        if (rawdata >>= tmsg) { 
          ret_msg = tmsg;
          retval=0;
        }

        events.pop_front();

      }
      catch(...) {
      }

      return retval;
    }

    virtual int getData( std::string &ret_msg );

    virtual int getData( CORBA::Any &ret );

    //
    // disconnect from the event channnel
    //
    // @returns  0  disconnect was successfull
    // @returns  -1 disconnect failed
    //
    virtual int disconnect(const int retries=DEFAULT_RETRIES, const int retry_wait=DEFAULT_WAIT );

    //
    // connect to the event channnel
    //
    // @returns  0  connection was successfull
    // @returns  -1 connection failed
    //
    virtual int connect(const int retries=DEFAULT_RETRIES, const int retry_wait=DEFAULT_WAIT );

  protected:

    //
    // Create a Subscriber interface to an Event Channel. Subscribers will listen for event messages and provide 1 of two methods
    // for data retrieval.  Via a data queue that can be extracted or from a registred callback. In either case the user of the Subscriber
    // interface is required to know  apriori the message format (i.e. message structure) to unmarshall the encoded stream.
    //
    // @param EventChannel   in event channel
    // @param retries number of retries to perform when trying to establish subscriber interface (-1 tries forever)
    // @param retry_wait number of millisecs to wait between retries
    Subscriber(  ossie::events::EventChannel_ptr     inChannel );

    //
    // Create a Subscriber 
    //
    // @param EventChannel   in event channel
    // @param newListener    callback to listen for incoming event messages
    // @param retries number of retries to perform when trying to establish subscriber interface (-1 tries forever)
    // @param retry_wait number of millisecs to wait between retries

    Subscriber(  ossie::events::EventChannel_ptr     inChannel,
                 DataArrivedListener *newListener );

    //
    // Create a Subscriber 
    //
    // @param EventChannel   in event channel
    // @param newListener    callback to listen for incoming event messages
    // @param retries number of retries to perform when trying to establish subscriber interface (-1 tries forever)
    // @param retry_wait number of millisecs to wait between retries
    Subscriber(  ossie::events::EventChannel_ptr     inChannel,
                 DataArrivedCallbackFn  newListener );

  protected:

    friend class DefaultConsumer;

    // handle to the Event Channel ... 
    ossie::events::EventChannel_var       channel;

    //
    // Subscriber proxy 
    //
    ossie::events::EventSubscriber_var     proxy;

    //
    // Event subscriber consumer/disconnect
    //
    Receiver                              *consumer;

    //
    // Callback to notify when data has arrived
    // 
    // Used by Consumer object to call registered callback
    //
    DataArrivedListenerPtr                 dataArrivedCB;

    //
    // Non-callback interface to queue data from event channels
    //
    //
    std::deque< CORBA::Any >               events;


  private:


    void   _init(  ossie::events::EventChannel_ptr     inChannel );
    rh_logger::LoggerPtr _subscriberLog;


  }; // end of Subscriber



};    // end of events namespace


REDHAWK_CPP_NAMESPACE_END


#endif   // __RH_CF_EVENTS_H__
