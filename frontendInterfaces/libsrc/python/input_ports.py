#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK frontendInterfaces.
#
# REDHAWK frontendInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK frontendInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import threading
from redhawk.frontendInterfaces import FRONTEND__POA
from redhawk.frontendInterfaces import FRONTEND
from bulkio.bulkioInterfaces import BULKIO
import copy


'''provides port(s)'''

class tuner_delegation(object):
    def getTunerType(self, id):
        raise FRONTEND.NotSupportedException("getTunerType not supported")
    def getTunerDeviceControl(self, id):
        raise FRONTEND.NotSupportedException("getTunerDeviceControl not supported")
    def getTunerGroupId(self, id):
        raise FRONTEND.NotSupportedException("getTunerGroupId not supported")
    def getTunerRfFlowId(self, id):
        raise FRONTEND.NotSupportedException("getTunerRfFlowId not supported")
    def getTunerStatus(self, id):
        raise FRONTEND.NotSupportedException("getTunerStatus not supported")

class analog_tuner_delegation(tuner_delegation):
    def setTunerCenterFrequency(self, id, freq):
        raise FRONTEND.NotSupportedException("setTunerCenterFrequency not supported")
    def getTunerCenterFrequency(self, id):
        raise FRONTEND.NotSupportedException("getTunerCenterFrequency not supported")
    def setTunerBandwidth(self, bw):
        raise FRONTEND.NotSupportedException("setTunerBandwidth not supported")
    def getTunerBandwidth(self, id):
        raise FRONTEND.NotSupportedException("getTunerBandwidth not supported")
    def setTunerAgcEnable(self, id, enable):
        raise FRONTEND.NotSupportedException("setTunerAgcEnable not supported")
    def getTunerAgcEnable(self, id):
        raise FRONTEND.NotSupportedException("getTunerAgcEnable not supported")
    def setTunerGain(self, id,gain):
        raise FRONTEND.NotSupportedException("setTunerGain not supported")
    def getTunerGain(self, id):
        raise FRONTEND.NotSupportedException("getTunerGain not supported")
    def setTunerReferenceSource(self, id, source):
        raise FRONTEND.NotSupportedException("setTunerReferenceSource not supported")
    def getTunerReferenceSource(self, id):
        raise FRONTEND.NotSupportedException("getTunerReferenceSource not supported")
    def setTunerEnable(self, id, enable):
        raise FRONTEND.NotSupportedException("setTunerEnable not supported")
    def getTunerEnable(self, id):
        raise FRONTEND.NotSupportedException("getTunerEnable not supported")

class digital_tuner_delegation(analog_tuner_delegation):
    def setTunerOutputSampleRate(self, id, sr):
        raise FRONTEND.NotSupportedException("setTunerOutputSampleRate not supported")
    def getTunerOutputSampleRate(self, id):
        raise FRONTEND.NotSupportedException("getTunerOutputSampleRate not supported")

class InFrontendTunerPort(FRONTEND__POA.FrontendTuner):
    def __init__(self, name, parent=tuner_delegation()):
        self.name = name
        self.port_lock = threading.Lock()
        self.parent = parent

    def getTunerType(self, id):
        self.port_lock.acquire()
        try:
            return self.parent.getTunerType(id)
        finally:
            self.port_lock.release()

    def getTunerDeviceControl(self, id):
        self.port_lock.acquire()
        try:
            return self.parent.getTunerDeviceControl(id)
        finally:
            self.port_lock.release()

    def getTunerGroupId(self, id):
        self.port_lock.acquire()
        try:
            return self.parent.getTunerGroupId(id)
        finally:
            self.port_lock.release()

    def getTunerRfFlowId(self, id):
        self.port_lock.acquire()
        try:
            return self.parent.getTunerRfFlowId(id)
        finally:
            self.port_lock.release()

    def getTunerStatus(self, id):
        self.port_lock.acquire()
        try:
            return self.parent.getTunerStatus(id)
        finally:
            self.port_lock.release()

class InAnalogTunerPort(FRONTEND__POA.AnalogTuner, InFrontendTunerPort):
    def __init__(self, name, parent=analog_tuner_delegation()):
        self.name = name
        self.port_lock = threading.Lock()
        self.parent = parent

    def setTunerCenterFrequency(self, id, freq):
        self.port_lock.acquire()
        try:
            return self.parent.setTunerCenterFrequency(id,freq)
        finally:
            self.port_lock.release()

    def getTunerCenterFrequency(self, id):
        self.port_lock.acquire()
        try:
            return self.parent.getTunerCenterFrequency(id)
        finally:
            self.port_lock.release()

    def setTunerBandwidth(self, id, bw):
        self.port_lock.acquire()
        try:
            return self.parent.setTunerBandwidth(id,bw)
        finally:
            self.port_lock.release()

    def getTunerBandwidth(self, id):
        self.port_lock.acquire()
        try:
            return self.parent.getTunerBandwidth(id)
        finally:
            self.port_lock.release()

    def setTunerAgcEnable(self, id, enable):
        self.port_lock.acquire()
        try:
            return self.parent.setTunerAgcEnable(id,enable)
        finally:
            self.port_lock.release()

    def getTunerAgcEnable(self, id):
        self.port_lock.acquire()
        try:
            return self.parent.getTunerAgcEnable(id)
        finally:
            self.port_lock.release()

    def setTunerGain(self, id, gain):
        self.port_lock.acquire()
        try:
            return self.parent.setTunerGain(id,gain)
        finally:
            self.port_lock.release()

    def getTunerGain(self, id):
        self.port_lock.acquire()
        try:
            return self.parent.getTunerGain(id)
        finally:
            self.port_lock.release()

    def setTunerReferenceSource(self, id, source):
        self.port_lock.acquire()
        try:
            return self.parent.setTunerReferenceSource(id,source)
        finally:
            self.port_lock.release()

    def getTunerReferenceSource(self, id):
        self.port_lock.acquire()
        try:
            return self.parent.getTunerReferenceSource(id)
        finally:
            self.port_lock.release()

    def setTunerEnable(self, id, enable):
        self.port_lock.acquire()
        try:
            return self.parent.setTunerEnable(id,enable)
        finally:
            self.port_lock.release()

    def getTunerEnable(self, id):
        self.port_lock.acquire()
        try:
            return self.parent.getTunerEnable(id)
        finally:
            self.port_lock.release()

class InDigitalTunerPort(FRONTEND__POA.DigitalTuner, InAnalogTunerPort):
    def __init__(self, name, parent=digital_tuner_delegation()):
        self.name = name
        self.port_lock = threading.Lock()
        self.parent = parent
        
    def setTunerOutputSampleRate(self, id, sr):
        self.port_lock.acquire()
        try:
            return self.parent.setTunerOutputSampleRate(id,sr)
        finally:
            self.port_lock.release()

    def getTunerOutputSampleRate(self, id):
        self.port_lock.acquire()
        try:
            return self.parent.getTunerOutputSampleRate(id)
        finally:
            self.port_lock.release()

class gps_delegation(object):
    def get_gps_info(self, port_name):
        _gpsinfo = FRONTEND.GPSInfo('','','',1L,1L,1L,1.0,1.0,1.0,1.0,1,1.0,'',BULKIO.PrecisionUTCTime(1,1,1.0,1.0,1.0),[])
        return _gpsinfo
    def set_gps_info(self, port_name, gps_info):
        pass
    def get_gps_time_pos(self, port_name):
        _positioninfo = FRONTEND.PositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
        _gpstimepos = FRONTEND.GpsTimePos(_positioninfo,BULKIO.PrecisionUTCTime(1,1,1.0,1.0,1.0))
        return _gpstimepos
    def set_gps_time_pos(self, port_name, gps_time_pos):
        pass

class InGPSPort(FRONTEND__POA.GPS):
    def __init__(self, name, parent=gps_delegation()):
        self.name = name
        self.port_lock = threading.Lock()
        self.parent = parent
        
    def _get_gps_info(self):
        self.port_lock.acquire()
        try:
            return copy.deepcopy(self.parent.get_gps_info(self.name))
        finally:
            self.port_lock.release()

    def _set_gps_info(self, data):
        self.port_lock.acquire()
        try:
            return self.parent.set_gps_info(self.name, copy.deepcopy(data))
        finally:
            self.port_lock.release()

    def _get_gps_time_pos(self):
        self.port_lock.acquire()
        try:
            return copy.deepcopy(self.parent.get_gps_time_pos(self.name))
        finally:
            self.port_lock.release()

    def _set_gps_time_pos(self, data):
        self.port_lock.acquire()
        try:
            return self.parent.set_gps_time_pos(self.name,copy.deepcopy(data))
        finally:
            self.port_lock.release()

class rfinfo_delegation(object):
    def get_rf_flow_id(self, port_name):
        return ""
    def set_rf_flow_id(self, port_name, id):
        pass
    def get_rfinfo_pkt(self, port_name):
        _antennainfo=FRONTEND.AntennaInfo('','','','')
        _freqrange=FRONTEND.FreqRange(0,0,[])
        _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
        _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
        _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange)
        _rfinfopkt=FRONTEND.RFInfoPkt('',0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
        return _rfinfopkt
    def set_rfinfo_pkt(self, port_name, pkt):
        pass

class InRFInfoPort(FRONTEND__POA.RFInfo):
    def __init__(self, name, parent=rfinfo_delegation()):
        self.name = name
        self.port_lock = threading.Lock()
        self.parent = parent
        
    def _get_rf_flow_id(self):
        self.port_lock.acquire()
        try:
            return self.parent.get_rf_flow_id(self.name)
        finally:
            self.port_lock.release()

    def _set_rf_flow_id(self, data):
        self.port_lock.acquire()
        try:
            return self.parent.set_rf_flow_id(self.name,data)
        finally:
            self.port_lock.release()

    def _get_rfinfo_pkt(self):
        self.port_lock.acquire()
        try:
            return copy.deepcopy(self.parent.get_rfinfo_pkt(self.name))
        finally:
            self.port_lock.release()

    def _set_rfinfo_pkt(self, data):
        self.port_lock.acquire()
        try:
            return self.parent.set_rfinfo_pkt(self.name,copy.deepcopy(data))
        finally:
            self.port_lock.release()

class rfsource_delegation(object):
    def get_available_rf_inputs(self, port_name):
        return []
    def set_available_rf_inputs(self, port_name, inputs):
        pass
    def get_current_rf_input(self, port_name):
        _antennainfo=FRONTEND.AntennaInfo('','','','')
        _freqrange=FRONTEND.FreqRange(0,0,[])
        _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
        _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
        _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange)
        _rfinfopkt=FRONTEND.RFInfoPkt('',0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
        return _rfinfopkt
    def set_current_rf_input(self, port_name, input):
        pass

class InRFSourcePort(FRONTEND__POA.RFSource):
    def __init__(self, name, parent=rfsource_delegation()):
        self.name = name
        self.port_lock = threading.Lock()
        self.parent = parent
        
    def _get_available_rf_inputs(self):
        self.port_lock.acquire()
        try:
            return copy.deepcopy(self.parent.get_available_rf_inputs(self.name))
        finally:
            self.port_lock.release()

    def _set_available_rf_inputs(self, data):
        self.port_lock.acquire()
        try:
            return self.parent.set_available_rf_inputs(self.name,copy.deepcopy(data))
        finally:
            self.port_lock.release()

    def _get_current_rf_input(self):
        self.port_lock.acquire()
        try:
            return copy.deepcopy(self.parent.get_current_rf_input(self.name))
        finally:
            self.port_lock.release()

    def _set_current_rf_input(self, data):
        self.port_lock.acquire()
        try:
            return self.parent.set_current_rf_input(self.name,copy.deepcopy(data))
        finally:
            self.port_lock.release()

class nav_delegation(object):
    def get_nav_packet(self, port_name):
        _time = BULKIO.PrecisionUTCTime(1,1,1.0,1.0,1.0)
        _positioninfo = FRONTEND.PositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
        _cartesianpos=FRONTEND.CartesianPositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
        _velocityinfo=FRONTEND.VelocityInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
        _accelerationinfo=FRONTEND.AccelerationInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
        _attitudeinfo=FRONTEND.AttitudeInfo(False,0.0,0.0,0.0)
        _navpacket=FRONTEND.NavigationPacket('','',_positioninfo,_cartesianpos,_velocityinfo,_accelerationinfo,_attitudeinfo,_time,[])
        return _navpacket
    def set_nav_packet(self, port_name, nav_info):
        pass

class InNavDataPort(FRONTEND__POA.NavData):
    def __init__(self, name, parent=nav_delegation()):
        self.name = name
        self.port_lock = threading.Lock()
        self.parent = parent
        
    def _get_nav_packet(self):
        self.port_lock.acquire()
        try:
            return copy.deepcopy(self.parent.get_nav_packet(self.name))
        finally:
            self.port_lock.release()

    def _set_nav_packet(self, data):
        self.port_lock.acquire()
        try:
            return self.parent.set_nav_packet(self.name,copy.deepcopy(data))
        finally:
            self.port_lock.release()
