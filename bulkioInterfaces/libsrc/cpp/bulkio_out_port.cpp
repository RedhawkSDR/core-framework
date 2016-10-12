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
#include <bulkio_p.h>
#include <uuid/uuid.h>

// Suppress warnings for access to "deprecated" currentSRI member--it's the
// public access that's deprecated, not the member itself
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

namespace  bulkio {


  /**
     OutPort Constructor

     Accepts connect/disconnect interfaces for notification when these events occur
  */

  template < typename PortTraits >
  OutPort< PortTraits >::OutPort(std::string port_name,
                                 LOGGER_PTR logger,
                                 ConnectionEventListener *connectCB,
                                 ConnectionEventListener *disconnectCB ) :
    Port_Uses_base_impl(port_name),
    logger(logger),
    _connectCB(),
    _disconnectCB()
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
  OutPort< PortTraits >::OutPort(std::string port_name,
                                 ConnectionEventListener *connectCB,
                                 ConnectionEventListener *disconnectCB ) :
    Port_Uses_base_impl(port_name),
    _connectCB(),
    _disconnectCB()
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
  OutPort< PortTraits >::~OutPort(){

  }


  template < typename PortTraits >
  void   OutPort< PortTraits >::setNewConnectListener( ConnectionEventListener *newListener ) {
    _connectCB =  boost::shared_ptr< ConnectionEventListener >(newListener, null_deleter());
  }

  template < typename PortTraits >
  void   OutPort< PortTraits >::setNewConnectListener( ConnectionEventCallbackFn  newListener ) {
    _connectCB =  boost::make_shared< StaticConnectionListener >( newListener );
  }

  template < typename PortTraits >
  void   OutPort< PortTraits >::setNewDisconnectListener( ConnectionEventListener *newListener ) {
    _disconnectCB =  boost::shared_ptr< ConnectionEventListener >(newListener, null_deleter());
  }

  template < typename PortTraits >
  void   OutPort< PortTraits >::setNewDisconnectListener( ConnectionEventCallbackFn  newListener ) {
    _disconnectCB =  boost::make_shared< StaticConnectionListener >( newListener );
  }



  template < typename PortTraits >
  void OutPort< PortTraits >::pushSRI(const BULKIO::StreamSRI& H) {


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
      bool portListed = false;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (i = outConnections.begin(); i != outConnections.end(); ++i) {
        for (ftPtr=filterTable.begin(); ftPtr!= filterTable.end(); ftPtr++) {

          if (ftPtr->port_name == this->name) portListed=true;

          if ( (ftPtr->port_name == this->name) and 
	       (ftPtr->connection_id == i->second) and 
	       (strcmp(ftPtr->stream_id.c_str(),H.streamID) == 0 ) ){
	    LOG_DEBUG(logger,"pushSRI - PORT:" << name << " CONNECTION:" << ftPtr->connection_id << " SRI streamID:" << H.streamID << " Mode:" << H.mode << " XDELTA:" << 1.0/H.xdelta );  
            try {
              i->first->pushSRI(H);
	      sri_iter->second.connections.insert( i->second );
            } catch(...) {
              LOG_ERROR( logger, "PUSH-SRI FAILED, PORT/CONNECTION: " << name << "/" << i->second );
            }
          }
        }
      }

      if (!portListed) {
	for (i = outConnections.begin(); i != outConnections.end(); ++i) {
          std::string connectionId = CORBA::string_dup(i->second.c_str());
	  LOG_DEBUG(logger,"pushSRI -2- PORT:" << name << " CONNECTION:" << connectionId << " SRI streamID:" << H.streamID << " Mode:" << H.mode << " XDELTA:" << 1.0/H.xdelta );  
	  try {
	    i->first->pushSRI(H);
	    sri_iter->second.connections.insert( i->second );
	  } catch(...) {
	    LOG_ERROR( logger, "PUSH-SRI FAILED, PORT/CONNECTION: " << name << "/" << i->second );
	  }
	}
      }
    }

    TRACE_EXIT(logger, "OutPort::pushSRI" );
    return;
  }

  /**
   * Push a packet whose payload cannot fit within the CORBA limit.
   * The packet is broken down into sub-packets and sent via multiple pushPacket
   * calls.  The EOS is set to false for all of the sub-packets, except for
   * the last sub-packet, who uses the input EOS argument.
   */
  template < typename PortTraits >
  void OutPort< PortTraits>::_pushOversizedPacket(
          const DataBufferType &    data,
          BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID)
  {
      // Use const alias to point to the start of the current sub-packet's data
      const TransportType* buffer = static_cast<const TransportType*>(&data[0]);
      size_t size = data.size();
      _pushOversizedPacket(buffer, size, T, EOS, streamID);
  }
  
  /**
   * Push a packet whose payload cannot fit within the CORBA limit.
   * The packet is broken down into sub-packets and sent via multiple pushPacket
   * calls.  The EOS is set to false for all of the sub-packets, except for
   * the last sub-packet, who uses the input EOS argument.
   */
  template < typename PortTraits >
  void OutPort< PortTraits>::_pushOversizedPacket(
          const TransportType*      buffer,
          size_t                    size,
          BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID)
  {
      // Multiply by some number < 1 to leave some margin for the CORBA header
      const size_t maxPayloadSize    = (size_t) (bulkio::Const::MaxTransferBytes() * .9);

      size_t maxSamplesPerPush = maxPayloadSize/sizeof(TransportType);
      // make sure maxSamplesPerPush is even so that complex data case is handled properly
      if (maxSamplesPerPush%2 != 0){
          maxSamplesPerPush--;
      }
      
      // Determine xdelta for this streamID to be used for time increment for subpackets
      typename OutPortSriMap::iterator sri_iter;
      sri_iter=  currentSRIs.find( streamID );
      double xdelta = 0.0;
      if ( sri_iter != currentSRIs.end() ) {
          xdelta = sri_iter->second.sri.xdelta;
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
          _pushPacket(subPacket, packetTime, packetEOS, streamID );
          packetTime = bulkio::time::utils::addSampleOffset(packetTime, pushSize, xdelta);

          // Advance buffer to next sub-packet boundary
          buffer += pushSize;
      } while (samplesRemaining > 0);
  }


  template < typename PortTraits >
  void OutPort< PortTraits >::pushPacket(
          NativeSequenceType &      data,
          BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID) {

    TRACE_ENTER(logger, "OutPort::pushPacket" );


    // check if sri has been pushed... 
    typename OutPortSriMap::iterator sri_iter =  currentSRIs.find( streamID );
    if ( sri_iter == currentSRIs.end() ) {
      BULKIO::StreamSRI H = bulkio::sri::create( streamID );
      pushSRI( H );
    }

    // don't want to process while command information is coming in
    SCOPED_LOCK lock(updatingPortsLock);
    _pushOversizedPacket(data, T, EOS, streamID);

    TRACE_EXIT(logger, "OutPort::pushPacket" );
  }
  
  template < typename PortTraits >
  void OutPort< PortTraits >::pushPacket(
          TransportType*            data,
          size_t                    size,
          BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID) {

    TRACE_ENTER(logger, "OutPort::pushPacket" );


    // check if sri has been pushed... 
    typename OutPortSriMap::iterator sri_iter =  currentSRIs.find( streamID );
    if ( sri_iter == currentSRIs.end() ) {
      BULKIO::StreamSRI H = bulkio::sri::create( streamID );
      pushSRI( H );
    }

    // don't want to process while command information is coming in
    SCOPED_LOCK lock(updatingPortsLock);
    _pushOversizedPacket(data, size, T, EOS, streamID);

    TRACE_EXIT(logger, "OutPort::pushPacket" );
  }

  template < typename PortTraits >
  void OutPort< PortTraits >::_pushPacket(
          const PortSequenceType &    data,
          BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID) {
      typename  ConnectionsList::iterator port;

      // grab SRI context 
      typename OutPortSriMap::iterator sri_iter =  currentSRIs.find( streamID );

      if (active) {
	bool portListed = false;
        std::vector<connection_descriptor_struct >::iterator ftPtr;
        for (port = outConnections.begin(); port != outConnections.end(); port++) {
          for (ftPtr=filterTable.begin(); ftPtr != filterTable.end(); ftPtr++) {

	    if  (ftPtr->port_name == this->name) portListed = true;

	    if ( (ftPtr->port_name == this->name) and 
		 (ftPtr->connection_id == port->second) and 
		 (ftPtr->stream_id == streamID) ){

	      if ( sri_iter != currentSRIs.end() && sri_iter->second.connections.count( port->second ) == 0 ) {
		this->_pushSRI( port, sri_iter->second );
	      }

              try {
		port->first->pushPacket(data, T, EOS, streamID.c_str());
		if ( stats.count(port->second) == 0 ) {
		  stats.insert( std::make_pair(port->second, linkStatistics( name, sizeof(NativeType) ) ) );
		}
                stats[port->second].update(data.length(), 0, EOS, streamID);
              } catch(...) {
                LOG_ERROR( logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << name << "/" << port->second );
              }
            }
          }
        }
	if (!portListed) {
	  for (port = outConnections.begin(); port != outConnections.end(); port++) {

	    if ( sri_iter != currentSRIs.end() && sri_iter->second.connections.count( port->second ) == 0 ) {
	      this->_pushSRI( port, sri_iter->second );
	    }

	    try {
	      port->first->pushPacket(data, T, EOS, streamID.c_str());
	      if ( stats.count(port->second) == 0 ) {
		stats.insert( std::make_pair(port->second, linkStatistics( name, sizeof(NativeType) ) ) );
	      }
	      stats[port->second].update(data.length(), 0, EOS, streamID);
            } catch(...) {
	      LOG_ERROR( logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << name << "/" << port->second );
            }
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
  void OutPort< PortTraits >::pushPacket(
          const DataBufferType &    data,
          BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID) {

    TRACE_ENTER(logger, "OutPort::pushPacket" );


    // check if sri has been pushed... 
    typename OutPortSriMap::iterator sri_iter =  currentSRIs.find( streamID );
    if ( sri_iter == currentSRIs.end() ) {
      BULKIO::StreamSRI H = bulkio::sri::create( streamID );
      pushSRI( H );
    }

    // don't want to process while command information is coming in
    SCOPED_LOCK lock(updatingPortsLock);
    _pushOversizedPacket(data, T, EOS, streamID);

    TRACE_EXIT(logger, "OutPort::pushPacket" );
  }




  template < typename PortTraits >
  BULKIO::UsesPortStatisticsSequence *  OutPort< PortTraits >::statistics()
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
  BULKIO::PortUsageType OutPort< PortTraits >::state()
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
  void OutPort< PortTraits >::enableStats(bool enable)
  {
    for (unsigned int i = 0; i < outConnections.size(); i++) {
      stats[outConnections[i].second].setEnabled(enable);
    }
  }


  template < typename PortTraits >
  ExtendedCF::UsesConnectionSequence * OutPort< PortTraits >::connections()
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
  void OutPort< PortTraits >::connectPort(CORBA::Object_ptr connection, const char* connectionId)
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
  void OutPort< PortTraits >::disconnectPort(const char* connectionId)
  {
    TRACE_ENTER(logger, "OutPort::disconnectPort" );
    {
      SCOPED_LOCK lock(updatingPortsLock);   // don't want to process while command information is coming in
      bool portListed = false;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {
        if (ftPtr->port_name == this->name) {
          portListed = true;
          break;
        }
      }

      std::string  cid(connectionId);      
      for (unsigned int i = 0; i < outConnections.size(); i++) {
        if (outConnections[i].second == connectionId) {
          PortSequenceType seq;
          typename OutPortSriMap::iterator cSRIs = currentSRIs.begin();
          BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::notSet();

          // send an EOS for every connection that's listed for this SRI
          for (; cSRIs!=currentSRIs.end(); cSRIs++) {
            std::string cSriSid(cSRIs->second.sri.streamID);

            // Check if we have sent out sri/data to the connection
            if ( cSRIs->second.connections.count( cid ) != 0 ) {
              bool sendEOS = false;
              if (portListed) {
                for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {
                  if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == cid ) and (ftPtr->stream_id == cSriSid)) {
                    sendEOS = true;
                    break;
                  }
                }
              } else {
                sendEOS = true;
              }
              
              if (sendEOS) {
                try {
                  outConnections[i].first->pushPacket(seq, tstamp, true, cSriSid.c_str());
                } catch (...) {
                  // Ignore all exceptions; the receiver may be dead
                }
              }
            }

            // remove connection id from sri connections list
            cSRIs->second.connections.erase( cid );

          }
          LOG_DEBUG( logger, "DISCONNECT, PORT/CONNECTION: "  << name << "/" << connectionId );
          outConnections.erase(outConnections.begin() + i);
          stats.erase( outConnections[i].second );
          break;
        }
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
  void  OutPort< PortTraits >::_pushSRI( typename ConnectionsList::iterator connPair, SriMapStruct &sri_ctx)
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
  void  OutPort< PortTraits >::_pushSRI( const std::string &connectionId, SriMapStruct &sri_ctx)
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
  bulkio::SriMap  OutPort< PortTraits >::getCurrentSRI()
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
  bulkio::SriList  OutPort< PortTraits >::getActiveSRIs()
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
  typename OutPort< PortTraits >::ConnectionsList  OutPort< PortTraits >::getConnections()
  {
      SCOPED_LOCK lock(updatingPortsLock);   // restrict access till method completes
      return outConnections;
  }

  template < typename PortTraits >
  void OutPort< PortTraits >::setLogger( LOGGER_PTR newLogger ) {
      logger = newLogger;
  }


  template < typename PortTraits >
  OutInt8Port< PortTraits >::OutInt8Port( std::string name,
                                          ConnectionEventListener *connectCB,
                                          ConnectionEventListener *disconnectCB ):
    OutPort < PortTraits >(name,connectCB, disconnectCB )
  {

  }


  template < typename PortTraits >
  OutInt8Port< PortTraits >::OutInt8Port( std::string name,
                                          LOGGER_PTR logger,
                                          ConnectionEventListener *connectCB,
                                          ConnectionEventListener *disconnectCB ) :
    OutPort < PortTraits >(name, logger, connectCB, disconnectCB )
  {

  }
  
  template <typename PortTraits>
  void OutInt8Port< PortTraits >::pushPacket( Int8* buffer, size_t size, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {

    TRACE_ENTER(this->logger, "OutInt8Port::pushPacket" );

    PortTypes::CharSequence seq = PortTypes::CharSequence(size, size, (CORBA::Char *)(buffer), false);
    _pushPacket(seq, T, EOS, streamID);

    TRACE_EXIT(this->logger, "OutInt8Port::pushPacket" );
  }
  
  template <typename PortTraits>
  void OutInt8Port< PortTraits >::pushPacket( char* buffer, size_t size, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {

    TRACE_ENTER(this->logger, "OutInt8Port::pushPacket" );

    PortTypes::CharSequence seq = PortTypes::CharSequence(size, size, (CORBA::Char *)(buffer), false);
    _pushPacket(seq, T, EOS, streamID);

    TRACE_EXIT(this->logger, "OutInt8Port::pushPacket" );
  }
  

  template <typename PortTraits>
  void OutInt8Port< PortTraits >::pushPacket( std::vector< Int8 >& data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {
    
    TRACE_ENTER(this->logger, "OutInt8Port::pushPacket" );
    
    PortTypes::CharSequence seq = PortTypes::CharSequence(data.size(), data.size(), (CORBA::Char *)&(data[0]), false);
    _pushPacket(seq, T, EOS, streamID);

    TRACE_EXIT(this->logger, "OutInt8Port::pushPacket" );
  }

  template <typename PortTraits>
  void OutInt8Port< PortTraits >::pushPacket( std::vector< Char >& data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {

    TRACE_ENTER(this->logger, "OutInt8Port::pushPacket" );

    // CORBA is allowed to delete the buffer when the sequence is destroyed.
    PortTypes::CharSequence seq = PortTypes::CharSequence(data.size(), data.size(), (CORBA::Char *)&(data[0]), false);
    _pushPacket(seq, T, EOS, streamID);

    TRACE_EXIT(this->logger, "OutInt8Port::pushPacket" );
  }

  template <typename PortTraits>
  void OutInt8Port< PortTraits >::_pushPacket( PortTypes::CharSequence& seq, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {
    
    // check if sri has been pushed... 
    typename OutInt8Port::OutPortSriMap::iterator sri_iter =  this->currentSRIs.find( streamID );
    if ( sri_iter == this->currentSRIs.end() ) {
      BULKIO::StreamSRI H = bulkio::sri::create( streamID );
      this->pushSRI( H );
    }

    SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
    
    if (this->active) {
      bool portListed = false;
      typename  OutInt8Port::ConnectionsList::iterator port;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
        for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {

          if (ftPtr->port_name == this->name)  portListed = true;

          if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == port->second) and (ftPtr->stream_id == streamID)) {
	    if ( sri_iter != this->currentSRIs.end() && sri_iter->second.connections.count( port->second ) == 0 ) {
	      this->_pushSRI( port, sri_iter->second );
	    }

            try {
              port->first->pushPacket(seq, T, EOS, streamID.c_str());
              if ( this->stats.count((*port).second) == 0 ) {
                this->stats.insert( std::pair< std::string, linkStatistics>((*port).second, linkStatistics( this->name, sizeof(Int8) ) ) );
              }
              this->stats[(*port).second].update(seq.length(), 0, 0, streamID);
            } catch(...) {
              LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
            }
          }
        }
      }
      if (!portListed) {
	for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {

	  if ( sri_iter != this->currentSRIs.end() && sri_iter->second.connections.count( port->second ) == 0 ) {
	    this->_pushSRI( port, sri_iter->second );
	  }

          try {
            port->first->pushPacket(seq, T, EOS, streamID.c_str());
            if ( this->stats.count((*port).second) == 0 ) {
              this->stats.insert( std::pair< std::string, linkStatistics>((*port).second, linkStatistics( this->name, sizeof(Int8) ) ) );
            }
            this->stats[(*port).second].update(seq.length(), 0, 0, streamID);
          } catch(...) {
            LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
          }
        }
      }
    }

    // if we have end of stream removed old sri
    try {
      if ( EOS ) this->currentSRIs.erase(streamID);
    }
    catch(...){
    }


    TRACE_EXIT(this->logger, "OutInt8Port::_pushPacket" );

  }



  template <typename PortTraits>
  OutStringPort< PortTraits >::OutStringPort ( std::string name,
                                               ConnectionEventListener *connectCB,
                                               ConnectionEventListener *disconnectCB ) :
    OutPort < PortTraits >(name,connectCB, disconnectCB )
  {

  }


  template <typename PortTraits>
  OutStringPort< PortTraits >::OutStringPort( std::string name,
                                              LOGGER_PTR logger,
                                              ConnectionEventListener *connectCB,
                                              ConnectionEventListener *disconnectCB ) :
    OutPort < PortTraits >(name,logger,connectCB, disconnectCB )
  {

  }


  template < typename PortTraits >
  void OutStringPort< PortTraits >::disconnectPort(const char* connectionId)
  {
    TRACE_ENTER(this->logger, "OutStringPort::disconnectPort" );
    {
        SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
        std::vector<connection_descriptor_struct >::iterator ftPtr;
        bool portListed = false;
        for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {
            if (ftPtr->port_name == this->name) {
                portListed = true;
                break;
            }
        }

	std::string cid(connectionId);
        for (unsigned int i = 0; i < this->outConnections.size(); i++) {
            if (this->outConnections[i].second == connectionId) {
                std::string data("");
                typename OutPort< PortTraits >::SriMap::iterator cSRIs = this->currentSRIs.begin();
                BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::notSet();
		  
		// send an EOS for every connection that's listed for this SRI
		for (; cSRIs!=this->currentSRIs.end(); cSRIs++) {
		  std::string cSriSid(cSRIs->second.first.streamID);
		  if (  cSRIs->second.connections.count( cid ) != 0 ) {
		    if (portListed) {
		      for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {
			if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == cid) and (ftPtr->stream_id == cSriSid)) {
			  try {
			    this->outConnections[i].first->pushPacket(data.c_str(), tstamp, true, cSriSid.c_str());
			  } catch(...) {}
			}
		      }
		    } else {
		      try {
			this->outConnections[i].first->pushPacket(data.c_str(), tstamp, true, cSriSid.c_str());
		      } catch(...) {}
		    }
		  }
		  // remove connection id from sri connections list
		  cSRIs->second.connections.erase( cid );
		}
                LOG_DEBUG( this->logger, "DISCONNECT, PORT/CONNECTION: "  << this->name << "/" << connectionId );
                this->outConnections.erase(this->outConnections.begin() + i);
                this->stats.erase( this->outConnections[i].second );
                break;
            }
        }

        if (this->outConnections.size() == 0) {
            this->active = false;
        }
        this->recConnectionsRefresh = true;
    }
    if (this->_disconnectCB) (*this->_disconnectCB)(connectionId);

    TRACE_EXIT(this->logger, "OutStringPort::disconnectPort" );
  }

  template <typename PortTraits>
  void OutStringPort< PortTraits >::pushPacket( const char *data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {


    TRACE_ENTER(this->logger, "OutStringPort::pushPacket" );

    // check if sri has been pushed... 
    typename OutStringPort::OutPortSriMap::iterator sri_iter =  this->currentSRIs.find( streamID );
    if ( sri_iter == this->currentSRIs.end() ) {
      BULKIO::StreamSRI H = bulkio::sri::create( streamID );
      this->pushSRI( H );
    }

    SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
    // grab SRI context 
    sri_iter =  this->currentSRIs.find( streamID );

    if (this->active) {
      bool portListed = false;
      typename OutPort< PortTraits >::ConnectionsList::iterator port;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
        for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {

          if (ftPtr->port_name == this->name) portListed=true;

          if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == port->second) and (ftPtr->stream_id == streamID)) {
	    if ( sri_iter != this->currentSRIs.end() && sri_iter->second.connections.count( port->second ) == 0 ) {
	      this->_pushSRI( port, sri_iter->second );
	    }
	    
            try {
              port->first->pushPacket(data, T, EOS, streamID.c_str());
              if ( this->stats.count((*port).second) == 0 ) {
                this->stats.insert( std::pair< std::string, linkStatistics>((*port).second, linkStatistics( this->name, sizeof(char) ) ) );
              }
              this->stats[(*port).second].update(1, 0, EOS, streamID);
            } catch(...) {
              LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
            }
          }
        }
      }
      if (!portListed) {
	for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {

	  if ( sri_iter != this->currentSRIs.end() && sri_iter->second.connections.count( port->second ) == 0 ) {
	    this->_pushSRI( port, sri_iter->second );
	  }

          try {
            port->first->pushPacket(data, T, EOS, streamID.c_str());
            if ( this->stats.count((*port).second) == 0 ) {
              this->stats.insert( std::pair< std::string, linkStatistics>((*port).second, linkStatistics( this->name, sizeof(char) ) ) );
            }
            this->stats[(*port).second].update(1, 0, EOS, streamID);
          } catch(...) {
            LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
          }
        }
      }
    }

    // if we have end of stream removed old sri
    try {
      if ( EOS ) this->currentSRIs.erase(streamID);
    }
    catch(...){
    }


    TRACE_EXIT(this->logger, "OutStringPort::pushPacket" );

  }


  template <typename PortTraits>
  void OutStringPort< PortTraits >::pushPacket( const char *data, bool EOS, const std::string& streamID) {

    TRACE_ENTER(this->logger, "OutStringPort::pushPacket" );

    // check if sri has been pushed... 
    typename OutStringPort::OutPortSriMap::iterator sri_iter =  this->currentSRIs.find( streamID );
    if ( sri_iter == this->currentSRIs.end() ) {
      BULKIO::StreamSRI H = bulkio::sri::create( streamID );
      this->pushSRI( H );
    }

    SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
    // grab SRI context 
    sri_iter =  this->currentSRIs.find( streamID );
    if (this->active) {
      bool portListed = false;
      typename OutPort< PortTraits >::ConnectionsList::iterator port;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
        for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {

          if (ftPtr->port_name == this->name) portListed = true;

          if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == port->second) and (ftPtr->stream_id == streamID)) {
	    if ( sri_iter != this->currentSRIs.end() && sri_iter->second.connections.count( port->second ) == 0 ) {
	      this->_pushSRI( port, sri_iter->second );
	    }

            try {
              BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();
              port->first->pushPacket(data, tstamp, EOS, streamID.c_str());
              if ( this->stats.count((*port).second) == 0 ) {
                this->stats.insert( std::pair< std::string, linkStatistics>((*port).second, linkStatistics( this->name, sizeof(char) ) ) );
              }
              this->stats[(*port).second].update(1, 0, EOS, streamID);
            } catch(...) {
              LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
            }
          }
        }
      }
      if (!portListed) {
	for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {

	  if ( sri_iter != this->currentSRIs.end() && sri_iter->second.connections.count( port->second ) == 0 ) {
	    this->_pushSRI( port, sri_iter->second );
	  }
          try {
            BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::now();
            port->first->pushPacket(data, tstamp, EOS, streamID.c_str());
            if ( this->stats.count((*port).second) == 0 ) {
              this->stats.insert( std::pair< std::string, linkStatistics>((*port).second, linkStatistics( this->name, sizeof(char) ) ) );
            }
            this->stats[(*port).second].update(1, 0, EOS, streamID);
          } catch(...) {
            LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
          }
        }
      }
    }

    // if we have end of stream removed old sri
    try {
      if ( EOS ) this->currentSRIs.erase(streamID);
    }
    catch(...){
    }

    TRACE_EXIT(this->logger, "OutStringPort::pushPacket" );

  }


  template <>
  void OutPort< XMLPortTraits >::disconnectPort(const char* connectionId) {
      TRACE_ENTER(this->logger, "OutPort::disconnectPort" );
    {
        SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
        std::vector<connection_descriptor_struct >::iterator ftPtr;
        bool portListed = false;
        for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {
            if (ftPtr->port_name == this->name) {
                portListed = true;
                break;
            }
        }

	std::string cid(connectionId);
        for (unsigned int i = 0; i < this->outConnections.size(); i++) {
            if (this->outConnections[i].second == connectionId) {
                std::string data("");
                OutPort< XMLPortTraits >::OutPortSriMap::iterator cSRIs = this->currentSRIs.begin();

		// send an EOS for every connection that's listed for this SRI
		for (; cSRIs!=this->currentSRIs.end(); cSRIs++) {
		  std::string cSriSid(cSRIs->second.sri.streamID);
		  if (  cSRIs->second.connections.count( cid ) != 0 ) {
		    if (portListed) {
		      for (ftPtr=this->filterTable.begin(); ftPtr != this->filterTable.end(); ftPtr++) {
			if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == outConnections[i].second) and (ftPtr->stream_id == cSriSid)) {
			  try {
			    outConnections[i].first->pushPacket(data.c_str(), true, cSriSid.c_str());
			  } catch(...) {}
			}
		      }
		    } else {
		      try {
			outConnections[i].first->pushPacket(data.c_str(), true, cSriSid.c_str());
		      } catch(...) {}
		    }
		  }
		  // remove connection id from sri connections list
		  cSRIs->second.connections.erase( cid );
                }
                LOG_DEBUG( this->logger, "DISCONNECT, PORT/CONNECTION: "  << this->name << "/" << connectionId );
                this->outConnections.erase(this->outConnections.begin() + i);
                this->stats.erase( this->outConnections[i].second );
                break;
            }
        }
        
        if (this->outConnections.size() == 0) {
            this->active = false;
        }
        this->recConnectionsRefresh = true;
    }
    if (this->_disconnectCB) (*this->_disconnectCB)(connectionId);
    
    TRACE_EXIT(this->logger, "OutStringPort::disconnectPort" );
  }
  
  template <>
  void OutStringPort< XMLPortTraits >::disconnectPort(const char* connectionId) {
      OutPort< XMLPortTraits >::disconnectPort(connectionId);
  }
  
  template <>
  void OutPort< FilePortTraits >::disconnectPort(const char* connectionId) {
      TRACE_ENTER(this->logger, "OutPort::disconnectPort" );
      {
          SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
	  std::string cid(connectionId);
          BULKIO::PrecisionUTCTime tstamp = bulkio::time::utils::notSet();
          for (unsigned int i = 0; i < this->outConnections.size(); i++) {
              if (this->outConnections[i].second == connectionId) {
                  std::string data("");
                  OutPort< FilePortTraits >::OutPortSriMap::iterator cSRIs = this->currentSRIs.begin();
		  
		  // send an EOS for every connection that's listed for this SRI
		  for (; cSRIs!=this->currentSRIs.end(); cSRIs++) {
		    std::string cSriSid(cSRIs->second.sri.streamID);
		    if (  cSRIs->second.connections.count( cid ) != 0 ) {
		      try {
			this->outConnections[i].first->pushPacket(data.c_str(), tstamp, true, cSriSid.c_str());
		      } catch(...) {}
		    }

		    // remove connection id from sri connections list
		    cSRIs->second.connections.erase( cid );
		  }
	      }
	      LOG_DEBUG( this->logger, "DISCONNECT, PORT/CONNECTION: "  << this->name << "/" << connectionId );
	      this->outConnections.erase(this->outConnections.begin() + i);
	      this->stats.erase( this->outConnections[i].second );
	      break;
          }
          
          if (this->outConnections.size() == 0) {
              this->active = false;
          }
          this->recConnectionsRefresh = true;
      }
      if (this->_disconnectCB) (*this->_disconnectCB)(connectionId);
      
      TRACE_EXIT(this->logger, "OutStringPort::disconnectPort" );
  }
  
  template <>
  void OutStringPort< FilePortTraits >::disconnectPort(const char* connectionId) {
      OutPort< FilePortTraits >::disconnectPort(connectionId);
  }
  
  template <>
  void OutStringPort< XMLPortTraits >::pushPacket( const char *data, BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID) {

    TRACE_ENTER(this->logger, "OutStringPort::pushPacket" );

    // check if sri has been pushed... 
    OutStringPort::OutPortSriMap::iterator sri_iter =  this->currentSRIs.find( streamID );
    if ( sri_iter == this->currentSRIs.end() ) {
      BULKIO::StreamSRI H = bulkio::sri::create( streamID );
      this->pushSRI( H );
    }

    SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
    sri_iter =  this->currentSRIs.find( streamID );
    if (this->active) {
      bool portListed = false;
      OutPort< XMLPortTraits >::ConnectionsList::iterator port;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
        for (ftPtr=filterTable.begin(); ftPtr != filterTable.end(); ftPtr++) {

	  if (ftPtr->port_name == this->name)  portListed = true;

          if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == port->second) and (ftPtr->stream_id == streamID)) {
	    if ( sri_iter != this->currentSRIs.end() && sri_iter->second.connections.count( port->second ) == 0 ) {
	      this->_pushSRI( port, sri_iter->second );
	    }

            try {
              port->first->pushPacket(data, EOS, streamID.c_str());
              this->stats[(*port).second].update(1, 0, EOS, streamID);
            } catch(...) {
              LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
            }
          }
        }
      }
      if (!portListed) {
	for (port = outConnections.begin(); port != outConnections.end(); port++) {

	  if ( sri_iter != this->currentSRIs.end() && sri_iter->second.connections.count( port->second ) == 0 ) {
	    this->_pushSRI( port, sri_iter->second );
	  }

          try {
            port->first->pushPacket(data, EOS, streamID.c_str());
            this->stats[(*port).second].update(1, 0, EOS, streamID);
          } catch(...) {
            LOG_ERROR( logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << name << "/" << port->second );
          }
        }
      }
    }

    // if we have end of stream removed old sri
    try {
      if ( EOS ) this->currentSRIs.erase(streamID);
    }
    catch(...){
    }

    TRACE_EXIT(this->logger, "OutStringPort::pushPacket" );

  }

  template <>
  void OutStringPort<  XMLPortTraits  >::pushPacket( const char *data, bool EOS, const std::string& streamID) {

    TRACE_ENTER(this->logger, "OutStringPort::pushPacket" );

    // check if sri has been pushed... 
    OutPort< XMLPortTraits >::OutPortSriMap::iterator sri_iter =  this->currentSRIs.find( streamID );
    if ( sri_iter == this->currentSRIs.end() ) {
      BULKIO::StreamSRI H = bulkio::sri::create( streamID );
      this->pushSRI( H );
    }

    SCOPED_LOCK lock(this->updatingPortsLock);   // don't want to process while command information is coming in
    sri_iter =  this->currentSRIs.find( streamID );
    if (this->active) {
      bool portListed = false;
      OutPort< XMLPortTraits >::ConnectionsList::iterator port;
      std::vector<connection_descriptor_struct >::iterator ftPtr;
      for (port = this->outConnections.begin(); port != this->outConnections.end(); port++) {
        for (ftPtr=filterTable.begin(); ftPtr != filterTable.end(); ftPtr++) {
	  if (ftPtr->port_name == this->name) portListed = true;
          if ((ftPtr->port_name == this->name) and (ftPtr->connection_id == port->second) and (ftPtr->stream_id == streamID)) {
	    if ( sri_iter != this->currentSRIs.end() && sri_iter->second.connections.count( port->second ) == 0 ) {
	      this->_pushSRI( port, sri_iter->second );
	    }
            try {
              port->first->pushPacket(data, EOS, streamID.c_str());
              this->stats[(*port).second].update(strlen(data), 0, EOS, streamID);
            } catch(...) {
              LOG_ERROR( this->logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << this->name << "/" << port->second );
            }
          }
        }
      }
      if (!portListed) {
	for (port = outConnections.begin(); port != outConnections.end(); port++) {
	  if ( sri_iter != this->currentSRIs.end() && sri_iter->second.connections.count( port->second ) == 0 ) {
	    this->_pushSRI( port, sri_iter->second );
	  }
          try {
            port->first->pushPacket(data, EOS, streamID.c_str());
            this->stats[(*port).second].update(strlen(data), 0, EOS, streamID);
          } catch(...) {
            LOG_ERROR( logger, "PUSH-PACKET FAILED, PORT/CONNECTION: " << name << "/" << port->second );
          }
        }
      }
    }

    // if we have end of stream removed old sri
    try {
      if ( EOS ) this->currentSRIs.erase(streamID);
    }
    catch(...){
    }


    TRACE_EXIT(this->logger, "OutStringPort::pushPacket" );
  }


  //
  // Required for Template Instantion for the compilation unit.
  // Note: we only define those valid types for which Bulkio IDL is defined. Users wanting to
  // inherit this functionality will be unable to since they cannot instantiate and
  // link against the template.
  //

  template class OutInt8Port<  CharPortTraits >;
  template class OutPort<  OctetPortTraits >;
  template class OutPort<  ShortPortTraits >;
  template class OutPort<  UShortPortTraits >;
  template class OutPort<  LongPortTraits >;
  template class OutPort<  ULongPortTraits >;
  template class OutPort<  LongLongPortTraits >;
  template class OutPort<  ULongLongPortTraits >;
  template class OutPort<  FloatPortTraits >;
  template class OutPort<  DoublePortTraits >;
  template class OutStringPort< FilePortTraits >;
  template class OutStringPort<  XMLPortTraits >;

  // The base template for certain types cannot be fully instantiated, so explicitly
  // instantiate any methods that are not directly referenced in the subclass (e.g.,
  // setLogger).
#define TEMPLATE_INSTANTIATE_BASE(x) \
  template void OutPort<x>::setLogger(LOGGER_PTR); \
  template void OutPort<x>::setNewConnectListener( ConnectionEventListener * ); \
  template void OutPort<x>::setNewConnectListener( ConnectionEventCallbackFn ); \
  template void OutPort<x>::setNewDisconnectListener( ConnectionEventListener * ); \
  template void OutPort<x>::setNewDisconnectListener( ConnectionEventCallbackFn );

  TEMPLATE_INSTANTIATE_BASE(CharPortTraits);
  TEMPLATE_INSTANTIATE_BASE(FilePortTraits);
  TEMPLATE_INSTANTIATE_BASE(XMLPortTraits);


} // end of bulkio namespace
