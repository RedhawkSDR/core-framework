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
from ossie.utils.idllib import IDLLibrary
model._idllib = IDLLibrary()
model._idllib.addSearchPath('../../../idl')
model._idllib.addSearchPath('/usr/local/redhawk/core/share/idl')
if 'OSSIEHOME' in _os.environ:
    model._idllib.addSearchPath(_os.path.join(_os.environ['OSSIEHOME'], 'share/idl'))

def str_to_class(s):
    if s in globals() and isinstance(globals()[s], types.ClassType):
        return globals()[s]
    return None
#
#
# BaseVectorPort
#
#  Runs 3 tests against a component using bulkio library classes
#  
# The component should provide a pass through interface from a corresponding 
# input/output port pair.  The names and types of the ports must match the 
# the table below... ie. int16 port test, input port name: dataShortIn output port name: dataShortOut
#
# test_push_packet:
#     test input/output port pair of the component, push 100 sample vector
#
# test_inport_api:
#     bulkio input port::
#            getMaxQueueDepth
#            setMaxQueueDepth
#            getPacket
#
#     ProvidesPortStatisticsProvider::
#            _get_statistics
#            _get_state
#     updateSRI
#            _get_activeSRIs
#            pushSRI
#
#      
#     data<Type>:
#         pushPacket
#
# test_outport_api:
#     bulkio output port::
#
#     UsesPortStatisticsProvider::
#            _get_statistics
#
#     CF::Port
#            _get_connections
#            connectPort
#            disconnectPort
#     data<Type>:
#         pushPacket


class BaseVectorPort(unittest.TestCase):
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

    def __init__(
            self,
            methodName='runTest',
            ptype='Int8',
            cname=None,
            srcData=None,
            cmpData=None,
            bio_in_module=bulkio.InCharPort,
            bio_out_module=bulkio.OutCharPort ):
        unittest.TestCase.__init__(self, methodName)
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
        self.bio_in_module = bio_in_module
        self.bio_out_module = bio_out_module

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
        in_sri = bulkio.sri.create()
        in_sri.streamID = "VECTOR-PUSHPACKET-SIMPLE"
        in_sri.mode = 0
        in_sri.xdelta  = 1/33.0
        dsource=sb.DataSource()
        dsink=sb.DataSink()
        c_spd_xml = test_dir + self.c_dir + '/' + self.c_name + '/' + self.c_name + '.spd.xml'
        print "Test Component:" + c_spd_xml
        test_comp=self.launch( c_spd_xml, execparams=self.execparams)
        data=self.seq

        dsource.connect(test_comp, providesPortName=self.c_inport )
        test_comp.connect(dsink, providesPortName=self.sink_inport, usesPortName=self.c_outport)
        sb.start()
        dsource.push( data, EOS=True, streamID=in_sri.streamID, sampleRate=33.0, complexData=(in_sri.mode==1))
        adata=dsink.getData(eos_block=True)
        #print "Result data: " + str(len(adata))
        #print data
        #print adata
        self.assertEqual(len(data),len(adata),"PUSH PACKET FAILED....Data Vector Mismatch")

        #
        # check sri values
        #
        sri = dsink.sri()
        print "StreamID   in:" + str(in_sri.streamID)+ " arrive:" + str(sri.streamID) 
        self.assertEqual(sri.streamID,in_sri.streamID,"PUSH PACKET FAILED....SRI StreamID Mismatch")

        print "Mode in:" + str(in_sri.mode)+ " arrive:" + str(sri.mode) 
        self.assertEqual(sri.mode,in_sri.mode,"PUSH PACKET FAILED....SRI Mode Mismatch")

        print "SampleRate in:" + str(in_sri.xdelta)+ " arrive:" + str(sri.xdelta) 
        self.assertAlmostEqual(sri.xdelta,in_sri.xdelta, 3, msg="PUSH PACKET FAILED....SRI SampleRate Mismatch")


    def test_push_packet_cplx(self):
        in_sri = bulkio.sri.create()
        in_sri.streamID = "VECTOR-PUSHPACKET-CPLX"
        in_sri.mode = 1
        in_sri.xdelta  = 1/33.0
        dsource=sb.DataSource()
        dsink=sb.DataSink()
        c_spd_xml = test_dir + self.c_dir + '/' + self.c_name + '/' + self.c_name + '.spd.xml'
        print "Test Component:" + c_spd_xml
        test_comp=self.launch( c_spd_xml, execparams=self.execparams)
        data=self.seq

        dsource.connect(test_comp, providesPortName=self.c_inport )
        test_comp.connect(dsink, providesPortName=self.sink_inport, usesPortName=self.c_outport)
        sb.start()
        dsource.push( data, EOS=True, streamID=in_sri.streamID, sampleRate=33.0, complexData=(in_sri.mode==1) )
        adata=dsink.getData(eos_block=True)
        #print "Result data: " + str(len(adata))
        #print data
        #print adata
        self.assertEqual(len(data),len(adata),"PUSH PACKET CPLX FAILED....Data Vector Mismatch")

        #
        # check sri values
        #
        sri = dsink.sri()
        print "StreamID   in:" + str(in_sri.streamID)+ " arrive:" + str(sri.streamID) 
        self.assertEqual(sri.streamID,in_sri.streamID,"PUSH PACKET CPLX FAILED....SRI StreamID Mismatch")

        print "Mode in:" + str(in_sri.mode)+ " arrive:" + str(sri.mode) 
        self.assertEqual(sri.mode,in_sri.mode,"PUSH PACKET CPLX FAILED....SRI Mode Mismatch")

        print "SampleRate in:" + str(in_sri.xdelta)+ " arrive:" + str(sri.xdelta) 
        self.assertAlmostEqual(sri.xdelta,in_sri.xdelta, 3, msg="PUSH PACKET CPLX FAILED....SRI SampleRate Mismatch")


    def test_inport_using_componet(self):
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


    def test_inport_python_api(self):
        ##
        ## test bulkio base class standalone
        ##
        bio = self.bio_in_module("xxx")

        ps = bio._get_statistics()
        self.assertNotEqual(ps,None,"Cannot get Port Statistics")

        s = bio._get_state()
        self.assertNotEqual(s,None,"Cannot get Port State")
        self.assertEqual(s,IDLE,"Invalid Port State")

        streams = bio._get_activeSRIs()
        self.assertNotEqual(streams,None,"Cannot get Streams List")

        qed = bio.getMaxQueueDepth()
        self.assertEqual(qed,100,"Get Stream Depth Failed")

        bio.setMaxQueueDepth(22)
        qed = bio.getMaxQueueDepth()
        self.assertEqual(qed,22,"Set/Get Stream Depth Failed")

        ts = bulkio.timestamp.now()
        sri = bulkio.sri.create()
        sri.streamID = "test_port_api"
        bio.pushSRI(sri)

        data=range(50)
        bio.pushPacket(data, ts, False, "test_port_api")

        # result of getPacket
        #    DATA_BUFFER=0
        #    TIME_STAMP=1
        #    END_OF_STREAM=2
        #    STREAM_ID=3
        #    SRI=4
        #    SRI_CHG=5
        #    QUEUE_FLUSH=6
        ##      this is missing in python
        ##pkt = bio.getPacket(bulkio.const.NON_BLOCKING)
        pkt = bio.getPacket()
        self.assertNotEqual(pkt,None,"pushPacket .. getPacket Failed")
        self.assertNotEqual(pkt[bulkio.InPort.DATA_BUFFER],None,"pushPacket .. getPacket Failed")
        self.assertNotEqual(pkt[bulkio.InPort.TIME_STAMP],None,"pushPacket .. getPacket Failed")
        self.assertNotEqual(pkt[bulkio.InPort.END_OF_STREAM],None,"pushPacket .. getPacket Failed")
        self.assertNotEqual(pkt[bulkio.InPort.STREAM_ID],None,"pushPacket .. getPacket Failed")
        self.assertNotEqual(pkt[bulkio.InPort.SRI],None,"pushPacket .. getPacket Failed")
        self.assertNotEqual(pkt[bulkio.InPort.SRI_CHG],None,"pushPacket .. getPacket Failed")
        self.assertNotEqual(pkt[bulkio.InPort.QUEUE_FLUSH],None,"pushPacket .. getPacket Failed")

        pkt = bio.getPacket()
        self.assertNotEqual(pkt,None,"Second getPacket should be Empty")
        self.assertEqual(pkt[bulkio.InPort.DATA_BUFFER],None,"pushPacket .. getPacket Failed")
        self.assertEqual(pkt[bulkio.InPort.TIME_STAMP],None,"pushPacket .. getPacket Failed")
        self.assertEqual(pkt[bulkio.InPort.END_OF_STREAM],None,"pushPacket .. getPacket Failed")
        self.assertEqual(pkt[bulkio.InPort.STREAM_ID],None,"pushPacket .. getPacket Failed")
        self.assertEqual(pkt[bulkio.InPort.SRI],None,"pushPacket .. getPacket Failed")
        self.assertEqual(pkt[bulkio.InPort.SRI_CHG],None,"pushPacket .. getPacket Failed")
        self.assertEqual(pkt[bulkio.InPort.QUEUE_FLUSH],None,"pushPacket .. getPacket Failed")

        sri.streamID = "test_port_api"
        sri.mode = 1
        bio.pushSRI(sri)
        data=range(50)
        bio.pushPacket(data, ts, True, "test_port_api")
        pkt = bio.getPacket()
        self.assertNotEqual(pkt,None,"pushPacket... getPacket FAILED")
        self.assertNotEqual(pkt[bulkio.InPort.DATA_BUFFER],None,"EOS: pushPacket .. getPacket Failed")
        self.assertEqual(pkt[bulkio.InPort.END_OF_STREAM],True,"EOS: pushPacket .. getPacket EOS TEST Failed")
        self.assertEqual(pkt[bulkio.InPort.SRI].mode,1,"EOS: pushPacket .. getPacket COMPLEX MODE Failed")

        pkt = bio.getPacket()
        self.assertEqual(pkt[bulkio.InPort.DATA_BUFFER],None,"pushPacket .. getPacket EOS Failed")
        self.assertEqual(pkt[bulkio.InPort.TIME_STAMP],None,"pushPacket .. getPacket EOS Failed")
        self.assertEqual(pkt[bulkio.InPort.END_OF_STREAM],None,"pushPacket .. getPacket EOS Failed")
        self.assertEqual(pkt[bulkio.InPort.STREAM_ID],None,"pushPacket .. getPacket EOS Failed")
        self.assertEqual(pkt[bulkio.InPort.SRI],None,"pushPacket .. getPacket EOS Failed")
        self.assertEqual(pkt[bulkio.InPort.SRI_CHG],None,"pushPacket .. getPacket EOS Failed")
        self.assertEqual(pkt[bulkio.InPort.QUEUE_FLUSH],None,"pushPacket .. getPacket EOS Failed")

    def test_outport_using_component(self):
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

        dsink=sb.DataSink()
        test_comp.connect(dsink, providesPortName=self.sink_inport, usesPortName=self.c_outport)

        cl = oport._get_connections()
        self.assertNotEqual(cl,None,"Cannot get Connections List")
        self.assertEqual(len(cl),1,"Incorrect Connections List Length")

        test_comp.disconnect(dsink)

        cl = oport._get_connections()
        self.assertNotEqual(cl,None,"Cannot get Connections List")
        self.assertEqual(len(cl),0,"Incorrect Connections List Length")

        ##
        ## Create bulkio base class port object
        ##
        bio = self.bio_out_module("xxx")
        cl = bio._get_connections()
        self.assertNotEqual(cl,None,"Cannot get Connections List")
        self.assertEqual(len(cl),0,"Incorrect Connections List Length")

        ts = bulkio.timestamp.now()
        sri = bulkio.sri.create()
        sri.streamID = "test_port_api"
        bio.pushSRI(sri)

        data=range(50)
        bio.pushPacket(data, ts, False, "test_port_api")
        bio.pushPacket(data, ts, True,  "test_port_api")
        bio.pushPacket(data, ts, False, "unknown_port_api")

        ps = bio._get_statistics()
        self.assertNotEqual(ps,None,"Cannot get Port Statistics")

        cnt = len(bio.sriDict)
        self.assertEqual(cnt,1,"SRI list should be 1")

        bio.enableStats(False)



    def test_outport_python_api(self):
        ##
        ## Create bulkio base class port object
        ##
        bio = self.bio_out_module("xxx")
        cl = bio._get_connections()
        self.assertNotEqual(cl,None,"Cannot get Connections List")
        self.assertEqual(len(cl),0,"Incorrect Connections List Length")

        connectionName="testing-connection-list" 
        dsink=sb.DataSink()
        inport=dsink.getPort(self.sink_inport)
        bio.connectPort(inport, connectionName )
        
        
        cl = bio._get_connections()
        self.assertNotEqual(cl,None,"Cannot get Connections List")
        self.assertEqual(len(cl),1,"Incorrect Connections List Length")

        bio.disconnectPort(connectionName)
        bio.disconnectPort(connectionName)

        cl = bio._get_connections()
        self.assertNotEqual(cl,None,"Cannot get Connections List")
        self.assertEqual(len(cl),0,"Incorrect Connections List Length")

        ts = bulkio.timestamp.now()
        sri = bulkio.sri.create()
        sri.streamID = "test_port_api"
        bio.pushSRI(sri)

        data=range(50)
        bio.pushPacket(data, ts, False, "test_port_api")
        bio.pushPacket(data, ts, True,  "test_port_api")
        bio.pushPacket(data, ts, False, "unknown_port_api")

        ps = bio._get_statistics()
        self.assertNotEqual(ps,None,"Cannot get Port Statistics")

        cnt = len(bio.sriDict)
        self.assertEqual(cnt,1,"SRI list should be 1")

        bio.enableStats(False)

        # repeating connect/disconnect to test ticket #1996
        connectionName="testing-connection-list" 
        dsink=sb.DataSink()
        inport=dsink.getPort(self.sink_inport)
        bio.connectPort(inport, connectionName )

        cl = bio._get_connections()
        self.assertNotEqual(cl,None,"Cannot get Connections List")
        self.assertEqual(len(cl),1,"Incorrect Connections List Length")

        bio.disconnectPort(connectionName)
        bio.disconnectPort(connectionName)

