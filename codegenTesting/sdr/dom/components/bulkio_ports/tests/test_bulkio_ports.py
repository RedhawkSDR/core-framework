#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK codegenTesting.
#
# REDHAWK codegenTesting is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK codegenTesting is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import ossie.utils.testing
from omniORB import any, CORBA

from ossie.utils.bulkio import bulkio_helpers
from bulkio.bulkioInterfaces import BULKIO__POA
from bulkio.bulkioInterfaces import BULKIO

class ComponentTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all component implementations in bulkio_ports"""

    def testScaBasicBehavior(self):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        #######################################################################
        # Verify the basic state of the component
        self.assertNotEqual(self.comp_obj, None)
        self.assertEqual(self.comp_obj._non_existent(), False)
        self.assertEqual(self.comp_obj._is_a("IDL:CF/Resource:1.0"), True)
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp_obj.configure(configureProps)
        
        #######################################################################
        # Validate that query returns all expected parameters
        # Query of '[]' should return the following set of properties
        expectedProps = []
        expectedProps.extend(self.getPropertySet(kinds=("configure", "execparam"), modes=("readwrite", "readonly"), includeNil=True))
        expectedProps.extend(self.getPropertySet(kinds=("allocate",), action="external", includeNil=True))
        props = self.comp_obj.query([])
        props = dict((x.id, any.from_any(x.value)) for x in props)
        # Query may return more than expected, but not less
        for expectedProp in expectedProps:
            self.assertEquals(props.has_key(expectedProp.id), True)
        
        #######################################################################
        # Verify that all expected ports are available
        for port in self.scd.get_componentfeatures().get_ports().get_uses():
            port_obj = self.comp_obj.getPort(str(port.get_usesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a("IDL:CF/Port:1.0"),  True)
            
        for port in self.scd.get_componentfeatures().get_ports().get_provides():
            port_obj = self.comp_obj.getPort(str(port.get_providesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a(port.get_repid()),  True)
            
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp_obj.start()
        self.comp_obj.stop()
        
        #######################################################################
        # Simulate regular component shutdown
        self.comp_obj.releaseObject()
        
        
class FunctionalTests(ossie.utils.testing.ScaComponentTestCase):    
    
    def setUp(self):
        ossie.utils.testing.ScaComponentTestCase.setUp(self)
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        #######################################################################
        # Verify the basic state of the component
        self.assertNotEqual(self.comp_obj, None)
        self.assertEqual(self.comp_obj._non_existent(), False)
        self.assertEqual(self.comp_obj._is_a("IDL:CF/Resource:1.0"), True)
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp_obj.configure(configureProps)
        
        #########################
        # Create the test inputs 
        self.helperCharInput = bulkio_helpers.createBULKIOInputPort(BULKIO__POA.dataChar)
        self.helperDoubleInput = bulkio_helpers.createBULKIOInputPort(BULKIO__POA.dataDouble)
        self.helperFileInput = bulkio_helpers.createBULKIOInputPort(BULKIO__POA.dataFile)
        self.helperFloatInput = bulkio_helpers.createBULKIOInputPort(BULKIO__POA.dataFloat)
        self.helperLongInput = bulkio_helpers.createBULKIOInputPort(BULKIO__POA.dataLong)
        self.helperOctetInput = bulkio_helpers.createBULKIOInputPort(BULKIO__POA.dataOctet)
        self.helperShortInput = bulkio_helpers.createBULKIOInputPort(BULKIO__POA.dataShort)
        self.helperUlongInput = bulkio_helpers.createBULKIOInputPort(BULKIO__POA.dataUlong)
        self.helperUshortInput = bulkio_helpers.createBULKIOInputPort(BULKIO__POA.dataUshort)
        self.helperXMLInput = bulkio_helpers.createBULKIOInputPort(BULKIO__POA.dataXML)
        self.helperLongLongInput = bulkio_helpers.createBULKIOInputPort(BULKIO__POA.dataLongLong)
        self.helperUlongLongInput = bulkio_helpers.createBULKIOInputPort(BULKIO__POA.dataUlongLong)
       
        #########################################
        # Get the input ports from the component
        self.dataCharInput = self.comp_obj.getPort('dataCharIn')
        self.dataDoubleInput = self.comp_obj.getPort('dataDoubleIn')
        self.dataFileInput = self.comp_obj.getPort('dataFileIn')
        self.dataFloatInput = self.comp_obj.getPort('dataFloatIn')
        self.dataLongInput = self.comp_obj.getPort('dataLongIn')
        self.dataOctetInput = self.comp_obj.getPort('dataOctetIn')
        self.dataShortInput = self.comp_obj.getPort('dataShortIn')
        self.dataUlongInput = self.comp_obj.getPort('dataUlongIn')
        self.dataUshortInput = self.comp_obj.getPort('dataUshortIn')
        self.dataXMLInput = self.comp_obj.getPort('dataXMLIn')
        self.dataLongLongInput = self.comp_obj.getPort('dataLongLongIn')
        self.dataUlongLongInput = self.comp_obj.getPort('dataUlongLongIn')
            
        ##########################################
        # Get the output ports from the component
        self.dataCharOutput = self.comp_obj.getPort('dataCharOut')
        self.dataDoubleOutput = self.comp_obj.getPort('dataDoubleOut')
        self.dataFileOutput = self.comp_obj.getPort('dataFileOut')
        self.dataFloatOutput = self.comp_obj.getPort('dataFloatOut')
        self.dataLongOutput = self.comp_obj.getPort('dataLongOut')
        self.dataOctetOutput = self.comp_obj.getPort('dataOctetOut')
        self.dataShortOutput = self.comp_obj.getPort('dataShortOut')
        self.dataUlongOutput = self.comp_obj.getPort('dataUlongOut')
        self.dataUshortOutput = self.comp_obj.getPort('dataUshortOut')
        self.dataXMLOutput = self.comp_obj.getPort('dataXMLOut')
        self.dataLongLongOutput = self.comp_obj.getPort('dataLongLongOut')
        self.dataUlongLongOutput = self.comp_obj.getPort('dataUlongLongOut')
        
        ################################
        # make all the port connections
        self.dataCharOutput.connectPort(self.helperCharInput, 'dataCharOut')
        self.dataDoubleOutput.connectPort(self.helperDoubleInput, 'dataDoubleOut')
        self.dataFileOutput.connectPort(self.helperFileInput, 'dataFileOut')
        self.dataFloatOutput.connectPort(self.helperFloatInput, 'dataFloatOut')
        self.dataLongOutput.connectPort(self.helperLongInput, 'dataLongOut')
        self.dataOctetOutput.connectPort(self.helperOctetInput, 'dataOctetOut')
        self.dataShortOutput.connectPort(self.helperShortInput, 'dataShortOut')
        self.dataUlongOutput.connectPort(self.helperUlongInput, 'dataUlongOut')
        self.dataUshortOutput.connectPort(self.helperUshortInput, 'dataUshortOut')
        self.dataXMLOutput.connectPort(self.helperXMLInput, 'dataXMLOut')
        self.dataLongLongOutput.connectPort(self.helperLongLongInput, 'dataLongLongOut')
        self.dataUlongLongOutput.connectPort(self.helperUlongLongInput, 'dataUlongLongOut')

    def tearDown(self):
        self.dataCharOutput.disconnectPort('dataCharOut')
        self.dataDoubleOutput.disconnectPort('dataDoubleOut')
        self.dataFileOutput.disconnectPort('dataFileOut')
        self.dataFloatOutput.disconnectPort('dataFloatOut')
        self.dataLongOutput.disconnectPort('dataLongOut')
        self.dataOctetOutput.disconnectPort('dataOctetOut')
        self.dataShortOutput.disconnectPort('dataShortOut')
        self.dataUlongOutput.disconnectPort('dataUlongOut')
        self.dataUshortOutput.disconnectPort('dataUshortOut')
        self.dataXMLOutput.disconnectPort('dataXMLOut')
        self.dataLongLongOutput.disconnectPort('dataLongLongOut')
        self.dataUlongLongOutput.disconnectPort('dataUlongLongOut')
        
        self.comp_obj.releaseObject()
        
    #Test to push data into each of the input ports, and get data
    #from each of the output ports.  The data is then examined for accuracy
    def testDataPush(self):
        charData = bulkio_helpers.genRandomDataSet(8, True, 1000)
        doubleData = bulkio_helpers.genRandomDataSet(64, True, 1000)
        floatData = bulkio_helpers.genRandomDataSet(32, True, 1000)
        longData = bulkio_helpers.genRandomDataSet(32, True, 1000)
        octetData = bulkio_helpers.genRandomDataSet(8, False, 1000)
        shortData = bulkio_helpers.genRandomDataSet(16, True, 1000)
        uLongData = bulkio_helpers.genRandomDataSet(32, False, 1000)
        uLongLongData = bulkio_helpers.genRandomDataSet(32, False, 1000)
        longLongData = bulkio_helpers.genRandomDataSet(64, True, 1000)
        uShortData = bulkio_helpers.genRandomDataSet(16, False, 1000)
        xmlData = '<?xml version="1.0" encoding="ISO-8859-1"?>'
        fileData = 'SCA:/data/redhawk/myfile'

        #Format the data sets to be sure they can be used in the pushPacket call
        formCharData = bulkio_helpers.formatData(charData,self.dataCharInput)
        formDoubleData = bulkio_helpers.formatData(doubleData,self.dataDoubleInput)
        formFloatData = bulkio_helpers.formatData(floatData,self.dataFloatInput)
        formLongData = bulkio_helpers.formatData(longData,self.dataLongInput)
        formOctetData = bulkio_helpers.formatData(octetData,self.dataOctetInput)
        formShortData = bulkio_helpers.formatData(shortData,self.dataShortInput)
        formULongData = bulkio_helpers.formatData(uLongData,self.dataUlongInput)
        formULongLongData = bulkio_helpers.formatData(uLongLongData,self.dataUlongLongInput)
        formLongLongData = bulkio_helpers.formatData(longLongData,self.dataLongLongInput)
        formUShortData = bulkio_helpers.formatData(uShortData,self.dataUshortInput)
        formXmlData = bulkio_helpers.formatData(xmlData,self.dataXMLInput)
        formFileData = bulkio_helpers.formatData(fileData,self.dataFileInput)

        self.comp_obj.start()

        num_test_runs = 10
        for i in range(num_test_runs):
            ################################
            # Send Data on each output port
            self.dataCharInput.pushPacket(formCharData, bulkio_helpers.createCPUTimestamp(), False, "s1")
            self.dataDoubleInput.pushPacket(formDoubleData, bulkio_helpers.createCPUTimestamp(), False, "s2")
            self.dataFloatInput.pushPacket(formFloatData, bulkio_helpers.createCPUTimestamp(), False, "s3")
            self.dataLongInput.pushPacket(formLongData, bulkio_helpers.createCPUTimestamp(), False, "s4")
            self.dataOctetInput.pushPacket(formOctetData, bulkio_helpers.createCPUTimestamp(), False, "s5")
            self.dataShortInput.pushPacket(formShortData, bulkio_helpers.createCPUTimestamp(), False, "s6")
            self.dataUlongInput.pushPacket(formULongData, bulkio_helpers.createCPUTimestamp(), False, "s7")
            self.dataUlongLongInput.pushPacket(formULongLongData, bulkio_helpers.createCPUTimestamp(), False, "s8")
            self.dataLongLongInput.pushPacket(formLongLongData, bulkio_helpers.createCPUTimestamp(), False, "s9")
            self.dataUshortInput.pushPacket(formUShortData, bulkio_helpers.createCPUTimestamp(), False, "s10")
            self.dataXMLInput.pushPacket(formXmlData, False, "s11")
            self.dataFileInput.pushPacket(formFileData, bulkio_helpers.createCPUTimestamp(), False, "s12")

            ##########################################################
            # Receive data from each output port
            # *** THESE GET PACKETS CAN BE SET TO BLOCK OR TIMEOUT ***
            _charData, T, EOS, streamID, sri, sriChanged, flushed = self.helperCharInput.getPacket(2)
            _doubleData, T, EOS, streamID, sri, sriChanged, flushed = self.helperDoubleInput.getPacket(2)
            _floatData, T, EOS, streamID, sri, sriChanged, flushed = self.helperFloatInput.getPacket(2)
            _longData, T, EOS, streamID, sri, sriChanged, flushed = self.helperLongInput.getPacket(2)
            _octetData, T, EOS, streamID, sri, sriChanged, flushed = self.helperOctetInput.getPacket(2)
            _shortData, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(2)
            _uLongData, T, EOS, streamID, sri, sriChanged, flushed = self.helperUlongInput.getPacket(2)
            _uLongLongData, T, EOS, streamID, sri, sriChanged, flushed = self.helperUlongLongInput.getPacket(2)
            _longLongData, T, EOS, streamID, sri, sriChanged, flushed = self.helperLongLongInput.getPacket(2)
            _uShortData, T, EOS, streamID, sri, sriChanged, flushed = self.helperUshortInput.getPacket(2)
            _xmlData, T, EOS, streamID, sri, sriChanged, flushed = self.helperXMLInput.getPacket(2)
            _fileData, T, EOS, streamID, sri, sriChanged, flushed = self.helperFileInput.getPacket(2)
            
            types = ['char', 'double', 'float', 'long', 'octet', 'short', 'uLong', 'uLongLong', 'longLong', 'uShort', 'xml', 'file']
            
            sentData = {'char':formCharData, 'double':formDoubleData, 'float':formFloatData, 'long':formLongData, 'octet':formOctetData,\
                        'short':formShortData, 'uLong':formULongData, 'uShort':formUShortData, 'xml':formXmlData, 'file':formFileData,\
                        'longLong':formLongLongData, 'uLongLong':formULongLongData}
            recData = {'char':_charData, 'double':_doubleData, 'float':_floatData, 'long':_longData, 'octet':_octetData, 'short':_shortData,\
                       'uLong':_uLongData, 'uShort':_uShortData, 'xml':_xmlData, 'file':_fileData, 'longLong':_longLongData, 'uLongLong':_uLongLongData}
            
            for x in types:
                self.assertNotEquals(None, recData[x], msg="No data was recieved for dataType (" + str(x) + ")")
            
            #############################################
            # Check that data received matches data sent
            for x in types:
                self.assertEqual(recData[x], sentData[x])
                
            #########################################
            # Check restoring the 8 bit data to its 
            # original yields the same results
            restCharData = bulkio_helpers.restoreData(_charData, '8bit')
            restOctetData = bulkio_helpers.restoreData(_octetData, 'u8bit')
            self.assertEquals(restCharData, charData)
            self.assertEquals(restOctetData, octetData)
        
        self.comp_obj.stop()
                
    def testMaxQueueDepth(self):
        shortData = bulkio_helpers.genRandomDataSet(16, True, 100)
        oldFlushTime = None
        
        #try changing it to 10
        newmqd = 10
        self.comp_obj.configure([ossie.cf.CF.DataType(id='mqd', value=CORBA.Any(CORBA.TC_short, newmqd))])
        currmqd = self.comp_obj.query([ossie.cf.CF.DataType(id=str('mqd'),value=any.to_any(None))])[0].value.value()
        self.assertEquals(currmqd, newmqd)
        
        #push the max + 1 packets, should see a flush on the last packet but not on any other
        #the reason the packets will be queuing up is because the component has not been started up.   
        for x in range(newmqd+1):
            stats = self.dataShortInput._get_statistics()
            for x in stats.keywords:
                if x.id == 'timeSinceLastFlush':
                    self.fail("Flush happened before it should have")
            self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, 'stream1')
        stats = self.dataShortInput._get_statistics()
        if 'timeSinceLastFlush' not in [y.id for y in stats.keywords]:
            self.fail("Should have flushed at the max+1 pushPacket")
        else:
            for x in stats.keywords:
                if x.id == 'timeSinceLastFlush':
                    oldFlushTime = x.value.value()
                    break
        
        #try setting it again
        newmqd = 250
        self.comp_obj.configure([ossie.cf.CF.DataType(id='mqd', value=CORBA.Any(CORBA.TC_short, newmqd))])
        currmqd = self.comp_obj.query([ossie.cf.CF.DataType(id=str('mqd'),value=any.to_any(None))])[0].value.value()
        self.assertEquals(int(currmqd), newmqd)
        
        #push the max packets (THERE IS ALREADY ONE PACKET IN THE QUEUE
        #should see a flush on the last packet but not on any other
        newFlushTime = None 
        for x in range(newmqd):
            stats = self.dataShortInput._get_statistics()
            for y in stats.keywords:
                if y.id == 'timeSinceLastFlush':
                    newFlushTime = y.value.value()
                    self.assertTrue(oldFlushTime <= newFlushTime)
                    oldFlushTime = newFlushTime
                    break
            self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, 'stream1')
        stats = self.dataShortInput._get_statistics()
        for x in stats.keywords:
            if x.id == 'timeSinceLastFlush':
                newFlushTime = x.value.value()
                break
        self.assertTrue(newFlushTime <= oldFlushTime)
        oldFlushTime = newFlushTime
        
        #try making it infinite
        newmqd = -1
        self.comp_obj.configure([ossie.cf.CF.DataType(id='mqd', value=CORBA.Any(CORBA.TC_short, newmqd))])
        currmqd = self.comp_obj.query([ossie.cf.CF.DataType(id=str('mqd'),value=any.to_any(None))])[0].value.value()
        self.assertEquals(int(currmqd), newmqd)
        
        #push 1000 packets, should not see a flush at all
        newFlushTime = None 
        for x in range(1000):
            stats = self.dataShortInput._get_statistics()
            for y in stats.keywords:
                if y.id == 'timeSinceLastFlush':
                    newFlushTime = y.value.value()
                    self.assertTrue(oldFlushTime <= newFlushTime)
                    oldFlushTime = newFlushTime
                    break
            self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, 'stream1')
        stats = self.dataShortInput._get_statistics()
        
        #set it to blocking, make sure it doesn't break
        newmqd = 0
        self.comp_obj.configure([ossie.cf.CF.DataType(id='mqd', value=CORBA.Any(CORBA.TC_short, newmqd))])
        currmqd = self.comp_obj.query([ossie.cf.CF.DataType(id=str('mqd'),value=any.to_any(None))])[0].value.value()
        self.assertEquals(int(currmqd), newmqd)
        
        #push 100 packets, the pushPacket returns immediately, the queueShould not grow
        firstaqd = self.dataShortInput._get_statistics().averageQueueDepth
        nextaqd = None
        newFlushTime = None 
        for x in range(5):
            self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, 'stream1')
            nextaqd = self.dataShortInput._get_statistics().averageQueueDepth
            self.assertEquals(firstaqd, nextaqd)

        
#    def testSDDSPorts(self):
#        ##################################################
#        # additional parameter sent in is an instance
#        # of the test class so that the attach and detach
#        # class can change the class variable created
#        # below (callExecution)
#        self.dataSDDSRec1 = bulkio_helpers.DataReceiver(BULKIO__POA.dataSDDS, atomic=True, testclass=self)
#        self.dataSDDSRec2 = bulkio_helpers.DataReceiver(BULKIO__POA.dataSDDS, atomic=True, testclass=self)
#        self.dataSDDSRec3 = bulkio_helpers.DataReceiver(BULKIO__POA.dataSDDS, atomic=True, testclass=self)
#        
#        self.dataSDDSInput = self.comp_obj.getPort('dataSDDSIn')
#        self.dataSDDSOutput = self.comp_obj.getPort('dataSDDSOut')
#        
#        self.dataSDDSOutput.connectPort(self.dataSDDSRec1.getPort(), 'dataSDDSOut1')
#        self.dataSDDSOutput.connectPort(self.dataSDDSRec2.getPort(), 'dataSDDSOut2')
#        self.dataSDDSOutput.connectPort(self.dataSDDSRec3.getPort(), 'dataSDDSOut3')
#        
#        ###############################################
#        # used for verifying behavior of the attach
#        # and detach calls.  Each attach should
#        # trigger a detach of all existing attachments.
#        # The attach and detach calls within the
#        # bulkio_helpers module will change this
#        # variable to indicate the current state
#        self.callExecution = ''
#        
#        self.comp_obj.start()
#        ############################
#        # Try the attach and detach 
#        # calls for the one port
#        _maddress = 'attachment'
#
#        sd = BULKIO.SDDSStreamDefinition(id='streamID', dataFormat=BULKIO.SDDS_SI, multicastAddress=_maddress, vlan=0, 
#                                         port=29495, sampleRate=10000, timeTagValid=True, privateInfo='none')
#        
#        recId = self.dataSDDSInput.attach(sd, 'blank')
#        self.dataSDDSInput.detach(recId)
#        
#        print 'callExecution:',self.callExecution
#        
#        attachedIDs = []
#        for x in range(5):
#            _maddress = 'address' + str(x)
#            _id = str(x+1)
#            _vlan = x+2
#            _port = x+3
#            _sr = x+4
#            _pi = str(x+5)
#            sd = BULKIO.SDDSStreamDefinition(id=_id, dataFormat=BULKIO.SDDS_SI, multicastAddress=_maddress, vlan=_vlan, 
#                                             port=_port, sampleRate=_sr, timeTagValid=True, privateInfo=_pi)
#            print
#            print 'unit test module: calling attach()'
#            recId = self.dataSDDSInput.attach(sd, 'blank')
#            expId = _maddress + '_' + _id + str(_vlan) + str(_port) + str(_sr) + _pi 
#            print "unit test module: received ID: " + recId
##           self.assertEquals(expId, recId)
#            attachedIDs.append(recId)
        
#        self.comp_obj.stop()
    
if __name__ == "__main__":
    ossie.utils.testing.main("../bulkio_ports.spd.xml") # By default tests all implementations
    
