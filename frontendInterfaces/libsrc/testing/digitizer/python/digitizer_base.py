#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: digitizer.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

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
import bulkio
import frontend
from frontend import FRONTEND
from ossie.properties import struct_to_props
BOOLEAN_VALUE_HERE=False
from omniORB import any as _any
from ossie.dynamiccomponent import DynamicComponent
import RDC

class enums:
    # Enumerated values for device_reference_source_global
    class device_reference_source_global:
        INTERNAL = "INTERNAL"
        EXTERNAL = "EXTERNAL"
        MIMO = "MIMO"
        GPSDO = "GPSDO"

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

class digitizer_base(CF__POA.AggregatePlainDevice, FrontendScannerDevice, AggregateDevice, digital_scanning_tuner_delegation, rfinfo_delegation, ThreadedComponent, DynamicComponent):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block

        def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
            FrontendScannerDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
            AggregateDevice.__init__(self)
            ThreadedComponent.__init__(self)
            DynamicComponent.__init__(self)

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
            self.port_RFInfo_out = frontend.OutRFInfoPort("RFInfo_out")
            self.port_RFInfo_out._portLog = self._baseLog.getChildLogger('RFInfo_out', 'ports')
            self.port_dataSDDS_out = bulkio.OutSDDSPort("dataSDDS_out")
            self.port_dataSDDS_out._portLog = self._baseLog.getChildLogger('dataSDDS_out', 'ports')
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

        port_RFInfo_in = providesport(name="RFInfo_in",
                                      repid="IDL:FRONTEND/RFInfo:1.0",
                                      type_="data")

        port_DigitalTuner_in = providesport(name="DigitalTuner_in",
                                            repid="IDL:FRONTEND/DigitalScanningTuner:1.0",
                                            type_="control")

        port_RFInfo_out = usesport(name="RFInfo_out",
                                   repid="IDL:FRONTEND/RFInfo:1.0",
                                   type_="data")

        port_dataSDDS_out = usesport(name="dataSDDS_out",
                                     repid="IDL:BULKIO/dataSDDS:1.0",
                                     type_="data")

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
        device_reference_source_global = simple_property(id_="device_reference_source_global",
                                                         name="device_reference_source_global",
                                                         type_="string",
                                                         defvalue="INTERNAL",
                                                         mode="readwrite",
                                                         action="external",
                                                         kinds=("property",))


        clock_sync = simple_property(id_="clock_sync",
                                     type_="boolean",
                                     defvalue=False,
                                     mode="readonly",
                                     action="external",
                                     kinds=("property",),
                                     description="""Indicates whether the clock source is synchronized with the digitizer.""")


        ip_address = simple_property(id_="ip_address",
                                     type_="string",
                                     defvalue="",
                                     mode="readwrite",
                                     action="external",
                                     kinds=("property",),
                                     description="""device IP address (leave empty to search for available devices)""")


        device_mode = simple_property(id_="device_mode",
                                      name="device_mode",
                                      type_="string",
                                      defvalue="16bit",
                                      mode="readwrite",
                                      action="external",
                                      kinds=("property",),
                                      description="""16bit or 8bit""")


        frontend_coherent_feeds = simpleseq_property(id_="FRONTEND::coherent_feeds",
                                                     name="frontend_coherent_feeds",
                                                     type_="string",
                                                     defvalue=[],
                                                     mode="readwrite",
                                                     action="external",
                                                     kinds=("allocation",))


        class device_characteristics_struct(object):
            ch_name = simple_property(
                                      id_="device_characteristics::ch_name",
                                      
                                      name="ch_name",
                                      type_="string")
        
            tuner_type = simple_property(
                                         id_="device_characteristics::tuner_type",
                                         
                                         name="tuner_type",
                                         type_="string")
        
            chan_num = simple_property(
                                       id_="device_characteristics::chan_num",
                                       
                                       name="chan_num",
                                       type_="short")
        
            antenna = simple_property(
                                      id_="device_characteristics::antenna",
                                      
                                      name="antenna",
                                      type_="string")
        
            bandwidth_current = simple_property(
                                                id_="device_characteristics::bandwidth_current",
                                                
                                                name="bandwidth_current",
                                                type_="double")
        
            bandwidth_min = simple_property(
                                            id_="device_characteristics::bandwidth_min",
                                            
                                            name="bandwidth_min",
                                            type_="double")
        
            bandwidth_max = simple_property(
                                            id_="device_characteristics::bandwidth_max",
                                            
                                            name="bandwidth_max",
                                            type_="double")
        
            rate_current = simple_property(
                                           id_="device_characteristics::rate_current",
                                           
                                           name="rate_current",
                                           type_="double")
        
            rate_min = simple_property(
                                       id_="device_characteristics::rate_min",
                                       
                                       name="rate_min",
                                       type_="double")
        
            rate_max = simple_property(
                                       id_="device_characteristics::rate_max",
                                       
                                       name="rate_max",
                                       type_="double")
        
            freq_current = simple_property(
                                           id_="device_characteristics::freq_current",
                                           
                                           name="freq_current",
                                           type_="double")
        
            freq_min = simple_property(
                                       id_="device_characteristics::freq_min",
                                       
                                       name="freq_min",
                                       type_="double")
        
            freq_max = simple_property(
                                       id_="device_characteristics::freq_max",
                                       
                                       name="freq_max",
                                       type_="double")
        
            gain_current = simple_property(
                                           id_="device_characteristics::gain_current",
                                           
                                           name="gain_current",
                                           type_="double")
        
            gain_min = simple_property(
                                       id_="device_characteristics::gain_min",
                                       
                                       name="gain_min",
                                       type_="double")
        
            gain_max = simple_property(
                                       id_="device_characteristics::gain_max",
                                       
                                       name="gain_max",
                                       type_="double")
        
            clock_min = simple_property(
                                        id_="device_characteristics::clock_min",
                                        
                                        name="clock_min",
                                        type_="double")
        
            clock_max = simple_property(
                                        id_="device_characteristics::clock_max",
                                        
                                        name="clock_max",
                                        type_="double")
        
            available_antennas = simpleseq_property(
                                                    id_="device_characteristics::available_antennas",
                                                    
                                                    name="available_antennas",
                                                    type_="string",
                                                    defvalue=[]
                                                    )
        
            def __init__(self, **kw):
                """Construct an initialized instance of this struct definition"""
                for classattr in type(self).__dict__.itervalues():
                    if isinstance(classattr, (simple_property, simpleseq_property)):
                        classattr.initialize(self)
                for k,v in kw.items():
                    setattr(self,k,v)
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["ch_name"] = self.ch_name
                d["tuner_type"] = self.tuner_type
                d["chan_num"] = self.chan_num
                d["antenna"] = self.antenna
                d["bandwidth_current"] = self.bandwidth_current
                d["bandwidth_min"] = self.bandwidth_min
                d["bandwidth_max"] = self.bandwidth_max
                d["rate_current"] = self.rate_current
                d["rate_min"] = self.rate_min
                d["rate_max"] = self.rate_max
                d["freq_current"] = self.freq_current
                d["freq_min"] = self.freq_min
                d["freq_max"] = self.freq_max
                d["gain_current"] = self.gain_current
                d["gain_min"] = self.gain_min
                d["gain_max"] = self.gain_max
                d["clock_min"] = self.clock_min
                d["clock_max"] = self.clock_max
                d["available_antennas"] = self.available_antennas
                return str(d)
        
            @classmethod
            def getId(cls):
                return "device_characteristics"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("ch_name",self.ch_name),("tuner_type",self.tuner_type),("chan_num",self.chan_num),("antenna",self.antenna),("bandwidth_current",self.bandwidth_current),("bandwidth_min",self.bandwidth_min),("bandwidth_max",self.bandwidth_max),("rate_current",self.rate_current),("rate_min",self.rate_min),("rate_max",self.rate_max),("freq_current",self.freq_current),("freq_min",self.freq_min),("freq_max",self.freq_max),("gain_current",self.gain_current),("gain_min",self.gain_min),("gain_max",self.gain_max),("clock_min",self.clock_min),("clock_max",self.clock_max),("available_antennas",self.available_antennas)]

        device_characteristics = struct_property(id_="device_characteristics",
                                                 name="device_characteristics",
                                                 structdef=device_characteristics_struct,
                                                 configurationkind=("property",),
                                                 mode="readonly",
                                                 description="""Describes the channels found in the digitizer""")


        class frontend_tuner_status_struct_struct(frontend.default_frontend_tuner_status_struct_struct):
            scan_mode_enabled = simple_property(
                                                id_="FRONTEND::tuner_status::scan_mode_enabled",
                                                
                                                name="scan_mode_enabled",
                                                type_="boolean")
        
            supports_scan = simple_property(
                                            id_="FRONTEND::tuner_status::supports_scan",
                                            
                                            name="supports_scan",
                                            type_="boolean")
        
            bandwidth_tolerance = simple_property(
                                                  id_="FRONTEND::tuner_status::bandwidth_tolerance",
                                                  
                                                  name="bandwidth_tolerance",
                                                  type_="double")
        
            sample_rate_tolerance = simple_property(
                                                    id_="FRONTEND::tuner_status::sample_rate_tolerance",
                                                    
                                                    name="sample_rate_tolerance",
                                                    type_="double")
        
            def __init__(self, allocation_id_csv="", bandwidth=0.0, center_frequency=0.0, enabled=False, group_id="", rf_flow_id="", sample_rate=0.0, scan_mode_enabled=False, supports_scan=False, tuner_type="", bandwidth_tolerance=0.0, sample_rate_tolerance=0.0):
                frontend.default_frontend_tuner_status_struct_struct.__init__(self, allocation_id_csv=allocation_id_csv, bandwidth=bandwidth, center_frequency=center_frequency, enabled=enabled, group_id=group_id, rf_flow_id=rf_flow_id, sample_rate=sample_rate, tuner_type=tuner_type)
                self.scan_mode_enabled = scan_mode_enabled
                self.supports_scan = supports_scan
                self.bandwidth_tolerance = bandwidth_tolerance
                self.sample_rate_tolerance = sample_rate_tolerance
        
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
                d["bandwidth_tolerance"] = self.bandwidth_tolerance
                d["sample_rate_tolerance"] = self.sample_rate_tolerance
                return str(d)
        
            @classmethod
            def getId(cls):
                return "FRONTEND::tuner_status_struct"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return frontend.default_frontend_tuner_status_struct_struct.getMembers(self) + [("scan_mode_enabled",self.scan_mode_enabled),("supports_scan",self.supports_scan),("bandwidth_tolerance",self.bandwidth_tolerance),("sample_rate_tolerance",self.sample_rate_tolerance)]


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

