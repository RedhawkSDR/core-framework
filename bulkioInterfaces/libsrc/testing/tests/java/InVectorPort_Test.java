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
import BULKIO.StreamSRI;
import BULKIO.PrecisionUTCTime;
import BULKIO.PortStatistics;
import BULKIO.PortUsageType;
import BULKIO.dataSDDSPackage.AttachError;
import BULKIO.dataSDDSPackage.DetachError;
import BULKIO.dataSDDSPackage.StreamInputError;

/**
 * Tests for {@link Foo}.
 *
 * @author 
 */
@RunWith(JUnit4.class)
public class InVectorPort_Test {

    class test_fact {

	String  name = "InInt8";

	String  port_name = new String("test-inport-api");

	String  sid = new String("test-inport-streamid");

	short   mode = 1;

	double  srate=22.0;

	test_fact( String tname ){
	    name=tname;
	};
    };

    class test_stream_cb implements bulkio.SriListener {

	test_fact ctx=null;

	test_stream_cb ( test_fact inCtx ) {
	    ctx=inCtx;
	}

	public void     newSRI( StreamSRI sri ) {
	    assertTrue("newSRI SRI Object Invalid",  null != sri );	    
	    assertTrue("newSRI StreamID Mismatch",  ctx.sid == sri.streamID );	    
	}

	public boolean  changedSRI( StreamSRI sri ) {
	    assertTrue("changedSRI SRI Object Invalid",  null != sri );	    
	    assertTrue("changedSRI Mode Mismatch",  ctx.mode == sri.mode );
            return true;
	}
    }


    Logger logger =  Logger.getRootLogger();


    test_fact   ctx = null;

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
	public void test_InInt8( ) {

	ctx=new test_fact("InInt8");

	logger.info("------ Testing " + ctx.name + " Port -----");
		
	bulkio.InInt8Port port = new bulkio.InInt8Port(ctx.port_name);
	port.setSriListener( new test_stream_cb( ctx ) );

	//
	// simple attribute tests
	//
	port.setLogger( logger );

	// port statistics test
	port.enableStats( false );
	BULKIO.PortStatistics stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats == null );

	port.enableStats( true );
	stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	BULKIO.StreamSRI []streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence Failed",  streams != null );
	
	int tmp = port.getMaxQueueDepth();
	assertTrue("MaxQueueDepth - default",  tmp == 100 );

	tmp = port.getCurrentQueueDepth();
	assertTrue("CurrentQueueDepth - new",  tmp == 0 );

	port.setMaxQueueDepth(22);
	tmp = port.getMaxQueueDepth();
	assertTrue("SetMaxQueueDepth - set value",  tmp == 22 );

	// check that port queue is empty
	bulkio.InInt8Port.Packet pkt  = port.getPacket( bulkio.Const.NON_BLOCKING );
	assertTrue("GetPacket - no data",  pkt == null );

	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - 1 SRI",  streams != null );
	assertTrue("Stream SRI Sequence - 1 SRI, length",  streams.length ==1 );

	char [] v = new char[0];
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid  );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 0  );

	// complex mode test
	sri.mode = ctx.mode;
	port.pushSRI(sri);

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - Complex SRI",  streams != null );
	assertTrue("Stream SRI Sequence - Complex, length",  streams.length ==1 );
	
	port.pushPacket( v, TS, false, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, CPLX pkt",  pkt != null );
	assertTrue("Get a Packet, CPLX StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, CPLX EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, CPLX mode",  pkt.SRI.mode == 1  );

	// test for EOS..
	port.pushPacket( v, TS, true, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == true );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 1  );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - SRI",  streams != null );
	assertTrue("Stream SRI Sequence - length",  streams.length !=1 );
	assertTrue("Stream SRI Sequence - length",  streams.length ==0 );
    }


    @Test
	public void test_InInt16( ) {

	ctx=new test_fact("InInt16");

	logger.info("------ Testing " + ctx.name + " Port -----");
		
	bulkio.InInt16Port port = new bulkio.InInt16Port(ctx.port_name);
	port.setSriListener( new test_stream_cb( ctx ) );

	//
	// simple attribute tests
	//
	port.setLogger( logger );

	// port statistics test
	port.enableStats( false );
	BULKIO.PortStatistics stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats == null );

	port.enableStats( true );
	stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	BULKIO.StreamSRI []streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence Failed",  streams != null );
	
	int tmp = port.getMaxQueueDepth();
	assertTrue("MaxQueueDepth - default",  tmp == 100 );

	tmp = port.getCurrentQueueDepth();
	assertTrue("CurrentQueueDepth - new",  tmp == 0 );

	port.setMaxQueueDepth(22);
	tmp = port.getMaxQueueDepth();
	assertTrue("SetMaxQueueDepth - set value",  tmp == 22 );

	// check that port queue is empty
	bulkio.InInt16Port.Packet pkt  = port.getPacket( bulkio.Const.NON_BLOCKING );
	assertTrue("GetPacket - no data",  pkt == null );

	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - 1 SRI",  streams != null );
	assertTrue("Stream SRI Sequence - 1 SRI, length",  streams.length ==1 );

	short [] v = new short[0];
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid  );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 0  );

	// complex mode test
	sri.mode = ctx.mode;
	port.pushSRI(sri);

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - Complex SRI",  streams != null );
	assertTrue("Stream SRI Sequence - Complex, length",  streams.length ==1 );
	
	port.pushPacket( v, TS, false, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, CPLX pkt",  pkt != null );
	assertTrue("Get a Packet, CPLX StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, CPLX EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, CPLX mode",  pkt.SRI.mode == 1  );

	// test for EOS..
	port.pushPacket( v, TS, true, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == true );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 1  );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - SRI",  streams != null );
	assertTrue("Stream SRI Sequence - length",  streams.length !=1 );
	assertTrue("Stream SRI Sequence - length",  streams.length ==0 );
    }


    @Test
	public void test_InInt32( ) {

	ctx=new test_fact("InInt32");

	logger.info("------ Testing " + ctx.name + " Port -----");
		
	bulkio.InInt32Port port = new bulkio.InInt32Port(ctx.port_name);
	port.setSriListener( new test_stream_cb( ctx ) );

	//
	// simple attribute tests
	//
	port.setLogger( logger );

	// port statistics test
	port.enableStats( false );
	BULKIO.PortStatistics stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats == null );

	port.enableStats( true );
	stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	BULKIO.StreamSRI []streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence Failed",  streams != null );
	
	int tmp = port.getMaxQueueDepth();
	assertTrue("MaxQueueDepth - default",  tmp == 100 );

	tmp = port.getCurrentQueueDepth();
	assertTrue("CurrentQueueDepth - new",  tmp == 0 );

	port.setMaxQueueDepth(22);
	tmp = port.getMaxQueueDepth();
	assertTrue("SetMaxQueueDepth - set value",  tmp == 22 );

	// check that port queue is empty
	bulkio.InInt32Port.Packet pkt  = port.getPacket( bulkio.Const.NON_BLOCKING );
	assertTrue("GetPacket - no data",  pkt == null );

	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - 1 SRI",  streams != null );
	assertTrue("Stream SRI Sequence - 1 SRI, length",  streams.length ==1 );

	int [] v = new int[0];
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid  );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 0  );

	// complex mode test
	sri.mode = ctx.mode;
	port.pushSRI(sri);

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - Complex SRI",  streams != null );
	assertTrue("Stream SRI Sequence - Complex, length",  streams.length ==1 );
	
	port.pushPacket( v, TS, false, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, CPLX pkt",  pkt != null );
	assertTrue("Get a Packet, CPLX StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, CPLX EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, CPLX mode",  pkt.SRI.mode == 1  );

	// test for EOS..
	port.pushPacket( v, TS, true, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == true );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 1  );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - SRI",  streams != null );
	assertTrue("Stream SRI Sequence - length",  streams.length !=1 );
	assertTrue("Stream SRI Sequence - length",  streams.length ==0 );
    }

    @Test
	public void test_InInt64( ) {

	ctx=new test_fact("InInt64");

	logger.info("------ Testing " + ctx.name + " Port -----");
		
	bulkio.InInt64Port port = new bulkio.InInt64Port(ctx.port_name);
	port.setSriListener( new test_stream_cb( ctx ) );

	//
	// simple attribute tests
	//
	port.setLogger( logger );

	// port statistics test
	port.enableStats( false );
	BULKIO.PortStatistics stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats == null );

	port.enableStats( true );
	stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	BULKIO.StreamSRI []streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence Failed",  streams != null );
	
	int tmp = port.getMaxQueueDepth();
	assertTrue("MaxQueueDepth - default",  tmp == 100 );

	tmp = port.getCurrentQueueDepth();
	assertTrue("CurrentQueueDepth - new",  tmp == 0 );

	port.setMaxQueueDepth(22);
	tmp = port.getMaxQueueDepth();
	assertTrue("SetMaxQueueDepth - set value",  tmp == 22 );

	// check that port queue is empty
	bulkio.InInt64Port.Packet pkt  = port.getPacket( bulkio.Const.NON_BLOCKING );
	assertTrue("GetPacket - no data",  pkt == null );

	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - 1 SRI",  streams != null );
	assertTrue("Stream SRI Sequence - 1 SRI, length",  streams.length ==1 );

	long [] v = new long[0];
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid  );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 0  );

	// complex mode test
	sri.mode = ctx.mode;
	port.pushSRI(sri);

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - Complex SRI",  streams != null );
	assertTrue("Stream SRI Sequence - Complex, length",  streams.length ==1 );
	
	port.pushPacket( v, TS, false, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, CPLX pkt",  pkt != null );
	assertTrue("Get a Packet, CPLX StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, CPLX EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, CPLX mode",  pkt.SRI.mode == 1  );

	// test for EOS..
	port.pushPacket( v, TS, true, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == true );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 1  );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - SRI",  streams != null );
	assertTrue("Stream SRI Sequence - length",  streams.length !=1 );
	assertTrue("Stream SRI Sequence - length",  streams.length ==0 );
    }


    @Test
	public void test_InUInt8( ) {

	ctx=new test_fact("InUInt8");

	logger.info("------ Testing " + ctx.name + " Port -----");
		
	bulkio.InUInt8Port port = new bulkio.InUInt8Port(ctx.port_name);
	port.setSriListener( new test_stream_cb( ctx ) );

	//
	// simple attribute tests
	//
	port.setLogger( logger );

	// port statistics test
	port.enableStats( false );
	BULKIO.PortStatistics stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats == null );

	port.enableStats( true );
	stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	BULKIO.StreamSRI []streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence Failed",  streams != null );
	
	int tmp = port.getMaxQueueDepth();
	assertTrue("MaxQueueDepth - default",  tmp == 100 );

	tmp = port.getCurrentQueueDepth();
	assertTrue("CurrentQueueDepth - new",  tmp == 0 );

	port.setMaxQueueDepth(22);
	tmp = port.getMaxQueueDepth();
	assertTrue("SetMaxQueueDepth - set value",  tmp == 22 );

	// check that port queue is empty
	bulkio.InUInt8Port.Packet pkt  = port.getPacket( bulkio.Const.NON_BLOCKING );
	assertTrue("GetPacket - no data",  pkt == null );

	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - 1 SRI",  streams != null );
	assertTrue("Stream SRI Sequence - 1 SRI, length",  streams.length ==1 );

	byte [] v = new byte[0];
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid  );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 0  );

	// complex mode test
	sri.mode = ctx.mode;
	port.pushSRI(sri);

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - Complex SRI",  streams != null );
	assertTrue("Stream SRI Sequence - Complex, length",  streams.length ==1 );
	
	port.pushPacket( v, TS, false, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, CPLX pkt",  pkt != null );
	assertTrue("Get a Packet, CPLX StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, CPLX EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, CPLX mode",  pkt.SRI.mode == 1  );

	// test for EOS..
	port.pushPacket( v, TS, true, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == true );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 1  );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - SRI",  streams != null );
	assertTrue("Stream SRI Sequence - length",  streams.length !=1 );
	assertTrue("Stream SRI Sequence - length",  streams.length ==0 );
    }


    @Test
	public void test_InUInt16( ) {

	ctx=new test_fact("InUInt16");

	logger.info("------ Testing " + ctx.name + " Port -----");
		
	bulkio.InUInt16Port port = new bulkio.InUInt16Port(ctx.port_name);
	port.setSriListener( new test_stream_cb( ctx ) );

	//
	// simple attribute tests
	//
	port.setLogger( logger );

	// port statistics test
	port.enableStats( false );
	BULKIO.PortStatistics stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats == null );

	port.enableStats( true );
	stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	BULKIO.StreamSRI []streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence Failed",  streams != null );
	
	int tmp = port.getMaxQueueDepth();
	assertTrue("MaxQueueDepth - default",  tmp == 100 );

	tmp = port.getCurrentQueueDepth();
	assertTrue("CurrentQueueDepth - new",  tmp == 0 );

	port.setMaxQueueDepth(22);
	tmp = port.getMaxQueueDepth();
	assertTrue("SetMaxQueueDepth - set value",  tmp == 22 );

	// check that port queue is empty
	bulkio.InUInt16Port.Packet pkt  = port.getPacket( bulkio.Const.NON_BLOCKING );
	assertTrue("GetPacket - no data",  pkt == null );

	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - 1 SRI",  streams != null );
	assertTrue("Stream SRI Sequence - 1 SRI, length",  streams.length ==1 );

	short [] v = new short[0];
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid  );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 0  );

	// complex mode test
	sri.mode = ctx.mode;
	port.pushSRI(sri);

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - Complex SRI",  streams != null );
	assertTrue("Stream SRI Sequence - Complex, length",  streams.length ==1 );
	
	port.pushPacket( v, TS, false, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, CPLX pkt",  pkt != null );
	assertTrue("Get a Packet, CPLX StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, CPLX EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, CPLX mode",  pkt.SRI.mode == 1  );

	// test for EOS..
	port.pushPacket( v, TS, true, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == true );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 1  );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - SRI",  streams != null );
	assertTrue("Stream SRI Sequence - length",  streams.length !=1 );
	assertTrue("Stream SRI Sequence - length",  streams.length ==0 );
    }


    @Test
	public void test_InUInt32( ) {

	ctx=new test_fact("InUInt32");

	logger.info("------ Testing " + ctx.name + " Port -----");
		
	bulkio.InUInt32Port port = new bulkio.InUInt32Port(ctx.port_name);
	port.setSriListener( new test_stream_cb( ctx ) );

	//
	// simple attribute tests
	//
	port.setLogger( logger );

	// port statistics test
	port.enableStats( false );
	BULKIO.PortStatistics stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats == null );

	port.enableStats( true );
	stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	BULKIO.StreamSRI []streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence Failed",  streams != null );
	
	int tmp = port.getMaxQueueDepth();
	assertTrue("MaxQueueDepth - default",  tmp == 100 );

	tmp = port.getCurrentQueueDepth();
	assertTrue("CurrentQueueDepth - new",  tmp == 0 );

	port.setMaxQueueDepth(22);
	tmp = port.getMaxQueueDepth();
	assertTrue("SetMaxQueueDepth - set value",  tmp == 22 );

	// check that port queue is empty
	bulkio.InUInt32Port.Packet pkt  = port.getPacket( bulkio.Const.NON_BLOCKING );
	assertTrue("GetPacket - no data",  pkt == null );

	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - 1 SRI",  streams != null );
	assertTrue("Stream SRI Sequence - 1 SRI, length",  streams.length ==1 );

	int [] v = new int[0];
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid  );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 0  );

	// complex mode test
	sri.mode = ctx.mode;
	port.pushSRI(sri);

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - Complex SRI",  streams != null );
	assertTrue("Stream SRI Sequence - Complex, length",  streams.length ==1 );
	
	port.pushPacket( v, TS, false, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, CPLX pkt",  pkt != null );
	assertTrue("Get a Packet, CPLX StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, CPLX EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, CPLX mode",  pkt.SRI.mode == 1  );

	// test for EOS..
	port.pushPacket( v, TS, true, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == true );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 1  );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - SRI",  streams != null );
	assertTrue("Stream SRI Sequence - length",  streams.length !=1 );
	assertTrue("Stream SRI Sequence - length",  streams.length ==0 );
    }

    @Test
	public void test_InUInt64( ) {

	ctx=new test_fact("InUInt64");

	logger.info("------ Testing " + ctx.name + " Port -----");
		
	bulkio.InUInt64Port port = new bulkio.InUInt64Port(ctx.port_name);
	port.setSriListener( new test_stream_cb( ctx ) );

	//
	// simple attribute tests
	//
	port.setLogger( logger );

	// port statistics test
	port.enableStats( false );
	BULKIO.PortStatistics stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats == null );

	port.enableStats( true );
	stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	BULKIO.StreamSRI []streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence Failed",  streams != null );
	
	int tmp = port.getMaxQueueDepth();
	assertTrue("MaxQueueDepth - default",  tmp == 100 );

	tmp = port.getCurrentQueueDepth();
	assertTrue("CurrentQueueDepth - new",  tmp == 0 );

	port.setMaxQueueDepth(22);
	tmp = port.getMaxQueueDepth();
	assertTrue("SetMaxQueueDepth - set value",  tmp == 22 );

	// check that port queue is empty
	bulkio.InUInt64Port.Packet pkt  = port.getPacket( bulkio.Const.NON_BLOCKING );
	assertTrue("GetPacket - no data",  pkt == null );

	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - 1 SRI",  streams != null );
	assertTrue("Stream SRI Sequence - 1 SRI, length",  streams.length ==1 );

	long [] v = new long[0];
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid  );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 0  );

	// complex mode test
	sri.mode = ctx.mode;
	port.pushSRI(sri);

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - Complex SRI",  streams != null );
	assertTrue("Stream SRI Sequence - Complex, length",  streams.length ==1 );
	
	port.pushPacket( v, TS, false, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, CPLX pkt",  pkt != null );
	assertTrue("Get a Packet, CPLX StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, CPLX EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, CPLX mode",  pkt.SRI.mode == 1  );

	// test for EOS..
	port.pushPacket( v, TS, true, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == true );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 1  );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - SRI",  streams != null );
	assertTrue("Stream SRI Sequence - length",  streams.length !=1 );
	assertTrue("Stream SRI Sequence - length",  streams.length ==0 );
    }


    @Test
	public void test_InDouble( ) {

	ctx=new test_fact("InDouble");

	logger.info("------ Testing " + ctx.name + " Port -----");
		
	bulkio.InDoublePort port = new bulkio.InDoublePort(ctx.port_name);
	port.setSriListener( new test_stream_cb( ctx ) );

	//
	// simple attribute tests
	//
	port.setLogger( logger );

	// port statistics test
	port.enableStats( false );
	BULKIO.PortStatistics stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats == null );

	port.enableStats( true );
	stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	BULKIO.StreamSRI []streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence Failed",  streams != null );
	
	int tmp = port.getMaxQueueDepth();
	assertTrue("MaxQueueDepth - default",  tmp == 100 );

	tmp = port.getCurrentQueueDepth();
	assertTrue("CurrentQueueDepth - new",  tmp == 0 );

	port.setMaxQueueDepth(22);
	tmp = port.getMaxQueueDepth();
	assertTrue("SetMaxQueueDepth - set value",  tmp == 22 );

	// check that port queue is empty
	bulkio.InDoublePort.Packet pkt  = port.getPacket( bulkio.Const.NON_BLOCKING );
	assertTrue("GetPacket - no data",  pkt == null );

	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - 1 SRI",  streams != null );
	assertTrue("Stream SRI Sequence - 1 SRI, length",  streams.length ==1 );

	double [] v = new double[0];
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid  );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 0  );

	// complex mode test
	sri.mode = ctx.mode;
	port.pushSRI(sri);

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - Complex SRI",  streams != null );
	assertTrue("Stream SRI Sequence - Complex, length",  streams.length ==1 );
	
	port.pushPacket( v, TS, false, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, CPLX pkt",  pkt != null );
	assertTrue("Get a Packet, CPLX StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, CPLX EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, CPLX mode",  pkt.SRI.mode == 1  );

	// test for EOS..
	port.pushPacket( v, TS, true, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == true );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 1  );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - SRI",  streams != null );
	assertTrue("Stream SRI Sequence - length",  streams.length !=1 );
	assertTrue("Stream SRI Sequence - length",  streams.length ==0 );
    }


    @Test
	public void test_InFloat( ) {

	ctx=new test_fact("InFloat");

	logger.info("------ Testing " + ctx.name + " Port -----");
		
	bulkio.InFloatPort port = new bulkio.InFloatPort(ctx.port_name);
	port.setSriListener( new test_stream_cb( ctx ) );

	//
	// simple attribute tests
	//
	port.setLogger( logger );

	// port statistics test
	port.enableStats( false );
	BULKIO.PortStatistics stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats == null );

	port.enableStats( true );
	stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	BULKIO.StreamSRI []streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence Failed",  streams != null );
	
	int tmp = port.getMaxQueueDepth();
	assertTrue("MaxQueueDepth - default",  tmp == 100 );

	tmp = port.getCurrentQueueDepth();
	assertTrue("CurrentQueueDepth - new",  tmp == 0 );

	port.setMaxQueueDepth(22);
	tmp = port.getMaxQueueDepth();
	assertTrue("SetMaxQueueDepth - set value",  tmp == 22 );

	// check that port queue is empty
	bulkio.InFloatPort.Packet pkt  = port.getPacket( bulkio.Const.NON_BLOCKING );
	assertTrue("GetPacket - no data",  pkt == null );

	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	port.pushSRI( sri );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - 1 SRI",  streams != null );
	assertTrue("Stream SRI Sequence - 1 SRI, length",  streams.length ==1 );

	float [] v = new float[0];
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushPacket( v, TS, false, ctx.sid  );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 0  );

	// complex mode test
	sri.mode = ctx.mode;
	port.pushSRI(sri);

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - Complex SRI",  streams != null );
	assertTrue("Stream SRI Sequence - Complex, length",  streams.length ==1 );
	
	port.pushPacket( v, TS, false, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, CPLX pkt",  pkt != null );
	assertTrue("Get a Packet, CPLX StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, CPLX EOS",  pkt.EOS == false );
	assertTrue("Get a Packet, CPLX mode",  pkt.SRI.mode == 1  );

	// test for EOS..
	port.pushPacket( v, TS, true, ctx.sid );

	// grab off packet
	pkt  = port.getPacket(bulkio.Const.NON_BLOCKING );
	assertTrue("Get a Packet, pkt",  pkt != null );
	assertTrue("Get a Packet, StreamID",  pkt.SRI.streamID== ctx.sid  );
	assertTrue("Get a Packet, EOS",  pkt.EOS == true );
	assertTrue("Get a Packet, mode",  pkt.SRI.mode == 1  );

	streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence - SRI",  streams != null );
	assertTrue("Stream SRI Sequence - length",  streams.length !=1 );
	assertTrue("Stream SRI Sequence - length",  streams.length ==0 );
    }

}
