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
/*******************************************************************************************


 *******************************************************************************************/
#include "bulkio_out_port.h"
#include "bulkio_p.h"
#include "bulkio_time_operators.h"
#include "bulkio_in_port.h"

#include "bulkio_connection.hpp"

// Suppress warnings for access to "deprecated" currentSRI member--it's the
// public access that's deprecated, not the member itself
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

namespace bulkio {
  /*
     OutPort Constructor

     Accepts connect/disconnect interfaces for notification when these events occur
  */

  template < typename PortTraits >
  OutPortBase< PortTraits >::OutPortBase(std::string port_name,
                                         LOGGER_PTR logger,
                                         ConnectionEventListener *connectCB,
                                         ConnectionEventListener *disconnectCB ) :
    Port_Uses_base_impl(port_name),
    logger(logger)
  {

    if ( connectCB ) {
      _connectCB = boost::shared_ptr< ConnectionEventListener >( connectCB, null_deleter() );
    }

    if ( disconnectCB ) {
      _disconnectCB = boost::shared_ptr< ConnectionEventListener >( disconnectCB, null_deleter() );
    }

    LOG_DEBUG( logger, "bulkio::OutPort::CTOR port:" << name );

  }


  template < typename PortTraits >
  OutPortBase< PortTraits >::OutPortBase(std::string port_name,
                                         ConnectionEventListener *connectCB,
                                         ConnectionEventListener *disconnectCB ) :
    Port_Uses_base_impl(port_name),
    logger()
  {

    if ( connectCB ) {
      _connectCB = boost::shared_ptr< ConnectionEventListener >( connectCB, null_deleter() );
    }

    if ( disconnectCB ) {
      _disconnectCB = boost::shared_ptr< ConnectionEventListener >( disconnectCB, null_deleter() );
    }

  }

  template < typename PortTraits >
  OutPortBase< PortTraits >::~OutPortBase(){

  }


  template < typename PortTraits >
  void OutPortBase< PortTraits >::pushSRI(const BULKIO::StreamSRI& H) {


    TRACE_ENTER(logger, "OutPort::pushSRI" );


    SCOPED_LOCK lock(updatingPortsLock);   // don't want to process while command information is coming in

    std::string sid( H.streamID );
    typename OutPortSriMap::iterator sri_iter;
    sri_iter=  currentSRIs.find( sid );
    if ( sri_iter == currentSRIs.end() ) {
      SriMapStruct sri_ctx( H );
      // need to use insert since we do not have default CTOR for SriMapStruct
      currentSRIs.insert( OutPortSriMap::value_type( sid, sri_ctx ) );
      sri_iter=  currentSRIs.find( sid );
    }
    else {
      // overwrite the SRI 
      sri_iter->second.sri = H;

      // reset connections list to be empty
      sri_iter->second.connections.clear();
   }

    if (active) {
      typename TransportMap::iterator port;

      for (port = _transportMap.begin(); port != _transportMap.end(); ++port) {
        if (!_isStreamRoutedToConnection(sid, port->first)) {
          continue;
        }

        LOG_DEBUG(logger,"pushSRI - PORT:" << name << " CONNECTION:" << port->first << " SRI streamID:"
                  << H.streamID << " Mode:" << H.mode << " XDELTA:" << 1.0/H.xdelta);
        try {
          port->second->pushSRI(H);
          sri_iter->second.connections.insert(port->first);
        } catch(...) {
          LOG_ERROR(logger, "PUSH-SRI FAILED, PORT/CONNECTION: " << name << "/" << port->first);
        }
      }
    }

    TRACE_EXIT(logger, "OutPort::pushSRI");
    return;
  }


  template < typename PortTraits >
  bool OutPortBase< PortTraits >::_isStreamRoutedToConnection(
          const std::string& streamID,
          const std::string& connectionID)
  {
    bool portListed = false;
    for (std::vector<connection_descriptor_struct>::iterator filter = filterTable.begin();
         filter != filterTable.end(); ++filter) {
      if (filter->port_name != name) {
        continue;
      }

      portListed = true;
      if ((filter->connection_id == connectionID) && (filter->stream_id == streamID)) {
        return true;
      }
    }
    return !portListed;
  }


  template < typename PortTraits >
  void OutPortBase< PortTraits >::_pushSinglePacket(
          PushArgumentType                data,
          const BULKIO::PrecisionUTCTime& T,
          bool                            EOS,
          const std::string&              streamID)
  {
    // don't want to process while command information is coming in
    SCOPED_LOCK lock(this->updatingPortsLock);
    _pushPacketLocked(data, T, EOS, streamID);
  }

  template < typename PortTraits >
  void OutPortBase< PortTraits >::_pushPacketLocked(
          PushArgumentType                data,
          const BULKIO::PrecisionUTCTime& T,
          bool                            EOS,
          const std::string&              streamID)
  {
      // grab SRI context 
      typename OutPortSriMap::iterator sri_iter =  currentSRIs.find( streamID );
      if (sri_iter == currentSRIs.end()) {
        // No SRI associated with the stream ID, create a default one and add
        // it to the list; it will get pushed to downstream connections below
        SriMapStruct sri_ctx(bulkio::sri::create(streamID));
        // need to use insert since we do not have default CTOR for SriMapStruct
        sri_iter = currentSRIs.insert(std::make_pair(streamID, sri_ctx)).first;
      }

      if (active) {
        typename TransportMap::iterator port;
        for (port = _transportMap.begin(); port != _transportMap.end(); ++port) {
          // Check whether filtering is enabled and if this connection should
          // receive the stream
          if (!_isStreamRoutedToConnection(streamID, port->first)) {
            continue;
          }

          if (sri_iter->second.connections.count(port->first) == 0) {
            this->_pushSRI(port, sri_iter->second);
          }

          try {
            port->second->pushPacket(data, T, EOS, sri_iter->second.sri);
          } catch(...) {
            LOG_ERROR( logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << name << "/" << port->first );
          }
        }
      }

      // if we have end of stream removed old sri
      try {
        if ( EOS ) currentSRIs.erase(streamID);
      }
      catch(...){
      }

  }


  template < typename PortTraits >
  BULKIO::UsesPortStatisticsSequence* OutPortBase< PortTraits >::statistics()
  {
    SCOPED_LOCK   lock(updatingPortsLock);
    BULKIO::UsesPortStatisticsSequence_var recStat = new BULKIO::UsesPortStatisticsSequence();
    for (typename TransportMap::iterator port = _transportMap.begin(); port != _transportMap.end(); ++port) {
      BULKIO::UsesPortStatistics stat;
      stat.connectionId = port->first.c_str();
      stat.statistics = port->second->stats.retrieve();
      ossie::corba::push_back(recStat, stat);
    }
    return recStat._retn();
  }

  template < typename PortTraits >
  BULKIO::PortUsageType OutPortBase< PortTraits >::state()
  {
    SCOPED_LOCK lock(updatingPortsLock);
    if (_transportMap.empty()) {
      return BULKIO::IDLE;
    } else {
      return BULKIO::ACTIVE;
    }
  }

  template < typename PortTraits >
  void OutPortBase< PortTraits >::enableStats(bool enable)
  {
    SCOPED_LOCK lock(updatingPortsLock);
    for (typename TransportMap::iterator port = _transportMap.begin(); port != _transportMap.end(); ++port) {
      port->second->stats.setEnabled(enable);
    }
  }


  template < typename PortTraits >
  ExtendedCF::UsesConnectionSequence * OutPortBase< PortTraits >::connections()
  {
    SCOPED_LOCK lock(updatingPortsLock);   // don't want to process while command information is coming in
    ExtendedCF::UsesConnectionSequence_var retVal = new ExtendedCF::UsesConnectionSequence();
    for (typename TransportMap::iterator port = _transportMap.begin(); port != _transportMap.end(); ++port) {
      ExtendedCF::UsesConnection conn;
      conn.connectionId = port->first.c_str();
      conn.port = port->second->objref();
      ossie::corba::push_back(retVal, conn);
    }
    return retVal._retn();
  }

  template < typename PortTraits >
  void OutPortBase< PortTraits >::connectPort(CORBA::Object_ptr connection, const char* connectionId)
  {
    TRACE_ENTER(logger, "OutPort::connectPort" );
    {
      SCOPED_LOCK lock(updatingPortsLock);   // don't want to process while command information is coming in
      PortVarType port;
      try {
        port = PortType::_narrow(connection);
        if (CORBA::is_nil(port)) {
            throw CF::Port::InvalidPort(1, "Unable to narrow");
        }
      }
      catch(...) {
        LOG_ERROR( logger, "CONNECT FAILED: UNABLE TO NARROW ENDPOINT,  USES PORT:" << name );
        throw CF::Port::InvalidPort(1, "Unable to narrow");
      }

      LocalPortType* local_port = ossie::corba::getLocalServant<LocalPortType>(port);
      if (local_port) {
        LOG_DEBUG(logger, "Using local connection to port " << local_port->getName()
                  << " for connection " << connectionId);
        _transportMap[connectionId] = _createLocalConnection(local_port, connectionId);
      } else {
        _transportMap[connectionId] = _createRemoteConnection(port, connectionId);
      }

      active = true;

      LOG_DEBUG( logger, "CONNECTION ESTABLISHED,  PORT/CONNECTION_ID:" << name << "/" << connectionId );

    }
    if (_connectCB) (*_connectCB)(connectionId);

    TRACE_EXIT(logger, "OutPort::connectPort" );
  }


  template < typename PortTraits >
  typename OutPortBase< PortTraits >::PortConnectionType*
  OutPortBase< PortTraits >::_createRemoteConnection(PortPtrType port, const std::string& connectionId)
  {
    return new RemoteConnection<PortTraits>(name, port);
  }


  template < typename PortTraits >
  typename OutPortBase< PortTraits >::PortConnectionType*
  OutPortBase< PortTraits >::_createLocalConnection(LocalPortType* port, const std::string& connectionId)
  {
    return new LocalConnection<PortTraits>(name, port);
  }
  

  template < typename PortTraits >
  void OutPortBase< PortTraits >::disconnectPort(const char* connectionId)
  {
    TRACE_ENTER(logger, "OutPort::disconnectPort" );
    {
      SCOPED_LOCK lock(updatingPortsLock);   // don't want to process while command information is coming in

      const std::string cid(connectionId);
      typename TransportMap::iterator port = _transportMap.find(connectionId);
      if (port != _transportMap.end()) {
        // Send an EOS for every connection that's listed for this SRI
        for (typename OutPortSriMap::iterator cSRIs = currentSRIs.begin(); cSRIs!=currentSRIs.end(); cSRIs++) {
          const std::string stream_id(cSRIs->second.sri.streamID);

          // Check if we have sent out sri/data to the connection
          if (cSRIs->second.connections.count( cid ) != 0) {
            if (_isStreamRoutedToConnection(stream_id, cid)) {
              try {
                port->second->sendEOS(stream_id);
              } catch (...) {
                // Ignore all exceptions; the receiver may be dead
              }
            }
          }

          // remove connection id from sri connections list
          cSRIs->second.connections.erase( cid );

        }

        LOG_DEBUG( logger, "DISCONNECT, PORT/CONNECTION: "  << name << "/" << connectionId );
        delete port->second;
        _transportMap.erase(port);

        if (_transportMap.empty()) {
          active = false;
        }
      }
    }
    if (_disconnectCB) {
      (*_disconnectCB)(connectionId);
    }

    TRACE_EXIT(logger, "OutPort::disconnectPort" );
  }

  template < typename PortTraits >
  void  OutPortBase< PortTraits >::_pushSRI(typename TransportMap::iterator connPair, SriMapStruct &sri_ctx)
  {
    TRACE_ENTER(logger, "OutPort::_pushSRI");
    // push SRI over port instance
    try {
      connPair->second->pushSRI(sri_ctx.sri);
      sri_ctx.connections.insert(connPair->first);
      LOG_TRACE(logger, "_pushSRI()  connection_id/streamID " << connPair->first << "/" << sri_ctx.sri.streamID);
    } catch(...) {
      LOG_ERROR(logger, "_pushSRI() PUSH-SRI FAILED, PORT/CONNECTION: " << name << "/" << connPair->first);
    }      
    TRACE_EXIT(logger, "OutPort::_pushSRI");
  }

  template < typename PortTraits >
  bulkio::SriMap  OutPortBase< PortTraits >::getCurrentSRI()
  {
    bulkio::SriMap ret;
    SCOPED_LOCK lock(updatingPortsLock);   // restrict access till method completes
    typename OutPortSriMap::iterator cSri = currentSRIs.begin();
    for ( ; cSri != currentSRIs.end(); cSri++ ) {
      ret[cSri->first] = std::make_pair<  BULKIO::StreamSRI, bool >( cSri->second.sri, false );
    }
    return ret;
  }

  template < typename PortTraits >
  bulkio::SriList  OutPortBase< PortTraits >::getActiveSRIs()
  {
    bulkio::SriList ret;
    SCOPED_LOCK lock(updatingPortsLock);   // restrict access till method completes
    typename OutPortSriMap::iterator cSri = currentSRIs.begin();
    for ( ; cSri != currentSRIs.end(); cSri++ ) {
      ret.push_back( cSri->second.sri );
    }
    return ret;
  }


  template < typename PortTraits >
  typename OutPortBase< PortTraits >::ConnectionsList  OutPortBase< PortTraits >::getConnections()
  {
    SCOPED_LOCK lock(updatingPortsLock);   // restrict access till method completes
    ConnectionsList outConnections;

    for (typename TransportMap::iterator port = _transportMap.begin(); port != _transportMap.end(); ++port) {
      outConnections.push_back(std::make_pair(port->second->objref(), port->first));
    }

    return outConnections;
  }


  template < typename PortTraits >
  void OutPortBase< PortTraits >::setNewConnectListener(ConnectionEventListener *newListener)
  {
    _connectCB =  boost::shared_ptr< ConnectionEventListener >(newListener, null_deleter());
  }

  template < typename PortTraits >
  void OutPortBase< PortTraits >::setNewConnectListener(ConnectionEventCallbackFn  newListener)
  {
    _connectCB =  boost::make_shared< StaticConnectionListener >( newListener );
  }

  template < typename PortTraits >
  void OutPortBase< PortTraits >::setNewDisconnectListener(ConnectionEventListener *newListener)
  {
    _disconnectCB =  boost::shared_ptr< ConnectionEventListener >(newListener, null_deleter());
  }

  template < typename PortTraits >
  void OutPortBase< PortTraits >::setNewDisconnectListener(ConnectionEventCallbackFn newListener)
  {
    _disconnectCB =  boost::make_shared< StaticConnectionListener >( newListener );
  }

  template < typename PortTraits >
  void OutPortBase< PortTraits >::setLogger(LOGGER_PTR newLogger)
  {
    logger = newLogger;
  }

  template < typename PortTraits >
  std::string   OutPortBase< PortTraits >::getRepid() const {
	return PortType::_PD_repoId;
    //return "IDL:CORBA/Object:1.0";
  }


  /*
     OutPort Constructor

     Accepts connect/disconnect interfaces for notification when these events occur
  */

  template < typename PortTraits >
  OutPort< PortTraits >::OutPort(std::string port_name,
                                 LOGGER_PTR logger,
                                 ConnectionEventListener *connectCB,
                                 ConnectionEventListener *disconnectCB ) :
    OutPortBase<PortTraits>(port_name, logger, connectCB, disconnectCB)
  {
  }


  template < typename PortTraits >
  OutPort< PortTraits >::OutPort(std::string port_name,
                                 ConnectionEventListener *connectCB,
                                 ConnectionEventListener *disconnectCB ) :
    OutPortBase<PortTraits>(port_name)
  {
  }


  template < typename PortTraits >
  OutPort< PortTraits >::~OutPort()
  {
  }


  template <typename PortTraits>
  typename OutPort<PortTraits>::PortConnectionType*
  OutPort<PortTraits>::_createRemoteConnection(PortPtrType port, const std::string& connectionId)
  {
    return new ChunkingConnection<PortTraits>(this->name, port);
  }


  template < typename PortTraits >
  void OutPort< PortTraits >::pushPacket(
          NativeSequenceType &      data,
          const BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID)
  {
    const PortSequenceType buffer(data.size(), data.size(), reinterpret_cast<TransportType*>(&data[0]), false);
    this->_pushPacketLocked(buffer, T, EOS, streamID);
  }
  
  template < typename PortTraits >
  void OutPort< PortTraits >::pushPacket(
          const DataBufferType &    data,
          const BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID)
  {
    const TransportType* ptr = reinterpret_cast<const TransportType*>(&data[0]);
    const PortSequenceType buffer(data.size(), data.size(), const_cast<TransportType*>(ptr), false);
    this->_pushPacketLocked(buffer, T, EOS, streamID);
  }

  template < typename PortTraits >
  void OutPort< PortTraits >::pushPacket(
          const TransportType*      data,
          size_t                    size,
          const BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID)
  {
    const PortSequenceType buffer(size, size, const_cast<TransportType*>(data), false);
    this->_pushPacketLocked(buffer, T, EOS, streamID);
  }

  template < typename PortTraits >
  void OutPort< PortTraits >::pushPacket(const ScalarBuffer& data,
                                         const BULKIO::PrecisionUTCTime& T,
                                         bool EOS,
                                         const std::string& streamID, 
                                         bool shared)
  {
    const TransportType* ptr = reinterpret_cast<const TransportType*>(data.data());
    const PortSequenceType buffer(data.size(), data.size(), const_cast<TransportType*>(ptr), false);
    this->_pushPacketLocked(buffer, T, EOS, streamID);
  }

  template < typename PortTraits >
  typename OutPort< PortTraits >::StreamType OutPort< PortTraits >::createStream(const std::string& streamID)
  {
    BULKIO::StreamSRI sri = bulkio::sri::create(streamID);
    return createStream(sri);
  }

  template < typename PortTraits >
  typename OutPort< PortTraits >::StreamType OutPort< PortTraits >::createStream(const BULKIO::StreamSRI& sri)
  {
    return StreamType(sri, this);
  }

  OutCharPort::OutCharPort( std::string name,
                            ConnectionEventListener *connectCB,
                            ConnectionEventListener *disconnectCB ):
    OutPort < CharPortTraits >(name,connectCB, disconnectCB)
  {

  }


  OutCharPort::OutCharPort( std::string name,
                            LOGGER_PTR logger,
                            ConnectionEventListener *connectCB,
                            ConnectionEventListener *disconnectCB ) :
    OutPort < CharPortTraits >(name, logger, connectCB, disconnectCB )
  {

  }
  
  void OutCharPort::pushPacket(const Int8* buffer, size_t size, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID)
  {
    const TransportType* data = reinterpret_cast<const TransportType*>(buffer);
    OutPort<CharPortTraits>::pushPacket(data, size, T, EOS, streamID);
  }
  
  void OutCharPort::pushPacket(const char* buffer, size_t size, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID)
  {
    const TransportType* data = reinterpret_cast<const TransportType*>(buffer);
    OutPort<CharPortTraits>::pushPacket(data, size, T, EOS, streamID);
  }
  

  void OutCharPort::pushPacket(const std::vector< Int8 >& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID)
  {
    const TransportType* buffer = reinterpret_cast<const TransportType*>(&data[0]);
    OutPort<CharPortTraits>::pushPacket(buffer, data.size(), T, EOS, streamID);
  }

  void OutCharPort::pushPacket(const std::vector< Char >& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID)
  {
    const TransportType* buffer = reinterpret_cast<const TransportType*>(&data[0]);
    OutPort<CharPortTraits>::pushPacket(buffer, data.size(), T, EOS, streamID);
  }


  OutFilePort::OutFilePort ( std::string name,
                             ConnectionEventListener *connectCB,
                             ConnectionEventListener *disconnectCB ) :
    OutPortBase < FilePortTraits >(name,connectCB, disconnectCB )
  {

  }


  OutFilePort::OutFilePort( std::string name,
                            LOGGER_PTR logger,
                            ConnectionEventListener *connectCB,
                            ConnectionEventListener *disconnectCB ) :
    OutPortBase < FilePortTraits >(name,logger,connectCB, disconnectCB )
  {

  }


  void OutFilePort::pushPacket( const char* URL, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID)
  {
    _pushSinglePacket(URL, T, EOS, streamID);
  }


  void OutFilePort::pushPacket( const std::string& URL, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID)
  {
    _pushSinglePacket(URL.c_str(), T, EOS, streamID);
  }


  void OutFilePort::pushPacket( const char *data, bool EOS, const std::string& streamID)
  {
    _pushSinglePacket(data, bulkio::time::utils::now(), EOS, streamID);
  }


  OutXMLPort::OutXMLPort ( std::string name,
                             ConnectionEventListener *connectCB,
                             ConnectionEventListener *disconnectCB ) :
    OutPortBase < XMLPortTraits >(name,connectCB, disconnectCB )
  {

  }


  OutXMLPort::OutXMLPort( std::string name,
                            LOGGER_PTR logger,
                            ConnectionEventListener *connectCB,
                            ConnectionEventListener *disconnectCB ) :
    OutPortBase < XMLPortTraits >(name,logger,connectCB, disconnectCB )
  {

  }


  void OutXMLPort::pushPacket( const char *data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID)
  {
    _pushSinglePacket(data, T, EOS, streamID);
  }


  void OutXMLPort::pushPacket( const char *data, bool EOS, const std::string& streamID)
  {
    // The time argument is never dereferenced for dataXML, so it is safe to
    // pass a null
    BULKIO::PrecisionUTCTime* time = 0;
    _pushSinglePacket(data, *time, EOS, streamID);
  }


  void OutXMLPort::pushPacket( const std::string& data, bool EOS, const std::string& streamID)
  {
    // The time argument is never dereferenced for dataXML, so it is safe to
    // pass a null
    BULKIO::PrecisionUTCTime* time = 0;
    _pushSinglePacket(data.c_str(), *time, EOS, streamID);
  }


  //
  // Required for Template Instantion for the compilation unit.
  // Note: we only define those valid types for which Bulkio IDL is defined. Users wanting to
  // inherit this functionality will be unable to since they cannot instantiate and
  // link against the template.
  //

#define INSTANTIATE_BASE_TEMPLATE(x) \
  template class OutPortBase<x>;

#define INSTANTIATE_TEMPLATE(x) \
  INSTANTIATE_BASE_TEMPLATE(x); template class OutPort<x>;

  INSTANTIATE_TEMPLATE(CharPortTraits);
  INSTANTIATE_TEMPLATE(OctetPortTraits);
  INSTANTIATE_TEMPLATE(ShortPortTraits);
  INSTANTIATE_TEMPLATE(UShortPortTraits);
  INSTANTIATE_TEMPLATE(LongPortTraits);
  INSTANTIATE_TEMPLATE(ULongPortTraits);
  INSTANTIATE_TEMPLATE(LongLongPortTraits);
  INSTANTIATE_TEMPLATE(ULongLongPortTraits);
  INSTANTIATE_TEMPLATE(FloatPortTraits);
  INSTANTIATE_TEMPLATE(DoublePortTraits);

  INSTANTIATE_BASE_TEMPLATE(FilePortTraits);
  INSTANTIATE_BASE_TEMPLATE(XMLPortTraits);

} // end of bulkio namespace
