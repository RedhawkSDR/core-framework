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
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: NicExecDevice.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

from ossie.device import ExecutableDevice
from ossie.threadedcomponent import *
from ossie.properties import simple_property
from ossie.properties import simpleseq_property
from ossie.properties import struct_property
from ossie.properties import structseq_property

import Queue, copy, time, threading

class NicExecDevice_base(CF__POA.ExecutableDevice, ExecutableDevice, ThreadedComponent):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block

        def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
            ExecutableDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
            ThreadedComponent.__init__(self)

            # self.auto_start is deprecated and is only kept for API compatibility
            # with 1.7.X and 1.8.0 devices.  This variable may be removed
            # in future releases
            self.auto_start = False
            # Instantiate the default implementations for all ports on this device

        def start(self):
            ExecutableDevice.start(self)
            ThreadedComponent.startThread(self, pause=self.PAUSE)

        def stop(self):
            ExecutableDevice.stop(self)
            if not ThreadedComponent.stopThread(self, self.TIMEOUT):
                raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")

        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._log.exception("Error stopping")
            ExecutableDevice.releaseObject(self)

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
                                      defvalue="GPP",
                                      mode="readonly",
                                      action="eq",
                                      kinds=("allocation",),
                                      description="""This specifies the device kind""")


        device_model = simple_property(id_="DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb",
                                       name="device_model",
                                       type_="string",
                                       defvalue="NicExecDevice",
                                       mode="readonly",
                                       action="eq",
                                       kinds=("allocation",),
                                       description=""" This specifies the specific device""")


        processor_name = simple_property(id_="DCE:9B445600-6C7F-11d4-A226-0050DA314CD6",
                                         name="processor_name",
                                         type_="string",
                                         defvalue="x86_64",
                                         mode="readonly",
                                         action="eq",
                                         kinds=("allocation",),
                                         description="""SCA required property describing the CPU type""")


        os_name = simple_property(id_="DCE:80BF17F0-6C7F-11d4-A226-0050DA314CD6",
                                  name="os_name",
                                  type_="string",
                                  defvalue="Linux",
                                  mode="readonly",
                                  action="eq",
                                  kinds=("allocation",),
                                  description="""SCA required property describing the Operating System Name""")


        os_version = simple_property(id_="DCE:0f3a9a37-a342-43d8-9b7f-78dc6da74192",
                                     name="os_version",
                                     type_="string",
                                     mode="readonly",
                                     action="eq",
                                     kinds=("allocation",),
                                     description="""SCA required property describing the Operating System Version""")


        nic_list = simpleseq_property(id_="nic_list",
                                      type_="string",
                                      defvalue=[],
                                      mode="readonly",
                                      action="external",
                                      kinds=("property",))


        class NicAllocation(object):
            identifier = simple_property(
                                         id_="nic_allocation::identifier",
                                         name="identifier",
                                         type_="string",
                                         defvalue=""
                                         )
        
            interface = simple_property(
                                        id_="nic_allocation::interface",
                                        name="interface",
                                        type_="string",
                                        defvalue=""
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
                d["identifier"] = self.identifier
                d["interface"] = self.interface
                return str(d)
        
            @classmethod
            def getId(cls):
                return "nic_allocation"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("identifier",self.identifier),("interface",self.interface)]

        nic_allocation = struct_property(id_="nic_allocation",
                                         structdef=NicAllocation,
                                         configurationkind=("allocation",),
                                         mode="readwrite")


        class NicAllocationStatusStruct(object):
            identifier = simple_property(
                                         id_="nic_allocation_status::identifier",
                                         name="identifier",
                                         type_="string")
        
            interface = simple_property(
                                        id_="nic_allocation_status::interface",
                                        name="interface",
                                        type_="string")
        
            def __init__(self, identifier="", interface=""):
                self.identifier = identifier
                self.interface = interface
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["identifier"] = self.identifier
                d["interface"] = self.interface
                return str(d)
        
            @classmethod
            def getId(cls):
                return "nic_allocation_status_struct"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("identifier",self.identifier),("interface",self.interface)]

        nic_allocation_status = structseq_property(id_="nic_allocation_status",
                                                   structdef=NicAllocationStatusStruct,
                                                   defvalue=[],
                                                   configurationkind=("property",),
                                                   mode="readonly")




