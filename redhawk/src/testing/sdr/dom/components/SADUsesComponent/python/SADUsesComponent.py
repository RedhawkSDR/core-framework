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
# Source: SADUsesComponent.spd.xml
# Generated on: Wed May 29 14:38:42 EDT 2013
# REDHAWK IDE
# Version: 1.8.4
# Build id: R201305151907
from ossie.resource import Resource, start_component
import logging

from ossie.cf import ExtendedCF, ExtendedCF__POA

from SADUsesComponent_base import * 

# Add the QueryablePort interface to 'comp_resource_out'; this should be rolled
# into the base classes and code generators eventually
class ResourceOutPort(ExtendedCF__POA.QueryablePort, PortCFResourceOut_i):
    def _get_connections(self):
        self.port_lock.acquire()
        try:    
            return [ExtendedCF.UsesConnection(connId, port) for connId, port in list(self.outConnections.items())]
        finally:
            self.port_lock.release()

class SADUsesComponent_i(SADUsesComponent_base):
    """<DESCRIPTION GOES HERE>"""
    def initialize(self):
        """
        This is called by the framework immediately after your component registers with the NameService.
        
        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()
        """
        SADUsesComponent_base.initialize(self)
        # TODO add customization here.
        self.port_comp_resource_out = ResourceOutPort(self, 'comp_resource_out')

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
    start_component(SADUsesComponent_i)
