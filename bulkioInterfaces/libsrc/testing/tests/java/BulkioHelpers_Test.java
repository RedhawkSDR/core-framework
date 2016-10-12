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
import org.ossie.properties.AnyUtils;
import CF.DataType;
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
public class BulkioHelpers_Test {

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

	public boolean changedSRI( StreamSRI sri ) {
	    assertTrue("changedSRI SRI Object Invalid",  null != sri );	    
	    assertTrue("changedSRI Mode Mismatch",  ctx.mode == sri.mode );
            return true;
	}
    }


    Logger logger =  Logger.getRootLogger();

    test_fact  ctx = null;

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
	public void test_sri_create( ) {

	logger.info("------ Testing  bulkio.sri.utils.create -----");

        //
        //
        //
        BULKIO.StreamSRI sri = bulkio.sri.utils.create();
        assertEquals("Stream ID mismatch.", "defaultSRI", sri.streamID);
        assertEquals("Version mismatch.", 1, sri.hversion );
	assertEquals("XUnits mismatch.", 1, sri.xunits );
	assertEquals("XStart mismatch.", sri.xstart,0.00,3 );
        assertEquals("XDelta mismatch.", sri.xdelta,1.00,3);
        assertEquals("YUnits mismatch.", sri.yunits,0);
        assertEquals("YStart mismatch.", sri.ystart,0.00,3);
        assertEquals( "YDelta mismatch.", sri.ydelta,0.00,3);
        assertEquals("Subsize mismatch.", sri.subsize,0);
        assertFalse("Blocking mismatch.", sri.blocking);
	CF.DataType []empty= new CF.DataType[0];
        assertEquals("Keywords mismatch.", sri.keywords,empty);
	sri = bulkio.sri.utils.create( "NEW-STREAM-ID", 22.0, (short)22, false);
        assertEquals("Stream ID mismatch.", sri.streamID,"NEW-STREAM-ID");
        assertEquals("SRATE  mismatch.", sri.xdelta,1/22.00, 3);
        assertEquals("XUNITS  mismatch.", sri.xunits,22);
        assertFalse("BLOCKING  mismatch.", sri.blocking );
    }


    @Test
	public void test_sri_compare( ) {

	logger.info("------ Testing bulkio.sri.DefaultComparator -----");;

	BULKIO.StreamSRI a_sri = bulkio.sri.utils.create();
	BULKIO.StreamSRI b_sri = bulkio.sri.utils.create();
	BULKIO.StreamSRI c_sri = bulkio.sri.utils.create();
	c_sri.streamID = "THIS_DOES_NOT_MATCH";

	bulkio.sri.Comparator bio_cmp = new bulkio.sri.DefaultComparator();;

        assertTrue("bio_cmp method - same.",  bio_cmp.compare( a_sri, b_sri ) );
        assertFalse(" bio_cmp method - different - StreamID .", bio_cmp.compare( a_sri, c_sri ) );

        c_sri = bulkio.sri.utils.create();
        c_sri.hversion = 2;
	assertFalse( " bio_cmp method - different - hversion ", bio_cmp.compare( a_sri, c_sri ) );

        c_sri = bulkio.sri.utils.create();
        c_sri.xstart = 3;
        assertFalse( " bio_cmp method - different - xstart ",  bio_cmp.compare( a_sri, c_sri ) );

	c_sri = bulkio.sri.utils.create();
	c_sri.xdelta = 100.0;
        assertFalse( " bio_cmp method - different - xdelta ", bio_cmp.compare( a_sri, c_sri ) );

        c_sri = bulkio.sri.utils.create();
        c_sri.xunits = 100;
        assertFalse( " bio_cmp method - different - xunits ", bio_cmp.compare( a_sri, c_sri ) );

        c_sri = bulkio.sri.utils.create();
        c_sri.subsize = 100;
        assertFalse( " bio_cmp method - different - subsize ", bio_cmp.compare( a_sri, c_sri ) );

        c_sri = bulkio.sri.utils.create();
        c_sri.ystart = 3;
        assertFalse( " bio_cmp method - different - ystart ",  bio_cmp.compare( a_sri, c_sri ) );

        c_sri = bulkio.sri.utils.create();
        c_sri.ydelta = 100.0;
        assertFalse( " bio_cmp method - different - ydelta ", bio_cmp.compare( a_sri, c_sri ) );

        c_sri = bulkio.sri.utils.create();
        c_sri.yunits = 100;
        assertFalse( " bio_cmp method - different - yunits ", bio_cmp.compare( a_sri, c_sri ) );

        c_sri = bulkio.sri.utils.create();
        c_sri.mode = 100;
        assertFalse( " bio_cmp method - different - mode ", bio_cmp.compare( a_sri, c_sri )  );

        CF.DataType kv = new CF.DataType( "key_one", AnyUtils.stringToAny("1", "long") );
        CF.DataType kv2 = new CF.DataType( "key_one",  AnyUtils.stringToAny("1", "long") );
        a_sri.keywords = new CF.DataType[1];
        a_sri.keywords[0] = kv;
        c_sri = bulkio.sri.utils.create();
        c_sri.keywords = new CF.DataType[1];
        c_sri.keywords[0] = kv2;
	assertTrue( " bio_cmp method - same - keyword item ",  bio_cmp.compare( a_sri, c_sri ) );

        kv2 = new CF.DataType( "key_one", AnyUtils.stringToAny("100", "long") );
        c_sri = bulkio.sri.utils.create();
        c_sri.keywords = new CF.DataType[1];
        c_sri.keywords[0] = kv2;
        assertFalse( " bio_cmp method - different - keywords value mismatch ",
		     bio_cmp.compare( a_sri, c_sri ) );

        kv2 = new CF.DataType( "key_two", AnyUtils.stringToAny("100", "long") );
        c_sri = bulkio.sri.utils.create();
        c_sri.keywords = new CF.DataType[1];
        c_sri.keywords[0] = kv2;
        assertFalse(" bio_cmp method - different - keywords name mismatch  ",
		   bio_cmp.compare( a_sri, c_sri ) );

    }


    @Test
	public void test_timestamp_create( ) {

	logger.info("------ Testing bulkio.time.utils.create -----");

        BULKIO.PrecisionUTCTime ts = bulkio.time.utils.now();
	assertEquals( " tcmode mismatch.", ts.tcmode,BULKIO.TCM_CPU.value );
        assertEquals( " tcstatus mismatch.", ts.tcstatus,BULKIO.TCS_VALID.value );
        assertEquals(" tcoff  mismatch.", ts.toff,0.00,3 );

        ts = bulkio.time.utils.create( 100.0, 0.125 );
	assertEquals( " tcmode mismatch.", ts.tcmode,BULKIO.TCM_CPU.value );
        assertEquals( " tcstatus mismatch.", ts.tcstatus,BULKIO.TCS_VALID.value );
        assertEquals( " tcwsec mismatch.", ts.twsec,100.0, 3 );
        assertEquals( " tcwsec mismatch.", ts.tfsec,0.125, 3 );

        ts = bulkio.time.utils.create( 100.0, 0.125, BULKIO.TCM_SDDS.value );
	assertEquals( " tcmode mismatch.", ts.tcmode,BULKIO.TCM_SDDS.value );
        assertEquals( " tcstatus mismatch.", ts.tcstatus,BULKIO.TCS_VALID.value );
        assertEquals( " tcwsec mismatch.", ts.twsec,100.0, 3 );
        assertEquals( " tcwsec mismatch.", ts.tfsec,0.125, 3 );


    }



}
