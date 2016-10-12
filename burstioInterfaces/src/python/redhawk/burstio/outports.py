#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK burstioInterfaces.
#
# REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import threading
import time
import numpy
import struct

from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA
from omniORB import CORBA

from ossie.utils.log4py import logging

import utils
import traits
from statistics import BurstStatistics
from usesport import UsesPort
from executor import ExecutorService

__all__ = ('ROUTE_ALL_INTERLEAVED', 'ROUTE_ALL_STREAMS', 'ROUTE_CONNECTION_STREAMS',
           'BurstByteOut', 'BurstUbyteOut',
           'BurstShortOut', 'BurstUshortOut',
           'BurstLongOut', 'BurstUlongOut',
           'BurstLongLongOut', 'BurstUlongLongOut',
           'BurstFloatOut', 'BurstDoubleOut')

ROUTE_ALL_INTERLEAVED = 0
ROUTE_ALL_STREAMS = 1
ROUTE_CONNECTION_STREAMS = 2

class Queue(object):
    def __init__(self, port, maxBursts, thresholdBytes, thresholdLatency, streamID=None):
        self._port = port
        self._streamID = streamID

        # Policy settings
        self._maxBursts = maxBursts
        self._thresholdBytes = thresholdBytes
        self._thresholdLatency = utils.usec_to_sec(thresholdLatency)

        # Queue management
        self._queue = []
        self._queueMutex = threading.Lock()
        self._queuedBytes = 0
        self._startTime = None

    def getMaxBursts(self):
        self._queueMutex.acquire()
        try:
            return self._maxBursts
        finally:
            self._queueMutex.release()

    def setMaxBursts(self, bursts):
        self._queueMutex.acquire()
        try:
            self._maxBursts = bursts
            if self._burstsExceeded():
                # Wake up the monitor thread to do a push
                self._log.debug('New max bursts %s triggering push', bursts);
                self._executeThreadedFlush()
        finally:
            self._queueMutex.release()
    
    def getByteThreshold(self):
        self._queueMutex.acquire()
        try:
            return self._thresholdBytes
        finally:
            self._queueMutex.release()

    def setByteThreshold(self, bytes):
        self._queueMutex.acquire()
        try:
            self._thresholdBytes = bytes
            if self._bytesExceeded():
                self._log.debug('New byte threshold %s triggering push', bytes);
                # Wake up the monitor thread to do a push
                self._executeThreadedFlush()
        finally:
            self._queueMutex.release()
    
    def getLatencyThreshold(self):
        self._queueMutex.acquire()
        try:
            return int(utils.sec_to_usec(self._thresholdLatency))
        finally:
            self._queueMutex.release()

    def setLatencyThreshold(self, usec):
        self._queueMutex.acquire()
        try:
            self._thresholdLatency = utils.usec_to_sec(usec)
            if self._latencyExceeded():
                # Wake up the monitor thread to do a push
                self._executeThreadedFlush()
            elif self._queue:
                # Schedule a check at the new latency threshold
                self._port._scheduleCheck(self._startTime + self._thresholdLatency)
        finally:
            self._queueMutex.release()

    def flush(self):
        self._queueMutex.acquire()
        try:
            self._flushQueue()
        finally:
            self._queueMutex.release()

    def _queueBurst(self, burst):
        self._queueMutex.acquire()
        try:
            # If this is the first burst, mark the time for latency guarantees
            if self._startTime is None:
                self._startTime = time.time()
                # Wake the monitor thread so it can set its timeout
                self._port._log.trace('Waking monitor thread on first queued burst')
                self._port._scheduleCheck(self._startTime + self._thresholdLatency)

            # Add the burst to the queue, tracking the total number of data
            # bytes queued
            self._queue.append(burst)
            self._queuedBytes += len(burst.data) * self._port._bytesPerElement

            # NB: Whether it is executed or not, this trace statement has a
            #     significant impact on performance
            #self._log.trace('Queue size: %s bursts / %s bytes', len(self._queue), self._queuedBytes)

            # Check whether the max bursts or byte threshold has been exceeded
            # NB: Function calls incur significant overhead in Python, so the
            #     relevant checks are inlined versus calling a method,
            if len(self._queue) >= self._maxBursts or self._queuedBytes >= self._thresholdBytes:
                self._port._log.debug('Queued burst exceeded threshold, flushing queue')
                self._flushQueue()
        finally:
            self._queueMutex.release()

    def _burstsExceeded(self):
        return len(self._queue) >= self._maxBursts

    def _bytesExceeded(self):
        return self._queuedBytes >= self._thresholdBytes

    def _elapsed(self):
        if self._startTime:
            return time.time() - self._startTime
        else:
            return None

    def _latencyExceeded(self):
        return self._elapsed() > self._thresholdLatency
    
    def _shouldFlush(self):
        return self._burstsExceeded() or self._bytesExceeded() or self._latencyExceeded()

    def _flushQueue(self):
        if not self._queue:
            return
        self._port._sendBursts(self._queue, self._startTime, len(self._queue)/self._maxBursts, self._streamID)
        self._queue = []
        self._queuedBytes = 0
        self._startTime = None

    def _executeThreadedFlush(self):
        self._port._monitor.execute(self.flush)

    def _checkFlush(self):
        self._queueMutex.acquire()
        try:
            if self._shouldFlush():
                self._flushQueue()
        finally:
            self._queueMutex.release()


class OutPort(UsesPort, BULKIO__POA.UsesPortStatisticsProvider):

    DEFAULT_MAX_BURSTS = 100
    DEFAULT_LATENCY_THRESHOLD = 10000 # 10000us = 10ms
    
    def __init__(self, name, traits):
        UsesPort.__init__(self, name, traits.PortType)

        # Type metadata
        self.BurstType = traits.BurstType
        self._bytesPerElement = traits.size()

        # Logging; default logger is the class name
        self._log = logging.getLogger(self.__class__.__name__)

        # Perform latency monitoring and deferred pushes on a separate thread
        self._monitor = ExecutorService()

        # Queue management
        self._queueMutex = threading.Lock()
        self._defaultQueue = Queue(self, self.DEFAULT_MAX_BURSTS, 0.9*2*(1024**2), self.DEFAULT_LATENCY_THRESHOLD)
        self._streamQueues = {}

        # Default to all connections receiving all streams interleaved
        self._routingMode = ROUTE_ALL_INTERLEAVED
        self._routes = {}

    def start(self):
        self._monitor.start()

    def stop(self):
        # Stop the monitor thread and remove any queued checks, as the queue(s)
        # will be flushed anyway
        self._monitor.stop()
        self._monitor.clear()

        self.flush()
    
    # Provide standard interface for start/stop
    startPort = start
    stopPort = stop

    def getMaxBursts(self):
        return self.getDefaultPolicy().getMaxBursts()

    def setMaxBursts(self, bursts):
        self.getDefaultPolicy().setMaxBursts(bursts)
    
    def getByteThreshold(self):
        return self.getDefaultPolicy().getByteThreshold()

    def setByteThreshold(self, bytes):
        self.getDefaultPolicy().setByteThreshold(bytes)
    
    def getLatencyThreshold(self):
        return self.getDefaultPolicy().getLatencyThreshold()

    def setLatencyThreshold(self, usec):
        self.getDefaultPolicy().setLatencyThreshold(usec)

    def getDefaultPolicy(self):
        return self._defaultQueue

    def getStreamPolicy(self, streamID):
        self._queueMutex.acquire()
        try:
            return self._getQueueForStream(streamID)
        finally:
            self._queueMutex.release()

    def setRoutingMode(self, mode):
        self._routingMode = mode

    def setLogger(self, logger):
        self._log = logger

    def updateConnectionFilter(self, filterTable):
        new_routes = {}
        for route in filterTable:
            if route.port_name != self._name:
                continue
            if not route.stream_id in self._routes:
                new_routes[route.stream_id] = set()
            new_routes[route.stream_id].add(route.connection_id)

        self._connectionMutex.acquire()
        try:
            self._routes = new_routes
        finally:
            self._connectionMutex.release()

    def addConnectionFilter(self, streamID, connectionID):
        self._connectionMutex.acquire()
        try:
            if not streamID in self._routes:
                self._routes[streamID] = set()
            self._routes[streamID].add(connectionID)
        finally:
            self._connectionMutex.release()

    def removeConnectionFilter(self, streamID, connectionID):
        self._connectionMutex.acquire()
        try:
            if streamID in self._routes:
                self._routes[streamID].discard(connectionID)
        finally:
            self._connectionMutex.release()

    def state(self):
        self._connectionMutex.acquire()
        try:
            if self._connections:
                return BULKIO.ACTIVE
            else:
                return BULKIO.IDLE
        finally:
            self._connectionMutex.release()

    def pushBurst(self, data, sri, timestamp=None, eos=False):
        # Ensure data is in the correct format
        data = self._formatData(data)
        if timestamp is None:
            timestamp = utils.now()
        burst = self.BurstType(sri, data, timestamp, eos)

        self._queueMutex.acquire()
        try:
            queue = self._getQueueForStream(sri.streamID)
            queue._queueBurst(burst)
            if eos:
                if not self._isInterleaved():
                    self._log.debug("Flushing '%s' on EOS", sri.streamID)
                    queue.flush()
                del self._streamQueues[sri.streamID]
        finally:
            self._queueMutex.release()

    def pushBursts(self, bursts):
        self._sendBursts(bursts, time.time(), 0.0)

    def flush(self):
        self._queueMutex.acquire()
        try:
            for queue in self._getAllQueues():
                queue.flush()
        finally:
            self._queueMutex.release()

    def _sendBursts(self, bursts, startTime, queueDepth, streamID=None):
        self._log.debug('Pushing %d bursts', len(bursts))

        # Count up total elements
        total_elements = sum(len(burst.data) for burst in bursts)

        self._connectionMutex.acquire()
        try:
            for connectionId, connection in self._connections.iteritems():
                # Check stream routing
                if not self._isStreamRoutedToConnection(streamID, connectionId):
                    continue

                # Record statistics
                delay = time.time() - startTime
                try:
                    connection.port.pushBursts(bursts)
                    connection.alive = True
                    connection.stats.record(len(bursts), total_elements, queueDepth, delay)
                except CORBA.MARSHAL, e:
                    if len(bursts) == 1:
                        if connection.alive:
                            self._log.error('pushBursts to %s failed because the burst size is too long')
                        connection.alive = False
                    else:
                        self._partitionBursts(bursts, startTime, queueDepth, connection)
                except Exception, e:
                    if connection.alive:
                        self._log.error('pushBursts to %s failed: %s', connectionId, e)
                    connection.alive = False
                except:
                    if connection.alive:
                        self._log.error('pushBursts to %s failed', connectionId)
                    connection.alive = False
        finally:
            self._connectionMutex.release()  

    def _partitionBursts(self, bursts, startTime, queueDepth, connection):
        first_burst = bursts[:len(bursts)/2]
        second_burst = bursts[len(first_burst):]
        delay = time.time() - startTime
        first_total_elements = sum(len(burst.data) for burst in first_burst)
        second_total_elements = sum(len(burst.data) for burst in second_burst)
        try:
            connection.port.pushBursts(first_burst)
            connection.alive = True
            connection.stats.record(len(first_burst), first_total_elements, queueDepth, delay)
        except CORBA.MARSHAL, e:
            self._partitionBursts(first_burst, startTime, queueDepth, connection)
        try:
            connection.port.pushBursts(second_burst)
            connection.alive = True
            connection.stats.record(len(second_burst), second_total_elements, queueDepth, delay)
        except CORBA.MARSHAL, e:
            self._partitionBursts(second_burst, startTime, queueDepth, connection)
        
    def _get_statistics(self):
        self._connectionMutex.acquire()
        try:
            return [self._retrieveStats(connectionId, connection)
                    for connectionId, connection in self._connections.iteritems()]
        finally:
            self._connectionMutex.release()

    def _retrieveStats(self, connectionId, connection):
        stats = connection.stats.retrieve()
        stats.streamIDs = [stream_id for stream_id in self._streamQueues.keys()
                           if self._isStreamRoutedToConnection(stream_id, connectionId)]
        return BULKIO.UsesPortStatistics(connectionId, stats)

    def _connectionAdded(self, connectionId, connection):
        connection.alive = True
        connection.stats = BurstStatistics(self._name, self._bytesPerElement)

    def _connectionModified(self, connectionId, connection):
        # Assume that the updated connection is alive
        connection.alive = True
    
    def _scheduleCheck(self, when):
        self._monitor.schedule(when, self._checkQueues)

    def _checkQueues(self):
        self._queueMutex.acquire()
        try:
            for queue in self._getAllQueues():
                queue._checkFlush()
        finally:
            self._queueMutex.release()
        
    def _formatData(self, data):
        if isinstance(data, numpy.ndarray):
            data = data.tolist()
        elif not isinstance(data, list):
            data = list(data)
        return data

    def _isInterleaved(self):
        return ROUTE_ALL_INTERLEAVED == self._routingMode

    def _getQueueForStream(self, streamID):
        if streamID in self._streamQueues:
            return self._streamQueues[streamID]
        elif self._isInterleaved():
            queue = self._defaultQueue
        else:
            self._log.trace('Creating new queue for stream %s', streamID)
            # Propogate the default queue's settings
            max_bursts = self._defaultQueue.getMaxBursts()
            byte_threshold = self._defaultQueue.getByteThreshold()
            latency_threshold = self._defaultQueue.getLatencyThreshold()
            queue = Queue(self, max_bursts, byte_threshold, latency_threshold, streamID=streamID)
        # Insert the new stream (with potentially a new queue) into the mapping
        self._streamQueues[streamID] = queue
        return queue

    def _getAllQueues(self):
        if self._isInterleaved():
            return [self._defaultQueue]
        else:
            return self._streamQueues.values()

    def _isStreamRoutedToConnection(self, streamID, connectionID):
        if ROUTE_CONNECTION_STREAMS != self._routingMode:
            return True
        return connectionID in self._routes.get(streamID, set())


class BurstByteOut(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, traits.ByteTraits)

    def _formatData(self, data):
        if isinstance(data, numpy.ndarray):
            if data.dtype != numpy.int8:
                data = data.astype(numpy.int8)
            data = data.tostring()
        elif not isinstance(data, str):
            # Use struct to pack as a series of len(data) bytes; this is about
            # as efficient as we can get, and enforces byte range
            packing = '%db' % len(data)
            data = struct.pack(packing, *data)
        return data

class BurstDoubleOut(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, traits.DoubleTraits)

class BurstFloatOut(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, traits.FloatTraits)

class BurstLongOut(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, traits.LongTraits)

class BurstLongLongOut(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, traits.LongLongTraits)

class BurstShortOut(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, traits.ShortTraits)

class BurstUbyteOut(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, traits.UbyteTraits)

    def _formatData(self, data):
        if isinstance(data, numpy.ndarray):
            if data.dtype != numpy.int8:
                data = data.astype(numpy.int8)
            data = data.tostring()
        elif not isinstance(data, str):
            # Use struct to pack as a series of len(data) unsigned bytes; this
            # is about as efficient as we can get, and enforces unsigned byte
            # range
            packing = '%dB' % len(data)
            data = struct.pack(packing, *data)
        return data

class BurstUlongOut(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, traits.UlongTraits)

class BurstUlongLongOut(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, traits.UlongLongTraits)

class BurstUshortOut(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, traits.UshortTraits)
