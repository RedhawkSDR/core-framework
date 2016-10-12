#!/usr/bin/env python
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

#
from omniORB import any
from ossie.cf import CF, CF__POA
from ossie.resource import Resource, usesport, providesport, start_component
from ossie.events import GenericEventConsumer
try:
    import CosEventChannelAdmin
    from ossie.cf import StandardEvent
    hasEvents = True
except:
    hasEvents = False


class UsesPort(CF__POA.Port):
    def __init__(self, repid):
        self._connections = {}
        self.__repid = repid

    def connectPort(self, connection, connectionId):
        try:
            port = connection._narrow(self.__repid)
        except:
            port = None
        if port is None:
            # InvalidPort includes a numeric error code, instead of a CF::ErrorNumberType,
            # though it's not particularly useful in this case, anyway.
            raise CF.Port.InvalidPort(0, "connection could not be narrowed to '%s'" % (self.__repid._NP_RepositoryId,))

        port = self._connectPort(port, connectionId)
        if port:
            self._connections[str(connectionId)] = port

    def disconnectPort(self, connectionId):
        try:
            del self._connections[str(connectionId)]
        except KeyError:
            raise CF.Port.InvalidPort(CF.CF_EINVAL, "No such connection '%s'" % (connectionId,))            

    def _connectPort(self, port, connectionId):
        return port


class TestProvidesPort(CF__POA.Resource):
    def __init__(self, identifier):
        self._identifier = identifier

    def _get_identifier(self):
        return self._identifier


class TestUsesPort(UsesPort):
    def __init__(self):
        UsesPort.__init__(self, CF.Resource)

    def getIdentifiers(self):
        props = []
        for id in self._connections:
            value = any.to_any(self._connections[id]._get_identifier())
            props.append(CF.DataType(id=id, value=value))
        return props


class PropSetUsesPort(UsesPort):
    def __init__(self):
        UsesPort.__init__(self, CF.PropertySet)

    def query(self, propsIn):
        propsOut = []
        for propset in self._connections.values():
            propsOut += propset.query(propsIn)
        return propsOut


class DomainManagerUsesPort(UsesPort):
    def __init__(self):
        UsesPort.__init__(self, CF.DomainManager)

    def getIdentifiers(self):
        props = []
        for id in self._connections:
            value = any.to_any(self._connections[id]._get_identifier())
            props.append(CF.DataType(id=id, value=value))
        return props


class DeviceManagerUsesPort(UsesPort):
    def __init__(self):
        UsesPort.__init__(self, CF.DeviceManager)

    def getIdentifiers(self):
        props = []
        for id in self._connections:
            value = any.to_any(self._connections[id]._get_identifier())
            props.append(CF.DataType(id=id, value=value))
        return props


class EventSupplierPort(UsesPort):
    def __init__(self):
        UsesPort.__init__(self, CosEventChannelAdmin.EventChannel)
    
    def sendEvent(self, event):
        for consumer in self._connections.values():
            try:
                consumer.push(event)
            except:
                pass
    
    def _connectPort(self, channel, connectionId):
        try:
            supplier_admin = channel.for_suppliers()
            proxy_consumer = supplier_admin.obtain_push_consumer()
            # Connect to the push supplier without a Supplier object; we will not
            # receive notification if we are disconnected.
            proxy_consumer.connect_push_supplier(None)
            return proxy_consumer
        except:
            return None


class EventConsumerPort(UsesPort):
    def __init__(self, onPush):
        UsesPort.__init__(self, CosEventChannelAdmin.EventChannel)
        self._onPush = onPush
    
    def _connectPort(self, channel, connectionId):
        try:
            supplier_admin = channel.for_consumers()
            proxy_supplier = supplier_admin.obtain_push_supplier()
            consumer = GenericEventConsumer(self._onPush, lambda : self._onDisconnect(connectionId))
            proxy_supplier.connect_push_consumer(consumer._this())
            return consumer
        except:
            return None

    def _onDisconnect(self, connectionId):
        self.disconnectPort(connectionId)


class PortTest(CF__POA.Resource, Resource):
    """Simple Python component for basic port testing"""

    toTest = usesport("resource_out", "IDL:CF/Resource:1.0", type_="test");
    fromOther = providesport("resource_in", "IDL:CF/Resource:1.0", type_="test")
    toPropSet = usesport("propset_out", "IDL:CF/PropertySet:1.0", type_="test")
    eventSupplier = usesport("event_supplier", "IDL:CosEventChannelAdmin/EventChannel:1.0", type_="test");
    eventConsumer = usesport("event_consumer", "IDL:CosEventChannelAdmin/EventChannel:1.0", type_="test");
    toDomainManager = usesport("domain_manager", "IDL:CF/DomainManager:1.0", type_="test")
    toDeviceManager = usesport("device_manager", "IDL:CF/DeviceManager:1.0", type_="test")

    def __init__(self, identifier, execparams):
        Resource.__init__(self, identifier, execparams)

    def initialize(self):
        Resource.initialize(self)
        self.toTest = TestUsesPort()
        self.fromOther = TestProvidesPort(self._get_identifier() + "/resource_in")
        self.toPropSet = PropSetUsesPort()
        self.eventSupplier = EventSupplierPort()
        self.eventConsumer = EventConsumerPort(self._onPush)
        self.toDomainManager = DomainManagerUsesPort()
        self.toDeviceManager = DeviceManagerUsesPort()

    def runTest(self, testid, properties):
        if testid == 0:
            return self.toTest.getIdentifiers()
        elif testid == 1:
            return self.toPropSet.query([])
        elif testid == 2:
            return self.toDomainManager.getIdentifiers()
        elif testid == 3:
            return self.toDeviceManager.getIdentifiers()
        else:
            raise CF.TestableObject.UnknownTest()
        return []

    def _onPush(self, data, typecode):
        if data == "message":
            self.eventSupplier.sendEvent(any.to_any("response"))


if __name__ == '__main__':
    start_component(PortTest)
