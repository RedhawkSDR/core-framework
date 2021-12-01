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

import unittest, os, signal, time
from _unitTestHelpers import scatest
from omniORB import URI, any
from ossie.cf import CF
import CosNaming
import threading
import subprocess
import CosEventComm,CosEventComm__POA
import CosEventChannelAdmin, CosEventChannelAdmin__POA
from ossie.cf import StandardEvent
from ossie.events import ChannelManager
from ossie.utils import redhawk

# create a class for consuming events
class ConsumerODM_i(CosEventComm__POA.PushConsumer):
    def __init__(self, parent):
        #self.event = threading.Event()
        self.parent = parent
        self.event_category_types = ['DEVICE_MANAGER','DEVICE','APPLICATION_FACTORY','APPLICATION','SERVICE']

        # set up some helper structures
        self.DMAdded = {}
        self.DMRemoved = {}
        for ect in self.event_category_types:
            self.DMAdded[ect] = {}
            self.DMAdded[ect]['event_count'] = 0
            self.DMRemoved[ect] = {}
            self.DMRemoved[ect]['event_count'] = 0

        self._sourceCategoryLookup = {
            StandardEvent.DEVICE_MANAGER:'DEVICE_MANAGER',
            StandardEvent.DEVICE:'DEVICE',
            StandardEvent.APPLICATION_FACTORY:'APPLICATION_FACTORY',
            StandardEvent.APPLICATION:'APPLICATION',
            StandardEvent.SERVICE:'SERVICE'
        }

    def push(self, data_obj):
        data = data_obj.value()

        # some error checking
        if data.sourceCategory not in self._sourceCategoryLookup:
            self.parent.fail("Invalid category: " + str(data.sourceCategory))
            return

        # add event
        if isinstance(data, StandardEvent.DomainManagementObjectAddedEventType):
            if self._sourceCategoryLookup[data.sourceCategory] == 'DEVICE':
                self.parent.assertEqual(data.producerId,'DCE:5f52f645-110f-4142-8cc9-4d9316ddd958')
                self.parent.assertEqual(data.sourceId,'DCE:8f3478e3-626e-45c3-bd01-0a8117dbe59b')
                self.parent.assertEqual(data.sourceName,'BasicTestDevice1')
            elif self._sourceCategoryLookup[data.sourceCategory] == 'SERVICE':
                self.parent.assertEqual(data.producerId,'DCE:5f52f645-110f-4142-8cc9-4d9316ddd958')
                self.parent.assertEqual(data.sourceId,'DCE:8f3478e3-626e-45c3-bd01-0a8117dbe59b')
                self.parent.assertEqual(data.sourceName,'BasicService1')
            elif self._sourceCategoryLookup[data.sourceCategory] == 'DEVICE_MANAGER':
                self.parent.assertEqual(data.producerId,'DCE:5f52f645-110f-4142-8cc9-4d9316ddd958')
                if data.sourceId == 'DCE:65f84ea9-cbe9-42cc-bc2e-6807f7ff9b8c':
                    self.parent.assertEqual(data.sourceName,'BasicTestDevice_node')
                elif data.sourceId == 'DCE:65f84ea9-cbe9-42cc-bc2e-6807f7ff9b10':
                    self.parent.assertEqual(data.sourceName,'BasicService_node')
                else:
                    self.parent.assertEqual(data.sourceId, 'unknown Device Manager')
            self.DMAdded[self._sourceCategoryLookup[data.sourceCategory]][data.sourceId] = data.sourceName
            self.DMAdded[self._sourceCategoryLookup[data.sourceCategory]]['event_count'] += 1
        # remove event
        elif isinstance(data, StandardEvent.DomainManagementObjectRemovedEventType):
            if self._sourceCategoryLookup[data.sourceCategory] == 'DEVICE':
                self.parent.assertEqual(data.producerId,'DCE:5f52f645-110f-4142-8cc9-4d9316ddd958')
                self.parent.assertEqual(data.sourceId,'DCE:8f3478e3-626e-45c3-bd01-0a8117dbe59b')
                self.parent.assertEqual(data.sourceName,'BasicTestDevice1')
            elif self._sourceCategoryLookup[data.sourceCategory] == 'SERVICE':
                self.parent.assertEqual(data.producerId,'DCE:5f52f645-110f-4142-8cc9-4d9316ddd958')
                self.parent.assertEqual(data.sourceId,'DCE:8f3478e3-626e-45c3-bd01-0a8117dbe59b')
                self.parent.assertEqual(data.sourceName,'BasicService1')
            elif self._sourceCategoryLookup[data.sourceCategory] == 'DEVICE_MANAGER':
                self.parent.assertEqual(data.producerId,'DCE:5f52f645-110f-4142-8cc9-4d9316ddd958')
                if data.sourceId == 'DCE:65f84ea9-cbe9-42cc-bc2e-6807f7ff9b8c':
                    self.parent.assertEqual(data.sourceName,'BasicTestDevice_node')
                elif data.sourceId == 'DCE:65f84ea9-cbe9-42cc-bc2e-6807f7ff9b10':
                    self.parent.assertEqual(data.sourceName,'BasicService_node')
                else:
                    self.parent.assertEqual(data.sourceId, 'unknown Device Manager')
            self.DMRemoved[self._sourceCategoryLookup[data.sourceCategory]][data.sourceId] = data.sourceName
            self.DMRemoved[self._sourceCategoryLookup[data.sourceCategory]]['event_count'] += 1


    def checkAddedEvent(self, eventType, sourceId, sourceName):
        if sourceId not in self.DMAdded[eventType.upper()]:
            return False
        if sourceName not in self.DMAdded[eventType.upper()][sourceId]:
            return False
        return True

    def checkRemovedEvent(self, eventType, sourceId, sourceName):
        if sourceId not in self.DMRemoved[eventType.upper()]:
            return False
        if sourceName not in self.DMRemoved[eventType.upper()][sourceId]:
            return False
        return True

    def getAddedEventCount(self, eventType):
        return self.DMAdded[eventType]['event_count']

    def getRemovedEventCount(self, eventType):
        return self.DMRemoved[eventType]['event_count']

    def disconnect_push_consumer (self):
        pass

class ODMEventsTest(scatest.CorbaTestCase):
    def setUp(self):
        pass

    def test_ODMEvents_DeviceManager(self):
        # Test DeviceManager related events

        # launch DomainManager
        nodebooter, self._domMgr = self.launchDomainManager()

        # connect to the channel
        domainName = scatest.getTestDomainName()
        odmChannel=None
        req = CF.EventChannelManager.EventRegistration( 'ODM_Channel', '')
        try:
            ecm = self._domMgr._get_eventChannelMgr()
            creg = ecm.registerResource( req ) 
            odmChannel = creg.channel
        except:
            pass

        if odmChannel == None:
            self.fail("Could not connect to the ODM_Channel")

        #channelManager = ChannelManager(self._orb)
        #odmChannel = channelManager.getEventChannel('ODM_Channel', domainName)

        # set up consumer
        consumer_admin = odmChannel.for_consumers()
        _proxy_supplier = consumer_admin.obtain_push_supplier()
        _consumer = ConsumerODM_i(self)
        _proxy_supplier.connect_push_consumer(_consumer._this())

        # start the device manager
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        timeout = 0.0
        while ((_consumer.getAddedEventCount('DEVICE_MANAGER') < 1 or _consumer.getAddedEventCount('DEVICE') < 1) and timeout < 2):
            timeout += 0.2
            time.sleep(0.2)

        try:
            devMgr.shutdown()
        except CORBA.Exception:
            pass

        timeout = 0.0
        while ((_consumer.getRemovedEventCount('DEVICE_MANAGER') < 1 or _consumer.getRemovedEventCount('DEVICE') < 1) and timeout < 2):
            timeout += 0.2
            time.sleep(0.2)

        # start the second device manager
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_BasicService_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        timeout = 0.0
        while ((_consumer.getAddedEventCount('DEVICE_MANAGER') < 2 or _consumer.getAddedEventCount('SERVICE') < 1) and timeout < 2):
            timeout += 0.2
            time.sleep(0.2)

        try:
            devMgr.shutdown()
        except CORBA.Exception:
            pass

        timeout = 0.0
        while ((_consumer.getRemovedEventCount('DEVICE_MANAGER') < 2 or _consumer.getRemovedEventCount('SERVICE') < 1) and timeout < 2):
            timeout += 0.2
            time.sleep(0.2)


        self.assertEqual(_consumer.getAddedEventCount('DEVICE_MANAGER'), 2)
        self.assertEqual(_consumer.getAddedEventCount('DEVICE'), 1)
        self.assertEqual(_consumer.getAddedEventCount('SERVICE'), 1)
        self.assertEqual(_consumer.getRemovedEventCount('DEVICE_MANAGER'), 2)
        self.assertEqual(_consumer.getRemovedEventCount('DEVICE'), 1)
        self.assertEqual(_consumer.getRemovedEventCount('SERVICE'), 1)

        self.terminateChild(devBooter)


class DomainEventReaderTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self._rhDom = redhawk.attach(scatest.getTestDomainName())
        self.assertEqual(len(self._rhDom._get_applications()), 0)

    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown, or failures will probably occur.
        redhawk.core._cleanUpLaunchedApps()
        scatest.CorbaTestCase.tearDown(self)
        # Need to let event service clean up event channels; cycle period is 10 milliseconds.
        time.sleep(0.1)

    def test_AddAndRemoveEvents(self):
        app = self._rhDom.createApplication('/waveforms/DomainEventReaderWaveform/DomainEventReaderWaveform.sad.xml')
        comp = app.comps[0]

        # Count Add and Remove events before starting 2nd device manager and device.
        def _waitFirstAddEvent():
            if comp.query([ CF.DataType( id='num_add_events', value=any.to_any(None)) ])[0].value._v:
                return True
            else:
                print('got num_add_events:  0')
        self.waitPredicate(_waitFirstAddEvent, 1)
        num_add_events = comp.query([ CF.DataType( id='num_add_events', value=any.to_any(None)) ])[0].value._v
        num_remove_events = comp.query([ CF.DataType( id='num_remove_events', value=any.to_any(None)) ])[0].value._v
        self.assertEqual(num_add_events, 1)  # the waveform
        self.assertEqual(num_remove_events, 0)

        _, self._devMgr_2 = self.launchDeviceManager("/nodes/DomainEventReader/DeviceManager.dcd.xml")
        self._devMgr_2.shutdown()

        # Count Add and Remove events after starting and stopping 2nd device manager and device.
        num_add_events = comp.query([ CF.DataType( id='num_add_events', value=any.to_any(None)) ])[0].value._v
        num_remove_events = comp.query([ CF.DataType( id='num_remove_events', value=any.to_any(None)) ])[0].value._v
        self.assertEqual(num_add_events, 3)  # the waveform, the 2nd device manager, the 2nd device
        self.assertEqual(num_remove_events, 2)  # the 2nd device manager, the 2nd device

