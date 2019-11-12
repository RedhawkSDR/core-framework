#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: wb_receiver.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid, model

from frontend import FrontendScannerDevice
from ossie.device import AggregateDevice
from frontend import digital_scanning_tuner_delegation
from frontend import rfinfo_delegation
from ossie.threadedcomponent import *
from ossie.properties import simple_property
from ossie.properties import simpleseq_property
from ossie.properties import struct_property
from ossie.properties import structseq_property

import Queue, copy, time, threading
from ossie.resource import usesport, providesport, PortCallError
from ossie.cf import ExtendedCF
from ossie.cf import ExtendedCF__POA
from redhawk.frontendInterfaces import FRONTEND
import frontend
from frontend import FRONTEND
from ossie.properties import struct_to_props
BOOLEAN_VALUE_HERE=False
from omniORB import any as _any
from ossie.componentfamily import DynamicComponentParent
import supersimple
import anothersimple

class enums:
    # Enumerated values for FRONTEND::scanner_allocation
    class frontend_scanner_allocation:
        # Enumerated values for FRONTEND::scanner_allocation::mode
        class mode:
            SPAN_SCAN = "SPAN_SCAN"
            DISCRETE_SCAN = "DISCRETE_SCAN"
    
        # Enumerated values for FRONTEND::scanner_allocation::control_mode
        class control_mode:
            TIME_BASED = "TIME_BASED"
            SAMPLE_BASED = "SAMPLE_BASED"

class wb_receiver_base(CF__POA.AggregatePlainDevice, FrontendScannerDevice, AggregateDevice, digital_scanning_tuner_delegation, rfinfo_delegation, ThreadedComponent, DynamicComponentParent):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block

        def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
            FrontendScannerDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
            AggregateDevice.__init__(self)
            ThreadedComponent.__init__(self)
            DynamicComponentParent.__init__(self)

            self.listeners={}
            # self.auto_start is deprecated and is only kept for API compatibility
            # with 1.7.X and 1.8.0 devices.  This variable may be removed
            # in future releases
            self.auto_start = False
            # Instantiate the default implementations for all ports on this device
            self.port_RFInfo_in = frontend.InRFInfoPort("RFInfo_in", self)
            self.port_RFInfo_in._portLog = self._baseLog.getChildLogger('RFInfo_in', 'ports')
            self.port_DigitalTuner_in = frontend.InDigitalScanningTunerPort("DigitalTuner_in", self)
            self.port_DigitalTuner_in._portLog = self._baseLog.getChildLogger('DigitalTuner_in', 'ports')
            self.port_RFInfo_out = PortFRONTENDRFInfoOut_i(self, "RFInfo_out")
            self.port_RFInfo_out._portLog = self._baseLog.getChildLogger('RFInfo_out', 'ports')
            self.device_kind = "FRONTEND::TUNER"
            self.frontend_listener_allocation = frontend.fe_types.frontend_listener_allocation()
            self.frontend_tuner_allocation = frontend.fe_types.frontend_tuner_allocation()
            self.frontend_scanner_allocation = frontend.fe_types.frontend_scanner_allocation()


        def start(self):
            FrontendScannerDevice.start(self)
            ThreadedComponent.startThread(self, pause=self.PAUSE)

        def stop(self):
            FrontendScannerDevice.stop(self)
            if not ThreadedComponent.stopThread(self, self.TIMEOUT):
                raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")

        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._baseLog.exception("Error stopping")
            FrontendScannerDevice.releaseObject(self)

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.

        # 'FRONTEND/RFInfo' port
        class PortFRONTENDRFInfoOut(ExtendedCF__POA.QueryablePort):
            """This class is a port template for the PortFRONTENDRFInfoOut_i port and
            should not be instantiated nor modified.
            
            The expectation is that the specific port implementation will extend
            from this class instead of the base CORBA class ExtendedCF__POA.QueryablePort.
            """
            pass

        port_RFInfo_in = providesport(name="RFInfo_in",
                                      repid="IDL:FRONTEND/RFInfo:1.0",
                                      type_="data")

        port_DigitalTuner_in = providesport(name="DigitalTuner_in",
                                            repid="IDL:FRONTEND/DigitalScanningTuner:1.0",
                                            type_="control")

        port_RFInfo_out = usesport(name="RFInfo_out",
                                   repid="IDL:FRONTEND/RFInfo:1.0",
                                   type_="data")

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
        frontend_coherent_feeds = simpleseq_property(id_="FRONTEND::coherent_feeds",
                                                     name="frontend_coherent_feeds",
                                                     type_="string",
                                                     defvalue=[],
                                                     mode="readwrite",
                                                     action="external",
                                                     kinds=("allocation",))


        class frontend_tuner_status_struct_struct(frontend.default_frontend_tuner_status_struct_struct):
            scan_mode_enabled = simple_property(
                                                id_="FRONTEND::tuner_status::scan_mode_enabled",
                                                
                                                name="scan_mode_enabled",
                                                type_="boolean")
        
            supports_scan = simple_property(
                                            id_="FRONTEND::tuner_status::supports_scan",
                                            
                                            name="supports_scan",
                                            type_="boolean")
        
            def __init__(self, allocation_id_csv="", bandwidth=0.0, center_frequency=0.0, enabled=False, group_id="", rf_flow_id="", sample_rate=0.0, scan_mode_enabled=False, supports_scan=False, tuner_type=""):
                frontend.default_frontend_tuner_status_struct_struct.__init__(self, allocation_id_csv=allocation_id_csv, bandwidth=bandwidth, center_frequency=center_frequency, enabled=enabled, group_id=group_id, rf_flow_id=rf_flow_id, sample_rate=sample_rate, tuner_type=tuner_type)
                self.scan_mode_enabled = scan_mode_enabled
                self.supports_scan = supports_scan
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["allocation_id_csv"] = self.allocation_id_csv
                d["bandwidth"] = self.bandwidth
                d["center_frequency"] = self.center_frequency
                d["enabled"] = self.enabled
                d["group_id"] = self.group_id
                d["rf_flow_id"] = self.rf_flow_id
                d["sample_rate"] = self.sample_rate
                d["scan_mode_enabled"] = self.scan_mode_enabled
                d["supports_scan"] = self.supports_scan
                d["tuner_type"] = self.tuner_type
                return str(d)
        
            @classmethod
            def getId(cls):
                return "FRONTEND::tuner_status_struct"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return frontend.default_frontend_tuner_status_struct_struct.getMembers(self) + [("scan_mode_enabled",self.scan_mode_enabled),("supports_scan",self.supports_scan)]


'''uses port(s). Send logging to _portLog '''

class PortFRONTENDRFInfoOut_i(wb_receiver_base.PortFRONTENDRFInfoOut):
    def __init__(self, parent, name):
        self.parent = parent
        self.name = name
        self.outConnections = {}
        self.port_lock = threading.Lock()

    def getConnectionIds(self):
        return self.outConnections.keys()

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

    def connectPort(self, connection, connectionId):
        self.port_lock.acquire()
        try:
            port = connection._narrow(FRONTEND.RFInfo)
            self.outConnections[str(connectionId)] = port
        finally:
            self.port_lock.release()

    def disconnectPort(self, connectionId):
        self.port_lock.acquire()
        try:
            self.outConnections.pop(str(connectionId), None)
        finally:
            self.port_lock.release()

    def _get_connections(self):
        self.port_lock.acquire()
        try:
            return [ExtendedCF.UsesConnection(name, port) for name, port in self.outConnections.iteritems()]
        finally:
            self.port_lock.release()

    def rf_flow_id(self, __connection_id__ = ""):
        return self._get_rf_flow_id(__connection_id__)

    def _get_rf_flow_id(self, __connection_id__ = ""):
        retVal = ""
        self.port_lock.acquire()


        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False)
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    try:
                        retVal = port._get_rf_flow_id()
                    except Exception:
                        self.parent._baseLog.exception("The call to _get_rf_flow_id failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        return retVal

    def _set_rf_flow_id(self, data, __connection_id__ = ""):
        self.port_lock.acquire()


        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False)
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    try:
                        port._set_rf_flow_id(data)
                    except Exception:
                        self.parent._baseLog.exception("The call to _set_rf_flow_id failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

    def rfinfo_pkt(self, __connection_id__ = ""):
        return self._get_rfinfo_pkt(__connection_id__)

    def _get_rfinfo_pkt(self, __connection_id__ = ""):
        retVal = None
        self.port_lock.acquire()


        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, True, False, False)
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    try:
                        retVal = port._get_rfinfo_pkt()
                    except Exception:
                        self.parent._baseLog.exception("The call to _get_rfinfo_pkt failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        return retVal

    def _set_rfinfo_pkt(self, data, __connection_id__ = ""):
        self.port_lock.acquire()


        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False)
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    try:
                        port._set_rfinfo_pkt(data)
                    except Exception:
                        self.parent._baseLog.exception("The call to _set_rfinfo_pkt failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        # Rebind tuner status property with custom struct definition
        frontend_tuner_status = FrontendScannerDevice.frontend_tuner_status.rebind()
        frontend_tuner_status.structdef = frontend_tuner_status_struct_struct

        def frontendTunerStatusChanged(self,oldValue, newValue):
            pass

        def getTunerStatus(self,allocation_id):
            tuner_id = self.getTunerMapping(allocation_id)
            if tuner_id < 0:
                raise FRONTEND.FrontendException(("ERROR: ID: " + str(allocation_id) + " IS NOT ASSOCIATED WITH ANY TUNER!"))
            _props = self.query([CF.DataType(id='FRONTEND::tuner_status',value=_any.to_any(None))])
            return _props[0].value._v[tuner_id]._v

        def assignListener(self,listen_alloc_id, allocation_id):
            # find control allocation_id
            existing_alloc_id = allocation_id
            if self.listeners.has_key(existing_alloc_id):
                existing_alloc_id = self.listeners[existing_alloc_id]
            self.listeners[listen_alloc_id] = existing_alloc_id



        def removeListener(self,listen_alloc_id):
            if self.listeners.has_key(listen_alloc_id):
                del self.listeners[listen_alloc_id]


        def removeAllocationIdRouting(self,tuner_id):
            pass

