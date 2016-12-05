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

#include <boost/ref.hpp>
#include <boost/scoped_ptr.hpp>

#include <bulkio_p.h>
#include <bulkio_in_port.h>

namespace  bulkio {

  // ----------------------------------------------------------------------------------------
  //  Source/Input Port Definitions
  // ----------------------------------------------------------------------------------------

  template < typename PortTraits >
  InPort< PortTraits >::InPort(std::string port_name, 
                                       LOGGER_PTR  logger,
                                       bulkio::sri::Compare sriCmp,
                                       SriListener *newStreamCB):
    Port_Provides_base_impl(port_name),
    sri_cmp(sriCmp),
    newStreamCallback(),
    maxQueue(100),
    breakBlock(false),
    blocking(false),
    stats(new linkStatistics(port_name, sizeof(TransportType))),
    logger(logger)
  {
    std::string _cmpMsg("USER_DEFINED");
    std::string _sriMsg("EMPTY");

    if ( !logger  ) {
        std::string pname("redhawk.bulkio.inport.");
        pname = pname + port_name;
        logger = rh_logger::Logger::getLogger(pname);
    }


    if (newStreamCB) {
      newStreamCallback = boost::ref(*newStreamCB);
      _sriMsg = "USER_DEFINED";
    }

    if (!sri_cmp) {
      _sriMsg = "DEFAULT";
      sri_cmp = bulkio::sri::DefaultComparator;
    }

    LOG_DEBUG( logger, "bulkio::InPort CTOR port:" << name << 
               " Blocking/MaxInputQueueSize " << blocking << "/" << maxQueue <<  
               " SriCompare/NewStreamCallback " << _cmpMsg << "/" << _sriMsg );
  }



  template < typename PortTraits >
  InPort< PortTraits >::~InPort()
  {
    TRACE_ENTER( logger, "InPort::DTOR" );

    // block any data coming out of getPacket.. 
    block();

    LOG_TRACE( logger, "PORT:" << name << " DUMP PKTS:" << packetQueue.size() );

    // purge the queue...
    while (packetQueue.size() != 0) {
      delete packetQueue.front();
      packetQueue.pop_front();
    }

    // clean up allocated containers
    if ( stats ) delete stats;

    TRACE_EXIT( logger, "InPort::DTOR"  );
  }



  template < typename PortTraits >
  BULKIO::PortStatistics * InPort< PortTraits >::statistics()
  {
    SCOPED_LOCK lock(dataBufferLock);
    BULKIO::PortStatistics_var recStat = new BULKIO::PortStatistics(stats->retrieve());
    // NOTE: You must delete the object that this function returns!
    return recStat._retn();
  }


  template < typename PortTraits >
  BULKIO::PortUsageType InPort< PortTraits >::state()
  {
    SCOPED_LOCK lock(dataBufferLock);
    if (packetQueue.size() == maxQueue) {
      return BULKIO::BUSY;
    } else if (packetQueue.empty()) {
      return BULKIO::IDLE;
    } else {
      return BULKIO::ACTIVE;
    }

    return BULKIO::BUSY;
  }


  template < typename PortTraits >
  BULKIO::StreamSRISequence * InPort< PortTraits >::activeSRIs()
  {
    SCOPED_LOCK lock(sriUpdateLock);
    BULKIO::StreamSRISequence_var retSRI = new BULKIO::StreamSRISequence();
    for (SriTable::iterator currH = currentHs.begin(); currH != currentHs.end(); ++currH) {
      ossie::corba::push_back(retSRI, currH->second.first.sri());
    }

    // NOTE: You must delete the object that this function returns!
    return retSRI._retn();
  }

  template < typename PortTraits >
  int InPort< PortTraits >::getMaxQueueDepth()
  {
    SCOPED_LOCK lock(dataBufferLock);
    return maxQueue;
  }

  template < typename PortTraits >
  int  InPort< PortTraits >::getCurrentQueueDepth()
  {
    SCOPED_LOCK lock(dataBufferLock);
    return packetQueue.size();
  }

  template < typename PortTraits >
  void InPort< PortTraits >::setMaxQueueDepth(int newDepth)
  {
    SCOPED_LOCK lock(dataBufferLock);
    maxQueue = newDepth;
  }

  template < typename PortTraits >
  void InPort< PortTraits >::pushSRI(const BULKIO::StreamSRI& H)
  {
    TRACE_ENTER( logger, "InPort::pushSRI"  );

    if (H.blocking) {
      SCOPED_LOCK lock(dataBufferLock);
      blocking = true;
    }

    const std::string streamID(H.streamID);
    LOG_TRACE(logger,"pushSRI - FIND- PORT:" << name << " NEW SRI:" << streamID << " Mode:" << H.mode << " XDELTA:" << 1.0/H.xdelta );

    SCOPED_LOCK lock(sriUpdateLock);
    SriTable::iterator currH = currentHs.find(streamID);
    if (currH == currentHs.end()) {
      LOG_DEBUG(logger,"pushSRI  PORT:" << name << " NEW SRI:" << streamID << " Mode:" << H.mode );
      StreamDescriptor sri(H);
      if (newStreamCallback) {
        // The callback takes a non-const SRI, so allow access via const_cast
        newStreamCallback(const_cast<BULKIO::StreamSRI&>(sri.sri()));
      }
      currentHs[streamID] = std::make_pair(sri, true);
      lock.unlock();
      
      createStream(streamID, sri);
    } else {
      if (sri_cmp && !sri_cmp(H, currH->second.first.sri())) {
        LOG_DEBUG(logger,"pushSRI  PORT:" << name << " SAME SRI:" << streamID << " Mode:" << H.mode );
        currH->second.first = StreamDescriptor(H);
        currH->second.second = true;
      }
    }
    TRACE_EXIT( logger, "InPort::pushSRI"  );
  }


  template < typename PortTraits >
  void  InPort< PortTraits >::queuePacket(const SharedBufferType& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const std::string& streamID)
  {
    TRACE_ENTER( logger, "InPort::pushPacket"  );
    // Discard packets for disabled streams
    if (!isStreamEnabled(streamID)) {
        if (EOS) {
            // Acknowledge the end-of-stream by removing the disabled stream
            // before discarding the packet
            removeStream(streamID);
        }
        return;
    }

    if (maxQueue == 0) {
      TRACE_EXIT( logger, "InPort::pushPacket"  );
      return;
    }

    StreamDescriptor sri;
    bool sriChanged = false;

    {
      SCOPED_LOCK lock(sriUpdateLock);

      SriTable::iterator currH = currentHs.find(streamID);
      if (currH != currentHs.end()) {
        sri = currH->second.first;
        sriChanged = currH->second.second;
        currH->second.second = false;
      } else {
        // Unknown stream ID, register a new default SRI following the logic in pushSRI,
        // and set the SRI changed flag
        LOG_WARN(logger, "InPort::pushPacket received data for stream '" << streamID << "' with no SRI");
        sriChanged = true;
        sri = StreamDescriptor(bulkio::sri::create(streamID));
        if (newStreamCallback) {
          // The callback takes a non-const SRI, so allow access via const_cast
          newStreamCallback(const_cast<BULKIO::StreamSRI&>(sri.sri()));
        }
        currentHs[streamID] = std::make_pair(sri, false);
        lock.unlock();

        createStream(streamID, sri);
      }
    }

    const size_t length = _getElementLength(data);
    {
      bool flushToReport = false;
      SCOPED_LOCK lock(dataBufferLock);
      LOG_DEBUG(logger, "bulkio::InPort port blocking:" << blocking);
      if (blocking) {
        while (packetQueue.size() >= maxQueue) {
          queueAvailable.wait(lock);
        }
      } else {
        bool sriChangedHappened = false;
        bool flagEOS = false;
        if (packetQueue.size() >= maxQueue) { // reached maximum queue depth - flush the queue
          LOG_DEBUG( logger, "bulkio::InPort pushPacket PURGE INPUT QUEUE (SIZE" << packetQueue.size() << ")" );
          flushToReport = true;
          while (packetQueue.size() != 0) {
            Packet *tmp = packetQueue.front();
            if (tmp->sriChanged == true) {
              sriChangedHappened = true;
            }
            if (tmp->EOS == true) {
              flagEOS = true;
            }
            packetQueue.pop_front();
            delete tmp;
          }
        }
        if (sriChangedHappened) {
          sriChanged = true;
        }
        if (flagEOS) {
          EOS = true;
        }
      }

      LOG_TRACE(logger, "bulkio::InPort pushPacket NEW PACKET (QUEUE" << packetQueue.size()+1 << ")");
      stats->update(length, (float)(packetQueue.size()+1)/(float)maxQueue, EOS, streamID, false);
      Packet *tmpIn = new Packet(data, T, EOS, sri, sriChanged, flushToReport);
      packetQueue.push_back(tmpIn);
      dataAvailable.notify_all();
    }

    packetWaiters.notify(streamID);

    TRACE_EXIT( logger, "InPort::pushPacket"  );
  }


  template < typename PortTraits >
  typename InPort< PortTraits >::Packet* InPort< PortTraits >::peekPacket(float timeout,
                                                                                  boost::unique_lock<boost::mutex>& lock)
  {
    uint64_t secs = (unsigned long)(trunc(timeout));
    uint64_t msecs = (unsigned long)((timeout - secs) * 1e6);
    boost::system_time to_time  = boost::get_system_time() + boost::posix_time::seconds(secs) + boost::posix_time::microseconds(msecs);
    while (!breakBlock && packetQueue.empty()) {
      if (timeout == 0) {
        break;
      } else if (timeout > 0) {
        if (!dataAvailable.timed_wait(lock, to_time)) {
          break;
        }
      } else {
        dataAvailable.wait(lock);
      }
    }

    if (breakBlock || packetQueue.empty()) {
      return 0;
    } else {
      return packetQueue.front();
    }
  }

  template < typename PortTraits >
  void InPort< PortTraits >::enableStats( bool enable )
  {
    if (stats ) {
      stats->setEnabled(enable);
    }
  }


  template < typename PortTraits >
  void InPort< PortTraits >::block()
  {
    TRACE_ENTER( logger, "InPort::block"  );
    breakBlock = true;
    dataAvailable.notify_all();
    packetWaiters.interrupt();
    TRACE_EXIT( logger, "InPort::block"  );
  }

  template < typename PortTraits >
  void  InPort< PortTraits >::unblock()
  {    
    breakBlock = false;
  }

  template < typename PortTraits >
  void InPort< PortTraits >::stopPort()
  {
    block();
  }

  template < typename PortTraits >
  void  InPort< PortTraits >::startPort()
  {
    unblock();
  }

  template < typename PortTraits >
  bool  InPort< PortTraits >::blocked()
  {    
    return breakBlock;
  }

  template < typename PortTraits >
  typename InPort< PortTraits >::StreamType InPort< PortTraits >::getCurrentStream(float timeout)
  {
    // Prefer a stream that already has buffered data
    {
      boost::mutex::scoped_lock lock(streamsMutex);
      for (typename StreamMap::iterator stream = streams.begin(); stream != streams.end(); ++stream) {
        if (stream->second.hasBufferedData()) {
          return stream->second;
        }
      }
    }

    // Otherwise, return the stream that owns the next packet on the queue,
    // potentially waiting for one to be received
    boost::mutex::scoped_lock lock(this->dataBufferLock);
    Packet* packet = this->peekPacket(timeout, lock);
    if (packet) {
      return getStream(packet->streamID);
    }

    return StreamType();
  }

  template < typename PortTraits >
  typename InPort< PortTraits >::StreamType InPort< PortTraits >::getStream(const std::string& streamID)
  {
    boost::mutex::scoped_lock lock(streamsMutex);
    typename StreamMap::iterator stream = streams.find(streamID);
    if (stream != streams.end()) {
      return stream->second;
    } else {
      return StreamType();
    }
  }

  template < typename PortTraits >
  typename InPort< PortTraits >::StreamList InPort< PortTraits >::getStreams()
  {
    StreamList result;
    boost::mutex::scoped_lock lock(streamsMutex);
    for (typename StreamMap::const_iterator stream = streams.begin(); stream != streams.end(); ++stream) {
      result.push_back(stream->second);
    }
    return result;
  }

  /*
   * getPacket
   *     description: retrieve data from the provides (input) port
   *
   *  timeout: the amount of time to wait for data before a NULL is returned.
   *           Use 0.0 for non-blocking and -1 for blocking.
   */
  template < typename PortTraits >
  typename InPort< PortTraits >::DataTransferType * InPort< PortTraits >::getPacket(float timeout)
  {
    return getPacket(timeout, "");
  }


  template < typename PortTraits >
  typename InPort< PortTraits >::DataTransferType * InPort< PortTraits >::getPacket(float timeout, const std::string& streamID)
  {
    DataTransferType* transfer = 0;
    boost::scoped_ptr<Packet> packet(nextPacket(timeout, streamID));
    if (packet) {
      transfer = new DataTransferType(PortSequenceType(), packet->T, packet->EOS, packet->streamID.c_str(), packet->SRI.sri(), packet->sriChanged, packet->inputQueueFlushed);
      transfer->dataBuffer.assign(packet->buffer.begin(), packet->buffer.end());
    }
    return transfer;
  }


  template <typename PortTraits>
  typename InPort<PortTraits>::Packet* InPort<PortTraits>::nextPacket(float timeout, const std::string& streamID)
  {
    TRACE_ENTER(logger, "InPort::nextPacket");
    if (breakBlock) {
      TRACE_EXIT(logger, "InPort::nextPacket");
      return NULL;
    }

    Packet* packet = 0;
    {
      SCOPED_LOCK lock(dataBufferLock);
      packet = fetchPacket(streamID);
      uint64_t secs = (unsigned long)(trunc(timeout));
      uint64_t msecs = (unsigned long)((timeout - secs) * 1e6);
      boost::system_time to_time  = boost::get_system_time() + boost::posix_time::seconds(secs) + boost::posix_time::microseconds(msecs);
      while (!packet) {
        if (timeout == 0.0) {
          TRACE_EXIT(logger, "InPort::nextPacket");
          return NULL;
        } else if (timeout > 0){
          if (!dataAvailable.timed_wait(lock, to_time)) {
            TRACE_EXIT(logger, "InPort::nextPacket");
            return NULL;
          }
        } else {
          dataAvailable.wait(lock);
        }
        if (breakBlock) {
          TRACE_EXIT(logger, "InPort::nextPacket");
          return NULL;
        }
        packet = fetchPacket(streamID);
      }
      
      if (!packet) {
        TRACE_EXIT(logger, "InPort::nextPacket");
        return NULL;
      }

      LOG_TRACE(logger, "InPort::nextPacket PORT:" << name << " (QUEUE="<< packetQueue.size() << ")");
      queueAvailable.notify_all();
    }

    bool turnOffBlocking = false;
    if (packet->EOS) {
      SCOPED_LOCK lock2(sriUpdateLock);
      SriTable::iterator target = currentHs.find(packet->streamID);
      if (target != currentHs.end()) {
        bool sriBlocking = target->second.first.blocking();
        currentHs.erase(target);
        if (sriBlocking) {
          turnOffBlocking = true;
          SriTable::iterator currH;
          for (currH = currentHs.begin(); currH != currentHs.end(); currH++) {
            if (currH->second.first.blocking()) {
              turnOffBlocking = false;
              break;
            }
          }
        }
      }
    }

    {
      SCOPED_LOCK lock(dataBufferLock);
      if (turnOffBlocking) {
        blocking = false;
      }
    }

    TRACE_EXIT( logger, "InPort::nextPacket"  );
    return packet;
  }


  namespace {
    template <class T>
    inline typename std::deque<T>::iterator do_erase(std::deque<T>& container, typename std::deque<T>::iterator pos)
    {
      if (pos == container.begin()) {
        // PERFORMANCE NOTE:
        // In a 1-item deque, erase will end up calling pop_back(); however,
        // this can lead to greatly reduced performance (observed as 1/4 the
        // data rate on some systems). In the case where the deque alternates
        // between 0 and 1 packets (i.e., data is consumed as fast as it is
        // produced), alternating calls to push_back() and pop_back() will
        // always cause allocation and deallocation. Explicitly calling
        // pop_front() if it's the first element prevents this worst case
        // scenario.
        container.pop_front();
        return container.begin();
      } else {
        return container.erase(pos);
      }
    }
  }

  template < typename PortTraits >
  void InPort< PortTraits >::createStream(const std::string& streamID,
                                          const bulkio::StreamDescriptor& sri)
  {
    StreamType stream(sri, this);
    boost::mutex::scoped_lock lock(streamsMutex);
    if (streams.count(streamID) == 0) {
      // New stream
      LOG_DEBUG(logger, "Creating new stream " << streamID);
      streams.insert(std::make_pair(streamID, stream));
      lock.unlock();

      streamAdded(stream);
    } else {
      // An active stream has the same stream ID; add this new stream to the
      // pending list
      LOG_DEBUG(logger, "Creating pending stream " << streamID);
      pendingStreams.insert(std::make_pair(streamID, stream));
    }
  }

  template < typename PortTraits >
  typename InPort< PortTraits >::Packet * InPort< PortTraits >::fetchPacket(const std::string &streamID)
  {
    if (streamID.empty()) {
      if (packetQueue.empty()) {
        return 0;
      }
      Packet* packet = packetQueue.front();
      packetQueue.pop_front();
      return packet;
    }

    for (typename PacketQueue::iterator ii = packetQueue.begin(); ii != packetQueue.end(); ++ii) {
      if ((*ii)->streamID == streamID) {
        Packet* packet = *ii;
        bulkio::do_erase(packetQueue, ii);
        return packet;
      }
    }
    return 0;
  }

  template < typename PortTraits >
  void InPort< PortTraits >::discardPacketsForStream(const std::string& streamID)
  {
    SCOPED_LOCK lock(dataBufferLock);
    for (typename PacketQueue::iterator ii = packetQueue.begin(); ii != packetQueue.end();) {
      if ((*ii)->streamID == streamID) {
        bool eos = (*ii)->EOS;
        delete *ii;
        ii = bulkio::do_erase(packetQueue, ii);
        queueAvailable.notify_one();
        if (eos) {
          break;
        }
      } else {
        ++ii;
      }
    }
  }

  template < typename PortTraits >
  std::string   InPort< PortTraits >::getRepid() const {
    return PortType::_PD_repoId;
  }

  template < typename PortTraits >
  int InPort< PortTraits >::_getElementLength(const SharedBufferType& data)
  {
    return data.size();
  }

  template < typename PortTraits >
  size_t InPort< PortTraits >::samplesAvailable (const std::string& streamID, bool firstPacket)
  {
    size_t samples = 0;
    size_t item_size = 1;
    SCOPED_LOCK lock(dataBufferLock);
    for (typename PacketQueue::iterator iter = packetQueue.begin(); iter != packetQueue.end(); ++iter) {
      Packet* packet = *iter;
      if (packet->streamID != streamID) {
        continue;
      }
      if ((packet->sriChanged) || (packet->inputQueueFlushed)) {
        if (!firstPacket) break;
      }
      firstPacket = false;
      if (packet->SRI.complex()) {
          item_size = 2;
      }
      samples += packet->buffer.size();
    }
    return samples / item_size;
  }

  template < typename PortTraits >
  void InPort< PortTraits >::removeStream(const std::string& streamID)
  {
    LOG_DEBUG(logger, "Removing stream " << streamID);
    boost::mutex::scoped_lock lock(streamsMutex);
    streams.erase(streamID);
    typename std::multimap<std::string,StreamType>::iterator next = pendingStreams.find(streamID);
    if (next != pendingStreams.end()) {
      LOG_DEBUG(logger, "Moving pending stream " << streamID << " to active");
      StreamType stream = next->second;
      streams.insert(*next);
      pendingStreams.erase(next);
      lock.unlock();

      streamAdded(stream);
    }
  }

  template < typename PortTraits >
  bool InPort< PortTraits >::isStreamActive(const std::string& streamID)
  {
    SCOPED_LOCK lock(streamsMutex);
    if (pendingStreams.count(streamID) > 0) {
      // The current stream has received an EOS
      return false;
    } else if (streams.count(streamID) == 0) {
      // Unknown stream, presumably no SRI was received
      return false;
    }
    return true;
  }

  template < typename PortTraits >
  bool InPort< PortTraits >::isStreamEnabled(const std::string& streamID)
  {
    SCOPED_LOCK lock(streamsMutex);
    if (pendingStreams.count(streamID) == 0) {
      typename StreamMap::iterator stream = streams.find(streamID);
      if (stream != streams.end()) {
        if (!stream->second.enabled()) {
          return false;
        }
      }
    }
    return true;
  }

  namespace {
    template <class StreamType>
    inline bool is_ready(StreamType& stream, size_t size)
    {
      if (size == 0) {
        return stream.ready();
      } else {
        return (stream.samplesAvailable() >= size);
      }
    }

    template <class StreamList>
    inline StreamList ready_streams(StreamList& streams, size_t size)
    {
      StreamList result;
      for (typename StreamList::iterator stream = streams.begin(); stream != streams.end(); ++stream) {
        if (bulkio::is_ready(*stream, size)) {
          result.push_back(*stream);
        }
      }
      return result;
    }
  }

  /*
   * Specializations of base class methods for dataFile ports
   */

  template <>
  int InPort< FilePortTraits >::_getElementLength(const std::string& /*unused*/)
  {
    return 1;
  }

  //
  template < typename PortTraits >
  InNumericPort< PortTraits >::InNumericPort(std::string port_name, 
                               LOGGER_PTR  logger,
                               bulkio::sri::Compare compareSri,
                               SriListener *newStreamCB ) :
    InPort<PortTraits>(port_name, logger, compareSri, newStreamCB)
  {
  }

  template < typename PortTraits >
  InNumericPort< PortTraits >::InNumericPort(std::string port_name, 
                               bulkio::sri::Compare compareSri,
                               SriListener *newStreamCB ) :
    InPort<PortTraits>(port_name, LOGGER_PTR(), compareSri, newStreamCB)
  {
  }

  template < typename PortTraits >
  InNumericPort< PortTraits >::InNumericPort(std::string port_name, void* /*unused*/) :
    InPort<PortTraits>(port_name, LOGGER_PTR())
  {
  }

  template < typename PortTraits >
  void InNumericPort< PortTraits >::pushPacket(const PortSequenceType& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
  {
    size_t size = data.length();
    TransportType* ptr = const_cast<PortSequenceType&>(data).get_buffer(1);
    this->queuePacket(SharedBufferType(reinterpret_cast<NativeType*>(ptr), size), T, EOS, streamID);
  }

  template < typename PortTraits >
  typename InNumericPort< PortTraits >::StreamList InNumericPort< PortTraits >::pollStreams(float timeout)
  {
    return pollStreams(0, timeout);
  }

  template < typename PortTraits >
  typename InNumericPort< PortTraits >::StreamList InNumericPort< PortTraits >::pollStreams(StreamList& pollset, float timeout)
  {
    return pollStreams(pollset, 0, timeout);
  }

  template < typename PortTraits >
  typename InNumericPort< PortTraits >::StreamList InNumericPort< PortTraits >::pollStreams(size_t samples, float timeout)
  {
    redhawk::signal<std::string>::waiter waiter(&packetWaiters, timeout);

    StreamList result = getReadyStreams(samples);
    while (result.empty()) {
      // No streams are currently ready, wait for any to become ready
      if (!waiter.wait()) {
        break;
      }

      // Get the updated set of ready streams
      result = getReadyStreams(samples);
    }

    return result;
  }

  template < typename PortTraits >
  typename InNumericPort< PortTraits >::StreamList InNumericPort< PortTraits >::pollStreams(StreamList& pollset, size_t samples, float timeout)
  {
    redhawk::signal<std::string>::waiter waiter(&packetWaiters, timeout);

    redhawk::signal<std::string>::signal_set stream_ids;
    for (typename StreamList::iterator ii = pollset.begin(); ii != pollset.end(); ++ii) {
      stream_ids.insert(ii->streamID());
    }

    StreamList result = bulkio::ready_streams(pollset, samples);
    while (result.empty()) {
      // None of the requested streams are currently ready, wait for any of
      // them to become ready
      if (!waiter.wait(stream_ids)) {
        break;
      }

      // Get the updated set of ready streams
      result = bulkio::ready_streams(pollset, samples);
    }

    return result;
  }

  template < typename PortTraits >
  typename InNumericPort< PortTraits >::StreamList InNumericPort< PortTraits >::getReadyStreams(size_t samples)
  {
    StreamList result;
    boost::mutex::scoped_lock lock(streamsMutex);
    for (typename StreamMap::iterator stream = streams.begin(); stream != streams.end(); ++stream) {
      if (bulkio::is_ready(stream->second, samples)) {
        result.push_back(stream->second);
      }
    }
    return result;
  }


  // ----------------------------------------------------------------------------------------
  //  Source Input Port String Definitions
  // ----------------------------------------------------------------------------------------
  InFilePort::InFilePort(std::string port_name, 
                         LOGGER_PTR  logger,
                         bulkio::sri::Compare compareSri,
                         SriListener *newStreamCB) :
    InPort<FilePortTraits>(port_name, logger, compareSri, newStreamCB)
  {
  }


  InFilePort::InFilePort(std::string port_name, 
                         bulkio::sri::Compare compareSri,
                         SriListener *newStreamCB) :
    InPort<FilePortTraits>(port_name, LOGGER_PTR(), compareSri, newStreamCB)
  {
  }

  InFilePort::InFilePort(std::string port_name, void* /*unused*/) :
    InPort<FilePortTraits>(port_name, LOGGER_PTR())
  {
  }

  void InFilePort::pushPacket(const char *data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
  {
    if (!data) {
      this->queuePacket(std::string(), T, EOS, streamID);
    } else {
      this->queuePacket(data, T, EOS, streamID);
    }
  }


  InXMLPort::InXMLPort(std::string name,
                       LOGGER_PTR  logger,
                       bulkio::sri::Compare compareSri,
                       SriListener* newStreamCB) :
    InPort<XMLPortTraits>(name, logger, compareSri, newStreamCB)
  {
  }


  InXMLPort::InXMLPort(std::string name,
                       bulkio::sri::Compare compareSri,
                       SriListener* newStreamCB) :
    InPort<XMLPortTraits>(name, LOGGER_PTR(), compareSri, newStreamCB)
  {
  }

  InXMLPort::InXMLPort(std::string name, void* /*unused*/) :
    InPort<XMLPortTraits>(name, LOGGER_PTR())
  {
  }

  void InXMLPort::pushPacket(const char* data, CORBA::Boolean EOS, const char* streamID)
  {
    std::string buffer;
    if (data) {
      buffer = data;
    }
    // Use a default timestamp of "not set" for XML
    this->queuePacket(buffer, bulkio::time::utils::notSet(), EOS, streamID);
  }

  void InXMLPort::pushPacket(const char* data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
  {
    std::string buffer;
    if (data) {
      buffer = data;
    }
    this->queuePacket(buffer, T, EOS, streamID);
  }


  //
  // Required for Template Instantion for the compilation unit.
  // Note: we only define those valid types for which Bulkio IDL is defined. Users wanting to
  // inherit this functionality will be unable to since they cannot instantiate and
  // link against the template.
  //

#define INSTANTIATE_TEMPLATE(x) \
  template class InPort<x>;

#define INSTANTIATE_NUMERIC_TEMPLATE(x) \
  INSTANTIATE_TEMPLATE(x); template class InNumericPort<x>;

  INSTANTIATE_NUMERIC_TEMPLATE(CharPortTraits);
  INSTANTIATE_NUMERIC_TEMPLATE(OctetPortTraits);
  INSTANTIATE_NUMERIC_TEMPLATE(ShortPortTraits);
  INSTANTIATE_NUMERIC_TEMPLATE(UShortPortTraits);
  INSTANTIATE_NUMERIC_TEMPLATE(LongPortTraits);
  INSTANTIATE_NUMERIC_TEMPLATE(ULongPortTraits);
  INSTANTIATE_NUMERIC_TEMPLATE(LongLongPortTraits);
  INSTANTIATE_NUMERIC_TEMPLATE(ULongLongPortTraits);
  INSTANTIATE_NUMERIC_TEMPLATE(FloatPortTraits);
  INSTANTIATE_NUMERIC_TEMPLATE(DoublePortTraits);

  INSTANTIATE_TEMPLATE(FilePortTraits);
  INSTANTIATE_TEMPLATE(XMLPortTraits);

} // end of bulkio namespace
