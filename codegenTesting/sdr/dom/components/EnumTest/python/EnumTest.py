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
#
# AUTO-GENERATED
#
# Source: EnumTest.spd.xml
from ossie.resource import start_component
import logging

from EnumTest_base import EnumTest_base, enums, NOOP

from ossie import properties
from ossie.cf import CF

class EnumTest_i(EnumTest_base):
    """<DESCRIPTION GOES HERE>"""
    def constructor(self):
        """
        This is called by the framework immediately after your component registers with the system.
        
        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()

        """
        # TODO add customization here.
        
    def process(self):
        """
        Basic functionality:
        
            The process method should process a single "chunk" of data and then return. This method
            will be called from the processing thread again, and again, and again until it returns
            FINISH or stop() is called on the component.  If no work is performed, then return NOOP.
            
        StreamSRI:
            To create a StreamSRI object, use the following code (this generates a normalized SRI that does not flush the queue when full):
                sri = bulkio.sri.create("my_stream_id")

        PrecisionUTCTime:
            To create a PrecisionUTCTime object, use the following code:
                tstamp = bulkio.timestamp.now() 
  
        Ports:

            Each port instance is accessed through members of the following form: self.port_<PORT NAME>
            
            Data is obtained in the process function through the getPacket call (BULKIO only) on a
            provides port member instance. The optional argument is a timeout value, in seconds.
            A zero value is non-blocking, while a negative value is blocking. Constants have been
            defined for these values, bulkio.const.BLOCKING and bulkio.const.NON_BLOCKING. If no
            timeout is given, it defaults to non-blocking.
            
            The return value is a named tuple with the following fields:
                - dataBuffer
                - T
                - EOS
                - streamID
                - SRI
                - sriChanged
                - inputQueueFlushed
            If no data is available due to a timeout, all fields are None.

            To send data, call the appropriate function in the port directly. In the case of BULKIO,
            convenience functions have been added in the port classes that aid in output.
            
            Interactions with non-BULKIO ports are left up to the component developer's discretion.
            
        Messages:
    
            To receive a message, you need (1) an input port of type MessageEvent, (2) a message prototype described
            as a structure property of kind message, (3) a callback to service the message, and (4) to register the callback
            with the input port.
        
            Assuming a property of type message is declared called "my_msg", an input port called "msg_input" is declared of
            type MessageEvent, create the following code:
        
            def msg_callback(self, msg_id, msg_value):
                print msg_id, msg_value
        
            Register the message callback onto the input port with the following form:
            self.port_input.registerMessage("my_msg", EnumTest_i.MyMsg, self.msg_callback)
        
            To send a message, you need to (1) create a message structure, and (2) send the message over the port.
        
            Assuming a property of type message is declared called "my_msg", an output port called "msg_output" is declared of
            type MessageEvent, create the following code:
        
            msg_out = EnumTest_i.MyMsg()
            this.port_msg_output.sendMessage(msg_out)

    Accessing the Device Manager and Domain Manager:
    
        Both the Device Manager hosting this Device and the Domain Manager hosting
        the Device Manager are available to the Device.
        
        To access the Domain Manager:
            dommgr = self.getDomainManager().getRef();
        To access the Device Manager:
            devmgr = self.getDeviceManager().getRef();
        Properties:
        
            Properties are accessed directly as member variables. If the property name is baudRate,
            then accessing it (for reading or writing) is achieved in the following way: self.baudRate.

            To implement a change callback notification for a property, create a callback function with the following form:

            def mycallback(self, id, old_value, new_value):
                pass

            where id is the property id, old_value is the previous value, and new_value is the updated value.
            
            The callback is then registered on the component as:
            self.addPropertyChangeListener('baudRate', self.mycallback)
            
            
        Example:
        
            # This example assumes that the component has two ports:
            #   - A provides (input) port of type bulkio.InShortPort called dataShort_in
            #   - A uses (output) port of type bulkio.OutFloatPort called dataFloat_out
            # The mapping between the port and the class if found in the component
            # base class.
            # This example also makes use of the following Properties:
            #   - A float value called amplitude
            #   - A boolean called increaseAmplitude
            
            packet = self.port_dataShort_in.getPacket()
            
            if packet.dataBuffer is None:
                return NOOP
                
            outData = range(len(packet.dataBuffer))
            for i in range(len(packet.dataBuffer)):
                if self.increaseAmplitude:
                    outData[i] = float(packet.dataBuffer[i]) * self.amplitude
                else:
                    outData[i] = float(packet.dataBuffer[i])
                
            # NOTE: You must make at least one valid pushSRI call
            if packet.sriChanged:
                self.port_dataFloat_out.pushSRI(packet.SRI);

            self.port_dataFloat_out.pushPacket(outData, packet.T, packet.EOS, packet.streamID)
            return NORMAL
            
        """

        # TODO fill in your code here
        self._log.debug("process() example log message")
        return NOOP

    def runTest(self, testid, testValues):
        if testid == 0:
            return self.runEnumTest(testValues)
        else:
            raise CF.TestableObject.UnknownTest()

    def runEnumTest(self, testValues):
        unknown = []

        for prop in testValues:
            value = {}
            if prop.id == "floatenum":
                value['DEFAULT'] = enums.floatenum.DEFAULT
                value['OTHER'] = enums.floatenum.OTHER
            elif prop.id == "stringenum":
                value['START'] = enums.stringenum.START
                value['STOPPED'] = enums.stringenum.STOPPED
            elif prop.id == "structprop":
                number_enums = {}
                number_enums['ZERO'] = enums.structprop.number.ZERO
                number_enums['ONE'] = enums.structprop.number.ONE
                number_enums['TWO'] = enums.structprop.number.TWO
                value['structprop::number'] = number_enums

                alpha_enums = {}
                alpha_enums['ABC'] = enums.structprop.alpha.ABC
                alpha_enums['DEF'] = enums.structprop.alpha.DEF
                value['structprop::alpha'] = alpha_enums
            elif prop.id == "structseq":
                number_enums = {}
                number_enums['POSITIVE'] = enums.structseq_struct.number.POSITIVE
                number_enums['ZERO'] = enums.structseq_struct.number.ZERO
                number_enums['NEGATIVE'] = enums.structseq_struct.number.NEGATIVE
                value['structseq::number'] = number_enums

                text_enums = {}
                text_enums['HEADER'] = enums.structseq_struct.text.HEADER
                text_enums['BODY'] = enums.structseq_struct.text.BODY
                text_enums['FOOTER'] = enums.structseq_struct.text.FOOTER
                value['structseq::text'] = text_enums
            else:
                unknown.append(prop)
            prop.value = properties.props_to_any(properties.props_from_dict(value))

        if unknown:
            raise CF.UnknownProperties(unknown)

        return testValues

if __name__ == '__main__':
    logging.getLogger().setLevel(logging.INFO)
    logging.debug("Starting Component")
    start_component(EnumTest_i)
