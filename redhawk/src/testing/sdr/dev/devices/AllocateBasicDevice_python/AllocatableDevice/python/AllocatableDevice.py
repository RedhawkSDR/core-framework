#!/usr/bin/env python
#
#
# AUTO-GENERATED
#
# Source: AllocatableDevice.spd.xml
from ossie.device import start_device
import logging

from AllocatableDevice_base import *

class AllocatableDevice_i(AllocatableDevice_base):
    """<DESCRIPTION GOES HERE>"""
    def constructor(self):
        """
        This is called by the framework immediately after your device registers with the system.
        
        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()

        """
        # Save initial "count" value as the max value it can ever have
        self._maxCount = self.count
        self.setAllocationImpl("count", self._allocateCount, self._deallocateCount)

    def _allocateCount(self, value):
        if self.count >= value:
            self.count -= value
            return True
        return False

    def _deallocateCount(self, value):
        count = self.count + value
        if count > self._maxCount:
            self._baseLog.warn("New count %d would exceed max count %d", count, self._maxCount)
            count = self._maxCount
        self.count = count
        
    def updateUsageState(self):
        """
        This is called automatically after allocateCapacity or deallocateCapacity are called.
        Your implementation should determine the current state of the device:
           self._usageState = CF.Device.IDLE   # not in use
           self._usageState = CF.Device.ACTIVE # in use, with capacity remaining for allocation
           self._usageState = CF.Device.BUSY   # in use, with no capacity remaining for allocation
        """
        if self.count == 0:
            self._usageState = CF.Device.BUSY
        elif self.count < self._maxCount:
            self._usageState = CF.Device.ACTIVE
        else:
            self._usageState = CF.Device.IDLE

    def process(self):
        """
        Basic functionality:
        
            The process method should process a single "chunk" of data and then return. This method
            will be called from the processing thread again, and again, and again until it returns
            FINISH or stop() is called on the device.  If no work is performed, then return NOOP.
            
        StreamSRI:
            To create a StreamSRI object, use the following code (this generates a normalized SRI that does not flush the queue when full):
                sri = bulkio.sri.create("my_stream_id")

        PrecisionUTCTime:
            To create a PrecisionUTCTime object, use the following code:
                tstamp = bulkio.timestamp.now() 
  
        Ports:

            Each port instance is accessed through members of the following form: self.port_<PORT NAME>
            
            Data is passed to the serviceFunction through by reading from input streams
            (BulkIO only). UDP multicast (dataSDDS and dataVITA49) ports do not support
            streams.

            The input stream from which to read can be requested with the getCurrentStream()
            method. The optional argument to getCurrentStream() is a floating point number that
            specifies the time to wait in seconds. A zero value is non-blocking. A negative value
            is blocking.  Constants have been defined for these values, bulkio.const.BLOCKING and
            bulkio.const.NON_BLOCKING.

            More advanced uses of input streams are possible; refer to the REDHAWK documentation
            for more details.

            Input streams return data blocks that include the SRI that was in effect at the time
            the data was received, and the time stamps associated with that data.

            To send data using a BulkIO interface, create an output stream and write the
            data to it. When done with the output stream, the close() method sends and end-of-
            stream flag and cleans up.

            If working with complex data (i.e., the "mode" on the SRI is set to 1),
            the data block's complex attribute will return True. Data blocks provide a
            cxdata attribute that gives the data as a list of complex values:

                if block.complex:
                    outData = [val.conjugate() for val in block.cxdata]
                    outputStream.write(outData, block.getStartTime())

            Interactions with non-BULKIO ports are left up to the device developer's discretion.

        Messages:
    
            To receive a message, you need (1) an input port of type MessageEvent, (2) a message prototype described
            as a structure property of kind message, (3) a callback to service the message, and (4) to register the callback
            with the input port.
        
            Assuming a property of type message is declared called "my_msg", an input port called "msg_input" is declared of
            type MessageEvent, create the following code:
        
            def msg_callback(self, msg_id, msg_value):
                print msg_id, msg_value
        
            Register the message callback onto the input port with the following form:
            self.port_input.registerMessage("my_msg", AllocatableDevice_i.MyMsg, self.msg_callback)
        
            To send a message, you need to (1) create a message structure, and (2) send the message over the port.
        
            Assuming a property of type message is declared called "my_msg", an output port called "msg_output" is declared of
            type MessageEvent, create the following code:
        
            msg_out = AllocatableDevice_i.MyMsg()
            self.port_msg_output.sendMessage(msg_out)

    Accessing the Application and Domain Manager:
    
        Both the Application hosting this Component and the Domain Manager hosting
        the Application are available to the Component.
        
        To access the Domain Manager:
            dommgr = self.getDomainManager().getRef()
        To access the Application:
            app = self.getApplication().getRef()
        Properties:
        
            Properties are accessed directly as member variables. If the property name is baudRate,
            then accessing it (for reading or writing) is achieved in the following way: self.baudRate.

            To implement a change callback notification for a property, create a callback function with the following form:

            def mycallback(self, id, old_value, new_value):
                pass

            where id is the property id, old_value is the previous value, and new_value is the updated value.
            
            The callback is then registered on the component as:
            self.addPropertyChangeListener('baudRate', self.mycallback)

        Logging:

            The member _baseLog is a logger whose base name is the component (or device) instance name.
            New logs should be created based on this logger name.

            To create a new logger,
                my_logger = self._baseLog.getChildLogger("foo")

            Assuming component instance name abc_1, my_logger will then be created with the 
            name "abc_1.user.foo".
            
        Allocation:
            
            Allocation callbacks are available to customize a Device's response to an allocation request. 
            Callback allocation/deallocation functions are registered using the setAllocationImpl function,
            usually in the initialize() function
            For example, allocation property "my_alloc" can be registered with allocation function 
            my_alloc_fn and deallocation function my_dealloc_fn as follows:
            
            self.setAllocationImpl("my_alloc", self.my_alloc_fn, self.my_dealloc_fn)
            
            def my_alloc_fn(self, value):
                # perform logic
                return True # successful allocation
            
            def my_dealloc_fn(self, value):
                # perform logic
                pass
            
        Example:
        
            # This example assumes that the device has two ports:
            #   - A provides (input) port of type bulkio.InShortPort called dataShort_in
            #   - A uses (output) port of type bulkio.OutFloatPort called dataFloat_out
            # The mapping between the port and the class is found in the device
            # base class.
            # This example also makes use of the following Properties:
            #   - A float value called amplitude
            #   - A boolean called increaseAmplitude
            
            inputStream = self.port_dataShort_in.getCurrentStream()
            if not inputStream:
                return NOOP

            outputStream = self.port_dataFloat_out.getStream(inputStream.streamID)
            if not outputStream:
                outputStream = self.port_dataFloat_out.createStream(inputStream.sri)

            block = inputStream.read()
            if not block:
                if inputStream.eos():
                    outputStream.close()
                return NOOP

            if self.increaseAmplitude:
                scale = self.amplitude
            else:
                scale = 1.0
            outData = [float(val) * scale for val in block.data]

            if block.sriChanged:
                outputStream.sri = block.sri

            outputStream.write(outData, block.getStartTime())
            return NORMAL
            
        """
        return FINISH

  
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.INFO)
    logging.debug("Starting Device")
    start_device(AllocatableDevice_i)

