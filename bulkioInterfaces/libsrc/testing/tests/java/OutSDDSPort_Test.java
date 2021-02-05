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
import org.omg.CORBA.ORB;

/**
 * Tests for {@link Foo}.
 *
 * @author 
 */
@RunWith(JUnit4.class)
public class OutSDDSPort_Test implements bulkio.InSDDSPort.Callback {

    Logger logger =  Logger.getRootLogger();
    int attachIdCount;
    int good_port = 12345;
    int bad_port = 54321;

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

    public String attach(BULKIO.SDDSStreamDefinition stream, String userid) throws BULKIO.dataSDDSPackage.AttachError, BULKIO.dataSDDSPackage.StreamInputError {
        if (stream.port == this.bad_port) {
            throw new BULKIO.dataSDDSPackage.AttachError("bad port");
        }
        String retval = new String("hello");
        retval += Integer.toString(this.attachIdCount);
        this.attachIdCount += 1;
        return retval;
    };

    public void detach(String attachId) throws BULKIO.dataSDDSPackage.DetachError, BULKIO.dataSDDSPackage.StreamInputError {
    };

    @Test
    public void test_OutSDDS_attachFail( ) throws CF.PortPackage.InvalidPort, CF.PortPackage.OccupiedPort, BULKIO.dataSDDSPackage.DetachError, BULKIO.dataSDDSPackage.AttachError, BULKIO.dataSDDSPackage.StreamInputError {

        this.attachIdCount = 0;
        test_fact out_ctx = new test_fact( "OutSDDS" );
        test_fact in_ctx=new test_fact("InSDDS");

        bulkio.OutSDDSPort out_port = new bulkio.OutSDDSPort(out_ctx.port_name);

        bulkio.InSDDSPort in_port = new bulkio.InSDDSPort(in_ctx.port_name );
        
        BULKIO.SDDSStreamDefinition newStreamDef = new BULKIO.SDDSStreamDefinition();
        newStreamDef.id = new String("some_id");
        newStreamDef.dataFormat = BULKIO.SDDSDataDigraph.SDDS_SF;
        newStreamDef.multicastAddress = new String("0.0.0.0");
        newStreamDef.privateInfo = new String("");
        newStreamDef.port = this.good_port;

        in_port.setAttachDetachCallback(this);

        String pname = out_port.getName();
        assertTrue("Port Name Failed",  pname == out_ctx.port_name );
        out_port.connectPort( in_port._this_object(ORB.init((String[]) null, null)), out_ctx.cid );
        boolean retval = out_port.addStream(newStreamDef);
        assertTrue(retval);

        BULKIO.SDDSStreamDefinition invalidStreamDef = new BULKIO.SDDSStreamDefinition();
        invalidStreamDef.id = new String("another_id");
        invalidStreamDef.dataFormat = BULKIO.SDDSDataDigraph.SDDS_SF;
        invalidStreamDef.multicastAddress = new String("0.0.0.0");
        invalidStreamDef.privateInfo = new String("");
        invalidStreamDef.port = this.bad_port;

        boolean exception_thrown = false;
        try {
            retval = out_port.addStream(invalidStreamDef);
        } catch (BULKIO.dataSDDSPackage.AttachError e) {
            exception_thrown = true;
        }
        assertTrue(exception_thrown);
        exception_thrown = false;
        try {
            String[] attach_id = out_port.attach(invalidStreamDef, out_ctx.user_id);
        } catch (BULKIO.dataSDDSPackage.AttachError e) {
            exception_thrown = true;
        }
        assertTrue(exception_thrown);

        BULKIO.SDDSStreamDefinition validStreamDef = new BULKIO.SDDSStreamDefinition();
        validStreamDef.id = new String("another_id");
        validStreamDef.dataFormat = BULKIO.SDDSDataDigraph.SDDS_SF;
        validStreamDef.multicastAddress = new String("0.0.0.0");
        validStreamDef.privateInfo = new String("");
        validStreamDef.port = this.good_port;

        retval = out_port.addStream(validStreamDef);
        assertTrue(retval);
    }
}
