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
import java.util.List;
import java.util.ArrayList;
import CF.DataType;
import BULKIO.StreamSRI;
import BULKIO.PrecisionUTCTime;
import BULKIO.PortStatistics;
import BULKIO.PortUsageType;

/**
 * Tests for {@link Foo}.
 *
 * @author 
 */
@RunWith(JUnit4.class)
public class Burstio_Utils_Test {

    Logger logger =  Logger.getRootLogger();

    @BeforeClass
	public static void oneTimeSetUp() {
	// Set up a simple configuration that logs on the console.
	BasicConfigurator.configure();
    }

    @AfterClass
	public static void oneTimeTearDown() {
        LogManager.shutdown();
    }

    @Before
	public void setUp() {

    }

    @After
	public void tearDown() {
	
    }

    @Test
	public void test_time_now( ) {

	logger.debug("------ Testing  bulkio.Utils.now -----");
        BULKIO.PrecisionUTCTime ts = burstio.Utils.now();
	assertEquals( " tcmode mismatch.", ts.tcmode,BULKIO.TCM_CPU.value );
        assertEquals( " tcstatus mismatch.", ts.tcstatus,BULKIO.TCS_VALID.value );
    }


    @Test
	public void test_sri_create( ) {

	logger.debug("------ Testing  burstio.Utils.createSRI -----");

        BURSTIO.BurstSRI sri = burstio.Utils.createSRI( "defaultSRI" );
        assertEquals("Stream ID mismatch.", "defaultSRI", sri.streamID);
        assertEquals("Version mismatch.", 1, sri.hversion );
        assertEquals("XDelta mismatch.", sri.xdelta,1.000,3);
        assertEquals("Mode mismatch.", sri.mode,0);
        assertEquals("Flags mismatch.", sri.flags,0);
        assertEquals("Tau mismatch.", sri.tau,0.00,3);
        assertEquals("Theta mismatch.", sri.theta,0.00,3);
        assertEquals("UW Length mismatch.", sri.uwlength,0);
        assertEquals("Burst Type mismatch.", sri.bursttype,0);
        assertEquals("Burst Length mismatch.", sri.burstLength,0);
        assertEquals("CHAN_RF mismatch.", sri.CHAN_RF,0.00,3);
        assertEquals("Baud Estimate mismatch.", sri.baudestimate,0.00,3);
        assertEquals("Baud Rate mismatch.", sri.baudrate,0.00,3);
        assertEquals("Carrier Offset mismatch.", sri.carrieroffset,0.00,3);
        assertEquals("SNR mismatch.", sri.SNR,0.00,3);
        assertEquals("Modulation mismatch.", sri.modulation, "");
        assertEquals("FEC mismatch.", sri.fec, "");
        assertEquals("FEC Rate mismatch.", sri.fecrate, "");
        assertEquals("Randomizer mismatch.", sri.randomizer, "");
        assertEquals("Overhead mismatch.", sri.overhead, "");
        assertEquals("Keywords Length mismatch.", sri.keywords.length, 0);

    }


    @Test
	public void test_keywords( ) {

	logger.debug("------ Testing  burstio.Utils.keywords -----");

        List<CF.DataType> kwl = new ArrayList<CF.DataType>();

        BURSTIO.BurstSRI sri = burstio.Utils.createSRI( "defaultSRI" );

        burstio.Utils.addKeyword( kwl, "test-kw-one", 22.5 );

        assertEquals("Keywords Length mismatch.", kwl.size(), 1);


        burstio.Utils.addKeyword( kwl, "test-kw-two", 111.5 );

        assertEquals("Keywords Length mismatch.", kwl.size(), 2);

    }

    @Test
	public void test_time_elapsed( ) {

	logger.debug("------ Testing  burstio.Utils.elapsed -----");

        BULKIO.PrecisionUTCTime b = burstio.Utils.now();
        BULKIO.PrecisionUTCTime e = b;
        
        double elapsed = burstio.Utils.elapsed( b, e );
        assertEquals("Elapse same begin/end mismatch.", elapsed,0.00,3);

        e=burstio.Utils.now();
        b.twsec = 100;
        b.tfsec = 0;
        e.twsec = 200;
        e.tfsec = 0;
        elapsed = burstio.Utils.elapsed( b, e );
        assertEquals("Elapse same begin/end mismatch.", elapsed,100.0,4);

        e=burstio.Utils.now();
        b.twsec = 100;
        b.tfsec = 50;
        e.twsec = 200;
        e.tfsec = 100;
        elapsed = burstio.Utils.elapsed( b, e );
        assertEquals("Elapse same begin/end mismatch.", elapsed,150.0,4);
    }


}
