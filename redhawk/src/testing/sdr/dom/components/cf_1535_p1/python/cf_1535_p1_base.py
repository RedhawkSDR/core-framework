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
# Source: P1.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

from ossie.component import Component
from ossie.threadedcomponent import *
from ossie.properties import simple_property
from ossie.properties import simpleseq_property
from ossie.properties import struct_property
from ossie.properties import structseq_property

import Queue, copy, time, threading
from ossie.resource import usesport, providesport
import bulkio

class cf_1535_p1_base(CF__POA.Resource, Component, ThreadedComponent):
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
            self.port_d1 = bulkio.InLongPort("d1", maxsize=self.DEFAULT_QUEUE_SIZE)
            self.port_d3 = bulkio.OutLongPort("d3")

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

        port_d1 = providesport(name="d1",
                               repid="IDL:BULKIO/dataLong:1.0",
                               type_="data")

        port_d3 = usesport(name="d3",
                           repid="IDL:BULKIO/dataLong:1.0",
                           type_="data")

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
        p1 = simple_property(id_="p1",
                             name="p1",
                             type_="string",
                             mode="readwrite",
                             action="external",
                             kinds=("property",),
                             description="""can you dig it { ( [""")


        window = simple_property(id_="window",
                                 name="window",
                                 type_="string",
                                 mode="readwrite",
                                 action="external",
                                 kinds=("property",))


        freq = simple_property(id_="freq",
                               name="freq",
                               type_="float",
                               defvalue=0.0,
                               mode="readwrite",
                               action="external",
                               kinds=("property",))


        s1 = simpleseq_property(id_="s1",
                                name="s1",
                                type_="long",
                                defvalue=[],
                                mode="readwrite",
                                action="external",
                                kinds=("property",))


        class St1(object):
            a1 = simple_property(
                                 id_="st1::a1",
                                 name="a1",
                                 type_="string")
        
            b1 = simple_property(
                                 id_="st1::b1",
                                 name="b1",
                                 type_="float")
        
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
                d["a1"] = self.a1
                d["b1"] = self.b1
                return str(d)
        
            @classmethod
            def getId(cls):
                return "st1"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("a1",self.a1),("b1",self.b1)]

        st1 = struct_property(id_="st1",
                              name="st1",
                              structdef=St1,
                              configurationkind=("property",),
                              mode="readwrite")


        class Ss1St1(object):
            a1 = simple_property(
                                 id_="ss1::st1::a1",
                                 name="a1",
                                 type_="string")
        
            b1 = simple_property(
                                 id_="ss1::st1::b1",
                                 name="b1",
                                 type_="float")
        
            def __init__(self, a1="", b1=0.0):
                self.a1 = a1
                self.b1 = b1
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["a1"] = self.a1
                d["b1"] = self.b1
                return str(d)
        
            @classmethod
            def getId(cls):
                return "ss1::st1"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("a1",self.a1),("b1",self.b1)]

        ss1 = structseq_property(id_="ss1",
                                 name="ss1",
                                 structdef=Ss1St1,
                                 defvalue=[],
                                 configurationkind=("property",),
                                 mode="readwrite")




