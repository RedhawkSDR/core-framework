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

import bulkio.ConnectionEventListener;

/**
 * Tests for {@link Foo}.
 *
 * @author 
 */
@RunWith(JUnit4.class)
public class OutSDDSPort_Test {

    Logger logger =  Logger.getRootLogger();

    class test_fact {

	String  name = "OutInt8";

	String  port_name = new String("test-outport-api");

	String  sid = new String("test-outport-streamid");

	String  cid = new String("connect-1");

	short   mode = 1;

	double  srate=22.0;

	String user_id =  "test_sdds_port_api";

	String ip_addr = "1.1.1.1";

	String  aid = null;


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



    @Test
    public void test_OutSDDS( ) {

	test_fact ctx = new test_fact( "OutSDDS" );

	bulkio.OutSDDSPort port = new bulkio.OutSDDSPort(ctx.port_name);

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
	    port.disconnectPort( ctx.cid );
	}
	catch(Exception e){
	}

	// push sri
	BULKIO.StreamSRI sri;
	sri = bulkio.sri.utils.create();
	sri.streamID = ctx.sid;
	BULKIO.PrecisionUTCTime TS = bulkio.time.utils.now();
	port.pushSRI( sri, TS );
	BULKIO.StreamSRI[] sris= port.activeSRIs();
	assertTrue("Current SRIs Failed",  sris.length == 1 );

        // Pushing an SRI with a null streamID should trigger an NPE
        sri = new BULKIO.StreamSRI();
        sri.streamID = null;
        boolean received_npe = false;
        try {
            port.pushSRI(sri, TS);
        } catch (NullPointerException npe) {
            received_npe = true;
        }
        assertTrue("Did not raise NPE for null streamID", received_npe);

	port.enableStats( true );
	try {
	    port.connectPort( p, ctx.cid );
	}
	catch(Exception e){
	}

	stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );
	assertTrue("Port Statistics Failed, length",  stats.length == 1 );
	assertTrue("Port Statistics Failed, connection id",  stats[0].connectionId == ctx.cid );

	try {
	    port.disconnectPort( ctx.cid );
	    port.disconnectPort( ctx.cid );
	}
	catch(Exception e){
	}

	stats = port.statistics();
	assertTrue("Port Statistics Failed",  stats != null );
	assertTrue("Port Statistics Failed, length",  stats.length == 0 );

    }


}
