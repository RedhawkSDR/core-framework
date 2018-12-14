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

#ifndef __bulkio_out_port_h
#define __bulkio_out_port_h

#include <queue>
#include <list>
#include <vector>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>

#include <ossie/CorbaUtils.h>
#include <ossie/UsesPort.h>

#include "bulkio_base.h"
#include "bulkio_typetraits.h"
#include "bulkio_callbacks.h"
#include "bulkio_out_stream.h"
#include "BulkioTransport.h"

namespace bulkio {

  template <class PortType>
  struct OutStreamTraits
  {
      typedef NumericOutputStream<PortType> OutStreamType;
  };

  template <>
  struct OutStreamTraits<BULKIO::dataBit> {
      typedef OutBitStream OutStreamType;
  };

  template <>
  struct OutStreamTraits<BULKIO::dataXML> {
      typedef OutXMLStream OutStreamType;
  };

  template <>
  struct OutStreamTraits<BULKIO::dataFile> {
      typedef OutFileStream OutStreamType;
  };

  //
  //  OutPort
  //
  //  Base template for data transfers between BULKIO ports.  This class is defined by 2 trait classes
  //    PortTraits - This template provides the context for the port's middleware transport classes and they base data types
  //                 passed between port objects
  //
  //
  template <typename PortType>
  class OutPort : public redhawk::NegotiableUsesPort
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
                , public virtual POA_BULKIO::internal::UsesPortStatisticsProviderExt
#endif
  {

  public:
    // The CORBA interface of this port (nested typedef for template parameter)
    typedef PortType CorbaType;

    //
    // Port Variable Definition
    //
    typedef typename PortType::_var_type PortVarType;

    //
    // Sequence container used during actual pushPacket call
    //
    typedef typename CorbaTraits<PortType>::SequenceType PortSequenceType;

    //
    // OutputStream class
    //
    typedef typename OutStreamTraits<PortType>::OutStreamType StreamType;

    //
    // ConnectionList Definition
    //
    typedef typename  bulkio::Connections< PortVarType >::List        ConnectionsList;

    //
    // Mapping of Stream IDs to SRI Map/Refresh objects
    //
    typedef std::map< std::string, SriMapStruct >                    OutPortSriMap;

    //
    // OutPort Creates a uses port object for publishing data to the framework
    //
    // @param name  name assigned to the port located in scd.xml file
    // @param logger  logger to receive port logging output
    // @param connectionCB  callback that will be called when the connectPort method is called
    // @pararm disconnectDB callback that receives notification when a disconnectPort happens
    //
    OutPort(const std::string& name, 
            LOGGER_PTR logger,
            ConnectionEventListener *connectCB=NULL,
            ConnectionEventListener *disconnectCB=NULL);

    
    //
    // virtual destructor to clean up resources
    //
    virtual ~OutPort();

    void updateConnectionFilter(const std::vector<connection_descriptor_struct> &_filterTable) {
        SCOPED_LOCK lock(updatingPortsLock);   // don't want to process while command information is coming in
        filterTable = _filterTable;
    };
    

    template< typename T > inline
    void setNewConnectListener(T &target, void (T::*func)( const char *connectionId )  )
    {
      _connectCB =  boost::make_shared< MemberConnectionEventListener< T > >( boost::ref(target), func );
    }

    template< typename T > inline
    void setNewConnectListener(T *target, void (T::*func)( const char *connectionId )  )
    {
      _connectCB =  boost::make_shared< MemberConnectionEventListener< T > >( boost::ref(*target), func );
    }

    template< typename T > inline
    void setNewDisconnectListener(T &target, void (T::*func)( const char *connectionId )  )
    {
      _disconnectCB =  boost::make_shared< MemberConnectionEventListener< T > >( boost::ref(target), func );
    }

    template< typename T > inline
    void setNewDisconnectListener(T *target, void (T::*func)( const char *connectionId )  )
    {
      _disconnectCB =  boost::make_shared< MemberConnectionEventListener< T > >( boost::ref(*target), func );
    }

    //
    // Attach listener interfaces for connect and disconnect events
    //
    void setNewConnectListener( ConnectionEventListener *newListener );
    void setNewConnectListener( ConnectionEventCallbackFn  newListener );
    void setNewDisconnectListener( ConnectionEventListener *newListener );
    void setNewDisconnectListener( ConnectionEventCallbackFn  newListener );

    //
    // pushSRI - called by the source component when SRI data about the stream changes, the data flow policy is this activity
    //           will occurr first before any data flows to the component.
    //
    // @param H - Incoming StreamSRI object that defines the state of the data flow portion of the stream (pushPacket)
    //
    virtual void pushSRI(const BULKIO::StreamSRI& H);


    //
    // statisics - returns a PortStatistics object for this uses port
    //      BULKIO::UsesPortStatisticsSequence: sequence of PortStatistics object
    //           PortStatistics
    //            portname - name of port
    //            elementsPerSecond - number of elements per second (element is based on size of port type )
    //            bitsPerSecond - number of bits per second (based on element storage size in bits)
    //            callsPerSecond - history window -1 / time between calls to this method
    //            streamIds - list of active stream id values
    //            averageQueueDepth - the average depth of the queue for this port
    //            timeSinceLastCall - time since this method as invoked and the last pushPacket happened
    //            Keyword Sequence - deprecated
    //
    // @return  BULKIO::UsesPortStatisticsSequenc - current data flow metrics collected for the port, the caller of the method
    //                                  is responsible for freeing this object
    //
    virtual BULKIO::UsesPortStatisticsSequence * statistics();

    //
    // state - returns the current state of the port as follows:
    //   BULKIO::BUSY - internal queue has reached FULL state
    //   BULKIO::IDLE - there are no items on the internal queue
    //   BULKIO::ACTIVE - there are items on the queue
    //
    // @return BULKIO::PortUsageType - current state of port
    //
    virtual BULKIO::PortUsageType state();

    /**
     * @brief  Gets the current set of active streams.
     * @returns  List of streams.
     */
    typedef std::list<StreamType> StreamList;
    StreamList getStreams();

    /**
     * @brief  Get the active stream with the given stream ID.
     * @param streamID  Stream identifier.
     * @returns  Output stream for @p streamID if it exists.
     * @returns  Null output stream if no such stream ID exists.
     */
    StreamType getStream(const std::string& streamID);

    /**
     * @brief  Creates a new output stream.
     * @param streamID  Stream identifier.
     * @returns  A new output stream.
     *
     * The returned output stream's SRI is initialized with default values.
     */
    StreamType createStream(const std::string& streamID);

    /**
     * @brief  Creates a new stream based on an existing SRI.
     * @param sri  Stream SRI.
     * @returns  A new output stream.
     */
    StreamType createStream(const BULKIO::StreamSRI& sri);

    //
    // turn on/off the port monitoring capability
    //
    void enableStats(bool enable);

    //
    // Return map of streamID/SRI objects 
    //
    bulkio::SriMap getCurrentSRI();

    //
    // Return list of SRI objects
    //
    bulkio::SriList getActiveSRIs();

    //
    // Return a ConnectionsList for the current ports and connections ids establish via connectPort method
    //
    ConnectionsList getConnections();

    //
    // Deprecation Warning
    //
    // The _getConnections and currentSRIs access will be deprecated in the next release of the 
    // the bulkio library class, in favor of getCurrentSRI and getConnections.
    //

    //
    // Allow access to the port's connection list
    //
    ConnectionsList __attribute__ ((deprecated)) _getConnections() {
      return getConnections();
    }

    std::string getRepid () const;

    //
    // List of SRIs sent out by this port
    //
    OutPortSriMap                            currentSRIs __attribute__ ((deprecated));

  protected:
    //
    // Shared buffer type used to transfer data without copies, where possible
    //
    typedef typename BufferTraits<PortType>::BufferType BufferType;

    //
    // Lookup table for connections to input ports in the same process space
    //
    typedef OutputTransport<PortType> PortTransportType;

    virtual redhawk::UsesTransport* _createLocalTransport(PortBase* port, CORBA::Object_ptr object, const std::string& connectionId);

    virtual redhawk::UsesTransport* _createTransport(CORBA::Object_ptr object, const std::string& connectionId);

    typedef redhawk::UsesPort::TransportIteratorAdapter<PortTransportType> TransportIterator;

    typedef std::map<std::string,StreamType> StreamMap;
    StreamMap streams;

    std::vector<connection_descriptor_struct> filterTable;
    boost::shared_ptr< ConnectionEventListener >    _connectCB;
    boost::shared_ptr< ConnectionEventListener >    _disconnectCB;

    void _connectListenerAdapter(const std::string& connectionId);
    void _disconnectListenerAdapter(const std::string& connectionId);

    //
    // Returns true if the given connection should receive SRI updates and data
    // for the given stream
    //
    bool _isStreamRoutedToConnection(const std::string& connectionID, const std::string& streamID);

    
    //
    // Sends data and metadata to all connections enabled for the given stream
    //
    friend class OutputStream<PortType>;
    void _sendPacket(const BufferType& data,
                     const BULKIO::PrecisionUTCTime& T,
                     bool EOS,
                     const std::string& streamID);

    StreamType _getStream(const std::string& streamID);
  };

  
  template <typename PortType>
  class OutNumericPort : public OutPort<PortType> {
  public:
    //
    // Data type contained in sequence container
    //
    typedef typename CorbaTraits<PortType>::TransportType TransportType;

    //
    // Data type of items passed into the pushPacket method
    //
    typedef typename NativeTraits<PortType>::NativeType NativeType;

    // 
    // Data type of the container for passing data into the pushPacket method
    //
    typedef typename BufferTraits<PortType>::VectorType VectorType;
    typedef VectorType NativeSequenceType;

    //
    // OutNumericPort Creates a uses port object for publishing data to the framework
    //
    // @param port_name name assigned to the port located in scd.xml file
    // @param connectionCB  callback that will be called when the connectPort method is called
    // @pararm disconnectDB callback that receives notification when a disconnectPort happens
    //
    OutNumericPort(const std::string& name, 
                   ConnectionEventListener *connectCB=NULL,
                   ConnectionEventListener *disconnectCB=NULL);

    OutNumericPort(const std::string& name, 
                   LOGGER_PTR logger,
                   ConnectionEventListener *connectCB=NULL,
                   ConnectionEventListener *disconnectCB=NULL);

    //
    // virtual destructor to clean up resources
    //
    virtual ~OutNumericPort();

    /*
     * pushPacket
     *     maps to data<Type> BULKIO method call for passing a limited amount of data from a source vector
     *
     *  data: pointer to a buffer of data
     *  size: number of data points in the buffer
     *  T: constant of type BULKIO::PrecisionUTCTime containing the timestamp for the outgoing data.
     *    tcmode: timecode mode
     *    tcstatus: timecode status
     *    toff: fractional sample offset
     *    twsec: J1970 GMT
     *    tfsec: fractional seconds: 0.0 to 1.0
     *  EOS: end-of-stream flag
     *  streamID: stream identifier
     */
    void pushPacket( const TransportType* data, size_t size, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    /*
     * pushPacket
     *     maps to data<Type> BULKIO method call for passing an entire vector of data
     *
     *  data: The sequence structure from an input port containing the payload to send out
     *  T: constant of type BULKIO::PrecisionUTCTime containing the timestamp for the outgoing data.
     *    tcmode: timecode mode
     *    tcstatus: timecode status
     *    toff: fractional sample offset
     *    twsec: J1970 GMT
     *    tfsec: fractional seconds: 0.0 to 1.0
     *  EOS: end-of-stream flag
     *  streamID: stream identifier
     */
    void pushPacket(const VectorType& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

  protected:
    typedef typename OutPort<PortType>::BufferType BufferType;
  };

  //
  // Character Specialization..
  //
  // This class overrides the pushPacket method to support Int8 and char data types
  //
  // Output port for Int8 and char data types
  class OutCharPort : public OutNumericPort<BULKIO::dataChar> {
  public:
    OutCharPort(const std::string& name,
		ConnectionEventListener *connectCB=NULL,
		ConnectionEventListener *disconnectCB=NULL );

    OutCharPort(const std::string& name, 
		LOGGER_PTR logger,
		ConnectionEventListener *connectCB=NULL,
		ConnectionEventListener *disconnectCB=NULL );

    // Push a vector of Int8 data
    void pushPacket(const std::vector< Int8 >& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    // Push a vector of Char data
    void pushPacket(const std::vector< Char >& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);
    
    // Push a subset of a vector of Int8 data
    void pushPacket(const Int8* buffer, size_t size, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    // Push a subset of a vector of Char data
    void pushPacket(const Char* buffer, size_t size, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);
   
  };


  class OutBitPort : public OutPort<BULKIO::dataBit> {
  public:
    typedef char* NativeSequenceType;

    OutBitPort(const std::string& name, LOGGER_PTR logger=LOGGER_PTR()); 

    void pushPacket(const redhawk::shared_bitbuffer& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);
  };

  //
  // OutFilePort
  //
  // This class defines the pushPacket interface for file URL data.
  //
  //
  class OutFilePort : public OutPort<BULKIO::dataFile> {
  public:
    typedef char* NativeSequenceType;

    OutFilePort(const std::string& name, 
                ConnectionEventListener *connectCB=NULL,
                ConnectionEventListener *disconnectCB=NULL);


    OutFilePort(const std::string& name, 
                LOGGER_PTR logger, 
                ConnectionEventListener *connectCB=NULL,
                ConnectionEventListener *disconnectCB=NULL);

    /*
     * pushPacket
     *     maps to dataFile BULKIO method call for passing the URL of a file
     *
     *  data: string containing the file URL to send out
     *  T: constant of type BULKIO::PrecisionUTCTime containing the timestamp for the outgoing data.
     *    tcmode: timecode mode
     *    tcstatus: timecode status
     *    toff: fractional sample offset
     *    twsec: J1970 GMT
     *    tfsec: fractional seconds: 0.0 to 1.0
     *  EOS: end-of-stream flag
     *  streamID: stream identifier
     */
    void pushPacket(const std::string& URL, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    /*
     * pushPacket
     *     maps to dataFile BULKIO method call for passing the URL of a file
     *
     *  data: string containing the file URL to send out
     *  T: constant of type BULKIO::PrecisionUTCTime containing the timestamp for the outgoing data.
     *    tcmode: timecode mode
     *    tcstatus: timecode status
     *    toff: fractional sample offset
     *    twsec: J1970 GMT
     *    tfsec: fractional seconds: 0.0 to 1.0
     *  EOS: end-of-stream flag
     *  streamID: stream identifier
     */
    void pushPacket(const char* URL, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    /*
     * DEPRECATED: maps to dataXML BULKIO method call for passing strings of data
     */
    void pushPacket(const char *data, bool EOS, const std::string& streamID);

  };


  //
  // OutXMLPort
  //
  // This class defines the pushPacket interface for XML data.
  //
  //
  class OutXMLPort : public OutPort<BULKIO::dataXML> {
  public:
    typedef char* NativeSequenceType;

    OutXMLPort(const std::string& name, 
               ConnectionEventListener *connectCB=NULL,
               ConnectionEventListener *disconnectCB=NULL);


    OutXMLPort(const std::string& name, 
               LOGGER_PTR logger, 
               ConnectionEventListener *connectCB=NULL,
               ConnectionEventListener *disconnectCB=NULL);

    /*
     * DEPRECATED: maps to dataFile BULKIO method call for passing strings of data 
     */
    void pushPacket(const char *data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    /*
     * pushPacket
     *     maps to dataXML BULKIO method call for passing an XML-formatted string
     *
     *  data: string containing the XML data to send out
     *  EOS: end-of-stream flag
     *  streamID: stream identifier
     */
    void pushPacket(const std::string& data, bool EOS, const std::string& streamID);

    /*
     * pushPacket
     *     maps to dataXML BULKIO method call for passing an XML-formatted string
     *
     *  data: string containing the XML data to send out
     *  EOS: end-of-stream flag
     *  streamID: stream identifier
     */
    void pushPacket(const char* data, bool EOS, const std::string& streamID);

  };


  /*
     Uses Port Definitions for All Bulk IO port definitions
     *
     */
  // Bulkio octet (UInt8) output
  typedef OutNumericPort<BULKIO::dataOctet>     OutOctetPort;
  // Bulkio UInt8 output
  typedef OutOctetPort                          OutUInt8Port;
  // Bulkio short output
  typedef OutNumericPort<BULKIO::dataShort>     OutShortPort;
  // Bulkio unsigned short output
  typedef OutNumericPort<BULKIO::dataUshort>    OutUShortPort;
  // Bulkio Int16 output
  typedef OutShortPort                          OutInt16Port;
  // Bulkio UInt16 output
  typedef OutUShortPort                         OutUInt16Port;
  // Bulkio long output
  typedef OutNumericPort<BULKIO::dataLong>      OutLongPort;
  // Bulkio unsigned long output
  typedef OutNumericPort<BULKIO::dataUlong>     OutULongPort;
  // Bulkio Int32 output
  typedef OutLongPort                           OutInt32Port;
  // Bulkio UInt32 output
  typedef OutULongPort                          OutUInt32Port;
  // Bulkio long long output
  typedef OutNumericPort<BULKIO::dataLongLong>  OutLongLongPort;
  // Bulkio unsigned long long output
  typedef OutNumericPort<BULKIO::dataUlongLong> OutULongLongPort;
  // Bulkio Int64 output
  typedef OutLongLongPort                       OutInt64Port;
  // Bulkio UInt64 output
  typedef OutULongLongPort                      OutUInt64Port;
  // Bulkio float output
  typedef OutNumericPort<BULKIO::dataFloat>     OutFloatPort;
  // Bulkio double output
  typedef OutNumericPort<BULKIO::dataDouble>    OutDoublePort;
  // Bulkio URL output
  typedef OutFilePort                           OutURLPort;
}  // end of bulkio namespace

inline bool operator>>= (const CORBA::Any& a, bulkio::connection_descriptor_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("connectionTable::connection_id", props[idx].id)) {
            if (!(props[idx].value >>= s.connection_id)) return false;
        } else if (!strcmp("connectionTable::stream_id", props[idx].id)) {
            if (!(props[idx].value >>= s.stream_id)) return false;
        } else if (!strcmp("connectionTable::port_name", props[idx].id)) {
            if (!(props[idx].value >>= s.port_name)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const bulkio::connection_descriptor_struct& s) {
    CF::Properties props;
    props.length(3);
    props[0].id = CORBA::string_dup("connectionTable::connection_id");
    props[0].value <<= s.connection_id;
    props[1].id = CORBA::string_dup("connectionTable::stream_id");
    props[1].value <<= s.stream_id;
    props[2].id = CORBA::string_dup("connectionTable::port_name");
    props[2].value <<= s.port_name;
    a <<= props;
};

inline bool operator== (const bulkio::connection_descriptor_struct& s1, const bulkio::connection_descriptor_struct& s2) {
    if (s1.connection_id!=s2.connection_id)
        return false;
    if (s1.stream_id!=s2.stream_id)
        return false;
    if (s1.port_name!=s2.port_name)
        return false;
    return true;
};

inline bool operator!= (const bulkio::connection_descriptor_struct& s1, const bulkio::connection_descriptor_struct& s2) {
    return !(s1==s2);
};

#endif
