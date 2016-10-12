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

  /**
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
  InPort< PortTraits >::InPort(std::string port_name, 
                               bulkio::sri::Compare sriCmp,
                               SriListener *newStreamCB):
    Port_Provides_base_impl(port_name),
    sri_cmp(sriCmp),
    newStreamCallback(),
    stats(NULL)
  {
    // create semaphore to track queue size
    queueSem = new queueSemaphore(100);

    // create stats object to track data ingest
    stats = new linkStatistics(port_name, sizeof(TransportType) );
    
    if ( newStreamCB ) {
      newStreamCallback = boost::shared_ptr< SriListener >( newStreamCB, null_deleter());
    }

    // set state to allow for back pressure to call if queue is full
    blocking = false;

    // controls the flow from getPacket... false will allow the flow
    breakBlock = false;

    if ( sri_cmp == NULL ) {
      sri_cmp = bulkio::sri::DefaultComparator;
    }

  }

  template < typename PortTraits >
  InPort< PortTraits >::InPort(std::string port_name, void *):
    Port_Provides_base_impl(port_name),
    sri_cmp(bulkio::sri::DefaultComparator),
    newStreamCallback(),
    stats(NULL)
  {
    // create semaphore to track queue size
    queueSem = new queueSemaphore(100);

    // create stats object to track data ingest
    stats = new linkStatistics(port_name, sizeof(TransportType) );

    // set state to allow for back pressure to call if queue is full
    blocking = false;

    // controls the flow from getPacket... false will allow the flow
    breakBlock = false;

  }


  template < typename PortTraits >
  InPort< PortTraits >::InPort(std::string port_name, 
                               LOGGER_PTR  logger,
                               bulkio::sri::Compare sriCmp,
                               SriListener *newStreamCB):
    Port_Provides_base_impl(port_name),
    sri_cmp(sriCmp),
    newStreamCallback(),
    stats(NULL),
    logger(logger)
  {
    std::string _cmpMsg("USER_DEFINED");
    std::string _sriMsg("EMPTY");

    // create semaphore to track queue size
    queueSem = new queueSemaphore(100);

    // create stats object to track data ingest
    stats = new linkStatistics(port_name, sizeof(TransportType) );
    
    if ( newStreamCB ) {
      newStreamCallback = boost::shared_ptr< SriListener >( newStreamCB, null_deleter());
      _sriMsg = "USER_DEFINED";
    }

    // set state to allow for back pressure to call if queue is full
    blocking = false;

    // controls the flow from getPacket... false will allow the flow
    breakBlock = false;

    if ( sri_cmp == NULL ) {
      _sriMsg = "DEFAULT";
      sri_cmp = bulkio::sri::DefaultComparator;
    }

    LOG_DEBUG( logger, "bulkio::InPort CTOR port:" << name << 
               " Blocking/MaxInputQueueSize " << blocking << "/" << queueSem->getMaxValue() <<  
               " SriCompare/NewStreamCallback " << _cmpMsg << "/" << _sriMsg );
  }



  template < typename PortTraits >
  InPort< PortTraits >::~InPort()
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
  BULKIO::StreamSRISequence * InPort< PortTraits >::activeSRIs()
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
  int InPort< PortTraits >::getMaxQueueDepth()
  {
    SCOPED_LOCK lock(dataBufferLock);
    return queueSem->getMaxValue();
  }

  template < typename PortTraits >
  int  InPort< PortTraits >::getCurrentQueueDepth()
  {
    SCOPED_LOCK lock(dataBufferLock);
    return workQueue.size();
  }

  template < typename PortTraits >
  void InPort< PortTraits >::setMaxQueueDepth(int newDepth)
  {
    SCOPED_LOCK lock(dataBufferLock);
    queueSem->setMaxValue(newDepth);
  }

  template < typename PortTraits >
  void InPort< PortTraits >::pushSRI(const BULKIO::StreamSRI& H)
  {
    TRACE_ENTER( logger, "InPort::pushSRI"  );

    if (H.blocking) {
      SCOPED_LOCK lock(dataBufferLock);
      blocking = true;
      queueSem->setCurrValue(workQueue.size());
    }

    SCOPED_LOCK lock(sriUpdateLock);
    BULKIO::StreamSRI tmpH = H;
    LOG_TRACE(logger,"pushSRI - FIND- PORT:" << name << " NEW SRI:" << H.streamID << " Mode:" << H.mode << " XDELTA:" << 1.0/H.xdelta );  
    SriMap::iterator currH = currentHs.find(std::string(H.streamID));
    if (currH == currentHs.end()) {
      LOG_DEBUG(logger,"pushSRI  PORT:" << name << " NEW SRI:" << H.streamID << " Mode:" << H.mode );
      if ( newStreamCallback ) (*newStreamCallback)(tmpH);
      currentHs[std::string(H.streamID)] = std::make_pair(tmpH, true);
    } else {
      if ( sri_cmp && !sri_cmp(tmpH, currH->second.first)) {
        LOG_DEBUG(logger,"pushSRI  PORT:" << name << " SAME SRI:" << H.streamID << " Mode:" << H.mode );
        currentHs[std::string(H.streamID)] = std::make_pair(tmpH, true);
      }
    }
    TRACE_EXIT( logger, "InPort::pushSRI"  );
  }


  template < typename PortTraits >
  void  InPort< PortTraits >::pushPacket(const PortSequenceType & data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
  {

    TRACE_ENTER( logger, "InPort::pushPacket"  );
    if (queueSem->getMaxValue() == 0) {
      TRACE_EXIT( logger, "InPort::pushPacket"  );
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
      }
      portBlocking = blocking;
    }


    LOG_DEBUG( logger, "bulkio::InPort port blocking:" << portBlocking );
    bool flushToReport = false;
    if(portBlocking) {
      queueSem->incr();
      SCOPED_LOCK lock(dataBufferLock);
      LOG_TRACE( logger, "bulkio::InPort pushPacket NEW PACKET (QUEUE" << workQueue.size()+1 << ")" );
      stats->update(data.length(), (float)(workQueue.size()+1)/(float)queueSem->getMaxValue(), EOS, streamID, false);
      DataTransferType *tmpIn = new DataTransferType(data, T, EOS, streamID, tmpH, sriChanged, false);
      workQueue.push_back(tmpIn);
      dataAvailable.notify_all();
    } else {
      SCOPED_LOCK lock(dataBufferLock);
      bool sriChangedHappened = false;
      if (workQueue.size() == queueSem->getMaxValue()) { // reached maximum queue depth - flush the queue
        LOG_DEBUG( logger, "bulkio::InPort pushPacket PURGE INPUT QUEUE (SIZE" << workQueue.size() << ")" );
        flushToReport = true;
        DataTransferType *tmp;
        while (workQueue.size() != 0) {
          tmp = workQueue.front();
          if (tmp->sriChanged == true) {
            sriChangedHappened = true;
          }
          workQueue.pop_front();
          delete tmp;
        }
      }
      if (sriChangedHappened)
        sriChanged = true;

      LOG_DEBUG( logger, "bulkio::InPort pushPacket NEW Packet (QUEUE=" << workQueue.size()+1 << ")");
      stats->update(data.length(), (float)(workQueue.size()+1)/(float)queueSem->getMaxValue(), EOS, streamID, flushToReport);
      DataTransferType *tmpIn = new DataTransferType(data, T, EOS, streamID, tmpH, sriChanged, flushToReport);
      workQueue.push_back(tmpIn);
      dataAvailable.notify_all();
    }

    TRACE_EXIT( logger, "InPort::pushPacket"  );
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
    queueSem->release();
    dataAvailable.notify_all();
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
    TRACE_ENTER( logger, "InPort::getPacket"  );
    if (breakBlock) {
      TRACE_EXIT( logger, "InPort::getPacket"  );
      return NULL;
    }

    DataTransferType *tmp = NULL;
    {
      SCOPED_LOCK lock(dataBufferLock);
      if (workQueue.size() == 0) {
        if (timeout == 0.0) {
          TRACE_EXIT( logger, "InPort::getPacket"  );
          return NULL;
        } else if (timeout > 0){

          uint64_t secs = (unsigned long)(trunc(timeout));
          //uint64_t nsecs = (unsigned long)((timeout - secs) * 1e9);
          uint64_t msecs = (unsigned long)((timeout - secs) * 1e6);
          boost::system_time to_time  = boost::get_system_time() + boost::posix_time::seconds(secs) + boost::posix_time::microseconds(msecs);
          LOG_DEBUG( logger, "bulkio.InPort getPacket PORT:" << name << " TIMED WAIT:" << timeout);
          if ( dataAvailable.timed_wait( lock, to_time) == false ) {
            TRACE_EXIT( logger, "InPort::getPacket"  );
            return NULL;
          }

          if (breakBlock) {
            TRACE_EXIT( logger, "InPort::getPacket"  );
            return NULL;
          }
        } else {
          LOG_DEBUG( logger, "bulkio.InPort getPacket PORT:" << name << " Block until data arrives" );
          dataAvailable.wait(lock);
          if (breakBlock) {
            TRACE_EXIT( logger, "InPort::getPacket"  );
            return NULL;
          }
        }
      }
      tmp = workQueue.front();
      workQueue.pop_front();
      LOG_TRACE( logger, "bulkio.InPort getPacket PORT:" << name << " (QUEUE="<< workQueue.size() << ")" );
    }

    bool turnOffBlocking = false;
    if (tmp && tmp->EOS) {
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
  typename InPort< PortTraits >::DataTransferType * InPort< PortTraits >::getPacket(float timeout, std::string &streamID )
  {
    TRACE_ENTER( logger, "InPort::getPacket"  );
    if (breakBlock) {
      TRACE_EXIT( logger, "InPort::getPacket"  );
      return NULL;
    }

    DataTransferType *tmp=NULL;
    {
      SCOPED_LOCK lock(dataBufferLock);
      if ( (workQueue.size() == 0 ) or (( workQueue.size() != 0 ) and ( workQueue.size() == lastQueueSize )) ){

        if (timeout == 0.0) {
          lastQueueSize  = workQueue.size();
          TRACE_EXIT( logger, "InPort::getPacket"  );
          return NULL;
        } else if (timeout > 0){

          uint64_t secs = (unsigned long)(trunc(timeout));
          //uint64_t nsecs = (unsigned long)((timeout - secs) * 1e9);
          uint64_t msecs = (unsigned long)((timeout - secs) * 1e6);
          boost::system_time to_time  = boost::get_system_time() + boost::posix_time::seconds(secs) + boost::posix_time::microseconds(msecs);
          if ( dataAvailable.timed_wait( lock, to_time) == false ) {
            TRACE_EXIT( logger, "InPort::getPacket"  );
            return NULL;
          }

          if (breakBlock) {

            return NULL;
          }
        } else {
          dataAvailable.wait(lock);
          if (breakBlock) {
            TRACE_EXIT( logger, "InPort::getPacket"  );
            return NULL;
          }
        }
      }

      if ( streamID == "" ) {
        tmp = workQueue.front();
        workQueue.pop_front();
      }
      else {
        typename WorkQueue::iterator p = workQueue.begin();
        while ( p != workQueue.end() ) {
          if ( (*p)->streamID == streamID ) {
            tmp = *p;
            workQueue.erase(p);
            break;
          }
          p++;
        }
      }
      
      LOG_TRACE( logger, "bulkio.InPort getPacket PORT:" << name << " (QUEUE="<< workQueue.size() << ")" );

      if ( tmp == NULL ) {
        TRACE_EXIT( logger, "InPort::getPacket"  );
        lastQueueSize = workQueue.size();
        return NULL;
      }
    }

    bool turnOffBlocking = false;
    if (tmp && tmp->EOS) {
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
    lastQueueSize = 0;
    return tmp;
  }


  template < typename PortTraits >
  void   InPort< PortTraits >::setNewStreamListener( SriListener *newListener ) {
    newStreamCallback =  boost::shared_ptr< SriListener >(newListener, null_deleter());
  }

  template < typename PortTraits >
  void   InPort< PortTraits >::setNewStreamListener( SriListenerCallbackFn  newListener ) {
    newStreamCallback =  boost::make_shared< StaticSriCallback >( newListener );

  }

  template < typename PortTraits >
  void   InPort< PortTraits >::setLogger( LOGGER_PTR newLogger ) {
    logger = newLogger;
  }



  // ----------------------------------------------------------------------------------------
  //  Source Input Port String Definitions
  // ----------------------------------------------------------------------------------------
  template < typename PortTraits >
  InStringPort< PortTraits >::InStringPort(std::string port_name, 
                                           LOGGER_PTR  logger,
                                           bulkio::sri::Compare compareSri,
                                           SriListener *newStreamCB ) :
    Port_Provides_base_impl(port_name),
    sri_cmp(compareSri),
    newStreamCallback(),
    stats(NULL),
    logger(logger)
  {
    std::string _cmpMsg("USER_DEFINED");
    std::string _sriMsg("EMPTY");

    // create semaphore to track queue size
    queueSem = new queueSemaphore(100);

    // create stats object to track data ingest
    stats = new linkStatistics( port_name, sizeof(TransportType) );

    if ( newStreamCB ) {
      newStreamCallback = boost::shared_ptr< SriListener >( newStreamCB, null_deleter());
    }

    // set state to allow for back pressure to call if queue is full
    blocking = false;

    // controls the flow from getPacket... false will allow the flow
    breakBlock = false;

    if ( sri_cmp == NULL ) {
      sri_cmp = bulkio::sri::DefaultComparator;
    }

    LOG_DEBUG( logger, "bulkio::CTOR port:" << name << 
               " Blocking/MaxInputQueueSize " << blocking << "/" << queueSem->getMaxValue() <<  
               " SriCompare/NewStreamCallback " << _cmpMsg << "/" << _sriMsg );

  }


  template < typename PortTraits >
  InStringPort< PortTraits >::InStringPort(std::string port_name, 
                                           bulkio::sri::Compare compareSri,
                                           SriListener *newStreamCB ) :
    Port_Provides_base_impl(port_name),
    sri_cmp(compareSri),
    newStreamCallback(),
    stats(NULL)
  {
    // create semaphore to track queue size
    queueSem = new queueSemaphore(100);

    // create stats object to track data ingest
    stats = new linkStatistics( port_name, sizeof(TransportType) );

    if ( newStreamCB ) {
      newStreamCallback = boost::shared_ptr< SriListener >( newStreamCB, null_deleter());
    }

    // set state to allow for back pressure to call if queue is full
    blocking = false;

    // controls the flow from getPacket... false will allow the flow
    breakBlock = false;

    if ( sri_cmp == NULL ) {
      sri_cmp = bulkio::sri::DefaultComparator;
    }

  }

  template < typename PortTraits >
  InStringPort< PortTraits >::InStringPort(std::string port_name, void * ) :
    Port_Provides_base_impl(port_name),
    sri_cmp(bulkio::sri::DefaultComparator),
    newStreamCallback(),
    stats(NULL)
  {
    // create semaphore to track queue size
    queueSem = new queueSemaphore(100);

    // create stats object to track data ingest
    stats = new linkStatistics( port_name, sizeof(TransportType) );

    // set state to allow for back pressure to call if queue is full
    blocking = false;

    // controls the flow from getPacket... false will allow the flow
    breakBlock = false;

  }

  template < typename PortTraits >
  InStringPort< PortTraits >::~InStringPort()
  {
    TRACE_ENTER( logger, "InStringPort::DTOR"  );

    // block any data coming out of getPacket.. we should be ok at this point but just incase
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

    TRACE_EXIT( logger, "InStringPort::DTOR"  );
  }



  template < typename PortTraits >
  BULKIO::PortStatistics * InStringPort< PortTraits >::statistics()
  {
    SCOPED_LOCK lock(dataBufferLock);
    BULKIO::PortStatistics_var recStat = new BULKIO::PortStatistics(stats->retrieve());
    // NOTE: You must delete the object that this function returns!
    return recStat._retn();
  }


  template < typename PortTraits >
  BULKIO::PortUsageType InStringPort< PortTraits >::state()
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
  BULKIO::StreamSRISequence * InStringPort< PortTraits >::activeSRIs()
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
  int InStringPort< PortTraits >::getMaxQueueDepth()
  {
    SCOPED_LOCK lock(dataBufferLock);
    return queueSem->getMaxValue();
  }

  template < typename PortTraits >
  int  InStringPort< PortTraits >::getCurrentQueueDepth()
  {
    SCOPED_LOCK lock(dataBufferLock);
    return workQueue.size();
  }

  template < typename PortTraits >
  void InStringPort< PortTraits >::setMaxQueueDepth(int newDepth)
  {
    SCOPED_LOCK lock(dataBufferLock);
    queueSem->setMaxValue(newDepth);
  }

  template < typename PortTraits >
  void InStringPort< PortTraits >::pushSRI(const BULKIO::StreamSRI& H)
  {
    TRACE_ENTER( logger, "InStringPort::pushSRI"  );
    
    if (H.blocking) {
      SCOPED_LOCK lock(dataBufferLock);
      blocking = true;
      queueSem->setCurrValue(workQueue.size());
    }

    SCOPED_LOCK lock(sriUpdateLock);
    BULKIO::StreamSRI tmpH = H;
    SriMap::iterator currH = currentHs.find(std::string(H.streamID));
    if (currH == currentHs.end()) {
      if ( newStreamCallback ) (*newStreamCallback)(tmpH);
      LOG_DEBUG(logger,"pushSRI  PORT:" << name << " NEW SRI:" << H.streamID);
      currentHs[std::string(H.streamID)] = std::make_pair(tmpH, true);
    } else {
      if ( sri_cmp && !sri_cmp(tmpH, currH->second.first)) {
        currentHs[std::string(H.streamID)] = std::make_pair(tmpH, true);
      }
    }

    TRACE_EXIT( logger, "InStringPort::pushSRI"  );
  }

  template < typename PortTraits >
  void  InStringPort< PortTraits >::pushPacket(const char *data, const BULKIO::PrecisionUTCTime& T, CORBA::Boolean EOS, const char* streamID)
  {
    TRACE_ENTER( logger, "InStringPort::pushPacket"  );

    if (queueSem->getMaxValue() == 0) {
      TRACE_EXIT( logger, "InStringPort::pushPacket"  );
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
      }
      portBlocking = blocking;
    }


    bool flushToReport = false;
    if(portBlocking) {
      queueSem->incr();
      SCOPED_LOCK lock(dataBufferLock);
      LOG_TRACE( logger, "bulkio::InStringPort pushPacket NEW PACKET (QUEUE" << workQueue.size()+1 << ")" );
      stats->update( _getElementLength(data), (float)(workQueue.size()+1)/(float)queueSem->getMaxValue(), EOS, streamID, flushToReport);
      DataTransferType *tmpIn = new DataTransferType(data, T, EOS, streamID, tmpH, sriChanged, false);
      workQueue.push_back(tmpIn);
      dataAvailable.notify_all();
    } else {
      SCOPED_LOCK lock(dataBufferLock);
      bool sriChangedHappened = false;
      if (workQueue.size() == queueSem->getMaxValue()) { // reached maximum queue depth - flush the queue
        LOG_DEBUG( logger, "bulkio::InStringPort pushPacket PURGE INPUT QUEUE (SIZE" << workQueue.size() << ")" );
        flushToReport = true;
        DataTransferType *tmp;
        while (workQueue.size() != 0) {
          tmp = workQueue.front();
          if (tmp->sriChanged == true) {
            sriChangedHappened = true;
          }
          workQueue.pop_front();
          delete tmp;
        }
      }
      if (sriChangedHappened)
        sriChanged = true;
      LOG_DEBUG( logger, "bulkio::InStringPort pushPacket NEW Packet (QUEUE=" << workQueue.size()+1 << ")");
      stats->update( _getElementLength(data), (float)(workQueue.size()+1)/(float)queueSem->getMaxValue(), EOS, streamID, flushToReport);
      DataTransferType *tmpIn = new DataTransferType(data, T, EOS, streamID, tmpH, sriChanged, flushToReport);
      workQueue.push_back(tmpIn);
      dataAvailable.notify_all();
    }
    
    TRACE_EXIT( logger, "InStringPort::pushPacket"  );
  }


  template < typename PortTraits >
  void  InStringPort< PortTraits >::pushPacket(const char *data, CORBA::Boolean EOS, const char* streamID)
  {
    TRACE_ENTER( logger, "InStringPort::pushPacket"  );

    if (queueSem->getMaxValue() == 0) {
      TRACE_EXIT( logger, "InStringPort::pushPacket"  );
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
      }
      portBlocking = blocking;
    }


    bool flushToReport = false;
    if(portBlocking) {
      queueSem->incr();
      SCOPED_LOCK lock(dataBufferLock);
      LOG_TRACE( logger, "bulkio::InStringPort pushPacket NEW PACKET (QUEUE" << workQueue.size()+1 << ")" );
      stats->update( _getElementLength(data), (float)(workQueue.size()+1)/(float)queueSem->getMaxValue(), EOS, streamID, flushToReport);
      DataTransferType *tmpIn = new DataTransferType(data, EOS, streamID, tmpH, sriChanged, false);
      workQueue.push_back(tmpIn);
      dataAvailable.notify_all();
    } else {
      SCOPED_LOCK lock(dataBufferLock);
      bool sriChangedHappened = false;
      if (workQueue.size() == queueSem->getMaxValue()) { // reached maximum queue depth - flush the queue
        LOG_DEBUG( logger, "bulkio::InStringPort pushPacket PURGE INPUT QUEUE (SIZE" << workQueue.size() << ")" );
        flushToReport = true;
        DataTransferType *tmp;
        while (workQueue.size() != 0) {
          tmp = workQueue.front();
          if (tmp->sriChanged == true) {
            sriChangedHappened = true;
          }
          workQueue.pop_front();
          delete tmp;
        }
      }
      if (sriChangedHappened)
        sriChanged = true;

      LOG_DEBUG( logger, "bulkio::InStringPort pushPacket NEW Packet (QUEUE=" << workQueue.size()+1 << ")");
      stats->update( _getElementLength(data), (float)(workQueue.size()+1)/(float)queueSem->getMaxValue(), EOS, streamID, flushToReport);
      DataTransferType *tmpIn = new DataTransferType(data, EOS, streamID, tmpH, sriChanged, flushToReport);
      workQueue.push_back(tmpIn);
      dataAvailable.notify_all();
    }
    TRACE_EXIT( logger, "InStringPort::pushPacket"  );
  }


  template < typename PortTraits >
  void InStringPort< PortTraits >::enableStats( bool enable )
  {
    if (stats ) {
      stats->setEnabled(enable);
    }
  }



  template < typename PortTraits >
  void InStringPort< PortTraits >::block()
  {
    breakBlock = true;
    dataAvailable.notify_all();
  }

  template < typename PortTraits >
  void  InStringPort< PortTraits >::unblock()
  {    breakBlock = false;
  }

  template < typename PortTraits >
  void InStringPort< PortTraits >::stopPort()
  {
    block();
  }

  template < typename PortTraits >
  void  InStringPort< PortTraits >::startPort()
  {
    unblock();
  }


  template < typename PortTraits >
  bool  InStringPort< PortTraits >::blocked()
  {    return breakBlock;
  }



  template < typename PortTraits >
  int  InStringPort< PortTraits >::_getElementLength( const char *data )
  {
    int retval=0;
    if ( data )  retval=strlen(data);
    return retval;
  }





  /*
   * getPacket
   *     description: retrieve data from the provides (input) port
   *
   *  timeout: the amount of time to wait for data before a NULL is returned.
   *           Use 0.0 for non-blocking and -1 for blocking.
   */

  template < typename PortTraits >
  typename InStringPort< PortTraits >::DataTransferType * InStringPort< PortTraits >::getPacket(float timeout)
  {
    TRACE_ENTER( logger, "InStringPort::getPacket"  );

    if (breakBlock) {
      TRACE_EXIT( logger, "InStringPort::getPacket"  );
      return NULL;
    }

    DataTransferType *tmp = NULL;
    {
      SCOPED_LOCK lock(dataBufferLock);
      if (workQueue.size() == 0) {
        if (timeout == 0.0) {
          TRACE_EXIT( logger, "InStringPort::getPacket"  );
          return NULL;
        } else if (timeout > 0){

          uint64_t secs = (unsigned long)(trunc(timeout));
          //uint64_t nsecs = (unsigned long)((timeout - secs) * 1e9);
          uint64_t msecs = (unsigned long)((timeout - secs) * 1e6);
          boost::system_time to_time  = boost::get_system_time() + boost::posix_time::seconds(secs) + boost::posix_time::microseconds(msecs);
          if ( dataAvailable.timed_wait( lock, to_time) == false ) {
            TRACE_EXIT( logger, "InStringPort::getPacket"  );
            return NULL;
          }

          if (breakBlock) {
            TRACE_EXIT( logger, "InStringPort::getPacket"  );
            return NULL;
          }
        } else {
          dataAvailable.wait(lock);
          if (breakBlock) {
            TRACE_EXIT( logger, "InStringPort::getPacket"  );
            return NULL;
          }
        }
      }
      tmp = workQueue.front();
      workQueue.pop_front();
      LOG_TRACE( logger, "bulkio.InStringPort getPacket PORT:" << name << " (QUEUE="<< workQueue.size() << ")" );
    }


    bool turnOffBlocking = false;
    if (tmp && tmp->EOS) {
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

    TRACE_EXIT( logger, "InStringPort::getPacket"  );
    return tmp;
  }



  /*
   * getPacket
   *     description: retrieve data from the provides (input) port
   *
   *  timeout: the amount of time to wait for data before a NULL is returned.
   *           Use 0.0 for non-blocking and -1 for blocking.
   */

  template < typename PortTraits >
  typename InStringPort< PortTraits >::DataTransferType * InStringPort< PortTraits >::getPacket(float timeout, std::string &streamID )
  {
    TRACE_ENTER( logger, "InStringPort::getPacket"  );

    if (breakBlock) {
      TRACE_EXIT( logger, "InStringPort::getPacket"  );
      return NULL;
    }

    DataTransferType *tmp=NULL;
    {
      SCOPED_LOCK lock(dataBufferLock);
      if ( (workQueue.size() == 0 ) or (( workQueue.size() != 0 ) and 
                                        ( workQueue.size() == lastQueueSize )) ){

        if (timeout == 0.0) {
          lastQueueSize  = workQueue.size();
          TRACE_EXIT( logger, "InStringPort::getPacket"  );
          return NULL;
        } else if (timeout > 0){

          uint64_t secs = (unsigned long)(trunc(timeout));
          //uint64_t nsecs = (unsigned long)((timeout - secs) * 1e9);
          uint64_t msecs = (unsigned long)((timeout - secs) * 1e6);
          boost::system_time to_time  = boost::get_system_time() + boost::posix_time::seconds(secs) + boost::posix_time::microseconds(msecs);
          if ( dataAvailable.timed_wait( lock, to_time) == false ) {
            TRACE_EXIT( logger, "InStringPort::getPacket"  );
            return NULL;
          }
          if (breakBlock) {
            TRACE_EXIT( logger, "InStringPort::getPacket"  );
            return NULL;
          }
        } else {
          dataAvailable.wait(lock);
          if (breakBlock) {
            TRACE_EXIT( logger, "InStringPort::getPacket"  );
            return NULL;
          }
        }
      }


      if ( streamID == "" ) {
        tmp = workQueue.front();
        workQueue.pop_front();
      }
      else {
        typename WorkQueue::iterator p = workQueue.begin();
        while ( p != workQueue.end() ) {
          if ( (*p)->streamID == streamID ) {
            tmp = *p;
            workQueue.erase(p);
            break;
          }
          p++;
        }
      }

      LOG_TRACE( logger, "bulkio.InStringPort getPacket PORT:" << name << " (QUEUE="<< workQueue.size() << ")" );
      if ( tmp == NULL ) {
        lastQueueSize = workQueue.size();
        TRACE_EXIT( logger, "InStringPort::getPacket"  );
        return NULL;
      }
    }

    bool turnOffBlocking = false;
    if (tmp && tmp->EOS) {
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

    TRACE_EXIT( logger, "InStringPort::getPacket"  );
    lastQueueSize = 0;
    return tmp;
  }

  template < typename PortTraits >
  void   InStringPort< PortTraits >::setNewStreamListener( SriListener *newListener ) {
    newStreamCallback =  boost::shared_ptr< SriListener >(newListener, null_deleter());
  }

  template < typename PortTraits >
  void   InStringPort< PortTraits >::setNewStreamListener( SriListenerCallbackFn  newListener ) {
    newStreamCallback =  boost::make_shared< StaticSriCallback >( newListener );

  }


  template <>
  int  InStringPort< XMLPortTraits  >::_getElementLength( const char *data )
  {
    int retval=1;
    return retval;
  }


  template < typename PortTraits >
  void   InStringPort< PortTraits >::setLogger( LOGGER_PTR newLogger ) {
    logger = newLogger;
  }



  //
  // Required for Template Instantion for the compilation unit.
  // Note: we only define those valid types for which Bulkio IDL is defined. Users wanting to
  // inherit this functionality will be unable to since they cannot instantiate and
  // link against the template.
  //

  template class InPort< CharPortTraits >;
  template class InPort< OctetPortTraits >;
  template class InPort< ShortPortTraits >;
  template class InPort< UShortPortTraits >;
  template class InPort< LongPortTraits >;
  template class InPort< ULongPortTraits >;
  template class InPort< LongLongPortTraits >;
  template class InPort< ULongLongPortTraits >;
  template class InPort< FloatPortTraits >;
  template class InPort< DoublePortTraits >;
  template class InStringPort< FilePortTraits >; 
  template class InStringPort< XMLPortTraits >;


} // end of bulkio namespace
