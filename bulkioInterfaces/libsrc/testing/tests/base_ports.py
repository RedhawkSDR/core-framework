#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import random
import unittest
import sys
from ossie.utils import sb
import time
import os as _os
from bulkio.bulkioInterfaces.BULKIO import *

# remove when sandbox support for relative path works
test_dir='../'

# Add the local search paths to find local IDL files
from ossie.utils import model
model._idllib.addSearchPath('../../../idl')

def str_to_class(s):
    if s in globals() and isinstance(globals()[s], types.ClassType):
        return globals()[s]
    return None


class BaseVectorPort(unittest.TestCase):
    """Test against a component using bulkio library classes.

    The component should provide a pass through interface from a corresponding
    input/output port pair.  The names and types of the ports must match the
    the PORT_FLOW table.
    """
    KEYS = ['c_name', 'c_inport', 'c_outport', 'sink_inport']
    PORT_FLOW = {
               'Int8' : [ 'dataCharIn', 'dataCharOut', 'charIn' ],
               'UInt8' : [ 'dataOctetIn', 'dataOctetOut', 'charIn' ],
               'Int16' : [ 'dataShortIn', 'dataShortOut', 'shortIn' ],
               'UInt16' : [ 'dataUShortIn', 'dataUShortOut', 'shortIn' ],
               'Int32' : [ 'dataLongIn', 'dataLongOut', 'longIn' ],
               'UInt32' : [ 'dataULongIn', 'dataULongOut', 'longIn' ],
               'Int64' : [ 'dataLongLongIn', 'dataLongLongOut', 'longlongIn' ],
               'UInt64' : [ 'dataULongLongIn', 'dataULongLongOut', 'longlongIn' ],
               'Float' : [ 'dataFloatIn', 'dataFloatOut', 'floatIn' ],
               'Double' : [ 'dataDoubleIn', 'dataDoubleOut', 'doubleIn' ],
               'File' : [ 'dataFileIn', 'dataFileOut', 'fileIn' ],
               'Xml' : [ 'dataXMLIn', 'dataXMLOut', 'xmlIn' ]
               }

    def __init__(self, ptype, cname, *args, **kwargs):
        srcData = kwargs.pop('srcData', None)
        cmpData = kwargs.pop('cmpData', None)
        unittest.TestCase.__init__(self, *args, **kwargs)
        self.c_dir = 'components'
        self.c_name = cname
        self.ptype = ptype
        self.execparams = {}
        self.c_inport = None
        self.c_outport = None
        self.sink_inport = None
        self.srcData = srcData
        self.cmpData = cmpData
        self.ctx = dict().fromkeys(BaseVectorPort.KEYS)

    def getPortFlow(self, ptype='Int8' ):
        return BaseVectorPort.PORT_FLOW[ptype]

    def setContext(self, ctx=None):
        ##print "cname " + str(self.c_name) + " ptype= " + str(self.ptype)
        self.ctx[ BaseVectorPort.KEYS[0] ] = self.c_name
        self.ctx[ BaseVectorPort.KEYS[1] ] = BaseVectorPort.PORT_FLOW[self.ptype][0]
        self.ctx[ BaseVectorPort.KEYS[2] ] = BaseVectorPort.PORT_FLOW[self.ptype][1]
        self.ctx[ BaseVectorPort.KEYS[3] ] = BaseVectorPort.PORT_FLOW[self.ptype][2]
        tmp=self.ctx
        if ctx:
            tmp = ctx
        try:
            self.c_inport = tmp['c_inport']
            self.c_outport = tmp['c_outport']
            self.sink_inport = tmp['sink_inport']
        except:
            pass


    def setUp(self):
        self.setContext()
        if self.srcData:
            self.seq = self.srcData
        else:
            self.seq = range(100)
        self.launchedComps = []

    def tearDown(self):
        for comp in self.launchedComps:
            comp.releaseObject()
        self.launchedComps = []

    def launch(self, *args, **kwargs):
        comp = sb.launch(*args, **kwargs)
        self.launchedComps.append(comp)
        return comp

    def test_push_packet(self):
        """Test input/output port pair of the component."""
        in_sri = bulkio.sri.create()
        in_sri.streamID = "VECTOR-PUSHPACKET-SIMPLE"
        in_sri.mode = 0
        in_sri.xdelta  = 1/33.0
        source = sb.StreamSource(streamID=in_sri.streamID)
        source.sri = in_sri
        sink = sb.StreamSink()
        c_spd_xml = test_dir + self.c_dir + '/' + self.c_name + '/' + self.c_name + '.spd.xml'
        print "Test Component:" + c_spd_xml
        test_comp=self.launch( c_spd_xml, execparams=self.execparams)
        data_in = self.seq

        source.connect(test_comp, providesPortName=self.c_inport)
        test_comp.connect(sink, providesPortName=self.sink_inport, usesPortName=self.c_outport)
        sb.start()
        source.write(data_in)
        source.close()
        stream_data = sink.read(eos=True)
        data_out = stream_data.data
        #print 'Result data: ' + str(len(data_out))
        #print data_in
        #print data_out
        if isinstance(data_in, basestring):
            self.assertEqual(data_in, data_out[0], 'PUSH PACKET FAILED....Data Vector Mismatch')
        else:
            self.assertEqual(len(data_in), len(data_out), 'PUSH PACKET FAILED....Data Vector Mismatch')

        #
        # check sri values
        #
        sri = stream_data.sri
        print "StreamID   in:" + str(in_sri.streamID)+ " arrive:" + str(sri.streamID) 
        self.assertEqual(sri.streamID,in_sri.streamID,"PUSH PACKET FAILED....SRI StreamID Mismatch")

        print "Mode in:" + str(in_sri.mode)+ " arrive:" + str(sri.mode) 
        self.assertEqual(sri.mode,in_sri.mode,"PUSH PACKET FAILED....SRI Mode Mismatch")

        print "SampleRate in:" + str(in_sri.xdelta)+ " arrive:" + str(sri.xdelta) 
        self.assertAlmostEqual(sri.xdelta,in_sri.xdelta, 3, msg="PUSH PACKET FAILED....SRI SampleRate Mismatch")


    def test_push_packet_cplx(self):
        """Test input/output port pair of the component."""
        in_sri = bulkio.sri.create()
        in_sri.streamID = "VECTOR-PUSHPACKET-CPLX"
        in_sri.mode = 1
        in_sri.xdelta  = 1/33.0
        source = sb.StreamSource(streamID=in_sri.streamID)
        source.sri = in_sri
        sink = sb.StreamSink()
        c_spd_xml = test_dir + self.c_dir + '/' + self.c_name + '/' + self.c_name + '.spd.xml'
        print "Test Component:" + c_spd_xml
        test_comp=self.launch( c_spd_xml, execparams=self.execparams)
        data_in = self.seq

        source.connect(test_comp, providesPortName=self.c_inport)
        test_comp.connect(sink, providesPortName=self.sink_inport, usesPortName=self.c_outport)
        sb.start()
        source.write(data_in)
        source.close()
        stream_data = sink.read(eos=True)
        data_out = stream_data.data
        #print "Result data_in: " + str(len(data_out))
        #print data_in
        #print data_out
        if isinstance(data_in, basestring):
            self.assertEqual(data_in, data_out[0], 'PUSH PACKET FAILED....Data Vector Mismatch')
        else:
            self.assertEqual(len(data_in), len(data_out), 'PUSH PACKET FAILED....Data Vector Mismatch')

        #
        # check sri values
        #
        sri = stream_data.sri
        print "StreamID   in:" + str(in_sri.streamID)+ " arrive:" + str(sri.streamID) 
        self.assertEqual(sri.streamID,in_sri.streamID,"PUSH PACKET CPLX FAILED....SRI StreamID Mismatch")

        print "Mode in:" + str(in_sri.mode)+ " arrive:" + str(sri.mode) 
        self.assertEqual(sri.mode,in_sri.mode,"PUSH PACKET CPLX FAILED....SRI Mode Mismatch")

        print "SampleRate in:" + str(in_sri.xdelta)+ " arrive:" + str(sri.xdelta) 
        self.assertAlmostEqual(sri.xdelta,in_sri.xdelta, 3, msg="PUSH PACKET CPLX FAILED....SRI SampleRate Mismatch")


    def test_inport_using_component(self):
        """Test some API calls.

        These comments appear out of date.

        bulkio input port
            getMaxQueueDepth  (unused.  remove/replace?)
            setMaxQueueDepth  (unused.  remove/replace?)
            getPacket  (unused.  remove/replace?)
        ProvidesPortStatisticsProvider:
            _get_statistics
            _get_state
        updateSRI
            _get_activeSRIs
            pushSRI
        data<Type>
            pushPacket  (unused.  remove/replace?)
        """
        c_spd_xml = test_dir + self.c_dir + '/' + self.c_name + '/' + self.c_name + '.spd.xml'
        print "Test Component:" + c_spd_xml
        test_comp=self.launch( c_spd_xml, execparams=self.execparams)

        ##
        ## grab port from component... this is corba port
        ##
        iport = test_comp.getPort(self.c_inport)
        self.assertNotEqual(iport,None,"Cannot get Input Port")

        ps = iport._get_statistics()
        self.assertNotEqual(ps,None,"Cannot get Port Statistics")

        s = iport._get_state()
        self.assertNotEqual(s,None,"Cannot get Port State")
        self.assertEqual(s,IDLE,"Invalid Port State")

        streams = iport._get_activeSRIs()
        self.assertNotEqual(streams,None,"Cannot get Streams List")

        ts = bulkio.timestamp.now()
        sri = bulkio.sri.create()
        sri.streamID = "test_port_api"
        iport.pushSRI(sri)


    def test_outport_using_component(self):
        """Test some API calls.

        These comments appear out of date.

        bulkio output port
            none
        UsesPortStatisticsProvider
            _get_statistics
        CF::Port
            _get_connections
            connectPort  (unused.  remove/replace?)
            disconnectPort  (unused.  remove/replace?)
        data<Type>
            pushPacket  (unused.  remove/replace?)
        """
        c_spd_xml = test_dir + self.c_dir + '/' + self.c_name + '/' + self.c_name + '.spd.xml'
        print "Test Component:" + c_spd_xml
        test_comp=self.launch( c_spd_xml, execparams=self.execparams)

        ##
        ## grab port from component... this is a corba port
        ##
        oport = test_comp.getPort(self.c_outport)
        self.assertNotEqual(oport,None,"Cannot get Output Port")

        ps = oport._get_statistics()
        self.assertNotEqual(ps,None,"Cannot get Port Statistics")

        sink = sb.StreamSink()
        test_comp.connect(sink, providesPortName=self.sink_inport, usesPortName=self.c_outport)

        cl = oport._get_connections()
        self.assertNotEqual(cl,None,"Cannot get Connections List")
        self.assertEqual(len(cl),1,"Incorrect Connections List Length")

        test_comp.disconnect(sink)

        cl = oport._get_connections()
        self.assertNotEqual(cl,None,"Cannot get Connections List")
        self.assertEqual(len(cl),0,"Incorrect Connections List Length")
