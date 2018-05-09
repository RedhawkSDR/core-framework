#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK core.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import threading
import time
import unittest

from omniORB.any import to_any, from_any

from ossie.cf import CF
from ossie.events import MessageSupplierPort, MessageConsumerPort
from ossie.properties import simple_property, simpleseq_property, props_to_any

class PortManager(object):
    def __init__(self):
        self._ports = []

    def addPort(self, port):
        self._ports.append(port)
        poa = port._default_POA()
        poa.activate_object(port)

    def startPorts(self):
        for port in self._ports:
            if hasattr(port, 'startPort'):
                port.startPort()

    def stopPorts(self):
        for port in self._ports:
            if hasattr(port, 'stopPort'):
                port.stopPort()

    def releasePorts(self):
        for port in self._ports:
            try:
                self.deactivatePort(port)
            except:
                # Ignore CORBA exceptions
                pass
        self._ports = []

    def deactivatePort(self, servant):
        poa = servant._default_POA()
        object_id = poa.servant_to_id(servant)
        poa.deactivate_object(object_id)

    def start(self):
        self.startPorts()

    def stop(self):
        self.stopPorts()

    def releaseObject(self):
        self.releasePorts()


# REDHAWK 2.1 generated message structs
class BasicMessage(object):
    value = simple_property(
                            id_="basic_message::value",
                            name="value",
                            type_="long")

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
        d["value"] = self.value
        return str(d)

    @classmethod
    def getId(cls):
        return "basic_message"

    @classmethod
    def isStruct(cls):
        return True

    def getMembers(self):
        return [("value",self.value)]

class TestMessage(object):
    item_float = simple_property(
                                 id_="test_message::item_float",
                                 name="item_float",
                                 type_="float")

    item_string = simple_property(
                                  id_="test_message::item_string",
                                  name="item_string",
                                  type_="string")

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
        d["item_float"] = self.item_float
        d["item_string"] = self.item_string
        return str(d)

    @classmethod
    def getId(cls):
        return "test_message"

    @classmethod
    def isStruct(cls):
        return True

    def getMembers(self):
        return [("item_float",self.item_float),("item_string",self.item_string)]


class MessageReceiver(object):
    def __init__(self):
        self.messages = []
        self._lock = threading.Lock()
        self._cond = threading.Condition(self._lock)

    def messageReceived(self, msgId, msgData):
        with self._lock:
            self.messages.append(msgData)
            self._cond.notify()

    def waitMessages(self, count, timeout):
        end = time.time() + timeout
        with self._lock:
            while True:
                if len(self.messages) >= count:
                    return True
                now = time.time()
                if now >= end:
                    return False
                self._cond.wait(end - now)


class MessagingTest(unittest.TestCase):
    def setUp(self):
        self._portManager = PortManager()
        self._supplier = MessageSupplierPort()
        self._consumer = MessageConsumerPort()

        self._portManager.addPort(self._supplier)
        self._portManager.addPort(self._consumer)

        # Connect the supplier and consumer
        objref = self._consumer._this()
        self._supplier.connectPort(objref, "connection_1")

        # Simulate component start
        self._portManager.start()

    def tearDown(self):
        self._portManager.stop()
        self._portManager.releaseObject()

    def testSendMessage(self):
        receiver = MessageReceiver()
        self._consumer.registerMessage("basic_message", BasicMessage, receiver.messageReceived)

        msg = BasicMessage()
        msg.value = 1

        self._supplier.sendMessage(msg)

        # Unlike C++, the Python message consumer is threaded, so we need to
        # give it some time to receive the message
        self.failUnless(receiver.waitMessages(1, 1.0))

        self.assertEqual("basic_message", receiver.messages[0].getId())
        self.assertEqual(msg.value, receiver.messages[0].value)

    def testSendMessageConnectionId(self):
        # Create and connect a second consumer port
        consumer_2 = MessageConsumerPort()
        self._portManager.addPort(consumer_2)
        objref = consumer_2._this()
        self._supplier.connectPort(objref, "connection_2")

        # Set up 2 receivers to distinguish which connection received a message
        receiver_1 = MessageReceiver()
        self._consumer.registerMessage("basic_message", BasicMessage, receiver_1.messageReceived)

        receiver_2 = MessageReceiver()
        consumer_2.registerMessage("basic_message", BasicMessage, receiver_2.messageReceived)

        # Target the first connection
        msg = BasicMessage()
        msg.value = 1
        self.assertEqual(0, len(receiver_1.messages))
        self.assertEqual(0, len(receiver_2.messages))
        self._supplier.sendMessage(msg, connectionId="connection_1")

        self.failUnless(receiver_1.waitMessages(1, 1.0))
        self.assertEqual("basic_message", receiver_1.messages[0].getId())
        self.assertEqual(1, receiver_1.messages[0].value)

        # Second should not receive it (give it a little time just in case)
        self.failIf(receiver_2.waitMessages(1, 0.1))

        # Target the second connection this time
        msg.value = 2
        self._supplier.sendMessage(msg, connectionId="connection_2")

        self.failUnless(receiver_2.waitMessages(1, 1.0))
        self.assertEqual("basic_message", receiver_2.messages[0].getId())
        self.assertEqual(2, receiver_2.messages[0].value)

        # This time, the first should not receive it
        self.failIf(receiver_1.waitMessages(2, 0.1))

        # Target both connections
        msg.value = 3
        self._supplier.sendMessage(msg)

        self.failUnless(receiver_1.waitMessages(2, 1.0))
        self.failUnless(receiver_2.waitMessages(2, 1.0))

        # Target invalid connection
        msg.value = 4
        self.assertRaises(ValueError, self._supplier.sendMessage, msg, "bad_connection")

    def testSendMessages(self):
        receiver = MessageReceiver()
        self._consumer.registerMessage("basic_message", BasicMessage, receiver.messageReceived)
        self._consumer.registerMessage("test_message", TestMessage, receiver.messageReceived)

        # Send two different message types in one batch
        messages = [BasicMessage(value=1),
                    TestMessage(item_float=2.0, item_string="two"),
                    BasicMessage(value=3)]

        self.assertEqual(0, len(receiver.messages))
        self._supplier.sendMessages(messages)
        self.failUnless(receiver.waitMessages(len(messages), 1.0))
        self.assertEqual('basic_message', receiver.messages[0].getId())
        self.assertEqual(1, receiver.messages[0].value)
        self.assertEqual('test_message', receiver.messages[1].getId())
        self.assertEqual(2.0, receiver.messages[1].item_float)
        self.assertEqual("two", receiver.messages[1].item_string)
        self.assertEqual('basic_message', receiver.messages[2].getId())
        self.assertEqual(3, receiver.messages[2].value)

    def testSendMessagesConnectionId(self):
        # Create and connect a second consumer port
        consumer_2 = MessageConsumerPort()
        self._portManager.addPort(consumer_2)
        objref = consumer_2._this()
        self._supplier.connectPort(objref, "connection_2")

        # Set up 2 receivers to distinguish which connection received a message
        receiver_1 = MessageReceiver()
        self._consumer.registerMessage("basic_message", BasicMessage, receiver_1.messageReceived)

        receiver_2 = MessageReceiver()
        consumer_2.registerMessage("basic_message", BasicMessage, receiver_2.messageReceived)

        # Target first connection
        messages_1 = [BasicMessage(value=x) for x in xrange(2)]
        self.assertEqual(0, len(receiver_1.messages))
        self.assertEqual(0, len(receiver_2.messages))
        self._supplier.sendMessages(messages_1, "connection_1")

        # Wait for the first receiver to get all messages; the second receiver
        # ought to get none (give it some time due to threading)
        self.failUnless(receiver_1.waitMessages(2, 1.0))
        self.assertEqual(2, len(receiver_1.messages))
        self.failIf(receiver_2.waitMessages(1, 0.1))

        # Target the second connection this time with a different set of
        # messages
        messages_2 = [BasicMessage(value=x) for x in xrange(2,5)]
        self._supplier.sendMessages(messages_2, "connection_2")

        # Wait for the second receiver to get all the messages (and check at
        # least the first value)
        self.failUnless(receiver_2.waitMessages(len(messages_2), 1.0))
        self.assertEqual(3, len(receiver_2.messages))
        self.assertEqual(2, receiver_2.messages[0].value)
        self.failIf(receiver_1.waitMessages(3, 0.1))

        # Target both connections
        self._supplier.sendMessages(messages_1)
        self.failUnless(receiver_1.waitMessages(4, 1.0))
        self.assertEqual(4, len(receiver_1.messages))
        self.failUnless(receiver_2.waitMessages(5, 1.0))
        self.assertEqual(5, len(receiver_2.messages))

        # Target invalid connection
        messages_3 = [BasicMessage(value=5)]
        self.assertRaises(ValueError, self._supplier.sendMessages, messages_3, "bad_connection")
        self.failIf(receiver_1.waitMessages(5, 0.1))
        self.failIf(receiver_2.waitMessages(6, 0.1))

    def testPush(self):
        receiver = MessageReceiver()
        self._consumer.registerMessage(None, None, receiver.messageReceived)

        # Pack the messages ourselves
        messages = []
        messages.append(CF.DataType('first', to_any(100)))
        messages.append(CF.DataType('second', to_any('some text')))
        messages.append(CF.DataType('third', to_any(0.25)))

        self._supplier.push(props_to_any(messages))

        self.failUnless(receiver.waitMessages(3, 1.0))
        self.assertEqual(3, len(receiver.messages))
        self.assertEqual(100, from_any(receiver.messages[0].value))
        self.assertEqual('some text', from_any(receiver.messages[1].value))
        self.assertEqual(0.25, from_any(receiver.messages[2].value))

    def testPushConnectionId(self):
        # Create and connect a second consumer port
        consumer_2 = MessageConsumerPort()
        self._portManager.addPort(consumer_2)
        objref = consumer_2._this()
        self._supplier.connectPort(objref, 'connection_2')

        # Set up 2 receivers to distinguish which connection received a message
        receiver_1 = MessageReceiver()
        self._consumer.registerMessage(None, None, receiver_1.messageReceived)

        receiver_2 = MessageReceiver()
        consumer_2.registerMessage(None, None, receiver_2.messageReceived)

        # Pack the messages ourselves and target the first connection
        messages_1 = []
        messages_1.append(CF.DataType('first', to_any(100)))
        messages_1.append(CF.DataType('second', to_any('some text')))
        messages_1.append(CF.DataType('third', to_any(0.25)))

        self._supplier.push(props_to_any(messages_1), 'connection_1')
        self.failUnless(receiver_1.waitMessages(3, 1.0))
        self.assertEqual(3, len(receiver_1.messages))
        self.failIf(receiver_2.waitMessages(1, 0.1))

        # Target the second connection with a different set of messages
        messages_2 = []
        messages_2.append(CF.DataType('one', to_any('abc')))
        messages_2.append(CF.DataType('two', to_any(False)))
        messages_2 = props_to_any(messages_2)
        self._supplier.push(messages_2, "connection_2")

        self.failUnless(receiver_2.waitMessages(2, 1.0))
        self.assertEqual(2, len(receiver_2.messages))
        self.failIf(receiver_1.waitMessages(4, 0.1))

        # Target both connections with yet another set of messages
        messages_3 = props_to_any([CF.DataType('all', to_any(3))])
        self._supplier.push(messages_3)
        self.failUnless(receiver_2.waitMessages(3, 1.0))
        self.assertEqual(3, len(receiver_2.messages))
        self.failUnless(receiver_1.waitMessages(4, 1.0))
        self.assertEqual(4, len(receiver_1.messages))

        # Target invalid connection
        messages_4 = props_to_any([CF.DataType('bad', to_any('bad_connection'))])
        self.assertRaises(ValueError, self._supplier.push, messages_4, 'bad_connection')
        self.failIf(receiver_2.waitMessages(4, 0.1))
        self.failIf(receiver_1.waitMessages(5, 0.1))
        self.assertEqual(3, len(receiver_2.messages))
        self.assertEqual(4, len(receiver_1.messages))

if __name__ == '__main__':
    import runtests
    runtests.main()
