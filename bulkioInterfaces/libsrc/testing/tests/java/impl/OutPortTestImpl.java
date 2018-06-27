/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
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
/*
 * WARNING: This file is generated from OutPortTestImpl.java.template.
 *          Do not modify directly.
 */

package impl;

import org.junit.*;
import org.junit.runner.RunWith;

import java.util.ArrayList;
import java.util.List;

import org.omg.PortableServer.Servant;

import bulkio.OutDataPort;

import stubs.Stub;
import helpers.TestHelper;

public class OutPortTestImpl<E extends BULKIO.updateSRIOperations,A> {

    protected OutDataPort<E,A> port;
    protected Stub<A> stub;

    protected TestHelper<E,A> helper;

    protected List<bulkio.connection_descriptor_struct> connectionTable = new ArrayList<bulkio.connection_descriptor_struct>();
    protected List<Servant> servants = new ArrayList<Servant>();

    public OutPortTestImpl(TestHelper<E,A> helper)
    {
        this.helper = helper;
    }

    @Before
    public void setUp() throws org.omg.CORBA.UserException
    {
        org.omg.CORBA.ORB orb = org.ossie.corba.utils.Init(new String[0], null);

        String name = helper.getName() + "_out";
        port = helper.createOutPort(name);
        org.ossie.corba.utils.RootPOA().activate_object(port);

        stub = _createStub();

        org.omg.CORBA.Object objref = stub._this();
        port.connectPort(objref, "connection_1");
    }

    @After
    public void tearDown()
    {
        _disconnectPorts();
    }

    protected void _disconnectPorts()
    {
        for (ExtendedCF.UsesConnection connection : port.connections()) {
            try {
                port.disconnectPort(connection.connectionId);
            } catch (Throwable exc) {
                // Ignore CORBA exceptions
            }
        }
    }

    protected void _releaseServants()
    {
        for (Servant servant : servants) {
            try {
                org.omg.PortableServer.POA poa = servant._default_POA();
                byte[] object_id = poa.servant_to_id(servant);
                poa.deactivate_object(object_id);
            } catch (Throwable exc) {
                // Ignore CORBA exceptions
            }
        }
    }

    protected Stub<A> _createStub() throws org.omg.CORBA.UserException
    {
        Stub<A> new_stub = helper.createStub();
        org.omg.PortableServer.Servant servant = new_stub.servant();
        org.ossie.corba.utils.RootPOA().activate_object(servant);
        servants.add(servant);
        return new_stub;
    }

    @Test
    public void testLegacyAPI()
    {
        port.enableStats(false);
        port.enableStats(true);

        // Pushing an SRI with a null streamID should trigger an NPE
        BULKIO.StreamSRI sri = new BULKIO.StreamSRI();
        sri.streamID = null;
        try {
            port.pushSRI(sri);
            Assert.fail("Did not raise NPE for null streamID");
        } catch (NullPointerException npe) {
            // Test passed
        }
    }

    @Test
    public void testConnectionListener() throws org.omg.CORBA.UserException
    {
        helpers.ConnectionListener listener = new helpers.ConnectionListener();
        port.setConnectionEventListener(listener);

        // Make a new connection
        Stub<A> stub2 = _createStub();
        port.connectPort(stub2._this(), "connection_2");
        Assert.assertEquals(1, listener.connected.size());
        Assert.assertEquals(0, listener.disconnected.size());
        Assert.assertEquals("connection_2", listener.connected.get(0));

        // Disconnect existing connection
        port.disconnectPort("connection_1");
        Assert.assertEquals(1, listener.connected.size());
        Assert.assertEquals(1, listener.disconnected.size());
        Assert.assertEquals("connection_1", listener.disconnected.get(0));

        // Remove listener and reconnect
        port.setConnectionEventListener(null);
        port.connectPort(stub._this(), "connection_1");
        Assert.assertEquals(1, listener.connected.size());
        Assert.assertEquals(1, listener.disconnected.size());
    }

    @Test
    public void testConnections() throws org.omg.CORBA.UserException
    {
        // Should start with one connection, to the in port stub
        ExtendedCF.UsesConnection[] connections = port.connections();
        Assert.assertNotNull(connections);
        Assert.assertEquals(1, connections.length);
        Assert.assertEquals("connection_1", connections[0].connectionId);
        org.omg.CORBA.Object objref = stub._this();
        Assert.assertTrue(connections[0].port._is_equivalent(objref));
        Assert.assertEquals("Port state should be active", BULKIO.PortUsageType.ACTIVE, port.state());

        // Should throw an invalid port on a nil
        try {
            port.connectPort(null, "connection_nil");
            Assert.fail("No exception thrown on connection to nil object");
        } catch (CF.PortPackage.InvalidPort exc) {
            // Test passed
        }

        // Normal connection
        Stub<A> stub2 = _createStub();
        objref = stub2._this();
        port.connectPort(objref, "connection_2");
        connections = port.connections();
        Assert.assertNotNull(connections);
        Assert.assertEquals(2, connections.length);
        for (ExtendedCF.UsesConnection connection : connections) {
            if (connection.connectionId.equals("connection_2")) {
                Assert.assertTrue(connection.port._is_equivalent(objref));
            } else if (!connection.connectionId.equals("connection_1")) {
                Assert.fail("Invalid connectionId in connections(): '" + connection.connectionId + "'");
            }
        }

        // Cannot reuse connection ID
        try {
            port.connectPort(objref, "connection_2");
            Assert.fail("No exception thrown on duplicate connectionId");
        } catch (CF.PortPackage.OccupiedPort exc) {
            // Test passed
        }

        // Disconnect second connection
        port.disconnectPort("connection_2");
        connections = port.connections();
        Assert.assertNotNull(connections);
        Assert.assertEquals(1, connections.length);
        Assert.assertEquals("connection_1", connections[0].connectionId);

        // Bad connection ID on disconnect
        try {
            port.disconnectPort("connection_bad");
            Assert.fail("No exception thrown on invalid connectionId");
        } catch (RuntimeException exc) {
            // Test passed
            // NB: For API backwards-compatibility reasons, Java ports do not
            //     throw CF.PortPackage.InvalidPort in disconnectPort()
        }

        // Disconnect the default stub; port should go to idle
        port.disconnectPort("connection_1");
        connections = port.connections();
        Assert.assertNotNull(connections);
        Assert.assertEquals(0, connections.length);
        Assert.assertEquals("Port state should be idle", BULKIO.PortUsageType.IDLE, port.state());
    }

    @Test
    public void testStatistics() throws org.omg.CORBA.UserException
    {
        // Even if there are no active SRIs, there should still be statistics
        // for existing connections
        BULKIO.UsesPortStatistics[] uses_stats = port.statistics();
        Assert.assertNotNull(uses_stats);
        Assert.assertEquals(1, uses_stats.length);
        Assert.assertEquals("connection_1", uses_stats[0].connectionId);

        // Push a packet of data to trigger meaningful statistics
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("port_stats");
        port.pushSRI(sri);
        helper.pushTestPacket(port, 1024, bulkio.time.utils.now(), false, sri.streamID);
        uses_stats = port.statistics();
        Assert.assertNotNull(uses_stats);
        Assert.assertEquals(1, uses_stats.length);
        BULKIO.PortStatistics stats = uses_stats[0].statistics;

        // Check that the statistics report the right element size
        Assert.assertTrue(stats.elementsPerSecond > 0.0);
        int bits_per_element = Math.round(stats.bitsPerSecond / stats.elementsPerSecond);
        Assert.assertEquals(helper.bitsPerElement(), bits_per_element);

        // Test that statistics are returned for all connections
        Stub<A> stub2 = _createStub();
        port.connectPort(stub2._this(), "connection_2");
        uses_stats = port.statistics();
        Assert.assertEquals("List of statistics does not match number of connections", 2, uses_stats.length);
    }

    @Test
    public void testActiveSRIs()
    {
        BULKIO.StreamSRI[] active_sris = port.activeSRIs();
        Assert.assertEquals(0, active_sris.length);

        // Push a new SRI, and make sure that it is immediately visible and
        // correct in activeSRIs
        BULKIO.StreamSRI sri_1 = bulkio.sri.utils.create("active_sri_1");
        port.pushSRI(sri_1);
        active_sris = port.activeSRIs();
        Assert.assertEquals(1, active_sris.length);
        Assert.assertTrue(bulkio.sri.utils.compare(active_sris[0], sri_1));

        // Push a second SRI, and make sure that activeSRIs is up-to-date
        BULKIO.StreamSRI sri_2 = bulkio.sri.utils.create("active_sri_2");
        port.pushSRI(sri_2);
        active_sris = port.activeSRIs();
        Assert.assertEquals(2, active_sris.length);
        for (BULKIO.StreamSRI current_sri : active_sris) {
            if (current_sri.streamID.equals("active_sri_2")) {
                Assert.assertTrue(bulkio.sri.utils.compare(current_sri, sri_2));
            } else if (!current_sri.streamID.equals("active_sri_1")) {
                Assert.fail("unexpected SRI '" + current_sri.streamID +"'");
            }
        }

        // Push an end-of-stream, and verify that the stream is no longer in
        // activeSRIs
        helper.pushTestPacket(port, 0, bulkio.time.utils.notSet(), true, sri_1.streamID);
        active_sris = port.activeSRIs();
        Assert.assertEquals(1, active_sris.length);
        Assert.assertEquals(active_sris[0].streamID, sri_2.streamID);
    }

    protected void _addStreamFilter(String streamId, String connectionId)
    {
        bulkio.connection_descriptor_struct desc = new bulkio.connection_descriptor_struct();
        desc.stream_id.setValue(streamId);
        desc.connection_id.setValue(connectionId);
        desc.port_name.setValue(port.getName());
        connectionTable.add(desc);
        port.updateConnectionFilter(connectionTable);
    }

    @Test
    public void testMultiOut() throws org.omg.CORBA.UserException
    {
        Stub<A> stub2 = _createStub();
        org.omg.CORBA.Object objref = stub2._this();
        port.connectPort(objref, "connection_2");

        // Set up a connection table that only routes the filtered stream to the
        // second stub, and another stream to both connections
        final String filter_stream_id = "filter_stream";
        _addStreamFilter(filter_stream_id, "connection_2");
        final String all_stream_id = "all_stream";
        _addStreamFilter(all_stream_id, "connection_1");
        _addStreamFilter(all_stream_id, "connection_2");

        // Push an SRI for the filtered stream; it should only be received by the
        // second stub
        BULKIO.StreamSRI sri = bulkio.sri.utils.create(filter_stream_id, 2.5e6);
        port.pushSRI(sri);
        Assert.assertTrue(stub.H.isEmpty());
        Assert.assertEquals(1, stub2.H.size());
        Assert.assertEquals(filter_stream_id, stub2.H.get(0).streamID);

        // Push a packet for the filtered stream; again, only received by #2
        helper.pushTestPacket(port, 91, bulkio.time.utils.now(), false, filter_stream_id);
        Assert.assertTrue(stub.packets.isEmpty());
        Assert.assertEquals(1, stub2.packets.size());
        Assert.assertEquals(91, helper.dataLength(stub2.packets.get(0).data));

        // Unknown (to the connection filter) stream should get dropped
        final String unknown_stream_id = "unknown_stream";
        sri = bulkio.sri.utils.create(unknown_stream_id);
        port.pushSRI(sri);
        Assert.assertTrue(stub.H.isEmpty());
        Assert.assertEquals(1, stub2.H.size());
        helper.pushTestPacket(port, 50, bulkio.time.utils.now(), false, unknown_stream_id);
        Assert.assertTrue(stub.packets.isEmpty());
        Assert.assertEquals(1, stub2.packets.size());

        // Check SRI routed to both connections...
        sri = bulkio.sri.utils.create(all_stream_id, 1e6);
        port.pushSRI(sri);
        Assert.assertEquals(1, stub.H.size());
        Assert.assertEquals(2, stub2.H.size());
        Assert.assertEquals(all_stream_id, stub.H.get(0).streamID);
        Assert.assertEquals(all_stream_id, stub2.H.get(1).streamID);

        // ...and data
        helper.pushTestPacket(port, 256, bulkio.time.utils.now(), false, all_stream_id);
        Assert.assertEquals(1, stub.packets.size());
        Assert.assertEquals(256, helper.dataLength(stub.packets.get(0).data));
        Assert.assertEquals(2, stub2.packets.size());
        Assert.assertEquals(256, helper.dataLength(stub2.packets.get(1).data));

        // Reset the connection filter and push data for the filtered stream again,
        // which should trigger an SRI push to the first stub
        connectionTable.clear();
        port.updateConnectionFilter(connectionTable);
        helper.pushTestPacket(port, 9, bulkio.time.utils.now(), false, filter_stream_id);
        Assert.assertEquals(2, stub.H.size());
        Assert.assertEquals(filter_stream_id, stub.H.get(1).streamID);
        Assert.assertEquals(2, stub.packets.size());
        Assert.assertEquals(9, helper.dataLength(stub.packets.get(1).data));
        Assert.assertEquals(2, stub2.H.size());
        Assert.assertEquals(3, stub2.packets.size());
        Assert.assertEquals(9, helper.dataLength(stub2.packets.get(2).data));
    }
}
