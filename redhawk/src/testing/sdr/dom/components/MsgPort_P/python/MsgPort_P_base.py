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
# Source: MsgPort_P.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

from ossie.component import Component
from ossie.threadedcomponent import *
from ossie.properties import simple_property
from ossie.properties import simpleseq_property
from ossie.properties import struct_property

import queue, copy, time, threading
from ossie.resource import usesport, providesport
from ossie.events import MessageSupplierPort

class MsgPort_P_base(CF__POA.Resource, Component, ThreadedComponent):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block

        def __init__(self, identifier, execparams):
            loggerName = (execparams['NAME_BINDING'].replace('/', '.')).rsplit("_", 1)[0]
            Component.__init__(self, identifier, execparams, loggerName=loggerName)
            ThreadedComponent.__init__(self)

            # self.auto_start is deprecated and is only kept for API compatibility
            # with 1.7.X and 1.8.0 components.  This variable may be removed
            # in future releases
            self.auto_start = False
            # Instantiate the default implementations for all ports on this component
            self.port_msg_out = MessageSupplierPort()

        def start(self):
            Component.start(self)
            ThreadedComponent.startThread(self, pause=self.PAUSE)

        def stop(self):
            Component.stop(self)
            if not ThreadedComponent.stopThread(self, self.TIMEOUT):
                raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")

        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._log.exception("Error stopping")
            Component.releaseObject(self)

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.

        port_msg_out = usesport(name="msg_out",
                                repid="IDL:ExtendedEvent/MessageEvent:1.0",
                                type_="control")

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
        class MsgOut(object):
            field1 = simple_property(
                                     id_="field1",
                                     name="field1",
                                     type_="string")
        
            field2 = simple_property(
                                     id_="field2",
                                     name="field2",
                                     type_="long")
        
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
                d["field1"] = self.field1
                d["field2"] = self.field2
                return str(d)
        
            @classmethod
            def getId(cls):
                return "msg_out"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("field1",self.field1),("field2",self.field2)]

        msg_out = struct_property(id_="msg_out",
                                  structdef=MsgOut,
                                  configurationkind=("message",),
                                  mode="readwrite")




