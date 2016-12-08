#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: msg_through.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

from ossie.component import Component
from ossie.threadedcomponent import *
from ossie.properties import simple_property
from ossie.properties import simpleseq_property
from ossie.properties import struct_property

import Queue, copy, time, threading
from ossie.resource import usesport, providesport
from ossie.events import MessageConsumerPort
from ossie.events import MessageSupplierPort

class msg_through_base(CF__POA.Resource, Component, ThreadedComponent):
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
            self.port_input = MessageConsumerPort(thread_sleep=0.1, parent = self)
            self.port_output = MessageSupplierPort()

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

        port_input = providesport(name="input",
                                  repid="IDL:ExtendedEvent/MessageEvent:1.0",
                                  type_="control")

        port_output = usesport(name="output",
                               repid="IDL:ExtendedEvent/MessageEvent:1.0",
                               type_="control")

        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
        class Foo(object):
            a = simple_property(
                                id_="a",
                                
                                type_="string")
        
            b = simple_property(
                                id_="b",
                                
                                type_="string")
        
            c = simple_property(
                                id_="c",
                                
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
                d["b"] = self.b
                d["c"] = self.c
                return str(d)
        
            @classmethod
            def getId(cls):
                return "foo"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("a",self.a),("b",self.b),("c",self.c)]

        foo = struct_property(id_="foo",
                              structdef=Foo,
                              configurationkind=("message",),
                              mode="readwrite")




