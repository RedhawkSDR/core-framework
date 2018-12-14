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

#include <bulkio_out_port.h>
#include <BulkioTransport.h>

#include "LocalTransport.h"
#include "CorbaTransport.h"
#include "bulkio_p.h"

// Suppress warnings for access to deprecated currentSRI member (on gcc 4.4, at
// least, the implicit destructor call from OutPort's destructor emits a
// warning)
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

namespace bulkio {
  /*
     OutPort Constructor

     Accepts connect/disconnect interfaces for notification when these events occur
  */

  template <typename PortType>
  OutPort<PortType>::OutPort(const std::string& name,
                                 LOGGER_PTR logger,
                                 ConnectionEventListener *connectCB,
                                 ConnectionEventListener *disconnectCB) :
    redhawk::NegotiableUsesPort(name)
  {

    if (!logger) {
        std::string pname("redhawk.bulkio.outport.");
        pname = pname + name;
        setLogger(rh_logger::Logger::getLogger(pname));
    } else {
        setLogger(logger);
    }

    if ( connectCB ) {
      _connectCB = boost::shared_ptr< ConnectionEventListener >( connectCB, null_deleter() );
    }
    addConnectListener(this, &OutPort::_connectListenerAdapter);

    if ( disconnectCB ) {
      _disconnectCB = boost::shared_ptr< ConnectionEventListener >( disconnectCB, null_deleter() );
    }
    addDisconnectListener(this, &OutPort::_disconnectListenerAdapter);

    LOG_DEBUG( _portLog, "bulkio::OutPort::CTOR port:" << name );
  }

  template <typename PortType>
  OutPort<PortType>::~OutPort(){

  }


  template <typename PortType>
  void OutPort<PortType>::pushSRI(const BULKIO::StreamSRI& H)
  {
      TRACE_ENTER(_portLog, "OutPort::pushSRI" );
      LOG_TRACE(_portLog, "OutPort::pushSRI" );

      const std::string sid(H.streamID);
      SCOPED_LOCK lock(updatingPortsLock);   // don't want to process while command information is coming in
      StreamType stream;
      typename StreamMap::iterator existing = streams.find(sid);
      if (existing == streams.end()) {
          // Insert new SRI
          stream = StreamType(H, this);
          streams[sid] = stream;
      } else {
          // Overwrite existing SRI
          stream = existing->second;
          stream.sri(H);
      }
      const BULKIO::StreamSRI& sri = stream.sri();

      if (active) {
          for (TransportIterator connection = _connections.begin(); connection != _connections.end(); ++connection) {
              PortTransportType* transport = connection.transport();
              const std::string& connection_id = connection.connectionId();

              // Skip ports known to be dead
              if (!transport->isAlive()) {
                  continue;
              }

              if (!_isStreamRoutedToConnection(sid, connection_id)) {
                  continue;
              }

              LOG_DEBUG(_portLog,"pushSRI - PORT:" << name << " CONNECTION:" << connection_id << " SRI streamID:"
                        << stream.streamID() << " Mode:" << sri.mode << " XDELTA:" << 1.0/sri.xdelta);
              try {
                  transport->pushSRI(sid, sri, stream.modcount());
              } catch (const redhawk::FatalTransportError& err) {
                  LOG_ERROR(_portLog, "PUSH-SRI FAILED " << err.what()
                            << " PORT/CONNECTION: " << name << "/" << connection_id);
              }
          }
      }

      TRACE_EXIT(_portLog, "OutPort::pushSRI");
  }

  template <typename PortType>
  void OutPort<PortType>::_connectListenerAdapter(const std::string& connectionId)
  {
      if (_connectCB) {
          (*_connectCB)(connectionId.c_str());
      }
  }

  template <typename PortType>
  void OutPort<PortType>::_disconnectListenerAdapter(const std::string& connectionId)
  {
      if (_disconnectCB) {
          (*_disconnectCB)(connectionId.c_str());
      }
  }

  template <typename PortType>
  bool OutPort<PortType>::_isStreamRoutedToConnection(
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


  template <typename PortType>
  typename OutPort<PortType>::StreamType OutPort<PortType>::_getStream(const std::string& streamID)
  {
      typename StreamMap::iterator existing = streams.find(streamID);
      if (existing == streams.end()) {
          LOG_TRACE(_portLog, "Creating new stream '" << streamID << "' with default SRI");

          // No SRI associated with the stream ID, create a default one and add
          // it to the list; it will get pushed to downstream connections below
          StreamType stream(bulkio::sri::create(streamID), this);
          streams[streamID] = stream;
          return stream;
      } else {
          return existing->second;
      }
  }


  template <typename PortType>
  void OutPort<PortType>::_sendPacket(
          const BufferType&         data,
          const BULKIO::PrecisionUTCTime& T,
          bool                            EOS,
          const std::string&              streamID)
  {
    // don't want to process while command information is coming in
    SCOPED_LOCK lock(this->updatingPortsLock);

    // grab SRI context 
    StreamType stream = _getStream(streamID);

    if (active) {
        for (TransportIterator connection = _connections.begin(); connection != _connections.end(); ++connection) {
            PortTransportType* transport = connection.transport();
            const std::string& connection_id = connection.connectionId();
            // Skip ports known to be dead
            if (!transport->isAlive()) {
                continue;
            }

            // Check whether filtering is enabled and if this connection should
            // receive the stream
            if (!_isStreamRoutedToConnection(streamID, connection_id)) {
                continue;
            }

            try {
                transport->pushSRI(streamID, stream.sri(), stream.modcount());
                transport->pushPacket(data, T, EOS, streamID, stream.sri());
            } catch (const redhawk::FatalTransportError& err) {
                LOG_ERROR(_portLog, "PUSH-PACKET FAILED " << err.what()
                          << " PORT/CONNECTION: " << name << "/" << connection_id);
                transport->setAlive(false);
            } catch (const redhawk::TransportError& err) {
                LOG_ERROR(_portLog, "pushPacket error on connection '" << connection_id << "': " << err.what());
            }
        }
    }

    // if we have end of stream removed old sri
    if (EOS) {
      streams.erase(streamID);
    }
  }


  template <typename PortType>
  BULKIO::UsesPortStatisticsSequence* OutPort<PortType>::statistics()
  {
      SCOPED_LOCK   lock(updatingPortsLock);
      BULKIO::UsesPortStatisticsSequence_var recStat = new BULKIO::UsesPortStatisticsSequence();
      for (TransportIterator connection = _connections.begin(); connection != _connections.end(); ++connection) {
          BULKIO::UsesPortStatistics stat;
          stat.connectionId = connection.connectionId().c_str();
          stat.statistics = connection.transport()->getStatistics();
          stat.statistics.portName = name.c_str();
          ossie::corba::push_back(recStat, stat);
      }
      return recStat._retn();
  }

  template <typename PortType>
  BULKIO::PortUsageType OutPort<PortType>::state()
  {
    SCOPED_LOCK lock(updatingPortsLock);
    if (_connections.empty()) {
      return BULKIO::IDLE;
    } else {
      return BULKIO::ACTIVE;
    }
  }

  template <typename PortType>
  void OutPort<PortType>::enableStats(bool enable)
  {
      // Statistics are always collected
  }

  template <typename PortType>
  redhawk::UsesTransport*
  OutPort<PortType>::_createLocalTransport(PortBase* port, CORBA::Object_ptr object, const std::string& connectionId)
  {
      return LocalTransport<PortType>::Factory(this, port);
  }

  template <typename PortType>
  redhawk::UsesTransport*
  OutPort<PortType>::_createTransport(CORBA::Object_ptr object, const std::string& connectionId)
  {
      PortVarType port;
      try {
          port = PortType::_narrow(object);
          if (CORBA::is_nil(port)) {
              throw CF::Port::InvalidPort(1, "Unable to narrow");
          }
      } catch (const CORBA::SystemException&) {
          LOG_ERROR( _portLog, "CONNECT FAILED: UNABLE TO NARROW ENDPOINT,  USES PORT:" << name );
          throw CF::Port::InvalidPort(1, "Unable to narrow");
      }

      return CorbaTransportFactory<PortType>::Create(this, port);
  }


  template <typename PortType>
  typename OutPort<PortType>::StreamType OutPort<PortType>::getStream(const std::string& streamID)
  {
      boost::mutex::scoped_lock lock(updatingPortsLock);
      typename StreamMap::iterator stream = streams.find(streamID);
      if (stream != streams.end()) {
          return stream->second;
      } else {
          return StreamType();
      }
  }

  template <typename PortType>
  typename OutPort<PortType>::StreamList OutPort<PortType>::getStreams()
  {
      StreamList result;
      boost::mutex::scoped_lock lock(updatingPortsLock);
      for (typename StreamMap::const_iterator stream = streams.begin(); stream != streams.end(); ++stream) {
          result.push_back(stream->second);
      }
      return result;
  }

  template <typename PortType>
  typename OutPort<PortType>::StreamType OutPort<PortType>::createStream(const std::string& streamID)
  {
    boost::mutex::scoped_lock lock(updatingPortsLock);
    typename StreamMap::iterator existing = streams.find(streamID);
    if (existing != streams.end()) {
      return existing->second;
    }
    StreamType stream(bulkio::sri::create(streamID), this);
    streams[streamID] = stream;
    return stream;
  }

  template <typename PortType>
  typename OutPort<PortType>::StreamType OutPort<PortType>::createStream(const BULKIO::StreamSRI& sri)
  {
    boost::mutex::scoped_lock lock(updatingPortsLock);
    const std::string streamID(sri.streamID);
    typename StreamMap::iterator existing = streams.find(streamID);
    if (existing != streams.end()) {
      // Update the stream's SRI from the argument
      existing->second.sri(sri);
      return existing->second;
    }
    StreamType stream(sri, this);
    streams[streamID] = stream;
    return stream;
  }

  template <typename PortType>
  bulkio::SriMap  OutPort<PortType>::getCurrentSRI()
  {
    bulkio::SriMap ret;
    SCOPED_LOCK lock(updatingPortsLock);   // restrict access till method completes
    for (typename StreamMap::iterator stream = streams.begin() ; stream != streams.end(); ++stream) {
        ret[stream->first] = std::make_pair(stream->second.sri(), false);
    }
    return ret;
  }

  template <typename PortType>
  bulkio::SriList  OutPort<PortType>::getActiveSRIs()
  {
    bulkio::SriList ret;
    SCOPED_LOCK lock(updatingPortsLock);   // restrict access till method completes
    for (typename StreamMap::iterator stream = streams.begin() ; stream != streams.end(); ++stream) {
        ret.push_back(stream->second.sri());
    }
    return ret;
  }


  template <typename PortType>
  typename OutPort<PortType>::ConnectionsList  OutPort<PortType>::getConnections()
  {
    SCOPED_LOCK lock(updatingPortsLock);   // restrict access till method completes
    ConnectionsList outConnections;

    for (ConnectionList::iterator iter = _connections.begin(); iter != _connections.end(); ++iter) {
        PortVarType port = ossie::corba::_narrowSafe<PortType>((*iter)->objref);
        outConnections.push_back(std::make_pair(port, (*iter)->connectionId));
    }

    return outConnections;
  }


  template <typename PortType>
  void OutPort<PortType>::setNewConnectListener(ConnectionEventListener *newListener)
  {
    _connectCB =  boost::shared_ptr< ConnectionEventListener >(newListener, null_deleter());
  }

  template <typename PortType>
  void OutPort<PortType>::setNewConnectListener(ConnectionEventCallbackFn  newListener)
  {
    _connectCB =  boost::make_shared< StaticConnectionListener >( newListener );
  }

  template <typename PortType>
  void OutPort<PortType>::setNewDisconnectListener(ConnectionEventListener *newListener)
  {
    _disconnectCB =  boost::shared_ptr< ConnectionEventListener >(newListener, null_deleter());
  }

  template <typename PortType>
  void OutPort<PortType>::setNewDisconnectListener(ConnectionEventCallbackFn newListener)
  {
    _disconnectCB =  boost::make_shared< StaticConnectionListener >( newListener );
  }

  template <typename PortType>
  std::string   OutPort<PortType>::getRepid() const {
	return PortType::_PD_repoId;
    //return "IDL:CORBA/Object:1.0";
  }

  /*
     OutPort Constructor

     Accepts connect/disconnect interfaces for notification when these events occur
  */

  template <typename PortType>
  OutNumericPort<PortType>::OutNumericPort(const std::string& name,
                                               LOGGER_PTR logger,
                                               ConnectionEventListener *connectCB,
                                               ConnectionEventListener *disconnectCB ) :
    OutPort<PortType>(name, logger, connectCB, disconnectCB)
  {
  }


  template <typename PortType>
  OutNumericPort<PortType>::OutNumericPort(const std::string& name,
                                               ConnectionEventListener *connectCB,
                                               ConnectionEventListener *disconnectCB) :
    OutPort<PortType>(name, LOGGER_PTR(), connectCB, disconnectCB)
  {
  }


  template <typename PortType>
  OutNumericPort<PortType>::~OutNumericPort()
  {
  }

  template <typename PortType>
  void OutNumericPort<PortType>::pushPacket(
          const VectorType& data,
          const BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID)
  {
    this->_sendPacket(BufferType::make_transient(&data[0], data.size()), T, EOS, streamID);
  }

  template <typename PortType>
  void OutNumericPort<PortType>::pushPacket(
          const TransportType*      data,
          size_t                    size,
          const BULKIO::PrecisionUTCTime& T,
          bool                      EOS,
          const std::string&        streamID)
  {
    const NativeType* ptr = reinterpret_cast<const NativeType*>(data);
    this->_sendPacket(BufferType::make_transient(ptr, size), T, EOS, streamID);
  }

  OutCharPort::OutCharPort(const std::string& name,
                           ConnectionEventListener *connectCB,
                           ConnectionEventListener *disconnectCB):
    OutNumericPort<BULKIO::dataChar>(name,connectCB, disconnectCB)
  {

  }


  OutCharPort::OutCharPort(const std::string& name,
                           LOGGER_PTR logger,
                           ConnectionEventListener *connectCB,
                           ConnectionEventListener *disconnectCB) :
    OutNumericPort<BULKIO::dataChar>(name, logger, connectCB, disconnectCB)
  {

  }
  
  void OutCharPort::pushPacket(const Int8* buffer, size_t size, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID)
  {
    const TransportType* data = reinterpret_cast<const TransportType*>(buffer);
    OutNumericPort<BULKIO::dataChar>::pushPacket(data, size, T, EOS, streamID);
  }
  
  void OutCharPort::pushPacket(const char* buffer, size_t size, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID)
  {
    const TransportType* data = reinterpret_cast<const TransportType*>(buffer);
    OutNumericPort<BULKIO::dataChar>::pushPacket(data, size, T, EOS, streamID);
  }
  

  void OutCharPort::pushPacket(const std::vector< Int8 >& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID)
  {
    const TransportType* buffer = reinterpret_cast<const TransportType*>(&data[0]);
    OutNumericPort<BULKIO::dataChar>::pushPacket(buffer, data.size(), T, EOS, streamID);
  }

  void OutCharPort::pushPacket(const std::vector< Char >& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID)
  {
    const TransportType* buffer = reinterpret_cast<const TransportType*>(&data[0]);
    OutNumericPort<BULKIO::dataChar>::pushPacket(buffer, data.size(), T, EOS, streamID);
  }


  OutBitPort::OutBitPort(const std::string& name, LOGGER_PTR logger) :
    OutPort<BULKIO::dataBit>(name, logger)
  {
  }


  void OutBitPort::pushPacket(const redhawk::shared_bitbuffer& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID)
  {
    _sendPacket(data, T, EOS, streamID);
  }


  OutFilePort::OutFilePort(const std::string& name,
                           ConnectionEventListener *connectCB,
                           ConnectionEventListener *disconnectCB) :
    OutPort<BULKIO::dataFile>(name, LOGGER_PTR(), connectCB, disconnectCB)
  {
  }


  OutFilePort::OutFilePort(const std::string& name,
                           LOGGER_PTR logger,
                           ConnectionEventListener *connectCB,
                           ConnectionEventListener *disconnectCB) :
    OutPort<BULKIO::dataFile>(name,logger, connectCB, disconnectCB)
  {
  }


  void OutFilePort::pushPacket(const std::string& URL, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID)
  {
    _sendPacket(URL, T, EOS, streamID);
  }

  void OutFilePort::pushPacket(const char* URL, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID)
  {
    std::string url_out;
    if (URL) {
      url_out = URL;
    }
    this->pushPacket(url_out, T, EOS, streamID);
  }

  void OutFilePort::pushPacket(const char *data, bool EOS, const std::string& streamID)
  {
    this->pushPacket(data, bulkio::time::utils::now(), EOS, streamID);
  }


  OutXMLPort::OutXMLPort(const std::string& name,
                         ConnectionEventListener *connectCB,
                         ConnectionEventListener *disconnectCB) :
    OutPort<BULKIO::dataXML>(name, LOGGER_PTR(), connectCB, disconnectCB)
  {
  }


  OutXMLPort::OutXMLPort(const std::string& name,
                         LOGGER_PTR logger,
                         ConnectionEventListener *connectCB,
                         ConnectionEventListener *disconnectCB) :
    OutPort<BULKIO::dataXML>(name,logger,connectCB, disconnectCB)
  {
  }


  void OutXMLPort::pushPacket(const char *data, const BULKIO::PrecisionUTCTime& T, bool EOS, const std::string& streamID)
  {
    std::string data_out;
    if (data) {
      data_out = data;
    }
    _sendPacket(data_out, T, EOS, streamID);
  }


  void OutXMLPort::pushPacket(const std::string& data, bool EOS, const std::string& streamID)
  {
    // XML ports do not officially support timestamps, although the port
    // implementation includes it (because it's templatized); always pass
    // "not set" for consistency
    _sendPacket(data, bulkio::time::utils::notSet(), EOS, streamID);
  }

  void OutXMLPort::pushPacket(const char* data, bool EOS, const std::string& streamID)
  {
    std::string data_out;
    if (data) {
      data_out = data;
    }
    this->pushPacket(data_out, EOS, streamID);
  }


  //
  // Required for Template Instantion for the compilation unit.
  // Note: we only define those valid types for which Bulkio IDL is defined. Users wanting to
  // inherit this functionality will be unable to since they cannot instantiate and
  // link against the template.
  //

#define INSTANTIATE_TEMPLATE(x) \
    template class OutPort<x>;

#define INSTANTIATE_NUMERIC_TEMPLATE(x) \
    template class OutNumericPort<x>;

    FOREACH_PORT_TYPE(INSTANTIATE_TEMPLATE);
    FOREACH_NUMERIC_PORT_TYPE(INSTANTIATE_NUMERIC_TEMPLATE);

} // end of bulkio namespace
