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

// Suppress warnings for access to "deprecated" currentSRI member--it's the
// public access that's deprecated, not the member itself
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

namespace  bulkio {

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


    recConnectionsRefresh = false;
    recConnections.length(0);

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

    recConnectionsRefresh = false;
    recConnections.length(0);

  }

  template < typename PortTraits >
  OutPortBase< PortTraits >::~OutPortBase(){

  }


  template < typename PortTraits >
  void OutPortBase< PortTraits >::pushSRI(const BULKIO::StreamSRI& H) {


    TRACE_ENTER(logger, "OutPort::pushSRI" );


    typename ConnectionsList::iterator i;

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
      for (i = outConnections.begin(); i != outConnections.end(); ++i) {
        if (!_isStreamRoutedToConnection(sid, i->second)) {
          continue;
        }

        LOG_DEBUG(logger,"pushSRI - PORT:" << name << " CONNECTION:" << i->second << " SRI streamID:" << H.streamID << " Mode:" << H.mode << " XDELTA:" << 1.0/H.xdelta );  
        try {
          i->first->pushSRI(H);
          sri_iter->second.connections.insert( i->second );
        } catch(...) {
          LOG_ERROR( logger, "PUSH-SRI FAILED, PORT/CONNECTION: " << name << "/" << i->second );
        }
      }
    }

    TRACE_EXIT(logger, "OutPort::pushSRI" );
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
  void OutPortBase< PortTraits >::_pushPacketToPort(
          PortPtrType                     port,
          PushArgumentType                data,
          const BULKIO::PrecisionUTCTime& T,
          bool                            EOS,
          const char*                     streamID)
  {
    port->pushPacket(data, T, EOS, streamID);
  }

  template < typename PortTraits >
  void OutPortBase< PortTraits >::_sendEOS(
          PortPtrType        port,
          const std::string& streamID)
  {
    port->pushPacket(PortSequenceType(), bulkio::time::utils::notSet(), true, streamID.c_str());
  }


  template < typename PortTraits >
  size_t OutPortBase< PortTraits >::_dataLength(PushArgumentType data)
  {
    return data.length();
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

      const size_t length = _dataLength(data);

      if (active) {
        typename  ConnectionsList::iterator port;
        for (port = outConnections.begin(); port != outConnections.end(); port++) {
          // Check whether filtering is enabled and if this connection should
          // receive the stream
          if (!_isStreamRoutedToConnection(streamID, port->second)) {
            continue;
          }

          if ( sri_iter != currentSRIs.end() && sri_iter->second.connections.count( port->second ) == 0 ) {
            this->_pushSRI( port, sri_iter->second );
          }

          try {
            _pushPacketToPort(port->first, data, T, EOS, streamID.c_str());
            if ( stats.count(port->second) == 0 ) {
              stats.insert( std::make_pair(port->second, linkStatistics( name, sizeof(NativeType) ) ) );
            }
            stats[port->second].update(length, 0, EOS, streamID);
          } catch(...) {
            LOG_ERROR( logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << name << "/" << port->second );
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
  BULKIO::UsesPortStatisticsSequence *  OutPortBase< PortTraits >::statistics()
  {
    SCOPED_LOCK   lock(updatingPortsLock);
    BULKIO::UsesPortStatisticsSequence_var recStat = new BULKIO::UsesPortStatisticsSequence();
    recStat->length(outConnections.size());
    for (unsigned int i = 0; i < outConnections.size(); i++) {
      recStat[i].connectionId = CORBA::string_dup(outConnections[i].second.c_str());
      recStat[i].statistics = stats[outConnections[i].second].retrieve();
    }
    return recStat._retn();
  }

  template < typename PortTraits >
  BULKIO::PortUsageType OutPortBase< PortTraits >::state()
  {
    SCOPED_LOCK lock(updatingPortsLock);
    if (outConnections.size() > 0) {
      return BULKIO::ACTIVE;
    } else {
      return BULKIO::IDLE;
    }

    return BULKIO::BUSY;
  }

  template < typename PortTraits >
  void OutPortBase< PortTraits >::enableStats(bool enable)
  {
    for (unsigned int i = 0; i < outConnections.size(); i++) {
      stats[outConnections[i].second].setEnabled(enable);
    }
  }


  template < typename PortTraits >
  ExtendedCF::UsesConnectionSequence * OutPortBase< PortTraits >::connections()
  {
    SCOPED_LOCK lock(updatingPortsLock);   // don't want to process while command information is coming in
    if (recConnectionsRefresh) {
      recConnections.length(outConnections.size());
      for (unsigned int i = 0; i < outConnections.size(); i++) {
        recConnections[i].connectionId = CORBA::string_dup(outConnections[i].second.c_str());
        recConnections[i].port = CORBA::Object::_duplicate(outConnections[i].first);
      }
      recConnectionsRefresh = false;
    }
    ExtendedCF::UsesConnectionSequence_var retVal = new ExtendedCF::UsesConnectionSequence(recConnections);
    // NOTE: You must delete the object that this function returns!
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
      outConnections.push_back(std::make_pair(port, connectionId));
      active = true;
      recConnectionsRefresh = true;

      LOG_DEBUG( logger, "CONNECTION ESTABLISHED,  PORT/CONNECTION_ID:" << name << "/" << connectionId );

    }
    if (_connectCB) (*_connectCB)(connectionId);

    TRACE_EXIT(logger, "OutPort::connectPort" );
  }


  template < typename PortTraits >
  void OutPortBase< PortTraits >::disconnectPort(const char* connectionId)
  {
    TRACE_ENTER(logger, "OutPort::disconnectPort" );
    {
      SCOPED_LOCK lock(updatingPortsLock);   // don't want to process while command information is coming in

      const std::string cid(connectionId);
      for (typename ConnectionsList::iterator ii = outConnections.begin(); ii != outConnections.end(); ++ii) {
        if (ii->second != connectionId) {
          continue;
        }

        typename OutPortSriMap::iterator cSRIs = currentSRIs.begin();

        // send an EOS for every connection that's listed for this SRI
        for (; cSRIs!=currentSRIs.end(); cSRIs++) {
          std::string cSriSid(cSRIs->second.sri.streamID);

          // Check if we have sent out sri/data to the connection
          if ( cSRIs->second.connections.count( cid ) != 0 ) {
            if (_isStreamRoutedToConnection(cSriSid, cid)) {
              try {
                _sendEOS(ii->first, cSriSid);
              } catch (...) {
                // Ignore all exceptions; the receiver may be dead
              }
            }
          }

          // remove connection id from sri connections list
          cSRIs->second.connections.erase( cid );

        }
        LOG_DEBUG( logger, "DISCONNECT, PORT/CONNECTION: "  << name << "/" << connectionId );
        stats.erase(ii->second);
        outConnections.erase(ii);
        break;
      }
    
      if (outConnections.size() == 0) {
        active = false;
      }
      recConnectionsRefresh = true;
    }
    if (_disconnectCB) (*_disconnectCB)(connectionId);

    TRACE_EXIT(logger, "OutPort::disconnectPort" );
  }

  template < typename PortTraits >
  void  OutPortBase< PortTraits >::_pushSRI( typename ConnectionsList::iterator connPair, SriMapStruct &sri_ctx)
  {
    TRACE_ENTER(logger, "OutPort::_pushSRI" );

    // assume parent will lock us...
    if ( connPair != outConnections.end() ) {

      // push SRI over port instance
      try {
	connPair->first->pushSRI(sri_ctx.sri);
	sri_ctx.connections.insert( connPair->second );
	LOG_TRACE( logger, "_pushSRI()  connection_id/streamID " << connPair->second << "/" << sri_ctx.sri.streamID );
      } catch(...) {
	LOG_ERROR( logger, "_pushSRI() PUSH-SRI FAILED, PORT/CONNECTION: " << name << "/" << connPair->second );
      }      
    }

    TRACE_EXIT(logger, "OutPort::_pushSRI" );
    return;
  }


  template < typename PortTraits >
  void  OutPortBase< PortTraits >::_pushSRI( const std::string &connectionId, SriMapStruct &sri_ctx)
  {
    TRACE_ENTER(logger, "OutPort::_pushSRI" );

    typename ConnectionsList::iterator i;

    for ( i=outConnections.begin(); i != outConnections.end(); i++ ) {
      if ( i->second == connectionId ) {
	this->_pushSRI( i, sri_ctx );
	break;
      }
    }
    TRACE_EXIT(logger, "OutPort::_pushSRI" );
    return;
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
   * Specializations of base class methods for dataXML ports
   */

  template <>
  void OutPortBase< XMLPortTraits >::_pushPacketToPort(
          BULKIO::dataXML_ptr             port,
          const char*                     data,
          const BULKIO::PrecisionUTCTime& /*unused*/,
          bool                            EOS,
          const char*                     streamID)
  {
    port->pushPacket(data, EOS, streamID);
  }

  
  template <>
  void OutPortBase< XMLPortTraits >::_sendEOS(
          BULKIO::dataXML_ptr port,
          const std::string&  streamID)
  {
    port->pushPacket("", true, streamID.c_str());
  }

  
  template <>
  size_t OutPortBase< XMLPortTraits >::_dataLength(const char* data)
  {
    if (!data) {
      return 0;
    }
    return strlen(data);
  }


  /*
   * Specializations of base class methods for dataFile ports
   */

  template <>
  void OutPortBase< FilePortTraits >::_sendEOS(
          BULKIO::dataFile_ptr port,
          const std::string&   streamID)
  {
    port->pushPacket("", bulkio::time::utils::notSet(), true, streamID.c_str());
  }

 
  template <>
  size_t OutPortBase< FilePortTraits >::_dataLength(const char* /*unused*/)
  {
    return 1;
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


  /*
   * Push a packet whose payload cannot fit within the CORBA limit.
   * The packet is broken down into sub-packets and sent via multiple pushPacket
   * calls.  The EOS is set to false for all of the sub-packets, except for
   * the last sub-packet, who uses the input EOS argument.
   */
  template < typename PortTraits >
  void OutPort< PortTraits>::_pushOversizedPacket(
          const TransportType*      buffer,
          size_t                    size,
          const BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID)
  {
      // don't want to process while command information is coming in
      SCOPED_LOCK lock(this->updatingPortsLock);

      // Multiply by some number < 1 to leave some margin for the CORBA header
      const size_t maxPayloadSize    = (size_t) (bulkio::Const::MaxTransferBytes() * .9);

      size_t maxSamplesPerPush = maxPayloadSize/sizeof(TransportType);
      typename OutPortSriMap::iterator sri_iter;
      sri_iter = currentSRIs.find( streamID );
      // Determine xdelta for this streamID to be used for time increment for subpackets
      double xdelta = 0.0;
      size_t itemSize = 1;
      if ( sri_iter != currentSRIs.end() ) {
          xdelta = sri_iter->second.sri.xdelta;
          itemSize = sri_iter->second.sri.mode?2:1;
      }

      if ( sri_iter != currentSRIs.end() ) {
        if (sri_iter->second.sri.subsize == 0) {
            // make sure maxSamplesPerPush is even so that complex data case is handled properly
            if (maxSamplesPerPush%2 != 0){
              maxSamplesPerPush--;
            }
          } else { // this is framed data, so it must be consistent with both subsize and complex
            while (maxSamplesPerPush%sri_iter->second.sri.subsize != 0) {
                maxSamplesPerPush -= maxSamplesPerPush%(sri_iter->second.sri.subsize);
                if (maxSamplesPerPush%2 != 0){
                    maxSamplesPerPush--;
                }
            }
        }
      } else {
        if (maxSamplesPerPush%2 != 0){
            maxSamplesPerPush--;
        }
      }

      // Always do at least one push (may be empty), ensuring that all samples
      // are pushed
      size_t samplesRemaining = size;

      // Initialize time of first subpacket
      BULKIO::PrecisionUTCTime packetTime = T;
      
      do {
          // Don't send more samples than are remaining
          const size_t pushSize = std::min(samplesRemaining, maxSamplesPerPush);
          samplesRemaining -= pushSize;

          // Send end-of-stream as false for all sub-packets except for the
          // last one (when there are no samples remaining after this push),
          // which gets the input EOS.
          bool packetEOS = false;
          if (samplesRemaining == 0) {
              packetEOS = EOS;
          }

          // Wrap a non-owning CORBA sequence (last argument is whether to free
          // the buffer on destruction) around this sub-packet's data
          const PortSequenceType subPacket(pushSize, pushSize, const_cast<TransportType*>(buffer), false);
          LOG_TRACE(logger,"_pushOversizedPacket calling pushPacket with pushSize " << pushSize << " and packetTime twsec: " << packetTime.twsec << " tfsec: " << packetTime.tfsec)
          this->_pushPacketLocked(subPacket, packetTime, packetEOS, streamID);

          // Synthesize the next packet timestamp
          if (packetTime.tcstatus == BULKIO::TCS_VALID) {
              packetTime += (pushSize/itemSize)* xdelta;
          }

          // Advance buffer to next sub-packet boundary
          buffer += pushSize;
      } while (samplesRemaining > 0);
  }


  template < typename PortTraits >
  void OutPort< PortTraits >::pushPacket(
          NativeSequenceType &      data,
          const BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID)
  {
    // Use const alias to start of buffer and defer to pointer-based push
    const TransportType* buffer = reinterpret_cast<const TransportType*>(&data[0]);
    const size_t size = data.size();
    pushPacket(buffer, size, T, EOS, streamID);
  }
  
  template < typename PortTraits >
  void OutPort< PortTraits >::pushPacket(
          const DataBufferType &    data,
          const BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID)
  {
    // Use const alias to start of buffer and defer to pointer-based push
    const TransportType* buffer = reinterpret_cast<const TransportType*>(&data[0]);
    const size_t size = data.size();
    pushPacket(buffer, size, T, EOS, streamID);
  }

  template < typename PortTraits >
  void OutPort< PortTraits >::pushPacket(
          const TransportType*      data,
          size_t                    size,
          const BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID) {

    TRACE_ENTER(logger, "OutPort::pushPacket" );

    _pushOversizedPacket(data, size, T, EOS, streamID);

    TRACE_EXIT(logger, "OutPort::pushPacket" );
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
