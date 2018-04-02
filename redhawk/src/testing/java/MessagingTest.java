/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import org.ossie.events.MessageSupplierPort;
import org.ossie.events.MessageConsumerPort;
import org.ossie.events.MessageListener;
import org.ossie.properties.*;

import utils.Assert;

@RunWith(JUnit4.class)
public class MessagingTest {

    public static class basic_message_struct extends StructDef {
        public final LongProperty value =
            new LongProperty(
                "basic_message::value", //id
                "value", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
    
        public basic_message_struct(Integer value) {
            this();
            this.value.setValue(value);
        }
    
        public void set_value(Integer value) {
            this.value.setValue(value);
        }

        public Integer get_value() {
            return this.value.getValue();
        }
    
        public basic_message_struct() {
            addElement(this.value);
        }
    
        public String getId() {
            return "basic_message";
        }
    };

    public static class test_message_struct extends StructDef {
        public final FloatProperty item_float =
            new FloatProperty(
                "test_message::item_float", //id
                "item_float", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
        public final StringProperty item_string =
            new StringProperty(
                "test_message::item_string", //id
                "item_string", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
    
        public test_message_struct(Float item_float, String item_string) {
            this();
            this.item_float.setValue(item_float);
            this.item_string.setValue(item_string);
        }
    
        public void set_item_float(Float item_float) {
            this.item_float.setValue(item_float);
        }

        public Float get_item_float() {
            return this.item_float.getValue();
        }

        public void set_item_string(String item_string) {
            this.item_string.setValue(item_string);
        }

        public String get_item_string() {
            return this.item_string.getValue();
        }
    
        public test_message_struct() {
            addElement(this.item_float);
            addElement(this.item_string);
        }
    
        public String getId() {
            return "test_message";
        }
    };

    public static class MessageReceiver<E> implements MessageListener<E>
    {
        public void messageReceived(String msgId, E msgData)
        {
            messages.add(msgData);
        }

        public List<E> messages = new ArrayList<>();
    }

    private static class GenericReceiver implements MessageListener<org.omg.CORBA.Any>
    {
        public void messageReceived(String messageId, org.omg.CORBA.Any messageData)
        {
            messages.add(new CF.DataType(messageId, messageData));
        }

        List<CF.DataType> messages = new ArrayList<>();
    }

    @Before
    public void setUp() throws Exception
    {
        org.omg.CORBA.ORB orb = org.ossie.corba.utils.Init(new String[0], null);

        _supplier = new MessageSupplierPort("supplier");
        _portManager.addPort(_supplier);
        
        _consumer = new MessageConsumerPort("consumer");
        org.omg.CORBA.Object objref = _portManager.addPort(_consumer);

        _supplier.connectPort(objref, "connection_1");
        
        // Simulate component start
        _portManager.start();
    }

    @After
    public void tearDown()
    {
        // Simulate component stop/shutdown
        _portManager.stop();
        _portManager.releaseObject();
    }

    @Test
    public void testSendMessage()
    {
        MessageReceiver<basic_message_struct> receiver = new MessageReceiver<>();
        _consumer.registerMessage("basic_message", basic_message_struct.class, receiver);

        basic_message_struct msg = new basic_message_struct(1);

        // Send the message and check that it's received. Currently, the
        // consumer's message handler is called directly from sendMessage, so
        // this is a synchronous operation; however, if at some point in the
        // future, threaded message dispatch is added, this test will need to
        // be revisted.
        Assert.assertEquals(0, receiver.messages.size());
        _supplier.sendMessage(msg);
        Assert.assertEquals(1, receiver.messages.size());

        Assert.assertEquals("basic_message", receiver.messages.get(0).getId());
        Assert.assertEquals(1, (int) receiver.messages.get(0).get_value());
    }

    @Test
    public void testSendMessageConnectionId() throws Exception
    {
        // Create and connect a second consumer port
        MessageConsumerPort consumer_2 = new MessageConsumerPort("consumer_2");
        org.omg.CORBA.Object objref = _portManager.addPort(consumer_2);
        _supplier.connectPort(objref, "connection_2");
        
        // Set up 2 receivers to distinguish which connection received a
        // message
        MessageReceiver<basic_message_struct> receiver_1 = new MessageReceiver<>();
        _consumer.registerMessage("basic_message", basic_message_struct.class, receiver_1);

        MessageReceiver<basic_message_struct> receiver_2 = new MessageReceiver<>();
        consumer_2.registerMessage("basic_message", basic_message_struct.class, receiver_2);

        basic_message_struct msg = new basic_message_struct(1);

        // Target the first connection (see above re: threading)
        Assert.assertEquals(0, receiver_1.messages.size());
        Assert.assertEquals(0, receiver_2.messages.size());
        _supplier.sendMessage(msg, "connection_1");
        Assert.assertEquals(1, receiver_1.messages.size());
        Assert.assertEquals(0, receiver_2.messages.size());

        // Target the second connection this time
        msg = new basic_message_struct(2);
        _supplier.sendMessage(msg, "connection_2");
        Assert.assertEquals(1, receiver_1.messages.size());
        Assert.assertEquals(1, receiver_2.messages.size());

        // Target both connections
        msg = new basic_message_struct(3);
        _supplier.sendMessage(msg);
        Assert.assertEquals(2, receiver_1.messages.size());
        Assert.assertEquals(2, receiver_2.messages.size());

        // Target invalid connection
        final basic_message_struct msg_4 = new basic_message_struct(4);
        Assert.assertThrows(IllegalArgumentException.class, () -> _supplier.sendMessage(msg_4, "bad_connection"));
        Assert.assertEquals(2, receiver_1.messages.size());
        Assert.assertEquals(2, receiver_2.messages.size());
    }

    @Test
    public void testSendMessages()
    {
        // Use a generic message receiver that can handle any StructDef, and
        // register it for both message types
        MessageReceiver<StructDef> receiver = new MessageReceiver<>();
        _consumer.registerMessage("basic_message", basic_message_struct.class, receiver);
        _consumer.registerMessage("test_message", test_message_struct.class, receiver);

        // Send two different message types in one batch
        basic_message_struct msg_1 = new basic_message_struct(1);
        test_message_struct msg_2 = new test_message_struct(2.0f, "two");
        basic_message_struct msg_3 = new basic_message_struct(3);
        List<StructDef> messages = Arrays.asList(msg_1, msg_2, msg_3);

        Assert.assertEquals(0, receiver.messages.size());
        _supplier.sendMessages(messages);

        // StructDef implements .equals(), so it will catch if the fields are
        // different between input and output, and they are serialized through
        // Anys so the references are different
        Assert.assertEquals(3, receiver.messages.size());
        Assert.assertEquals(msg_1, receiver.messages.get(0));
        Assert.assertEquals(msg_2, receiver.messages.get(1));
        Assert.assertEquals(msg_3, receiver.messages.get(2));

    }

    @Test
    public void testSendMessagesConnectionId() throws Exception
    {
        // Create and connect a second consumer port
        MessageConsumerPort consumer_2 = new MessageConsumerPort("consumer_2");
        org.omg.CORBA.Object objref = _portManager.addPort(consumer_2);
        _supplier.connectPort(objref, "connection_2");

        // Set up 2 receivers to distinguish which connection received a
        // message
        MessageReceiver<basic_message_struct> receiver_1 = new MessageReceiver<>();
        _consumer.registerMessage("basic_message", basic_message_struct.class, receiver_1);

        MessageReceiver<basic_message_struct> receiver_2 = new MessageReceiver<>();
        consumer_2.registerMessage("basic_message", basic_message_struct.class, receiver_2);

        // Build a list of messages and send to the first connection
        List<StructDef> messages_1 = Arrays.asList(new basic_message_struct(1),
                                                   new basic_message_struct(2));
        Assert.assertEquals(0, receiver_1.messages.size());
        Assert.assertEquals(0, receiver_2.messages.size());
        _supplier.sendMessages(messages_1, "connection_1");
        Assert.assertEquals(2, receiver_1.messages.size());
        Assert.assertEquals(0, receiver_2.messages.size());

        // Target the second connection this time with a different set of messages
        List<StructDef> messages_2 = Arrays.asList(new basic_message_struct(3),
                                                   new basic_message_struct(4),
                                                   new basic_message_struct(5));
        _supplier.sendMessages(messages_2, "connection_2");
        Assert.assertEquals(2, receiver_1.messages.size());
        Assert.assertEquals(3, receiver_2.messages.size());

        // Target both connections
        _supplier.sendMessages(messages_1);
        Assert.assertEquals(4, receiver_1.messages.size());
        Assert.assertEquals(5, receiver_2.messages.size());

        // Target invalid connection
        final List<StructDef> messages_3 = Arrays.asList(new basic_message_struct(6));
        Assert.assertThrows(IllegalArgumentException.class, () -> _supplier.sendMessages(messages_3, "bad_connection"));
        Assert.assertEquals(4, receiver_1.messages.size());
        Assert.assertEquals(5, receiver_2.messages.size());
    }

    @Test
    public void testPush()
    {
        GenericReceiver receiver = new GenericReceiver();
        _consumer.registerMessage(receiver);

        // Pack the messages ourselves
        CF.DataType[] messages_out = new CF.DataType[3];
        messages_out[0] = new CF.DataType("first", AnyUtils.toAny(100, "long"));
        messages_out[1] = new CF.DataType("second", AnyUtils.toAny("some text", "string"));
        messages_out[2] = new CF.DataType("third", AnyUtils.toAny(0.25, "double"));
        org.omg.CORBA.Any any = org.omg.CORBA.ORB.init().create_any();
        CF.PropertiesHelper.insert(any, messages_out);
        _supplier.push(any);

        Assert.assertEquals(3, receiver.messages.size());
        Assert.assertEquals("first", receiver.messages.get(0).id);
        Assert.assertEquals(100, receiver.messages.get(0).value.extract_long());
        Assert.assertEquals("second", receiver.messages.get(1).id);
        Assert.assertEquals("some text", receiver.messages.get(1).value.extract_string());
        Assert.assertEquals("third", receiver.messages.get(2).id);
        Assert.assertEquals(0.25, receiver.messages.get(2).value.extract_double(), 0.0);
    }

    @Test
    public void testPushConnectionId() throws Exception
    {
        // Create and connect a second consumer port
        MessageConsumerPort consumer_2 = new MessageConsumerPort("consumer_2");
        org.omg.CORBA.Object objref = _portManager.addPort(consumer_2);
        _supplier.connectPort(objref, "connection_2");

        // Set up 2 receivers to distinguish which connection received a
        // message
        GenericReceiver receiver_1 = new GenericReceiver();
        _consumer.registerMessage(receiver_1);

        GenericReceiver receiver_2 = new GenericReceiver();
        consumer_2.registerMessage(receiver_2);

        // Pack the messages ourselves and target the first connection
        CF.DataType[] messages_1 = new CF.DataType[3];
        messages_1[0] = new CF.DataType("first", AnyUtils.toAny(100, "long"));
        messages_1[1] = new CF.DataType("second", AnyUtils.toAny("some text", "string"));
        messages_1[2] = new CF.DataType("third", AnyUtils.toAny(0.25, "double"));
        org.omg.CORBA.Any any = org.omg.CORBA.ORB.init().create_any();
        CF.PropertiesHelper.insert(any, messages_1);
        _supplier.push(any, "connection_1");

        Assert.assertEquals(3, receiver_1.messages.size());
        Assert.assertEquals(0, receiver_2.messages.size());

        // Target the second connection with a different set of messages
        CF.DataType[] messages_2 = new CF.DataType[2];
        messages_2[0] = new CF.DataType("one", AnyUtils.toAny("abc", "string"));
        messages_2[1] = new CF.DataType("two", AnyUtils.toAny(false, "boolean"));
        any = org.omg.CORBA.ORB.init().create_any();
        CF.PropertiesHelper.insert(any, messages_2);
        _supplier.push(any, "connection_2");

        Assert.assertEquals(3, receiver_1.messages.size());
        Assert.assertEquals(2, receiver_2.messages.size());

        // Target both connections with yet another set of messages
        CF.DataType[] messages_3 = new CF.DataType[1];
        messages_3[0] = new CF.DataType("all", AnyUtils.toAny(3, "long"));
        any = org.omg.CORBA.ORB.init().create_any();
        CF.PropertiesHelper.insert(any, messages_3);
        _supplier.push(any);

        Assert.assertEquals(4, receiver_1.messages.size());
        Assert.assertEquals(3, receiver_2.messages.size());

        // Target invalid connection
        CF.DataType[] messages_4 = new CF.DataType[1];
        messages_4[0] = new CF.DataType("bad", AnyUtils.toAny("bad connection", "string"));
        final org.omg.CORBA.Any any_4 = org.omg.CORBA.ORB.init().create_any();
        CF.PropertiesHelper.insert(any_4, messages_4);
        Assert.assertThrows(IllegalArgumentException.class, () -> _supplier.push(any_4, "bad_connection"));

        Assert.assertEquals(4, receiver_1.messages.size());
        Assert.assertEquals(3, receiver_2.messages.size());
    }

    protected PortManager _portManager = new PortManager();

    protected MessageSupplierPort _supplier;
    protected MessageConsumerPort _consumer;
}
