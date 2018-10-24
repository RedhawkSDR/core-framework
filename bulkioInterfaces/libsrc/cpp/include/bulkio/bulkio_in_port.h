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

#include <ossie/callback.h>
#include <ossie/signalling.h>
#include <ossie/ProvidesPort.h>

#include "bulkio_base.h"
#include "bulkio_typetraits.h"
#include "bulkio_datatransfer.h"
#include "bulkio_in_stream.h"
#include "bulkio_callbacks.h"

namespace bulkio {

  template <class PortType>
  class LocalTransport;

  template <typename PortType>
  class InputTransport;

  template <class PortType>
  struct InStreamTraits {
      typedef BufferedInputStream<PortType> InStreamType;
  };

  template <>
  struct InStreamTraits<BULKIO::dataXML> {
      typedef InXMLStream InStreamType;
  };

  template <>
  struct InStreamTraits<BULKIO::dataFile> {
      typedef InFileStream InStreamType;
  };

  //
  //  InPort
  //  Base template for data transfers between BULKIO ports.  This class is defined by 2 trait classes
  //    DataTransferTraits:  This template trait defines the DataTranfer object that is returned by the getPacket method
  //    PortTraits - This template provides the context for the port's middleware transport classes and they base data types
  //                 passed between port objects
  //
  template <typename PortType>
  class InPort : public redhawk::NegotiableProvidesPortBase
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
               , public virtual CorbaTraits<PortType>::POATypeExt
#endif
  {
  public:
    // The CORBA interface of this port (nested typedef for template parameter)
    typedef PortType CorbaType;

    // Transport Sequence Type use to during push packet
    typedef typename CorbaTraits<PortType>::SequenceType PortSequenceType;

    //
    // Transport type used by this port
    //
    typedef typename CorbaTraits<PortType>::TransportType TransportType;

    //
    // Declaration of DataTransfer class from TransportType trait and DataBuffer type trait
    //
    typedef typename BufferTraits<PortType>::VectorType VectorType;
    typedef DataTransfer<VectorType> DataTransferType;

    // backwards compatible definition
    typedef DataTransferType dataTransfer;

    // Input stream interface used by this port
    typedef typename InStreamTraits<PortType>::InStreamType StreamType;

    // List type for input streams provided by this port
    typedef std::list<StreamType> StreamList;

    //
    // ~InPort - call the virtual destructor to remove all allocated memebers
    //
    virtual ~InPort();

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
    DataTransferType *getPacket(float timeout);

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
    DataTransferType *getPacket(float timeout, const std::string& streamID);

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
    void enableStats(bool enable);

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
    int getCurrentQueueDepth();

    /*
     *  getMaxQueueDepth - returns the maximum size of the queue , if this water mark is reached the queue will be purged, and the
     *                     component of the port will be notified in getPacket method
     * @return int - maximum size the queue can reach before purging occurs
     */
    int getMaxQueueDepth();

    /*
     * setMaxQueueDepth - allow users of this port to modify the maximum number of allowable vectors on the queue.
     */
    void setMaxQueueDepth(int newDepth);

    //
    // Allow the component to control the flow of data from the port to the component.  Block will restrict the flow of data back into the
    // component.  Call in component's stop method
    //
    void block();

    //
    // Allow the component to control the flow of data from the port to the component.  Unblock will release the flow of data back into the
    // component. Called in component's start method.
    //
    void unblock();

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
    bool blocked();

    /**
     * @brief  Registers a callback for new streams.
     * @param target  Class instance.
     * @param func  Member function pointer.
     */
    template <class Target, class Func>
    void addStreamListener(Target target, Func func) {
      streamAdded.add(target, func);
    }

    /**
     * @brief  Unregisters a callback for new streams.
     * @param target  Class instance.
     * @param func  Member function pointer.
     */
    template <class Target, class Func>
    void removeStreamListener(Target target, Func func) {
      streamAdded.remove(target, func);
    }

    /**
     * @brief  Gets the stream that should be used for the next basic read.
     * @param timeout  Seconds to wait for a stream; a negative value waits
     *                 indefinitely.
     * @returns  Input stream ready for reading on success.
     * @returns  Null input stream if timeout expires or port is stopped.
     */
    StreamType getCurrentStream(float timeout=bulkio::Const::BLOCKING);

    /**
     * @brief  Get the active stream with the given stream ID.
     * @param streamID  Stream identifier.
     * @returns  Input stream for @p streamID if it exists.
     * @returns  Null input stream if no such stream ID exits.
     */
    StreamType getStream(const std::string& streamID);

    /**
     * @brief  Gets the current set of active streams.
     * @returns  List of streams.
     */
    StreamList getStreams();

    /*
     * Assign a callback for notification when a new SRI StreamId is received
     */
    template <typename Target, typename Func>
    inline void setNewStreamListener(Target target, Func func) {
      newStreamCallback.assign(target, func);
    }

    /*
     * Assign a callback for notification when a new SRI StreamId is received
     */
    template <typename Func>
    inline void setNewStreamListener(Func func) {
      newStreamCallback = func;
    }

    void setNewStreamListener(SriListener *newListener);

	// Return the interface that this Port supports
    std::string getRepid () const;

  protected:
    //
    // InPort  - creates a provides port that can accept data vectors from a source
    //
    // @param port_name  name of the port taken from .scd.xml file
    // @param sriCmp comparator function that accepts to StreamSRI objects and compares their contents,
    //                       if all members match then return true, otherwise false.  This is used during the pushSRI method
    // @param newStreamCB interface that is called when new SRI.streamID is received
    InPort(std::string port_name, 
           LOGGER_PTR   logger,
           bulkio::sri::Compare sriCmp = bulkio::sri::DefaultComparator,
           SriListener *newStreamCB = NULL );

    typedef typename BufferTraits<PortType>::BufferType BufferType;
    
    struct Packet {
      Packet(const BufferType& buffer, const BULKIO::PrecisionUTCTime& T, bool EOS, const StreamDescriptor& SRI, bool sriChanged, bool inputQueueFlushed) :
        buffer(buffer),
        T(T),
        EOS(EOS),
        SRI(SRI),
        sriChanged(sriChanged),
        inputQueueFlushed(inputQueueFlushed),
        streamID(SRI.streamID())
      {
      }

      BufferType buffer;
      BULKIO::PrecisionUTCTime T;
      bool EOS;
      StreamDescriptor SRI;
      bool sriChanged;
      bool inputQueueFlushed;
      std::string streamID;
    };

    //
    // FIFO of data vectors and time stamps waiting to be processed by a component
    //
    typedef std::deque<Packet*> PacketQueue;
    PacketQueue packetQueue;

    //
    // SRI compare method used by pushSRI method to determine how to match incoming SRI objects and streamsID
    //
    bulkio::sri::Compare                           sri_cmp;

    //
    // Callback for notifications when new SRI streamID's are received
    //
    redhawk::callback<void (BULKIO::StreamSRI&)> newStreamCallback;

    //
    //  List of SRI objects managed by StreamID
    //
    typedef std::map<std::string,std::pair<StreamDescriptor,bool> > SriTable;
    SriTable currentHs;

    //
    // synchronizes access to the workQueue member
    //
    MUTEX                                          dataBufferLock;
    CONDITION dataAvailable;
    CONDITION queueAvailable;
    size_t maxQueue;

    //
    // synchronizes access to the currentHs member
    //
    MUTEX                                          sriUpdateLock;

    //
    //  used to control data flow from getPacket call
    //
    bool                                           breakBlock;

    //
    //  Transfers blocking request from data provider to this port that will block pushPacket calls if queue has reached a maximum value
    //
    bool                                           blocking;

    //
    //  Statistics provider object used by the port monitoring interface
    //
    linkStatistics                                 *stats;

    //
    // Synchronized waiter list for use in poll()
    //
    redhawk::signal<std::string> packetWaiters;

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

    // Allow non-CORBA data ingress (shared memory, VITA49)
    friend class InputTransport<PortType>;

    //
    // Queues a packet received via pushPacket; in most cases, this method maps
    // exactly to pushPacket, except for dataFile
    //
    void queuePacket(const BufferType& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const std::string& streamID);

    // Allow local transport classes to directly queue packets
    friend class LocalTransport<PortType>;

    //
    // Fetches the next packet for the given stream ID, blocking for up to
    // timeout seconds for one to be available
    //
    Packet* nextPacket(float timeout, const std::string& streamID);

    //
    // Returns a pointer to the first packet in the queue, blocking for up to
    // timeout seconds for one to be available
    //
    Packet* peekPacket(float timeout, boost::unique_lock<boost::mutex>& lock);

    Packet* fetchPacket(const std::string& streamID);

    // Discard currently queued packets for the given stream ID, up to the
    // first end-of-stream
    void discardPacketsForStream(const std::string& streamID);

    friend class InputStream<PortType>;
    size_t samplesAvailable(const std::string& streamID, bool firstPacket);

    void createStream(const std::string& streamID, const StreamDescriptor& sri);
    void removeStream(const std::string& streamID);

    bool isStreamActive(const std::string& streamID);
    bool isStreamEnabled(const std::string& streamID);

    // Purges the input queue, discarding existing packets while preserving
    // end-of-stream and SRI change flags; must hold both dataBufferLock and
    // sriUpdateLock
    void _flushQueue();

    // Checks whether the packet should be queued or discarded; also handles
    // notifying disabled streams of end-of-stream if the packet is being
    // discarded
    bool _acceptPacket(const std::string& streamID, bool EOS);

    // Stops tracking the SRI for streamID, returning true if the stream was
    // the last blocking stream, indicating that blocking can be turned off
    // for the work queue
    bool _handleEOS(const std::string& streamID);

    //
    // Returns the total number of elements of data in a pushPacket call, for
    // statistical tracking; enables XML and File specialization, which have
    // different notions of size
    //
    int _getElementLength(const BufferType& data);
  };

  template <typename PortType>
  class InNumericPort : public InPort<PortType>
  {
  public:
    // Transport Sequence Type use to during push packet
    typedef typename InPort<PortType>::PortSequenceType    PortSequenceType;

    //
    // Transport type used by this port
    //
    typedef typename InPort<PortType>::TransportType  TransportType;

    //
    // Native type mapping of TransportType
    //
    typedef typename NativeTraits<PortType>::NativeType NativeType;

    typedef typename InPort<PortType>::StreamType StreamType;

    typedef typename InPort<PortType>::StreamList StreamList;

    //
    // InNumericPort - creates a provides port that can accept data vectors from a source
    //
    // @param port_name  name of the port taken from .scd.xml file
    // @param sriCmp comparator function that accepts to StreamSRI objects and compares their contents,
    //                       if all members match then return true, otherwise false.  This is used during the pushSRI method
    // @param newStreamCB interface that is called when new SRI.streamID is received
    InNumericPort(std::string port_name, 
                  LOGGER_PTR logger,
                  bulkio::sri::Compare sriCmp = bulkio::sri::DefaultComparator,
                  SriListener *newStreamCB = NULL);

    InNumericPort(std::string port_name, 
                  bulkio::sri::Compare sriCmp = bulkio::sri::DefaultComparator,
                  SriListener *newStreamCB = NULL);
       
    InNumericPort(std::string port_name, void *);

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

    StreamList pollStreams(float timeout);
    StreamList pollStreams(StreamList& pollset, float timeout);

    StreamList pollStreams(size_t samples, float timeout);
    StreamList pollStreams(StreamList& pollset, size_t samples, float timeout);

  protected:
    // Shared buffer type used for local transfers
    typedef typename InPort<PortType>::BufferType BufferType;

    typedef InPort<PortType> super;
    using super::packetWaiters;
    using super::_portLog;
    typedef typename super::StreamMap StreamMap;
    using super::streams;
    using super::streamsMutex;
    typedef typename super::Packet Packet;

    StreamList getReadyStreams(size_t samples);
  };

  class InBitPort : public InPort<BULKIO::dataBit>
  {
  public:
    InBitPort(const std::string& name, LOGGER_PTR logger=LOGGER_PTR());

    virtual void pushPacket(const BULKIO::BitSequence& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID);
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


  class InFilePort : public InPort<BULKIO::dataFile>
  {
  public:
    //
    // InStringPort  - creates a provides port that can accept floating point vectors from a source
    //
    // @param port_name  name of the port taken from .scd.xml file
    // @param SriCompareFunc comparator function that accepts to StreamSRI objects and compares their contents,
    //                       if all members match then return true, otherwise false.  This is used during the pushSRI method
    // @param newStreamCB interface that is called when new SRI.streamID is received

    InFilePort(std::string port_name, 
               LOGGER_PTR  logger,
               bulkio::sri::Compare=bulkio::sri::DefaultComparator,
               SriListener* newStreamCB=0);

    InFilePort(std::string port_name, 
               bulkio::sri::Compare=bulkio::sri::DefaultComparator,
               SriListener* newStreamCB=0);

    InFilePort(std::string port_name, void*);

    //
    // pushPacket called by the source component when pushing a vector of data into a component.  This method will save off the data
    //            vector, timestamp, EOS and streamID onto a queue for consumption by the component via the getPacket method
    //
    // @param data - the vector of data to be consumed
    // @param T    - a time stamp for the data, the time represents the associated time value for the first entry of the data vector
    // @param EOS  - indicator that the stream has ended, (stream is identified by streamID)
    // @param streamID - name of the stream the vector and stream context data are associated with
    virtual void pushPacket(const char *data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID);
  };


  class InXMLPort : public InPort<BULKIO::dataXML>
  {
  public:
    InXMLPort(std::string port_name, LOGGER_PTR logger,
              bulkio::sri::Compare=bulkio::sri::DefaultComparator,
              SriListener* newStreamCB=NULL);

    InXMLPort(std::string port_name, 
              bulkio::sri::Compare=bulkio::sri::DefaultComparator,
              SriListener* newStreamCB=NULL);

    InXMLPort(std::string port_name, void*);
    
    //
    // pushPacket called by the source component when pushing a vector of data into a component.  This method will save off the data
    //            vector, timestamp, EOS and streamID onto a queue for consumption by the component via the getPacket method
    //
    // @param data - the vector of data to be consumed
    // @param EOS  - indicator that the stream has ended, (stream is identified by streamID)
    // @param streamID - name of the stream the vector and stream context data are associated with
    virtual void pushPacket(const char *data, CORBA::Boolean EOS, const char* streamID);

    void pushPacket(const char* data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID) __attribute__ ((deprecated));
  };


  /*
     Provides Port Definitions for All Bulk IO pushPacket Port definitions
     *
     */
  // Bulkio char (Int8) input
  typedef InNumericPort<BULKIO::dataChar>      InCharPort;
  // Bulkio octet (UInt8) input
  typedef InNumericPort<BULKIO::dataOctet>     InOctetPort;
  // Bulkio Int8 input
  typedef InCharPort                           InInt8Port;
  // Bulkio UInt8 input
  typedef InOctetPort                          InUInt8Port;
  // Bulkio short (Int16) input
  typedef InNumericPort<BULKIO::dataShort>     InShortPort;
  // Bulkio unsigned short (UInt16) input
  typedef InNumericPort<BULKIO::dataUshort>    InUShortPort;
  // Bulkio Int16 input
  typedef InShortPort                          InInt16Port;
  // Bulkio UInt16 input
  typedef InUShortPort                         InUInt16Port;
  // Bulkio long (Int32) input
  typedef InNumericPort<BULKIO::dataLong>      InLongPort;
  // Bulkio unsigned long (UInt32) input
  typedef InNumericPort<BULKIO::dataUlong>     InULongPort;
  // Bulkio Int32 input
  typedef InLongPort                           InInt32Port;
  // Bulkio UInt32 input
  typedef InULongPort                          InUInt32Port;
  // Bulkio long long (Int64) input
  typedef InNumericPort<BULKIO::dataLongLong>  InLongLongPort;
  // Bulkio unsigned long long (UInt64) input
  typedef InNumericPort<BULKIO::dataUlongLong> InULongLongPort;
  // Bulkio Int64 input
  typedef InLongLongPort                       InInt64Port;
  // Bulkio UInt64 input
  typedef InULongLongPort                      InUInt64Port;
  // Bulkio float input
  typedef InNumericPort<BULKIO::dataFloat>     InFloatPort;
  // Bulkio double input
  typedef InNumericPort<BULKIO::dataDouble>    InDoublePort;
  // Maintained for backwards compatibility
  typedef InFilePort InURLPort;

}  // end of bulkio namespace


#endif
