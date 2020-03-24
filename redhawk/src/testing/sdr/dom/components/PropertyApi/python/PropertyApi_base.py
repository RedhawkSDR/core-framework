#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: PropertyApi.spd.xml
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
from ossie.resource import usesport, providesport, PortCallError
import bulkio

class PropertyApi_base(CF__POA.Resource, Component, ThreadedComponent):
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
            self.port_dataFloat_in = bulkio.InFloatPort("dataFloat_in", maxsize=self.DEFAULT_QUEUE_SIZE)
            self.port_dataFloat_in._portLog = self._baseLog.getChildLogger('dataFloat_in', 'ports')
            self.port_dataFloat_out = bulkio.OutFloatPort("dataFloat_out")
            self.port_dataFloat_out._portLog = self._baseLog.getChildLogger('dataFloat_out', 'ports')

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
                self._baseLog.exception("Error stopping")
            Component.releaseObject(self)

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.

        port_dataFloat_in = providesport(name="dataFloat_in",
                                         repid="IDL:BULKIO/dataFloat:1.0",
                                         type_="data")

        port_dataFloat_out = usesport(name="dataFloat_out",
                                      repid="IDL:BULKIO/dataFloat:1.0",
                                      type_="data")

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
        simp_seq_c = simpleseq_property(id_="simp_seq_c",
                                        name="simp_seq_c",
                                        type_="string",
                                        defvalue=["c_1", "c_2"                                             ],
                                        mode="readwrite",
                                        action="external",
                                        kinds=("property",))


        simp_seq_d = simpleseq_property(id_="simp_seq_d",
                                        name="simp_seq_d",
                                        type_="string",
                                        defvalue=["d_1", "d_2"                                             ],
                                        mode="readwrite",
                                        action="external",
                                        kinds=("property",))


        class StructA(object):
            simple_a = simple_property(
                                       id_="struct_a::simple_a",
                                       
                                       name="simple_a",
                                       type_="string",
                                       defvalue="simp_a"
                                       )
        
            simpseq_a1 = simpleseq_property(
                                            id_="struct_a::simpseq_a1",
                                            
                                            name="simpseq_a1",
                                            type_="string",
                                            defvalue=["a1_1", "a2_2"]
                                            )
        
            simpseq_a2 = simpleseq_property(
                                            id_="struct_a::simpseq_a2",
                                            
                                            name="simpseq_a2",
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
                d["simple_a"] = self.simple_a
                d["simpseq_a1"] = self.simpseq_a1
                d["simpseq_a2"] = self.simpseq_a2
                return str(d)
        
            @classmethod
            def getId(cls):
                return "struct_a"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("simple_a",self.simple_a),("simpseq_a1",self.simpseq_a1),("simpseq_a2",self.simpseq_a2)]

        struct_a = struct_property(id_="struct_a",
                                   name="struct_a",
                                   structdef=StructA,
                                   configurationkind=("property",),
                                   mode="readwrite")


        class StructB(object):
            simple_b = simple_property(
                                       id_="struct_seq_b::simple_b",
                                       
                                       name="simple_b",
                                       type_="string",
                                       defvalue="simp_b"
                                       )
        
            simp_seq_b1 = simpleseq_property(
                                             id_="struct_seq_b::simp_seq_b1",
                                             
                                             name="simp_seq_b1",
                                             type_="string",
                                             defvalue=[]
                                             )
        
            simp_seq_b2 = simpleseq_property(
                                             id_="struct_seq_b::simp_seq_b2",
                                             
                                             name="simp_seq_b2",
                                             type_="string",
                                             defvalue=["b2_1", "b2_2"]
                                             )
        
            def __init__(self, simple_b="simp_b", simp_seq_b1=[], simp_seq_b2=["b2_1", "b2_2"]):
                self.simple_b = simple_b
                self.simp_seq_b1 = simp_seq_b1
                self.simp_seq_b2 = simp_seq_b2
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["simple_b"] = self.simple_b
                d["simp_seq_b1"] = self.simp_seq_b1
                d["simp_seq_b2"] = self.simp_seq_b2
                return str(d)
        
            @classmethod
            def getId(cls):
                return "struct_seq_b::struct_b"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("simple_b",self.simple_b),("simp_seq_b1",self.simp_seq_b1),("simp_seq_b2",self.simp_seq_b2)]

        struct_seq_b = structseq_property(id_="struct_seq_b",
                                          name="struct_seq_b",
                                          structdef=StructB,
                                          defvalue=[StructB(simple_b="simp_b",simp_seq_b1=[""],simp_seq_b2=["b2_1","b2_2"])],
                                          configurationkind=("property",),
                                          mode="readwrite")


        class StructE(object):
            simple_e = simple_property(
                                       id_="struct_seq_e::simple_e",
                                       
                                       name="simple_e",
                                       type_="string",
                                       defvalue="simp_e"
                                       )
        
            simpseq_e1 = simpleseq_property(
                                            id_="struct_seq_e::simpseq_e1",
                                            
                                            name="simpseq_e1",
                                            type_="string",
                                            defvalue=["e1_1", "e1_2"]
                                            )
        
            simpseq_e2 = simpleseq_property(
                                            id_="struct_seq_e::simpseq_e2",
                                            
                                            name="simpseq_e2",
                                            type_="string",
                                            defvalue=[]
                                            )
        
            def __init__(self, simple_e="simp_e", simpseq_e1=["e1_1", "e1_2"], simpseq_e2=[]):
                self.simple_e = simple_e
                self.simpseq_e1 = simpseq_e1
                self.simpseq_e2 = simpseq_e2
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["simple_e"] = self.simple_e
                d["simpseq_e1"] = self.simpseq_e1
                d["simpseq_e2"] = self.simpseq_e2
                return str(d)
        
            @classmethod
            def getId(cls):
                return "struct_seq_e::struct_e"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("simple_e",self.simple_e),("simpseq_e1",self.simpseq_e1),("simpseq_e2",self.simpseq_e2)]

        struct_seq_e = structseq_property(id_="struct_seq_e",
                                          name="struct_seq_e",
                                          structdef=StructE,
                                          defvalue=[StructE(simple_e="simp_e",simpseq_e1=["e1_1","e1_2"],simpseq_e2=[""]),StructE(simple_e="simp_e",simpseq_e1=["e1_1","e1_2"],simpseq_e2=[""])],
                                          configurationkind=("property",),
                                          mode="readwrite")




