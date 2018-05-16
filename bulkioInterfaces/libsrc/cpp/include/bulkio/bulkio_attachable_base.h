/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#ifndef __bulkio_attachable_base_h
#define __bulkio_attachable_base_h

#include <queue>
#include <list>
#include <vector>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <ossie/ossieSupport.h>

#include "bulkio_base.h"
#include "bulkio_traits.h"
#include "bulkio_callbacks.h"

namespace bulkio {
  
    
  struct AttachableSriMapStruct : public SriMapStruct {
    BULKIO::PrecisionUTCTime   time;

    AttachableSriMapStruct( const BULKIO::StreamSRI &in_sri, const BULKIO::PrecisionUTCTime &in_ts )
        : SriMapStruct(in_sri) {
      sri = in_sri;
      time = in_ts;
    };

    AttachableSriMapStruct( const AttachableSriMapStruct &src )
       : SriMapStruct(src.sri) {
      sri = src.sri;
      time = src.time;
      connections = src.connections;
    };

  };
  
  //
  // Mapping of Stream IDs to SRI Map/Refresh objects
  //
  typedef std::map<std::string, AttachableSriMapStruct > OutPortSriMap;


  template <typename StreamDefinition, typename PortType, typename StreamSequence, typename POAType>
  class InAttachablePort : public Port_Provides_base_impl, public virtual POAType {

  protected:
    
    typedef void (SRICallbackFn)(const BULKIO::StreamSRI&);

    typedef boost::function<SRICallbackFn> SRICallback;

  public:

    //
    // Interface to notify when an attach and detach event/request is made
    //
    class  Callback  {

    public:

      virtual char* attach(const StreamDefinition& stream, const char* userid)
        throw (typename PortType::AttachError, typename PortType::StreamInputError) = 0;

      virtual void detach(const char* attachId) = 0;

    };


    //
    // InAttachablePort
    //
    // Input port constructor. This class accepts 3 different interfaces
    //
    // Callback - Providers of this interface will be notified when attach and detach methods are called.
    // sri::Compare  - Compare method for comparing SRI object when pushSRI happens
    // time::Compare - Compare method for comparing TimeStamp objects when pushSRI happens
    //
    InAttachablePort(std::string port_name, 
                     InAttachablePort::Callback *attach_detach_cb = NULL,
                     bulkio::sri::Compare  sriCmp = bulkio::sri::DefaultComparator,
                     bulkio::time::Compare timeCmp = bulkio::time::DefaultComparator,
                     SRICallback *newSriCB = NULL,
                     SRICallback *sriUpdatedCB = NULL );


    InAttachablePort(std::string port_name, 
                     LOGGER_PTR    new_logger,
                     InAttachablePort::Callback *attach_detach_cb = NULL,
                     bulkio::sri::Compare sriCmp = bulkio::sri::DefaultComparator, 
                     bulkio::time::Compare timeCmp = bulkio::time::DefaultComparator,
                     SRICallback *newSriCB = NULL,
                     SRICallback *sriUpdatedCB = NULL );

    virtual ~InAttachablePort();

    virtual bool hasSriChanged ();

    // ----------------------------------------------------------------------------------------
    // InAttachablePort
    // ----------------------------------------------------------------------------------------

    void setLogger( LOGGER_PTR newLogger ); 
    void setNewAttachDetachCallback( Callback *newCallback);
    
    template <typename T> inline
    void setNewSriListener(T &target, void (T::*func)( const BULKIO::StreamSRI &) ) {
        newSRICallback = boost::bind(func, &target, _1);
    }
  
    template <typename T> inline
    void setNewSriListener(T *target, void (T::*func)( const BULKIO::StreamSRI &) ) {
        newSRICallback = boost::bind(func, target, _1);
    }
    
    template <typename T> inline
    void setSriChangeListener(T &target, void (T::*func)( const BULKIO::StreamSRI &) ) {
        sriChangeCallback = boost::bind(func, &target, _1);
    }
  
    template <typename T> inline
    void setSriChangeListener(T *target, void (T::*func)( const BULKIO::StreamSRI &) ) {
        sriChangeCallback = boost::bind(func, target, _1);
    }

    void setNewSriListener( SRICallback newCallback );

    void setNewSriListener( SRICallbackFn newCallback );
    
    void setSriChangeListener( SRICallback newCallback );

    void setSriChangeListener( SRICallbackFn newCallback );
    
    //
    // Port Statistics Interface
    //
    virtual void enableStats(bool enable);  
    virtual void setBitSize(double bitSize);
    virtual void updateStats(unsigned int elementsReceived, float queueSize, bool EOS, std::string streamID);
    virtual BULKIO::PortStatistics * statistics();
    BULKIO::PortUsageType state();

    //
    // updateSRI Interface
    //

    //
    // activeSRIs - returns a sequence of BULKIO::StreamSRI objectsPort
    //
    // @return BULKIO::StreamSRISequence - list of active SRI objects for this port
    //                                     the caller is responsible for freeing the memory returned
    //
    virtual BULKIO::StreamSRISequence * activeSRIs();
    virtual void pushSRI(const BULKIO::StreamSRI& H, const BULKIO::PrecisionUTCTime& T);


    //
    // Attachable Interface
    //

    //
    // attach
    //
    // Request an attach operation to the provided stream definition.  Each requestor is required
    // to provide a subscriber id to track attachment requests.  Upon successfull
    // completion, this method returns an attachment identifier.  This identifier is required
    // to perform the detach operation
    //
    virtual char* attach(const StreamDefinition& stream, const char* userid)
      throw (typename PortType::AttachError, typename PortType::StreamInputError);

    //
    // detach
    //
    // Process a request to detach from a stream for the provided attachment identifier.
    //
    virtual void detach(const char* attachId);

    //
    // getStreamDefinition
    //
    // @return   StreamDefinition Return the stream definition for this attachId
    //           NULL attachId was not found
    //
    virtual StreamDefinition* getStreamDefinition(const char* attachId);

    //
    // getUser
    //
    // @return char * the user id that made the attachId request
    //         NULL attachId was not found
    //
    virtual char* getUser(const char* attachId);

    //
    // attachedSRIs
    //
    // @return BULKIO::StreamSRISequence Returns a list of StreamSRI objects for each attached 
    //                                   stream definition. The caller is responsible for freeing
    //                                   the provided objects
    //
    virtual BULKIO::StreamSRISequence* attachedSRIs();

    //
    // attachedStreams
    //
    // @return StreamSequence Returns a list of attached Stream Defintions. The caller
    //                                 is responsible for freeing the provided objects
    //
    virtual StreamSequence* attachedStreams();

    //
    // attachmentIds
    //
    // @return BULKIO::StringSequence Return the current list of attachment identifiers. The caller
    //                                is responsible for freeing the provided objects
    //
    virtual BULKIO::StringSequence* attachmentIds();

    //
    // usageState
    //
    // If the number of attached streams == 0 
    //      return typename PortType::IDLE;
    // If the number of attached streams == 1
    //      return typename PortType::BUSY;
    // other
    //    return typename PortType::ACTIVE;
    //
    // @return   typename PortType::InputUsageState return the current state of the port base on how many attach requests have been received
    //            
    virtual typename PortType::InputUsageState usageState();

	std::string getRepid() const;
 

  protected:

    typedef  std::map<std::string, StreamDefinition*>    AttachedStreams;

    typedef std::map<std::string, std::string >          AttachedUsers;

    typedef std::map<std::string, std::pair<BULKIO::StreamSRI, BULKIO::PrecisionUTCTime> >   SriMap;

    typedef boost::upgrade_lock< boost::shared_mutex >   UPGRADE_LOCK;
    
    typedef boost::shared_lock< boost::shared_mutex >    READ_LOCK;


    // maps a stream ID to a pair of Stream and userID
    AttachedStreams      attachedStreamMap;

    // Holds list of uers
    AttachedUsers        attachedUsers;

    // Current set of SRI objects passed to us
    SriMap               currentHs;

    bool                 sriChanged;

    MUTEX                statUpdateLock;

    boost::shared_mutex  sriUpdateLock;

    boost::shared_mutex  attachmentLock;

    bulkio::sri::Compare     sri_cmp;

    bulkio::time::Compare    time_cmp;

    Callback                 *attach_detach_callback;
    
    // statistics
    linkStatistics           *stats;
    
    SRICallback          newSRICallback;

    SRICallback          sriChangeCallback;
  };


  // ----------------------------------------------------------------------------------------
  // Attachable Output Port
  // ----------------------------------------------------------------------------------------
  template <typename StreamDefinition, typename PT, typename StreamSequence>
  class OutAttachablePort : public Port_Uses_base_impl, public virtual POA_BULKIO::UsesPortStatisticsProvider {

    protected:

      typedef PT PortType;

      class Stream;


      //
      // Stream Attachment represents a single port attachment
      //
      class StreamAttachment {
    
        public:

          //
          // Constructors
          //
          StreamAttachment(const std::string& connection_id, 
                           const std::string& attach_id, 
                           typename PortType::_ptr_type input_port);
          StreamAttachment(const std::string& connection_id, 
                           typename PortType::_ptr_type input_port);
          StreamAttachment(const std::string& connection_id, 
                           const std::string& attach_id, 
                           typename PortType::_ptr_type input_port,
                           OutAttachablePort *p );
          StreamAttachment(const std::string& connection_id, 
                           typename PortType::_ptr_type input_port, 
                           OutAttachablePort *p );
          
          void setLogger(LOGGER_PTR newLogger);
      
          // 
          // detach
          //
          void detach();

          //
          // Getters
          //
          std::string getConnectionId();
          std::string getAttachId(); 
          typename PortType::_ptr_type getInputPort(); 

          //
          // Setters
          //
          void setConnectionId(const std::string& connection_id);
          void setAttachId(const std::string& attach_id);
          void setInputPort(typename PortType::_ptr_type input_port);
  
        private:
          std::string _connectionId;
          std::string _attachId;
          typename PortType::_var_type _inputPort;
          LOGGER_PTR logger;
          OutAttachablePort *_port;
      };
  
      typedef StreamAttachment AttachmentType;
      typedef typename std::vector<AttachmentType> StreamAttachmentList;
      typedef typename StreamAttachmentList::iterator StreamAttachmentIter;
  
      //
      // Streams represent the flow of a single stream
      //
      class Stream {

        public:
  
          //
          // Constructors
          //
          Stream( );
          Stream(const StreamDefinition& stream_def,
                 const std::string name,
                 const std::string stream_id);
          Stream(const StreamDefinition& stream_def,
                 const std::string name,
                 const std::string stream_id,
                 BULKIO::StreamSRI sri,
                 BULKIO::PrecisionUTCTime time);
          Stream(const Stream& obj);
          Stream& operator=(const Stream& obj)
          { 
            _streamDefinition = obj._streamDefinition;
            _name = obj._name;
            _streamId = obj._streamId;
            _streamAttachments = obj._streamAttachments;
            _sri = obj._sri;
            _time = obj._time;
            port = obj.port;
            return *this;
          }
          
          void setLogger(LOGGER_PTR newLogger);
  
          //
          // detach
          //
          void detachAll();
          void detachByConnectionId(const std::string& connectionId);
          void detachByAttachId(const std::string& attachId);
          void detachByAttachIdConnectionId(const std::string& attachId, const std::string& connectionId);

          //
          // Getters
          //
          StreamDefinition& getStreamDefinition();
          std::string getName();
          std::string getStreamId();
          StreamAttachmentList getStreamAttachments();
          BULKIO::StreamSRI getSRI();
          BULKIO::PrecisionUTCTime getTime();
          std::set<std::string> getConnectionIds();
          OutAttachablePort *getPort() { return port; };
          
          //
          // Setters
          //
          void setStreamDefintion(StreamDefinition& stream_definition);
          void setName(const std::string& name);
          void setStreamId(const std::string& stream_id);
          void setSRI(BULKIO::StreamSRI sri);
          void setTime(BULKIO::PrecisionUTCTime time);
          void setPort( OutAttachablePort *p );
          
          //
          // Creating Attachments
          //
          void createNewAttachment(const std::string& connectionId, 
                                   typename PortType::_ptr_type port);
          
          //
          // State Validators
          //
          bool hasAttachId(const std::string& attachId);
          bool hasConnectionId(const std::string& connectionId);
          bool isValid();
  
          //
          // Search methods 
          //
          StreamAttachmentList findAttachmentsByAttachId(const std::string& attachId);
          StreamAttachmentList findAttachmentsByConnectionId(const std::string& connectionId);
          void updateAttachments(StreamAttachmentList expectedAttachments);
  
        private:
          StreamDefinition _streamDefinition;
          std::string _name;
          std::string _streamId;
          StreamAttachmentList _streamAttachments;
          BULKIO::StreamSRI _sri;
          BULKIO::PrecisionUTCTime _time;
          MUTEX attachmentsUpdateLock;
          LOGGER_PTR logger;
          OutAttachablePort  *port;
      };
      
      typedef typename std::vector<Stream> StreamList;
      typedef typename StreamList::iterator StreamIter;
  
      //
      // StreamContainer is a unique list of Stream objects
      //
      class StreamContainer {
  
        public:
  
          //
          // Constructors
          //
          StreamContainer( OutAttachablePort &p);

          void setLogger(LOGGER_PTR newLogger);

          // set the parent, used to log link statistics
          const OutAttachablePort &getPort() { return port; };
 
          //
          // Add/remove streams
          //
          void addStream(Stream& newStream);
          void removeStreamByStreamId(const std::string& streamId);

          //
          // State validators
          //
          bool hasStreamId(const std::string& streamId);
          bool hasAttachments();
  
          //
          // Getters
          //
          std::vector<std::string> getStreamIds();
          std::vector<std::string> getAttachIds();
          StreamList getStreams();
         
          //
          // Add connections/attachments
          //
          void addConnectionToAllStreams(const std::string& connectionId, typename PortType::_ptr_type port);
          void addConnectionToStream(const std::string& connectionId, typename PortType::_ptr_type port, const std::string& streamId);
  
          //
          // Time/SRI updates
          //
          void updateSRIForAllStreams(OutPortSriMap currentSRIs);
          void updateStreamSRI(const std::string& streamId, const BULKIO::StreamSRI sri);
          void updateStreamTime(const std::string& streamId, const BULKIO::PrecisionUTCTime time);
            
          //  
          // Detach
          //
          void detachByAttachIdConnectionId(const std::string& attachId, const std::string& connectionId);
          void detachByConnectionId(const std::string& connectionId);
          void detachByAttachId(const std::string& attachId);
         
          // 
          // Search methods
          //
          Stream* findByStreamId(const std::string& streamId);
          Stream* findByAttachId(const std::string& attachId);
          
          //
          // Debug helpers
          //
          void printState(const std::string& title);
          void printBlock(const std::string& title, const std::string& id, size_t indents);
  
        private:
          StreamList _streams;
          LOGGER_PTR logger;
          OutAttachablePort  &port;
      };
      
  private:

    typedef std::vector < std::pair<typename PortType::_var_type, std::string> > Connections;
    
    typedef typename Connections::iterator                                     ConnectionsIter;

    typedef std::map<std::string, std::pair<StreamDefinition*, std::string> >  AttachedStreams;

    typedef std::map<typename PortType::_var_type, std::string>                AttachedPorts;

  public:

    // Output Attachable port constructor. This class accepts 2 different interfaces
    //
    // ConnectNotifier - Providers of this interface will be notified when connectPort event happens
    // DisconnectNotifier - Providers of this interface will be notified when disconnectPort event happens
    //
    OutAttachablePort(std::string port_name,
                      ConnectionEventListener *connectCB=NULL,
                      ConnectionEventListener *disconnectCB=NULL );

    OutAttachablePort(std::string port_name,
                      LOGGER_PTR  new_logger,
                      ConnectionEventListener *connectCB=NULL,
                      ConnectionEventListener *disconnectCB=NULL );

    virtual ~OutAttachablePort();
   
    //
    // Allow users to set own Logger
    //
    void setLogger( LOGGER_PTR newLogger );

    //
    // Port Statistics Interface
    //
    virtual BULKIO::UsesPortStatisticsSequence * statistics(); 
    virtual BULKIO::PortUsageType state();
    virtual void enableStats(bool enable);
    virtual void setBitSize(double bitSize);
    virtual void updateStats(unsigned int elementsReceived, unsigned int queueSize, bool EOS, std::string streamID);
    virtual void updateStats( const std::string &cid );
    virtual bool reportConnectionErrors( const std::string &cid, const uint64_t n=1 );

    //
    // Uses Port Interface
    //
    virtual void connectPort(CORBA::Object_ptr connection, const char* connectionId);
    virtual void disconnectPort(const char* connectionId);
    virtual ExtendedCF::UsesConnectionSequence * connections();
    void updateConnectionFilter(const std::vector<bulkio::connection_descriptor_struct> &_filterTable);
    
    template< typename T > inline
      void setNewConnectListener(T &target, void (T::*func)( const char *connectionId )  ) {
      _connectCB =  boost::make_shared< MemberConnectionEventListener< T > >( boost::ref(target), func );
    };

    template< typename T > inline
      void setNewConnectListener(T *target, void (T::*func)( const char *connectionId )  ) {
      _connectCB =  boost::make_shared< MemberConnectionEventListener< T > >( boost::ref(*target), func );
    };

    template< typename T > inline
      void setNewDisconnectListener(T &target, void (T::*func)( const char *connectionId )  ) {
      _disconnectCB =  boost::make_shared< MemberConnectionEventListener< T > >( boost::ref(target), func );
    };

    template< typename T > inline
      void setNewDisconnectListener(T *target, void (T::*func)( const char *connectionId )  ) {
      _disconnectCB =  boost::make_shared< MemberConnectionEventListener< T > >( boost::ref(*target), func );
    };

    //
    // Attach listener interfaces for connect and disconnect events
    //
    void   setNewConnectListener( ConnectionEventListener *newListener );
    void   setNewConnectListener( ConnectionEventCallbackFn  newListener );
    void   setNewDisconnectListener( ConnectionEventListener *newListener );
    void   setNewDisconnectListener( ConnectionEventCallbackFn  newListener );

    //
    // pushSRI to allow for insertion of SRI context into data stream
    //
    void pushSRI(const BULKIO::StreamSRI& H, const BULKIO::PrecisionUTCTime& T);

    //
    // getStreamDefinition
    //
    // @return   StreamDefinition Return the stream definition for this attachId
    //           NULL attachId was not found
    //
    virtual StreamDefinition* getStreamDefinition(const char* attachId);

    //
    // getUser
    //
    // @return char * the user id that made the attachId request
    //         NULL attachId was not found
    //
    virtual char* getUser(const char* attachId);

    //
    // usageState
    //
    // If the number of attached streams == 0 
    //      return typename PortType::IDLE;
    // If the number of attached streams == 1
    //      return typename PortType::BUSY;
    // other
    //    return typename PortType::ACTIVE;
    //
    // @return   typename PortType::InputUsageState return the current state of the port base on how many attach requests have been received
    //
    virtual typename PortType::InputUsageState usageState();

    //
    // attachedStreams
    //
    // @return StreamSequence Returns a last Stream Defintion that was made. The caller
    //                                    is responsible for freeing the provided objects
    //
    virtual StreamSequence* attachedStreams();

    //
    // attachmentIds
    //
    // @return BULKIO::StringSequence Return the current list of attachment identifiers. The caller
    //                                is responsible for freeing the provided objects
    //
    virtual BULKIO::StringSequence* attachmentIds();
    
    //
    // attachmentIds
    //
    // @return BULKIO::StringSequence Return the current list of attachment identifiers. The caller
    //                                is responsible for freeing the provided objects
    //
    virtual BULKIO::StringSequence* attachmentIds(const std::string& streamId);

    //
    //  Attachable Interface
    //

    //
    // attach
    //
    // Send out a request to attach to the provided stream definition.  The end point servicing
    // this request will provide an attachment identifier that can be used by the detach request
    //    
    virtual char* attach(const StreamDefinition& stream, const char* userid) 
        throw (typename PortType::AttachError, typename PortType::StreamInputError);

    //
    // detach
    //
    // Send a request to detach from a stream definition for the provided attachment identifier.
    //
    virtual void detach(const char* attach_id );
    virtual void detach(const char* attach_id, const char *connection_id );

    //
    // add
    //
    virtual bool addStream(const StreamDefinition& stream);
    
    //
    // add
    //
    virtual bool updateStream(const StreamDefinition& stream);

    //
    // remove 
    //
    virtual void removeStream(const std::string& streamId);

	std::string getRepid() const;


  protected:

    typedef std::map<std::string, linkStatistics  >    _StatsMap;

    // mapping of current SRI objects to streamIDs
    OutPortSriMap  currentSRIs;

    // Connections List
    Connections   outConnections;

    MUTEX updatingPortsLock;

    // Provides housekeeping of streams->attachments->connections
    StreamContainer streamContainer;

    // track last attachment request made
    std::string  user_id;

    // connection sequence object to interface with outside world
    ExtendedCF::UsesConnectionSequence  recConnections;
    bool  recConnectionsRefresh;
  
    // list of statistic objects
    _StatsMap  stats;

    std::vector<bulkio::connection_descriptor_struct> filterTable;

  private:
    boost::shared_ptr< ConnectionEventListener >    _connectCB;
    boost::shared_ptr< ConnectionEventListener >    _disconnectCB;

    typename PortType::_ptr_type getConnectedPort(const std::string& connectionId);

    void updateSRIForAllConnections(); 
  };


  // 
  // SDDS Bulkio Input (sink/provides)/ Output (source/uses) definitions
  //
  typedef InAttachablePort<BULKIO::SDDSStreamDefinition, 
                           BULKIO::dataSDDS, 
                           BULKIO::SDDSStreamSequence, 
                           POA_BULKIO::dataSDDS> InSDDSPort;
  typedef OutAttachablePort<BULKIO::SDDSStreamDefinition, 
                            BULKIO::dataSDDS, 
                            BULKIO::SDDSStreamSequence> OutSDDSPort;
  
  // 
  // VITA4949 Bulkio Input (sink/provides)/ Output (source/uses) definitions
  //
  typedef InAttachablePort<BULKIO::VITA49StreamDefinition, 
                           BULKIO::dataVITA49, 
                           BULKIO::VITA49StreamSequence, 
                           POA_BULKIO::dataVITA49> InVITA49Port;
  typedef OutAttachablePort<BULKIO::VITA49StreamDefinition, 
                            BULKIO::dataVITA49, 
                            BULKIO::VITA49StreamSequence> OutVITA49Port;


}  // end of bulkio namespace

#endif
