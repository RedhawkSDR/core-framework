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
#ifndef __bulkio_in_port_h
#define __bulkio_in_port_h

#include <queue>
#include <list>
#include <vector>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>

#include <ossie/callback.h>
#include <ossie/signalling.h>

#include "bulkio_base.h"
#include "bulkio_traits.h"
#include "bulkio_in_stream.h"
#include "bulkio_callbacks.h"

namespace bulkio {

  //
  //  InPortBase
  //  Base template for data transfers between BULKIO ports.  This class is defined by 2 trait classes
  //    DataTransferTraits:  This template trait defines the DataTranfer object that is returned by the getPacket method
  //    PortTraits - This template provides the context for the port's middleware transport classes and they base data types
  //                 passed between port objects
  //
  template < typename PortTraits >
  class InPortBase : public PortTraits::POAPortType, public Port_Provides_base_impl
  {

  public:

    typedef PortTraits Traits;

    // Transport Sequence Type use to during push packet
    typedef typename Traits::SequenceType PortSequenceType;

    //
    // Transport type used by this port
    //
    typedef typename Traits::TransportType TransportType;

    //
    // True type of argument to pushPacket, typically "const PortSequenceType&"
    // except for dataXML and dataFile (which use "const char*")
    //
    typedef typename Traits::PushType PushArgumentType;

	typedef typename Traits::PortType PortType;

    //
    // Declaration of DataTransfer class from TransportType trait and DataBuffer type trait
    //
    typedef DataTransfer< typename Traits::DataTransferTraits > DataTransferType;

    // Queue of data transfer objects maintained by the port
    typedef std::deque< DataTransferType * > WorkQueue;

    //
    // ~InPortBase - call the virtual destructor to remove all allocated memebers
    //
    virtual ~InPortBase();

    /*
     * getPacket - interface used by components to grab data from the port's internal queue object for processing.  The timeout parameter allows
     * the calling component to perform blocking and non-blocking retrievals.
     *
     * @param timeout - timeout == bulkio::Const::NON_BLOCKING (0.0) non-blocking io
     *                  timeout == bulkio::Const::BLOCKING (-1) block until data arrives or lock is broken on exit
     *                  timeout > 0.0 wait until time expires.
     * @return dataTranfer *  pointer to a data transfer object from the port's work queue
     * @return NULL - no data available
     */
    virtual DataTransferType *getPacket(float timeout);

    /*
     * getPacket - interface used by components to grab data from the port's internal queue object for a specified streamID
     *
     * @param timeout - timeout == bulkio::Const::NON_BLOCKING (0.0) non-blocking io
     *                  timeout == bulkio::Const::BLOCKING (-1) block until data arrives or lock is broken on exit
     *                  timeout > 0.0 wait until time expires.
     * @param streamID  stream id to match on for when pulling data from the port's work queue
     * @return dataTranfer *  pointer to a data transfer object from the port's work queue
     * @return NULL - no data available
     */
    virtual DataTransferType *getPacket(float timeout, const std::string &streamID);

    //
    // BULKIO IDL interface for pushing Floating Point vectors between components
    //

    /*
     * pushSRI - called by the source component when SRI data about the stream changes, the data flow policy is this activity
     *           will occurr first before any data flows to the component.
     *
     * @param H - Incoming StreamSRI object that defines the state of the data flow portion of the stream (pushPacket)
     */
    virtual void pushSRI(const BULKIO::StreamSRI& H);

    //
    //  Port Statistics Interface
    //

    /*
     * turn on/off the port monitoring capability
     */
    virtual void enableStats(bool enable);

    //
    // state - returns the current state of the port as follows:
    //   BULKIO::BUSY - internal queue has reached FULL state
    //   BULKIO::IDLE - there are no items on the internal queue
    //   BULKIO::ACTIVE - there are items on the queue
    //
    // @return BULKIO::PortUsageType - current state of port
    //
    virtual BULKIO::PortUsageType state();

    //
    // statisics - returns a PortStatistics object for this provides port
    //     PortStatistics:
    //            portname - name of port
    //            elementsPerSecond - number of elements per second (element is based on size of port type )
    //            bitsPerSecond - number of bits per second (based on element storage size in bits)
    //            callsPerSecond - history window -1 / time between calls to this method
    //            streamIds - list of active stream id values
    //            averageQueueDepth - the average depth of the queue for this port
    //            timeSinceLastCall - time since this method as invoked and the last pushPacket happened
    //            Keyword Sequence - deprecated
    //
    // @return BULKIO::PortStatistics - current data flow metrics collected for the port.
    //                                  the caller of the method is responsible for freeing this object
    //
    virtual BULKIO::PortStatistics* statistics();

    //
    // activeSRIs - returns a sequence of BULKIO::StreamSRI objectsPort
    //
    // @return BULKIO::StreamSRISequence - list of activte SRI objects for this port
    //                                     the caller of the method is responsible for freeing this object
    //
    virtual BULKIO::StreamSRISequence* activeSRIs();

    /*
     * getCurrentQueueDepth - returns the current number of elements in the queue
     *
     * @return int  - number of items in the queue
     */
    virtual int getCurrentQueueDepth();

    /*
     *  getMaxQueueDepth - returns the maximum size of the queue , if this water mark is reached the queue will be purged, and the
     *                     component of the port will be notified in getPacket method
     * @return int - maximum size the queue can reach before purging occurs
     */
    virtual int getMaxQueueDepth();

    /*
     * setMaxQueueDepth - allow users of this port to modify the maximum number of allowable vectors on the queue.
     */
    virtual void setMaxQueueDepth(int newDepth);

    //
    // Allow the component to control the flow of data from the port to the component.  Block will restrict the flow of data back into the
    // component.  Call in component's stop method
    //
    virtual void block();

    //
    // Allow the component to control the flow of data from the port to the component.  Unblock will release the flow of data back into the
    // component. Called in component's start method.
    //
    virtual void unblock();

    //
    // Support function for automatic component-managed start.  Calls unblock.
    //
    virtual void startPort();

    //
    // Support function for automatic component-managed stop.  Calls block.
    //
    virtual void stopPort();

    /*
     * blocked
     *
     * @return bool returns state of breakBlock variable used to release any upstream blocking pushPacket calls
     */
    virtual bool blocked();
    
    /*
     * Assign a callback for notification when a new SRI StreamId is received
     */
    template< typename T > inline
      void setNewStreamListener(T &target, void (T::*func)( BULKIO::StreamSRI &)  ) {
      newStreamCallback =  boost::make_shared< MemberSriListener< T > >( boost::ref(target), func );
    };

    /*
     * Assign a callback for notification when a new SRI StreamId is received
     */
    template< typename T > inline
      void setNewStreamListener(T *target, void (T::*func)( BULKIO::StreamSRI &)  ) {
      newStreamCallback =  boost::make_shared< MemberSriListener< T > >( boost::ref(*target), func );

    };

    void setNewStreamListener( SriListener *newListener );

    void setNewStreamListener( SriListenerCallbackFn  newListener );

    void setLogger( LOGGER_PTR logger );

	// Return the interface that this Port supports
    std::string getRepid () const;

  protected:
    //
    // InPortBase  - creates a provides port that can accept data vectors from a source
    //
    // @param port_name  name of the port taken from .scd.xml file
    // @param sriCmp comparator function that accepts to StreamSRI objects and compares their contents,
    //                       if all members match then return true, otherwise false.  This is used during the pushSRI method
    // @param newStreamCB interface that is called when new SRI.streamID is received
    InPortBase(std::string port_name, 
               LOGGER_PTR   logger,
               bulkio::sri::Compare sriCmp = bulkio::sri::DefaultComparator,
               SriListener *newStreamCB = NULL );

    //
    // FIFO of data vectors and time stamps waiting to be processed by a component
    //
    WorkQueue                                      workQueue;

    //
    // Track size of work queue between getPacket calls when using streamID for extraction
    //
    uint32_t                                       lastQueueSize;

    //
    // SRI compare method used by pushSRI method to determine how to match incoming SRI objects and streamsID
    //
    bulkio::sri::Compare                           sri_cmp;

    //
    // Callback for notifications when new SRI streamID's are received
    //
    boost::shared_ptr< SriListener >              newStreamCallback;

    //
    //  List of SRI objects managed by StreamID
    //
    SriMap                                         currentHs;

    //
    // synchronizes access to the workQueue member
    //
    MUTEX                                          dataBufferLock;

    //
    // synchronizes access to the currentHs member
    //
    MUTEX                                          sriUpdateLock;

    //
    // mutex for use with condition variable to signify when data is available for consumption
    //  RESOVLE: combine deque and condition into template for pushing and poping items onto the queue... 
    //           refer to ConditionList.h example
    //
    MUTEX                                          dataAvailableMutex;

    CONDITION                                      dataAvailable;

    //
    //  used to control data flow from getPacket call
    //
    bool                                           breakBlock;

    //
    //  Transfers blocking request from data provider to this port that will block pushPacket calls if queue has reached a maximum value
    //
    bool                                           blocking;

    //
    //  An abstraction of a counting semaphore to control access to the workQueue member
    //
    queueSemaphore                                 *queueSem;

    //
    //  Statistics provider object used by the port monitoring interface
    //
    linkStatistics                                 *stats;

    LOGGER_PTR                                     logger;

    //
    // Synchronized waiter list for use in poll()
    //
    redhawk::signal<std::string> packetWaiters;

    //
    // Queues a packet received via pushPacket; in most cases, this method maps
    // exactly to pushPacket, except for dataFile
    //
    void queuePacket(PushArgumentType data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID);

    //
    // Returns a pointer to the first packet in the queue, blocking for up to
    // timeout seconds for one to be available
    //
    DataTransferType* peekPacket(float timeout);

    virtual void createStream(const std::string& streamID, const BULKIO::StreamSRI& sri);
    virtual void removeStream(const std::string& streamID);

    virtual bool isStreamActive(const std::string& streamID);
    virtual bool isStreamEnabled(const std::string& streamID);

    DataTransferType* fetchPacket(const std::string& streamID);
    void packetReceived(const std::string& streamID);

    friend class InputStream<PortTraits>;
    size_t samplesAvailable(const std::string& streamID, bool firstPacket);

    //
    // Returns the total number of elements of data in a pushPacket call, for
    // statistical tracking; enables XML and File specialization, which have
    // different notions of size
    //
    int _getElementLength(PushArgumentType data);
  };

  template < typename PortTraits >
  class InPort : public InPortBase<PortTraits>
  {
  public:
    typedef PortTraits  Traits;

    // Port Variable Type
    typedef typename Traits::POAPortType   PortVarType;

    //  Interface Type
    typedef typename  Traits::PortType      PortType;

    //  Interface Type
    typedef typename  Traits::PortType      ProvidesPortType;

    // Transport Sequence Type use to during push packet
    typedef typename Traits::SequenceType    PortSequenceType;

    //
    // Transport type used by this port
    //
    typedef typename Traits::TransportType  TransportType;

    //
    // Native type mapping of TransportType
    //
    typedef typename Traits::NativeType      NativeType;

    //
    // Declaration of DataTransfer class from TransportType trait and DataBuffer type trait
    //
    typedef DataTransfer< typename Traits::DataTransferTraits > DataTransferType;

    // backwards compatible definition
    typedef DataTransferType dataTransfer;

    // queue of dataTranfer objects maintained by the port
    typedef   std::deque< DataTransferType * > WorkQueue;

    // Input stream interface used by this port
    typedef InputStream<PortTraits> StreamType;

    // List type for input streams provided by this port
    typedef std::list<StreamType> StreamList;

    //
    // InPort - creates a provides port that can accept data vectors from a source
    //
    // @param port_name  name of the port taken from .scd.xml file
    // @param sriCmp comparator function that accepts to StreamSRI objects and compares their contents,
    //                       if all members match then return true, otherwise false.  This is used during the pushSRI method
    // @param newStreamCB interface that is called when new SRI.streamID is received
    InPort(std::string port_name, 
               LOGGER_PTR   logger,
               bulkio::sri::Compare sriCmp = bulkio::sri::DefaultComparator,
               SriListener *newStreamCB = NULL );

    InPort(std::string port_name, 
           bulkio::sri::Compare sriCmp = bulkio::sri::DefaultComparator,
           SriListener *newStreamCB = NULL );
       
    InPort(std::string port_name, void *);

    //
    // pushPacket called by the source component when pushing a vector of data into a component.  This method will save off the data
    //            vector, timestamp, EOS and streamID onto a queue for consumption by the component via the getPacket method
    //
    // @param data - the vector of data to be consumed
    // @param T    - a time stamp for the data, the time represents the associated time value for the first entry of the data vector
    // @param EOS  - indicator that the stream has ended, (stream is identified by streamID)
    // @param streamID - name of the stream the vector and stream context data are associated with
    virtual void pushPacket(const PortSequenceType& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID);

    //
    // Stream-based input API
    //

    // Returns the stream that should be used for the next basic read
    StreamType getCurrentStream(float timeout=bulkio::Const::BLOCKING);

    // Returns the current stream with the given stream ID, if available
    StreamType getStream(const std::string& streamID);

    StreamList getStreams();

    StreamList pollStreams(float timeout);
    StreamList pollStreams(StreamList& pollset, float timeout);

    StreamList pollStreams(size_t samples, float timeout);
    StreamList pollStreams(StreamList& pollset, size_t samples, float timeout);

    template <class Target, class Func>
    void addStreamListener(Target target, Func func) {
      streamAdded.add(target, func);
    }

    template <class Target, class Func>
    void removeStreamListener(Target target, Func func) {
      streamAdded.remove(target, func);
    }

  protected:
    typedef InPortBase<PortTraits> super;
    using super::packetWaiters;
    using super::logger;

    // Allow the input stream type friend access so it can call removeStream()
    // when it acknowledges an end-of-stream
    friend class InputStream<PortTraits>;

    //
    // Notification for new stream creation
    //
    ossie::notification<void (StreamType)> streamAdded;

    //
    // Streams that are currently active
    //
    typedef std::map<std::string,StreamType> StreamMap;
    StreamMap streams;
    boost::mutex streamsMutex;

    // Streams that have the same stream ID as an active stream, when an
    // end-of-stream has been queued but not yet read 
    std::multimap<std::string,StreamType> pendingStreams;

    virtual void createStream(const std::string& streamID, const BULKIO::StreamSRI& sri);
    virtual void removeStream(const std::string& streamID);

    virtual bool isStreamActive(const std::string& streamID);
    virtual bool isStreamEnabled(const std::string& streamID);

    StreamList getReadyStreams(size_t samples);
  };

  //
  //  InStringPort
  //  Base template for simple data transfers between Input/Output ports.  This class is defined by 2 trait classes
  //    DataTransferTraits:  This template trait defines the DataTranfer object that is returned by the getPacket method
  //    PortTraits - This template provides the context for the port's middleware transport classes and they base data types
  //                 passed between port objects
  //
  //  Both classes have a simlar types of TransportType and SequenceType and the DataTransferTraits defines the the type for the 
  //  data buffer used to store incoming streams of data.  These 2 class should be combined to described InputPortTraits.
  //


  template < typename PortTraits >
  class InStringPort : public InPortBase<PortTraits>
  {

  public:

    typedef PortTraits  Traits;

    // Port Variable Type
    typedef typename Traits::POAPortType    PortVarType;

    //  Interface Type
    typedef typename  Traits::PortType      PortType;

    //  Interface Type
    typedef typename  Traits::PortType      ProvidesPortType;

    // Transport Sequence Type use to during push packet
    typedef char *                             PortSequenceType;

    //
    // Transport type used by this port
    //
    typedef typename Traits::TransportType  TransportType;

    //
    //  Native type mapping of TransportType
    //
    typedef typename Traits::NativeType      NativeType;

    //
    // Data transfer object from ports to components
    //
    typedef DataTransfer< typename Traits::DataTransferTraits >  DataTransferType;


    // backwards compatible defintion
    typedef DataTransfer< typename Traits::DataTransferTraits > dataTransfer;


    // queue of dataTranfer objects maintained by the port
    typedef   std::deque< DataTransferType * >       WorkQueue;


    //
    // InStringPort  - creates a provides port that can accept floating point vectors from a source
    //
    // @param port_name  name of the port taken from .scd.xml file
    // @param SriCompareFunc comparator function that accepts to StreamSRI objects and compares their contents,
    //                       if all members match then return true, otherwise false.  This is used during the pushSRI method
    // @param newStreamCB interface that is called when new SRI.streamID is received

    InStringPort(std::string port_name, 
      LOGGER_PTR  logger,
      bulkio::sri::Compare = bulkio::sri::DefaultComparator,
      SriListener *newStreamCB = NULL );

    InStringPort(std::string port_name, 
         bulkio::sri::Compare = bulkio::sri::DefaultComparator,
         SriListener *newStreamCB = NULL );

    InStringPort(std::string port_name, void * );

    //
    // pushPacket called by the source component when pushing a vector of data into a component.  This method will save off the data
    //            vector, timestamp, EOS and streamID onto a queue for consumption by the component via the getPacket method
    //
    // @param data - the vector of data to be consumed
    // @param T    - a time stamp for the data, the time represents the associated time value for the first entry of the data vector
    // @param EOS  - indicator that the stream has ended, (stream is identified by streamID)
    // @param streamID - name of the stream the vector and stream context data are associated with
    virtual void pushPacket(const char *data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID);


    //
    // pushPacket called by the source component when pushing a vector of data into a component.  This method will save off the data
    //            vector, timestamp, EOS and streamID onto a queue for consumption by the component via the getPacket method
    //
    // @param data - the vector of data to be consumed
    // @param EOS  - indicator that the stream has ended, (stream is identified by streamID)
    // @param streamID - name of the stream the vector and stream context data are associated with
    virtual void pushPacket( const char *data, CORBA::Boolean EOS, const char* streamID);
  };


  /*
     Provides Port Definitions for All Bulk IO pushPacket Port definitions
     *
     */
  // Bulkio char (Int8) input
  typedef InPort< CharPortTraits >                 InCharPort;
  // Bulkio octet (UInt8) input
  typedef InPort< OctetPortTraits >                InOctetPort;
  // Bulkio Int8 input
  typedef InCharPort                               InInt8Port;
  // Bulkio UInt8 input
  typedef InOctetPort                              InUInt8Port;
  // Bulkio short (Int16) input
  typedef InPort< ShortPortTraits >                InShortPort;
  // Bulkio unsigned short (UInt16) input
  typedef InPort< UShortPortTraits >               InUShortPort;
  // Bulkio Int16 input
  typedef InShortPort                              InInt16Port;
  // Bulkio UInt16 input
  typedef InUShortPort                             InUInt16Port;
  // Bulkio long (Int32) input
  typedef InPort< LongPortTraits >                 InLongPort;
  // Bulkio unsigned long (UInt32) input
  typedef InPort< ULongPortTraits >                InULongPort;
  // Bulkio Int32 input
  typedef InLongPort                               InInt32Port;
  // Bulkio UInt32 input
  typedef InULongPort                              InUInt32Port;
  // Bulkio long long (Int64) input
  typedef InPort< LongLongPortTraits >             InLongLongPort;
  // Bulkio unsigned long long (UInt64) input
  typedef InPort< ULongLongPortTraits >            InULongLongPort;
  // Bulkio Int64 input
  typedef InLongLongPort                           InInt64Port;
  // Bulkio UInt64 input
  typedef InULongLongPort                          InUInt64Port;
  // Bulkio float input
  typedef InPort< FloatPortTraits >                InFloatPort;
  // Bulkio double input
  typedef InPort< DoublePortTraits >               InDoublePort;
  // Bulkio URL input
  typedef InStringPort< URLPortTraits >            InURLPort;
  // Bulkio File (URL) input
  typedef InStringPort< FilePortTraits >           InFilePort;
  // Bulkio XML input
  typedef InStringPort< XMLPortTraits >            InXMLPort;




}  // end of bulkio namespace


#endif
