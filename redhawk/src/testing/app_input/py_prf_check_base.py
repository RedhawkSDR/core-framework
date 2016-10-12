#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: py_prf_check.spd.xml
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

class py_prf_check_base(CF__POA.Resource, Component, ThreadedComponent):
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

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
        some_simple = simple_property(id_="some_simple",
                                      type_="string",
                                      mode="readwrite",
                                      action="external",
                                      kinds=("property",))
        
        some_sequence = simpleseq_property(id_="some_sequence",
                                          type_="string",
                                          defvalue=[],
                                          mode="readwrite",
                                          action="external",
                                          kinds=("property",))
        
        class SomeStruct(object):
            a = simple_property(
                                id_="a",
                                type_="string")
        
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
                d["a"] = self.a
                return str(d)
        
            @classmethod
            def getId(cls):
                return "some_struct"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("a",self.a)]
        
        some_struct = struct_property(id_="some_struct",
                                      structdef=SomeStruct,
                                      configurationkind=("property",),
                                      mode="readwrite")
        
        class Foo(object):
            b = simple_property(
                                id_="b",
                                type_="string")
        
            def __init__(self, b=""):
                self.b = b
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["b"] = self.b
                return str(d)
        
            @classmethod
            def getId(cls):
                return "foo"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("b",self.b)]
        
        some_struct_seq = structseq_property(id_="some_struct_seq",
                                             structdef=Foo,
                                             defvalue=[],
                                             configurationkind=("property",),
                                             mode="readwrite")
        


