#!/usr/bin/env python
#
#
# AUTO-GENERATED
#
# Source: rf_ctrl.spd.xml
from ossie.resource import start_component
import logging
from ossie.resource import PortCallError
from bulkio.bulkioInterfaces import BULKIO

from rf_ctrl_base import *

class rf_ctrl_i(rf_ctrl_base):
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
            self.port_input.registerMessage("my_msg", rf_ctrl_i.MyMsg, self.msg_callback)
        
            To send a message, you need to (1) create a message structure, and (2) send the message over the port.
        
            Assuming a property of type message is declared called "my_msg", an output port called "msg_output" is declared of
            type MessageEvent, create the following code:
        
            msg_out = rf_ctrl_i.MyMsg()
            self.port_msg_output.sendMessage(msg_out)

    Accessing the Device Manager and Domain Manager:
    
        Both the Device Manager hosting this Device and the Domain Manager hosting
        the Device Manager are available to the Device.
        
        To access the Domain Manager:
            dommgr = self.getDomainManager().getRef()
        To access the Device Manager:
            devmgr = self.getDeviceManager().getRef()
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
            # The mapping between the port and the class is found in the component
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
        self.get_rfinfo = "ok"
        try:
            self.port_rfinfo_out.rf_flow_id()
        except PortCallError, e:
            self.get_rfinfo = str(e)
        
        self.set_rfinfo = "ok"
        try:
            rf_flow_id = "hello"
            self.port_rfinfo_out._set_rf_flow_id(rf_flow_id)
        except PortCallError, e:
            self.set_rfinfo = str(e)
        
        self.get_current_rf = "ok"
        try:
            self.port_rfsource_out.current_rf_input()
        except PortCallError, e:
            self.get_current_rf = str(e)
        
        self.set_current_rf = "ok"
        try:
            _antennainfo=FRONTEND.AntennaInfo('','','','')
            _freqrange=FRONTEND.FreqRange(0,0,[])
            _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
            _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
            _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange)
            foo = FRONTEND.RFInfoPkt('',0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
            self.port_rfsource_out._set_current_rf_input(foo)
        except PortCallError, e:
            self.set_current_rf = str(e)
        
        self.get_available_rf = "ok"
        try:
            self.port_rfsource_out.available_rf_inputs()
        except PortCallError, e:
            self.get_available_rf = str(e)
        
        self.set_available_rf = "ok"
        try:
            foo = []
            self.port_rfsource_out._set_available_rf_inputs(foo)
        except PortCallError, e:
            self.set_available_rf = str(e)
        
        self.bad_connection = "ok"
        try:
            self.port_rfsource_out._get_available_rf_inputs("invalid_connectionid")
        except PortCallError, e:
            self.bad_connection = str(e)
        
    
        tmp = ''
        self.get_tunertype = "ok"
        try:
            self.port_digitaltuner_out.getTunerType(tmp)
        except PortCallError, e:
            self.get_tunertype = str(e)
        
        self.get_tunerdevicecontrol = "ok"
        try:
            self.port_digitaltuner_out.getTunerDeviceControl(tmp)
        except PortCallError, e:
            self.get_tunerdevicecontrol = str(e)
        
        self.get_tunergroupid = "ok"
        try:
            self.port_digitaltuner_out.getTunerGroupId(tmp)
        except PortCallError, e:
            self.get_tunergroupid = str(e)
        
        self.get_tunerrfflowid = "ok"
        try:
            self.port_digitaltuner_out.getTunerRfFlowId(tmp)
        except PortCallError, e:
            self.get_tunerrfflowid = str(e)
        
        self.get_tunerstatus = "ok"
        try:
            self.port_digitaltuner_out.getTunerStatus(tmp)
        except PortCallError, e:
            self.get_tunerstatus = str(e)
        
        self.get_tunercenterfrequency = "ok"
        try:
            self.port_digitaltuner_out.getTunerCenterFrequency(tmp)
        except PortCallError, e:
            self.get_tunercenterfrequency = str(e)
        
        self.set_tunercenterfrequency = "ok"
        try:
            self.port_digitaltuner_out.setTunerCenterFrequency(tmp, 1.0)
        except PortCallError, e:
            self.set_tunercenterfrequency = str(e)
        
        self.get_tunerbandwidth = "ok"
        try:
            self.port_digitaltuner_out.getTunerBandwidth(tmp)
        except PortCallError, e:
            self.get_tunerbandwidth = str(e)
        
        self.set_tunerbandwidth = "ok"
        try:
            self.port_digitaltuner_out.setTunerBandwidth(tmp, 1.0)
        except PortCallError, e:
            self.set_tunerbandwidth = str(e)
        
        self.get_tuneragcenable = "ok"
        try:
            self.port_digitaltuner_out.getTunerAgcEnable(tmp)
        except PortCallError, e:
            self.get_tuneragcenable = str(e)
        
        self.set_tuneragcenable = "ok"
        try:
            self.port_digitaltuner_out.setTunerAgcEnable(tmp, False)
        except PortCallError, e:
            self.set_tuneragcenable = str(e)
        
        self.get_tunergain = "ok"
        try:
            self.port_digitaltuner_out.getTunerGain(tmp)
        except PortCallError, e:
            self.get_tunergain = str(e)
        
        self.set_tunergain = "ok"
        try:
            self.port_digitaltuner_out.setTunerGain(tmp, 1.0)
        except PortCallError, e:
            self.set_tunergain = str(e)
        
        self.get_tunerreferencesource = "ok"
        try:
            self.port_digitaltuner_out.getTunerReferenceSource(tmp)
        except PortCallError, e:
            self.get_tunerreferencesource = str(e)
        
        self.set_tunerreferencesource = "ok"
        try:
            self.port_digitaltuner_out.setTunerReferenceSource(tmp, 2)
        except PortCallError, e:
            self.set_tunerreferencesource = str(e)
        
        self.get_tunerenable = "ok"
        try:
            self.port_digitaltuner_out.getTunerEnable(tmp)
        except PortCallError, e:
            self.get_tunerenable = str(e)
        
        self.set_tunerenable = "ok"
        try:
            self.port_digitaltuner_out.setTunerEnable(tmp, False)
        except PortCallError, e:
            self.set_tunerenable = str(e)
        
        self.get_tuneroutputsamplerate = "ok"
        try:
            self.port_digitaltuner_out.getTunerOutputSampleRate(tmp)
        except PortCallError, e:
            self.get_tuneroutputsamplerate = str(e)
        
        self.set_tuneroutputsamplerate = "ok"
        try:
            self.port_digitaltuner_out.setTunerOutputSampleRate(tmp, 1.0)
        except PortCallError, e:
            self.set_tuneroutputsamplerate = str(e)
        
    
        self.get_gpsinfo = "ok"
        try:
            self.port_gps_out.gps_info()
        except PortCallError, e:
            self.get_gpsinfo = str(e)
        
        self.set_gpsinfo = "ok"
        try:
            _gps = FRONTEND.GPSInfo('','','',1L,1L,1L,1.0,1.0,1.0,1.0,1,1.0,'',BULKIO.PrecisionUTCTime(1,1,1.0,1.0,1.0),[])
            self.port_gps_out._set_gps_info(_gps)
        except PortCallError, e:
            self.set_gpsinfo = str(e)
        
        self.get_gps_timepos = "ok"
        try:
            self.port_gps_out.gps_time_pos()
        except PortCallError, e:
            self.get_gps_timepos = str(e)
        
        self.set_gps_timepos = "ok"
        try:
            _positioninfo = FRONTEND.PositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
            _gps = FRONTEND.GpsTimePos(_positioninfo,BULKIO.PrecisionUTCTime(1,1,1.0,1.0,1.0))
            self.port_gps_out._set_gps_time_pos(_gps)
        except PortCallError, e:
            self.set_gps_timepos = str(e)
        
    
        self.get_nav_packet = "ok"
        try:
            self.port_navdata_out.nav_packet()
        except PortCallError, e:
            self.get_nav_packet = str(e)
        
        self.set_nav_packet = "ok"
        try:
            _time = BULKIO.PrecisionUTCTime(1,1,1.0,1.0,1.0)
            _positioninfo = FRONTEND.PositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
            _cartesianpos=FRONTEND.CartesianPositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
            _velocityinfo=FRONTEND.VelocityInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
            _accelerationinfo=FRONTEND.AccelerationInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
            _attitudeinfo=FRONTEND.AttitudeInfo(False,0.0,0.0,0.0)
            _nav = FRONTEND.NavigationPacket('','',_positioninfo,_cartesianpos,_velocityinfo,_accelerationinfo,_attitudeinfo,_time,[])
            self.port_navdata_out._set_nav_packet(_nav)
        except PortCallError, e:
            self.set_nav_packet = str(e)
        
        return NOOP

  
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.INFO)
    logging.debug("Starting Component")
    start_component(rf_ctrl_i)

