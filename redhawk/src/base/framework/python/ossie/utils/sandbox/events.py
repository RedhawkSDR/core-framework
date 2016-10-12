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
import weakref

from omniORB import CORBA
import CosEventComm
import CosEventChannelAdmin
import CosEventChannelAdmin__POA

from ossie.utils.log4py import logging
from ossie.utils.notify import notification

# Prepare the ORB
orb = CORBA.ORB_init()
poa = orb.resolve_initial_references("RootPOA")

log = logging.getLogger(__name__)

class EventChannel(CosEventChannelAdmin__POA.EventChannel):
    class ProxyPushConsumer(CosEventChannelAdmin__POA.ProxyPushConsumer):
        __slots__ = ('_channel', '_admin', '_supplier', '_connected')

        def __init__(self, channel, admin):
            self._channel = channel
            self._admin = admin
            self._supplier = None
            self._connected = False

        def push(self, data):
            if not self._connected:
                raise CosEventComm.Disconnected
            self._channel.push(data)

        def connect_push_supplier(self, supplier):
            if self._connected:
                raise CosEventChannelAdmin.AlreadyConnected
            self._channel.supplierConnected(supplier)
            self._connected = True
            self._supplier = supplier

        def disconnect_push_consumer(self):
            self._channel.supplierDisconnected(self._supplier)
            self._admin.remove_consumer(self)

        def destroy(self):
            if self._supplier:
                try:
                    self._supplier.disconnect_push_supplier()
                except:
                    pass
            try:
                poa.deactivate_object(poa.servant_to_id(self))
            except:
                pass

    class SupplierAdmin(CosEventChannelAdmin__POA.SupplierAdmin):
        def __init__(self, channel):
            self._channel = channel
            self._consumers = []

        def obtain_push_consumer(self):
            consumer = EventChannel.ProxyPushConsumer(self._channel, self)
            self._consumers.append(consumer)
            return consumer._this()

        def remove_consumer(self, consumer):
            self._consumers.remove(consumer)

        def _destroy(self):
            for consumer in self._consumers:
                consumer.destroy()
            self._consumers = []
            try:
                poa.deactivate_object(poa.servant_to_id(self))
            except:
                pass

    class ProxyPushSupplier(CosEventChannelAdmin__POA.ProxyPushSupplier):
        __slots__ = ('_channel', '_admin', '_consumer')

        def __init__(self, channel, admin):
            self._channel = channel
            self._admin = admin
            self._consumer = None

        def connect_push_consumer(self, consumer):
            if not consumer:
                raise CORBA.BAD_PARAM
            if self._consumer:
                raise CosEventChannelAdmin.AlreadyConnected
            self._consumer = consumer
            self._channel.consumerConnected(consumer)

        def disconnect_push_supplier(self):
            if self._consumer:
                self._channel.consumerDisconnected(self._consumer)

        def push(self, data):
            if self._consumer:
                self._consumer.push(data)

        def destroy(self):
            if self._consumer:
                try:
                    self._consumer.disconnect_push_consumer()
                except:
                    pass
            try:
                poa.deactivate_object(poa.servant_to_id(self))
            except:
                pass

    class ConsumerAdmin(CosEventChannelAdmin__POA.ConsumerAdmin):
        def __init__(self, channel):
            self._channel = channel
            self._suppliers = []

        def obtain_push_supplier(self):
            supplier = EventChannel.ProxyPushSupplier(self._channel, self)
            self._suppliers.append(supplier)
            return supplier._this()

        def _push(self, data):
            for supplier in self._suppliers:
                supplier.push(data)

        def _destroy(self):
            for supplier in self._suppliers:
                supplier.destroy()
            self._suppliers = []
            try:
                poa.deactivate_object(poa.servant_to_id(self))
            except:
                pass

    @notification
    def consumerConnected(self, consumer):
        """
        A new consumer was connected to this event channel.
        """
        log.trace("Consumer '%s' connected to event channel '%s'", consumer, self._name)

    @notification
    def consumerDisconnected(self, consumer):
        """
        A consumer disconnected from this event channel.
        """
        log.trace("Consumer '%s' disconnected from event channel '%s'", consumer, self._name)

    @notification
    def supplierConnected(self, supplier):
        """
        A new supplier was connected to this event channel.
        """
        log.trace("Supplier '%s' connected to event channel '%s'", supplier, self._name)

    @notification
    def supplierDisconnected(self, supplier):
        """
        A supplier disconnected from this event channel.
        """
        log.trace("Supplier '%s' disconnected from event channel '%s'", supplier, self._name)

    @notification
    def eventReceived(self, event):
        """
        An event was posted to this event channel.
        """
        log.trace("Channel '%s' received event '%s'", self._name, event)

    def __init__(self, name):
        self._name = name
        self._consumerAdmin = EventChannel.ConsumerAdmin(weakref.proxy(self))
        self._supplierAdmin = EventChannel.SupplierAdmin(weakref.proxy(self))

    @property
    def name(self):
        return self._name

    @property
    def supplier_count(self):
        return len(self._supplierAdmin._consumers)

    @property
    def consumer_count(self):
        return len(self._consumerAdmin._suppliers)

    def for_consumers(self):
        return self._consumerAdmin._this()

    def for_suppliers(self):
        return self._supplierAdmin._this()

    def push(self, data):
        self.eventReceived(data)
        self._consumerAdmin._push(data)

    def destroy(self):
        log.trace("Destroying event channel '%s'", self._name)
        self._consumerAdmin._destroy()
        self._supplierAdmin._destroy()
        try:
            poa.deactivate_object(poa.servant_to_id(self))
        except:
            pass
