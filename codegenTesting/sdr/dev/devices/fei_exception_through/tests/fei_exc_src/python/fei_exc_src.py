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
# Source: fei_exc_src.spd.xml
from ossie.device import start_device
import logging

from fei_exc_src_base import *

class fei_exc_src_i(fei_exc_src_base):
    """<DESCRIPTION GOES HERE>"""
    def constructor(self):
        """
        This is called by the framework immediately after your device registers with the system.
        
        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()

        For a tuner device, the structure frontend_tuner_status needs to match the number
        of tuners that this device controls and what kind of device it is.
        The options for devices are: TX, RX, RX_DIGITIZER, CHANNELIZER, DDC, RC_DIGITIZER_CHANNELIZER
     
        For example, if this device has 5 physical
        tuners, each an RX_DIGITIZER, then the code in the construct function should look like this:

        self.setNumChannels(5, "RX_DIGITIZER")
     
        The incoming request for tuning contains a string describing the requested tuner
        type. The string for the request must match the string in the tuner status.
        """
        # TODO add customization here.
        self.setNumChannels(1, "RX_DIGITIZER")
        self.rf_info_list=None
        self.rf_idx=0
        
    def process(self):
        """
        Basic functionality:
        
            The process method should process a single "chunk" of data and then return. This method
            will be called from the processing thread again, and again, and again until it returns
            FINISH or stop() is called on the device.  If no work is performed, then return NOOP.
            
        StreamSRI:
            To create a StreamSRI object, use the following code (this generates a normalized SRI that does not flush the queue when full):
                self.sri = bulkio.sri.create(self.stream_id)

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
            self.port_input.registerMessage("my_msg", fei_exc_src_i.MyMsg, self.msg_callback)
        
            To send a message, you need to (1) create a message structure, and (2) send the message over the port.
        
            Assuming a property of type message is declared called "my_msg", an output port called "msg_output" is declared of
            type MessageEvent, create the following code:
        
            msg_out = fei_exc_src_i.MyMsg()
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
                self.port_dataFloat_out.pushSRI(packet.SRI)

            self.port_dataFloat_out.pushPacket(outData, packet.T, packet.EOS, packet.streamID)
            return NORMAL
            
        """

        # TODO fill in your code here
        self._log.debug("process() example log message")
        return NOOP

    '''
    *************************************************************
    Functions supporting tuning allocation
    *************************************************************'''
    def deviceEnable(self, fts, tuner_id):
        '''
        ************************************************************
        modify fts, which corresponds to self.frontend_tuner_status[tuner_id]
        Make sure to set the 'enabled' member of fts to indicate that tuner as enabled
        ************************************************************'''
        print "deviceEnable(): Enable the given tuner  *********"
        fts.enabled = True
        return

    def deviceDisable(self,fts, tuner_id):
        '''
        ************************************************************
        modify fts, which corresponds to self.frontend_tuner_status[tuner_id]
        Make sure to reset the 'enabled' member of fts to indicate that tuner as disabled
        ************************************************************'''
        print "deviceDisable(): Disable the given tuner  *********"
        fts.enabled = False
        return

    def deviceSetTuning(self,request, fts, tuner_id):
        '''
        ************************************************************
        modify fts, which corresponds to self.frontend_tuner_status[tuner_id]
        
        The bandwidth, center frequency, and sampling rate that the hardware was actually tuned
        to needs to populate fts (to make sure that it meets the tolerance requirement. For example,
        if the tuned values match the requested values, the code would look like this:
        
        fts.bandwidth = request.bandwidth
        fts.center_frequency = request.center_frequency
        fts.sample_rate = request.sample_rate
        
        return True if the tuning succeeded, and False if it failed
        ************************************************************'''
        print "deviceSetTuning(): Evaluate whether or not a tuner is added  *********"
        return True

    def deviceDeleteTuning(self, fts, tuner_id):
        '''
        ************************************************************
        modify fts, which corresponds to self.frontend_tuner_status[tuner_id]
        return True if the tune deletion succeeded, and False if it failed
        ************************************************************'''
        print "deviceDeleteTuning(): Deallocate an allocated tuner  *********"
        return True

    '''
    *************************************************************
    Functions servicing the tuner control port
    *************************************************************'''
    def getScanStatus(self, id):
        _scan_strategy=FRONTEND.ScanningTuner.ScanStrategy(FRONTEND.ScanningTuner.MANUAL_SCAN, FRONTEND.ScanningTuner.ScanModeDefinition(center_frequency=1.0), FRONTEND.ScanningTuner.TIME_BASED, 123.0)
        _scan_status=FRONTEND.ScanningTuner.ScanStatus(_scan_strategy,
                                           start_time=bulkio.timestamp.now(),
                                           center_tune_frequencies=[],
                                           started=False)
        return _scan_status

    def setScanStartTime(self, id, start_time):
        pass

    def setScanStrategy(self, id, scan_strategy):
        pass

    def getTunerType(self,allocation_id):
        return ""

    def getTunerDeviceControl(self,allocation_id):
        return False

    def getTunerGroupId(self,allocation_id):
        return ""

    def getTunerRfFlowId(self,allocation_id):
        return ""

    def setTunerCenterFrequency(self,allocation_id, freq):
        pass

    def getTunerCenterFrequency(self,allocation_id):
        return 1.0

    def setTunerBandwidth(self,allocation_id, bw):
        pass

    def getTunerBandwidth(self,allocation_id):
        return 1.0

    def setTunerAgcEnable(self,allocation_id, enable):
        pass

    def getTunerAgcEnable(self,allocation_id):
        return False

    def setTunerGain(self,allocation_id, gain):
        pass

    def getTunerGain(self,allocation_id):
        return 1.0

    def setTunerReferenceSource(self,allocation_id, source):
        pass

    def getTunerReferenceSource(self,allocation_id):
        return 1L

    def setTunerEnable(self,allocation_id, enable):
        pass

    def getTunerEnable(self,allocation_id):
        return False

    def setTunerOutputSampleRate(self,allocation_id, sr):
        pass

    def getTunerOutputSampleRate(self,allocation_id):
        return 1.0

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

    '''
    *************************************************************
    Functions servicing the RFInfo port(s)
    - port_name is the port over which the call was received
    *************************************************************'''
    def get_rf_flow_id(self,port_name):
        return ""

    def set_rf_flow_id(self,port_name, id):
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

    '''
    *************************************************************
    Functions servicing the RFInfo port(s)
    - port_name is the port over which the call was received
    *************************************************************
    '''
    def make_rfinfo(self,freq):
        _antennainfo = FRONTEND.AntennaInfo('','','','')
        _freqrange = FRONTEND.FreqRange(0,0,[])
        _feedinfo = FRONTEND.FeedInfo('','',_freqrange)
        _sensorinfo = FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
        _rfcapabilities = FRONTEND.RFCapabilities(_freqrange,_freqrange)
        _rfinfopkt = FRONTEND.RFInfoPkt('',0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
        _rfinfopkt.rf_center_freq=freq;
        _rfinfopkt.rf_bandwidth=freq/2.0;
        return _rfinfopkt

    def get_available_rf_inputs(self,port_name):
        if not self.rf_info_list:
            self.rf_info_list=[]
            for x in range(10):
                rf=self.make_rfinfo(( (x+1)*100.0))
                self.rf_info_list.append( rf )

        return self.rf_info_list

    def set_available_rf_inputs(self,port_name, inputs):
        pass

    def get_current_rf_input(self,port_name):
        _ret=self.rf_info_list[ self.rf_idx ]
        self.rf_idx  = ( self.rf_idx + 1) % size(self.rf_info_list)
        return _ret

    def set_current_rf_input(self, port_name, pkt):
        pass

  
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.INFO)
    logging.debug("Starting Device")
    start_device(fei_exc_src_i)

