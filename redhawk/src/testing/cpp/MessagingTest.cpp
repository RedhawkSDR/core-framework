/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#include "MessagingTest.h"

#include <ossie/PropertyMap.h>

CPPUNIT_TEST_SUITE_REGISTRATION(MessagingTest);

namespace {
    // Internal structs and methods are encapsulated in an anonymous namespace
    // to prevent external symbol table pollution and collisions.

    // NB: Because the Any extraction operator for std::string is defined in
    // the global namespace (not std), we need to bring it into this namespace
    // so that it can be found by the custom struct extraction operators.
    using ::operator>>=;

    // Legacy message struct generated with REDHAWK 1.8. This serves two
    // purposes: it maintains compatibility with 1.8 code, and forces the
    // MessageSupplierPort to use Any serialization instead of direct message
    // transfer.
    struct legacy_message_struct {
        legacy_message_struct ()
        {
        };

        std::string getId() {
            return std::string("legacy_message");
        };
	
        CORBA::Long value;
    };

    inline bool operator>>= (const CORBA::Any& a, legacy_message_struct& s) {
        CF::Properties* temp;
        if (!(a >>= temp)) return false;
        CF::Properties& props = *temp;
        for (unsigned int idx = 0; idx < props.length(); idx++) {
            if (!strcmp("value", props[idx].id)) {
                if (!(props[idx].value >>= s.value)) return false;
            }
        }
        return true;
    };

    inline void operator<<= (CORBA::Any& a, const legacy_message_struct& s) {
        CF::Properties props;
        props.length(1);
        props[0].id = CORBA::string_dup("value");
        props[0].value <<= s.value;
        a <<= props;
    };

    // REDHAWK 2.1 message struct, with a getFormat() method that allows it to
    // be used with direct in-process messaging to skip Any serialization
    struct direct_message_struct {
        direct_message_struct ()
        {
        }

        static std::string getId() {
            return std::string("direct_message");
        }

        static const char* getFormat() {
            return "is";
        }

        CORBA::Long value;
        std::string body;
    };

    inline bool operator>>= (const CORBA::Any& a, direct_message_struct& s) {
        CF::Properties* temp;
        if (!(a >>= temp)) return false;
        const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
        if (props.contains("value")) {
            if (!(props["value"] >>= s.value)) return false;
        }
        if (props.contains("body")) {
            if (!(props["body"] >>= s.body)) return false;
        }
        return true;
    }

    inline void operator<<= (CORBA::Any& a, const direct_message_struct& s) {
        redhawk::PropertyMap props;
 
        props["value"] = s.value;
 
        props["body"] = s.body;
        a <<= props;
    }

    inline bool operator== (const direct_message_struct& s1, const direct_message_struct& s2) {
        if (s1.value!=s2.value)
            return false;
        if (s1.body!=s2.body)
            return false;
        return true;
    }

    inline bool operator!= (const direct_message_struct& s1, const direct_message_struct& s2) {
        return !(s1==s2);
    }

    // Utility class for message consumer callbacks
    template <class T>
    class MessageReceiver
    {
    public:
        typedef T message_type;

        void messageReceived(const std::string& messageId, const message_type& msgData)
        {
            _received.push_back(msgData);
            _addresses.push_back(&msgData);
        }

        const std::vector<message_type>& received() const
        {
            return _received;
        }

        const std::vector<const message_type*>& addresses() const
        {
            return _addresses;
        }

    private:
        std::vector<message_type> _received;
        std::vector<const message_type*> _addresses;
    };

    // Utility class for generic message (CORBA::Any) callbacks
    class GenericReceiver
    {
    public:
        void messageReceived(const std::string& messageId, const CORBA::Any& msgData)
        {
            _received.push_back(redhawk::PropertyType(messageId, msgData));
        }

        const redhawk::PropertyMap& received() const
        {
            return _received;
        }

    private:
        redhawk::PropertyMap _received;
    };
}

void MessagingTest::setUp()
{
    _supplier = new MessageSupplierPort("supplier");
    _consumer = new MessageConsumerPort("consumer");

    PortableServer::POA_ptr root_poa = ossie::corba::RootPOA();
    PortableServer::ObjectId_var oid = root_poa->activate_object(_supplier);
    oid = root_poa->activate_object(_consumer);

    // Connect the supplier and consumer
    CORBA::Object_var objref = _consumer->_this();
    _supplier->connectPort(objref, "connection_1");

    // Simulate component start
    _supplier->startPort();
    _consumer->startPort();
}

void MessagingTest::tearDown()
{
    // Simulate component stop/shutdown
    _supplier->stopPort();
    _consumer->stopPort();

    PortableServer::POA_ptr root_poa = ossie::corba::RootPOA();
    try {
        PortableServer::ObjectId_var oid = root_poa->servant_to_id(_supplier);
        root_poa->deactivate_object(oid);
        _supplier->_remove_ref();
    } catch (const CORBA::Exception&) {
        // TODO: Print message?
    }
    _supplier = 0;

    try {
        PortableServer::ObjectId_var oid = root_poa->servant_to_id(_consumer);
        root_poa->deactivate_object(oid);
        _consumer->_remove_ref();
    } catch (const CORBA::Exception&) {
        // TODO: Print message?
    }
    _consumer = 0;
}

void MessagingTest::testConnections()
{
    CORBA::Object_var objref = _consumer->_this();

    // Verify the connections list 
    ExtendedCF::UsesConnectionSequence_var connections = _supplier->connections();
    CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 1, connections->length());
    CPPUNIT_ASSERT_EQUAL(std::string("connection_1"), std::string(connections[0].connectionId));
    CPPUNIT_ASSERT(objref->_is_equivalent(connections[0].port));

    // Make two more connections
    _supplier->connectPort(objref, "connection_2");
    _supplier->connectPort(objref, "connection_3");
    connections = _supplier->connections();
    CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 3, connections->length());

    // Check all the connections; there is no guarantee of ordering in the
    // connection list, so collect the names in a set
    std::set<std::string> names;
    for (CORBA::ULong index = 0; index < connections->length(); ++index) {
        names.insert(std::string(connections[index].connectionId));
        CPPUNIT_ASSERT(objref->_is_equivalent(connections[index].port));
    }
    CPPUNIT_ASSERT(names.find("connection_1") != names.end());
    CPPUNIT_ASSERT(names.find("connection_2") != names.end());
    CPPUNIT_ASSERT(names.find("connection_3") != names.end());

    // Disconnect one of the connections and check again
    _supplier->disconnectPort("connection_2");
    connections = _supplier->connections();
    CPPUNIT_ASSERT_EQUAL((CORBA::ULong) 2, connections->length());
    names.clear();
    for (CORBA::ULong index = 0; index < connections->length(); ++index) {
        names.insert(std::string(connections[index].connectionId));
        CPPUNIT_ASSERT(objref->_is_equivalent(connections[index].port));
    }
    CPPUNIT_ASSERT(names.find("connection_1") != names.end());
    CPPUNIT_ASSERT(names.find("connection_3") != names.end());
}

void MessagingTest::testSendMessage()
{
    // Set up receiver
    typedef MessageReceiver<legacy_message_struct> receiver_type;
    receiver_type receiver;
    _consumer->registerMessage("legacy_message", &receiver, &receiver_type::messageReceived);

    legacy_message_struct msg;
    msg.value = 1;

    // Send the message and check that it's received. Currently, the consumer's
    // message handler is called directly from sendMessage, so this is a
    // synchronous operation; however, if at some point in the future, threaded
    // message dispatch is added, this test will need to be revisted.
    CPPUNIT_ASSERT(receiver.received().empty());
    _supplier->sendMessage(msg);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, receiver.received().size());

    // Check the message contents, working around the old code generators' non-
    // const, non-static getId() method with a const_cast
    const legacy_message_struct& received = receiver.received().front();
    CPPUNIT_ASSERT_EQUAL(std::string("legacy_message"), const_cast<legacy_message_struct&>(received).getId());
    CPPUNIT_ASSERT_EQUAL(msg.value, received.value);

    // This test is designed to use the slower Any serialization path for
    // message transfer, so check that it actually does
    CPPUNIT_ASSERT_MESSAGE("unexpected direct message send", receiver.addresses().front() != &msg);
}

void MessagingTest::testSendMessageDirect()
{
    // Set up receiver
    typedef MessageReceiver<direct_message_struct> receiver_type;
    receiver_type receiver;
    _consumer->registerMessage("direct_message", &receiver, &receiver_type::messageReceived);

    direct_message_struct msg;
    msg.value = 2;
    msg.body = "text string";

    // Send the message and check that it's received. Currently, the consumer's
    // message handler is called directly from sendMessage, so this is a
    // synchronous operation; however, if at some point in the future, threaded
    // message dispatch is added, this test will need to be revisted.
    CPPUNIT_ASSERT(receiver.received().empty());
    _supplier->sendMessage(msg);
    CPPUNIT_ASSERT_EQUAL((size_t) 1, receiver.received().size());

    // Check the message contents
    const direct_message_struct& received = receiver.received().front();
    CPPUNIT_ASSERT_EQUAL(direct_message_struct::getId(), received.getId());
    CPPUNIT_ASSERT(msg == received);

    // This test is designed to use the slower Any serialization path for
    // message transfer, so check that it actually does
    CPPUNIT_ASSERT_MESSAGE("direct message transfer not used", receiver.addresses().front() == &msg);
}

void MessagingTest::testSendMessages()
{
    // Set up receiver
    typedef MessageReceiver<legacy_message_struct> receiver_type;
    receiver_type receiver;
    _consumer->registerMessage("legacy_message", &receiver, &receiver_type::messageReceived);

    std::vector<legacy_message_struct> messages;
    for (size_t index = 0; index < 3; ++index) {
        legacy_message_struct msg;
        msg.value = index;
        messages.push_back(msg);
    }

    // Send the messages and check that they are received (see above re:
    // threading).
    CPPUNIT_ASSERT(receiver.received().empty());
    _supplier->sendMessages(messages);
    CPPUNIT_ASSERT_EQUAL(messages.size(), receiver.received().size());

    // Check the message bodies and make sure that the port used the Any
    // serialization path for message transfer
    for (size_t index = 0; index < messages.size(); ++index) {
        CPPUNIT_ASSERT_EQUAL(messages[index].value, receiver.received()[index].value);
        CPPUNIT_ASSERT_MESSAGE("unexpected direct message transfer", &messages[index] != receiver.addresses()[index]);
    }
}

void MessagingTest::testSendMessagesDirect()
{
    // Set up receiver
    typedef MessageReceiver<direct_message_struct> receiver_type;
    receiver_type receiver;
    _consumer->registerMessage("direct_message", &receiver, &receiver_type::messageReceived);

    const char* text[] = { "lorem", "ipsum", "dolor", "sit", "amet", 0 };
    std::vector<direct_message_struct> messages;
    for (size_t index = 0; text[index] != 0; ++index) {
        direct_message_struct msg;
        msg.value = index;
        msg.body = text[index];
        messages.push_back(msg);
    }

    // Send the messages and check that they are received (see above re:
    // threading).
    CPPUNIT_ASSERT(receiver.received().empty());
    _supplier->sendMessages(messages);
    CPPUNIT_ASSERT_EQUAL(messages.size(), receiver.received().size());

    // Check all the messages at once
    CPPUNIT_ASSERT(messages == receiver.received());

    // Make sure the port used direct transfer
    for (size_t index = 0; index < messages.size(); ++index) {
        CPPUNIT_ASSERT_MESSAGE("direct message transfer not used", &messages[index] == receiver.addresses()[index]);
    }
}

void MessagingTest::testGenericCallback()
{
    // Set up receiver
    GenericReceiver receiver;
    _consumer->registerMessage(&receiver, &GenericReceiver::messageReceived);

    // Send legacy_message and direct_message; with a generic callback,
    // everything necessarily goes through Any serialization, so there's no
    // distinction made other than there being two different message types
    legacy_message_struct legacy;
    legacy.value = 50;
    _supplier->sendMessage(legacy);

    direct_message_struct direct;
    direct.value = 100;
    direct.body = "lorem ipsum";
    _supplier->sendMessage(direct);

    // Check that the messages were received (see above re: threading)
    const redhawk::PropertyMap& messages = receiver.received();
    CPPUNIT_ASSERT_EQUAL((size_t) 2, messages.size());

    legacy_message_struct legacy_out;
    CPPUNIT_ASSERT_EQUAL(std::string("legacy_message"), messages[0].getId());
    CPPUNIT_ASSERT(messages[0].getValue() >>= legacy_out);
    CPPUNIT_ASSERT_EQUAL(legacy.value, legacy_out.value);

    direct_message_struct direct_out;
    CPPUNIT_ASSERT_EQUAL(direct_message_struct::getId(), messages[1].getId());
    CPPUNIT_ASSERT(messages[1].getValue() >>= direct_out);
    CPPUNIT_ASSERT(direct == direct_out);
}
