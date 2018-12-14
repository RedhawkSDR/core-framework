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
#include "bulkio_p.h"
#include "bulkio_traits.h"

namespace  bulkio {

  queueSemaphore::queueSemaphore(unsigned int initialMaxValue):
    mutex(),
    condition() 
  {
    currValue=0;   
    maxValue = initialMaxValue;
  }

  void queueSemaphore::release() {
    currValue=0;
    condition.notify_all();
  }

  void queueSemaphore::setMaxValue(unsigned int newMaxValue) {
    UNIQUE_LOCK lock(mutex);
    maxValue = newMaxValue;
  }

  unsigned int queueSemaphore::getMaxValue(void) {
    UNIQUE_LOCK lock(mutex);
    return maxValue;
  }

  void queueSemaphore::setCurrValue(unsigned int newValue) {
    UNIQUE_LOCK lock(mutex);
    if (newValue < maxValue) {
      unsigned int oldValue = currValue;
      currValue = newValue;

      if (oldValue > newValue) {
        condition.notify_one();
      }
    }
  }

  void queueSemaphore::incr() {
    UNIQUE_LOCK lock(mutex);
    while (currValue >= maxValue) {
      condition.wait(lock);
    }
    ++currValue;
  }

  void queueSemaphore::decr() {
    UNIQUE_LOCK lock(mutex);
    if (currValue > 0) {
      --currValue;
    }
    condition.notify_one();
  }


  linkStatistics::linkStatistics( ):
    portName(""),
    nbytes(1)
  {
    enabled = true;
    bitSize = nbytes * 8.0;
    historyWindow = 10;
    activeStreamIDs.resize(0);
    receivedStatistics_idx = 0;
    receivedStatistics.resize(historyWindow);
    runningStats.elementsPerSecond = -1.0;
    runningStats.bitsPerSecond = -1.0;
    runningStats.callsPerSecond = -1.0;
    runningStats.averageQueueDepth = -1.0;
    runningStats.streamIDs.length(0);
    runningStats.timeSinceLastCall = -1;
    flush_sec = 0;
    flush_usec = 0;
    connection_errors=0;
  }


  linkStatistics::linkStatistics(const std::string &portName, const int nbytes):
    portName(portName),
    nbytes(nbytes)
  {
    enabled = true;
    bitSize = nbytes * 8.0;
    historyWindow = 10;
    activeStreamIDs.resize(0);
    receivedStatistics_idx = 0;
    receivedStatistics.resize(historyWindow);
    runningStats.elementsPerSecond = -1.0;
    runningStats.bitsPerSecond = -1.0;
    runningStats.callsPerSecond = -1.0;
    runningStats.averageQueueDepth = -1.0;
    runningStats.streamIDs.length(0);
    runningStats.timeSinceLastCall = -1;
    flush_sec = 0;
    flush_usec = 0;
    connection_errors=0;
  }


  void linkStatistics::setEnabled(bool enableStats) {
    enabled = enableStats;
  }

  void linkStatistics::setBitSize(double inBitSize) {
    bitSize = inBitSize;
  }

  uint64_t linkStatistics::connectionErrors( const uint64_t n) {
      connection_errors += n;
      return connection_errors;
  }

  void linkStatistics::update(unsigned int elementsReceived, float queueSize, bool EOS, const std::string &streamID, bool flush ) {

    // reset error counter;
    connection_errors=0;

    if (!enabled) {
      return;
    }
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);
#if 0
    std::cout << "linkStats::update name " << portName << " i:" <<  receivedStatistics_idx << 
	" elements:" << elementsReceived << 
	" qs:" << queueSize << 
	" eos:" << EOS << std::endl;
#endif
    receivedStatistics[receivedStatistics_idx].elements = elementsReceived;
    receivedStatistics[receivedStatistics_idx].queueSize = queueSize;
    receivedStatistics[receivedStatistics_idx].secs = tv.tv_sec;
    receivedStatistics[receivedStatistics_idx++].usecs = tv.tv_usec;
    receivedStatistics_idx = receivedStatistics_idx % historyWindow;
    if (flush) {
      this->flush_sec = tv.tv_sec;
      this->flush_usec = tv.tv_usec;
    }
    if (!EOS) {
      StreamIDList::iterator p = activeStreamIDs.begin();
      bool foundStreamID = false;
      while (p != activeStreamIDs.end()) {
        if (*p == streamID) {
          foundStreamID = true;
          break;
        }
        p++;
      }
      if (!foundStreamID) {
        activeStreamIDs.push_back(streamID);
      }
    } else {
      StreamIDList::iterator p = activeStreamIDs.begin();
      while (p != activeStreamIDs.end()) {
        if (*p == streamID) {
          activeStreamIDs.erase(p);
          break;
        }
        p++;
      }
    }
  }

  BULKIO::PortStatistics linkStatistics::retrieve() {
    if (!enabled) {
      return runningStats;
    }
    struct timeval tv;
    struct timezone tz;
    gettimeofday(&tv, &tz);

    int idx = (receivedStatistics_idx == 0) ? (historyWindow - 1) : (receivedStatistics_idx - 1);
    double front_sec = receivedStatistics[idx].secs;
    double front_usec = receivedStatistics[idx].usecs;
    double secDiff = tv.tv_sec - receivedStatistics[receivedStatistics_idx].secs;
    double usecDiff = (tv.tv_usec - receivedStatistics[receivedStatistics_idx].usecs) / ((double)1e6);

    double totalTime = secDiff + usecDiff;
    double totalData = 0;
    float queueSize = 0;
    int startIdx = (receivedStatistics_idx + 1) % historyWindow;
    int aggregateSum = 0;
    for (int i = startIdx; i != receivedStatistics_idx; ) {
      totalData += receivedStatistics[i].elements;
      queueSize += receivedStatistics[i].queueSize;
      i = (i + 1) % historyWindow;
      aggregateSum++;
    }
#if 0    
    std::cout << "linkStats::retrieve  name :" << portName << 
      " total data:" <<  totalData << 
      " aggreateSum:" << aggregateSum <<
      " bitsize:" << bitSize << 
      " totalTime:" << totalTime << std::endl;
#endif
    runningStats.portName = CORBA::string_dup(portName.c_str());
    runningStats.bitsPerSecond = ((totalData * bitSize) / totalTime);
    runningStats.elementsPerSecond = (totalData / totalTime);
    runningStats.averageQueueDepth = (queueSize / aggregateSum);
    runningStats.callsPerSecond = (double(historyWindow - 1) / totalTime);
    runningStats.timeSinceLastCall = (((double)tv.tv_sec) - front_sec) + (((double)tv.tv_usec - front_usec) / ((double)1e6));
    unsigned int streamIDsize = activeStreamIDs.size();
    StreamIDList::iterator p = activeStreamIDs.begin();
    runningStats.streamIDs.length(streamIDsize);
    for (unsigned int i = 0; i < streamIDsize; i++) {
      if (p == activeStreamIDs.end()) {
        break;
      }
      runningStats.streamIDs[i] = CORBA::string_dup((*p).c_str());
      p++;
    }

    if ((this->flush_sec != 0) && (this->flush_usec != 0)) {
      double flushTotalTime = (((double)tv.tv_sec) - this->flush_sec) + (((double)tv.tv_usec - this->flush_usec) / ((double)1e6));
      this->runningStats.keywords.length(1);
      this->runningStats.keywords[0].id = CORBA::string_dup("timeSinceLastFlush");
      this->runningStats.keywords[0].value <<= CORBA::Double(flushTotalTime);
    }

    return runningStats;
  }

} // end of bulkio namespace
