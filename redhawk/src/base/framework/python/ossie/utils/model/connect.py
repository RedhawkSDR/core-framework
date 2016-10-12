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
        for _identifier, (identifier, uses, provides) in self.__connections.iteritems():
            if uses.hasComponent(usesComponent) and provides.hasComponent(providesComponent):
                connections[_identifier] = (identifier, uses, provides)
        return connections

    def registerConnection(self, identifier, uses, provides):
        _name = uses.getName()
        if _name+identifier in self.__connections:
            log.warn("Skipping registration of duplicate connection id '%s'", identifier)
            return
        log.debug("Registering connection '%s' from %s to %s", identifier, uses, provides)
        self.__connections[_name+identifier] = (identifier, uses, provides)

    def unregisterConnection(self, identifier, _uses):
        _name = _uses.getName()
        if not _name+identifier in self.__connections:
            log.warn("Skipping unregistration of unknown connection id '%s'", identifier)
            return
        log.debug("Unregistering connection '%s'", identifier)
        del self.__connections[_name+identifier]
    
    def resetConnections(self):
        self.__connections = {}
    
    def refreshConnections(self, components):
        providesPorts = []
        for component in components:
            for port in component.ports:
                if port._direction == 'Provides':
                    providesPorts.append((component,port))
                    
        for component in components:
            for port in component.ports:
                if port._direction == 'Uses':
                    usesPort = {}
                    usesPort["Port Name"] = port.name
                    usesPort["Port Interface"] = None
                    for connection in port._get_connections():
                        if usesPort["Port Interface"] == None:
                            usesPort["Port Interface"] = connection.port._NP_RepositoryId
                            _usesPortEndpoint = PortEndpoint(component, usesPort)
                        _providesPortEndpoint = None
                        for component in components:
                            if connection.port._is_equivalent(component.ref):
                                _providesPortEndpoint = ComponentEndpoint(component)
                                self.registerConnection(connection.connectionId, _usesPortEndpoint, _providesPortEndpoint)
                                break
                        if _providesPortEndpoint != None:
                            continue
                        for _provides in providesPorts:
                            if connection.port._is_equivalent(_provides[1].ref):
                                provPort = {}
                                provPort["Port Name"] = _provides[1].name
                                provPort["Port Interface"] = usesPort["Port Interface"]
                                _providesPortEndpoint = PortEndpoint(_provides[0],provPort)
                                self.registerConnection(connection.connectionId, _usesPortEndpoint, _providesPortEndpoint)
                                break

    def breakConnection(self, identifier, _uses):
        _name = _uses.getName()
        _identifier, uses, provides = self.__connections[_name+identifier]
        log.debug("Breaking connection '%s'", identifier)
        try:
            usesPort = uses.getReference()
            usesPort.disconnectPort(identifier)
        except:
            log.warn("Ignoring exception breaking connection '%s'", identifier)
        uses.disconnected(identifier)
        provides.disconnected(identifier)

    def cleanup(self):
        for _identifier in self.__connections.iterkeys():
            self.breakConnection(self.__connections[_identifier][0], self.__connections[_identifier][1])
        self.__connections = {}
