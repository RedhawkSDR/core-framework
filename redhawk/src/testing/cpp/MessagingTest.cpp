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

CPPUNIT_TEST_SUITE_REGISTRATION(MessagingTest);

void MessagingTest::setUp()
{
    _supplier = new MessageSupplierPort("supplier");
    _consumer = new MessageConsumerPort("consumer");

    PortableServer::POA_ptr root_poa = ossie::corba::RootPOA();
    PortableServer::ObjectId_var oid = root_poa->activate_object(_supplier);
    oid = root_poa->activate_object(_consumer);

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
    // Connect the supplier and consumer
    CORBA::Object_var objref = _consumer->_this();
    _supplier->connectPort(objref, "connection_1");

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
