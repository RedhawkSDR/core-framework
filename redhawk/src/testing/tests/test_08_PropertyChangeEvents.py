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
import commands
import ossie.properties as properties
import CosEventComm,CosEventComm__POA
import CosEventChannelAdmin, CosEventChannelAdmin__POA
from ossie.cf import StandardEvent
from ossie.events import ChannelManager
from _unitTestHelpers import runtestHelpers

java_support = runtestHelpers.haveJavaSupport('../Makefile')

# create a class for consuming events
class Consumer_i(CosEventComm__POA.PushConsumer):
    def __init__(self, parent):
        #self.event = threading.Event()
        self.parent = parent

    def push(self, data_obj):
        data = data_obj.value()
        self.parent.eventReceived(data_obj)

    def disconnect_push_consumer (self):
        pass

class PropertyChangeEventsTest(scatest.CorbaTestCase):
    def setUp(self):
        pass

    def eventReceived(self, data):
        dataEvent = any.from_any(data, keep_structs=True)
        if dataEvent.sourceId != self.sourceId:
            self.successfullPropChange = False
            return
        if len(dataEvent.properties) == 0:
            self.successfullPropChange = False
            return

        props = properties.props_to_dict(dataEvent.properties)
        if props.has_key('myprop'):
            self.received_myprop = True
            if self.successfullPropChange == None:
                self.successfullPropChange = True
        if props.has_key('anotherprop'):
            self.received_anotherprop = True
            if self.successfullPropChange == None:
                self.successfullPropChange = True
        if props.has_key('seqprop'):
            if props['seqprop'] == [1.0, 2.0, 3.0]:
                self.received_seqprop = True
                if self.successfullPropChange == None:
                    self.successfullPropChange = True
        if props.has_key('some_struct') and props['some_struct'] != None:
            if props['some_struct'] == {"some_number": 3.0, "some_string": "hello"}:
                self.received_some_struct = True
                if self.successfullPropChange == None:
                    self.successfullPropChange = True
        if props.has_key('structseq_prop'):
            if len(props['structseq_prop']) == 1:
                if props['structseq_prop'][0] == {"some_number": 7.0, "some_string": "second message"}:
                    self.received_structseq_prop = True
                    if self.successfullPropChange == None:
                        self.successfullPropChange = True

    def test_PropertyChangeEvents_Py(self):
        self.received_myprop = False
        self.received_anotherprop = False
        self.received_seqprop = False
        self.received_some_struct = False
        self.received_structseq_prop = False
        self.sourceId = 'prop_change_instance:PropertyChangeEvents_1'
        self.successfullPropChange = None
        self.set = 1
        # Test DeviceManager related events

        # launch DomainManager
        nodebooter, self._domMgr = self.launchDomainManager()
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self._domMgr.installApplication("/waveforms/PropertyChangeEvents/PropertyChangeEvents.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)
        appFact = self._domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])

        # set up consumer
        _consumer = Consumer_i(self)
        self._domMgr.registerWithEventChannel(_consumer._this(), 'some_id', 'propertyChanges')

        app.configure([CF.DataType(id='myprop',value=any.to_any(2))])
        app.configure([CF.DataType(id='anotherprop',value=any.to_any(2))])
        sequence = CF.DataType(id='seqprop',value=any.to_any([1.0, 2.0, 3.0]))
        sequence.value._t._d = (19,6,0) # change the type from double to float
        some_struct = CF.DataType(id='some_struct',value=any.to_any([CF.DataType(id='some_number',value=any.to_any(3.0)),CF.DataType(id='some_string',value=any.to_any('hello'))]))
        app.configure([sequence])
        app.configure([some_struct])
        structseq_prop = CF.DataType(id='structseq_prop',value=any.to_any([any.to_any([CF.DataType(id='some_number',value=any.to_any(7.0)),CF.DataType(id='some_string',value=any.to_any('second message'))])]))
        app.configure([structseq_prop])
        app.configure([structseq_prop])

        # Ticket #1061: Ensure that eventable structs and struct sequences with
        # 'None' values don't cause exceptions inside the port code.
        results = app.runTest(1061, [])

        # Convert test results into a dictionary
        results = dict((r.id, any.from_any(r.value)) for r in results)

        time.sleep(1)
        self.assertEqual(self.received_myprop, True)
        self.assertEqual(self.received_anotherprop, False)
        self.assertEqual(self.received_seqprop, True)
        self.assertEqual(self.received_some_struct, True)
        self.assertEqual(self.received_structseq_prop, True)
        self.assertEqual(self.successfullPropChange, True)
        self.assertEqual(results['1061'], True)
        self._domMgr.unregisterFromEventChannel('some_id', 'propertyChanges')

    def test_PropertyChangeEvents_Cpp(self):
        self.received_myprop = False
        self.received_anotherprop = False
        self.received_seqprop = False
        self.received_some_struct = False
        self.received_structseq_prop = False
        self.sourceId = 'prop_change_instance:PropertyChangeEventsCpp_1'
        self.successfullPropChange = None
        self.set = 1
        # Test DeviceManager related events

        # launch DomainManager
        nodebooter, self._domMgr = self.launchDomainManager()
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self._domMgr.installApplication("/waveforms/PropertyChangeEventsCpp/PropertyChangeEventsCpp.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)
        appFact = self._domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])

        # set up consumer
        _consumer = Consumer_i(self)
        self._domMgr.registerWithEventChannel(_consumer._this(), 'some_id', 'propertyChanges')

        app.configure([CF.DataType(id='myprop',value=any.to_any(2))])
        app.configure([CF.DataType(id='anotherprop',value=any.to_any(2))])
        sequence = CF.DataType(id='seqprop',value=any.to_any([1.0, 2.0, 3.0]))
        sequence.value._t._d = (19,6,0) # change the type from double to float
        some_struct = CF.DataType(id='some_struct',value=any.to_any([CF.DataType(id='some_number',value=any.to_any(3.0)),CF.DataType(id='some_string',value=any.to_any('hello'))]))
        app.configure([sequence])
        app.configure([some_struct])
        structseq_prop = CF.DataType(id='structseq_prop',value=any.to_any([any.to_any([CF.DataType(id='some_number',value=any.to_any(7.0)),CF.DataType(id='some_string',value=any.to_any('second message'))])]))
        app.configure([structseq_prop])
        app.configure([structseq_prop])
        time.sleep(1)
        self.assertEqual(self.received_myprop, True)
        self.assertEqual(self.received_anotherprop, True)
        self.assertEqual(self.received_seqprop, True)
        self.assertEqual(self.received_some_struct, True)
        self.assertEqual(self.received_structseq_prop, True)
        self.assertEqual(self.successfullPropChange, True)
        self._domMgr.unregisterFromEventChannel('some_id', 'propertyChanges')

    def test_PropertyChangeEvents_Java(self):
        if not java_support:
            return
        self.received_myprop = False
        self.received_anotherprop = False
        self.received_seqprop = False
        self.received_some_struct = False
        self.received_structseq_prop = False
        self.sourceId = 'prop_change_instance:PropertyChangeEventsJava_1'
        self.successfullPropChange = None
        self.set = 1
        # Test DeviceManager related events

        # launch DomainManager
        nodebooter, self._domMgr = self.launchDomainManager()
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self._domMgr.installApplication("/waveforms/PropertyChangeEventsJava/PropertyChangeEventsJava.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)
        appFact = self._domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])

        # set up consumer
        _consumer = Consumer_i(self)
        self._domMgr.registerWithEventChannel(_consumer._this(), 'some_id', 'propertyChanges')

        app.configure([CF.DataType(id='myprop',value=any.to_any(2))])
        app.configure([CF.DataType(id='anotherprop',value=any.to_any(2))])
        sequence = CF.DataType(id='seqprop',value=any.to_any([1.0, 2.0, 3.0]))
        some_struct = CF.DataType(id='some_struct',value=any.to_any([CF.DataType(id='some_number',value=any.to_any(3.0)),CF.DataType(id='some_string',value=any.to_any('hello'))]))
        structseq_prop = CF.DataType(id='structseq_prop',value=any.to_any([any.to_any([CF.DataType(id='some_number',value=any.to_any(7.0)),CF.DataType(id='some_string',value=any.to_any('second message'))])]))
        sequence.value._t._d = (19,6,0) # change the type from double to float
        app.configure([sequence])
        app.configure([some_struct])
        app.configure([structseq_prop])
        app.configure([structseq_prop])
        time.sleep(1)
        self.assertEqual(self.received_myprop, True)
        self.assertEqual(self.received_anotherprop, True)
        self.assertEqual(self.received_seqprop, True)
        self.assertEqual(self.received_some_struct, True)
        self.assertEqual(self.received_structseq_prop, True)
        self.assertEqual(self.successfullPropChange, True)
        self._domMgr.unregisterFromEventChannel('some_id', 'propertyChanges')
