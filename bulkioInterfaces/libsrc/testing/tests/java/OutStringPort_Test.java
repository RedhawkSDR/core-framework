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

/**
 * Tests for {@link Foo}.
 *
 * @author 
 */
@RunWith(JUnit4.class)
public class OutStringPort_Test {

    Logger logger =  Logger.getRootLogger();

    class test_fact {

	String  name = "OutInt8";

	String  port_name = new String("test-outport-api");

	String  sid = new String("test-outport-streamid");

	String  cid = new String("connect-1");

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
	public void test_OutFile( ) {

	test_fact ctx = new test_fact( "OutFile" );

	logger.info("------ Testing " + ctx.name + " Port ------");

	bulkio.OutFilePort port = new bulkio.OutFilePort(ctx.port_name);

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
	
	org.omg.CORBA.Object  p =  null;
	try {
	    port.connectPort( p, ctx.cid );

	    port.disconnectPort( ctx.cid );
	}
	catch(Exception e){
	}

	// push sri
	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );
	StreamSRI []sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );

	// push data
	String v = new String("");
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

    }


    @Test
	public void test_OutXML( ) {

	test_fact ctx = new test_fact( "OutXML" );

	logger.info("------ Testing " + ctx.name + " Port ------");

	bulkio.OutXMLPort port = new bulkio.OutXMLPort(ctx.port_name);

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
	
	org.omg.CORBA.Object  p =  null;
	try {
	    port.connectPort( p, ctx.cid );

	    port.disconnectPort( ctx.cid );
	}
	catch(Exception e){
	}

	// push sri
	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );
	StreamSRI []sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );

	// push data
	String v = new String("");
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, false, ctx.sid );

	// check SRI list
	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );
	
	// resolve - need to add end point to test stats
	//stats = port.statistics();
	//assertTrue("Port Statistics Failed",  stats.length == 1 );

	port.pushPacket( v, true,  ctx.sid );
	port.pushPacket( v, true, "unknown_stream_id" );

	sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 0 );

    }




}
