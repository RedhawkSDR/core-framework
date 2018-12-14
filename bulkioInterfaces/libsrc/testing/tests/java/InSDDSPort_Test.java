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

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import org.apache.log4j.Logger;

import BULKIO.StreamSRI;

/**
 * Tests for {@link Foo}.
 *
 * @author 
 */
@RunWith(JUnit4.class)
public class InSDDSPort_Test {

    class test_fact {

	String  name = "InInt8";

	String  port_name = new String("test-inport-api");

	String  sid = new String("test-inport-streamid");

	short   mode = 1;

	double  srate=22.0;

	String user_id =  "test_sdds_port_api";

	String ip_addr = "1.1.1.1";

	String  aid = null;

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

	public boolean changedSRI( StreamSRI sri ) {
	    assertTrue("changedSRI SRI Object Invalid",  null != sri );	    
	    assertTrue("changedSRI Mode Mismatch",  ctx.mode == sri.mode );
            return true;
	}
    }

    class test_sdds_cb implements bulkio.InSDDSPort.Callback  {

	test_fact ctx=null;

	test_sdds_cb ( test_fact inCtx ) {
	    ctx=inCtx;
	}


	public String attach(BULKIO.SDDSStreamDefinition stream, String userid) {
	    assertTrue("sdds stream def,  null",  null != stream );	    
	    assertTrue("sdds stream ip addr mismatch",  stream.multicastAddress == ctx.ip_addr );	    
	    assertTrue("sdds stream user_id mismatch",  userid == ctx.user_id );
	    return null;
	}


        public void detach(String attachId)  {
	    assertTrue("detach ID, ",  null != attachId );	    
	    assertTrue("detach ID, Mismatch",  ctx.aid == attachId );
	}

    };


    Logger logger =  Logger.getRootLogger();

    test_fact  ctx = null;

    @Test
    public void test_InSDDS( ) {


	ctx=new test_fact("InSDDS");

	bulkio.InSDDSPort port = new bulkio.InSDDSPort(ctx.port_name );

	port.setSriListener( new test_stream_cb( ctx ) );
	port.setAttachDetachCallback( new test_sdds_cb( ctx ) );

	port.setLogger(logger);

	port.enableStats(false);

	BULKIO.PortStatistics stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats == null );

	port.enableStats(true);
	stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );

	BULKIO.PortUsageType rt = port.state();
	assertTrue("Port Usage Type Failed",  rt == BULKIO.PortUsageType.IDLE );

	BULKIO.StreamSRI []streams = port.activeSRIs();
	assertTrue("Stream SRI Sequence Failed",  streams != null );
	assertTrue("Stream SRI Sequence - 1 SRI, length",  streams.length ==0 );

	streams = port.attachedSRIs();
	assertTrue("Stream SRI Sequence - 1 SRI",  streams != null );
	assertTrue("Stream SRI Sequence - 1 SRI, length",  streams.length ==0 );

	String [] ids = port.attachmentIds();
	assertTrue("Attached ID List, empty",  ids != null );
	assertTrue("Attached ID List, length",  ids.length == 0 );

	try {
	    String uid = port.getUser("some aid");
	    assertTrue("Check attached user",  uid == null );
	}
	catch(Exception e ) {
	}
	
	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushSRI( sri, TS );

	//	streams = port.activeSRIs();
	//	assertTrue("Stream SRI Sequence - 1 SRI",  streams != null );
	//	assertTrue("Stream SRI Sequence - 1 SRI, length",  streams.length ==1 );

	BULKIO.SDDSStreamDefinition sdef = new BULKIO.SDDSStreamDefinition();
	sdef.id = "test_sdds_id";
	sdef.dataFormat = BULKIO.SDDSDataDigraph.SDDS_SB;
	sdef.multicastAddress = ctx.ip_addr;
	sdef.vlan = 1234;
	sdef.port = 5678;
	String aid=null;
	try {
	    aid = port.attach( sdef, ctx.user_id );
	    assertTrue("Attach ID, not null ",  aid != null );
	}
	catch( Exception e ){
	}
	ctx.aid = aid;
	BULKIO.SDDSStreamDefinition [] sss = port.attachedStreams();
	assertTrue("attachStreams failed ",  sss != null );
	assertTrue("attachStreams failed ",  sss.length == 1);
	String paddr = sss[0].multicastAddress;
	assertTrue("attachStreams ip addr failed ",  paddr == ctx.ip_addr );

	streams = port.attachedSRIs();
	assertTrue("Stream SRI Sequence - 1 SRI",  streams != null );
	assertTrue("Stream SRI Sequence - 1 SRI, length",  streams.length ==1 );

	ids = port.attachmentIds();
	assertTrue("Attached ID List, empty",  ids != null );
	assertTrue("Attached ID List, length",  ids.length == 1 );

	try {
	    String uid = port.getUser(aid);
	    assertTrue("Check attached user",  uid != null );
	    assertTrue("Check attached user value",  uid == ctx.user_id );
	}
	catch(Exception e ) {
	}

	try {
	    port.detach(aid);
	}
	catch(Exception e ) {
	}

	sss = port.attachedStreams();
	assertTrue("attachStreams failed ",  sss != null );
	assertTrue("attachStreams failed ",  sss.length == 0);

	ids = port.attachmentIds();
	assertTrue("Attached ID List, empty",  ids != null );
	assertTrue("Attached ID List, length",  ids.length == 0 );

    }


}
