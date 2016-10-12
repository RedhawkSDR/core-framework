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
#
# AUTO-GENERATED
#
# Source: TestLoggingAPI.spd.xml
from ossie.resource import start_component
import logging

from TestLoggingAPI_base import *


class TestLoggingAPI_i(TestLoggingAPI_base):

    class MyLogListenerCB(object):
        def __init__(self, Parent):
            self.parent = Parent

        def logLevelChanged( self, log_id, new_v):
            self.parent.new_log_level = new_v

        def logConfigChanged( self, new_config):
            self.parent.new_log_cfg = new_config

    """<DESCRIPTION GOES HERE>"""
    def __init__(self, identifier, execparams):
        TestLoggingAPI_base.__init__(self, identifier, execparams )
        if execparams.has_key('DISABLE_CB'):
            if execparams['DISABLE_CB'].upper() != 'TRUE':
                self.setLogListenerCallback( TestLoggingAPI_i.MyLogListenerCB(self) )
        else:
          self.setLogListenerCallback( TestLoggingAPI_i.MyLogListenerCB(self) )


    def initialize(self):
        """
        This is called by the framework immediately after your component registers with the NameService.
        
        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()
        """
        TestLoggingAPI_base.initialize(self)
        # TODO add customization here.
        

    def process(self):
        """
        Basic functionality:
        
            The process method should process a single "chunk" of data and then return. This method
            will be called from the processing thread again, and again, and again until it returns
            FINISH or stop() is called on the component.  If no work is performed, then return NOOP.
            
        StreamSRI:
            To create a StreamSRI object, use the following code (this generates a normalized SRI that does not flush the queue when full):
                self.sri = bulkio.sri.create(self.stream_id)

        PrecisionUTCTime:
            To create a PrecisionUTCTime object, use the following code:
                tstamp = bulkio.timestamp.now() 
  
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

            To implement a change callback for a property, add the following member to the class:

            def onconfigure_prop_baudRate(self, old_value, new_value):
                self.baudRate = new_value

            where old_value is the previous value, and new_value is the updated value.

            Note: the value of the property (i.e.: self.baudRate) must be manually updated
            
        Example:
        
            # This example assumes that the component has two ports:
            #   - A provides (input) port of type bulkio.InShortPort called dataShort_in
            #   - A uses (output) port of type bulkio.OutFloatPort called dataFloat_out
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
        self._log.fatal("process() example log message - FATAL ")
        self._log.error("process() example log message - ERROR ")
        self._log.warn("process() example log message - WARN ")
        self._log.info("process() example log message - INFO ")
        self._log.debug("process() example log message - DEBUG ")
        self._log.trace("process() example log message - TRACE ")
        return NOOP
        
  
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.INFO)
    logging.debug("Starting Component")
    start_component(TestLoggingAPI_i)

