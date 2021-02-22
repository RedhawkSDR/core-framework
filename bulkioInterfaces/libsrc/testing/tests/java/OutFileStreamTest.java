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
 * WARNING: This file is generated from NumericOutPortTest.java.template.
 *          Do not modify directly.
 */

import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;

import org.omg.PortableServer.Servant;
import org.omg.CORBA.ORB;
import org.omg.CORBA.Any;

import org.junit.*;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import helpers.ArrayData;
import helpers.FileTestHelper;
import stubs.Stub;
import bulkio.OutFilePort;
import bulkio.OutFileStream;

@RunWith(JUnit4.class)
public class OutFileStreamTest {

    /**
     * Input port being tested
     */
    protected OutFilePort port;
    protected Stub<String> stub;
    protected List<Servant> servants = new ArrayList<Servant>();

    /**
     * External CORBA interface to the tested port.
     */
    protected BULKIO.dataFileOperations corbaPort;

    protected FileTestHelper helper;

    public OutFileStreamTest()
    {
        this.helper = new FileTestHelper();
    }

    protected Stub<String> _createStub() throws org.omg.CORBA.UserException
    {
        Stub<String> new_stub = helper.createStub();
        org.omg.PortableServer.Servant servant = new_stub.servant();
        org.ossie.corba.utils.RootPOA().activate_object(servant);
        servants.add(servant);
        return new_stub;
    }

    protected void _writeSinglePacket(OutFileStream stream, int size)
    {
        BULKIO.PrecisionUTCTime t = bulkio.time.utils.now();
        this._writeSinglePacket(stream, size, t);
    }

    protected String _generateString(int size)
    {
        char[] array = new char[size];
        char _c = 'a';
        Arrays.fill(array, _c);
        String buffer = new String(array);
        return buffer;
    }

    protected void _writeSinglePacket(OutFileStream stream, int size, BULKIO.PrecisionUTCTime time)
    {
        String buffer = _generateString(size);
        stream.write(buffer, time);
    }

    @Before
    @SuppressWarnings("unchecked")
    public void setUp() throws org.omg.CORBA.UserException
    {
        String name = helper.getName() + "_out";
        port = helper.createOutPort(name);
        org.ossie.corba.utils.RootPOA().activate_object(port);

        stub = _createStub();

        org.omg.CORBA.Object objref = stub._this();
        port.connectPort(objref, "connection_1");
    }

    @Test
    public void testOperators()
    {
        // Create a new stream
        OutFileStream good_stream = this.port.createStream("test_operators");
        Assert.assertNotNull(good_stream);

        // Get another handle to the same stream, should be equal
        OutFileStream same_stream = port.getStream("test_operators");
        Assert.assertEquals(same_stream, good_stream);

        // Create a new stream, should not be equal
        OutFileStream other_stream = port.createStream("test_operators_2");
        Assert.assertNotEquals(other_stream, good_stream);
    }

    @Test
    public void testBasicWrite()
    {
        OutFileStream stream = port.createStream("test_basic_write");
        Assert.assertTrue(stub.packets.isEmpty());

        BULKIO.PrecisionUTCTime time = bulkio.time.utils.now();
        _writeSinglePacket(stream, 256, time);
        Assert.assertEquals(stub.packets.size(), 1);
        Assert.assertEquals(256, stub.packets.get(0).data.length());
        Assert.assertFalse(stub.packets.get(0).EOS);
        Assert.assertEquals(stream.streamID(), stub.packets.get(0).streamID);
    }

    @Test
    public void testSriFields()
    {
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("test_sri");
        sri.xstart = -2.5;
        sri.xdelta = 0.125;
        sri.xunits = BULKIO.UNITS_FREQUENCY.value;
        sri.subsize = 1024;
        sri.ystart = 2.5;
        sri.ydelta = 1.0;
        sri.yunits = BULKIO.UNITS_TIME.value;
        sri.mode = 1;
        sri.blocking = true;
        sri.keywords = new CF.DataType[2];
        sri.keywords[0] = new CF.DataType();
        sri.keywords[0].value = ORB.init().create_any();
        sri.keywords[0].id = "string";
        sri.keywords[0].value.insert_string("value");
        sri.keywords[1] = new CF.DataType();
        sri.keywords[1].value = ORB.init().create_any();
        sri.keywords[1].id = "number";
        sri.keywords[1].value.insert_long(100);
    
        // Create a stream from the SRI; assign to a const variable to ensure that
        // all accessors are const-safe
        OutFileStream stream = port.createStream(sri);
        Assert.assertTrue(stream.streamID().equals(sri.streamID));
        Assert.assertTrue(stream.xstart() == sri.xstart);
        Assert.assertTrue(stream.xdelta() == sri.xdelta);
        Assert.assertTrue(stream.xunits() == sri.xunits);
        Assert.assertTrue(stream.subsize() == sri.subsize);
        Assert.assertTrue(stream.ystart() == sri.ystart);
        Assert.assertTrue(stream.ydelta() == sri.ydelta);
        Assert.assertTrue(stream.yunits() == sri.yunits);
        Assert.assertTrue(stream.complex());
        Assert.assertTrue(stream.blocking());
        Assert.assertTrue(sri.keywords.length == stream.keywords().length);
        String value = stream.getKeyword("string").extract_string();
        Assert.assertTrue(value.equals("value"));
        Assert.assertEquals(100, stream.getKeyword("number").extract_long());
    }

    // Test that the SRI is not pushed when not needed.
    // Calls to set attributes of the SRI, or set or erase SRI keywords should
    // only cause the SRI to be pushed if a change was made.
    @Test
    public void testSriNoChange()
    {
        int expected_H_size = 0;

        OutFileStream stream = port.createStream("test_sri_no_change");
        BULKIO.StreamSRI sri = stream.sri();

        Assert.assertEquals(stub.H.size(), expected_H_size);

        _writeSinglePacket(stream, 10);
        expected_H_size++;
        Assert.assertEquals(stub.H.size(), expected_H_size);

        //BULKIO.StreamSRI changed_sri = stream.sri();
        //changed_sri.streamID = "changed_sri";
        stream.sri(sri);
        _writeSinglePacket(stream, 10);
        Assert.assertEquals(stub.H.size(), expected_H_size);

        stream.xdelta(3.0);
        expected_H_size++;
        _writeSinglePacket(stream, 10);
        Assert.assertEquals(stub.H.size(), expected_H_size);

        stream.xdelta(3.0);
        _writeSinglePacket(stream, 10);
        Assert.assertEquals(stub.H.size(), expected_H_size);

        CF.DataType[] props = new CF.DataType[2];
        props[0] = new CF.DataType();
        props[0].value = ORB.init().create_any();
        props[0].id = "foo";
        props[0].value.insert_string("word1");
        props[1] = new CF.DataType();
        props[1].value = ORB.init().create_any();
        props[1].id = "bar";
        props[1].value.insert_string("word2");
        stream.keywords(props);
        expected_H_size++;
        _writeSinglePacket(stream, 10);
        Assert.assertEquals(stub.H.size(), expected_H_size);

        stream.setKeyword("foo", "word8");
        expected_H_size++;
        _writeSinglePacket(stream, 10);
        Assert.assertEquals(stub.H.size(), expected_H_size);

        stream.eraseKeyword("bar");
        expected_H_size++;
        _writeSinglePacket(stream, 10);
        Assert.assertEquals(stub.H.size(), expected_H_size);

        stream.eraseKeyword("bar");
        _writeSinglePacket(stream, 10);
        Assert.assertEquals(stub.H.size(), expected_H_size);
    }

    @Test
    public void testSriUpdate()
    {
        // Create initial stream; all changes should be queued up for the first
        // write
        OutFileStream stream = port.createStream("test_sri_update");
        double xdelta = 1.0 / 1.25e6;
        stream.xdelta(xdelta);
        stream.blocking(true);
        Assert.assertTrue(stub.H.isEmpty());

        // Write data to trigger initial SRI update
        _writeSinglePacket(stream, 10);
        Assert.assertTrue(stub.H.size() == 1);
        Assert.assertTrue(stub.H.get(stub.H.size()-1).blocking);
        Assert.assertEquals(Double.compare(xdelta, stub.H.get(stub.H.size()-1).xdelta), 0);

        // Update xdelta; no SRI update should occur
        double new_xdelta = 1.0/2.5e6;
        stream.xdelta(new_xdelta);
        Assert.assertTrue(stub.H.size() == 1);
        Assert.assertEquals(Double.compare(xdelta, stub.H.get(stub.H.size()-1).xdelta), 0);

        // Write data to trigger SRI update
        _writeSinglePacket(stream, 25);
        Assert.assertTrue(stub.H.size() == 2);
        Assert.assertEquals(Double.compare(new_xdelta, stub.H.get(stub.H.size()-1).xdelta), 0);

        // Change blocking flag, then trigger an SRI update
        stream.blocking(false);
        Assert.assertTrue(stub.H.size() == 2);
        Assert.assertTrue(stub.H.get(stub.H.size()-1).blocking);
        _writeSinglePacket(stream, 25);
        Assert.assertTrue(stub.H.size() == 3);
        Assert.assertFalse(stub.H.get(stub.H.size()-1).blocking);

        // Change multiple fields, but only one SRI update should occur (after the
        // next write)
        stream.complex(true);
        stream.subsize(16);
        stream.xstart(-Math.PI);
        stream.xdelta(2.0 * Math.PI / 1024.0);
        stream.xunits(BULKIO.UNITS_FREQUENCY.value);
        stream.ydelta(1024.0 / 1.25e6);
        stream.yunits(BULKIO.UNITS_TIME.value);
        Assert.assertTrue(stub.H.size() == 3);

        // Trigger SRI update and verify that it matches
        _writeSinglePacket(stream, 1024);
        Assert.assertTrue(stub.H.size() == 4);
        bulkio.sri.DefaultComparator comparator = new bulkio.sri.DefaultComparator();

        Assert.assertTrue(comparator.compare(stream.sri(), stub.H.get(stub.H.size()-1)));
    }

    @Test
    public void testKeywords()
    {
        OutFileStream stream = port.createStream("test_keywords");
        _writeSinglePacket(stream, 1);
        Assert.assertTrue(stub.H.size() == 1);

        // Set/get keywords
        stream.setKeyword("integer", 250);
        stream.setKeyword("string", "value");
        stream.setKeyword("double", 101.1e6);
        stream.setKeyword("boolean", false);
        Assert.assertEquals(250, stream.getKeyword("integer").extract_long());
        Assert.assertEquals("value", stream.getKeyword("string").extract_string());
        Assert.assertEquals(Double.compare(101.1e6, stream.getKeyword("double").extract_double()), 0);
        Assert.assertTrue(!stream.getKeyword("boolean").extract_boolean());

        // Erase and check for presence of keywords
        stream.eraseKeyword("string");
        Assert.assertTrue(stream.hasKeyword("integer"));
        Assert.assertTrue(!stream.hasKeyword("string"));
        Assert.assertTrue(stream.hasKeyword("double"));
        Assert.assertTrue(stream.hasKeyword("boolean"));

        // Write a packet to trigger an SRI update
        Assert.assertTrue(stub.H.size() == 1);
        _writeSinglePacket(stream, 1);
        Assert.assertTrue(stub.H.size() == 2);
        {
            CF.DataType[] keywords = stub.H.get(stub.H.size()-1).keywords;
            Assert.assertEquals(stream.keywords().length, keywords.length);
            CF.DataType _integer = null;
            CF.DataType _double = null;
            CF.DataType _boolean = null;
            for (int ii=0; ii<keywords.length; ii++) {
                if (keywords[ii].id.equals("integer")) {
                    _integer = keywords[ii];
                } else if (keywords[ii].id.equals("double")) {
                    _double = keywords[ii];
                } else if (keywords[ii].id.equals("boolean")) {
                    _boolean = keywords[ii];
                }
            }
            Assert.assertNotNull(_integer);
            Assert.assertNotNull(_double);
            Assert.assertNotNull(_boolean);

            Assert.assertEquals(stream.getKeyword("integer").extract_long(), _integer.value.extract_long());
            Assert.assertEquals(Double.compare(stream.getKeyword("double").extract_double(), _double.value.extract_double()), 0);
            Assert.assertEquals(stream.getKeyword("boolean").extract_boolean(), _boolean.value.extract_boolean());
        }

        // Replace keywords with a new set
        CF.DataType[] new_keywords = new CF.DataType[2];
        new_keywords[0] = new CF.DataType();
        new_keywords[0].value = ORB.init().create_any();
        new_keywords[0].id = "COL_RF";
        new_keywords[0].value.insert_double(100.0e6);
        new_keywords[1] = new CF.DataType();
        new_keywords[1].value = ORB.init().create_any();
        new_keywords[1].id = "CHAN_RF";
        new_keywords[1].value.insert_double(101.1e6);
        stream.keywords(new_keywords);
        Assert.assertEquals(2, stream.keywords().length);
        Assert.assertEquals(Double.compare(100.0e6, stream.getKeyword("COL_RF").extract_double()), 0);
        Assert.assertEquals(Double.compare(101.1e6, stream.getKeyword("CHAN_RF").extract_double()), 0);

        // Trigger another SRI update
        Assert.assertTrue(stub.H.size() == 2);
        _writeSinglePacket(stream, 1);
        Assert.assertTrue(stub.H.size() == 3);
        {
            CF.DataType[] keywords = stub.H.get(stub.H.size()-1).keywords;
            Assert.assertEquals(stream.keywords().length, keywords.length);
            CF.DataType _COL_RF = null;
            CF.DataType _CHAN_RF = null;
            for (int ii=0; ii<keywords.length; ii++) {
                if (keywords[ii].id.equals("COL_RF")) {
                    _COL_RF = keywords[ii];
                } else if (keywords[ii].id.equals("CHAN_RF")) {
                    _CHAN_RF = keywords[ii];
                }
            }
            Assert.assertNotNull(_COL_RF);
            Assert.assertNotNull(_CHAN_RF);
            Assert.assertEquals(Double.compare(stream.getKeyword("COL_RF").extract_double(), _COL_RF.value.extract_double()), 0);
            Assert.assertEquals(Double.compare(stream.getKeyword("CHAN_RF").extract_double(), _CHAN_RF.value.extract_double()), 0);
        }
    }

    @Test
    public void testSendEosOnClose()
    {
        OutFileStream stream = port.createStream("close_eos");

        Assert.assertTrue(stub.H.size() == 0);
        Assert.assertTrue(stub.packets.size() == 0);

        _writeSinglePacket(stream, 16);

        Assert.assertTrue(stub.H.size() == 1);
        Assert.assertTrue(stub.packets.size() == 1);
        Assert.assertTrue(!stub.packets.get(stub.packets.size()-1).EOS);

        stream.close();
        Assert.assertTrue(stub.packets.size() == 2);
        Assert.assertTrue(stub.packets.get(stub.packets.size()-1).EOS);
    }
}
