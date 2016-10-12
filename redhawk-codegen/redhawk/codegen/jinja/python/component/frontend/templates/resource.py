#{#
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
#}
#% set className = component.userclass.name
#% set baseClass = component.baseclass.name
#% set artifactType = component.artifacttype
#!/usr/bin/env python
#
# AUTO-GENERATED
#
# Source: ${component.profile.spd}
#{% if component is device %}
from ossie.device import start_device
#{% else %}
from ossie.resource import Resource, start_component
#{% endif %}
import logging

from ${baseClass} import *

class ${className}(${baseClass}):
#{% if component.description %}
    """${component.description()}"""
#{% else %}
    """<DESCRIPTION GOES HERE>"""
#{% endif %}
    def initialize(self):
        """
        This is called by the framework immediately after your ${artifactType} registers with the NameService.
        
        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()
        """
        ${baseClass}.initialize(self)
        # TODO add customization here.
        
#{% if component is device %}
    def updateUsageState(self):
        """
        This is called automatically after allocateCapacity or deallocateCapacity are called.
        Your implementation should determine the current state of the device:
           self._usageState = CF.Device.IDLE   # not in use
           self._usageState = CF.Device.ACTIVE # in use, with capacity remaining for allocation
           self._usageState = CF.Device.BUSY   # in use, with no capacity remaining for allocation
        """
        return NOOP

#{% endif %}

    def process(self):
        """
        Basic functionality:
        
            The process method should process a single "chunk" of data and then return. This method
            will be called from the processing thread again, and again, and again until it returns
            FINISH or stop() is called on the ${artifactType}.  If no work is performed, then return NOOP.
            
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
            
            Interactions with non-BULKIO ports are left up to the ${artifactType} developer's discretion.
            
        Properties:
        
            Properties are accessed directly as member variables. If the property name is baudRate,
            then accessing it (for reading or writing) is achieved in the following way: self.baudRate.
            
        Example:
        
            # This example assumes that the ${artifactType} has two ports:
            #   - A provides (input) port of type bulkio.InShortPort called dataShort_in
            #   - A uses (output) port of type bulkio.OutFloatPort called dataFloat_out
            # The mapping between the port and the class if found in the ${artifactType}
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

#{% if component.isafrontendtuner %}
######################################################
##### tuner configuration functions ## -- overrides base class implementations
######################################################

    def removeTuner(self, tuner_id):
removeTuner(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return BOOL_VALUE_HERE

######################################################
##### Required device specific functions ## -- implemented by device developer
######################################################

    def push_EOS_on_listener(self,listener_allocation_id):
push_EOS_on_listener(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return BOOL_VALUE_HERE;

    def _valid_tuner_type(self,tuner_type):
_valid_tuner_type(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return BOOL_VALUE_HERE;

    def _valid_center_frequency(self,req_freq, tuner_id):
_valid_center_frequency(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return BOOL_VALUE_HERE;

    def _valid_bandwidth(self,req_bw, tuner_id):
_valid_bandwidth(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return BOOL_VALUE_HERE;

    def _valid_sample_rate(self, req_sr, tuner_id):
_valid_sample_rate(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return BOOL_VALUE_HERE;

    def _dev_enable(self, tuner_id):
_dev_enable(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return BOOL_VALUE_HERE;

    def _dev_disable(self, tuner_id):
_dev_disable(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return BOOL_VALUE_HERE;

    def _dev_set_all(self, req_freq, req_bw, req_sr, tuner_id):
_dev_set_all(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return BOOL_VALUE_HERE;

    def _dev_set_center_frequency(self, req_freq, tuner_id):
_dev_set_center_frequency(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return BOOL_VALUE_HERE;

    def _dev_set_bandwidth(self, req_bw, tuner_id):
_dev_set_bandwidth(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return BOOL_VALUE_HERE;

    def _dev_set_sample_rate(self, req_sr, tuner_id):
_dev_set_sample_rate(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return BOOL_VALUE_HERE;

    def _dev_get_all(self, freq, bw, sr, tuner_id):
_dev_get_all(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return BOOL_VALUE_HERE;

    def _dev_get_center_frequency(self, tuner_id):
_dev_get_center_frequency(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return DOUBLE_VALUE_HERE;

    def _dev_get_bandwidth(self, tuner_id):
_dev_get_bandwidth(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return DOUBLE_VALUE_HERE;

    def _dev_get_sample_rate(self, tuner_id):
_dev_get_sample_rate(): DEVELOPER MUST IMPLEMENT THIS METHOD  *********
        return DOUBLE_VALUE_HERE;
#{% endif %}

#{% set foundInFrontendInterface = False %}
#{% set foundInAnalogInterface = False %}
#{% set foundInDigitalInterface = False %}
#{% set foundInGPSPort = False %}
#{% set foundInNavDataPort = False %}
#{% set foundInRFInfoPort = False %}
#{% set foundInRFSourcePort = False %}
#{% for port in component.ports if port is provides %}
#{%     if port.cpptype == "frontend.InDigitalTunerPort" or
            port.cpptype == "frontend.InAnalogTunerPort" or
            port.cpptype == "frontend.InFrontendTunerPort" %}
    #{#- add FEI FrontendTuner callback functions #}
#{%         if foundInFrontendInterface == False %}
#        def fe_getTunerType(self, id):
#            "fe_getTunerType(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_getTunerDeviceControl(self, id):
#            "fe_getTunerDeviceControl(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_getTunerGroupId(self, id):
#            "fe_getTunerGroupId(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_getTunerRfFlowId(self, id):
#            "fe_getTunerRfFlowId(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_getTunerStatus(self, id):
#            "fe_getTunerStatus(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }
#{%             set foundInFrontendInterface = True %}
#{%         endif %}
#{%     endif %}
#{%     if port.cpptype == "frontend.InDigitalTunerPort" or
            port.cpptype == "frontend.InAnalogTunerPort" %}
    #{#- add FEI AnalogTuner callback functions #}
#{%         if foundInAnalogInterface == False %}
#        def fe_getTunerCenterFrequency(self, id):
#            "fe_getTunerCenterFrequency(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setTunerCenterFrequency(self, id, freq):
#            "fe_setTunerCenterFrequency(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_getTunerBandwidth(self, id){
#            "fe_getTunerBandwidth(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setTunerBandwidth(self, id, bw){
#            "fe_setTunerBandwidth(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_getTunerAgcEnable(self, id){
#            "fe_getTunerAgcEnable(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setTunerAgcEnable(self, id, enable){
#            "fe_setTunerAgcEnable(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_getTunerGain(self, id){
#            "fe_getTunerGain(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setTunerGain(self, id, gain){
#            "fe_setTunerGain(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_getTunerReferenceSource(self, id){
#            "fe_getTunerReferenceSource(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_getTunerAgcEnable(self, id){
#            "fe_getTunerAgcEnable(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setTunerAgcEnable(self, id, enable){
#            "fe_setTunerAgcEnable(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_getTunerGain(self, id){
#            "fe_getTunerGain(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setTunerGain(self, id, gain){
#            "fe_setTunerGain(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_getTunerReferenceSource(self, id){
#            "fe_getTunerReferenceSource(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setTunerReferenceSource(self, id, source){
#            "fe_setTunerReferenceSource(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_getTunerEnable(self, id){
#            "fe_getTunerEnable(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setTunerEnable(self, id, enable){
#            "fe_setTunerEnable(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }
#{%             set foundInAnalogInterface = True %}
#{%         endif %}
#{% endif %}
#{%     if port.cpptype == "frontend.InDigitalTunerPort" %}
    #{#- add FEI DigitalTuner callback functions #}
#{%         if foundInDigitalInterface == False %}
#        def fe_getTunerOutputSampleRate(self, id){
#            "fe_getTunerOutputSampleRate(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setTunerOutputSampleRate(self, id, sr){
#            "fe_setTunerOutputSampleRate(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }
#{%             set foundInDigitalInterface = True %}
#{%         endif %}
#{%     endif %}
#{%     if port.cpptype == "frontend.InGPSPort" %}
#{%         if foundInGPSPort == False %}
#        def fe_getGPSInfo(self){
#            "fe_getGPSInfo(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setGPSInfo(self,data){
#            "fe_setGPSInfo(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_getGpsTimePos(self){
#            "fe_getGpsTimePos(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setGpsTimePos(self,data){
#            "fe_setGpsTimePos(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }
#{%             set foundInGPSPort = True %}
#{%         endif %}
#{%     endif %}
#{%     if port.cpptype == "frontend.InNavDataPort" %}
#{%         if foundInNavDataPort == False %}
#        def fe_getNavPkt(self){
#            "fe_getNavPkt(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setNavPkt(self,data){
#            "fe_setNavPkt(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }
#{%             set foundInNavDataPort = True %}
#{%         endif %}
#{%     endif %}
#{%     if port.cpptype == "frontend.InRFInfoPort" %}
#{%         if foundInRFInfoPort == False %}
#        def fe_getRFFlowId(self){
#            "fe_getRFFlowId(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setRFFlowId(self,data){
#            "fe_setRFFlowId(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_getRFInfoPkt(self){
#            "fe_getRFInfoPkt(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setRFInfoPkt(self, data){
#            "fe_setRFInfoPkt(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }
#{%             set foundInRFInfoPort = True %}
#{%         endif %}
#{%     endif %}
#{%     if port.cpptype == "frontend.InRFSourcePort" %}
#{%         if foundInRFSourcePort == False %}
#        def fe_getAvailableRFInputs(self){
#            "fe_getAvailableRFInputs(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setAvailableRFInputs(self,data){
#            "fe_setAvailableRFInputs(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_getCurrentRFInput(self){
#            "fe_getCurrentRFInput(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }

#        def fe_setCurrentRFInput(self, data){
#            "fe_setCurrentRFInput(): DEVELOPER CAN IMPLEMENT TO OVERRIDE BASE METHOD"
#        }
#{%             set foundInRFSourcePort = True %}
#{%         endif %}
#{%     endif %}
#{% endfor %}

        
  
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.INFO)
#{% if component is device %}
    logging.debug("Starting Device")
    start_device(${className})
#{% else %}
    logging.debug("Starting Component")
    start_component(${className})
#{% endif %}
