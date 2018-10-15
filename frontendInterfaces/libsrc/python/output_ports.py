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

from ossie.cf import CF, CF__POA, ExtendedCF
from ossie.cf.CF import Port
from ossie.utils import uuid

import threading
from redhawk.frontendInterfaces import FRONTEND
from ossie.resource import PortCallError


class OutPort (CF__POA.Port ):
    def __init__(self, name, PortTypeClass):
        self.name = name
        self.PortType = PortTypeClass
        self.outConnections = {} # key=connectionId,  value=port
        self.port_lock = threading.Lock()

    def connectPort(self, connection, connectionId):
        self.port_lock.acquire()
        try:
           try:
              port = connection._narrow(self.PortType)
              self.outConnections[str(connectionId)] = port
              self.refreshSRI = True
           except:
              raise Port.InvalidPort(1, "Invalid Port for Connection ID:" + str(connectionId) )
        finally:
            self.port_lock.release()

    def disconnectPort(self, connectionId):
        self.port_lock.acquire()
        try:
            self.outConnections.pop(str(connectionId), None)
        finally:
            self.port_lock.release()

    def getConnectionIds(self):
        return self.outConnections.keys()

    def _get_connections(self):
        currentConnections = []
        self.port_lock.acquire()
        for id_, port in self.outConnections.items():
            currentConnections.append(ExtendedCF.UsesConnection(id_, port))
        self.port_lock.release()
        return currentConnections

    def _evaluateRequestBasedOnConnections(self, __connection_id__, returnValue, inOut, out):
        if not __connection_id__ and len(self.outConnections) > 1:
            if (out or inOut or returnValue):
                raise PortCallError("Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.", self.getConnectionIds())

        if len(self.outConnections) == 0:
            if (out or inOut or returnValue):
                raise PortCallError("No connections available.", self.getConnectionIds())
            else:
                if __connection_id__:
                    raise PortCallError("The requested connection id ("+__connection_id__+") does not exist.", self.getConnectionIds())
        if __connection_id__ and len(self.outConnections) > 0:
            foundConnection = False
            for connId, port in self.outConnections.items():
                if __connection_id__ == connId:
                    foundConnection = True
                    break
            if not foundConnection:
                raise PortCallError("The requested connection id ("+__connection_id__+") does not exist.", self.getConnectionIds())

class OutFrontendTunerPort(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, FRONTEND.FrontendTuner)

    def getTunerType(self, id, __connection_id__=""):
        retVal = ""
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    try:
                        retVal = port.getTunerType(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()

        return retVal

    def getTunerDeviceControl(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    try:
                        retVal = port.getTunerDeviceControl(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerGroupId(self, id, __connection_id__=""):
        retVal = ""
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    try:
                        retVal = port.getTunerGroupId(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerRfFlowId(self, id, __connection_id__=""):
        retVal = ""
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    try:
                        retVal = port.getTunerRfFlowId(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerStatus(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    try:
                        retVal = port.getTunerStatus(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

class OutAnalogTunerPort(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, FRONTEND.AnalogTuner)

    def getTunerType(self, id, __connection_id__=""):
        retVal = ""
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerType(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerDeviceControl(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerDeviceControl(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerGroupId(self, id, __connection_id__=""):
        retVal = ""
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerGroupId(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerRfFlowId(self, id, __connection_id__=""):
        retVal = ""
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerRfFlowId(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerStatus(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerStatus(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerCenterFrequency(self, id, freq, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, false, false);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port.setTunerCenterFrequency(id, freq)
        finally:
            self.port_lock.release()
            
    def getTunerCenterFrequency(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerCenterFrequency(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerBandwidth(self, id, bw, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port.setTunerBandwidth(id, bw)
        finally:
            self.port_lock.release()
            
    def getTunerBandwidth(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerBandwidth(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerAgcEnable(self, id, enable, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port.setTunerAgcEnable(id, enable)
        finally:
            self.port_lock.release()
            
    def getTunerAgcEnable(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerAgcEnable(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerGain(self, id, gain, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port.setTunerGain(id, gain)
        finally:
            self.port_lock.release()
            
    def getTunerGain(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerGain(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerReferenceSource(self, id, source, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port.setTunerReferenceSource(id, source)
        finally:
            self.port_lock.release()
            
    def getTunerReferenceSource(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerReferenceSource(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerEnable(self, id, enable, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port.setTunerEnable(id, enable)
        finally:
            self.port_lock.release()
            
    def getTunerEnable(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerEnable(id)
        finally:
            self.port_lock.release()
            
        return retVal

class OutDigitalTunerPort(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, FRONTEND.DigitalTuner)

    def getTunerType(self, id, __connection_id__=""):
        retVal = ""
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerType(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerDeviceControl(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerDeviceControl(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerGroupId(self, id, __connection_id__=""):
        retVal = ""
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerGroupId(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerRfFlowId(self, id, __connection_id__=""):
        retVal = ""
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerRfFlowId(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerStatus(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerStatus(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerCenterFrequency(self, id, freq, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port.setTunerCenterFrequency(id, freq)
        finally:
            self.port_lock.release()
            
    def getTunerCenterFrequency(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerCenterFrequency(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerBandwidth(self, id, bw, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port.setTunerBandwidth(id, bw)
        finally:
            self.port_lock.release()
            
    def getTunerBandwidth(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerBandwidth(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerAgcEnable(self, id, enable, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port.setTunerAgcEnable(id, enable)
        finally:
            self.port_lock.release()
            
    def getTunerAgcEnable(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerAgcEnable(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerGain(self, id, gain, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port.setTunerGain(id, gain)
        finally:
            self.port_lock.release()
            
    def getTunerGain(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerGain(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerReferenceSource(self, id, source, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port.setTunerReferenceSource(id, source)
        finally:
            self.port_lock.release()
            
    def getTunerReferenceSource(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerReferenceSource(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerEnable(self, id, enable, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port.setTunerEnable(id, enable)
        finally:
            self.port_lock.release()
            
    def getTunerEnable(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerEnable(id)
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerOutputSampleRate(self, id, sr, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port.setTunerOutputSampleRate(id, sr)
        finally:
            self.port_lock.release()
            
    def getTunerOutputSampleRate(self, id, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port.getTunerOutputSampleRate(id)
        finally:
            self.port_lock.release()
            
        return retVal

class OutGPSPort(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, FRONTEND.GPS)

    def _get_gps_info(self, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port._get_gps_info()
        finally:
            self.port_lock.release()
            
        return retVal

    def _set_gps_info(self, data, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port._set_gps_info(data)
        finally:
            self.port_lock.release()
            
    def _get_gps_time_pos(self, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port._get_gps_time_pos()
        finally:
            self.port_lock.release()
            
        return retVal

    def _set_gps_time_pos(self, data, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port._set_gps_time_pos(data)
        finally:
            self.port_lock.release()
            
class OutRFInfoPort(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, FRONTEND.RFInfo)

    def _get_rf_flow_id(self, __connection_id__=""):
        retVal = ""
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port._get_rf_flow_id()
        finally:
            self.port_lock.release()
            
        return retVal

    def _set_rf_flow_id(self, data, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port._set_rf_flow_id(data)
        finally:
            self.port_lock.release()
            
    def _get_rfinfo_pkt(self, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port._get_rfinfo_pkt()
        finally:
            self.port_lock.release()
            
        return retVal

    def _set_rfinfo_pkt(self, data, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port._set_rfinfo_pkt(data)
        finally:
            self.port_lock.release()
            
class OutRFSourcePort(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, FRONTEND.RFSource)

    def _get_available_rf_inputs(self, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port._get_available_rf_inputs()
        finally:
            self.port_lock.release()
            
        return retVal

    def _set_available_rf_inputs(self, data, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port._set_available_rf_inputs(data)
        finally:
            self.port_lock.release()
            
    def _get_current_rf_input(self, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port._get_current_rf_input()
        finally:
            self.port_lock.release()
            
        return retVal

    def _set_current_rf_input(self, data, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port._set_current_rf_input(data)
        finally:
            self.port_lock.release()
            
class OutNavDataPort(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, FRONTEND.NavData)

    def _get_nav_packet(self, __connection_id__=""):
        retVal = None
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    retVal = port._get_nav_packet()
        finally:
            self.port_lock.release()
            
        return retVal

    def _set_nav_packet(self, data, __connection_id__=""):
        self.port_lock.acquire()

        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False);
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    port._set_nav_packet(data)
        finally:
            self.port_lock.release()
