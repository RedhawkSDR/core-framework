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

import static org.junit.Assert.*;

import org.junit.BeforeClass;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.After;
import org.junit.Test;
import org.junit.Ignore;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.Logger;
import org.apache.log4j.LogManager;
import org.apache.log4j.PropertyConfigurator;
import org.apache.log4j.LogManager;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.Layout;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Appender;
import org.apache.log4j.Level;
import org.apache.log4j.xml.DOMConfigurator;
import org.omg.CORBA.Object;
import BULKIO.StreamSRI;
import BULKIO.PrecisionUTCTime;
import BULKIO.PortStatistics;
import BULKIO.PortUsageType;
import BULKIO.dataSDDSPackage.AttachError;
import BULKIO.dataSDDSPackage.DetachError;
import BULKIO.dataSDDSPackage.StreamInputError;
import bulkio.ConnectionEventListener;
import org.omg.CORBA.ORB;

/**
 * Tests for {@link Foo}.
 *
 * @author 
 */
@RunWith(JUnit4.class)
public class OutVectorPort_Test {

    Logger logger =  Logger.getRootLogger();

    public static ORB orb;

    class test_fact {

	String  name = "OutInt8";

	String  port_name = new String("test-outport-api");

	String  sid = new String("test-outport-streamid");

	String  cid = new String("connect-1");
	String  cid2 = new String("connect-2");

	short   mode = 1;

	double  srate=22.0;

	test_fact( String tname ) {
	    name=tname;
	}

    };

    class connect_listener implements bulkio.ConnectionEventListener {

	test_fact ctx=null;

	connect_listener( test_fact inCtx ) {
	    ctx = inCtx;
	}
	
	public void connect( String sid ){
	    assertTrue("Connection Callback, StreamID mismatch",  ctx.cid == sid );
	};

	public void disconnect( String sid ){
	    assertTrue("Disconnection Callback, StreamID mismatch",  ctx.cid == sid );
	};
    };



    @BeforeClass
	public static void oneTimeSetUp() {
	// Set up a simple configuration that logs on the console.
	BasicConfigurator.configure();

        // Create and initialize the ORB
        String [] args = new String[0];
        orb = ORB.init(args, null);
    }

    @AfterClass
	public static void oneTimeTearDown() {

    }

    @Before
	public void setUp() {
    }

    @After
	public void tearDown() {
	
    }


    @Test
	public void test_OutInt8( ) {

	test_fact ctx = new test_fact( "OutInt8" );

	logger.info("------ Testing " + ctx.name + " Port ------");

	bulkio.OutInt8Port port = new bulkio.OutInt8Port(ctx.port_name);

	String pname = port.getName();
	assertTrue("Port Name Failed",  pname == ctx.port_name );

	port.setLogger(logger);

	// port statistics
	port.enableStats( false );

	port.enableStats( true );
	BULKIO.UsesPortStatistics []stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	ExtendedCF.UsesConnection []clist = port.connections();
	assertTrue("Uses Connections List Failed",  clist != null );
	assertTrue("Uses Connections List Failed",  clist.length == 0 );

	//
	// test callback feature
	//
	port.setConnectionEventListener(new connect_listener(ctx) );
	
	bulkio.InInt8Port p = new bulkio.InInt8Port("sink_1",logger);
	try {

	    port.connectPort( p._this_object(orb), ctx.cid );

	    port.disconnectPort( ctx.cid );
	    port.disconnectPort( ctx.cid );
	}
	catch(Exception e){
	}

        // clear callback and reconnect
        port.setConnectionEventListener(null);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );
	}
	catch(Exception e){
            fail("Unable to connect port");
	}

	// push sri
	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );
	StreamSRI []sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );

        // Pushing an SRI with a null streamID should trigger an NPE
        sri = new BULKIO.StreamSRI();
        sri.streamID = null;
        boolean received_npe = false;
        try {
            port.pushSRI(sri);
        } catch (NullPointerException npe) {
            received_npe = true;
        }
        assertTrue("Did not raise NPE for null streamID", received_npe);

	// push data
	char []v = new char[] { (char)0 };
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid );

	// check SRI list
	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );
	
	// resolve - need to add end point to test stats
	//stats = port.statistics();
	//assertTrue("Port Statistics Failed",  stats.length == 1 );

	port.pushPacket( v, TS, true,  ctx.sid );
	port.pushPacket( v, TS, true, "unknown_stream_id" );

	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 0 );

        // Check that the statistics report the right element size
        test_element_size(port, 8);

        // Test that statistics are returned for all connections
        test_statistics(port, p._this_object(orb), ctx);
    }


    @Test
	public void test_OutInt16( ) {


	test_fact ctx = new test_fact( "OutInt16" );

	logger.info("------ Testing " + ctx.name + " Port ------");

	bulkio.OutInt16Port port = new bulkio.OutInt16Port(ctx.port_name);

	String pname = port.getName();
	assertTrue("Port Name Failed",  pname == ctx.port_name );

	port.setLogger(logger);

	// port statistics
	port.enableStats( false );

	port.enableStats( true );
	BULKIO.UsesPortStatistics []stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	ExtendedCF.UsesConnection []clist = port.connections();
	assertTrue("Uses Connections List Failed",  clist != null );
	assertTrue("Uses Connections List Failed",  clist.length == 0 );

	//
	// test callback feature
	//
	port.setConnectionEventListener(new connect_listener(ctx) );
	
	bulkio.InInt16Port p = new bulkio.InInt16Port("sink_1",logger);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );

	    port.disconnectPort( ctx.cid );
	    port.disconnectPort( ctx.cid );
	}
	catch(Exception e){
	}

        // clear callback and reconnect
        port.setConnectionEventListener(null);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );
	}
	catch(Exception e){
            fail("Unable to connect port");
	}

	// push sri
	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );
	StreamSRI []sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );

        // Pushing an SRI with a null streamID should trigger an NPE
        sri = new BULKIO.StreamSRI();
        sri.streamID = null;
        boolean received_npe = false;
        try {
            port.pushSRI(sri);
        } catch (NullPointerException npe) {
            received_npe = true;
        }
        assertTrue("Did not raise NPE for null streamID", received_npe);

	// push data
	short []v = new short[] { (short)0 };
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid );

	// check SRI list
	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );
	
	// resolve - need to add end point to test stats
	//stats = port.statistics();
	//assertTrue("Port Statistics Failed",  stats.length == 1 );

	port.pushPacket( v, TS, true,  ctx.sid );
	port.pushPacket( v, TS, true, "unknown_stream_id" );

	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 0 );

        // Check that the statistics report the right element size
        test_element_size(port, 16);

        // Test that statistics are returned for all connections
        test_statistics(port, p._this_object(orb), ctx);
    }

    @Test
	public void test_OutInt32( ) {


	test_fact ctx = new test_fact( "OutInt32" );

	logger.info("------ Testing " + ctx.name + " Port ------");

	bulkio.OutInt32Port port = new bulkio.OutInt32Port(ctx.port_name);

	String pname = port.getName();
	assertTrue("Port Name Failed",  pname == ctx.port_name );

	port.setLogger(logger);

	// port statistics
	port.enableStats( false );

	port.enableStats( true );
	BULKIO.UsesPortStatistics []stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	ExtendedCF.UsesConnection []clist = port.connections();
	assertTrue("Uses Connections List Failed",  clist != null );
	assertTrue("Uses Connections List Failed",  clist.length == 0 );

	//
	// test callback feature
	//
	port.setConnectionEventListener(new connect_listener(ctx) );
	
	bulkio.InInt32Port p = new bulkio.InInt32Port("sink_1",logger);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );

	    port.disconnectPort( ctx.cid );
	    port.disconnectPort( ctx.cid );
	}
	catch(Exception e){
	}

        // clear callback and reconnect
        port.setConnectionEventListener(null);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );
	}
	catch(Exception e){
            fail("Unable to connect port");
	}

	// push sri
	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );
	StreamSRI []sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );

        // Pushing an SRI with a null streamID should trigger an NPE
        sri = new BULKIO.StreamSRI();
        sri.streamID = null;
        boolean received_npe = false;
        try {
            port.pushSRI(sri);
        } catch (NullPointerException npe) {
            received_npe = true;
        }
        assertTrue("Did not raise NPE for null streamID", received_npe);

	// push data
	int []v = new int[] { 0 };
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid );

	// check SRI list
	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );
	
	// resolve - need to add end point to test stats
	//stats = port.statistics();
	//assertTrue("Port Statistics Failed",  stats.length == 1 );

	port.pushPacket( v, TS, true,  ctx.sid );
	port.pushPacket( v, TS, true, "unknown_stream_id" );

	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 0 );

        // Check that the statistics report the right element size
        test_element_size(port, 32);

        // Test that statistics are returned for all connections
        test_statistics(port, p._this_object(orb), ctx);
    }


    @Test
	public void test_OutInt64( ) {


	test_fact ctx = new test_fact( "OutInt64" );

	logger.info("------ Testing " + ctx.name + " Port ------");

	bulkio.OutInt64Port port = new bulkio.OutInt64Port(ctx.port_name);

	String pname = port.getName();
	assertTrue("Port Name Failed",  pname == ctx.port_name );

	port.setLogger(logger);

	// port statistics
	port.enableStats( false );

	port.enableStats( true );
	BULKIO.UsesPortStatistics []stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	ExtendedCF.UsesConnection []clist = port.connections();
	assertTrue("Uses Connections List Failed",  clist != null );
	assertTrue("Uses Connections List Failed",  clist.length == 0 );

	//
	// test callback feature
	//
	port.setConnectionEventListener(new connect_listener(ctx) );
	
	bulkio.InInt64Port p = new bulkio.InInt64Port("sink_1",logger);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );

	    port.disconnectPort( ctx.cid );
	    port.disconnectPort( ctx.cid );
	}
	catch(Exception e){
	}

        // clear callback and reconnect
        port.setConnectionEventListener(null);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );
	}
	catch(Exception e){
            fail("Unable to connect port");
	}

	// push sri
	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );
	StreamSRI []sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );

        // Pushing an SRI with a null streamID should trigger an NPE
        sri = new BULKIO.StreamSRI();
        sri.streamID = null;
        boolean received_npe = false;
        try {
            port.pushSRI(sri);
        } catch (NullPointerException npe) {
            received_npe = true;
        }
        assertTrue("Did not raise NPE for null streamID", received_npe);

	// push data
	long []v = new long[] { 0L };
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid );

	// check SRI list
	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );
	
	// resolve - need to add end point to test stats
	//stats = port.statistics();
	//assertTrue("Port Statistics Failed",  stats.length == 1 );

	port.pushPacket( v, TS, true,  ctx.sid );
	port.pushPacket( v, TS, true, "unknown_stream_id" );

	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 0 );

        // Check that the statistics report the right element size
        test_element_size(port, 64);

        // Test that statistics are returned for all connections
        test_statistics(port, p._this_object(orb), ctx);
    }


    @Test
	public void test_OutUInt8( ) {


	test_fact ctx = new test_fact( "OutUInt8" );

	logger.info("------ Testing " + ctx.name + " Port ------");

	bulkio.OutUInt8Port port = new bulkio.OutUInt8Port(ctx.port_name);

	String pname = port.getName();
	assertTrue("Port Name Failed",  pname == ctx.port_name );

	port.setLogger(logger);

	// port statistics
	port.enableStats( false );

	port.enableStats( true );
	BULKIO.UsesPortStatistics []stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	ExtendedCF.UsesConnection []clist = port.connections();
	assertTrue("Uses Connections List Failed",  clist != null );
	assertTrue("Uses Connections List Failed",  clist.length == 0 );

	//
	// test callback feature
	//
	port.setConnectionEventListener(new connect_listener(ctx) );
	
	bulkio.InUInt8Port p = new bulkio.InUInt8Port("sink_1",logger);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );

	    port.disconnectPort( ctx.cid );
	    port.disconnectPort( ctx.cid );
	}
	catch(Exception e){
	}

        // clear callback and reconnect
        port.setConnectionEventListener(null);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );
	}
	catch(Exception e){
            fail("Unable to connect port");
	}

	// push sri
	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );
	StreamSRI []sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );

        // Pushing an SRI with a null streamID should trigger an NPE
        sri = new BULKIO.StreamSRI();
        sri.streamID = null;
        boolean received_npe = false;
        try {
            port.pushSRI(sri);
        } catch (NullPointerException npe) {
            received_npe = true;
        }
        assertTrue("Did not raise NPE for null streamID", received_npe);

	// push data
	byte []v = new byte[] { (byte)0 };
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid );

	// check SRI list
	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );
	
	// resolve - need to add end point to test stats
	//stats = port.statistics();
	//assertTrue("Port Statistics Failed",  stats.length == 1 );

	port.pushPacket( v, TS, true,  ctx.sid );
	port.pushPacket( v, TS, true, "unknown_stream_id" );

	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 0 );

        // Check that the statistics report the right element size
        test_element_size(port, 8);

        // Test that statistics are returned for all connections
        test_statistics(port, p._this_object(orb), ctx);
    }


    @Test
	public void test_OutUInt16( ) {


	test_fact ctx = new test_fact( "OutUInt16" );

	logger.info("------ Testing " + ctx.name + " Port ------");

	bulkio.OutUInt16Port port = new bulkio.OutUInt16Port(ctx.port_name);

	String pname = port.getName();
	assertTrue("Port Name Failed",  pname == ctx.port_name );

	port.setLogger(logger);

	// port statistics
	port.enableStats( false );

	port.enableStats( true );
	BULKIO.UsesPortStatistics []stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	ExtendedCF.UsesConnection []clist = port.connections();
	assertTrue("Uses Connections List Failed",  clist != null );
	assertTrue("Uses Connections List Failed",  clist.length == 0 );

	//
	// test callback feature
	//
	port.setConnectionEventListener(new connect_listener(ctx) );
	
	bulkio.InUInt16Port p = new bulkio.InUInt16Port("sink_1",logger);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );

	    port.disconnectPort( ctx.cid );
	    port.disconnectPort( ctx.cid );
	}
	catch(Exception e){
	}

        // clear callback and reconnect
        port.setConnectionEventListener(null);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );
	}
	catch(Exception e){
            fail("Unable to connect port");
	}

	// push sri
	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );
	StreamSRI []sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );

        // Pushing an SRI with a null streamID should trigger an NPE
        sri = new BULKIO.StreamSRI();
        sri.streamID = null;
        boolean received_npe = false;
        try {
            port.pushSRI(sri);
        } catch (NullPointerException npe) {
            received_npe = true;
        }
        assertTrue("Did not raise NPE for null streamID", received_npe);

	// push data
	short []v = new short[] { (short)0 };
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid );

	// check SRI list
	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );
	
	// resolve - need to add end point to test stats
	//stats = port.statistics();
	//assertTrue("Port Statistics Failed",  stats.length == 1 );

	port.pushPacket( v, TS, true,  ctx.sid );
	port.pushPacket( v, TS, true, "unknown_stream_id" );

	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 0 );

        // Check that the statistics report the right element size
        test_element_size(port, 16);

        // Test that statistics are returned for all connections
        test_statistics(port, p._this_object(orb), ctx);
    }

    @Test
	public void test_OutUInt32( ) {


	test_fact ctx = new test_fact( "OutUInt32" );

	logger.info("------ Testing " + ctx.name + " Port ------");

	bulkio.OutUInt32Port port = new bulkio.OutUInt32Port(ctx.port_name);

	String pname = port.getName();
	assertTrue("Port Name Failed",  pname == ctx.port_name );

	port.setLogger(logger);

	// port statistics
	port.enableStats( false );

	port.enableStats( true );
	BULKIO.UsesPortStatistics []stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	ExtendedCF.UsesConnection []clist = port.connections();
	assertTrue("Uses Connections List Failed",  clist != null );
	assertTrue("Uses Connections List Failed",  clist.length == 0 );

	//
	// test callback feature
	//
	port.setConnectionEventListener(new connect_listener(ctx) );
	
	bulkio.InUInt32Port p = new bulkio.InUInt32Port("sink_1",logger);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );

	    port.disconnectPort( ctx.cid );
	    port.disconnectPort( ctx.cid );
	}
	catch(Exception e){
	}

        // clear callback and reconnect
        port.setConnectionEventListener(null);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );
	}
	catch(Exception e){
            fail("Unable to connect port");
	}

	// push sri
	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );
	StreamSRI []sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );

        // Pushing an SRI with a null streamID should trigger an NPE
        sri = new BULKIO.StreamSRI();
        sri.streamID = null;
        boolean received_npe = false;
        try {
            port.pushSRI(sri);
        } catch (NullPointerException npe) {
            received_npe = true;
        }
        assertTrue("Did not raise NPE for null streamID", received_npe);

	// push data
	int []v = new int[] { 0 };
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid );

	// check SRI list
	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );
	
	// resolve - need to add end point to test stats
	//stats = port.statistics();
	//assertTrue("Port Statistics Failed",  stats.length == 1 );

	port.pushPacket( v, TS, true,  ctx.sid );
	port.pushPacket( v, TS, true, "unknown_stream_id" );

	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 0 );

        // Check that the statistics report the right element size
        test_element_size(port, 32);

        // Test that statistics are returned for all connections
        test_statistics(port, p._this_object(orb), ctx);
    }


    @Test
	public void test_OutUInt64( ) {


	test_fact ctx = new test_fact( "OutUInt64" );

	logger.info("------ Testing " + ctx.name + " Port ------");

	bulkio.OutUInt64Port port = new bulkio.OutUInt64Port(ctx.port_name);

	String pname = port.getName();
	assertTrue("Port Name Failed",  pname == ctx.port_name );

	port.setLogger(logger);

	// port statistics
	port.enableStats( false );

	port.enableStats( true );
	BULKIO.UsesPortStatistics []stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	ExtendedCF.UsesConnection []clist = port.connections();
	assertTrue("Uses Connections List Failed",  clist != null );
	assertTrue("Uses Connections List Failed",  clist.length == 0 );

	//
	// test callback feature
	//
	port.setConnectionEventListener(new connect_listener(ctx) );
	
	bulkio.InUInt64Port p = new bulkio.InUInt64Port("sink_1",logger);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );

	    port.disconnectPort( ctx.cid );
	    port.disconnectPort( ctx.cid );
	}
	catch(Exception e){
	}

        // clear callback and reconnect
        port.setConnectionEventListener(null);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );
	}
	catch(Exception e){
            fail("Unable to connect port");
	}

	// push sri
	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );
	StreamSRI []sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );

        // Pushing an SRI with a null streamID should trigger an NPE
        sri = new BULKIO.StreamSRI();
        sri.streamID = null;
        boolean received_npe = false;
        try {
            port.pushSRI(sri);
        } catch (NullPointerException npe) {
            received_npe = true;
        }
        assertTrue("Did not raise NPE for null streamID", received_npe);

	// push data
	long []v = new long[] { 0L };
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid );

	// check SRI list
	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );
	
	// resolve - need to add end point to test stats
	//stats = port.statistics();
	//assertTrue("Port Statistics Failed",  stats.length == 1 );

	port.pushPacket( v, TS, true,  ctx.sid );
	port.pushPacket( v, TS, true, "unknown_stream_id" );

	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 0 );

        // Check that the statistics report the right element size
        test_element_size(port, 64);

        // Test that statistics are returned for all connections
        test_statistics(port, p._this_object(orb), ctx);
    }



    @Test
	public void test_OutDouble( ) {


	test_fact ctx = new test_fact( "OutDouble" );

	logger.info("------ Testing " + ctx.name + " Port ------");

	bulkio.OutDoublePort port = new bulkio.OutDoublePort(ctx.port_name);

	String pname = port.getName();
	assertTrue("Port Name Failed",  pname == ctx.port_name );

	port.setLogger(logger);

	// port statistics
	port.enableStats( false );

	port.enableStats( true );
	BULKIO.UsesPortStatistics []stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	ExtendedCF.UsesConnection []clist = port.connections();
	assertTrue("Uses Connections List Failed",  clist != null );
	assertTrue("Uses Connections List Failed",  clist.length == 0 );

	//
	// test callback feature
	//
	port.setConnectionEventListener(new connect_listener(ctx) );
	
	bulkio.InDoublePort p = new bulkio.InDoublePort("sink_1",logger);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );

	    port.disconnectPort( ctx.cid );
	    port.disconnectPort( ctx.cid );
	}
	catch(Exception e){
	}

        // clear callback and reconnect
        port.setConnectionEventListener(null);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );
	}
	catch(Exception e){
            fail("Unable to connect port");
	}

	// push sri
	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );
	StreamSRI []sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );

        // Pushing an SRI with a null streamID should trigger an NPE
        sri = new BULKIO.StreamSRI();
        sri.streamID = null;
        boolean received_npe = false;
        try {
            port.pushSRI(sri);
        } catch (NullPointerException npe) {
            received_npe = true;
        }
        assertTrue("Did not raise NPE for null streamID", received_npe);

	// push data
	double []v = new double[] { 0.0 };
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid );

	// check SRI list
	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );
	
	// resolve - need to add end point to test stats
	//stats = port.statistics();
	//assertTrue("Port Statistics Failed",  stats.length == 1 );

	port.pushPacket( v, TS, true,  ctx.sid );
	port.pushPacket( v, TS, true, "unknown_stream_id" );

	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 0 );

        // Check that the statistics report the right element size
        test_element_size(port, 64);

        // Test that statistics are returned for all connections
        test_statistics(port, p._this_object(orb), ctx);
    }

    @Test
	public void test_OutFloat( ) {


	test_fact ctx = new test_fact( "OutFloat" );

	logger.info("------ Testing " + ctx.name + " Port ------");

	bulkio.OutFloatPort port = new bulkio.OutFloatPort(ctx.port_name);

	String pname = port.getName();
	assertTrue("Port Name Failed",  pname == ctx.port_name );

	port.setLogger(logger);

	// port statistics
	port.enableStats( false );

	port.enableStats( true );
	BULKIO.UsesPortStatistics []stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	ExtendedCF.UsesConnection []clist = port.connections();
	assertTrue("Uses Connections List Failed",  clist != null );
	assertTrue("Uses Connections List Failed",  clist.length == 0 );

	//
	// test callback feature
	//
	port.setConnectionEventListener(new connect_listener(ctx) );
	
	bulkio.InFloatPort p = new bulkio.InFloatPort("sink_1",logger);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );

	    port.disconnectPort( ctx.cid );
	    port.disconnectPort( ctx.cid );
	}
	catch(Exception e){
	}

        // clear callback and reconnect
        port.setConnectionEventListener(null);
	try {
	    port.connectPort( p._this_object(orb), ctx.cid );
	}
	catch(Exception e){
            fail("Unable to connect port");
	}

	// push sri
	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );
	StreamSRI []sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );

        // Pushing an SRI with a null streamID should trigger an NPE
        sri = new BULKIO.StreamSRI();
        sri.streamID = null;
        boolean received_npe = false;
        try {
            port.pushSRI(sri);
        } catch (NullPointerException npe) {
            received_npe = true;
        }
        assertTrue("Did not raise NPE for null streamID", received_npe);

	// push data
	float []v = new float[] { 0.0f };
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid );

	// check SRI list
	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );
	
	// resolve - need to add end point to test stats
	//stats = port.statistics();
	//assertTrue("Port Statistics Failed",  stats.length == 1 );

	port.pushPacket( v, TS, true,  ctx.sid );
	port.pushPacket( v, TS, true, "unknown_stream_id" );

	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 0 );

        // Check that the statistics report the right element size
        test_element_size(port, 32);

        // Test that statistics are returned for all connections
        test_statistics(port, p._this_object(orb), ctx);
    }

    private void test_element_size(BULKIO.UsesPortStatisticsProviderOperations port, int bits)
    {
	BULKIO.UsesPortStatistics[] stats = port.statistics();
	assertEquals("No statistics",  stats.length, 1);
        double bpe = stats[0].statistics.bitsPerSecond / stats[0].statistics.elementsPerSecond;
        assertEquals("Incorrect element size", (double)bits, bpe, 1e-6);
    }

    private void test_statistics(BULKIO.UsesPortStatisticsProviderPOA port,
                                 org.omg.CORBA.Object sink,
                                 test_fact ctx)
    {
        try {
            port.connectPort(sink, ctx.cid);
            port.connectPort(sink, ctx.cid2);
        } catch (final CF.PortPackage.OccupiedPort ex) {
            fail("Port should never throw CF.Port.OccupiedPort");
        } catch (final CF.PortPackage.InvalidPort ex) {
            fail("Failed to connect ports");
        }
        BULKIO.UsesPortStatistics[] stats = port.statistics();
        assertEquals("Statistics returned wrong number of connections", stats.length, 2);
        assertNotNull("Statistics[0] is null", stats[0]);
        assertNotNull("Statistics[1] is null", stats[1]);
    }

}
