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
#% extends "pull/resource.py"

#{% block updateUsageState %}
#{%   for sc in component.superclasses if sc.name == "Device" %}
    ${super()}
#{%  endfor %}
#{% endblock %}

#{% block extensions %}
#{% if 'FrontendTuner' in component.implements %}
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
    def getTunerType(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].tuner_type

    def getTunerDeviceControl(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        if self.getControlAllocationId(idx) == allocation_id:
            return True
        return False

    def getTunerGroupId(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].group_id

    def getTunerRfFlowId(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].rf_flow_id

#{% endif %}
#{% if 'AnalogTuner' in component.implements %}

    def setTunerCenterFrequency(self,allocation_id, freq):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        if allocation_id != self.getControlAllocationId(idx):
            raise FRONTEND.FrontendException(("ID "+str(allocation_id)+" does not have authorization to modify the tuner"))
        if freq<0: raise FRONTEND.BadParameterException()
        # set hardware to new value. Raise an exception if it's not possible
        self.frontend_tuner_status[idx].center_frequency = freq

    def getTunerCenterFrequency(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].center_frequency

    def setTunerBandwidth(self,allocation_id, bw):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        if allocation_id != self.getControlAllocationId(idx):
            raise FRONTEND.FrontendException(("ID "+str(allocation_id)+" does not have authorization to modify the tuner"))
        if bw<0: raise FRONTEND.BadParameterException()
        # set hardware to new value. Raise an exception if it's not possible
        self.frontend_tuner_status[idx].bandwidth = bw

    def getTunerBandwidth(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].bandwidth

    def setTunerAgcEnable(self,allocation_id, enable):
        raise FRONTEND.NotSupportedException("setTunerAgcEnable not supported")

    def getTunerAgcEnable(self,allocation_id):
        raise FRONTEND.NotSupportedException("getTunerAgcEnable not supported")

    def setTunerGain(self,allocation_id, gain):
        raise FRONTEND.NotSupportedException("setTunerGain not supported")

    def getTunerGain(self,allocation_id):
        raise FRONTEND.NotSupportedException("getTunerGain not supported")

    def setTunerReferenceSource(self,allocation_id, source):
        raise FRONTEND.NotSupportedException("setTunerReferenceSource not supported")

    def getTunerReferenceSource(self,allocation_id):
        raise FRONTEND.NotSupportedException("getTunerReferenceSource not supported")

    def setTunerEnable(self,allocation_id, enable):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        if allocation_id != self.getControlAllocationId(idx):
            raise FRONTEND.FrontendException(("ID "+str(allocation_id)+" does not have authorization to modify the tuner"))
        # set hardware to new value. Raise an exception if it's not possible
        self.frontend_tuner_status[idx].enabled = enable

    def getTunerEnable(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].enabled

#{% endif %}
#{% if 'DigitalTuner' in component.implements %}

    def setTunerOutputSampleRate(self,allocation_id, sr):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        if allocation_id != self.getControlAllocationId(idx):
            raise FRONTEND.FrontendException(("ID "+str(allocation_id)+" does not have authorization to modify the tuner"))
        if sr<0: raise FRONTEND.BadParameterException()
        # set hardware to new value. Raise an exception if it's not possible
        self.frontend_tuner_status[idx].sample_rate = sr

    def getTunerOutputSampleRate(self,allocation_id):
        idx = self.getTunerMapping(allocation_id)
        if idx < 0: raise FRONTEND.FrontendException("Invalid allocation id")
        return self.frontend_tuner_status[idx].sample_rate

#{% endif %}
#{% if 'GPS' in component.implements %}

    def get_gps_info(self,port_name):
        return frontend.GPSInfo()

    def set_gps_info(self,port_name, gps_info):
        pass

    def get_gps_time_pos(self,port_name):
        return frontend.GpsTimePos()

    def set_gps_time_pos(self,port_name, gps_time_pos):
        pass
#{% endif %}
#{% if 'NavData' in component.implements %}

    def get_nav_packet(self,port_name):
        return frontend.NavigationPacket()

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
        return "none"

    def set_rf_flow_id(self,port_name, id):
        pass

    def get_rfinfo_pkt(self,port_name):
        return frontend.RFInfoPkt()

    def set_rfinfo_pkt(self,port_name, pkt):
        pass
#{% endif %}
#{% if 'RFSource' in component.implements %}

    def get_available_rf_inputs(self,port_name):
        return []

    def set_available_rf_inputs(self,port_name, inputs):
        pass

    def get_current_rf_input(self,port_name):
        return frontend.RFInfoPkt()

    def set_current_rf_input(self, port_name, pkt):
        pass
#{% endif %}
#{% endblock %}
