#!/usr/bin/env python3
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: wb_receiver.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

from ossie.device import Device
from ossie.device import AggregateDevice
from ossie.threadedcomponent import *
from ossie.properties import simple_property
from ossie.properties import simpleseq_property
from ossie.properties import struct_property
from ossie.properties import structseq_property

import queue, copy, time, threading
from ossie.dynamiccomponent import DynamicComponent
import anothersimple
import supersimple

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



class wb_receiver_base(CF__POA.AggregatePlainDevice, Device, AggregateDevice, ThreadedComponent, DynamicComponent):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block

        def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
            Device.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
            AggregateDevice.__init__(self)
            ThreadedComponent.__init__(self)
            DynamicComponent.__init__(self)

            # self.auto_start is deprecated and is only kept for API compatibility
            # with 1.7.X and 1.8.0 devices.  This variable may be removed
            # in future releases
            self.auto_start = False
            # Instantiate the default implementations for all ports on this device

        def start(self):
            Device.start(self)
            ThreadedComponent.startThread(self, pause=self.PAUSE)

        def stop(self):
            Device.stop(self)
            if not ThreadedComponent.stopThread(self, self.TIMEOUT):
                raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")

        def releaseObject(self):
            try:
                self.stop()
            except Exception as e:
                self._baseLog.exception("Error stopping")
            Device.releaseObject(self)

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
        device_kind = simple_property(id_="DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d",
                                      name="device_kind",
                                      type_="string",
                                      defvalue="FRONTEND::PARENT",
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


        frontend_coherent_feeds = simpleseq_property(id_="FRONTEND::coherent_feeds",
                                                     name="frontend_coherent_feeds",
                                                     type_="string",
                                                     defvalue=[],
                                                     mode="readwrite",
                                                     action="external",
                                                     kinds=("allocation",))


        class FrontendListenerAllocation(object):
            existing_allocation_id = simple_property(
                                                     id_="FRONTEND::listener_allocation::existing_allocation_id",

                                                     name="existing_allocation_id",
                                                     type_="string")

            listener_allocation_id = simple_property(
                                                     id_="FRONTEND::listener_allocation::listener_allocation_id",

                                                     name="listener_allocation_id",
                                                     type_="string")

            def __init__(self, **kw):
                """Construct an initialized instance of this struct definition"""
                for classattr in type(self).__dict__.values():
                    if isinstance(classattr, (simple_property, simpleseq_property)):
                        classattr.initialize(self)
                for k,v in list(kw.items()):
                    setattr(self,k,v)

            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["existing_allocation_id"] = self.existing_allocation_id
                d["listener_allocation_id"] = self.listener_allocation_id
                return str(d)

            @classmethod
            def getId(cls):
                return "FRONTEND::listener_allocation"

            @classmethod
            def isStruct(cls):
                return True

            def getMembers(self):
                return [("existing_allocation_id",self.existing_allocation_id),("listener_allocation_id",self.listener_allocation_id)]


        frontend_listener_allocation = struct_property(id_="FRONTEND::listener_allocation",
                                                       name="frontend_listener_allocation",
                                                       structdef=FrontendListenerAllocation,
                                                       configurationkind=("allocation",),
                                                       mode="writeonly",
                                                       description="""Allocation structure to acquire "listener" capability on a tuner based off a previous allocation. "Listeners" have the ability to receive the data but can not modify the settings of the tuner.""")


        class FrontendTunerAllocation(object):
            tuner_type = simple_property(
                                         id_="FRONTEND::tuner_allocation::tuner_type",

                                         name="tuner_type",
                                         type_="string")

            allocation_id = simple_property(
                                            id_="FRONTEND::tuner_allocation::allocation_id",

                                            name="allocation_id",
                                            type_="string")

            center_frequency = simple_property(
                                               id_="FRONTEND::tuner_allocation::center_frequency",

                                               name="center_frequency",
                                               type_="double")

            bandwidth = simple_property(
                                        id_="FRONTEND::tuner_allocation::bandwidth",

                                        name="bandwidth",
                                        type_="double")

            bandwidth_tolerance = simple_property(
                                                  id_="FRONTEND::tuner_allocation::bandwidth_tolerance",

                                                  name="bandwidth_tolerance",
                                                  type_="double")

            sample_rate = simple_property(
                                          id_="FRONTEND::tuner_allocation::sample_rate",

                                          name="sample_rate",
                                          type_="double")

            sample_rate_tolerance = simple_property(
                                                    id_="FRONTEND::tuner_allocation::sample_rate_tolerance",

                                                    name="sample_rate_tolerance",
                                                    type_="double")

            device_control = simple_property(
                                             id_="FRONTEND::tuner_allocation::device_control",

                                             name="device_control",
                                             type_="boolean")

            group_id = simple_property(
                                       id_="FRONTEND::tuner_allocation::group_id",

                                       name="group_id",
                                       type_="string")

            rf_flow_id = simple_property(
                                         id_="FRONTEND::tuner_allocation::rf_flow_id",

                                         name="rf_flow_id",
                                         type_="string")

            def __init__(self, **kw):
                """Construct an initialized instance of this struct definition"""
                for classattr in type(self).__dict__.values():
                    if isinstance(classattr, (simple_property, simpleseq_property)):
                        classattr.initialize(self)
                for k,v in list(kw.items()):
                    setattr(self,k,v)

            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["tuner_type"] = self.tuner_type
                d["allocation_id"] = self.allocation_id
                d["center_frequency"] = self.center_frequency
                d["bandwidth"] = self.bandwidth
                d["bandwidth_tolerance"] = self.bandwidth_tolerance
                d["sample_rate"] = self.sample_rate
                d["sample_rate_tolerance"] = self.sample_rate_tolerance
                d["device_control"] = self.device_control
                d["group_id"] = self.group_id
                d["rf_flow_id"] = self.rf_flow_id
                return str(d)

            @classmethod
            def getId(cls):
                return "FRONTEND::tuner_allocation"

            @classmethod
            def isStruct(cls):
                return True

            def getMembers(self):
                return [("tuner_type",self.tuner_type),("allocation_id",self.allocation_id),("center_frequency",self.center_frequency),("bandwidth",self.bandwidth),("bandwidth_tolerance",self.bandwidth_tolerance),("sample_rate",self.sample_rate),("sample_rate_tolerance",self.sample_rate_tolerance),("device_control",self.device_control),("group_id",self.group_id),("rf_flow_id",self.rf_flow_id)]


        frontend_tuner_allocation = struct_property(id_="FRONTEND::tuner_allocation",
                                                    name="frontend_tuner_allocation",
                                                    structdef=FrontendTunerAllocation,
                                                    configurationkind=("allocation",),
                                                    mode="writeonly",
                                                    description="""Allocation structure to acquire capability on a tuner based off tuner settings""")


        class FrontendScannerAllocation(object):
            min_freq = simple_property(
                                       id_="FRONTEND::scanner_allocation::min_freq",

                                       name="min_freq",
                                       type_="double")

            max_freq = simple_property(
                                       id_="FRONTEND::scanner_allocation::max_freq",

                                       name="max_freq",
                                       type_="double")

            mode = simple_property(
                                   id_="FRONTEND::scanner_allocation::mode",

                                   name="mode",
                                   type_="string")

            control_mode = simple_property(
                                           id_="FRONTEND::scanner_allocation::control_mode",

                                           name="control_mode",
                                           type_="string")

            control_limit = simple_property(
                                            id_="FRONTEND::scanner_allocation::control_limit",

                                            name="control_limit",
                                            type_="double")

            def __init__(self, **kw):
                """Construct an initialized instance of this struct definition"""
                for classattr in type(self).__dict__.values():
                    if isinstance(classattr, (simple_property, simpleseq_property)):
                        classattr.initialize(self)
                for k,v in list(kw.items()):
                    setattr(self,k,v)

            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["min_freq"] = self.min_freq
                d["max_freq"] = self.max_freq
                d["mode"] = self.mode
                d["control_mode"] = self.control_mode
                d["control_limit"] = self.control_limit
                return str(d)

            @classmethod
            def getId(cls):
                return "FRONTEND::scanner_allocation"

            @classmethod
            def isStruct(cls):
                return True

            def getMembers(self):
                return [("min_freq",self.min_freq),("max_freq",self.max_freq),("mode",self.mode),("control_mode",self.control_mode),("control_limit",self.control_limit)]


        frontend_scanner_allocation = struct_property(id_="FRONTEND::scanner_allocation",
                                                      name="frontend_scanner_allocation",
                                                      structdef=FrontendScannerAllocation,
                                                      configurationkind=("allocation",),
                                                      mode="writeonly")


        class FrontendTunerStatusStruct(object):
            allocation_id_csv = simple_property(
                                                id_="FRONTEND::tuner_status::allocation_id_csv",

                                                name="allocation_id_csv",
                                                type_="string")

            bandwidth = simple_property(
                                        id_="FRONTEND::tuner_status::bandwidth",

                                        name="bandwidth",
                                        type_="double")

            center_frequency = simple_property(
                                               id_="FRONTEND::tuner_status::center_frequency",

                                               name="center_frequency",
                                               type_="double")

            enabled = simple_property(
                                      id_="FRONTEND::tuner_status::enabled",

                                      name="enabled",
                                      type_="boolean")

            group_id = simple_property(
                                       id_="FRONTEND::tuner_status::group_id",

                                       name="group_id",
                                       type_="string")

            rf_flow_id = simple_property(
                                         id_="FRONTEND::tuner_status::rf_flow_id",

                                         name="rf_flow_id",
                                         type_="string")

            sample_rate = simple_property(
                                          id_="FRONTEND::tuner_status::sample_rate",

                                          name="sample_rate",
                                          type_="double")

            scan_mode_enabled = simple_property(
                                                id_="FRONTEND::tuner_status::scan_mode_enabled",

                                                name="scan_mode_enabled",
                                                type_="boolean")

            supports_scan = simple_property(
                                            id_="FRONTEND::tuner_status::supports_scan",

                                            name="supports_scan",
                                            type_="boolean")

            tuner_type = simple_property(
                                         id_="FRONTEND::tuner_status::tuner_type",

                                         name="tuner_type",
                                         type_="string")

            def __init__(self, allocation_id_csv="", bandwidth=0.0, center_frequency=0.0, enabled=False, group_id="", rf_flow_id="", sample_rate=0.0, scan_mode_enabled=False, supports_scan=False, tuner_type=""):
                self.allocation_id_csv = allocation_id_csv
                self.bandwidth = bandwidth
                self.center_frequency = center_frequency
                self.enabled = enabled
                self.group_id = group_id
                self.rf_flow_id = rf_flow_id
                self.sample_rate = sample_rate
                self.scan_mode_enabled = scan_mode_enabled
                self.supports_scan = supports_scan
                self.tuner_type = tuner_type

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
                return [("allocation_id_csv",self.allocation_id_csv),("bandwidth",self.bandwidth),("center_frequency",self.center_frequency),("enabled",self.enabled),("group_id",self.group_id),("rf_flow_id",self.rf_flow_id),("sample_rate",self.sample_rate),("scan_mode_enabled",self.scan_mode_enabled),("supports_scan",self.supports_scan),("tuner_type",self.tuner_type)]


        frontend_tuner_status = structseq_property(id_="FRONTEND::tuner_status",
                                                   name="frontend_tuner_status",
                                                   structdef=FrontendTunerStatusStruct,
                                                   defvalue=[],
                                                   configurationkind=("property",),
                                                   mode="readonly",
                                                   description="""Status of each tuner, including entries for both allocated and un-allocated tuners. Each entry represents a single tuner.""")




