#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
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
# Source: multiout_attachable.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

from ossie.resource import Resource
from ossie.threadedcomponent import *
from ossie.properties import simple_property
from ossie.properties import struct_property
from ossie.properties import structseq_property

import Queue, copy, time, threading
from ossie.resource import usesport, providesport
import bulkio

class multiout_attachable_base(CF__POA.Resource, Resource, ThreadedComponent):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block

        def __init__(self, identifier, execparams):
            loggerName = (execparams['NAME_BINDING'].replace('/', '.')).rsplit("_", 1)[0]
            Resource.__init__(self, identifier, execparams, loggerName=loggerName)
            ThreadedComponent.__init__(self)

            # self.auto_start is deprecated and is only kept for API compatibility
            # with 1.7.X and 1.8.0 components.  This variable may be removed
            # in future releases
            self.auto_start = False
            # Instantiate the default implementations for all ports on this component
            self.port_dataSDDS_in = bulkio.InSDDSPort("dataSDDS_in")
            self.port_dataVITA49_in = bulkio.InVITA49Port("dataVITA49_in")
            self.port_dataFloat_in = bulkio.InFloatPort("dataFloat_in", maxsize=self.DEFAULT_QUEUE_SIZE)
            self.port_dataSDDS_out = bulkio.OutSDDSPort("dataSDDS_out")
            self.port_dataVITA49_out = bulkio.OutVITA49Port("dataVITA49_out")
            self.addPropertyChangeListener('connectionTable',self.updated_connectionTable)

        def start(self):
            Resource.start(self)
            ThreadedComponent.startThread(self, pause=self.PAUSE)

        def updated_connectionTable(self, id, oldval, newval):
            self.port_dataSDDS_out.updateConnectionFilter(newval)
            self.port_dataVITA49_out.updateConnectionFilter(newval)

        def stop(self):
            if not ThreadedComponent.stopThread(self, self.TIMEOUT):
                raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")
            Resource.stop(self)

        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._log.exception("Error stopping")
            Resource.releaseObject(self)

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.

        port_dataSDDS_in = providesport(name="dataSDDS_in",
                                        repid="IDL:BULKIO/dataSDDS:1.0",
                                        type_="control")

        port_dataVITA49_in = providesport(name="dataVITA49_in",
                                          repid="IDL:BULKIO/dataVITA49:1.0",
                                          type_="control")

        port_dataFloat_in = providesport(name="dataFloat_in",
                                         repid="IDL:BULKIO/dataFloat:1.0",
                                         type_="control")

        port_dataSDDS_out = usesport(name="dataSDDS_out",
                                     repid="IDL:BULKIO/dataSDDS:1.0",
                                     type_="control")

        port_dataVITA49_out = usesport(name="dataVITA49_out",
                                       repid="IDL:BULKIO/dataVITA49:1.0",
                                       type_="control")

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
        packets_ingested = simple_property(id_="packets_ingested",
                                           name="packets_ingested",
                                           type_="ushort",
                                           defvalue=0,
                                           mode="readwrite",
                                           action="external",
                                           kinds=("configure",))
        
        class CallbackStats(object):
            num_sdds_attaches = simple_property(id_="num_sdds_attaches",
                                                type_="ushort")
        
            num_sdds_detaches = simple_property(id_="num_sdds_detaches",
                                                type_="ushort")
        
            num_vita49_attaches = simple_property(id_="num_vita49_attaches",
                                                  type_="ushort")
        
            num_vita49_detaches = simple_property(id_="num_vita49_detaches",
                                                  type_="ushort")
        
            num_new_sri_callbacks = simple_property(id_="num_new_sri_callbacks",
                                                    type_="ushort")
        
            num_sri_change_callbacks = simple_property(id_="num_sri_change_callbacks",
                                                       type_="ushort")
        
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
                d["num_sdds_attaches"] = self.num_sdds_attaches
                d["num_sdds_detaches"] = self.num_sdds_detaches
                d["num_vita49_attaches"] = self.num_vita49_attaches
                d["num_vita49_detaches"] = self.num_vita49_detaches
                d["num_new_sri_callbacks"] = self.num_new_sri_callbacks
                d["num_sri_change_callbacks"] = self.num_sri_change_callbacks
                return str(d)
        
            def getId(self):
                return "callback_stats"
        
            def isStruct(self):
                return True
        
            def getMembers(self):
                return [("num_sdds_attaches",self.num_sdds_attaches),("num_sdds_detaches",self.num_sdds_detaches),("num_vita49_attaches",self.num_vita49_attaches),("num_vita49_detaches",self.num_vita49_detaches),("num_new_sri_callbacks",self.num_new_sri_callbacks),("num_sri_change_callbacks",self.num_sri_change_callbacks)]
        
        callback_stats = struct_property(id_="callback_stats",
                                         structdef=CallbackStats,
                                         configurationkind=("configure",),
                                         mode="readwrite")
        
        connectionTable = structseq_property(id_="connectionTable",
                                             structdef=bulkio.connection_descriptor_struct,
                                             defvalue=[],
                                             configurationkind=("configure",),
                                             mode="readwrite")
        
        class SDDSStreamDefinition(object):
            id = simple_property(id_="sdds::id",
                                 name="id",
                                 type_="string")
        
            multicastAddress = simple_property(id_="sdds::multicastAddress",
                                               name="multicastAddress",
                                               type_="string",
                                               defvalue="0.0.0.0")
        
            vlan = simple_property(id_="sdds::vlan",
                                   name="vlan",
                                   type_="ulong")
        
            port = simple_property(id_="sdds::port",
                                   name="port",
                                   type_="ulong")
        
            sampleRate = simple_property(id_="sdds::sampleRate",
                                         name="sampleRate",
                                         type_="ulong")
        
            timeTagValid = simple_property(id_="sdds::timeTagValid",
                                           name="timeTagValid",
                                           type_="boolean")
        
            privateInfo = simple_property(id_="sdds::privateInfo",
                                          name="privateInfo",
                                          type_="string")
        
            def __init__(self, id="", multicastAddress="0.0.0.0", vlan=0, port=0, sampleRate=0, timeTagValid=False, privateInfo=""):
                self.id = id
                self.multicastAddress = multicastAddress
                self.vlan = vlan
                self.port = port
                self.sampleRate = sampleRate
                self.timeTagValid = timeTagValid
                self.privateInfo = privateInfo
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["id"] = self.id
                d["multicastAddress"] = self.multicastAddress
                d["vlan"] = self.vlan
                d["port"] = self.port
                d["sampleRate"] = self.sampleRate
                d["timeTagValid"] = self.timeTagValid
                d["privateInfo"] = self.privateInfo
                return str(d)
        
            def getId(self):
                return "SDDSStreamDefinition"
        
            def isStruct(self):
                return True
        
            def getMembers(self):
                return [("id",self.id),("multicastAddress",self.multicastAddress),("vlan",self.vlan),("port",self.port),("sampleRate",self.sampleRate),("timeTagValid",self.timeTagValid),("privateInfo",self.privateInfo)]
        
        SDDSStreamDefinitions = structseq_property(id_="SDDSStreamDefinitions",
                                                   structdef=SDDSStreamDefinition,
                                                   defvalue=[],
                                                   configurationkind=("configure",),
                                                   mode="readwrite")
        
        class VITA49StreamDefinition(object):
            id = simple_property(id_="vita49::id",
                                 name="id",
                                 type_="string")
        
            ip_address = simple_property(id_="vita49::ip_address",
                                         name="ip_address",
                                         type_="string",
                                         defvalue="0.0.0.0")
        
            vlan = simple_property(id_="vita49::vlan",
                                   name="vlan",
                                   type_="ulong")
        
            port = simple_property(id_="vita49::port",
                                   name="port",
                                   type_="ulong")
        
            valid_data_format = simple_property(id_="vita49::valid_data_format",
                                                name="valid_data_format",
                                                type_="boolean")
        
            packing_method_processing_efficient = simple_property(id_="vita49::packing_method_processing_efficient",
                                                                  name="packing_method_processing_efficient",
                                                                  type_="boolean")
        
            repeating = simple_property(id_="vita49::repeating",
                                        name="repeating",
                                        type_="boolean")
        
            event_tag_size = simple_property(id_="vita49::event_tag_size",
                                             name="event_tag_size",
                                             type_="long")
        
            channel_tag_size = simple_property(id_="vita49::channel_tag_size",
                                               name="channel_tag_size",
                                               type_="long")
        
            item_packing_field_size = simple_property(id_="vita49::item_packing_field_size",
                                                      name="item_packing_field_size",
                                                      type_="long")
        
            data_item_size = simple_property(id_="vita49::data_item_size",
                                             name="data_item_size",
                                             type_="long")
        
            repeat_count = simple_property(id_="vita49::repeat_count",
                                           name="repeat_count",
                                           type_="long")
        
            vector_size = simple_property(id_="vita49::vector_size",
                                          name="vector_size",
                                          type_="long")
        
            def __init__(self, id="", ip_address="0.0.0.0", vlan=0, port=0, valid_data_format=False, packing_method_processing_efficient=False, repeating=False, event_tag_size=0, channel_tag_size=0, item_packing_field_size=0, data_item_size=0, repeat_count=0, vector_size=0):
                self.id = id
                self.ip_address = ip_address
                self.vlan = vlan
                self.port = port
                self.valid_data_format = valid_data_format
                self.packing_method_processing_efficient = packing_method_processing_efficient
                self.repeating = repeating
                self.event_tag_size = event_tag_size
                self.channel_tag_size = channel_tag_size
                self.item_packing_field_size = item_packing_field_size
                self.data_item_size = data_item_size
                self.repeat_count = repeat_count
                self.vector_size = vector_size
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["id"] = self.id
                d["ip_address"] = self.ip_address
                d["vlan"] = self.vlan
                d["port"] = self.port
                d["valid_data_format"] = self.valid_data_format
                d["packing_method_processing_efficient"] = self.packing_method_processing_efficient
                d["repeating"] = self.repeating
                d["event_tag_size"] = self.event_tag_size
                d["channel_tag_size"] = self.channel_tag_size
                d["item_packing_field_size"] = self.item_packing_field_size
                d["data_item_size"] = self.data_item_size
                d["repeat_count"] = self.repeat_count
                d["vector_size"] = self.vector_size
                return str(d)
        
            def getId(self):
                return "VITA49StreamDefinition"
        
            def isStruct(self):
                return True
        
            def getMembers(self):
                return [("id",self.id),("ip_address",self.ip_address),("vlan",self.vlan),("port",self.port),("valid_data_format",self.valid_data_format),("packing_method_processing_efficient",self.packing_method_processing_efficient),("repeating",self.repeating),("event_tag_size",self.event_tag_size),("channel_tag_size",self.channel_tag_size),("item_packing_field_size",self.item_packing_field_size),("data_item_size",self.data_item_size),("repeat_count",self.repeat_count),("vector_size",self.vector_size)]
        
        VITA49StreamDefinitions = structseq_property(id_="VITA49StreamDefinitions",
                                                     structdef=VITA49StreamDefinition,
                                                     defvalue=[],
                                                     configurationkind=("configure",),
                                                     mode="readwrite")
        
        class SddsAttachment(object):
            streamId = simple_property(id_="sdds::streamId",
                                       name="streamId",
                                       type_="string")
        
            attachId = simple_property(id_="sdds::attachId",
                                       name="attachId",
                                       type_="string")
        
            port = simple_property(id_="sdds::rec_port",
                                   name="port",
                                   type_="ulong")
        
            def __init__(self, streamId="", attachId="", port=0):
                self.streamId = streamId
                self.attachId = attachId
                self.port = port
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["streamId"] = self.streamId
                d["attachId"] = self.attachId
                d["port"] = self.port
                return str(d)
        
            def getId(self):
                return "sdds_attachment"
        
            def isStruct(self):
                return True
        
            def getMembers(self):
                return [("streamId",self.streamId),("attachId",self.attachId),("port",self.port)]
        
        received_sdds_attachments = structseq_property(id_="received_sdds_attachments",
                                                       structdef=SddsAttachment,
                                                       defvalue=[],
                                                       configurationkind=("configure",),
                                                       mode="readwrite")
        
        class Vita49Attachment(object):
            streamId = simple_property(id_="vita49::streamId",
                                       name="streamId",
                                       type_="string")
        
            attachId = simple_property(id_="vita49::attachId",
                                       name="attachId",
                                       type_="string")
        
            port = simple_property(id_="vita49::rec_port",
                                   name="port",
                                   type_="ulong")
        
            def __init__(self, streamId="", attachId="", port=0):
                self.streamId = streamId
                self.attachId = attachId
                self.port = port
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["streamId"] = self.streamId
                d["attachId"] = self.attachId
                d["port"] = self.port
                return str(d)
        
            def getId(self):
                return "vita49_attachment"
        
            def isStruct(self):
                return True
        
            def getMembers(self):
                return [("streamId",self.streamId),("attachId",self.attachId),("port",self.port)]
        
        received_vita49_attachments = structseq_property(id_="received_vita49_attachments",
                                                         structdef=Vita49Attachment,
                                                         defvalue=[],
                                                         configurationkind=("configure",),
                                                         mode="readwrite")
        

