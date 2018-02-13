#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import time
import struct
from ossie.cf import CF
from omniORB import CORBA
from bulkio.bulkioInterfaces.BULKIO import PortStatistics
from bulkio.bulkioInterfaces.BULKIO import UsesPortStatistics

class InStats:
    class statPoint:
        def __init__(self):
            self.elements = 0
            self.queueSize = 0.0
            self.secs = 0.0

    def __init__(self, name, element_type='', bits=0):
        # Backwards-compatibility: accept an element type string for use with
        # struct.calcsize
        if bits == 0:
            bits = struct.calcsize(element_type) * 8
        self.enabled = True
        self.flushTime = None
        self.historyWindow = 10
        self.receivedStatistics = []
        self.name = name
        self.receivedStatistics_idx = 0
        self.bitSize = bits
        self.activeStreamIDs = []
        for i in range(self.historyWindow):
            self.receivedStatistics.append(self.statPoint())
        self.runningStats = None

    def setEnabled(self, enableStats):
        self.enabled = enableStats

    def setBitSize(self, bitSize ):
        self.bitSize = bitSize

    def update(self, elementsReceived, queueSize, EOS, streamID, flush=False):
        if not self.enabled:
            return

        self.receivedStatistics[self.receivedStatistics_idx].elements = elementsReceived
        self.receivedStatistics[self.receivedStatistics_idx].queueSize = queueSize
        self.receivedStatistics[self.receivedStatistics_idx].secs = time.time()
        self.receivedStatistics_idx += 1
        self.receivedStatistics_idx = self.receivedStatistics_idx%self.historyWindow
        if flush:
            self.flushTime = self.receivedStatistics[self.receivedStatistics_idx].secs

        if EOS :
            try:
                self.activeStreamIDs.remove(streamID)
            except:
                pass
        else:
            if streamID not in self.activeStreamIDs:
                self.activeStreamIDs.append( streamID )

            

    def retrieve(self):
        if not self.enabled:
            return None

        self.runningStats = PortStatistics(portName=self.name, averageQueueDepth=-1, elementsPerSecond=-1, bitsPerSecond=-1, callsPerSecond=-1, streamIDs=[], timeSinceLastCall=-1, keywords=[])

        listPtr = (self.receivedStatistics_idx + 1) % self.historyWindow    # don't count the first set of data, since we're looking at change in time rather than absolute time
        frontTime = self.receivedStatistics[(self.receivedStatistics_idx - 1) % self.historyWindow].secs
        backTime = self.receivedStatistics[self.receivedStatistics_idx].secs
        totalData = 0.0
        queueSize = 0.0
        while (listPtr != self.receivedStatistics_idx):
            totalData += self.receivedStatistics[listPtr].elements
            queueSize += self.receivedStatistics[listPtr].queueSize
            listPtr += 1
            listPtr = listPtr%self.historyWindow

        # copy stream ids used 
        streamIDs = []
        for sid in self.activeStreamIDs:
            streamIDs.append(sid)

        receivedSize = len(self.receivedStatistics)
        currentTime = time.time()
        totalTime = currentTime - backTime
        if totalTime == 0:
            totalTime = 1e6
        self.runningStats.bitsPerSecond = (totalData * self.bitSize) / totalTime
        self.runningStats.elementsPerSecond = totalData / totalTime
        self.runningStats.averageQueueDepth = queueSize / receivedSize
        self.runningStats.callsPerSecond = float((receivedSize - 1)) / totalTime
        self.runningStats.streamIDs = streamIDs
        self.runningStats.timeSinceLastCall = currentTime - frontTime
        if not self.flushTime == None:
            flushTotalTime = currentTime - self.flushTime
            self.runningStats.keywords = [CF.DataType(id="timeSinceLastFlush", value=CORBA.Any(CORBA.TC_double, flushTotalTime))]

        return self.runningStats



class OutStats:
    class statPoint:
        def __init__(self):
            self.elements = 0
            self.queueSize = 0.0
            self.secs = 0.0
            self.streamID = ""

    def __init__(self, name, element_type='', bits=0):
        # Backwards-compatibility: accept an element type string for use with
        # struct.calcsize
        if not bits:
            bits = struct.calcsize(element_type) * 8
        self.enabled = True
        self.bitSize = bits
        self.historyWindow = 10
        self.receivedStatistics = {}
        self.name = name
        self.receivedStatistics_idx = {}
        self.activeStreamIDs = []
        self.connection_errors={}

    def setEnabled(self, enableStats):
        self.enabled = enableStats

    def connectionErrors(self, connection_id):
        self.connection_errors.setdefault(connection_id,0)
        return self.connection_errors[connection_id]

    def connectionErrors(self, connection_id, n ):
        self.connection_errors.setdefault(connection_id,0)
        self.connection_errors[connection_id] = self.connection_errors[connection_id]+n
        return self.connection_errors[connection_id]

    def add(self, connectionId):
        self.receivedStatistics[connectionId] = [self.statPoint() for xx in xrange(self.historyWindow)]
        self.receivedStatistics_idx[connectionId] = 0
        self.connection_errors[connectionId] = 0
    
    def remove(self, connectionId):
        if self.receivedStatistics.has_key(connectionId):
            self.receivedStatistics.pop(connectionId)
        if self.connection_errors.has_key(connectionId):
            self.connection_errors.pop(connectionId)

    def update(self, elementsReceived, queueSize, EOS, streamID, connectionId):
        self.connection_errors.setdefault(connectionId,0)
        self.connection_errors[connectionId]=0
        if not self.enabled:
            return

        if not self.receivedStatistics.has_key(connectionId):
            self.add(connectionId)
        self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].elements = elementsReceived
        self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].queueSize = queueSize
        self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].secs = time.time()
        self.receivedStatistics[connectionId][self.receivedStatistics_idx[connectionId]].streamID = streamID
        self.receivedStatistics_idx[connectionId] += 1
        self.receivedStatistics_idx[connectionId] = self.receivedStatistics_idx[connectionId]%self.historyWindow

    def retrieve(self):
        if not self.enabled:
            return

        retVal = []
        for entry in self.receivedStatistics:
            runningStats = PortStatistics(portName=self.name,averageQueueDepth=-1,elementsPerSecond=-1,bitsPerSecond=-1,callsPerSecond=-1,streamIDs=[],timeSinceLastCall=-1,keywords=[])

            listPtr = (self.receivedStatistics_idx[entry] + 1) % self.historyWindow    # don't count the first set of data, since we're looking at change in time rather than absolute time
            frontTime = self.receivedStatistics[entry][(self.receivedStatistics_idx[entry] - 1) % self.historyWindow].secs
            backTime = self.receivedStatistics[entry][self.receivedStatistics_idx[entry]].secs
            totalData = 0.0
            queueSize = 0.0
            streamIDs = []
            while (listPtr != self.receivedStatistics_idx[entry]):
                totalData += self.receivedStatistics[entry][listPtr].elements
                queueSize += self.receivedStatistics[entry][listPtr].queueSize
                streamIDptr = 0
                foundstreamID = False
                while (streamIDptr != len(streamIDs)):
                    if (streamIDs[streamIDptr] == self.receivedStatistics[entry][listPtr].streamID):
                        foundstreamID = True
                        break
                    streamIDptr += 1
                if (not foundstreamID):
                    streamIDs.append(self.receivedStatistics[entry][listPtr].streamID)
                listPtr += 1
                listPtr = listPtr % self.historyWindow

            currentTime = time.time()
            totalTime = currentTime - backTime
            if totalTime == 0:
                totalTime = 1e6
            receivedSize = len(self.receivedStatistics[entry])
            runningStats.bitsPerSecond = (totalData * self.bitSize) / totalTime
            runningStats.elementsPerSecond = totalData/totalTime
            runningStats.averageQueueDepth = queueSize / receivedSize
            runningStats.callsPerSecond = float((receivedSize - 1)) / totalTime
            runningStats.streamIDs = streamIDs
            runningStats.timeSinceLastCall = currentTime - frontTime
            usesPortStat = UsesPortStatistics(connectionId=entry, statistics=runningStats)
            retVal.append(usesPortStat)
        return retVal
