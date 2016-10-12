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

from ossie.cf import CF, ExtendedCF

class Connection(object):
    def __init__(self, port):
        self.port = port

class UsesPort(object):
    def __init__(self, name, portType):
        self._connectionMutex = threading.Lock()
        self._name = name
        self._connections = {}
        self.PortType = portType
        self._connectListeners = []
        self._disconnectListeners = []

    def getName(self):
        return self._name

    def addConnectListener(self, listener):
        self._connectListeners.append(listener)

    def removeConnectListener(self, listener):
        try:
            self._connectListeners.remove(listener)
        except:
            pass

    def addDisconnectListener(self, listener):
        self._disconnectListeners.append(listener)

    def removeDisconnectListener(self, listener):
        try:
            self._disconnectListeners.remove(listener)
        except:
            pass

    def connectPort(self, connection, connectionId):
        # Give a specific exception message for nil
        if connection is None:
            raise CF.Port.InvalidPort(1, 'Nil object reference')

        # Attempt to narrow the remote object to the correct type; note this
        # does not require the lock
        try:
            port = connection._narrow(self.PortType)
        except:
            raise CF.Port.InvalidPort(1, 'Object unreachable')

        # If the narrow returned nil without throwing an exception, it's safe
        # to assume the object is the wrong type
        if port is None:
            raise CF.Port.InvalidPort(1, 'Object is not a ' + self.PortType._NP_RepositoryId)

        self._connectionMutex.acquire()
        try:
            entry = self._connections.get(connectionId, None)
            if entry is None:
                # Store the new connection and pass the new entry along to
                # _connectionAdded
                entry = Connection(port)
                self._connections[connectionId] = entry

                # Allow subclasses to do additional bookkeeping
                self._connectionAdded(connectionId, entry)
            else:
                # Replace the object reference
                entry.port = port

                # Allow subclasses to do additional bookkeeping
                self._connectionModified(connectionId, entry)
        finally:
            self._connectionMutex.release()

        # Notify connection listeners
        for listener in self._connectListeners:
            listener(connectionId)
    
    def disconnectPort(self, connectionId):
        self._connectionMutex.acquire()  
        try:
            if not connectionId in self._connections:
                raise CF.Port.InvalidPort(2, 'No connection ' + connectionId)

            # Allow subclasses to do additional cleanup
            self._connectionRemoved(connectionId, self._connections[connectionId])
            del self._connections[connectionId]
        finally:
            self._connectionMutex.release()  

        # Notify disconnection listeners
        for listener in self._disconnectListeners:
            listener(connectionId)

    def _get_connections(self):
        self._connectionMutex.acquire()  
        try:
            return [ExtendedCF.UsesConnection(k,v.port) for k, v in self._connections.iteritems()]
        finally:
            self._connectionMutex.release()  

    def _connectionAdded(self, connectionId, connection):
        pass

    def _connectionModified(self, connectionId, connection):
        pass

    def _connectionRemoved(self, connectionId, connection):
        pass
