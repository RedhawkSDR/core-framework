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

#from ossie.cf import CF, CF__POA
#from ossie.utils import uuid

#import copy, time
import threading
#from ossie.resource import usesport, providesport
#import bulkio
from redhawk.frontendInterfaces import FRONTEND__POA
from redhawk.frontendInterfaces import FRONTEND


'''provides port(s)'''

class InFrontendTunerPort(FRONTEND__POA.FrontendTuner):
    def __init__(self, name,
                newTunerTypeGetterCB = None,
                newTunerDeviceControlGetterCB = None,
                newTunerGroupIdGetterCB = None,
                newTunerRfFlowIdGetterCB = None,
                newTunerStatusGetterCB = None):
        self.name = name
        self.port_lock = threading.Lock()
        
        self.getTunerTypeCB = newTunerTypeGetterCB
        self.getTunerDeviceControlCB = newTunerDeviceControlGetterCB
        self.getTunerGroupIdCB = newTunerGroupIdGetterCB
        self.getTunerRfFlowIdCB = newTunerRfFlowIdGetterCB
        self.getTunerStatusCB = newTunerStatusGetterCB

    def setTunerTypeGetterCB(self, newTunerTypeGetterCB):
        self.port_lock.acquire()
        self.getTunerTypeCB = newTunerTypeGetterCB
        self.port_lock.release()
    def setTunerDeviceControlGetterCB(self, newTunerDeviceControlGetterCB):
        self.port_lock.acquire()
        self.getTunerDeviceControlCB = newTunerDeviceControlGetterCB
        self.port_lock.release()
    def setTunerGroupIdGetterCB(self, newTunerGroupIdGetterCB):
        self.port_lock.acquire()
        self.getTunerGroupIdCB = newTunerGroupIdGetterCB
        self.port_lock.release()
    def setTunerRfFlowIdGetterCB(self, newTunerRfFlowIdGetterCB):
        self.port_lock.acquire()
        self.getTunerRfFlowIdCB = newTunerRfFlowIdGetterCB
        self.port_lock.release()
    def setTunerStatusGetterCB(self, newTunerStatusGetterCB):
        self.port_lock.acquire()
        self.getTunerStatusCB = newTunerStatusGetterCB
        self.port_lock.release()
        

    def getTunerType(self, id):
        self.port_lock.acquire()
        if ( self.getTunerTypeCB ):
            return self.getTunerTypeCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerType(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerDeviceControl(self, id):
        self.port_lock.acquire()
        if ( self.getTunerDeviceControlCB ):
            return self.getTunerDeviceControlCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerDeviceControl(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerGroupId(self, id):
        self.port_lock.acquire()
        if ( self.getTunerGroupIdCB ):
            return self.getTunerGroupIdCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerGroupId(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerRfFlowId(self, id):
        self.port_lock.acquire()
        if ( self.getTunerRfFlowIdCB ):
            return self.getTunerRfFlowIdCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerRfFlowId(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerStatus(self, id):
        self.port_lock.acquire()
        if ( self.getTunerStatusCB ):
            return self.getTunerStatusCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerStatus(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

class InAnalogTunerPort(FRONTEND__POA.AnalogTuner):
    def __init__(self, name,
                newTunerTypeGetterCB = None,
                newTunerDeviceControlGetterCB = None,
                newTunerGroupIdGetterCB = None,
                newTunerRfFlowIdGetterCB = None,
                newTunerStatusGetterCB = None,
                newTunerCenterFrequencyGetterCB = None,
                newTunerCenterFrequencySetterCB = None,
                newTunerBandwidthGetterCB = None,
                newTunerBandwidthSetterCB = None,
                newTunerAgcEnableGetterCB = None,
                newTunerAgcEnableSetterCB = None,
                newTunerGainGetterCB = None,
                newTunerGainSetterCB = None,
                newTunerReferenceSourceGetterCB = None,
                newTunerReferenceSourceSetterCB = None,
                newTunerEnableGetterCB = None,
                newTunerEnableSetterCB = None):
        self.name = name
        self.port_lock = threading.Lock()
        
        self.getTunerTypeCB = newTunerTypeGetterCB
        self.getTunerDeviceControlCB = newTunerDeviceControlGetterCB
        self.getTunerGroupIdCB = newTunerGroupIdGetterCB
        self.getTunerRfFlowIdCB = newTunerRfFlowIdGetterCB
        self.getTunerStatusCB = newTunerStatusGetterCB
        self.getTunerCenterFrequencyCB = newTunerCenterFrequencyGetterCB
        self.setTunerCenterFrequencyCB = newTunerCenterFrequencySetterCB
        self.getTunerBandwidthCB = newTunerBandwidthGetterCB
        self.setTunerBandwidthCB = newTunerBandwidthSetterCB
        self.getTunerAgcEnableCB = newTunerAgcEnableGetterCB
        self.setTunerAgcEnableCB = newTunerAgcEnableSetterCB
        self.getTunerGainCB = newTunerGainGetterCB
        self.setTunerGainCB = newTunerGainSetterCB
        self.getTunerReferenceSourceCB = newTunerReferenceSourceGetterCB
        self.setTunerReferenceSourceCB = newTunerReferenceSourceSetterCB
        self.getTunerEnableCB = newTunerEnableGetterCB
        self.setTunerEnableCB = newTunerEnableSetterCB

    def setTunerTypeGetterCB(self, newTunerTypeGetterCB):
        self.port_lock.acquire()
        self.getTunerTypeCB = newTunerTypeGetterCB
        self.port_lock.release()
    def setTunerDeviceControlGetterCB(self, newTunerDeviceControlGetterCB):
        self.port_lock.acquire()
        self.getTunerDeviceControlCB = newTunerDeviceControlGetterCB
        self.port_lock.release()
    def setTunerGroupIdGetterCB(self, newTunerGroupIdGetterCB):
        self.port_lock.acquire()
        self.getTunerGroupIdCB = newTunerGroupIdGetterCB
        self.port_lock.release()
    def setTunerRfFlowIdGetterCB(self, newTunerRfFlowIdGetterCB):
        self.port_lock.acquire()
        self.getTunerRfFlowIdCB = newTunerRfFlowIdGetterCB
        self.port_lock.release()
    def setTunerStatusGetterCB(self, newTunerStatusGetterCB):
        self.port_lock.acquire()
        self.getTunerStatusCB = newTunerStatusGetterCB
        self.port_lock.release()
    def setTunerCenterFrequencyGetterCB(self, newTunerCenterFrequencyGetterCB):
        self.port_lock.acquire()
        self.getTunerCenterFrequencyCB = newTunerCenterFrequencyGetterCB
        self.port_lock.release()
    def setTunerCenterFrequencySetterCB(self, newTunerCenterFrequencySetterCB):
        self.port_lock.acquire()
        self.setTunerCenterFrequencyCB = newTunerCenterFrequencySetterCB
        self.port_lock.release()
    def setTunerBandwidthGetterCB(self, newTunerBandwidthGetterCB):
        self.port_lock.acquire()
        self.getTunerBandwidthCB = newTunerBandwidthGetterCB
        self.port_lock.release()
    def setTunerBandwidthSetterCB(self, newTunerBandwidthSetterCB):
        self.port_lock.acquire()
        self.setTunerBandwidthCB = newTunerBandwidthSetterCB
        self.port_lock.release()
    def setTunerAgcEnableGetterCB(self, newTunerAgcEnableGetterCB):
        self.port_lock.acquire()
        self.getTunerAgcEnableCB = newTunerAgcEnableGetterCB
        self.port_lock.release()
    def setTunerAgcEnableSetterCB(self, newTunerAgcEnableSetterCB):
        self.port_lock.acquire()
        self.setTunerAgcEnableCB = newTunerAgcEnableSetterCB
        self.port_lock.release()
    def setTunerGainGetterCB(self, newTunerGainGetterCB):
        self.port_lock.acquire()
        self.getTunerGainCB = newTunerGainGetterCB
        self.port_lock.release()
    def setTunerGainSetterCB(self, newTunerGainSetterCB):
        self.port_lock.acquire()
        self.setTunerGainCB = newTunerGainSetterCB
        self.port_lock.release()
    def setTunerReferenceSourceGetterCB(self, newTunerReferenceSourceGetterCB):
        self.port_lock.acquire()
        self.getTunerReferenceSourceCB = newTunerReferenceSourceGetterCB
        self.port_lock.release()
    def setTunerReferenceSourceSetterCB(self, newTunerReferenceSourceSetterCB):
        self.port_lock.acquire()
        self.setTunerReferenceSourceCB = newTunerReferenceSourceSetterCB
        self.port_lock.release()
    def setTunerEnableGetterCB(self, newTunerEnableGetterCB):
        self.port_lock.acquire()
        self.getTunerEnableCB = newTunerEnableGetterCB
        self.port_lock.release()
    def setTunerEnableSetterCB(self, newTunerEnableSetterCB):
        self.port_lock.acquire()
        self.setTunerEnableCB = newTunerEnableSetterCB
        self.port_lock.release()
        
    def getTunerType(self, id):
        self.port_lock.acquire()
        if ( self.getTunerTypeCB ):
            return self.getTunerTypeCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerType(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerDeviceControl(self, id):
        self.port_lock.acquire()
        if ( self.getTunerDeviceControlCB ):
            return self.getTunerDeviceControlCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerDeviceControl(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerGroupId(self, id):
        self.port_lock.acquire()
        if ( self.getTunerGroupIdCB ):
            return self.getTunerGroupIdCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerGroupId(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerRfFlowId(self, id):
        self.port_lock.acquire()
        if ( self.getTunerRfFlowIdCB ):
            return self.getTunerRfFlowIdCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerRfFlowId(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerStatus(self, id):
        self.port_lock.acquire()
        if ( self.getTunerStatusCB ):
            return self.getTunerStatusCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerStatus(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def setTunerCenterFrequency(self, id, freq):
        self.port_lock.acquire()
        if ( self.setTunerCenterFrequencyCB ):
            return self.setTunerCenterFrequencyCB(id,freq)
        else:
            raise FRONTEND.NotSupportedException("getTunerCenterFrequency(id,freq) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerCenterFrequency(self, id):
        self.port_lock.acquire()
        if ( self.getTunerCenterFrequencyCB ):
            return self.getTunerCenterFrequencyCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerCenterFrequency(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def setTunerBandwidth(self, id, bw):
        self.port_lock.acquire()
        if ( self.setTunerBandwidthCB ):
            return self.setTunerBandwidthCB(id,bw)
        else:
            raise FRONTEND.NotSupportedException("getTunerBandwidth(id,bw) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerBandwidth(self, id):
        self.port_lock.acquire()
        if ( self.getTunerBandwidthCB ):
            return self.getTunerBandwidthCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerBandwidth(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def setTunerAgcEnable(self, id, enable):
        self.port_lock.acquire()
        if ( self.setTunerAgcEnableCB ):
            return self.setTunerAgcEnableCB(id,enable)
        else:
            raise FRONTEND.NotSupportedException("getTunerAgcEnable(id,enable) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerAgcEnable(self, id):
        self.port_lock.acquire()
        if ( self.getTunerAgcEnableCB ):
            return self.getTunerAgcEnableCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerAgcEnable(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def setTunerGain(self, id, gain):
        self.port_lock.acquire()
        if ( self.setTunerGainCB ):
            return self.setTunerGainCB(id,gain)
        else:
            raise FRONTEND.NotSupportedException("getTunerGain(id,gain) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerGain(self, id):
        self.port_lock.acquire()
        if ( self.getTunerGainCB ):
            return self.getTunerGainCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerGain(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def setTunerReferenceSource(self, id, source):
        self.port_lock.acquire()
        if ( self.setTunerReferenceSourceCB ):
            return self.setTunerReferenceSourceCB(id,source)
        else:
            raise FRONTEND.NotSupportedException("getTunerReferenceSource(id,source) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerReferenceSource(self, id):
        self.port_lock.acquire()
        if ( self.getTunerReferenceSourceCB ):
            return self.getTunerReferenceSourceCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerReferenceSource(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def setTunerEnable(self, id, enable):
        self.port_lock.acquire()
        if ( self.setTunerEnableCB ):
            return self.setTunerEnableCB(id,enable)
        else:
            raise FRONTEND.NotSupportedException("getTunerEnable(id,enable) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerEnable(self, id):
        self.port_lock.acquire()
        if ( self.getTunerEnableCB ):
            return self.getTunerEnableCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerEnable(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

class InDigitalTunerPort(FRONTEND__POA.DigitalTuner):
    def __init__(self, name,
                newTunerTypeGetterCB = None,
                newTunerDeviceControlGetterCB = None,
                newTunerGroupIdGetterCB = None,
                newTunerRfFlowIdGetterCB = None,
                newTunerStatusGetterCB = None,
                newTunerCenterFrequencyGetterCB = None,
                newTunerCenterFrequencySetterCB = None,
                newTunerBandwidthGetterCB = None,
                newTunerBandwidthSetterCB = None,
                newTunerAgcEnableGetterCB = None,
                newTunerAgcEnableSetterCB = None,
                newTunerGainGetterCB = None,
                newTunerGainSetterCB = None,
                newTunerReferenceSourceGetterCB = None,
                newTunerReferenceSourceSetterCB = None,
                newTunerEnableGetterCB = None,
                newTunerEnableSetterCB = None,
                newTunerOutputSampleRateGetterCB = None,
                newTunerOutputSampleRateSetterCB = None):
        self.name = name
        self.port_lock = threading.Lock()
        
        self.getTunerTypeCB = newTunerTypeGetterCB
        self.getTunerDeviceControlCB = newTunerDeviceControlGetterCB
        self.getTunerGroupIdCB = newTunerGroupIdGetterCB
        self.getTunerRfFlowIdCB = newTunerRfFlowIdGetterCB
        self.getTunerStatusCB = newTunerStatusGetterCB
        self.getTunerCenterFrequencyCB = newTunerCenterFrequencyGetterCB
        self.setTunerCenterFrequencyCB = newTunerCenterFrequencySetterCB
        self.getTunerBandwidthCB = newTunerBandwidthGetterCB
        self.setTunerBandwidthCB = newTunerBandwidthSetterCB
        self.getTunerAgcEnableCB = newTunerAgcEnableGetterCB
        self.setTunerAgcEnableCB = newTunerAgcEnableSetterCB
        self.getTunerGainCB = newTunerGainGetterCB
        self.setTunerGainCB = newTunerGainSetterCB
        self.getTunerReferenceSourceCB = newTunerReferenceSourceGetterCB
        self.setTunerReferenceSourceCB = newTunerReferenceSourceSetterCB
        self.getTunerEnableCB = newTunerEnableGetterCB
        self.setTunerEnableCB = newTunerEnableSetterCB
        self.getTunerOutputSampleRateCB = newTunerOutputSampleRateGetterCB
        self.setTunerOutputSampleRateCB = newTunerOutputSampleRateSetterCB

    def setTunerTypeGetterCB(self, newTunerTypeGetterCB):
        self.port_lock.acquire()
        self.getTunerTypeCB = newTunerTypeGetterCB
        self.port_lock.release()
    def setTunerDeviceControlGetterCB(self, newTunerDeviceControlGetterCB):
        self.port_lock.acquire()
        self.getTunerDeviceControlCB = newTunerDeviceControlGetterCB
        self.port_lock.release()
    def setTunerGroupIdGetterCB(self, newTunerGroupIdGetterCB):
        self.port_lock.acquire()
        self.getTunerGroupIdCB = newTunerGroupIdGetterCB
        self.port_lock.release()
    def setTunerRfFlowIdGetterCB(self, newTunerRfFlowIdGetterCB):
        self.port_lock.acquire()
        self.getTunerRfFlowIdCB = newTunerRfFlowIdGetterCB
        self.port_lock.release()
    def setTunerStatusGetterCB(self, newTunerStatusGetterCB):
        self.port_lock.acquire()
        self.getTunerStatusCB = newTunerStatusGetterCB
        self.port_lock.release()
    def setTunerCenterFrequencyGetterCB(self, newTunerCenterFrequencyGetterCB):
        self.port_lock.acquire()
        self.getTunerCenterFrequencyCB = newTunerCenterFrequencyGetterCB
        self.port_lock.release()
    def setTunerCenterFrequencySetterCB(self, newTunerCenterFrequencySetterCB):
        self.port_lock.acquire()
        self.setTunerCenterFrequencyCB = newTunerCenterFrequencySetterCB
        self.port_lock.release()
    def setTunerBandwidthGetterCB(self, newTunerBandwidthGetterCB):
        self.port_lock.acquire()
        self.getTunerBandwidthCB = newTunerBandwidthGetterCB
        self.port_lock.release()
    def setTunerBandwidthSetterCB(self, newTunerBandwidthSetterCB):
        self.port_lock.acquire()
        self.setTunerBandwidthCB = newTunerBandwidthSetterCB
        self.port_lock.release()
    def setTunerAgcEnableGetterCB(self, newTunerAgcEnableGetterCB):
        self.port_lock.acquire()
        self.getTunerAgcEnableCB = newTunerAgcEnableGetterCB
        self.port_lock.release()
    def setTunerAgcEnableSetterCB(self, newTunerAgcEnableSetterCB):
        self.port_lock.acquire()
        self.setTunerAgcEnableCB = newTunerAgcEnableSetterCB
        self.port_lock.release()
    def setTunerGainGetterCB(self, newTunerGainGetterCB):
        self.port_lock.acquire()
        self.getTunerGainCB = newTunerGainGetterCB
        self.port_lock.release()
    def setTunerGainSetterCB(self, newTunerGainSetterCB):
        self.port_lock.acquire()
        self.setTunerGainCB = newTunerGainSetterCB
        self.port_lock.release()
    def setTunerReferenceSourceGetterCB(self, newTunerReferenceSourceGetterCB):
        self.port_lock.acquire()
        self.getTunerReferenceSourceCB = newTunerReferenceSourceGetterCB
        self.port_lock.release()
    def setTunerReferenceSourceSetterCB(self, newTunerReferenceSourceSetterCB):
        self.port_lock.acquire()
        self.setTunerReferenceSourceCB = newTunerReferenceSourceSetterCB
        self.port_lock.release()
    def setTunerEnableGetterCB(self, newTunerEnableGetterCB):
        self.port_lock.acquire()
        self.getTunerEnableCB = newTunerEnableGetterCB
        self.port_lock.release()
    def setTunerEnableSetterCB(self, newTunerEnableSetterCB):
        self.port_lock.acquire()
        self.setTunerEnableCB = newTunerEnableSetterCB
        self.port_lock.release()
    def setTunerOutputSampleRateGetterCB(self, newTunerOutputSampleRateGetterCB):
        self.port_lock.acquire()
        self.getTunerOutputSampleRateCB = newTunerOutputSampleRateGetterCB
        self.port_lock.release()
    def setTunerOutputSampleRateSetterCB(self, newTunerOutputSampleRateSetterCB):
        self.port_lock.acquire()
        self.setTunerOutputSampleRateCB = newTunerOutputSampleRateSetterCB
        self.port_lock.release()
        
    def getTunerType(self, id):
        self.port_lock.acquire()
        if ( self.getTunerTypeCB ):
            return self.getTunerTypeCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerType(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerDeviceControl(self, id):
        self.port_lock.acquire()
        if ( self.getTunerDeviceControlCB ):
            return self.getTunerDeviceControlCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerDeviceControl(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerGroupId(self, id):
        self.port_lock.acquire()
        if ( self.getTunerGroupIdCB ):
            return self.getTunerGroupIdCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerGroupId(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerRfFlowId(self, id):
        self.port_lock.acquire()
        if ( self.getTunerRfFlowIdCB ):
            return self.getTunerRfFlowIdCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerRfFlowId(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerStatus(self, id):
        self.port_lock.acquire()
        if ( self.getTunerStatusCB ):
            return self.getTunerStatusCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerStatus(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def setTunerCenterFrequency(self, id, freq):
        self.port_lock.acquire()
        if ( self.setTunerCenterFrequencyCB ):
            return self.setTunerCenterFrequencyCB(id,freq)
        else:
            raise FRONTEND.NotSupportedException("getTunerCenterFrequency(id,freq) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerCenterFrequency(self, id):
        self.port_lock.acquire()
        if ( self.getTunerCenterFrequencyCB ):
            return self.getTunerCenterFrequencyCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerCenterFrequency(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def setTunerBandwidth(self, id, bw):
        self.port_lock.acquire()
        if ( self.setTunerBandwidthCB ):
            return self.setTunerBandwidthCB(id,bw)
        else:
            raise FRONTEND.NotSupportedException("getTunerBandwidth(id,bw) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerBandwidth(self, id):
        self.port_lock.acquire()
        if ( self.getTunerBandwidthCB ):
            return self.getTunerBandwidthCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerBandwidth(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def setTunerAgcEnable(self, id, enable):
        self.port_lock.acquire()
        if ( self.setTunerAgcEnableCB ):
            return self.setTunerAgcEnableCB(id,enable)
        else:
            raise FRONTEND.NotSupportedException("getTunerAgcEnable(id,enable) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerAgcEnable(self, id):
        self.port_lock.acquire()
        if ( self.getTunerAgcEnableCB ):
            return self.getTunerAgcEnableCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerAgcEnable(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def setTunerGain(self, id, gain):
        self.port_lock.acquire()
        if ( self.setTunerGainCB ):
            return self.setTunerGainCB(id,gain)
        else:
            raise FRONTEND.NotSupportedException("getTunerGain(id,gain) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerGain(self, id):
        self.port_lock.acquire()
        if ( self.getTunerGainCB ):
            return self.getTunerGainCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerGain(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def setTunerReferenceSource(self, id, source):
        self.port_lock.acquire()
        if ( self.setTunerReferenceSourceCB ):
            return self.setTunerReferenceSourceCB(id,source)
        else:
            raise FRONTEND.NotSupportedException("getTunerReferenceSource(id,source) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerReferenceSource(self, id):
        self.port_lock.acquire()
        if ( self.getTunerReferenceSourceCB ):
            return self.getTunerReferenceSourceCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerReferenceSource(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def setTunerEnable(self, id, enable):
        self.port_lock.acquire()
        if ( self.setTunerEnableCB ):
            return self.setTunerEnableCB(id,enable)
        else:
            raise FRONTEND.NotSupportedException("getTunerEnable(id,enable) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerEnable(self, id):
        self.port_lock.acquire()
        if ( self.getTunerEnableCB ):
            return self.getTunerEnableCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerEnable(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def setTunerOutputSampleRate(self, id, sr):
        self.port_lock.acquire()
        if ( self.setTunerOutputSampleRateCB ):
            return self.setTunerOutputSampleRateCB(id,sr)
        else:
            raise FRONTEND.NotSupportedException("getTunerOutputSampleRate(id,sr) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def getTunerOutputSampleRate(self, id):
        self.port_lock.acquire()
        if ( self.getTunerOutputSampleRateCB ):
            return self.getTunerOutputSampleRateCB(id)
        else:
            raise FRONTEND.NotSupportedException("getTunerOutputSampleRate(id) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

class InGPSPort(FRONTEND__POA.GPS):
    def __init__(self, name,
              newGPSInfoGetterCB = None,
              newGpsTimePosGetterCB = None,
              newGPSInfoSetterCB = None,
              newGpsTimePosSetterCB = None):
        self.name = name
        self.port_lock = threading.Lock()
        
        self.getGPSInfoCB = newGPSInfoGetterCB
        self.getGpsTimePosCB = newGpsTimePosGetterCB
        self.setGPSInfoCB = newGPSInfoSetterCB
        self.setGpsTimePosCB = newGpsTimePosSetterCB

    def setGPSInfoGetterCB(self, newGPSInfoGetterCB):
        self.port_lock.acquire()
        self.getGPSInfoCB = newGPSInfoGetterCB
        self.port_lock.release()
    def setGpsTimePosGetterCB(self, newGpsTimePosGetterCB):
        self.port_lock.acquire()
        self.getGpsTimePosCB = newGpsTimePosGetterCB
        self.port_lock.release()
    def setGPSInfoSetterCB(self, newGPSInfoSetterCB):
        self.port_lock.acquire()
        self.setGPSInfoCB = newGPSInfoSetterCB
        self.port_lock.release()
    def setGpsTimePosSetterCB(self, newGpsTimePosSetterCB):
        self.port_lock.acquire()
        self.setGpsTimePosCB = newGpsTimePosSetterCB
        self.port_lock.release()

    def _get_gps_info(self):
        self.port_lock.acquire()
        if ( self.getGPSInfoCB ):
            return self.getGPSInfoCB()
        else:
            raise FRONTEND.NotSupportedException("_get_gps_info() IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def _set_gps_info(self, data):
        self.port_lock.acquire()
        if ( self.setGPSInfoCB ):
            return self.setGPSInfoCB(data)
        else:
            raise FRONTEND.NotSupportedException("_set_gps_info(data) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def _get_gps_time_pos(self):
        self.port_lock.acquire()
        if ( self.getGpsTimePosCB ):
            return self.getGpsTimePosCB()
        else:
            raise FRONTEND.NotSupportedException("_get_gps_time_pos() IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def _set_gps_time_pos(self, data):
        self.port_lock.acquire()
        if ( self.setGpsTimePosCB ):
            return self.setGpsTimePosCB(data)
        else:
            raise FRONTEND.NotSupportedException("_set_gps_time_pos(data) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

class InRFInfoPort(FRONTEND__POA.RFInfo):
    def __init__(self, name,
                 newRFFlowIdGetterCB = None,
                 newRFInfoPktGetterCB = None,
                 newRFFlowIdSetterCB = None,
                 newRFInfoPktSetterCB = None):
        self.name = name
        self.port_lock = threading.Lock()
        
        self.getRFFlowIdCB = newRFFlowIdGetterCB
        self.getRFInfoPktCB = newRFInfoPktGetterCB
        self.setRFFlowIdCB = newRFFlowIdSetterCB
        self.setRFInfoPktCB = newRFInfoPktSetterCB

    def setRFFlowIdGetterCB(self, newRFFlowIdGetterCB):
        self.port_lock.acquire()
        self.getRFFlowIdCB = newRFFlowIdGetterCB
        self.port_lock.release()
    def setRFInfoPktGetterCB(self, newRFInfoPktGetterCB):
        self.port_lock.acquire()
        self.getRFInfoPktCB = newRFInfoPktGetterCB
        self.port_lock.release()
    def setRFFlowIdSetterCB(self, newRFFlowIdSetterCB):
        self.port_lock.acquire()
        self.setRFFlowIdCB = newRFFlowIdSetterCB
        self.port_lock.release()
    def setRFInfoPktSetterCB(self, newRFInfoPktSetterCB):
        self.port_lock.acquire()
        self.setRFInfoPktCB = newRFInfoPktSetterCB
        self.port_lock.release()

    def _get_rf_flow_id(self):
        self.port_lock.acquire()
        if ( self.getRFFlowIdCB ):
            return self.getRFFlowIdCB()
        else:
            raise FRONTEND.NotSupportedException("_get_rf_flow_id() IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def _set_rf_flow_id(self, data):
        self.port_lock.acquire()
        if ( self.setRFFlowIdCB ):
            return self.setRFFlowIdCB(data)
        else:
            raise FRONTEND.NotSupportedException("_set_rf_flow_id(data) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def _get_rfinfo_pkt(self):
        self.port_lock.acquire()
        if ( self.getRFInfoPktCB ):
            return self.getRFInfoPktCB()
        else:
            raise FRONTEND.NotSupportedException("_get_rfinfo_pkt() IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def _set_rfinfo_pkt(self, data):
        self.port_lock.acquire()
        if ( self.setRFInfoPktCB ):
            return self.setRFInfoPktCB(data)
        else:
            raise FRONTEND.NotSupportedException("_set_rfinfo_pkt(data) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

class InRFSourcePort(FRONTEND__POA.RFSource):
    def __init__(self, name,
               newAvailableRFInputsGetterCB = None,
               newCurrentRFInputGetterCB = None,
               newAvailableRFInputsSetterCB = None,
               newCurrentRFInputSetterCB = None ):
        self.name = name
        self.port_lock = threading.Lock()
        
        self.getAvailableRFInputsCB = newAvailableRFInputsGetterCB
        self.getCurrentRFInputCB = newCurrentRFInputGetterCB
        self.setAvailableRFInputsCB = newAvailableRFInputsSetterCB
        self.setCurrentRFInputCB = newCurrentRFInputSetterCB

    def setAvailableRFInputsGetterCB(self, newAvailableRFInputsGetterCB):
        self.port_lock.acquire()
        self.getAvailableRFInputsCB = newAvailableRFInputsGetterCB
        self.port_lock.release()
    def setCurrentRFInputGetterCB(self, newCurrentRFInputGetterCB):
        self.port_lock.acquire()
        self.getCurrentRFInputCB = newCurrentRFInputGetterCB
        self.port_lock.release()
    def setAvailableRFInputsSetterCB(self, newAvailableRFInputsSetterCB):
        self.port_lock.acquire()
        self.setAvailableRFInputsCB = newAvailableRFInputsSetterCB
        self.port_lock.release()
    def setCurrentRFInputSetterCB(self, newCurrentRFInputSetterCB):
        self.port_lock.acquire()
        self.setCurrentRFInputCB = newCurrentRFInputSetterCB
        self.port_lock.release()
        
    def _get_available_rf_inputs(self):
        self.port_lock.acquire()
        if ( self.getAvailableRFInputsCB ):
            return self.getAvailableRFInputsCB()
        else:
            raise FRONTEND.NotSupportedException("_get_available_rf_inputs() IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def _set_available_rf_inputs(self, data):
        self.port_lock.acquire()
        if ( self.setAvailableRFInputsCB ):
            return self.setAvailableRFInputsCB(data)
        else:
            raise FRONTEND.NotSupportedException("_set_available_rf_inputs(data) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def _get_current_rf_input(self):
        self.port_lock.acquire()
        if ( self.getCurrentRFInputCB ):
            return self.getCurrentRFInputCB()
        else:
            raise FRONTEND.NotSupportedException("_get_current_rf_input() IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def _set_current_rf_input(self, data):
        self.port_lock.acquire()
        if ( self.setCurrentRFInputCB ):
            return self.setCurrentRFInputCB(data)
        else:
            raise FRONTEND.NotSupportedException("_set_current_rf_input(data) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

class InNavDataPort(FRONTEND__POA.NavData):
    def __init__(self, name,
              newNavPktGetterCB = None,
              newNavPktSetterCB = None):
        self.name = name
        self.port_lock = threading.Lock()
        
        self.getNavPktCB = newNavPktGetterCB
        self.setNavPktCB = newNavPktSetterCB

    def setNavPktGetterCB(self, newNavPktGetterCB):
        self.port_lock.acquire()
        self.getNavPktCB = newNavPktGetterCB
        self.port_lock.release()
    def setNavPktSetterCB(self, newNavPktSetterCB):
        self.port_lock.acquire()
        self.setNavPktCB = newNavPktSetterCB
        self.port_lock.release()

    def _get_nav_packet(self):
        self.port_lock.acquire()
        if ( self.getNavPktCB ):
            return self.getNavPktCB()
        else:
            raise FRONTEND.NotSupportedException("_get_nav_packet() IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()

    def _set_nav_packet(self, data):
        self.port_lock.acquire()
        if ( self.setNavPktCB ):
            return self.setNavPktCB(data)
        else:
            raise FRONTEND.NotSupportedException("_set_nav_packet(data) IS NOT CURRENTLY SUPPORTED");
        self.port_lock.release()
