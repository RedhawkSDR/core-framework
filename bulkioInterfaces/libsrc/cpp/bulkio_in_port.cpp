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
#include <boost/thread.hpp>

#include <bulkio_p.h>
#include <bulkio_in_port.h>

namespace bulkio {

  // ----------------------------------------------------------------------------------------
  //  Source/Input Port Definitions
  // ----------------------------------------------------------------------------------------

  template <typename PortType>
  InPort<PortType>::InPort(std::string port_name, 
                           LOGGER_PTR  logger,
                           bulkio::sri::Compare sriCmp,
                           SriListener *newStreamCB):
    redhawk::NegotiableProvidesPortBase(port_name),
    sri_cmp(sriCmp),
    newStreamCallback(),
    maxQueue(100),
    breakBlock(false),
    blocking(false),
    stats(new linkStatistics(port_name))
  {
    // Manually set the bit size because the statistics ctor only takes a byte
    // count
    stats->setBitSize(NativeTraits<PortType>::bits);

    std::string _cmpMsg("USER_DEFINED");
    std::string _sriMsg("EMPTY");

    if (!logger) {
        std::string pname("redhawk.bulkio.inport.");
        pname = pname + port_name;
        setLogger(rh_logger::Logger::getLogger(pname));
    } else {
        setLogger(logger);
    }


    if (newStreamCB) {
      newStreamCallback = boost::ref(*newStreamCB);
      _sriMsg = "USER_DEFINED";
    }

    if (!sri_cmp) {
      _sriMsg = "DEFAULT";
      sri_cmp = bulkio::sri::DefaultComparator;
    }

    LOG_DEBUG( _portLog, "bulkio::InPort CTOR port:" << name << 
               " Blocking/MaxInputQueueSize " << blocking << "/" << maxQueue <<  
               " SriCompare/NewStreamCallback " << _cmpMsg << "/" << _sriMsg );
  }



  template <typename PortType>
  InPort<PortType>::~InPort()
  {
    TRACE_ENTER( _portLog, "InPort::DTOR" );

    // block any data coming out of getPacket.. 
    block();

    LOG_TRACE( _portLog, "PORT:" << name << " DUMP PKTS:" << packetQueue.size() );

    // purge the queue...
    while (packetQueue.size() != 0) {
      delete packetQueue.front();
      packetQueue.pop_front();
    }

    // clean up allocated containers
    if ( stats ) delete stats;

    TRACE_EXIT( _portLog, "InPort::DTOR"  );
  }


  template <typename PortType>
  BULKIO::PortStatistics * InPort<PortType>::statistics()
  {
    SCOPED_LOCK lock(dataBufferLock);
    BULKIO::PortStatistics_var recStat = new BULKIO::PortStatistics(stats->retrieve());
    // NOTE: You must delete the object that this function returns!
    return recStat._retn();
  }


  template <typename PortType>
  BULKIO::PortUsageType InPort<PortType>::state()
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


  template <typename PortType>
  BULKIO::StreamSRISequence * InPort<PortType>::activeSRIs()
  {
    SCOPED_LOCK lock(sriUpdateLock);
    BULKIO::StreamSRISequence_var retSRI = new BULKIO::StreamSRISequence();
    for (SriTable::iterator currH = currentHs.begin(); currH != currentHs.end(); ++currH) {
      ossie::corba::push_back(retSRI, currH->second.first.sri());
    }

    // NOTE: You must delete the object that this function returns!
    return retSRI._retn();
  }

  template <typename PortType>
  int InPort<PortType>::getMaxQueueDepth()
  {
    SCOPED_LOCK lock(dataBufferLock);
    return maxQueue;
  }

  template <typename PortType>
  int  InPort<PortType>::getCurrentQueueDepth()
  {
    SCOPED_LOCK lock(dataBufferLock);
    return packetQueue.size();
  }

  template <typename PortType>
  void InPort<PortType>::setMaxQueueDepth(int newDepth)
  {
    SCOPED_LOCK lock(dataBufferLock);
    maxQueue = newDepth;
  }

  template <typename PortType>
  void InPort<PortType>::setNewStreamListener(SriListener* newListener) {
      if (newListener) {
          newStreamCallback = boost::ref(*newListener);
      } else {
          newStreamCallback.clear();
      }
  }

  template <typename PortType>
  void InPort<PortType>::pushSRI(const BULKIO::StreamSRI& H)
  {
    TRACE_ENTER( _portLog, "InPort::pushSRI"  );

    // Acquire the mutex for the packet queue before the SRI mutex to avoid
    // priority inversion. Strictly speaking it's only required if the stream ID
    // already exists, while traversing the queue to look for EOS packets, but
    // this is always safe.
    SCOPED_LOCK data_lock(dataBufferLock);
    if (H.blocking) {
      blocking = true;
    }

    const std::string streamID(H.streamID);
    LOG_TRACE(_portLog,"pushSRI - FIND- PORT:" << name << " NEW SRI:" << streamID << " Mode:" << H.mode << " XDELTA:" << 1.0/H.xdelta );

    SCOPED_LOCK lock(sriUpdateLock);
    SriTable::iterator currH = currentHs.find(streamID);
    StreamDescriptor sri(H);
    if (currH == currentHs.end()) {
      // No need to access the packet queue, release the lock
      data_lock.unlock();

      LOG_DEBUG(_portLog,"pushSRI  PORT:" << name << " NEW SRI:" << streamID << " Mode:" << H.mode );
      if (newStreamCallback) {
        // The callback takes a non-const SRI, so allow access via const_cast
        newStreamCallback(const_cast<BULKIO::StreamSRI&>(sri.sri()));
      }
      currentHs[streamID] = std::make_pair(sri, true);
      lock.unlock();
      
      createStream(streamID, sri);
    } else {
      int eos_count = 0;
      for (typename PacketQueue::iterator ii = this->packetQueue.begin(); ii != this->packetQueue.end(); ++ii) {
          if (((*ii)->streamID == streamID) and ((*ii)->EOS)) {
              eos_count++;
          }
      }
      // Finished accessing the packet queue, release the lock
      data_lock.unlock();

      int additional_streams = 1+pendingStreams.count(streamID); // count current and pending streams
      if (additional_streams == eos_count) { // current and pending streams are all eos
        createStream(streamID, sri);
      } else {
        if (sri_cmp && !sri_cmp(H, currH->second.first.sri())) {
            LOG_DEBUG(_portLog,"pushSRI  PORT:" << name << " SAME SRI:" << streamID << " Mode:" << H.mode );
            currH->second.first = StreamDescriptor(H);
            currH->second.second = true;
        }
      }
    }
    TRACE_EXIT( _portLog, "InPort::pushSRI"  );
  }

    namespace {
        template <typename T>
        inline bool is_copy_required(const redhawk::shared_buffer<T>& data)
        {
            // If the data comes from a non-shared source (a vector or raw
            // pointer), we need to make a copy.
            return (data.transient() && !data.empty());
        }

        template <typename T>
        inline redhawk::shared_buffer<T> copy_data(const redhawk::shared_buffer<T>& data)
        {
            return data.copy();
        }

        inline bool is_copy_required(const redhawk::shared_bitbuffer& data)
        {
            // If the data comes from a non-shared source (a raw pointer), we
            // need to make a copy.
            return (data.transient() && !data.empty());
        }

        inline const redhawk::shared_bitbuffer copy_data(const redhawk::shared_bitbuffer& data)
        {
            return data.copy();
        }

        inline bool is_copy_required(const std::string&)
        {
            // Strings don't have sharing semantics, so no copy is required.
            return false;
        }

        inline const std::string& copy_data(const std::string& data)
        {
            // Pass through the string by reference (no copies made)
            return data;
        }

    }

  template <typename PortType>
  void  InPort<PortType>::queuePacket(const BufferType& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const std::string& streamID)
  {
    TRACE_ENTER( _portLog, "InPort::pushPacket"  );

    // Discard packets for disabled streams
    if (!_acceptPacket(streamID, EOS)) {
        if (EOS) {
            // If this was the only blocking stream, turn off blocking
            bool turnOffBlocking = _handleEOS(streamID);
            if (turnOffBlocking) {
                SCOPED_LOCK lock(dataBufferLock);
                blocking = false;
            }
        }
        return;
    }

    if (maxQueue == 0) {
      TRACE_EXIT( _portLog, "InPort::pushPacket"  );
      return;
    }

    // Discard empty packets if EOS is not set, as there is no useful data or
    // metadata to be had--since T applies to the 1st sample (which does not
    // exist), all we have is a stream ID
    if (data.empty() && !EOS) {
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
        LOG_WARN(_portLog, "InPort::pushPacket received data for stream '" << streamID << "' with no SRI");
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
      LOG_DEBUG(_portLog, "bulkio::InPort port blocking:" << blocking);
      if (blocking) {
        while (packetQueue.size() >= maxQueue) {
          queueAvailable.wait(lock);
        }
      } else {
        if (packetQueue.size() >= maxQueue) { // reached maximum queue depth - flush the queue
          LOG_DEBUG( _portLog, "bulkio::InPort pushPacket PURGE INPUT QUEUE (SIZE" << packetQueue.size() << ")" );
          flushToReport = true;

          // Need to hold the SRI mutex while flushing the queue because it may
          // update SRI change state
          SCOPED_LOCK lock(sriUpdateLock);
          _flushQueue();

          // Update the SRI change flag for this stream, which may have been
          // modified during the queue flush
          sriChanged = currentHs[streamID].second;
          currentHs[streamID].second = false;
        }
      }

      LOG_TRACE(_portLog, "bulkio::InPort pushPacket NEW PACKET (QUEUE" << packetQueue.size()+1 << ")");
      stats->update(length, (float)(packetQueue.size()+1)/(float)maxQueue, EOS, streamID, flushToReport);
      Packet *tmpIn;
      if (is_copy_required(data)) {
          tmpIn = new Packet(copy_data(data), T, EOS, sri, sriChanged, false);
      } else {
          tmpIn = new Packet(data, T, EOS, sri, sriChanged, false);
      }
      packetQueue.push_back(tmpIn);

      if (EOS) {
          SCOPED_LOCK lock(sriUpdateLock);
          SriTable::iterator target = currentHs.find(streamID);
          if (target != currentHs.end()) {
              currentHs.erase(target);
          }
      }

      // If a flush occurred, always set the flag on the first packet; this may
      // not be the packet that was just inserted if there were any EOS packets
      // on the queue
      if (flushToReport) {
        packetQueue.front()->inputQueueFlushed = true;
      }
      dataAvailable.notify_all();
    }

    packetWaiters.notify(streamID);

    TRACE_EXIT( _portLog, "InPort::pushPacket"  );
  }


  template <typename PortType>
  void InPort<PortType>::_flushQueue()
  {
    std::set<std::string> sri_changed;
    PacketQueue saved_packets;
    for (typename PacketQueue::iterator iter = packetQueue.begin(); iter != packetQueue.end(); ++iter) {
      Packet* packet = *iter;
      if (packet->EOS) {
        // Remove the SRI change flag for this stream, as further SRI changes
        // apply to a different stream; set the SRI change flag for the EOS
        // packet if there was one for this stream earlier in the queue
        if (sri_changed.erase(packet->streamID)) {
          packet->sriChanged = true;
        }

        // Discard data and preserve the EOS packet
        packet->buffer = BufferType();
        saved_packets.push_back(packet);
      } else {
        if (packet->sriChanged) {
          sri_changed.insert(packet->streamID);
        }
        delete packet;
      }
    }
    packetQueue.swap(saved_packets);

    // Save any SRI change flags that were collected and not applied to an EOS
    // packet
    for (std::set<std::string>::iterator stream_id = sri_changed.begin();
         stream_id != sri_changed.end(); ++stream_id) {
      // It should be safe to assume that an entry exists for the stream ID,
      // but just in case, use find instead of operator[]
      SriTable::iterator currH = currentHs.find(*stream_id);
      if (currH != currentHs.end()) {
        currH->second.second = true;
      }
    }
  }


  template <typename PortType>
  typename InPort<PortType>::Packet* InPort<PortType>::peekPacket(float timeout,
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

  template <typename PortType>
  void InPort<PortType>::enableStats( bool enable )
  {
    if (stats ) {
      stats->setEnabled(enable);
    }
  }


  template <typename PortType>
  void InPort<PortType>::block()
  {
    TRACE_ENTER( _portLog, "InPort::block"  );
    breakBlock = true;
    dataAvailable.notify_all();
    packetWaiters.interrupt();
    TRACE_EXIT( _portLog, "InPort::block"  );
  }

  template <typename PortType>
  void  InPort<PortType>::unblock()
  {    
    breakBlock = false;
  }

  template <typename PortType>
  void InPort<PortType>::stopPort()
  {
    block();
  }

  template <typename PortType>
  void  InPort<PortType>::startPort()
  {
    unblock();
  }

  template <typename PortType>
  bool  InPort<PortType>::blocked()
  {    
    return breakBlock;
  }

  template <typename PortType>
  typename InPort<PortType>::StreamType InPort<PortType>::getCurrentStream(float timeout)
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

  template <typename PortType>
  typename InPort<PortType>::StreamType InPort<PortType>::getStream(const std::string& streamID)
  {
    boost::mutex::scoped_lock lock(streamsMutex);
    typename StreamMap::iterator stream = streams.find(streamID);
    if (stream != streams.end()) {
      return stream->second;
    } else {
      return StreamType();
    }
  }

  template <typename PortType>
  typename InPort<PortType>::StreamList InPort<PortType>::getStreams()
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
  template <typename PortType>
  typename InPort<PortType>::DataTransferType * InPort<PortType>::getPacket(float timeout)
  {
    return getPacket(timeout, "");
  }

  template <typename PortType>
  typename InPort<PortType>::DataTransferType * InPort<PortType>::getPacket(float timeout, const std::string& streamID)
  {
    DataTransferType* transfer = 0;
    boost::scoped_ptr<Packet> packet(nextPacket(timeout, streamID));
    if (packet) {
      transfer = new DataTransferType(packet->buffer, packet->T, packet->EOS, packet->streamID.c_str(), packet->SRI.sri(), packet->sriChanged, packet->inputQueueFlushed);
      if (packet->EOS) {
        // When user code is calling getPacket(), it is safe to assume they are
        // not using using the stream API, so remove the associated stream here
        // to avoid leaking memory
        removeStream(packet->streamID);
      }
    }
    return transfer;
  }


  template <typename PortType>
  typename InPort<PortType>::Packet* InPort<PortType>::nextPacket(float timeout, const std::string& streamID)
  {
    TRACE_ENTER(_portLog, "InPort::nextPacket");
    if (breakBlock) {
      TRACE_EXIT(_portLog, "InPort::nextPacket");
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
          TRACE_EXIT(_portLog, "InPort::nextPacket");
          return NULL;
        } else if (timeout > 0){
          if (!dataAvailable.timed_wait(lock, to_time)) {
            TRACE_EXIT(_portLog, "InPort::nextPacket");
            return NULL;
          }
        } else {
            to_time  = boost::get_system_time() + boost::posix_time::seconds(1);
            while ((not breakBlock) and (not dataAvailable.timed_wait(lock, to_time))) {
                to_time  = boost::get_system_time() + boost::posix_time::seconds(1);
            }
        }
        if (breakBlock) {
          TRACE_EXIT(_portLog, "InPort::nextPacket");
          return NULL;
        }
        packet = fetchPacket(streamID);
      }
      
      if (!packet) {
        TRACE_EXIT(_portLog, "InPort::nextPacket");
        return NULL;
      }

      LOG_TRACE(_portLog, "InPort::nextPacket PORT:" << name << " (QUEUE="<< packetQueue.size() << ")");
      queueAvailable.notify_all();
    }

    bool turnOffBlocking = false;
    if (packet->EOS) {
      turnOffBlocking = _handleEOS(packet->streamID);
    }

    {
      SCOPED_LOCK lock(dataBufferLock);
      if (turnOffBlocking) {
        blocking = false;
      }
    }

    TRACE_EXIT( _portLog, "InPort::nextPacket"  );
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

  template <typename PortType>
  void InPort<PortType>::createStream(const std::string& streamID,
                                          const bulkio::StreamDescriptor& sri)
  {
    StreamType stream(sri, this);
    boost::mutex::scoped_lock lock(streamsMutex);
    if (streams.count(streamID) == 0) {
      // New stream
      LOG_DEBUG(_portLog, "Creating new stream " << streamID);
      streams.insert(std::make_pair(streamID, stream));
      lock.unlock();

      streamAdded(stream);
    } else {
      // An active stream has the same stream ID; add this new stream to the
      // pending list
      LOG_DEBUG(_portLog, "Creating pending stream " << streamID);
      pendingStreams.insert(std::make_pair(streamID, stream));
    }
  }

  template <typename PortType>
  typename InPort<PortType>::Packet * InPort<PortType>::fetchPacket(const std::string &streamID)
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

  template <typename PortType>
  void InPort<PortType>::discardPacketsForStream(const std::string& streamID)
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

  template <typename PortType>
  bool InPort<PortType>::_handleEOS(const std::string& streamID)
  {
      bool turnOffBlocking = true;
      SCOPED_LOCK lock(sriUpdateLock);
      SriTable::iterator target = currentHs.find(streamID);
      if (target != currentHs.end()) {
          SriTable::iterator currH;
          for (currH = currentHs.begin(); currH != currentHs.end(); currH++) {
              if (currH->second.first.blocking()) {
                  turnOffBlocking = false;
                  break;
              }
          }
      }
      return turnOffBlocking;
  }

  template <typename PortType>
  std::string   InPort<PortType>::getRepid() const {
    return PortType::_PD_repoId;
  }

  template <typename PortType>
  int InPort<PortType>::_getElementLength(const BufferType& data)
  {
    return data.size();
  }

  template <typename PortType>
  size_t InPort<PortType>::samplesAvailable (const std::string& streamID, bool firstPacket)
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

  template <typename PortType>
  void InPort<PortType>::removeStream(const std::string& streamID)
  {
    LOG_DEBUG(_portLog, "Removing stream " << streamID);
    boost::mutex::scoped_lock lock(streamsMutex);

    // Remove the current stream, and if there's a pending stream with the same
    // stream ID, move it to the active list
    typename StreamMap::iterator current = streams.find(streamID);
    if (current != streams.end()) {
      // There should always be a stream with the expected streamID when this
      // method is called, but just to be safe, only close and remove when we
      // know it's a valid stream
      current->second.close();
      streams.erase(current);
    }
    typename std::multimap<std::string,StreamType>::iterator next = pendingStreams.find(streamID);
    if (next != pendingStreams.end()) {
      LOG_DEBUG(_portLog, "Moving pending stream " << streamID << " to active");
      StreamType stream = next->second;
      streams.insert(*next);
      pendingStreams.erase(next);
      lock.unlock();

      streamAdded(stream);
    }
  }

  template <typename PortType>
  bool InPort<PortType>::isStreamActive(const std::string& streamID)
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

  template <typename PortType>
  bool InPort<PortType>::_acceptPacket(const std::string& streamID, bool EOS)
  {
      // Acquire streamsMutex for the duration of this call to ensure that
      // end-of-stream is handled atomically for disabled streams
      boost::mutex::scoped_lock lock(streamsMutex);

      // Find the current stream for the stream ID and check whether it's
      // enabled
      typename StreamMap::iterator stream = streams.find(streamID);
      if (stream == streams.end() || stream->second.enabled()) {
          return true;
      }

      // If there's a pending stream, the packet is designated for that
      if (pendingStreams.find(streamID) != pendingStreams.end()) {
          return true;
      }

      if (EOS) {
          // Acknowledge the end-of-stream by removing the disabled stream
          // before discarding the packet
          LOG_DEBUG(_portLog, "Removing stream " << streamID);
          stream->second.close();
          streams.erase(stream);

          typename std::multimap<std::string,StreamType>::iterator next = pendingStreams.find(streamID);
          if (next != pendingStreams.end()) {
              LOG_DEBUG(_portLog, "Moving pending stream " << streamID << " to active");
              StreamType stream = next->second;
              streams.insert(*next);
              pendingStreams.erase(next);
              lock.unlock();

              streamAdded(stream);
          }
      }
      return false;
  }

  template <typename PortType>
  bool InPort<PortType>::isStreamEnabled(const std::string& streamID)
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
  int InPort<BULKIO::dataFile>::_getElementLength(const std::string& /*unused*/)
  {
    return 1;
  }

  //
  template <typename PortType>
  InNumericPort<PortType>::InNumericPort(std::string port_name, 
                                         LOGGER_PTR  logger,
                                         bulkio::sri::Compare compareSri,
                                         SriListener *newStreamCB) :
    InPort<PortType>(port_name, logger, compareSri, newStreamCB)
  {
  }

  template <typename PortType>
  InNumericPort<PortType>::InNumericPort(std::string port_name, 
                                         bulkio::sri::Compare compareSri,
                                         SriListener *newStreamCB) :
    InPort<PortType>(port_name, LOGGER_PTR(), compareSri, newStreamCB)
  {
  }

  template <typename PortType>
  InNumericPort<PortType>::InNumericPort(std::string port_name, void* /*unused*/) :
    InPort<PortType>(port_name, LOGGER_PTR())
  {
  }

  template <typename PortType>
  void InNumericPort<PortType>::pushPacket(const PortSequenceType& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
  {
    size_t size = data.length();
    TransportType* ptr = const_cast<PortSequenceType&>(data).get_buffer(1);
    this->queuePacket(BufferType(reinterpret_cast<NativeType*>(ptr), size), T, EOS, streamID);
  }

  template <typename PortType>
  typename InNumericPort<PortType>::StreamList InNumericPort<PortType>::pollStreams(float timeout)
  {
    return pollStreams(0, timeout);
  }

  template <typename PortType>
  typename InNumericPort<PortType>::StreamList InNumericPort<PortType>::pollStreams(StreamList& pollset, float timeout)
  {
    return pollStreams(pollset, 0, timeout);
  }

  template <typename PortType>
  typename InNumericPort<PortType>::StreamList InNumericPort<PortType>::pollStreams(size_t samples, float timeout)
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

  template <typename PortType>
  typename InNumericPort<PortType>::StreamList InNumericPort<PortType>::pollStreams(StreamList& pollset, size_t samples, float timeout)
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

  template <typename PortType>
  typename InNumericPort<PortType>::StreamList InNumericPort<PortType>::getReadyStreams(size_t samples)
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


  InBitPort::InBitPort(const std::string& name, LOGGER_PTR logger) :
    InPort<BULKIO::dataBit>(name, logger)
  {
  }

  void InBitPort::pushPacket(const BULKIO::BitSequence& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
  {
    redhawk::shared_bitbuffer::data_type* ptr = const_cast<BULKIO::BitSequence&>(data).data.get_buffer(1);
    redhawk::shared_bitbuffer buffer(ptr, data.bits);
    queuePacket(buffer, T, EOS, streamID);
  }

  // ----------------------------------------------------------------------------------------
  //  Source Input Port String Definitions
  // ----------------------------------------------------------------------------------------
  InFilePort::InFilePort(std::string port_name, 
                         LOGGER_PTR  logger,
                         bulkio::sri::Compare compareSri,
                         SriListener *newStreamCB) :
    InPort<BULKIO::dataFile>(port_name, logger, compareSri, newStreamCB)
  {
  }


  InFilePort::InFilePort(std::string port_name, 
                         bulkio::sri::Compare compareSri,
                         SriListener *newStreamCB) :
    InPort<BULKIO::dataFile>(port_name, LOGGER_PTR(), compareSri, newStreamCB)
  {
  }

  InFilePort::InFilePort(std::string port_name, void* /*unused*/) :
    InPort<BULKIO::dataFile>(port_name, LOGGER_PTR())
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
    InPort<BULKIO::dataXML>(name, logger, compareSri, newStreamCB)
  {
  }


  InXMLPort::InXMLPort(std::string name,
                       bulkio::sri::Compare compareSri,
                       SriListener* newStreamCB) :
    InPort<BULKIO::dataXML>(name, LOGGER_PTR(), compareSri, newStreamCB)
  {
  }

  InXMLPort::InXMLPort(std::string name, void* /*unused*/) :
    InPort<BULKIO::dataXML>(name, LOGGER_PTR())
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
    template class InNumericPort<x>;

    FOREACH_PORT_TYPE(INSTANTIATE_TEMPLATE);
    FOREACH_NUMERIC_PORT_TYPE(INSTANTIATE_NUMERIC_TEMPLATE);

} // end of bulkio namespace
