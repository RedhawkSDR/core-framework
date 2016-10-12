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
import scatest
from omniORB import URI, any
from ossie.cf import CF
import threading
import time
try:
    import CosEventComm,CosEventComm__POA
    import CosEventChannelAdmin, CosEventChannelAdmin__POA
    from ossie.cf import StandardEvent
    hasEvents = True
except:
    hasEvents = False


class Supplier_i(CosEventComm__POA.PushSupplier):
    def disconnect_push_supplier (self):
        pass

class Consumer_i(CosEventComm__POA.PushConsumer):
    def __init__(self, parent):
        self.parent = parent
    
    def push(self, data):
        if data._v == "bye":
            self.parent.eventFlag = True
            self.parent.localEvent.set()
    
    def disconnect_push_consumer (self):
        pass

class ConsumerDevice_i(CosEventComm__POA.PushConsumer):
    def __init__(self, parent):
        self.parent = parent
    
    def push(self, data):
        if data._v == "bye device":
            self.parent.eventFlag = True
            self.parent.localEvent.set()
    
    def disconnect_push_consumer (self):
        pass


class EventPortConnectionsTest(scatest.CorbaTestCase):
    def setUp(self):
        self._domBooter, self._domMgr = self.launchDomainManager(debug=9)

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

    def test_EventDevicePortConnectionFromPython(self):
        self.localEvent = threading.Event()
        self.eventFlag = False
        
        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr, debug=9)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/MessageTestPyCpp/MessageTestPyCpp.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        app.start() # kick off events
        time.sleep(2)
        components = app._get_registeredComponents()
        for component in components:
            print component.componentObject._get_identifier()
            if 'DCE:b1fe6cc1-2562-4878-9a69-f191f89a6ef8' in component.componentObject._get_identifier():
                stuff = component.componentObject.query([])
        recval = any.from_any(stuff[0].value)
        self.assertEquals(6, len(recval))
        for val in recval:
            self.assertEquals('test_message' in val, True)
        app.releaseObject() # kill producer/consumer

    def test_EventDevicePortConnectionCppOnly(self):
        self.localEvent = threading.Event()
        self.eventFlag = False
        
        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr, debug=9)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/MessageTestCpp/MessageTestCpp.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        app.start() # kick off events
        time.sleep(2)
        components = app._get_registeredComponents()
        for component in components:
            print component.componentObject._get_identifier()
            if 'DCE:b1fe6cc1-2562-4878-9a69-f191f89a6ef8' in component.componentObject._get_identifier():
                stuff = component.componentObject.query([])
        recval = any.from_any(stuff[0].value)
        self.assertEquals(6, len(recval))
        for val in recval:
            self.assertEquals('test_message' in val, True)
        app.releaseObject() # kill producer/consumer


# Disable the tests entirely if events are not enabled.
if scatest.getBuildDefineValue("ENABLE_EVENTS") in (None, "0"):
    del EventPortConnectionsTest


