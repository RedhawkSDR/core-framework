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

import logging

log = logging.getLogger(__name__)

class Endpoint(object):
    def disconnected(self, connectionId):
        pass

class PortEndpoint(Endpoint):
    def __init__(self, supplier, port):
        self.supplier = supplier
        self.port = port

    def getReference(self):
        return self.supplier.getPort(self.port['Port Name'])

    def getName(self):
        return '%s/%s' % (self.supplier._instanceName, self.port['Port Name'])

    def getInterface(self):
        return self.port['Port Interface']

    def hasComponent(self, component):
        return self.supplier._refid == component._refid

    def getRefid(self):
        return self.supplier._refid

    def getPortName(self):
        return self.port['Port Name']

    def disconnected(self, connectionId):
        self.supplier._disconnected(connectionId)


class ComponentEndpoint(Endpoint):
    def __init__(self, component):
        self.component = component

    def getReference(self):
        return self.component.ref

    def getName(self):
        return self.component._instanceName

    def getInterface(self):
        return 'IDL:CF/Resource:1.0'

    def hasComponent(self, component):
        return self.component._refid == component._refid

    def getRefid(self):
        return self.component._refid

    def getPortName(self):
        return None

class ObjectEndpoint(Endpoint):
    def __init__(self, objref, interface):
        self.objref = objref
        self.interface = interface

    def getReference(self):
        return self.objref.ref

    def getName(self):
        try:
            return self.objref._instanceName
        except AttributeError:
            return "(unnamed object)"

    def getInterface(self):
        return self.interface

    def hasComponent(self, component):
        try:
            return self.objref._refid == component._refid
        except AttributeError:
            return self.objref.ref._is_equivalent(component.ref)

    def getRefid(self):
        try:
            return self.objref._refid
        except AttributeError:
            return "(no refid)"

class ConnectionManager(object):
    __instance__ = None

    @classmethod
    def instance(cls):
        if not cls.__instance__:
            cls.__instance__ = ConnectionManager()
        return cls.__instance__

    def __init__(self):
        self.__connections = {}

    def getConnections(self):
        return self.__connections

    def getConnectionsBetween(self, usesComponent, providesComponent):
        connections = {}
        for identifier, (uses, provides) in self.__connections.iteritems():
            if uses.hasComponent(usesComponent) and provides.hasComponent(providesComponent):
                connections[identifier] = (uses, provides)
        return connections

    def registerConnection(self, identifier, uses, provides):
        if identifier in self.__connections:
            log.warn("Skipping registration of duplicate connection id '%s'", identifier)
            return
        log.debug("Registering connection '%s' from %s to %s", identifier, uses, provides)
        self.__connections[identifier] = (uses, provides)

    def unregisterConnection(self, identifier):
        if not identifier in self.__connections:
            log.warn("Skipping unregistration of unknown connection id '%s'", identifier)
            return
        log.debug("Unregistering connection '%s'", identifier)
        del self.__connections[identifier]

    def breakConnection(self, identifier):
        uses, provides = self.__connections[identifier]
        log.debug("Breaking connection '%s'", identifier)
        try:
            usesPort = uses.getReference()
            usesPort.disconnectPort(identifier)
        except:
            log.warn("Ignoring exception breaking connection '%s'", identifier)
        uses.disconnected(identifier)
        provides.disconnected(identifier)

    def cleanup(self):
        for identifier in self.__connections.iterkeys():
            self.breakConnection(identifier)
        self.__connections = {}
