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
#include <bulkio_p.h>

#include <bulkio_in_port.h>

namespace  bulkio {

  /*
   * Wrap Callback functions as SriListerer objects
   */
  class StaticSriCallback : public SriListener
  {
  public:
    virtual void operator() ( BULKIO::StreamSRI& sri)
    {
      (*func_)(sri);
    }

    StaticSriCallback ( SriListenerCallbackFn func) :
      func_(func)
    {
    }

  private:

    SriListenerCallbackFn func_;
  };


  // ----------------------------------------------------------------------------------------
  //  Source/Input Port Definitions
  // ----------------------------------------------------------------------------------------

  template < typename PortTraits >
  InPortBase< PortTraits >::InPortBase(std::string port_name, 
                                       LOGGER_PTR  logger,
                                       bulkio::sri::Compare sriCmp,
                                       SriListener *newStreamCB):
    Port_Provides_base_impl(port_name),
    sri_cmp(sriCmp),
    newStreamCallback(),
    breakBlock(false),
    blocking(false),
    queueSem(new queueSemaphore(100)),
    stats(new linkStatistics(port_name, sizeof(TransportType))),
    logger(logger)
  {
    std::string _cmpMsg("USER_DEFINED");
    std::string _sriMsg("EMPTY");

    if (newStreamCB) {
      newStreamCallback = boost::shared_ptr< SriListener >( newStreamCB, null_deleter());
      _sriMsg = "USER_DEFINED";
    }

    if (!sri_cmp) {
      _sriMsg = "DEFAULT";
      sri_cmp = bulkio::sri::DefaultComparator;
    }

    LOG_DEBUG( logger, "bulkio::InPort CTOR port:" << name << 
               " Blocking/MaxInputQueueSize " << blocking << "/" << queueSem->getMaxValue() <<  
               " SriCompare/NewStreamCallback " << _cmpMsg << "/" << _sriMsg );
  }



  template < typename PortTraits >
  InPortBase< PortTraits >::~InPortBase()
  {
    TRACE_ENTER( logger, "InPort::DTOR" );

    // block any data coming out of getPacket.. 
    block();

    LOG_TRACE( logger, "PORT:" << name << " DUMP PKTS:" << workQueue.size() );

    // purge the queue...
    while (workQueue.size() != 0) {
      DataTransferType *tmp = workQueue.front();
      workQueue.pop_front();
      delete tmp;
    }

    // clean up allocated containers
    if ( queueSem ) delete queueSem;

    if ( stats ) delete stats;

    TRACE_EXIT( logger, "InPort::DTOR"  );
  }



  template < typename PortTraits >
  BULKIO::PortStatistics * InPortBase< PortTraits >::statistics()
  {
    SCOPED_LOCK lock(dataBufferLock);
    BULKIO::PortStatistics_var recStat = new BULKIO::PortStatistics(stats->retrieve());
    // NOTE: You must delete the object that this function returns!
    return recStat._retn();
  }


  template < typename PortTraits >
  BULKIO::PortUsageType InPortBase< PortTraits >::state()
  {
    SCOPED_LOCK lock(dataBufferLock);
    if (workQueue.size() == queueSem->getMaxValue()) {
      return BULKIO::BUSY;
    } else if (workQueue.size() == 0) {
      return BULKIO::IDLE;
    } else {
      return BULKIO::ACTIVE;
    }

    return BULKIO::BUSY;
  }


  template < typename PortTraits >
  BULKIO::StreamSRISequence * InPortBase< PortTraits >::activeSRIs()
  {
    SCOPED_LOCK lock(sriUpdateLock);
    BULKIO::StreamSRISequence seq_rtn;
    SriMap::iterator currH;
    int i = 0;
    for (currH = currentHs.begin(); currH != currentHs.end(); currH++) {
      i++;
      seq_rtn.length(i);
      seq_rtn[i-1] = currH->second.first;
    }
    BULKIO::StreamSRISequence_var retSRI = new BULKIO::StreamSRISequence(seq_rtn);

    // NOTE: You must delete the object that this function returns!
    return retSRI._retn();
  }

  template < typename PortTraits >
  int InPortBase< PortTraits >::getMaxQueueDepth()
  {
    SCOPED_LOCK lock(dataBufferLock);
    return queueSem->getMaxValue();
  }

  template < typename PortTraits >
  int  InPortBase< PortTraits >::getCurrentQueueDepth()
  {
    SCOPED_LOCK lock(dataBufferLock);
    return workQueue.size();
  }

  template < typename PortTraits >
  void InPortBase< PortTraits >::setMaxQueueDepth(int newDepth)
  {
    SCOPED_LOCK lock(dataBufferLock);
    queueSem->setMaxValue(newDepth);
  }

  template < typename PortTraits >
  void InPortBase< PortTraits >::pushSRI(const BULKIO::StreamSRI& H)
  {
    TRACE_ENTER( logger, "InPort::pushSRI"  );

    if (H.blocking) {
      SCOPED_LOCK lock(dataBufferLock);
      blocking = true;
      queueSem->setCurrValue(workQueue.size());
    }

    const std::string streamID(H.streamID);
    BULKIO::StreamSRI tmpH = H; // mutable copy for callbacks
    LOG_TRACE(logger,"pushSRI - FIND- PORT:" << name << " NEW SRI:" << streamID << " Mode:" << H.mode << " XDELTA:" << 1.0/H.xdelta );

    SCOPED_LOCK lock(sriUpdateLock);
    SriMap::iterator currH = currentHs.find(streamID);
    if (currH == currentHs.end()) {
      LOG_DEBUG(logger,"pushSRI  PORT:" << name << " NEW SRI:" << streamID << " Mode:" << H.mode );
      if ( newStreamCallback ) (*newStreamCallback)(tmpH);
      currentHs[streamID] = std::make_pair(tmpH, true);
      lock.unlock();
      
      createStream(streamID, tmpH);
    } else {
      if ( sri_cmp && !sri_cmp(tmpH, currH->second.first)) {
        LOG_DEBUG(logger,"pushSRI  PORT:" << name << " SAME SRI:" << streamID << " Mode:" << H.mode );
        currentHs[streamID] = std::make_pair(tmpH, true);
      }
    }
    TRACE_EXIT( logger, "InPort::pushSRI"  );
  }


  template < typename PortTraits >
  void  InPortBase< PortTraits >::queuePacket(const PushArgumentType data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
  {

    TRACE_ENTER( logger, "InPort::pushPacket"  );
    if (queueSem->getMaxValue() == 0) {
      TRACE_EXIT( logger, "InPort::pushPacket"  );
      return;
    }

    if (!isStreamEnabled(streamID)) {
      return;
    }

    BULKIO::StreamSRI tmpH = {1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, streamID, false, 0};
    bool sriChanged = false;
    bool portBlocking = false;

    SriMap::iterator currH;
    {
      SCOPED_LOCK lock(sriUpdateLock);

      currH = currentHs.find(std::string(streamID));
      if (currH != currentHs.end()) {
        tmpH = currH->second.first;
        sriChanged = currH->second.second;
        currentHs[streamID] = std::make_pair(currH->second.first, false);
      } else {
        // Unknown stream ID, register a new default SRI following the logic in pushSRI,
        // and set the SRI changed flag
        LOG_WARN(logger, "InPort::pushPacket received data for stream '" << streamID << "' with no SRI");
        sriChanged = true;
        if (newStreamCallback) {
          (*newStreamCallback)(tmpH);
        }
        currentHs[streamID] = std::make_pair(tmpH, false);
        lock.unlock();

        createStream(streamID, tmpH);
      }
      portBlocking = blocking;
    }

    const size_t length = _getElementLength(data);
    LOG_DEBUG( logger, "bulkio::InPort port blocking:" << portBlocking );
    bool flushToReport = false;
    if(portBlocking) {
      queueSem->incr();
      SCOPED_LOCK lock(dataBufferLock);
      LOG_TRACE( logger, "bulkio::InPort pushPacket NEW PACKET (QUEUE" << workQueue.size()+1 << ")" );
      stats->update(length, (float)(workQueue.size()+1)/(float)queueSem->getMaxValue(), EOS, streamID, false);
      DataTransferType *tmpIn = new DataTransferType(data, T, EOS, streamID, tmpH, sriChanged, false);
      workQueue.push_back(tmpIn);
      dataAvailable.notify_all();
    } else {
      SCOPED_LOCK lock(dataBufferLock);
      bool sriChangedHappened = false;
      bool flagEOS = false;
      if (workQueue.size() == queueSem->getMaxValue()) { // reached maximum queue depth - flush the queue
        LOG_DEBUG( logger, "bulkio::InPort pushPacket PURGE INPUT QUEUE (SIZE" << workQueue.size() << ")" );
        flushToReport = true;
        DataTransferType *tmp;
        while (workQueue.size() != 0) {
          tmp = workQueue.front();
          if (tmp->sriChanged == true) {
            sriChangedHappened = true;
          }
          if (tmp->EOS == true) {
              flagEOS = true;
          }
          workQueue.pop_front();
          delete tmp;
        }
      }
      if (sriChangedHappened)
        sriChanged = true;
      if (flagEOS)
          EOS = true;

      LOG_DEBUG( logger, "bulkio::InPort pushPacket NEW Packet (QUEUE=" << workQueue.size()+1 << ")");
      stats->update(length, (float)(workQueue.size()+1)/(float)queueSem->getMaxValue(), EOS, streamID, flushToReport);
      DataTransferType *tmpIn = new DataTransferType(data, T, EOS, streamID, tmpH, sriChanged, flushToReport);
      workQueue.push_back(tmpIn);
      dataAvailable.notify_all();
    }

    packetReceived(streamID);

    TRACE_EXIT( logger, "InPort::pushPacket"  );
  }


  template < typename PortTraits >
  typename InPortBase< PortTraits >::DataTransferType* InPortBase< PortTraits >::peekPacket(float timeout)
  {
    uint64_t secs = (unsigned long)(trunc(timeout));
    uint64_t msecs = (unsigned long)((timeout - secs) * 1e6);
    boost::system_time to_time  = boost::get_system_time() + boost::posix_time::seconds(secs) + boost::posix_time::microseconds(msecs);
    boost::mutex::scoped_lock lock(this->dataBufferLock);
    while (!breakBlock && workQueue.empty()) {
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

    if (breakBlock || workQueue.empty()) {
      return 0;
    } else {
      return workQueue.front();
    }
  }

  template < typename PortTraits >
  bool InPortBase< PortTraits >::isStreamActive(const std::string& streamID)
  {
    return true;
  }

  template < typename PortTraits >
  bool InPortBase< PortTraits >::isStreamEnabled(const std::string& streamID)
  {
    return true;
  }

  template < typename PortTraits >
  void InPortBase< PortTraits >::packetReceived(const std::string& streamID)
  {
    if (isStreamActive(streamID)) {
      packetWaiters.notify(streamID);
    }
  }


  template < typename PortTraits >
  void InPortBase< PortTraits >::enableStats( bool enable )
  {
    if (stats ) {
      stats->setEnabled(enable);
    }
  }


  template < typename PortTraits >
  void InPortBase< PortTraits >::block()
  {
    TRACE_ENTER( logger, "InPort::block"  );
    breakBlock = true;
    queueSem->release();
    dataAvailable.notify_all();
    packetWaiters.interrupt();
    TRACE_EXIT( logger, "InPort::block"  );
  }

  template < typename PortTraits >
  void  InPortBase< PortTraits >::unblock()
  {    
    breakBlock = false;
  }

  template < typename PortTraits >
  void InPortBase< PortTraits >::stopPort()
  {
    block();
  }

  template < typename PortTraits >
  void  InPortBase< PortTraits >::startPort()
  {
    unblock();
  }

  template < typename PortTraits >
  bool  InPortBase< PortTraits >::blocked()
  {    
    return breakBlock;
  }

  /*
   * getPacket
   *     description: retrieve data from the provides (input) port
   *
   *  timeout: the amount of time to wait for data before a NULL is returned.
   *           Use 0.0 for non-blocking and -1 for blocking.
   */
  template < typename PortTraits >
  typename InPortBase< PortTraits >::DataTransferType * InPortBase< PortTraits >::getPacket(float timeout)
  {
    return getPacket(timeout, "");
  }


  template < typename PortTraits >
  typename InPortBase< PortTraits >::DataTransferType * InPortBase< PortTraits >::getPacket(float timeout, const std::string& streamID)
  {
    TRACE_ENTER( logger, "InPort::getPacket"  );
    if (breakBlock) {
      TRACE_EXIT( logger, "InPort::getPacket"  );
      return NULL;
    }

    DataTransferType *tmp=NULL;
    {
      SCOPED_LOCK lock(dataBufferLock);
      tmp = fetchPacket(streamID);
      uint64_t secs = (unsigned long)(trunc(timeout));
      uint64_t msecs = (unsigned long)((timeout - secs) * 1e6);
      boost::system_time to_time  = boost::get_system_time() + boost::posix_time::seconds(secs) + boost::posix_time::microseconds(msecs);
      while (!tmp) {
        if (timeout == 0.0) {
          TRACE_EXIT( logger, "InPort::getPacket"  );
          return NULL;
        } else if (timeout > 0){
          if (!dataAvailable.timed_wait(lock, to_time)) {
            TRACE_EXIT( logger, "InPort::getPacket"  );
            return NULL;
          }
        } else {
          dataAvailable.wait(lock);
        }
        if (breakBlock) {
          TRACE_EXIT( logger, "InPort::getPacket"  );
          return NULL;
        }
        tmp = fetchPacket(streamID);
      }
      
      LOG_TRACE( logger, "bulkio.InPort getPacket PORT:" << name << " (QUEUE="<< workQueue.size() << ")" );

    }

    if (!tmp) {
      TRACE_EXIT( logger, "InPort::getPacket"  );
      return NULL;
    }

    bool turnOffBlocking = false;
    if (tmp->EOS) {
      SCOPED_LOCK lock2(sriUpdateLock);
      SriMap::iterator target = currentHs.find(std::string(tmp->streamID));
      if (target != currentHs.end()) {
        bool sriBlocking = target->second.first.blocking;
        currentHs.erase(target);
        if (sriBlocking) {
          turnOffBlocking = true;
          SriMap::iterator currH;
          for (currH = currentHs.begin(); currH != currentHs.end(); currH++) {
            if (currH->second.first.blocking) {
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
        queueSem->setCurrValue(0);
        blocking = false;
      }

      if (blocking) {
        queueSem->decr();
      }
    }

    TRACE_EXIT( logger, "InPort::getPacket"  );
    return tmp;
  }


  template < typename PortTraits >
  typename InPortBase< PortTraits >::DataTransferType * InPortBase< PortTraits >::fetchPacket(const std::string &streamID)
  {
    if (streamID.empty()) {
      if (workQueue.empty()) {
        return 0;
      }
      DataTransferType* packet = workQueue.front();
      workQueue.pop_front();
      return packet;
    }

    for (typename WorkQueue::iterator ii = workQueue.begin(); ii != workQueue.end(); ++ii) {
      if ((*ii)->streamID == streamID) {
        DataTransferType* packet = *ii;
        if (ii == workQueue.begin()) {
          // PERFORMANCE NOTE:
          // In a 1-item deque, erase will end up calling pop_back(); however,
          // this can lead to greatly reduced performance (observed as 1/4 the
          // data rate on some systems). In the case where the deque alternates
          // between 0 and 1 packets (i.e., data is consumed as fast as it is
          // produced), alternating calls to push_back() and pop_back() will
          // always cause allocation and deallocation. Explicitly calling
          // pop_front() if it's the first element prevents this worst case
          // scenario.
          workQueue.pop_front();
        } else {
          workQueue.erase(ii);
        }
        return packet;
      }
    }
    return 0;
  }

  template < typename PortTraits >
  void   InPortBase< PortTraits >::setNewStreamListener( SriListener *newListener ) {
    newStreamCallback =  boost::shared_ptr< SriListener >(newListener, null_deleter());
  }

  template < typename PortTraits >
  void   InPortBase< PortTraits >::setNewStreamListener( SriListenerCallbackFn  newListener ) {
    newStreamCallback =  boost::make_shared< StaticSriCallback >( newListener );

  }

  template < typename PortTraits >
  void InPortBase< PortTraits >::createStream(const std::string& streamID, const BULKIO::StreamSRI& sri)
  {
  }

  template < typename PortTraits >
  void InPortBase< PortTraits >::removeStream(const std::string& streamID)
  {
  }

  template < typename PortTraits >
  void   InPortBase< PortTraits >::setLogger( LOGGER_PTR newLogger ) {
    logger = newLogger;
  }

  template < typename PortTraits >
  std::string   InPortBase< PortTraits >::getRepid() const {
	return PortType::_PD_repoId;
  }

  template < typename PortTraits >
  int InPortBase< PortTraits >::_getElementLength(const PushArgumentType data)
  {
    return data.length();
  }

  template < typename PortTraits >
  size_t InPortBase< PortTraits >::samplesAvailable (const std::string& streamID, bool firstPacket)
  {
    size_t samples = 0;
    size_t item_size = 1;
    SCOPED_LOCK lock(dataBufferLock);
    for (typename WorkQueue::iterator iter = workQueue.begin(); iter != workQueue.end(); ++iter) {
      DataTransferType* packet = *iter;
      if (packet->streamID != streamID) {
        continue;
      }
      if ((packet->sriChanged) || (packet->inputQueueFlushed)) {
        if (!firstPacket) break;
      }
      firstPacket = false;
      if (packet->SRI.mode) {
          item_size = 2;
      }
      samples += packet->dataBuffer.size();
    }
    return samples / item_size;
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
   * Specializations of base class methods for dataXML ports
   */

  template <>
  int InPortBase< FilePortTraits >::_getElementLength(const char* /*unused*/)
  {
    return 1;
  }

  /*
   * Specializations of base class methods for dataFile ports
   */

  template <>
  int InPortBase< XMLPortTraits >::_getElementLength(const char* data)
  {
    if (!data) {
      return 0;
    }
    return strlen(data);
  }

  //
  template < typename PortTraits >
  InPort< PortTraits >::InPort(std::string port_name, 
                               LOGGER_PTR  logger,
                               bulkio::sri::Compare compareSri,
                               SriListener *newStreamCB ) :
    InPortBase<PortTraits>(port_name, logger, compareSri, newStreamCB)
  {
  }

  template < typename PortTraits >
  InPort< PortTraits >::InPort(std::string port_name, 
                               bulkio::sri::Compare compareSri,
                               SriListener *newStreamCB ) :
    InPortBase<PortTraits>(port_name, LOGGER_PTR(), compareSri, newStreamCB)
  {
  }

  template < typename PortTraits >
  InPort< PortTraits >::InPort(std::string port_name, void* /*unused*/) :
    InPortBase<PortTraits>(port_name, LOGGER_PTR())
  {
  }

  template < typename PortTraits >
  void InPort< PortTraits >::pushPacket(const PortSequenceType& data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
  {
    this->queuePacket(data, T, EOS, streamID);
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
    DataTransferType* packet = this->peekPacket(timeout);
    if (packet) {
      const std::string& streamID = packet->streamID;
      return getStream(streamID);
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

  template < typename PortTraits >
  typename InPort< PortTraits >::StreamList InPort< PortTraits >::pollStreams(float timeout)
  {
    return pollStreams(0, timeout);
  }

  template < typename PortTraits >
  typename InPort< PortTraits >::StreamList InPort< PortTraits >::pollStreams(StreamList& pollset, float timeout)
  {
    return pollStreams(pollset, 0, timeout);
  }

  template < typename PortTraits >
  typename InPort< PortTraits >::StreamList InPort< PortTraits >::pollStreams(size_t samples, float timeout)
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
  typename InPort< PortTraits >::StreamList InPort< PortTraits >::pollStreams(StreamList& pollset, size_t samples, float timeout)
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
  void InPort< PortTraits >::createStream(const std::string& streamID, const BULKIO::StreamSRI& sri)
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

  template < typename PortTraits >
  typename InPort< PortTraits >::StreamList InPort< PortTraits >::getReadyStreams(size_t samples)
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
  template < typename PortTraits >
  InStringPort< PortTraits >::InStringPort(std::string port_name, 
                                           LOGGER_PTR  logger,
                                           bulkio::sri::Compare compareSri,
                                           SriListener *newStreamCB ) :
    InPortBase<PortTraits>(port_name, logger, compareSri, newStreamCB)
  {
  }


  template < typename PortTraits >
  InStringPort< PortTraits >::InStringPort(std::string port_name, 
                                           bulkio::sri::Compare compareSri,
                                           SriListener *newStreamCB ) :
    InPortBase<PortTraits>(port_name, LOGGER_PTR(), compareSri, newStreamCB)
  {
  }

  template < typename PortTraits >
  InStringPort< PortTraits >::InStringPort(std::string port_name, void* /*unused*/) :
    InPortBase<PortTraits>(port_name, LOGGER_PTR())
  {
  }

  template < typename PortTraits >
  void InStringPort< PortTraits >::pushPacket(const char *data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
  {
    this->queuePacket(data, T, EOS, streamID);
  }


  template < typename PortTraits >
  void InStringPort< PortTraits >::pushPacket(const char *data, CORBA::Boolean EOS, const char* streamID)
  {
    this->queuePacket(data, BULKIO::PrecisionUTCTime(), EOS, streamID);
  }

  //
  // Required for Template Instantion for the compilation unit.
  // Note: we only define those valid types for which Bulkio IDL is defined. Users wanting to
  // inherit this functionality will be unable to since they cannot instantiate and
  // link against the template.
  //

#define INSTANTIATE_BASE_TEMPLATE(x) \
  template class InPortBase<x>;

#define INSTANTIATE_TEMPLATE(x) \
  INSTANTIATE_BASE_TEMPLATE(x); template class InPort<x>;

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
  template class InStringPort< FilePortTraits >; 
  template class InStringPort< XMLPortTraits >;


} // end of bulkio namespace
