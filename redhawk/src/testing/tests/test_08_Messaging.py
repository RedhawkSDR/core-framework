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
from ossie.utils import sb
from ossie.properties import simple_property, simpleseq_property
import os
globalsdrRoot = os.environ['SDRROOT']

import CosEventChannelAdmin

class Foo(object):
    a = simple_property(id_="a",type_="string")
    b = simple_property(id_="b",type_="string")
    c = simple_property(id_="c",type_="string")

    def __init__(self, **kw):
        """Construct an initialized instance of this struct definition"""
        for classattr in type(self).__dict__.itervalues():
            if isinstance(classattr, (simple_property, simpleseq_property)):
                classattr.initialize(self)
        for k,v in kw.items():
            setattr(self,k,v)

    def __str__(self):
        """Return a string representation of this structure"""
        d = {}
        d["a"] = self.a
        d["b"] = self.b
        d["c"] = self.c
        return str(d)

    @classmethod
    def getId(cls):
        return "foo"

    @classmethod
    def isStruct(cls):
        return True

    def getMembers(self):
        return [("a",self.a),("b",self.b),("c",self.c)]

class MessagMarshalErrorTest(scatest.CorbaTestCase):
    def setUp(self):
        sb.setDEBUG(False)
        self.test_comp = "Sandbox"
        # Flagrant violation of sandbox API: if the sandbox singleton exists,
        # clean up previous state and dispose of it.
        if sb.domainless._sandbox:
            sb.domainless._sandbox.shutdown()
            sb.domainless._sandbox = None
        self.rcv_msg = None

    def tearDown(self):
        sb.domainless._getSandbox().shutdown()
        sb.setDEBUG(False)
        os.environ['SDRROOT'] = globalsdrRoot

    @scatest.requireLog4cxx
    def test_MessageMarshalCpp(self):
        snk=sb.MessageSink('')
        c=sb.launch('huge_msg_cpp', execparams={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/logconfig.cfg'})
        c.connect(snk)
        sb.start()
        time.sleep(1)
        fp = None
        try:
            fp = open('foo/bar/test.log','r')
        except:
            pass
        if fp != None:
            log_contents = fp.read()
            fp.close()
        try:
            os.remove('foo/bar/test.log')
        except:
            pass
        try:
            os.rmdir('foo/bar')
        except:
            pass
        try:
            os.rmdir('foo')
        except:
            pass
        number_warnings = log_contents.count('Could not deliver the message. Maximum message size exceeded')
        self.assertEquals(number_warnings, 2)

    @scatest.requireJava
    def test_MessageMarshalJava(self):
        snk=sb.MessageSink('')
        c=sb.launch('huge_msg_java', execparams={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/logconfig.cfg'})
        c.connect(snk)
        sb.start()
        time.sleep(3)
        fp = None
        try:
            fp = open('foo/bar/test.log','r')
        except:
            pass
        if fp != None:
            log_contents = fp.read()
            fp.close()
        try:
            os.remove('foo/bar/test.log')
        except:
            pass
        try:
            os.rmdir('foo/bar')
        except:
            pass
        try:
            os.rmdir('foo')
        except:
            pass
        number_warnings = log_contents.count('Could not deliver the message. Maximum message size exceeded')
        self.assertEquals(number_warnings, 2)

    def test_MessageMarshalPython(self):
        snk=sb.MessageSink('')
        c=sb.launch('huge_msg_python', execparams={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/logconfig.cfg'})
        c.connect(snk)
        sb.start()
        time.sleep(1)
        fp = None
        try:
            fp = open('foo/bar/test.log','r')
        except:
            pass
        if fp != None:
            log_contents = fp.read()
            fp.close()
        try:
            os.remove('foo/bar/test.log')
        except:
            pass
        try:
            os.rmdir('foo/bar')
        except:
            pass
        try:
            os.rmdir('foo')
        except:
            pass
        number_warnings = log_contents.count('Could not deliver the message. Maximum message size exceeded')
        self.assertEquals(number_warnings, 2)

class MessagingCompatibilityTest(scatest.CorbaTestCase):
    def setUp(self):
        sb.setDEBUG(False)
        self.test_comp = "Sandbox"
        # Flagrant violation of sandbox API: if the sandbox singleton exists,
        # clean up previous state and dispose of it.
        if sb.domainless._sandbox:
            sb.domainless._sandbox.shutdown()
            sb.domainless._sandbox = None
        self.rcv_msg = None

    def tearDown(self):
        sb.domainless._getSandbox().shutdown()
        sb.setDEBUG(False)
        os.environ['SDRROOT'] = globalsdrRoot

    def callback(self, _id, _data):
        self.rcv_msg = _data

    def test_MessagingPython(self):
        src=sb.MessageSource('foo')
        c=sb.launch('msg_through')
        src.connect(c)
        snk=sb.MessageSink('foo',Foo, self.callback)
        c.connect(snk)
        sb.start()
        src.sendMessage({'a':'A','b':'B','c':'C', 'd':'D'})
        _begin = time.time()
        _now = time.time()
        while _now - _begin < 5:
            if self.rcv_msg != None:
                break
            _now = time.time()
        self.assertEquals(self.rcv_msg.a, 'A')
        self.assertEquals(self.rcv_msg.b, 'B')
        self.assertEquals(self.rcv_msg.c, 'C')
        self.rcv_msg = None
        src.sendMessage({'a':'A','b':'B','c':'C'})
        while _now - _begin < 5:
            if self.rcv_msg != None:
                break
            _now = time.time()
        self.assertEquals(self.rcv_msg.a, 'A')
        self.assertEquals(self.rcv_msg.b, 'B')
        self.assertEquals(self.rcv_msg.c, 'C')
        self.rcv_msg = None
        src.sendMessage({'a':'A','c':'C'})
        while _now - _begin < 5:
            if self.rcv_msg != None:
                break
            _now = time.time()
        self.assertEquals(self.rcv_msg.a, 'A')
        self.assertEquals(self.rcv_msg.b, None)
        self.assertEquals(self.rcv_msg.c, 'C')
        self.rcv_msg = None

    def test_MessagingCpp(self):
        src=sb.MessageSource('foo')
        c=sb.launch('msg_through_cpp')
        src.connect(c)
        snk=sb.MessageSink('foo',Foo, self.callback)
        c.connect(snk)
        sb.start()
        src.sendMessage({'a':'A','b':'B','c':'C', 'd':'D'})
        _begin = time.time()
        _now = time.time()
        while _now - _begin < 5:
            if self.rcv_msg != None:
                break
            _now = time.time()
        self.assertEquals(self.rcv_msg.a, 'A')
        self.assertEquals(self.rcv_msg.b, 'B')
        self.assertEquals(self.rcv_msg.c, 'C')
        self.rcv_msg = None
        src.sendMessage({'a':'A','b':'B','c':'C'})
        while _now - _begin < 5:
            if self.rcv_msg != None:
                break
            _now = time.time()
        self.assertEquals(self.rcv_msg.a, 'A')
        self.assertEquals(self.rcv_msg.b, 'B')
        self.assertEquals(self.rcv_msg.c, 'C')
        self.rcv_msg = None
        src.sendMessage({'a':'A','c':'C'})
        while _now - _begin < 5:
            if self.rcv_msg != None:
                break
            _now = time.time()
        self.assertEquals(self.rcv_msg.a, 'A')
        self.assertEquals(self.rcv_msg.b, '')
        self.assertEquals(self.rcv_msg.c, 'C')
        self.rcv_msg = None

    @scatest.requireJava
    def test_MessagingJava(self):
        src=sb.MessageSource('foo')
        c=sb.launch('msg_through_java')
        src.connect(c)
        snk=sb.MessageSink('foo',Foo, self.callback)
        c.connect(snk)
        sb.start()
        src.sendMessage({'a':'A','b':'B','c':'C', 'd':'D'})
        _begin = time.time()
        _now = time.time()
        while _now - _begin < 5:
            if self.rcv_msg != None:
                break
            _now = time.time()
        self.assertEquals(self.rcv_msg.a, 'A')
        self.assertEquals(self.rcv_msg.b, 'B')
        self.assertEquals(self.rcv_msg.c, 'C')
        self.rcv_msg = None
        src.sendMessage({'a':'A','b':'B','c':'C'})
        while _now - _begin < 5:
            if self.rcv_msg != None:
                break
            _now = time.time()
        self.assertEquals(self.rcv_msg.a, 'A')
        self.assertEquals(self.rcv_msg.b, 'B')
        self.assertEquals(self.rcv_msg.c, 'C')
        self.rcv_msg = None
        src.sendMessage({'a':'A','c':'C'})
        while _now - _begin < 5:
            if self.rcv_msg != None:
                break
            _now = time.time()
        self.assertEquals(self.rcv_msg.a, 'A')
        self.assertEquals(self.rcv_msg.b, None)
        self.assertEquals(self.rcv_msg.c, 'C')
        self.rcv_msg = None

    def __MessageCycle(self, comp):
        source = sb.MessageSource(messageId='test_message',messageFormat={'item_float':'float','item_string':'string'})

        source.connect(comp)
        source.start()

        # As a pre-condition, there should have been no messages yet
        self.assertEqual(comp._get_started(), False)
        self.assertEqual(comp.received_messages, [])

        source.sendMessage({'item_float':0.0, 'item_string':'first'})

        # Wait until the component has time to process the message
        timeout = time.time() + 1.0
        while len(comp.received_messages) == 0 and time.time() < timeout:
            time.sleep(0.1)

        self.assertEqual(len(comp.received_messages), 1)
        self.assertEqual(comp.received_messages[0][:14], "test_message,0")

        comp.start()
        self.assertEqual(comp._get_started(), True)
        self.assertEqual(len(comp.received_messages), 1)
        source.sendMessage({'item_float':1.0, 'item_string':'first'})

        # Wait until the component has time to process the message
        timeout = time.time() + 1.0
        while len(comp.received_messages) == 1 and time.time() < timeout:
            time.sleep(0.1)

        self.assertEqual(len(comp.received_messages), 2)
        self.assertEqual(comp.received_messages[1][:14], "test_message,1")

        comp.stop()
        self.assertEqual(comp._get_started(), False)
        self.assertEqual(len(comp.received_messages), 2)
        source.sendMessage({'item_float':2.0, 'item_string':'first'})

        # Wait until the component has time to process the message
        timeout = time.time() + 1.0
        while len(comp.received_messages) == 2 and time.time() < timeout:
            time.sleep(0.1)

        self.assertEqual(len(comp.received_messages), 3)
        self.assertEqual(comp.received_messages[2][:14], "test_message,2")

    def test_MessageCyclePython(self):
        comp = sb.launch('MessageReceiverPy')
        self.__MessageCycle(comp)

    @scatest.requireJava
    def test_MessageCycleJava(self):
        comp = sb.launch('EventReceive')
        self.__MessageCycle(comp)

    def test_MessageCycleCpp(self):
        comp = sb.launch('MessageReceiverCpp')
        self.__MessageCycle(comp)

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

        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/MessageTestPy/MessageTestPy.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        app.start()
        time.sleep(2)
        components = app._get_registeredComponents()
        for component in components:
            print component.componentObject._get_identifier()
            if 'MessageReceiverPy_1' in component.componentObject._get_identifier():
                stuff = component.componentObject.query([])
        recval = any.from_any(stuff[0].value)
        self.assertEquals(6, len(recval))
        for val in recval:
            self.assertEquals('test_message' in val, True)
        app.releaseObject()

    def test_QueryablePortPython(self):
        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/MessageTestPy/MessageTestPy.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        for component in app._get_registeredComponents():
            if 'MessageSenderPy_1' in component.componentObject._get_identifier():
                sender_port = component.componentObject.getPort('message_out')
            elif 'MessageReceiverPy_1' in component.componentObject._get_identifier():
                receiver_port = component.componentObject.getPort('message_in')

        # There should be two connections, one to the receiver's port and
        # another to the event channel
        connections = sender_port._get_connections()
        self.assertEqual(len(connections), 2)
        for connection in connections:
            if 'direct' in connection.connectionId:
                self.assertTrue(connection.port._is_equivalent(receiver_port))
            else:
                channel = connection.port._narrow(CosEventChannelAdmin.EventChannel)
                self.assertNotEqual(channel, None)
        app.releaseObject()
