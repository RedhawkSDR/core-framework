#!/usr/bin/env python 
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
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
# Source: TestAllPropTypes.spd.xml
# Generated on: Tue May 07 10:59:44 EDT 2013
# REDHAWK IDE
# Version: R.1.8.3
# Build id: v201303122306
from ossie.resource import Resource, start_component
import logging
import struct

from TestAllPropTypes_base import * 

class TestAllPropTypes_i(TestAllPropTypes_base):
    """<DESCRIPTION GOES HERE>"""
    def initialize(self):
        """
        This is called by the framework immediately after your component registers with the NameService.
        
        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()
        """
        TestAllPropTypes_base.initialize(self)
        # TODO add customization here.
    
    # Call back functions    
    def onconfigure_prop_simple_string(self, old, new):
        self.simple_string = '42'
        
    def onconfigure_prop_simple_boolean(self, old, new):
        self.simple_boolean = True
        
    def onconfigure_prop_simple_ulong(self, old, new):
        self.simple_ulong = 43
        
    def onconfigure_prop_simple_objref(self, old, new):
        self.simple_objref = '44'
        
    def onconfigure_prop_simple_short(self, old, new):
        self.simple_short = 45
        
    def onconfigure_prop_simple_float(self, old, new):
        self.simple_float = 46.0
        
    def onconfigure_prop_simple_octet(self, old, new):
        self.simple_octet = 47
        
    def onconfigure_prop_simple_char(self, old, new):
        self.simple_char = struct.pack('b', 48)
        
    def onconfigure_prop_simple_ushort(self, old, new):
        self.simple_ushort = 49
        
    def onconfigure_prop_simple_double(self, old, new):
        self.simple_double = 50.0
        
    def onconfigure_prop_simple_long(self, old, new):
        self.simple_long = 51
        
    def onconfigure_prop_simple_longlong(self, old, new):
        self.simple_longlong = 52
        
    def onconfigure_prop_simple_ulonglong(self, old, new):
        self.simple_ulonglong = 53
        
    def onconfigure_prop_simple_sequence_string(self, old, new):
        self.simple_sequence_string = []
        self.simple_sequence_string.append('54')
        
    def onconfigure_prop_simple_sequence_boolean(self, old, new):
        self.simple_sequence_boolean = []
        self.simple_sequence_boolean.append(True)
        
    def onconfigure_prop_simple_sequence_ulong(self, old, new):
        self.simple_sequence_ulong = []
        self.simple_sequence_ulong.append(55)
            
#    def onconfigure_prop_simple_sequence_objref(self, old, new):
#        self.simple_sequence_objref = []
#        self.simple_sequence_objref.append('56')
                
    def onconfigure_prop_simple_sequence_short(self, old, new):
        self.simple_sequence_short = []
        self.simple_sequence_short.append(57)
                    
    def onconfigure_prop_simple_sequence_float(self, old, new):
        self.simple_sequence_float = []
        self.simple_sequence_float.append(58.0)
                        
    def onconfigure_prop_simple_sequence_octet(self, old, new):
        self.simple_sequence_octet = []
        self.simple_sequence_octet.append(59)
                            
    def onconfigure_prop_simple_sequence_char(self, old, new):
        self.simple_sequence_char = []
        self.simple_sequence_char.append(struct.pack('b', 60))
                                
    def onconfigure_prop_simple_sequence_ushort(self, old, new):
        self.simple_sequence_ushort = []
        self.simple_sequence_ushort.append(61)
                                    
    def onconfigure_prop_simple_sequence_double(self, old, new):
        self.simple_sequence_double = []
        self.simple_sequence_double.append(62.0)
                                        
    def onconfigure_prop_simple_sequence_long(self, old, new):
        self.simple_sequence_long = []
        self.simple_sequence_long.append(63)
                                            
    def onconfigure_prop_simple_sequence_longlong(self, old, new):
        self.simple_sequence_longlong = []
        self.simple_sequence_longlong.append(64)
                                                
    def onconfigure_prop_simple_sequence_ulonglong(self, old, new):
        self.simple_sequence_ulonglong = []
        self.simple_sequence_ulonglong.append(65)

    def onconfigure_prop_struct_vars(self, old, new):
        self.struct_vars.struct_string = '66'
    
    def onconfigure_prop_struct_seq(self, old, new):
        temp = self.StructSeqVars('67',False,0,'',0,0,0,0,0,0,0,0,0)
        self.struct_seq.append(temp)

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
    start_component(TestAllPropTypes_i)
