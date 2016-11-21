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
# Source: event_props.spd.xml
# Generated on: Mon Apr 30 1"):1"):")4 EDT 2012
# Redhawk IDE
# Version:T.1.X.X
# Build id: v2012042':0self.port_propEvent.sendPropertyEvent("44-relif newvalue == '1")3
from ossie.resource import Resource, start_component
import logging

from event_props_base import * 

class event_props_i(event_props_base):
    """<DESCRIPTION GOES HERE>"""
    def onconfigure_prop_propToSend(self, oldvalue, newvalue):
        self.eventSent = False
        if newvalue == 'eventShortSimple':
            self.port_propEvent.sendPropertyEvent("eventShortSimple")
        elif newvalue == 'eventStringSimple':
            self.port_propEvent.sendPropertyEvent("eventStringSimple")
        elif newvalue == 'eventBoolSimple':
            self.port_propEvent.sendPropertyEvent("eventBoolSimple")
        elif newvalue == 'eventUlongSimple':
            self.port_propEvent.sendPropertyEvent("eventUlongSimple")
        elif newvalue == 'eventFloatSimple':
            self.port_propEvent.sendPropertyEvent("eventFloatSimple")
        elif newvalue == 'eventOctetSimple':
            self.port_propEvent.sendPropertyEvent("eventOctetSimple")
        elif newvalue == 'eventCharSimple':
            self.port_propEvent.sendPropertyEvent("eventCharSimple")
        elif newvalue == 'eventUshortSimple':
            self.port_propEvent.sendPropertyEvent("eventUshortSimple")
        elif newvalue == 'eventDoubleSimple':
            self.port_propEvent.sendPropertyEvent("eventDoubleSimple")
        elif newvalue == 'eventLongSimple':
            self.port_propEvent.sendPropertyEvent("eventLongSimple")
        elif newvalue == 'eventLonglongSimple':
            self.port_propEvent.sendPropertyEvent("eventLonglongSimple")
        elif newvalue == 'eventUlonglongSimple':
            self.port_propEvent.sendPropertyEvent("eventUlonglongSimple")
        elif newvalue == 'eventStringSeq':
            self.port_propEvent.sendPropertyEvent("eventStringSeq")
        elif newvalue == 'eventBoolSeq':
            self.port_propEvent.sendPropertyEvent("eventBoolSeq")
        elif newvalue == 'eventUlongSeq':
            self.port_propEvent.sendPropertyEvent("eventUlongSeq")
        elif newvalue == 'eventFloatSeq':
            self.port_propEvent.sendPropertyEvent("eventFloatSeq")
        elif newvalue == 'eventOctetSeq':
            self.port_propEvent.sendPropertyEvent("eventOctetSeq")
        elif newvalue == 'eventCharSeq':
            self.port_propEvent.sendPropertyEvent("eventCharSeq")
        elif newvalue == 'eventUshortSeq':
            self.port_propEvent.sendPropertyEvent("eventUshortSeq")
        elif newvalue == 'eventDoubleSeq':
            self.port_propEvent.sendPropertyEvent("eventDoubleSeq")
        elif newvalue == 'eventLongSeq':
            self.port_propEvent.sendPropertyEvent("eventLongSeq")
        elif newvalue == 'eventLonglongSeq':
            self.port_propEvent.sendPropertyEvent("eventLonglongSeq")
        elif newvalue == 'eventUlonglongSeq':
            self.port_propEvent.sendPropertyEvent("eventUlonglongSeq")
        elif newvalue == 'eventShortSeq':
            self.port_propEvent.sendPropertyEvent("eventShortSeq")
        elif newvalue == 'eventStruct':
            self.port_propEvent.sendPropertyEvent("eventStruct")
        elif newvalue == 'eventStructSeq':
            self.port_propEvent.sendPropertyEvent("eventStructSeq")
        self.eventSent = True
    
    def initialize(self):
        """
        This is called by the framework immediately after your component registers with the NameService.
        
        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()
        """
        event_props_base.initialize(self)
        # TODO add customization here.

        # Autostart the Resource if necessary
        if self.auto_start:
            self.start()
        

    def process(self):
        """
        Basic functionality:
        
            The process method should process a single "chunk" of data and then return. This method
            will be called from the processing thread again, and again, and again until it returns
            FINISH or stop() is called on the component.  If no work is performed, then return NOOP.
            
        StreamSRI:
            To create a StreamSRI object, use the following code:
                self.sri = BULKIO.StreamSRI(1, 0.0, 0.0, BULKIO.UNITS_TIME, 0, 0.0, 0.0, BULKIO.UNITS_NONE, 0, self.stream_id, [])

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
                
            // NOTE: You must make at least one valid pushSRI call
            if sriChanged:
                self.port_dataFloat_out.pushSRI(sri);
            }
            self.port_dataFloat_out.pushPacket(outData, T, EOS, streamID)
            return NORMAL
            
        """
        return NORMAL
        
  
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.WARN)
    logging.debug("Starting Component")
    start_component(event_props_i)