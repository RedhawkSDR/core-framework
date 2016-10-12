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

import unittest
from _unitTestHelpers import scatest
from omniORB import URI, any
from ossie.cf import CF
import threading
import time
import CosEventComm,CosEventComm__POA
import CosEventChannelAdmin, CosEventChannelAdmin__POA
from ossie.cf import StandardEvent


class Supplier_i(CosEventComm__POA.PushSupplier):
    def disconnect_push_supplier (self):
        pass

class Consumer_i(CosEventComm__POA.PushConsumer):
    def __init__(self, parent):
        self.parent = parent

    def push(self, data):
        if data._v == "response":
            self.parent.eventFlag = True
            self.parent.localEvent.set()

    def disconnect_push_consumer (self):
        pass

class ConsumerDevice_i(CosEventComm__POA.PushConsumer):
    def __init__(self, parent):
        self.parent = parent

    def push(self, data):
        if data._v == "response device":
            self.parent.eventFlag = True
            self.parent.localEvent.set()

    def disconnect_push_consumer (self):
        pass


class EventPortConnectionsTest(scatest.CorbaTestCase):
    def setUp(self):
        self._domBooter, self._domMgr = self.launchDomainManager()

    def tearDown(self):
        try:
            self._app.stop()
            self._app.releaseObject()
        except AttributeError:
            pass

        try:
            self._devMgr.shutdown()
        except AttributeError:
            pass

        try:
            self.terminateChild(self._devBooter)
        except AttributeError:
            pass

        try:
            self.terminateChild(self._domBooter)
        except AttributeError:
            pass

        # Do all application and node booter shutdown before calling the base
        # class tearDown, or failures will occur.
        scatest.CorbaTestCase.tearDown(self)

    def test_EventDevicePortConnection(self):
        self.localEvent = threading.Event()
        self.eventFlag = False
        
        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_EventPortTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        time.sleep(1)   # this sleep is here for the connections to be established to the event service

        domainName = scatest.getTestDomainName()

        req = CF.EventChannelManager.EventRegistration( 'deviceEvent', '')
        try:
            ecm = self._domMgr._get_eventChannelMgr()
            creg = ecm.registerResource( req ) 
            devChannel = creg.channel
        except:
            self.assertEqual(False, True)
        else:
            self.assertEqual(True, True)

        # resolve the producer for the event
        supplier_admin = devChannel.for_suppliers()
        _proxy_consumer = supplier_admin.obtain_push_consumer()
        _supplier = Supplier_i()
        _proxy_consumer.connect_push_supplier(_supplier._this())

        # resolve the consumer for the event
        consumer_admin = devChannel.for_consumers()
        _proxy_supplier = consumer_admin.obtain_push_supplier()
        _consumer = ConsumerDevice_i(self)
        _proxy_supplier.connect_push_consumer(_consumer._this())

        _proxy_consumer.push(any.to_any("message device"))
        self.localEvent.wait(5.0)
        self.assertEqual(self.eventFlag, True)

    def test_EventAppPortConnection(self):
        self.localEvent = threading.Event()
        self.eventFlag = False
        
        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_EventPortTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        domainName = scatest.getTestDomainName()
        self._domMgr.installApplication("/waveforms/PortConnectFindByDomainFinderEvent/PortConnectFindByDomainFinderEvent.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        app.start()

        # rh 1.11 and forward event channels belong to the Domain...
        req = CF.EventChannelManager.EventRegistration( 'anotherChannel', '')
        try:
            ecm = self._domMgr._get_eventChannelMgr()
            creg = ecm.registerResource( req ) 
            appChannel = creg.channel
        except:
            self.assertEqual(False, True)
        else:
            self.assertEqual(True, True)

        # resolve the producer for the event
        supplier_admin = appChannel.for_suppliers()
        _proxy_consumer = supplier_admin.obtain_push_consumer()
        _supplier = Supplier_i()
        _proxy_consumer.connect_push_supplier(_supplier._this())

        # resolve the consumer for the event
        consumer_admin = appChannel.for_consumers()
        _proxy_supplier = consumer_admin.obtain_push_supplier()
        _consumer = Consumer_i(self)
        _proxy_supplier.connect_push_consumer(_consumer._this())

        _proxy_consumer.push(any.to_any("message"))
        self.localEvent.wait(5.0)
        self.assertEqual(self.eventFlag, True)

        app.releaseObject()
        # we're not actually removing the channel from the naming service right now
        #channelName = URI.stringToName("%s/%s" % (domainName, 'anotherChannel'))
        #try:
        #    appChannel = self._root.resolve(channelName)._narrow(CosEventChannelAdmin__POA.EventChannel)
        #except:
        #    self.assertEqual(True, True)
        #else:
        #    self.assertEqual(False, True)
