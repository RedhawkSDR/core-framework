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
import collections
import struct

from bulkio.bulkioInterfaces import BULKIO
from redhawk.burstioInterfaces import BURSTIO__POA

import statistics
import traits

__all__ = ('BurstByteIn', 'BurstUbyteIn',
           'BurstShortIn', 'BurstUshortIn',
           'BurstLongIn', 'BurstUlongIn',
           'BurstLongLongIn', 'BurstUlongLongIn',
           'BurstFloatIn', 'BurstDoubleIn')

class BurstPacket(object):
    def __init__(self, burst, blockOccurred, traits):
        self._eos = burst.EOS
        self._sri = burst.SRI
        self._time = burst.T
        self._data = burst.data
        self._blockOccurred = blockOccurred
        self._traits = traits

    def getStreamID(self):
        return self._sri.streamID

    def getSize(self):
        return len(self._data)

    def getData(self):
        return self._traits.toArray(self._data)

    def isComplex(self):
        return (self._sri.mode == 1)

    def getComplexData(self):
        return self._traits.toComplexArray(self._data)

    def getEOS(self):
        return self._eos

    def getTime(self):
        return self._time

    def getSRI(self):
        return self._sri

    def blockOccurred(self):
        return self._blockOccurred

    def getSequence(self):
        return self._data


class InPort(object):
    DEFAULT_QUEUE_THRESHOLD = 100

    def __init__(self, name, traits):
        self._name = name
        self._traits = traits
        self._queue = collections.deque()
        self._queueLock = threading.Lock()
        self._queueNotFull = threading.Condition(self._queueLock)
        self._queueNotEmpty = threading.Condition(self._queueLock)
        self._started = False
        self._queueThreshold = self.DEFAULT_QUEUE_THRESHOLD
        self._blockOccurred = False
        self._streamIDs = set()

        self._statistics = statistics.ReceiverStatistics(self._name, traits.size())

    def _get_state(self):
        self._queueLock.acquire()
        try:
            if not self._queue:
                return BULKIO.IDLE
            elif len(self._queue) < self._queueThreshold:
                return BULKIO.ACTIVE
            else:
                return BULKIO.BUSY
        finally:
            self._queueLock.release()

    def _get_statistics(self):
        self._queueLock.acquire()
        try:
            stats = self._statistics.retrieve()
            stats.streamIDs = list(self._streamIDs)
            return stats
        finally:
            self._queueLock.release()

    def getQueueThreshold(self):
        return self._queueThreshold

    def setQueueThreshold(self, count):
        self._queueLock.acquire()
        try:
            if count > self._queueThreshold:
                self._queueNotFull.notifyAll()
            self._queueThreshold = count
        finally:
            self._queueLock.release()

    def getQueueDepth(self):
        self._queueLock.acquire()
        try:
            return len(self._queue)
        finally:
            self._queueLock.release()

    def flush(self):
        self._queueLock.acquire()
        try:
            self._statistics.flushOccurred(len(self._queue))
            self._queue = collections.deque()
            self._queueNotFull.notifyAll()
        finally:
            self._queueLock.release()
        

    def start(self):
        self._queueLock.acquire()
        try:
            self._started = True
        finally:
            self._queueLock.release()

    def stop(self):
        self._queueLock.acquire()
        try:
            if not self._started:
                return
            self._started = False
            self._queueNotEmpty.notifyAll()
            self._queueNotFull.notifyAll()
        finally:
            self._queueLock.release()

    def pushBursts(self, bursts):
        begin = time.time()

        self._queueLock.acquire()
        try:
            # Calculate queue depth based on state at invocation; this makes it
            # easy to tell if a consumer is keeping up (average = 0, or at
            # least doesn't grow) or blocking (average >= 100)
            queue_depth = len(self._queue) / float(self._queueThreshold)

            # Only set the block flag once to avoid multiple notifications
            block_reported = False

            # Wait until the queue is below the blocking threshold
            while self._started and len(self._queue) >= self._queueThreshold:
                # Report that this call blocked
                if not block_reported:
                    self._blockOccurred = True
                    block_reported = True
                self._queueNotFull.wait()

            # Discard bursts if processing is not started
            if not self._started:
                return

            # Count total elements
            total_bursts = len(bursts)
            total_elements = 0
            for burst in bursts:
                total_elements += len(burst.data)
                self._streamIDs.add(burst.SRI.streamID)

            # Add bursts to queue
            self._queue.extend(bursts)
            self._queueNotEmpty.notifyAll()

            # Record total time spent in pushBursts for latency measurement
            elapsed = time.time() - begin
            self._statistics.record(total_bursts, total_elements, queue_depth, elapsed)
        finally:
            self._queueLock.release()

    def getBurst(self, timeout=-1.0):
        self._queueLock.acquire()
        try:
            if not self._waitBurst(timeout):
                return None

            burst = self._queue.popleft()
            if burst.EOS:
                self._streamIDs.discard(burst.SRI.streamID)
            block_occurred = self._blockOccurred
            self._blockOccurred = False
            if len(self._queue) < self._queueThreshold:
                self._queueNotFull.notifyAll()
        finally:
            self._queueLock.release()
        return BurstPacket(burst, block_occurred, self._traits)

    def getBursts(self, timeout=-1.0):
        self._queueLock.acquire()
        try:
            if not self._waitBurst(timeout):
                return []
            bursts = self._queue
            self._queue = collections.deque()
            self._queueNotFull.notifyAll()
        finally:
            self._queueLock.release()
        return bursts

    def blockOccurred(self):
        self._queueLock.acquire()
        try:
            block_occurred = self._blockOccurred
            self._blockOccurred = False
            return block_occurred
        finally:
            self._queueLock.release()

    def _waitBurst(self, timeout):
        # Determine the absolute time at which the timeout expires (ignored if
        # timeout is negative)
        end = time.time() + timeout
        while self._started and not self._queue:
            if timeout < 0.0:
                self._queueNotEmpty.wait()
            else:
                # Calculate remaining timeout
                remain = end - time.time()
                if remain <= 0.0:
                    break
                self._queueNotEmpty.wait(remain)
        return len(self._queue) > 0

class BurstByteIn(InPort, BURSTIO__POA.burstByte):
    def __init__(self, name):
        InPort.__init__(self, name, traits.ByteTraits)

class BurstDoubleIn(InPort, BURSTIO__POA.burstDouble):
    def __init__(self, name):
        InPort.__init__(self, name, traits.DoubleTraits)

class BurstFloatIn(InPort, BURSTIO__POA.burstFloat):
    def __init__(self, name):
        InPort.__init__(self, name, traits.FloatTraits)

class BurstLongIn(InPort, BURSTIO__POA.burstLong):
    def __init__(self, name):
        InPort.__init__(self, name, traits.LongTraits)

class BurstLongLongIn(InPort, BURSTIO__POA.burstLongLong):
    def __init__(self, name):
        InPort.__init__(self, name, traits.LongLongTraits)

class BurstShortIn(InPort, BURSTIO__POA.burstShort):
    def __init__(self, name):
        InPort.__init__(self, name, traits.ShortTraits)

class BurstUbyteIn(InPort, BURSTIO__POA.burstUbyte):
    def __init__(self, name):
        InPort.__init__(self, name, traits.UbyteTraits)

class BurstUlongIn(InPort, BURSTIO__POA.burstUlong):
    def __init__(self, name):
        InPort.__init__(self, name, traits.UlongTraits)

class BurstUlongLongIn(InPort, BURSTIO__POA.burstUlongLong):
    def __init__(self, name):
        InPort.__init__(self, name, traits.UlongLongTraits)

class BurstUshortIn(InPort, BURSTIO__POA.burstUshort):
    def __init__(self, name):
        InPort.__init__(self, name, traits.UshortTraits)
