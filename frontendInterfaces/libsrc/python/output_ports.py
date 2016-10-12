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

#import copy, time
import threading
#from ossie.resource import usesport, providesport
#import bulkio
#from redhawk.frontendInterfaces import FRONTEND__POA
from redhawk.frontendInterfaces import FRONTEND


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

    def _get_connections(self):
        currentConnections = []
        self.port_lock.acquire()
        for id_, port in self.outConnections.items():
            currentConnections.append(ExtendedCF.UsesConnection(id_, port))
        self.port_lock.release()
        return currentConnections

class OutFrontendTunerPort(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, FRONTEND.FrontendTuner)

    def getTunerType(self, id):
        retVal = ""
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerType(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerDeviceControl(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerDeviceControl(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerGroupId(self, id):
        retVal = ""
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerGroupId(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerRfFlowId(self, id):
        retVal = ""
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerRfFlowId(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerStatus(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
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

    def getTunerType(self, id):
        retVal = ""
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerType(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerDeviceControl(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerDeviceControl(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerGroupId(self, id):
        retVal = ""
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerGroupId(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerRfFlowId(self, id):
        retVal = ""
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerRfFlowId(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerStatus(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerStatus(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerCenterFrequency(self, id, freq):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setTunerCenterFrequency(id, freq)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def getTunerCenterFrequency(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerCenterFrequency(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerBandwidth(self, id, bw):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setTunerBandwidth(id, bw)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def getTunerBandwidth(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerBandwidth(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerAgcEnable(self, id, enable):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setTunerAgcEnable(id, enable)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def getTunerAgcEnable(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerAgcEnable(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerGain(self, id, gain):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setTunerGain(id, gain)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def getTunerGain(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerGain(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerReferenceSource(self, id, source):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setTunerReferenceSource(id, source)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def getTunerReferenceSource(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerReferenceSource(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerEnable(self, id, enable):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setTunerEnable(id, enable)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def getTunerEnable(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerEnable(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

class OutDigitalTunerPort(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, FRONTEND.DigitalTuner)

    def getTunerType(self, id):
        retVal = ""
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerType(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerDeviceControl(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerDeviceControl(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerGroupId(self, id):
        retVal = ""
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerGroupId(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerRfFlowId(self, id):
        retVal = ""
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerRfFlowId(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def getTunerStatus(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerStatus(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerCenterFrequency(self, id, freq):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setTunerCenterFrequency(id, freq)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def getTunerCenterFrequency(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerCenterFrequency(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerBandwidth(self, id, bw):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setTunerBandwidth(id, bw)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def getTunerBandwidth(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerBandwidth(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerAgcEnable(self, id, enable):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setTunerAgcEnable(id, enable)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def getTunerAgcEnable(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerAgcEnable(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerGain(self, id, gain):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setTunerGain(id, gain)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def getTunerGain(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerGain(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerReferenceSource(self, id, source):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setTunerReferenceSource(id, source)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def getTunerReferenceSource(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerReferenceSource(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerEnable(self, id, enable):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setTunerEnable(id, enable)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def getTunerEnable(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerEnable(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def setTunerOutputSampleRate(self, id, sr):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port.setTunerOutputSampleRate(id, sr)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def getTunerOutputSampleRate(self, id):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port.getTunerOutputSampleRate(id)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

class OutGPSPort(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, FRONTEND.GPS)

    def _get_gps_info(self):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port._get_gps_info()
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def _set_gps_info(self, data):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port._set_gps_info(data)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def _get_gps_time_pos(self):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port._get_gps_time_pos()
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def _set_gps_time_pos(self, data):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port._set_gps_time_pos(data)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
class OutRFInfoPort(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, FRONTEND.RFInfo)

    def _get_rf_flow_id(self):
        retVal = ""
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port._get_rf_flow_id()
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def _set_rf_flow_id(self, data):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port._set_rf_flow_id(data)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def _get_rfinfo_pkt(self):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port._get_rfinfo_pkt()
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def _set_rfinfo_pkt(self, data):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port._set_rfinfo_pkt(data)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
class OutRFSourcePort(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, FRONTEND.RFSource)

    def _get_available_rf_inputs(self):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port._get_available_rf_inputs()
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def _set_available_rf_inputs(self, data):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port._set_available_rf_inputs(data)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
    def _get_current_rf_input(self):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port._get_current_rf_input()
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def _set_current_rf_input(self, data):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port._set_current_rf_input(data)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
class OutNavDataPort(OutPort):
    def __init__(self, name):
        OutPort.__init__(self, name, FRONTEND.NavData)

    def _get_nav_packet(self):
        retVal = None
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        retVal = port._get_nav_packet()
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
            
        return retVal

    def _set_nav_packet(self, data):
        self.port_lock.acquire()

        try:
            for connId, port in self.outConnections.items():
                if port != None:
                    try:
                        port._set_nav_packet(data)
                    except Exception:
                        pass
        finally:
            self.port_lock.release()
