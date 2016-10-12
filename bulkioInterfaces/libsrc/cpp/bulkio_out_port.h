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
#include <set>
#include <vector>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>

#include "bulkio_base.h"
#include "bulkio_traits.h"
#include "ossie/CorbaUtils.h"

namespace bulkio {

    struct connection_descriptor_struct {
        connection_descriptor_struct ()
        {
        };

        static std::string getId() {
            return std::string("connection_descriptor");
        };

        std::string connection_id;
        std::string stream_id;
        std::string port_name;
    };

  //
  // Callback interface used by BULKIO Ports when connect/disconnect event happens
  //
  typedef void   (*ConnectionEventCallbackFn)( const char *connectionId );

  //
  // Interface definition that will be notified when a connect/disconnect event happens
  //
  class ConnectionEventListener {

    public:
      virtual void operator() ( const char *connectionId ) = 0;
      virtual ~ConnectionEventListener() {};

  };

    /**
     * Allow for member functions to receive connect/disconnect notifications
     */
    template <class T>
    class MemberConnectionEventListener : public ConnectionEventListener
    {
    public:
      typedef boost::shared_ptr< MemberConnectionEventListener< T > > SPtr;
      
      typedef void (T::*MemberFn)( const char *connectionId );

      static SPtr Create( T &target, MemberFn func ){
	return SPtr( new MemberConnectionEventListener(target, func ) );
      };

      virtual void operator() ( const char *connectionId )
      {
	(target_.*func_)(connectionId);
      }

      // Only allow PropertySet_impl to instantiate this class.
      MemberConnectionEventListener ( T& target,  MemberFn func) :
      target_(target),
	func_(func)
        {
        }
    private:
      T& target_;
      MemberFn func_;
    };

  /**
   * Wrap Callback functions as ConnectionEventListener objects
   */
  class StaticConnectionListener : public ConnectionEventListener
  {
    public:
    virtual void operator() ( const char *connectionId )
        {
            (*func_)(connectionId);
        }

    StaticConnectionListener ( ConnectionEventCallbackFn func) :
      func_(func)
        {
        }

  private:

    ConnectionEventCallbackFn func_;
  };


  struct SriMapStruct {
      BULKIO::StreamSRI        sri;
      std::set<std::string>    connections;

    SriMapStruct( const BULKIO::StreamSRI &in_sri ) {
      sri = in_sri;
    };

    SriMapStruct( const SriMapStruct &src ) {
      sri = src.sri;
      connections = src.connections;
    };
  };



  //
  //  OutPort
  //
  //  Base template for data transfers between BULKIO ports.  This class is defined by 2 trait classes
  //    PortTraits - This template provides the context for the port's middleware transport classes and they base data types
  //                 passed between port objects
  //
  //
  template < typename PortTraits >
    class OutPort : public Port_Uses_base_impl, public virtual POA_BULKIO::UsesPortStatisticsProvider
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

    typedef typename Traits::PortTraits       UsesPortType;

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



  public:

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
    virtual void   setNewConnectListener( ConnectionEventListener *newListener );
    virtual void   setNewConnectListener( ConnectionEventCallbackFn  newListener );
    virtual void   setNewDisconnectListener( ConnectionEventListener *newListener );
    virtual void   setNewDisconnectListener( ConnectionEventCallbackFn  newListener );


    //
    // pushSRI - called by the source component when SRI data about the stream changes, the data flow policy is this activity
    //           will occurr first before any data flows to the component.
    //
    // @param H - Incoming StreamSRI object that defines the state of the data flow portion of the stream (pushPacket)
    //
    virtual void pushSRI(const BULKIO::StreamSRI& H);

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
    void pushPacket( NativeSequenceType & data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);
    
    /*
     * pushPacket
     *     maps to data<Type> BULKIO method call for passing vectors of data
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
    void pushPacket( TransportType* data, size_t size, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    /*
     * pushPacket
     *     maps to data<Type> BULKIO method call for passing vectors of data
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
    void pushPacket( const DataBufferType & data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

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

  public:
    virtual void   setLogger( LOGGER_PTR newLogger );

  protected:

    void _pushOversizedPacket(
            const DataBufferType &      data,
            BULKIO::PrecisionUTCTime&   T,
            bool                        EOS,
            const std::string&          streamID);
    void _pushOversizedPacket(
            const TransportType*        buffer,
            size_t                      size,
            BULKIO::PrecisionUTCTime&   T,
            bool                        EOS,
            const std::string&          streamID);
    void _pushPacket(
            const PortSequenceType &      data,
            BULKIO::PrecisionUTCTime&   T,
            bool                        EOS,
            const std::string&          streamID);

  };


  //
  // Character Specialization..
  //
  // This class overrides the pushPacket method to use the Int8 parameter and also overriding the PortSequence constructor to
  // use the CORBA::Char type.
  //
  // For some reason, you cannot specialize a method of a template and have the template be inherited which caused major
  // issues during compilation.  Every member variable was being reported as unknown and the _Connections type was being
  //  lost.
  //
  template < typename PortTraits >
    class OutInt8Port: public OutPort<  PortTraits >  {

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

    typedef typename Traits::PortTraits       UsesPortType;

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

    OutInt8Port(std::string port_name,
		ConnectionEventListener *connectCB=NULL,
		ConnectionEventListener *disconnectCB=NULL );

    OutInt8Port(std::string port_name, 
		LOGGER_PTR logger,
		ConnectionEventListener *connectCB=NULL,
		ConnectionEventListener *disconnectCB=NULL );


    virtual ~OutInt8Port() {};

    void pushPacket( std::vector< Int8 >& data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    void pushPacket( std::vector< Char >& data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);
    
    void pushPacket( Int8* buffer, size_t size, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    void pushPacket( Char* buffer, size_t size, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);
    
  protected:
    void _pushPacket( PortTypes::CharSequence& seq, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

  };



  //
  // OutStringPort
  //
  // This class defines the pushPacket interface for string of data. This template is use by 
  // both the File and XML port classes for pushing data downstream.  
  //
  //
  template < typename PortTraits >
    class OutStringPort : public OutPort < PortTraits > {

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

    typedef typename Traits::PortTraits       UsesPortType;

    //
    // Sequence container used during actual pushPacket call
    //
    typedef typename Traits::SequenceType     PortSequenceType;

    //
    // Data type contained in sequence container
    //
    typedef typename Traits::TransportType    TransportType;

    // 
    // Data type of the container for passing data into the pushPacket method
    //
    typedef char*                             NativeSequenceType;

    //
    // Data type of items passed into the pushPacket method
    //
    typedef typename Traits::NativeType       NativeType;


    OutStringPort( std::string pname, 
		ConnectionEventListener *connectCB=NULL,
		ConnectionEventListener *disconnectCB=NULL );


    OutStringPort(std::string port_name, 
		  LOGGER_PTR logger, 
		  ConnectionEventListener *connectCB=NULL,
		  ConnectionEventListener *disconnectCB=NULL );



    virtual ~OutStringPort() {};


    virtual void disconnectPort(const char* connectionId);

    /*
     * pushPacket
     *     maps to dataFile BULKIO method call for passing strings of data 
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
    virtual void  pushPacket(const char *data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID);

    /*
     * pushPacket
     *     maps to dataXML BULKIO method call for passing strings of data
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
    virtual void  pushPacket(const char *data, bool EOS, const std::string& streamID);

  };


  /**
     Uses Port Definitions for All Bulk IO port definitions
     *
     */
  typedef OutInt8Port<  CharPortTraits >     OutCharPort;
  typedef OutPort< OctetPortTraits >         OutOctetPort;
  typedef OutOctetPort                       OutUInt8Port;
  typedef OutPort<  ShortPortTraits >        OutShortPort;
  typedef OutPort<  UShortPortTraits >       OutUShortPort;
  typedef OutShortPort                       OutInt16Port;
  typedef OutUShortPort                      OutUInt16Port;
  typedef OutPort<  LongPortTraits >         OutLongPort;
  typedef OutPort< ULongPortTraits >         OutULongPort;
  typedef OutLongPort                        OutInt32Port;
  typedef OutULongPort                       OutUInt32Port;
  typedef OutPort<  LongLongPortTraits >     OutLongLongPort;
  typedef OutPort<  ULongLongPortTraits >    OutULongLongPort;
  typedef OutLongLongPort                    OutInt64Port;
  typedef OutULongLongPort                   OutUInt64Port;
  typedef OutPort<  FloatPortTraits >        OutFloatPort;
  typedef OutPort<  DoublePortTraits >       OutDoublePort;
  typedef OutStringPort< URLPortTraits >     OutURLPort;
  typedef OutStringPort< FilePortTraits >    OutFilePort;
  typedef OutStringPort< XMLPortTraits >     OutXMLPort;


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
