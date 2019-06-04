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
#
# AUTO-GENERATED
#
# Source: props.spd.xml
from ossie.resource import Resource, start_component
import logging

from props_base import *

class props_i(props_base):
    """<DESCRIPTION GOES HERE>"""
    def initialize(self):
        """
        This is called by the framework immediately after your component registers with the NameService.
        
        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()
        """
        props_base.initialize(self)
        # TODO add customization here.
        
    def onconfigure_prop_stringSimple(self, oldvalue, newvalue):
        self.stringSimple = newvalue*2
        
    def onconfigure_prop_boolSimple(self, oldvalue, newvalue):
        if newvalue == None:
            self.boolSimple = None
            return
        self.boolSimple = not newvalue

    def onconfigure_prop_ulongSimple(self, oldvalue, newvalue):
        self.ulongSimple = newvalue*2

    def onconfigure_prop_shortSimple(self, oldvalue, newvalue):
        self.shortSimple = newvalue*2
    
    def onconfigure_prop_floatSimple(self, oldvalue, newvalue):
        self.floatSimple = newvalue*2

    def onconfigure_prop_octetSimple(self, oldvalue, newvalue):
        self.octetSimple = newvalue*2

    def onconfigure_prop_charSimple(self, oldvalue, newvalue):
        self.charSimple = newvalue.upper()

    def onconfigure_prop_ushortSimple(self, oldvalue, newvalue):
        self.ushortSimple = newvalue*2

    def onconfigure_prop_doubleSimple(self, oldvalue, newvalue):
        self.doubleSimple = newvalue*2

    def onconfigure_prop_longSimple(self, oldvalue, newvalue):
        self.longSimple = newvalue*2
        
    def onconfigure_prop_longlongSimple(self, oldvalue, newvalue):
        self.longlongSimple = newvalue*2
        
    def onconfigure_prop_ulonglongSimple(self, oldvalue, newvalue):
        self.ulonglongSimple = newvalue*2
        
    def onconfigure_prop_stringSeq(self, oldvalue, newvalue):
        if newvalue:
            newvalue.reverse()
            self.stringSeq = newvalue
        
    def onconfigure_prop_boolSeq(self, oldvalue, newvalue):
        if newvalue:
            newvalue.reverse()
            self.boolSeq = newvalue

    def onconfigure_prop_ulongSeq(self, oldvalue, newvalue):
        if newvalue:
            newvalue.reverse()
            self.ulongSeq = newvalue

    def onconfigure_prop_shortSeq(self, oldvalue, newvalue):
        if newvalue:
            newvalue.reverse()
            self.shortSeq = newvalue
    
    def onconfigure_prop_floatSeq(self, oldvalue, newvalue):
        if newvalue:
            newvalue.reverse()
            self.floatSeq = newvalue

    def onconfigure_prop_octetSeq(self, oldvalue, newvalue):
        if newvalue:
            list = [x for x in newvalue]
            list.reverse()
            tmp = ''
            for x in list:
                tmp = tmp+x
            self.octetSeq = tmp

    def onconfigure_prop_charSeq(self, oldvalue, newvalue):
        if newvalue:
            list = [x for x in newvalue]
            list.reverse()
            self.charSeq = list
            tmp = ''
            for x in list:
                tmp = tmp+x
            self.charSeq = tmp

    def onconfigure_prop_ushortSeq(self, oldvalue, newvalue):
        if newvalue:
            newvalue.reverse()
            self.ushortSeq = newvalue

    def onconfigure_prop_doubleSeq(self, oldvalue, newvalue):
        if newvalue:
            newvalue.reverse()
            self.doubleSeq = newvalue

    def onconfigure_prop_longSeq(self, oldvalue, newvalue):
        if newvalue:
            newvalue.reverse()
            self.longSeq = newvalue
        
    def onconfigure_prop_longlongSeq(self, oldvalue, newvalue):
        if newvalue:
            newvalue.reverse()
            self.longlongSeq = newvalue
        
    def onconfigure_prop_ulonglongSeq(self, oldvalue, newvalue):
        if newvalue:
            newvalue.reverse()
            self.ulonglongSeq = newvalue
            
    def onconfigure_prop_structProp(self, oldvalue, newvalue):
        try:
            self.structProp.structShortSimple = newvalue.structShortSimple * 2
            self.structProp.structFloatSimple = newvalue.structFloatSimple * 2
            self.structProp.structOctetSimple = newvalue.structOctetSimple * 2
            self.structProp.structUlongSimple = newvalue.structUlongSimple * 2
            self.structProp.structUshortSimple = newvalue.structUshortSimple * 2
            self.structProp.structStringSimple = newvalue.structStringSimple * 2
            self.structProp.structDoubleSimple = newvalue.structDoubleSimple * 2
            self.structProp.structLonglongSimple = newvalue.structLonglongSimple * 2
            self.structProp.structBoolSimple = not newvalue.structBoolSimple
            self.structProp.structLongSimple = newvalue.structLongSimple * 2
            self.structProp.structUlonglongSimple = newvalue.structUlonglongSimple * 2
            self.structProp.structCharSimple = newvalue.structCharSimple.upper()
        except:
            pass

    def onconfigure_prop_structSeqProp(self, oldvalue, newvalue):
        for x in newvalue:
            shortVal = x.structSeqShortSimple * 2
            floatVal = x.structSeqFloatSimple * 2
            stringVal = x.structSeqStringSimple * 2
            boolVal = not x.structSeqBoolSimple
            
            self.structSeqProp.append(self.StructSeq(stringVal, boolVal, shortVal, floatVal)) 

    def process(self):
        """
        Basic functionality:
        
            The process method should process a single "chunk" of data and then return. This method
            will be called from the processing thread again, and again, and again until it returns
            FINISH or stop() is called on the component.  If no work is performed, then return NOOP.
            
        StreamSRI:
            To create a StreamSRI object, use the following code (this generates a normalized SRI that does not flush the queue when full):
                self.sri = BULKIO.StreamSRI(1, 0.0, 0.0, BULKIO.UNITS_TIME, 0, 0.0, 0.0, BULKIO.UNITS_NONE, 0, self.stream_id, True, [])

        PrecisionUTCTime:
            To create a PrecisionUTCTime object, use the following code:
                tmp_time = time.time()
                wsec = math.modf(tmp_time)[1]
                fsec = math.modf(tmp_time)[0]
                tstamp = BULKIO.PrecisionUTCTime(BULKIO.TCM_CPU, BULKIO.TCS_VALID, 0, wsec, fsec)
  
        Ports:

            Each port instance is accessed through members of the following form: self.port_<PORT NAME>
            
            Data is obtained in the process function through the getPacket call (BULKIO only) on a
            provides port member instance. The getPacket function call is non-blocking - if no data
            is available, it will return immediately with all values == None.
            
            To send data, call the appropriate function in the port directly. In the case of BULKIO,
            convenience functions have been added in the port classes that aid in output.
            
            Interactions with non-BULKIO ports are left up to the component developer's discretion.
            
        Properties:
        
            Properties are accessed directly as member variables. If the property name is baudRate,
            then accessing it (for reading or writing) is achieved in the following way: self.baudRate.
            
        Example:
        
            # This example assumes that the component has two ports:
            #   - A provides (input) port of type BULKIO.dataShort called dataShort_in
            #   - A uses (output) port of type BULKIO.dataFloat called dataFloat_out
            # The mapping between the port and the class if found in the component
            # base class.
            # This example also makes use of the following Properties:
            #   - A float value called amplitude
            #   - A boolean called increaseAmplitude
            
            data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed = self.port_dataShort_in.getPacket()
            
            if data == None:
                return NOOP
                
            outData = range(len(data))
            for i in range(len(data)):
                if self.increaseAmplitude:
                    outData[i] = float(data[i]) * self.amplitude
                else:
                    outData[i] = float(data[i])
                
            # NOTE: You must make at least one valid pushSRI call
            if sriChanged:
                self.port_dataFloat_out.pushSRI(sri);

            self.port_dataFloat_out.pushPacket(outData, T, EOS, streamID)
            return NORMAL
            
        """

        # TODO fill in your code here
        self._log.debug("process() example log message")
        return NOOP
        
  
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.WARN)
    logging.debug("Starting Component")
    start_component(props_i)

