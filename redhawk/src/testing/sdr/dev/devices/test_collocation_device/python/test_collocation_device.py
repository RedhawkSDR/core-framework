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
# Source: test_collocation_device.spd.xml
# Generated on: Wed Oct 31 16:44:08 EDT 2012
# Redhawk IDE
# Version:@buildLabel@
# Build id: @buildId@
from ossie.device import start_device
import logging

from test_collocation_device_base import * 

class test_collocation_device_i(test_collocation_device_base):
    """<DESCRIPTION GOES HERE>"""
    def initialize(self):
        """
        This is called by the framework immediately after your component registers with the NameService.
        
        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()
        """
        test_collocation_device_base.initialize(self)
        # TODO add customization here.
        self.current_components = 0
        
    def updateUsageState(self):
        """
        This is called automatically after allocateCapacity or deallocateCapacity are called.
        Your implementation should determine the current state of the device:
           self._usageState = CF.Device.IDLE   # not in use
           self._usageState = CF.Device.ACTIVE # in use, with capacity remaining for allocation
           self._usageState = CF.Device.BUSY   # in use, with no capacity remaining for allocation
        """
        return NOOP
    
    ###########################################
    # CF::Device
    ###########################################
    def allocateCapacity(self, properties):
        self.allocation_attempts += 1
        result = test_collocation_device_base.allocateCapacity(self, properties)
        # After the allocation send an event for all properties
        try:
            self.port_propEvent.sendPropertiesEvent()
        except:
            self._log.exception("Error sending properties event")
        return result
        
    def deallocateCapacity(self, properties):
        test_collocation_device_base.deallocateCapacity(self, properties)
        # After the deallocation send an event for all properties
        try:
            self.port_propEvent.sendPropertiesEvent()
        except:
            self._log.exception("Error sending properties event")
            
            
    def allocate_supported_components(self, value):
        self._log.info("Allocate:   Value/Total Allowed/Current Deployed " + str(value) + "/" + str( self.supported_components) +
                            "/" + str(self.current_components) )
        
        # see if calculated capacity and measured capacity is avaliable
        if  self.supported_components - self.current_components == 0 :
            return False

        self.current_components = self.current_components + value
        self._log.info(" Completed Allocate:  Total/Remaining" +  str(self.supported_components) + "/" + str( self.supported_components - self.current_components) );

        return True
    
    # overrides deallocateCapacity for memCapacity
    def deallocate_supported_components(self, value):
        self._log.debug("deallocate_supported_components: " + str( value) )
        # make sure this isn't over the actual capacity (due to an update of the memThreshold)
        if self.current_components - value < 0:
            return False;
        
        self.current_components = self.current_components - value
        if self.current_components < 0:  
            self.current_components = 0
            
        self._log.info(" DeAllocate Completed:  Total/Remaining" +  str(self.supported_components) + "/" + str( self.supported_components - self.current_components) );



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
                
            // NOTE: You must make at least one valid pushSRI call
            if sriChanged:
                self.port_dataFloat_out.pushSRI(sri);
            }
            self.port_dataFloat_out.pushPacket(outData, T, EOS, streamID)
            return NORMAL
            
        """

        # TODO fill in your code here
        self._log.debug("process() example log message")
        return NOOP
        
  
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.WARN)
    logging.debug("Starting Device")
    start_device(test_collocation_device_i)
