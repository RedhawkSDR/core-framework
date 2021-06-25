#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: None
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

from frontend import FrontendTunerDevice
from frontend import digital_tuner_delegation
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
import bulkio
import frontend
from frontend import FRONTEND
from ossie.properties import struct_to_props
BOOLEAN_VALUE_HERE=False
from omniORB import any as _any
from ossie.dynamiccomponent import DynamicComponent

class RDC_base(CF__POA.Device, FrontendTunerDevice, digital_tuner_delegation, rfinfo_delegation, ThreadedComponent, DynamicComponent):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block

        def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
            FrontendTunerDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
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
            self.port_DigitalTuner_in = frontend.InDigitalTunerPort("DigitalTuner_in", self)
            self.port_DigitalTuner_in._portLog = self._baseLog.getChildLogger('DigitalTuner_in', 'ports')
            self.port_DeviceStatus_out = PortCFDeviceStatusOut_i(self, "DeviceStatus_out")
            self.port_DeviceStatus_out._portLog = self._baseLog.getChildLogger('DeviceStatus_out', 'ports')
            self.port_dataShort_out = bulkio.OutShortPort("dataShort_out")
            self.port_dataShort_out._portLog = self._baseLog.getChildLogger('dataShort_out', 'ports')

        def start(self):
            FrontendTunerDevice.start(self)
            ThreadedComponent.startThread(self, pause=self.PAUSE)

        def stop(self):
            FrontendTunerDevice.stop(self)
            if not ThreadedComponent.stopThread(self, self.TIMEOUT):
                raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")

        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._baseLog.exception("Error stopping")

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.

        # 'CF/DeviceStatus' port
        class PortCFDeviceStatusOut(ExtendedCF__POA.QueryablePort):
            """This class is a port template for the PortCFDeviceStatusOut_i port and
            should not be instantiated nor modified.
            
            The expectation is that the specific port implementation will extend
            from this class instead of the base CORBA class ExtendedCF__POA.QueryablePort.
            """
            pass

        port_RFInfo_in = providesport(name="RFInfo_in",
                                      repid="IDL:FRONTEND/RFInfo:1.0",
                                      type_="data")

        port_DigitalTuner_in = providesport(name="DigitalTuner_in",
                                            repid="IDL:FRONTEND/DigitalTuner:1.0",
                                            type_="control")

        port_DeviceStatus_out = usesport(name="DeviceStatus_out",
                                         repid="IDL:CF/DeviceStatus:1.0",
                                         type_="control")

        port_dataShort_out = usesport(name="dataShort_out",
                                      repid="IDL:BULKIO/dataShort:1.0",
                                      type_="data")

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
        device_kind = simple_property(id_="DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d",
                                      name="device_kind",
                                      type_="string",
                                      defvalue="FRONTEND::TUNER",
                                      mode="readonly",
                                      action="eq",
                                      kinds=("allocation",),
                                      description="""This specifies the device kind""")


        device_model = simple_property(id_="DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb",
                                       name="device_model",
                                       type_="string",
                                       mode="readonly",
                                       action="eq",
                                       kinds=("allocation",),
                                       description=""" This specifies the specific device""")


        rx_autogain_on_tune = simple_property(id_="rx_autogain_on_tune",
                                              name="rx_autogain_on_tune",
                                              type_="boolean",
                                              defvalue=False,
                                              mode="readwrite",
                                              action="external",
                                              kinds=("property",),
                                              description="""If true the device will check in the input receive levels and set the hardware device gain appropriately when a new tuner is allocated or adjusted. """)


        trigger_rx_autogain = simple_property(id_="trigger_rx_autogain",
                                              name="trigger_rx_autogain",
                                              type_="boolean",
                                              defvalue=False,
                                              mode="readwrite",
                                              action="external",
                                              kinds=("property",),
                                              description="""Setting to true will trigger an auto gain calculation where the input levels will be monitored and the hardware gain will be adjusted. After this operation the property will be set back to false. """)


        rx_autogain_guard_bits = simple_property(id_="rx_autogain_guard_bits",
                                                 name="rx_autogain_guard_bits",
                                                 type_="ushort",
                                                 defvalue=1,
                                                 mode="readwrite",
                                                 action="external",
                                                 kinds=("property",),
                                                 description="""The number of bits in the sample that are not used to express the value. This allows the signal strength to change without saturating the ADC, and prevents clipping.""")


        device_gain = simple_property(id_="device_gain",
                                      name="device_gain",
                                      type_="float",
                                      defvalue=0.0,
                                      mode="readwrite",
                                      action="external",
                                      kinds=("property",),
                                      description="""gain value""")


        rdc_only_property = simple_property(id_="rdc_only_property",
                                            name="rdc_only_property",
                                            type_="float",
                                            defvalue=0.0,
                                            mode="readwrite",
                                            action="external",
                                            kinds=("property",),
                                            description="""sample property""")


        device_mode = simple_property(id_="device_mode",
                                      name="device_mode",
                                      type_="string",
                                      defvalue="16bit",
                                      mode="readwrite",
                                      action="external",
                                      kinds=("property",),
                                      description="""16bit or 8bit""")


        frontend_listener_allocation = struct_property(id_="FRONTEND::listener_allocation",
                                                       name="frontend_listener_allocation",
                                                       structdef=frontend.frontend_listener_allocation,
                                                       configurationkind=("allocation",),
                                                       mode="writeonly",
                                                       description="""Allocation structure to acquire "listener" capability on a tuner based off a previous allocation. "Listeners" have the ability to receive the data but can not modify the settings of the tuner.""")


        frontend_tuner_allocation = struct_property(id_="FRONTEND::tuner_allocation",
                                                    name="frontend_tuner_allocation",
                                                    structdef=frontend.frontend_tuner_allocation,
                                                    configurationkind=("allocation",),
                                                    mode="writeonly",
                                                    description="""Allocation structure to acquire capability on a tuner based off tuner settings""")


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
            bandwidth_tolerance = simple_property(
                                                  id_="FRONTEND::tuner_status::bandwidth_tolerance",
                                                  
                                                  name="bandwidth_tolerance",
                                                  type_="double")
        
            sample_rate_tolerance = simple_property(
                                                    id_="FRONTEND::tuner_status::sample_rate_tolerance",
                                                    
                                                    name="sample_rate_tolerance",
                                                    type_="double")
        
            def __init__(self, allocation_id_csv="", bandwidth=0.0, center_frequency=0.0, enabled=False, group_id="", rf_flow_id="", sample_rate=0.0, tuner_type="", bandwidth_tolerance=0.0, sample_rate_tolerance=0.0):
                frontend.default_frontend_tuner_status_struct_struct.__init__(self, allocation_id_csv=allocation_id_csv, bandwidth=bandwidth, center_frequency=center_frequency, enabled=enabled, group_id=group_id, rf_flow_id=rf_flow_id, sample_rate=sample_rate, tuner_type=tuner_type)
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
                return frontend.default_frontend_tuner_status_struct_struct.getMembers(self) + [("bandwidth_tolerance",self.bandwidth_tolerance),("sample_rate_tolerance",self.sample_rate_tolerance)]

        frontend_tuner_status = structseq_property(id_="FRONTEND::tuner_status",
                                                   name="frontend_tuner_status",
                                                   structdef=frontend_tuner_status_struct_struct,
                                                   defvalue=[],
                                                   configurationkind=("property",),
                                                   mode="readonly",
                                                   description="""Status of each tuner, including entries for both allocated and un-allocated tuners. Each entry represents a single tuner.""")



'''uses port(s). Send logging to _portLog '''

class PortCFDeviceStatusOut_i(RDC_base.PortCFDeviceStatusOut):
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
            port = connection._narrow(CF.DeviceStatus)
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

    def statusChanged(self, status, __connection_id__ = ""):
        self.port_lock.acquire()


        try:
            self._evaluateRequestBasedOnConnections(__connection_id__, False, False, False)
            for connId, port in self.outConnections.items():
                if (__connection_id__ and __connection_id__ != connId):
                    continue
                if port != None:
                    try:
                        port.statusChanged(status)
                    except Exception:
                        self.parent._baseLog.exception("The call to statusChanged failed on port %s connection %s instance %s", self.name, connId, port)
                        raise
        finally:
            self.port_lock.release()

        # Rebind tuner status property with custom struct definition
        frontend_tuner_status = FrontendTunerDevice.frontend_tuner_status.rebind()
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

