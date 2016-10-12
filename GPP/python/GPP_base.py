#!/usr/bin/env python
#
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
#

#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: GPP.spd.xml
# Generated on: Mon Dec 09 18:17:47 EST 2013
# REDHAWK IDE
# Version: 1.8.4
# Build id: R201305151907
from ossie.cf import CF, CF__POA
from ossie.utils import uuid

from ossie.device import ExecutableDevice , AggregateDevice
from ossie.properties import simple_property
from ossie.properties import simpleseq_property
from ossie.properties import struct_property
from ossie.properties import structseq_property

import Queue, copy, time, threading
from ossie.resource import usesport
from ossie.events import PropertyEventSupplier

NOOP = -1
NORMAL = 0
FINISH = 1
class ProcessThread(threading.Thread):
    def __init__(self, target, pause=0.0125):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self.target = target
        self.pause = pause
        self.stop_signal = threading.Event()

    def stop(self):
        self.stop_signal.set()

    def updatePause(self, pause):
        self.pause = pause

    def run(self):
        state = NORMAL
        while (state != FINISH) and (not self.stop_signal.isSet()):
            state = self.target()
            if (state == NOOP):
                # If there was no data to process sleep to avoid spinning
                time.sleep(self.pause)

class GPP_base(CF__POA.AggregateExecutableDevice,ExecutableDevice, AggregateDevice):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block
        
        def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
            ExecutableDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
            AggregateDevice.__init__(self)
            self.threadControlLock = threading.RLock()
            self.process_thread = None
            # self.auto_start is deprecated and is only kept for API compatability
            # with 1.7.X and 1.8.0 components.  This variable may be removed
            # in future releases
            self.auto_start = False
            
        def initialize(self):
            ExecutableDevice.initialize(self)
            
            # Instantiate the default implementations for all ports on this component

            self.port_propEvent = PropertyEventSupplier(self)

        def start(self):
            self.threadControlLock.acquire()
            try:
                ExecutableDevice.start(self)
                if self.process_thread == None:
                    self.process_thread = ProcessThread(target=self.process, pause=self.PAUSE)
                    self.process_thread.start()
            finally:
                self.threadControlLock.release()

        def process(self):
            """The process method should process a single "chunk" of data and then return.  This method will be called
            from the processing thread again, and again, and again until it returns FINISH or stop() is called on the
            component.  If no work is performed, then return NOOP"""
            raise NotImplementedError

        def stop(self):
            self.threadControlLock.acquire()
            try:
                process_thread = self.process_thread
                self.process_thread = None

                if process_thread != None:
                    process_thread.stop()
                    process_thread.join(self.TIMEOUT)
                    if process_thread.isAlive():
                        raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")
                ExecutableDevice.stop(self)
            finally:
                self.threadControlLock.release()

        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._log.exception("Error stopping")
            self.threadControlLock.acquire()
            try:
                ExecutableDevice.releaseObject(self)
            finally:
                self.threadControlLock.release()

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.
        
        def compareSRI(self, a, b):
            if a.hversion != b.hversion:
                return False
            if a.xstart != b.xstart:
                return False
            if a.xdelta != b.xdelta:
                return False
            if a.xunits != b.xunits:
                return False
            if a.subsize != b.subsize:
                return False
            if a.ystart != b.ystart:
                return False
            if a.ydelta != b.ydelta:
                return False
            if a.yunits != b.yunits:
                return False
            if a.mode != b.mode:
                return False
            if a.streamID != b.streamID:
                return False
            if a.blocking != b.blocking:
                return False
            if len(a.keywords) != len(b.keywords):
                return False
            for keyA, keyB in zip(a.keywords, b.keywords):
                if keyA.value._t != keyB.value._t:
                    return False
                if keyA.value._v != keyB.value._v:
                    return False
            return True


        port_propEvent = usesport(name="propEvent",
                                            repid="IDL:omg.org/CosEventChannelAdmin/EventChannel:1.0",
                                            type_="responses",)        

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.       
        hostName = simple_property(id_="DCE:9190eb70-bd1e-4556-87ee-5a259dcfee39",
                                          name="hostName", 
                                          type_="string",
                                          defvalue="localhost",
                                          mode="readonly",
                                          action="external",
                                          kinds=("configure",),
                                          description="""Host name on which the device is deployed""" 
                                          )       
        DeviceKind = simple_property(id_="DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d",
                                          name="DeviceKind", 
                                          type_="string",
                                          defvalue="GPP",
                                          mode="readonly",
                                          action="eq",
                                          kinds=("configure","allocation"),
                                          description="""The type of this device""" 
                                          )       
        os_name = simple_property(id_="DCE:4a23ad60-0b25-4121-a630-68803a498f75",
                                          name="os_name", 
                                          type_="string",
                                          defvalue="Linux",
                                          mode="readwrite",
                                          action="eq",
                                          kinds=("execparam","allocation"),
                                          description="""SCA required property describing the Operating System Name.""" 
                                          )       
        os_version = simple_property(id_="DCE:0f3a9a37-a342-43d8-9b7f-78dc6da74192",
                                          name="os_version", 
                                          type_="string",
                                          mode="readwrite",
                                          action="eq",
                                          kinds=("allocation","execparam"),
                                          description="""SCA required property describing the Operating System Version""" 
                                          )       
        processor_name = simple_property(id_="DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b",
                                          name="processor_name", 
                                          type_="string",
                                          mode="readwrite",
                                          action="eq",
                                          kinds=("allocation","execparam"),
                                          description="""SCA required property describing the CPU type""" 
                                          )       
        processor_cores = simple_property(id_="DCE:2df4cfe4-675c-41ec-9cc8-84dff2f468b3",
                                          name="processor_cores", 
                                          type_="long",
                                          mode="readonly",
                                          action="external",
                                          kinds=("allocation",),
                                          description="""The number of true processing cores that can execute programs concurrently (i.e. not hyperthreaded).  Although external, implemented with behavior identical to a matching property with action 'le'""" 
                                          )       
        memTotal = simple_property(id_="DCE:329d9304-839e-4fec-a36f-989e3b4d311d",
                                          name="memTotal", 
                                          type_="long",
                                          units="MiB", 
                                          mode="readonly",
                                          action="external",
                                          kinds=("configure",),
                                          description="""Total amount of RAM installed in the GPP""" 
                                          )       
        memFree = simple_property(id_="DCE:6565bffd-cb09-4927-9385-2ecac68035c7",
                                          name="memFree", 
                                          type_="long",
                                          units="MiB", 
                                          mode="readonly",
                                          action="external",
                                          kinds=("configure","event"),
                                          description="""Amount of RAM in the GPP not in use (measured)""" 
                                          )       
        memCapacity = simple_property(id_="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb",
                                          name="memCapacity", 
                                          type_="long",
                                          units="MiB", 
                                          mode="readwrite",
                                          action="external",
                                          kinds=("allocation","event"),
                                          description="""Amount of RAM in the GPP not allocated to an application""" 
                                          )       
        memThreshold = simple_property(id_="DCE:fc24e19d-eda9-4200-ae96-8ba2ed953128",
                                          name="memThreshold", 
                                          type_="long",
                                          defvalue=80,
                                          units="%", 
                                          mode="readwrite",
                                          action="external",
                                          kinds=("execparam",),
                                          description="""Percentage of total system memory this GPP can use for capacity management.""" 
                                          )       
        mcastnicInterface = simple_property(id_="DCE:4e416acc-3144-47eb-9e38-97f1d24f7700",
                                          name="mcastnicInterface", 
                                          type_="string",
                                          defvalue="",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("execparam",),
                                          description="""The Multicast NIC interface associated with this GPP (e.g. eth1).  If not provided no multicast allocations are permitted.""" 
                                          )       
        mcastnicIngressTotal = simple_property(id_="DCE:5a41c2d3-5b68-4530-b0c4-ae98c26c77ec",
                                          name="mcastnicIngressTotal", 
                                          type_="long",
                                          defvalue=0,
                                          units="Mb/s", 
                                          mode="readwrite",
                                          action="external",
                                          kinds=("execparam",),
                                          description="""Total NIC bandwidth for the interfaces defined in mcastnicInterface.  This must be specified in the PRF or DCD because ethtool requires super-user privs.""" 
                                          )       
        mcastnicEgressTotal = simple_property(id_="DCE:442d5014-2284-4f46-86ae-ce17e0749da0",
                                          name="mcastnicEgressTotal", 
                                          type_="long",
                                          defvalue=0,
                                          units="Mb/s", 
                                          mode="readwrite",
                                          action="external",
                                          kinds=("execparam",),
                                          description="""Total NIC bandwidth for the interfaces defined in mcastnicInterface.  This must be specified in the PRF or DCD because ethtool requires super-user privs.""" 
                                          )       
        mcastnicIngressCapacity = simple_property(id_="DCE:506102d6-04a9-4532-9420-a323d818ddec",
                                          name="mcastnicIngressCapacity", 
                                          type_="long",
                                          units="Mb/s", 
                                          mode="readwrite",
                                          action="external",
                                          kinds=("allocation","event"),
                                          description="""Amount of ingress multicast NIC capacity in the GPP not allocated to an application""" 
                                          )       
        mcastnicEgressCapacity = simple_property(id_="DCE:eb08e43f-11c7-45a0-8750-edff439c8b24",
                                          name="mcastnicEgressCapacity", 
                                          type_="long",
                                          units="Mb/s", 
                                          mode="readwrite",
                                          action="external",
                                          kinds=("allocation","event"),
                                          description="""Amount of egress multicast NIC capacity in the GPP not allocated to an application""" 
                                          )       
        mcastnicIngressFree = simple_property(id_="DCE:0b57a27a-8fa2-412b-b0ae-010618b8f40e",
                                          name="mcastnicIngressFree", 
                                          type_="long",
                                          defvalue=0,
                                          units="Mb/s", 
                                          mode="readonly",
                                          action="external",
                                          kinds=("configure","event"),
                                          description="""Free NIC bandwidth for the interfaces defined in mcastnicInterface.""" 
                                          )       
        mcastnicEgressFree = simple_property(id_="DCE:9b5bbdcb-1894-4b95-847c-787f121c05ae",
                                          name="mcastnicEgressFree", 
                                          type_="long",
                                          defvalue=0,
                                          units="Mb/s", 
                                          mode="readonly",
                                          action="external",
                                          kinds=("configure","event"),
                                          description="""Free NIC bandwidth for the interfaces defined in mcastnicInterface.""" 
                                          )       
        mcastnicThreshold = simple_property(id_="DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1",
                                          name="mcastnicThreshold", 
                                          type_="long",
                                          defvalue=80,
                                          units="%", 
                                          mode="readwrite",
                                          action="external",
                                          kinds=("execparam",),
                                          description="""Percentage of total Multicast NIC this GPP can use for capacity management.""" 
                                          )       
        loadTotal = simple_property(id_="DCE:28b23bc8-e4c0-421b-9c52-415a24715209",
                                          name="loadTotal", 
                                          type_="double",
                                          mode="readonly",
                                          action="external",
                                          kinds=("configure",),
                                          description="""Equal to "processor_cores" x "loadCapacityPerCore".""" 
                                          )       
        loadCapacityPerCore = simple_property(id_="DCE:3bf07b37-0c00-4e2a-8275-52bd4e391f07",
                                          name="loadCapacityPerCore", 
                                          type_="double",
                                          defvalue=1.0,
                                          mode="readwrite",
                                          action="gt",
                                          kinds=("execparam","allocation"),
                                          description="""The performance ratio of this machine, relative to the benchmark machine.""" 
                                          )       
        loadFree = simple_property(id_="DCE:6c000787-6fea-4765-8686-2e051e6c24b0",
                                          name="loadFree", 
                                          type_="double",
                                          mode="readonly",
                                          action="external",
                                          kinds=("configure","event"),
                                          description="""Equal to loadCapacity""" 
                                          )       
        loadCapacity = simple_property(id_="DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056",
                                          name="loadCapacity", 
                                          type_="double",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("allocation","event"),
                                          description="""The amount of load capacity remaining to be allocated.""" 
                                          )       
        loadThreshold = simple_property(id_="DCE:22a60339-b66e-4309-91ae-e9bfed6f0490",
                                          name="loadThreshold", 
                                          type_="long",
                                          defvalue=80,
                                          units="%", 
                                          mode="readwrite",
                                          action="external",
                                          kinds=("execparam",),
                                          description="""Percentage of toal load that can be used for capacity.""" 
                                          )       
        diskThreshold = simple_property(id_="DCE:b911fa00-e411-4eb6-93d4-fff12dcf03bc",
                                          name="diskThreshold", 
                                          type_="long",
                                          defvalue=80,
                                          units="%", 
                                          mode="readwrite",
                                          action="external",
                                          kinds=("execparam",),
                                          description="""Percentage of total disk this GPP can use for capacity management.""" 
                                          )       
        useScreen = simple_property(id_="DCE:218e612c-71a7-4a73-92b6-bf70959aec45",
                                          name="useScreen", 
                                          type_="boolean",
                                          defvalue=False,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("execparam",),
                                          description="""If true, GNU screen will be used for the execution of components.""" 
                                          )       
        componentOutputLog = simple_property(id_="DCE:c80f6c5a-e3ea-4f57-b0aa-46b7efac3176",
                                          name="componentOutputLog", 
                                          type_="string",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",),
                                          description="""If provided, all component output will be redirected to this file.  The GPP will not delete or rotate these logs.  The provided value may contain environment variables or reference component exec-params with @EXEC_PARAM@.  For example, this would be a valid value $SDRROOT/logs/@COMPONENT_IDENTIFIER@.log""" 
                                          )       
        propertyEventRate = simple_property(id_="propertyEventRate",
                                          type_="ushort",
                                          defvalue=60,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("execparam",),
                                          description="""The rate at which property events will be emitted for back-ground status""" 
                                          ) 
        processor_flags = simpleseq_property(id_="DCE:7c922411-5085-4a89-992d-0a0a51767aea",
                                          name="processor_flags",   
                                          type_="string",
                                          defvalue=None,
                                          mode="readonly",
                                          action="external",
                                          kinds=("allocation",),
                                          description="""The flags associated with the CPU, for e.g., 'sse2'""" 
                                          ) 
        mcastnicVLANs = simpleseq_property(id_="DCE:65544aad-4c73-451f-93de-d4d76984025a",
                                          name="mcastnicVLANs",   
                                          type_="long",
                                          defvalue=None,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("allocation",),
                                          description="""When queired, returns the list of vlans on this host.  When used as an allocation, defines the list of VLANS the component requires.""" 
                                          )
        class LoadAverage(object):
            onemin = simple_property(id_="onemin",
                                          name="onemin", 
                                          type_="double",
                                          )
            fivemin = simple_property(id_="fivemin",
                                          name="fivemin", 
                                          type_="double",
                                          )
            fifteenmin = simple_property(id_="fifteenmin",
                                          name="fifteenmin", 
                                          type_="double",
                                          )
        
            def __init__(self, **kw):
                """Construct an initialized instance of this struct definition"""
                for attrname, classattr in type(self).__dict__.items():
                    if type(classattr) == simple_property:
                        classattr.initialize(self)
                for k,v in kw.items():
                    setattr(self,k,v)

            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["onemin"] = self.onemin
                d["fivemin"] = self.fivemin
                d["fifteenmin"] = self.fifteenmin
                return str(d)

            def getId(self):
                return "DCE:9da85ebc-6503-48e7-af36-b77c7ad0c2b4"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("onemin",self.onemin),("fivemin",self.fivemin),("fifteenmin",self.fifteenmin)]

        
        loadAverage = struct_property(id_="DCE:9da85ebc-6503-48e7-af36-b77c7ad0c2b4",
                                          name="loadAverage", 
                                          structdef=LoadAverage,
                                          configurationkind=("configure","event"),
                                          mode="readonly",
                                          description="""The current load average, as reported by /proc/loadavg.  Each core on a computer can have a load average between 0.0 and 1.0.  This differs greatly from CPU percentage (as reported by top).  Load averages differ in two significant ways: 1) they measure the trend of CPU utlization, and 2) they include all demand for the CPU not only how much was active at the time of measurement.  Load averages do not include any processes or threads waiting on I/O, networking, databases, or anything else not demanding the CPU.""" 
                                          )
        class Filesystem(object):
            device = simple_property(id_="device",
                                          name="device", 
                                          type_="string",
                                          )
            mount = simple_property(id_="mount",
                                          name="mount", 
                                          type_="string",
                                          )
            total = simple_property(id_="total",
                                          name="total", 
                                          type_="ulong",
                                          )
            used = simple_property(id_="used",
                                          name="used", 
                                          type_="ulong",
                                          )
            available = simple_property(id_="available",
                                          name="available", 
                                          type_="ulong",
                                          )
        
            def __init__(self, device="", mount="", total=0, used=0, available=0):
                self.device = device
                self.mount = mount
                self.total = total
                self.used = used
                self.available = available

            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["device"] = self.device
                d["mount"] = self.mount
                d["total"] = self.total
                d["used"] = self.used
                d["available"] = self.available
                return str(d)

            def getId(self):
                return "filesystem"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("device",self.device),("mount",self.mount),("total",self.total),("used",self.used),("available",self.available)]

                
        fileSystems = structseq_property(id_="DCE:f5f78038-b7d4-4fcd-8294-344369c8a74f",
                                          name="fileSystems", 
                                          structdef=Filesystem,                          
                                          defvalue=[],
                                          configurationkind=("configure","event"),
                                          mode="readonly",
                                          description="""The list of file systems mounted on this host and their current usage.""" 
                                          )
        class DiskCapacity(object):
            diskCapacityPath = simple_property(id_="diskCapacityPath",
                                          name="diskCapacityPath", 
                                          type_="string",
                                          )
            diskCapacity = simple_property(id_="diskCapacity",
                                          name="diskCapacity", 
                                          type_="ulong",
                                          )
        
            def __init__(self, diskCapacityPath="", diskCapacity=0):
                self.diskCapacityPath = diskCapacityPath
                self.diskCapacity = diskCapacity

            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["diskCapacityPath"] = self.diskCapacityPath
                d["diskCapacity"] = self.diskCapacity
                return str(d)

            def getId(self):
                return "diskCapacityStruct"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("diskCapacityPath",self.diskCapacityPath),("diskCapacity",self.diskCapacity)]

                
        diskCapacity = structseq_property(id_="DCE:6786dd11-1e30-4910-aaac-a92b8b82614c",
                                          name="diskCapacity", 
                                          structdef=DiskCapacity,                          
                                          defvalue=[],
                                          configurationkind=("allocation","event"),
                                          mode="readwrite",
                                          description="""Used as an allocatation dependency to ensure the specified path has enough disk space.""" 
                                          )
        class DiskrateTotal(object):
            diskrateTotalDevice = simple_property(id_="diskrateTotalDevice",
                                          name="diskrateTotalDevice", 
                                          type_="string",
                                          )
            diskrateTotal = simple_property(id_="diskrateTotal",
                                          name="diskrateTotal", 
                                          type_="string",
                                          )
        
            def __init__(self, diskrateTotalDevice="", diskrateTotal=""):
                self.diskrateTotalDevice = diskrateTotalDevice
                self.diskrateTotal = diskrateTotal

            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["diskrateTotalDevice"] = self.diskrateTotalDevice
                d["diskrateTotal"] = self.diskrateTotal
                return str(d)

            def getId(self):
                return "diskrateTotalStruct"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("diskrateTotalDevice",self.diskrateTotalDevice),("diskrateTotal",self.diskrateTotal)]

                
        diskrateTotal = structseq_property(id_="DCE:8c79aea8-479c-4b9b-98ab-efbb89305750",
                                          name="diskrateTotal", 
                                          structdef=DiskrateTotal,                          
                                          defvalue=[],
                                          configurationkind=("allocation",),
                                          mode="readwrite"
                                          )
        class DiskrateCapacity(object):
            diskrateCapacityPath = simple_property(id_="diskrateCapacityPath",
                                          name="diskrateCapacityPath", 
                                          type_="string",
                                          )
            diskrateCapacity = simple_property(id_="diskrateCapacity",
                                          name="diskrateCapacity", 
                                          type_="string",
                                          )
        
            def __init__(self, diskrateCapacityPath="", diskrateCapacity=""):
                self.diskrateCapacityPath = diskrateCapacityPath
                self.diskrateCapacity = diskrateCapacity

            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["diskrateCapacityPath"] = self.diskrateCapacityPath
                d["diskrateCapacity"] = self.diskrateCapacity
                return str(d)

            def getId(self):
                return "diskrateCapacityStruct"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("diskrateCapacityPath",self.diskrateCapacityPath),("diskrateCapacity",self.diskrateCapacity)]

                
        diskrateCapacity = structseq_property(id_="DCE:41c4eecf-bd8c-4904-a1ac-b617d87df67b",
                                          name="diskrateCapacity", 
                                          structdef=DiskrateCapacity,                          
                                          defvalue=[],
                                          configurationkind=("allocation","event"),
                                          mode="readwrite"
                                          )

'''uses port(s)'''

