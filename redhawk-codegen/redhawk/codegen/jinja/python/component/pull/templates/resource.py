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
#% block license
#% endblock
#
# AUTO-GENERATED
#
# Source: ${component.profile.spd}
#{% if component is device %}
from ossie.device import start_device
#{% else %}
from ossie.resource import start_component
#{% endif %}
import logging

from ${baseClass} import *

class ${className}(${baseClass}):
#{% if component.description %}
    """${component.description()}"""
#{% else %}
    """<DESCRIPTION GOES HERE>"""
#{% endif %}
    def constructor(self):
        """
        This is called by the framework immediately after your ${artifactType} registers with the system.
        
        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()

#{% if component.hastunerstatusstructure %}
        For a tuner device, the structure frontend_tuner_status needs to match the number
        of tuners that this device controls and what kind of device it is.
        The options for devices are: TX, RX, RX_DIGITIZER, CHANNELIZER, DDC, RX_DIGITIZER_CHANNELIZER
     
        For example, if this device has 5 physical
        tuners, 3 RX_DIGITIZER and 2 CHANNELIZER, then the code in the construct function 
        should look like this:

        self.addChannels(3, "RX_DIGITIZER")
        self.addChannels(2, "CHANNELIZER")
     
        The incoming request for tuning contains a string describing the requested tuner
        type. The string for the request must match the string in the tuner status.
#{% endif %}
        """
        # TODO add customization here.
#{% if component.hastunerstatusstructure %}
        self.addChannels(1, "RX_DIGITIZER")
#{% endif %}
        
#{% block updateUsageState %}
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
#{% endblock %}
    def process(self):
        """
        Basic functionality:
        
            The process method should process a single "chunk" of data and then return. This method
            will be called from the processing thread again, and again, and again until it returns
            FINISH or stop() is called on the ${artifactType}.  If no work is performed, then return NOOP.
            
        StreamSRI:
            To create a StreamSRI object, use the following code (this generates a normalized SRI that does not flush the queue when full):
                sri = bulkio.sri.create("my_stream_id")
#{% if component is device %}
#{% if 'FrontendTuner' in component.implements %}

            To create a StreamSRI object based on tuner status structure index 'idx' and collector center frequency of 100:
                sri = frontend.sri.create("my_stream_id", self.frontend_tuner_status[idx], self._id, collector_frequency=100)
#{% endif %}
#{% endif %}

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

            Interactions with non-BULKIO ports are left up to the ${artifactType} developer's discretion.

        Messages:
    
            To receive a message, you need (1) an input port of type MessageEvent, (2) a message prototype described
            as a structure property of kind message, (3) a callback to service the message, and (4) to register the callback
            with the input port.
        
            Assuming a property of type message is declared called "my_msg", an input port called "msg_input" is declared of
            type MessageEvent, create the following code:
        
            def msg_callback(self, msg_id, msg_value):
                print msg_id, msg_value
        
            Register the message callback onto the input port with the following form:
            self.port_input.registerMessage("my_msg", ${className}.MyMsg, self.msg_callback)
        
            To send a message, you need to (1) create a message structure, and (2) send the message over the port.
        
            Assuming a property of type message is declared called "my_msg", an output port called "msg_output" is declared of
            type MessageEvent, create the following code:
        
            msg_out = ${className}.MyMsg()
            self.port_msg_output.sendMessage(msg_out)

#{% if component is device %}
    Accessing the Application and Domain Manager:
    
        Both the Application hosting this Component and the Domain Manager hosting
        the Application are available to the Component.
        
        To access the Domain Manager:
            dommgr = self.getDomainManager().getRef()
        To access the Application:
            app = self.getApplication().getRef()
#{% else %}
    Accessing the Device Manager and Domain Manager:
    
        Both the Device Manager hosting this Device and the Domain Manager hosting
        the Device Manager are available to the Device.
        
        To access the Domain Manager:
            dommgr = self.getDomainManager().getRef()
        To access the Device Manager:
            devmgr = self.getDeviceManager().getRef()
#{% endif %}
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
            
#{% if component is device %}
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
#{% endif %}
            
        Example:
        
            # This example assumes that the ${artifactType} has two ports:
            #   - A provides (input) port of type bulkio.InShortPort called dataShort_in
            #   - A uses (output) port of type bulkio.OutFloatPort called dataFloat_out
            # The mapping between the port and the class is found in the ${artifactType}
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

        # TODO fill in your code here
        self._baseLog.debug("process() example log message")
        return NOOP

#{% block fei_port_delegations %}
#{% if 'FrontendTuner' in component.implements %}
    '''
    *************************************************************
    Functions servicing the tuner control port
    *************************************************************'''
    def getTunerType(self,allocation_id):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("getTunerType not supported")

    def getTunerDeviceControl(self,allocation_id):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("getTunerDeviceControl not supported")

    def getTunerGroupId(self,allocation_id):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("getTunerGroupId not supported")

    def getTunerRfFlowId(self,allocation_id):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("getTunerRfFlowId not supported")

#{% endif %}
#{% if 'AnalogTuner' in component.implements %}

    def setTunerCenterFrequency(self,allocation_id, freq):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("setTunerCenterFrequency not supported")

    def getTunerCenterFrequency(self,allocation_id):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("getTunerCenterFrequency not supported")

    def setTunerBandwidth(self,allocation_id, bw):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("setTunerBandwidth not supported")

    def getTunerBandwidth(self,allocation_id):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("getTunerBandwidth not supported")

    def setTunerAgcEnable(self,allocation_id, enable):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("setTunerAgcEnable not supported")

    def getTunerAgcEnable(self,allocation_id):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("getTunerAgcEnable not supported")

    def setTunerGain(self,allocation_id, gain):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("setTunerGain not supported")

    def getTunerGain(self,allocation_id):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("getTunerGain not supported")

    def setTunerReferenceSource(self,allocation_id, source):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("setTunerReferenceSource not supported")

    def getTunerReferenceSource(self,allocation_id):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("getTunerReferenceSource not supported")

    def setTunerEnable(self,allocation_id, enable):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("setTunerEnable not supported")

    def getTunerEnable(self,allocation_id):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("getTunerEnable not supported")

#{% endif %}
#{% if 'DigitalTuner' in component.implements %}

    def setTunerOutputSampleRate(self,allocation_id, sr):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("setTunerOutputSampleRate not supported")

    def getTunerOutputSampleRate(self,allocation_id):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("getTunerOutputSampleRate not supported")

#{% endif %}
#{% if 'ScanningTuner' in component.implements %}

    def getScanStatus(self, allocation_id):
        # Example of how to build a ScanStatus data structure
        #_scan_strategy=FRONTEND.ScanningTuner.ScanStrategy(
        #    FRONTEND.ScanningTuner.MANUAL_SCAN, 
        #    FRONTEND.ScanningTuner.ScanModeDefinition(center_frequency=1.0), 
        #    FRONTEND.ScanningTuner.TIME_BASED, 
        #    0.0)
        #_scan_status=FRONTEND.ScanningTuner.ScanStatus(_scan_strategy,
        #                                   start_time=bulkio.timestamp.now(),
        #                                   center_tune_frequencies=[],
        #                                   started=False)
        raise FRONTEND.NotSupportedException("getScanStatus not supported")

    def setScanStartTime(self, allocation_id, start_time):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("setScanStartTime not supported")

    def setScanStrategy(self, allocation_id, scan_strategy):
        # WARNING: this device does not contain tuner allocation/status structures
        #          allocation_id has no meaning
        raise FRONTEND.NotSupportedException("setScanStrategy not supported")

#{% endif %}
#{% if 'GPS' in component.implements %}

    def get_gps_info(self,port_name):
        _time = bulkio.timestamp.now()
        _gpsinfo = FRONTEND.GPSInfo('','','',1L,1L,1L,1.0,1.0,1.0,1.0,1,1.0,'',_time,[])
        return _gpsinfo

    def set_gps_info(self,port_name, gps_info):
        pass

    def get_gps_time_pos(self,port_name):
        _time = bulkio.timestamp.now()
        _positioninfo = FRONTEND.PositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
        _gpstimepos = FRONTEND.GpsTimePos(_positioninfo,_time)
        return _gpstimepos

    def set_gps_time_pos(self,port_name, gps_time_pos):
        pass
#{% endif %}
#{% if 'NavData' in component.implements %}

    def get_nav_packet(self,port_name):
        _time = bulkio.timestamp.now()
        _positioninfo = FRONTEND.PositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
        _cartesianpos = FRONTEND.CartesianPositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
        _velocityinfo = FRONTEND.VelocityInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
        _accelerationinfo = FRONTEND.AccelerationInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
        _attitudeinfo = FRONTEND.AttitudeInfo(False,0.0,0.0,0.0)
        _navpacket = FRONTEND.NavigationPacket('','',_positioninfo,_cartesianpos,_velocityinfo,_accelerationinfo,_attitudeinfo,_time,[])
        return _navpacket

    def set_nav_packet(self,port_name, nav_info):
        pass

#{% endif %}
#{% if 'RFInfo' in component.implements %}
    '''
    *************************************************************
    Functions servicing the RFInfo port(s)
    - port_name is the port over which the call was received
    *************************************************************'''
    def get_rf_flow_id(self,port_name):
        return ''

    def set_rf_flow_id(self,port_name, _id):
        pass

    def get_rfinfo_pkt(self,port_name):
        _antennainfo=FRONTEND.AntennaInfo('','','','')
        _freqrange=FRONTEND.FreqRange(0,0,[])
        _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
        _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
        _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange)
        _rfinfopkt=FRONTEND.RFInfoPkt('',0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
        return _rfinfopkt

    def set_rfinfo_pkt(self,port_name, pkt):
        pass
#{% endif %}
#{% if 'RFSource' in component.implements %}

    def get_available_rf_inputs(self,port_name):
        return []

    def set_available_rf_inputs(self,port_name, inputs):
        pass

    def get_current_rf_input(self,port_name):
        _antennainfo = FRONTEND.AntennaInfo('','','','')
        _freqrange = FRONTEND.FreqRange(0,0,[])
        _feedinfo = FRONTEND.FeedInfo('','',_freqrange)
        _sensorinfo = FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
        _rfcapabilities = FRONTEND.RFCapabilities(_freqrange,_freqrange)
        _rfinfopkt = FRONTEND.RFInfoPkt('',0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
        return _rfinfopkt

    def set_current_rf_input(self, port_name, pkt):
        pass
#{% endif %}
#{% endblock %}
#{% block extensions %}
#{% endblock %}
  
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.INFO)
#{% if component is device %}
    logging.debug("Starting Device")
    start_device(${className})
#{% else %}
    logging.debug("Starting Component")
    start_component(${className})
#{% endif %}
