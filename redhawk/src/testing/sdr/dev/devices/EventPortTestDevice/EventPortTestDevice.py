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

from ossie.cf import CF, CF__POA
from omniORB import URI, any, PortableServer, CORBA
from ossie.cf import CF, CF__POA
from ossie.device import ExecutableDevice, start_device
import commands, os, sys
import logging
try:
    import CosEventComm,CosEventComm__POA
    import CosEventChannelAdmin, CosEventChannelAdmin__POA
    from ossie.cf import StandardEvent
    hasEvents = True
except:
    hasEvents = False

from EventPortTestDeviceProps import PROPERTIES


class Supplier_i(CosEventComm__POA.PushSupplier):
    def disconnect_push_supplier (self):
        pass

class Consumer_i(CosEventComm__POA.PushConsumer):
    def __init__(self, portRef):
        self.portRef = portRef
    
    def push(self, data):
        if data._v == "message device":
            self.portRef.parent.ports['event_supplier'].sendEvent(any.to_any("response device"))
    
    def disconnect_push_consumer (self):
        pass


class supplierOut_i(CF__POA.Port):
    def __init__(self, parent, name):
        self.parent = parent
        self.name = name
        self.outPorts = {}
    
    def connectPort(self, connection, connectionId):
        port = connection._narrow(CosEventChannelAdmin__POA.EventChannel)
        self.outPorts[str(connectionId)] = port
        self._connectSupplierToEventChannel(port)
    
    def disconnectPort(self, connectionId):
        if self.outPorts.has_key(str(connectionId)):
            self.outPorts.pop(str(connectionId), None)
    
    def sendEvent(self, eventData):
        self._proxy_consumer.push(eventData)
    
    def getIdentifiers(self):
        props = []
        for id in self.outPorts:
            value = any.to_any(self.outPorts[id]._get_identifier())
            props.append(CF.DataType(id=id, value=value))
        return props
    
    def _connectSupplierToEventChannel(self, channel):
        if channel is None:
            return

        try:
            supplier_admin = channel.for_suppliers()
            self._proxy_consumer = supplier_admin.obtain_push_consumer()
            self._supplier = Supplier_i()
            self._proxy_consumer.connect_push_supplier(self._supplier._this())
        except:
            print "Failed to connect to channel"


class consumerOut_i(CF__POA.Port):
    def __init__(self, parent, name):
        self.parent = parent
        self.name = name
        self.outPorts = {}
    
    def connectPort(self, connection, connectionId):
        port = connection._narrow(CosEventChannelAdmin__POA.EventChannel)
        self.outPorts[str(connectionId)] = port
        self._connectConsumerToEventChannel(port)
    
    def disconnectPort(self, connectionId):
        if self.outPorts.has_key(str(connectionId)):
            self.outPorts.pop(str(connectionId), None)
    
    def getIdentifiers(self):
        props = []
        for id in self.outPorts:
            value = any.to_any(self.outPorts[id]._get_identifier())
            props.append(CF.DataType(id=id, value=value))
        return props
    
    def _connectConsumerToEventChannel(self, channel):
        if channel is None:
            return

        try:
            supplier_admin = channel.for_consumers()
            self._proxy_supplier = supplier_admin.obtain_push_supplier()
            self._consumer = Consumer_i(self)
            self._proxy_supplier.connect_push_consumer(self._consumer._this())
        except:
            print "Failed to connect to channel"


class PortDevice_impl(CF__POA.ExecutableDevice, ExecutableDevice):
    def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
        # NB: For now, the ports must be created before calling ExecutableDevice.__init__(),
        #     which registers with the DomainManager, otherwise a race condition may exist.
        #     The DomainManager attempts to make the connections from the DCD, and it is
        #     possible that getPort() will be called before __init__() completes.
        self.ports = {}
        self.ports['event_supplier'] = supplierOut_i(self, 'event_supplier')
        self.ports['event_consumer'] = consumerOut_i(self, 'event_consumer')

        ExecutableDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams, PROPERTIES)
        self._props["memCapacity"] = 100000000
        self._props["BogoMipsCapacity"] = 100000000

    def getPort(self, name):
        if self.ports.has_key(name):
            return self.ports[str(name)]._this()
        else:
            raise CF.PortSupplier.UnknownPort()

    def runTest(self, testid, properties):
        if testid == 0:
            port = self.ports['resource_out']
            return port.getIdentifiers()
        elif testid == 1:
            port = self.ports['devicemanager_out']
            return port.getIdentifiers()
        else:
            raise CF.TestableObject.UnknownTest('unknown test: ' + str(id))
        return []

    def allocate_memCapacity(self, value):
      if self._props["memCapacity"] < value:
          return False
      self._props["memCapacity"] = self._props["memCapacity"] - value
      return True

    def allocate_BogoMipsCapacity(self, value):
      if self._props["BogoMipsCapacity"] < value:
          return False
      self._props["BogoMipsCapacity"] = self._props["BogoMipsCapacity"] - value
      return True

    def deallocate_memCapacity(self, value):
      self._props["memCapacity"] = self._props["memCapacity"] + value

    def deallocate_BogoMipsCapacity(self, value):
      self._props["BogoMipsCapacity"] = self._props["BogoMipsCapacity"] + value

    def updateUsageState(self):
      # Update usage state
      if self._props["memCapacity"] == 0 and self._props["BogoMipsCapacity"] == 0:
          self._usageState = CF.Device.BUSY
      elif self._props["memCapacity"] == 100000000 and self._props["BogoMipsCapacity"] == 100000000:
          self._usageState = CF.Device.IDLE
      else:
          self._usageState = CF.Device.ACTIVE

if __name__ == "__main__":
    logging.getLogger().setLevel(logging.WARN)
    logging.debug("Starting Device")
    start_device(PortDevice_impl)
