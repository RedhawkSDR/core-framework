#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

from ossie.cf import CF__POA
from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA 
import threading

class VPort(BULKIO__POA.virtualPort, CF__POA.Port):
    def __init__(self, parent, vars={}, **extras):
        self.parent = parent
        
        self.vars = dict(vars)
        self.vars.update(extras)
        self.port_lock = threading.Lock()
        self.refreshSRI = False
        self.defaultStreamSRI = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 200, 0.0, 0.001, 1, 1, "sampleStream", [])
        self.active = True 
        self.refreshSRIList = {}
        self.connectionToStream = {}
        self.streamToConnection = {}
        self.streamToPorts = {}
        self.defaultSRIList = {}
        self.outPorts = {}
        
    def connectPort(self, connection, connectionId):
        self.parent.virtual_port.port_lock.acquire()
        port = connection._narrow(self.parent.virtual_port.vars["type"])
        self.parent.virtual_port.outPorts[str(connectionId)] = port
        self.parent.virtual_port.refreshSRI = True
        self.parent.virtual_port.port_lock.release()

    def disconnectPort(self, connectionId):
        self.port_lock.acquire()
        if self.outPorts.has_key(str(connectionId)):
            self.outPorts.pop(str(connectionId), None)
        else:
            self.port_lock.release()
            return
        self.port_lock.release()

    def assign(self, connectionName, streamId):
        destStreams = []
        destConns = []
        destPorts = []
        self.outPorts = self.parent.virtual_port.outPorts
        
        # make sure that the connection exists
        if (self.outPorts[connectionName] != None):
            if (not connectionName in self.connectionToStream):
                destStreams = []
                self.connectionToStream[connectionName] = destStreams
        else:
            raise BULKIO.invalidConnectionName()

        # make sure that the streamId exists
        if (self.defaultSRIList[streamId] == None):
            raise BULKIO.invalidStreamID()
        else:
            if (not streamId in self.streamToConnection):
                destConns = []
                self.streamToConnection[streamId] = destConns
            
            if (not streamId in self.streamToPorts):
                destPorts = []
                self.streamToPorts[streamId] = destPorts

        destStreams.append(streamId)
        destConns.append(connectionName)
        destPorts.append(self.outPorts[connectionName])
        
        self.refreshSRIList[streamId] = True

    def unassign(self, connectionName, streamId):
        destStreams = []
        destConns = []
        destPorts = []
        
        # make sure that the connection exists
        if (self.outPorts[connectionName] != None):
            if (not connectionName in self.connectionToStream):
                destStreams = []
                self.connectionToStream[connectionName] = destStreams
        else:
            raise BULKIO.invalidConnectionName()

        # make sure that the streamId exists
        if (self.defaultSRIList[streamId] == None):
            raise BULKIO.invalidStreamID()

        if (destStreams != None):
            destStreams.remove(streamId)

        if (streamId in self.streamToConnection):
            destConns = self.streamToConnection[streamId]
            destConns.remove(connectionName)
            
        if (streamId in self.streamToPorts):
            destPorts = self.streamToPorts.get[streamId]
            destPorts.remove(self.outPorts[connectionName])
                            
    def assignStreams(self, connectionName, streamIdList):
        for streamId in self.streamIdList: 
            self.assign(connectionName, streamId)

    def unassignStreams(self, connectionName, streamIdList):    
        for streamId in self.streamIdList: 
            self.unassign(connectionName, streamId)

    def assignConnections(self, connectionNameList, streamId):
        for connectionName in self.connectionNameList: 
            self.assign(connectionName, streamId)

    def unassignConnections(self, connectionNameList, streamId):
        for connectionName in self.connectionNameList: 
            self.unassign(connectionName, streamId)

    def assignAllStreams(self, connectionName):    
        for streamId in self.defaultSRIList: 
            try:
                self.assign(connectionName, streamId)
            except:
                raise BULKIO.invalidStreamID() 

    def unassignAllStreams(self, connectionName):
        for streamId in self.defaultSRIList: 
            try:
                self.unassign(connectionName, streamId)
            except:
                raise BULKIO.invalidStreamID()

    def queryConnection(self, connectionName):
        streams = self.connectionToStream.get[connectionName]
        
        # make sure that the connection exists
        if (streams == None): 
            raise BULKIO.invalidConnectionName()
        
        return streams
    
    def queryStreamConnections(self, streamId):
        connections = self.streamToConnection[streamId]
        
        # make sure that the streamId exists
        if (connections == None): 
            raise BULKIO.invalidStreamID()
        
        return connections
    
    def queryStreamIDs(self): 
        streamIDList = []
        
        for streamID in self.defaultSRIList:
            streamIDList.append(streamID)
        
        return streamIDLists

    def queryStreamSRI(self, streamId):
        node = self.defaultSRIList[streamId]
        
        if (node == None): 
            raise BULKIO.invalidStreamID()
        
        return node

    def queryStreamSRIs(self): 
        streamSRIList = []
        
        for streamID in self.defaultSRIList:
            streamSRIList.append(self.defaultSRIList[streamID])
            
        return streamSRIList    

    def addSRI(self, streamSri): 
        self.defaultSRIList[streamSri.streamID] = streamSri
        self.refreshSRIList[streamSri.streamID] = True

    def removeSRI(self, streamId):
        found = self.defaultSRIList.remove(streamId) != None
        
        if (found):
            self.refreshSRIList.remove(streamId)
        
        return found

    def findSRI(self, streamId):
        return self.defaultSRIList[streamId]
    
    def setRefreshSRI(self, streamId, refresh):
        self.refreshSRIList[streamId] = refresh
    
    def getRefreshSRI(self, streamId):
        val = self.refreshSRIList[streamId]
        
        if (val != None):
            if (val):
                return True
        
        return False
    
    def getPorts(self):
        return self.outPorts
