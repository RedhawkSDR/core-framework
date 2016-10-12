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

#include "bulkio_base.h"
#include "bulkio_traits.h"
#include "bulkio_callbacks.h"
#include "bulkio_out_stream.h"

namespace bulkio {

  //
  //  OutPortBase
  //
  //  Base template for data transfers between BULKIO ports.  This class is defined by 2 trait classes
  //    PortTraits - This template provides the context for the port's middleware transport classes and they base data types
  //                 passed between port objects
  //
  //
  template < typename PortTraits >
  class OutPortBase : public Port_Uses_base_impl
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
  , public virtual POA_BULKIO::UsesPortStatisticsProvider
#endif
  {

  public:
 
    typedef PortTraits                        Traits;

    //
    // Port Variable Definition
    //
    typedef typename Traits::PortVarType      PortVarType;

    //
    // BULKIO Interface Type
    //
    typedef typename Traits::PortType         PortType;

    //
    // Port pointer type
    //
    typedef typename PortType::_ptr_type      PortPtrType;

    //
    // Sequence container used during actual pushPacket call
    //
    typedef typename Traits::SequenceType     PortSequenceType;

    //
    // True type of argument to pushPacket, typically "const PortSequenceType&"
    // except for dataXML and dataFile (which use "const char*")
    //
    typedef typename Traits::PushType         PushArgumentType;

    //
    // Data type of items passed into the pushPacket method
    //
    typedef typename Traits::NativeType       NativeType;

    //
    // ConnectionList Definition
    //
    typedef typename  bulkio::Connections< PortVarType >::List        ConnectionsList;

    //
    // Mapping of Stream IDs to SRI Map/Refresh objects
    //
    typedef std::map< std::string, SriMapStruct >                    OutPortSriMap;


    //
    // OutPortBase Creates a uses port object for publishing data to the framework
    //
    // @param port_name name assigned to the port located in scd.xml file
    // @param connectionCB  callback that will be called when the connectPort method is called
    // @pararm disconnectDB callback that receives notification when a disconnectPort happens
    //
    OutPortBase(std::string port_name, 
                ConnectionEventListener *connectCB=NULL,
                ConnectionEventListener *disconnectCB=NULL );

    OutPortBase(std::string port_name, 
                LOGGER_PTR    logger,
                ConnectionEventListener *connectCB=NULL,
                ConnectionEventListener *disconnectCB=NULL );

    
    //
    // virtual destructor to clean up resources
    //
    virtual ~OutPortBase();

    //
    //  Interface used by framework to connect/disconnect ports together and introspection of connection states
    //

    //
    // connections - Return a list of connection objects and identifiers for each connection made by connectPort
    //
    // @return ExtendedCF::UsesConnectionSequence * List of connection objects and identifiers
    //
    virtual ExtendedCF::UsesConnectionSequence * connections();

    //
    // connectPort - Called by the framework to connect this port to a Provides port object, the connection is established
    // via the association and identified by the connectionId string, no formal "type capatablity" or "bukio interface support"
    // is resolved at this time.  All data flow occurs from point A to B via the pushPacket/pushSRI interface.
    //
    // @param CORBA::Object_ptr pointer to an instance of a Provides port
    // @param connectionsId identifer for this connection, allows for external users to reference the connection association
    //
    virtual void connectPort(CORBA::Object_ptr connection, const char* connectionId);

    //
    // disconnectPort - Called by the framework to disconnect this port from the Provides port object.  The port basicall removes
    // the association to the provides port that was established with the connectionId.
    //
    // @param connectionsId identifer for this connection, allows for external users to reference the connection association
    virtual void disconnectPort(const char* connectionId);

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

    //
    // turn on/off the port monitoring capability
    //
    virtual void enableStats(bool enable);

    //
    // Return map of streamID/SRI objects 
    //
    virtual bulkio::SriMap getCurrentSRI();

    //
    // Return list of SRI objects
    //
    virtual bulkio::SriList getActiveSRIs();

    //
    // Return a ConnectionsList for the current ports and connections ids establish via connectPort method
    //
    virtual ConnectionsList getConnections();

    //
    // Deprecation Warning
    //
    // The _getConnections and currentSRIs access will be deprecated in the next release of the 
    // the bulkio library class, in favor of getCurrentSRI and getConnections.
    //

    //
    // Allow access to the port's connection list
    //
    virtual ConnectionsList __attribute__ ((deprecated)) _getConnections() {
      return outConnections;
    }

    void setLogger( LOGGER_PTR newLogger );

	std::string getRepid () const;

  protected:


    // Map of stream ids and statistic object
    typedef typename  std::map<std::string, linkStatistics  >    _StatsMap;

  public:
    //
    // List of SRIs sent out by this port
    //
    OutPortSriMap                            currentSRIs __attribute__ ((deprecated));

  protected:
    //
    // List of Port connections and connection identifiers
    //
    ConnectionsList                          outConnections;

    //
    // List of connections returned by connections() method.  Used to increase efficiency when there a large amount
    // of connections for a port.
    //
    ExtendedCF::UsesConnectionSequence       recConnections;

    //
    //
    //
    bool                                     recConnectionsRefresh;

    //
    //  Set of statistical collector objects for each stream id
    //
    _StatsMap                                 stats;

    //
    // _pushSRI - method to push given SRI to a specific connections
    //
    void _pushSRI( typename ConnectionsList::iterator connPair, SriMapStruct &sri_ctx);
    void _pushSRI( const std::string &connectionId, SriMapStruct &sri_ctx);

    LOGGER_PTR                                logger;
    std::vector<connection_descriptor_struct> filterTable;
    boost::shared_ptr< ConnectionEventListener >    _connectCB;
    boost::shared_ptr< ConnectionEventListener >    _disconnectCB;

    //
    // Returns true if the given connection should receive SRI updates and data
    // for the given stream
    //
    bool _isStreamRoutedToConnection(const std::string& connectionID, const std::string& streamID);

    //
    // Sends the given data and metadata as a single push, for subclasses that
    // will never break a push into multiple packets (XML, File); acquires and
    // releases the port lock
    //
    void _pushSinglePacket(
            PushArgumentType                data,
            const BULKIO::PrecisionUTCTime& T,
            bool                            EOS,
            const std::string&              streamID);

    //
    // Sends the given data and metadata to all appropriate connections and
    // updates the associated SRI if necessary (or creates one if it does not
    // exist); must be called with the port lock held
    //
    void _pushPacketLocked(
            PushArgumentType                data,
            const BULKIO::PrecisionUTCTime& T,
            bool                            EOS,
            const std::string&              streamID);

    //
    // Sends an end-of-stream packet for the given stream to a particular port,
    // for use when disconnecting; enables XML and File specialization for
    // consistent end-of-stream behavior
    //
    void _sendEOS(PortPtrType        port,
                  const std::string& streamID);

    //
    // Low-level push of data and metadata to the given port; enables XML and
    // File specialization for consistent high-level pushPacket behavior
    // 
    void _pushPacketToPort(
            PortPtrType                     port,
            PushArgumentType                data,
            const BULKIO::PrecisionUTCTime& T,
            bool                            EOS,
            const char*                     streamID);

    //
    // Returns the total number of elements of data in a pushPacket call, for
    // statistical tracking; enables XML and File specialization, which have
    // different notions of size
    //
    size_t _dataLength(PushArgumentType data);
  };

  
  template < typename PortTraits >
  class OutPort : public OutPortBase< PortTraits > {
  public:

    typedef PortTraits                        Traits;

    //
    // Port Variable Definition
    //
    typedef typename Traits::PortVarType      PortVarType;

    //
    // BULKIO Interface Type
    //
    typedef typename Traits::PortType         PortType;

    //
    // Sequence container used during actual pushPacket call
    //
    typedef typename Traits::SequenceType     PortSequenceType;

    //
    // Data type contained in sequence container
    //
    typedef typename Traits::TransportType    TransportType;

    //
    // Data type of items passed into the pushPacket method
    //
    typedef typename Traits::NativeType       NativeType;

    // 
    // Data type of the container for passing data into the pushPacket method
    //
    typedef std::vector< NativeType >         NativeSequenceType;

    //
    // Sequence of data returned from an input port and can be passed to the output port
    //
    typedef typename Traits::DataBufferType   DataBufferType;

    //
    // ConnectionList Definition
    //
    typedef typename  bulkio::Connections< PortVarType >::List        ConnectionsList;

    //
    // Mapping of Stream IDs to SRI Map/Refresh objects
    //
    typedef std::map< std::string, SriMapStruct >                    OutPortSriMap;

    //
    // OutputStream class
    //
    typedef OutputStream<PortTraits> StreamType;

    //
    // OutPort Creates a uses port object for publishing data to the framework
    //
    // @param port_name name assigned to the port located in scd.xml file
    // @param connectionCB  callback that will be called when the connectPort method is called
    // @pararm disconnectDB callback that receives notification when a disconnectPort happens
    //
    OutPort(std::string port_name, 
            ConnectionEventListener *connectCB=NULL,
            ConnectionEventListener *disconnectCB=NULL );

    OutPort(std::string port_name, 
            LOGGER_PTR    logger,
            ConnectionEventListener *connectCB=NULL,
            ConnectionEventListener *disconnectCB=NULL );

    //
    // virtual destructor to clean up resources
    //
    virtual ~OutPort();

    /*
     * pushPacket
     *     maps to data<Type> BULKIO method call for passing vectors of data
     *
     *  data: sequence structure containing the payload to send out
     *  T: constant of type BULKIO::PrecisionUTCTime containing the timestamp for the outgoing data.
     *    tcmode: timecode mode
     *    tcstatus: timecode status
     *    toff: fractional sample offset
     *    twsec: J1970 GMT
     *    tfsec: fractional seconds: 0.0 to 1.0
     *  EOS: end-of-stream flag
     *  streamID: stream identifier
     */
    void pushPacket( NativeSequenceType & data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);
    
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
    void pushPacket( const DataBufferType & data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    // Create a new stream based on a stream ID
    StreamType createStream(const std::string& streamID);
    // Create a new stream based on an SRI instance
    StreamType createStream(const BULKIO::StreamSRI& sri);
    using OutPortBase<PortTraits>::currentSRIs;

  protected:
    using OutPortBase<PortTraits>::logger;

    void _pushOversizedPacket(
            const TransportType*            buffer,
            size_t                          size,
            const BULKIO::PrecisionUTCTime& T,
            bool                            EOS,
            const std::string&              streamID);
  };

  //
  // Character Specialization..
  //
  // This class overrides the pushPacket method to support Int8 and char data types
  //
  // Output port for Int8 and char data types
  class OutCharPort : public OutPort < CharPortTraits > {
  public:
    OutCharPort(std::string port_name,
		ConnectionEventListener *connectCB=NULL,
		ConnectionEventListener *disconnectCB=NULL );

    OutCharPort(std::string port_name, 
		LOGGER_PTR logger,
		ConnectionEventListener *connectCB=NULL,
		ConnectionEventListener *disconnectCB=NULL );


    virtual ~OutCharPort() {};

    // Push a vector of Int8 data
    void pushPacket(const std::vector< Int8 >& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    // Push a vector of Char data
    void pushPacket(const std::vector< Char >& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);
    
    // Push a subset of a vector of Int8 data
    void pushPacket(const Int8* buffer, size_t size, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    // Push a subset of a vector of Char data
    void pushPacket(const Char* buffer, size_t size, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);
   
  };


  //
  // OutFilePort
  //
  // This class defines the pushPacket interface for file URL data.
  //
  //
  class OutFilePort : public OutPortBase < FilePortTraits > {

  public:

    typedef FilePortTraits                    Traits;

    //
    // Port Variable Definition
    //
    typedef Traits::PortVarType      PortVarType;

    //
    // BULKIO Interface Type
    //
    typedef Traits::PortType         PortType;

    //
    // Sequence container used during actual pushPacket call
    //
    typedef Traits::SequenceType     PortSequenceType;

    //
    // Data type contained in sequence container
    //
    typedef Traits::TransportType    TransportType;

    // 
    // Data type of the container for passing data into the pushPacket method
    //
    typedef char*                    NativeSequenceType;

    //
    // Data type of items passed into the pushPacket method
    //
    typedef Traits::NativeType       NativeType;


    OutFilePort( std::string pname, 
                 ConnectionEventListener *connectCB=NULL,
                 ConnectionEventListener *disconnectCB=NULL );


    OutFilePort( std::string port_name, 
                 LOGGER_PTR logger, 
                 ConnectionEventListener *connectCB=NULL,
                 ConnectionEventListener *disconnectCB=NULL );



    virtual ~OutFilePort() {};

    /*
     * pushPacket
     *     maps to dataFile BULKIO method call for passing the URL of a file
     *
     *  data: char string containing the file URL to send out
     *  T: constant of type BULKIO::PrecisionUTCTime containing the timestamp for the outgoing data.
     *    tcmode: timecode mode
     *    tcstatus: timecode status
     *    toff: fractional sample offset
     *    twsec: J1970 GMT
     *    tfsec: fractional seconds: 0.0 to 1.0
     *  EOS: end-of-stream flag
     *  streamID: stream identifier
     */
    void pushPacket(const char *URL, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);
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
  class OutXMLPort : public OutPortBase < XMLPortTraits > {

  public:

    typedef XMLPortTraits            Traits;

    typedef OutPortBase<XMLPortTraits> Base;

    //
    // Port Variable Definition
    //
    typedef Traits::PortVarType      PortVarType;

    //
    // BULKIO Interface Type
    //
    typedef Traits::PortType         PortType;

    //
    // Sequence container used during actual pushPacket call
    //
    typedef Traits::SequenceType     PortSequenceType;

    //
    // Data type contained in sequence container
    //
    typedef Traits::TransportType    TransportType;

    // 
    // Data type of the container for passing data into the pushPacket method
    //
    typedef char*                    NativeSequenceType;

    //
    // Data type of items passed into the pushPacket method
    //
    typedef Traits::NativeType       NativeType;


    OutXMLPort( std::string pname, 
		ConnectionEventListener *connectCB=NULL,
		ConnectionEventListener *disconnectCB=NULL );


    OutXMLPort( std::string port_name, 
                LOGGER_PTR logger, 
                ConnectionEventListener *connectCB=NULL,
                ConnectionEventListener *disconnectCB=NULL );



    virtual ~OutXMLPort() {};

    /*
     * DEPRECATED: maps to dataFile BULKIO method call for passing strings of data 
     */
    void pushPacket(const char *data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    /*
     * pushPacket
     *     maps to dataXML BULKIO method call for passing an XML-formatted string
     *
     *  data: character string containing the XML data to send out
     *  EOS: end-of-stream flag
     *  streamID: stream identifier
     */
    void pushPacket(const char *data, bool EOS, const std::string& streamID);

    /*
     * pushPacket
     *     maps to dataXML BULKIO method call for passing an XML-formatted string
     *
     *  data: string containing the XML data to send out
     *  EOS: end-of-stream flag
     *  streamID: stream identifier
     */
    void pushPacket(const std::string& data, bool EOS, const std::string& streamID);

  };


  /*
     Uses Port Definitions for All Bulk IO port definitions
     *
     */
  // Bulkio octet (UInt8) output
  typedef OutPort< OctetPortTraits >         OutOctetPort;
  // Bulkio UInt8 output
  typedef OutOctetPort                       OutUInt8Port;
  // Bulkio short output
  typedef OutPort<  ShortPortTraits >        OutShortPort;
  // Bulkio unsigned short output
  typedef OutPort<  UShortPortTraits >       OutUShortPort;
  // Bulkio Int16 output
  typedef OutShortPort                       OutInt16Port;
  // Bulkio UInt16 output
  typedef OutUShortPort                      OutUInt16Port;
  // Bulkio long output
  typedef OutPort<  LongPortTraits >         OutLongPort;
  // Bulkio unsigned long output
  typedef OutPort< ULongPortTraits >         OutULongPort;
  // Bulkio Int32 output
  typedef OutLongPort                        OutInt32Port;
  // Bulkio UInt32 output
  typedef OutULongPort                       OutUInt32Port;
  // Bulkio long long output
  typedef OutPort<  LongLongPortTraits >     OutLongLongPort;
  // Bulkio unsigned long long output
  typedef OutPort<  ULongLongPortTraits >    OutULongLongPort;
  // Bulkio Int64 output
  typedef OutLongLongPort                    OutInt64Port;
  // Bulkio UInt64 output
  typedef OutULongLongPort                   OutUInt64Port;
  // Bulkio float output
  typedef OutPort<  FloatPortTraits >        OutFloatPort;
  // Bulkio double output
  typedef OutPort<  DoublePortTraits >       OutDoublePort;
  // Bulkio URL output
  typedef OutFilePort                        OutURLPort;

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
