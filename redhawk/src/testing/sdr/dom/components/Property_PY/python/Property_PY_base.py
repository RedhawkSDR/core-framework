#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: Property_PY.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

from ossie.component import Component
from ossie.threadedcomponent import *
from ossie.properties import simple_property
from ossie.properties import simpleseq_property
from ossie.properties import struct_property

import Queue, copy, time, threading

class Property_PY_base(CF__POA.Resource, Component, ThreadedComponent):
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
        p1 = simple_property(id_="p1",
                             type_="string",
                             mode="readwrite",
                             action="external",
                             kinds=("property",))
        
        p2 = simple_property(id_="p2",
                             type_="double",
                             mode="readwrite",
                             action="external",
                             kinds=("property",))
        
        p3 = simple_property(id_="p3",
                             type_="long",
                             mode="readwrite",
                             action="external",
                             kinds=("property",))
        
        class P4(object):
            p4sub1 = simple_property(
                                     id_="p4sub1",
                                     type_="string")
        
            p4sub2 = simple_property(
                                     id_="p4sub2",
                                     type_="float")
        
            def __init__(self, **kw):
                """Construct an initialized instance of this struct definition"""
                for attrname, classattr in type(self).__dict__.items():
                    if type(classattr) == simple_property or type(classattr) == simpleseq_property:
                        classattr.initialize(self)
                for k,v in kw.items():
                    setattr(self,k,v)
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["p4sub1"] = self.p4sub1
                d["p4sub2"] = self.p4sub2
                return str(d)
        
            def getId(self):
                return "p4"
        
            def isStruct(self):
                return True
        
            def getMembers(self):
                return [("p4sub1",self.p4sub1),("p4sub2",self.p4sub2)]
        
        p4 = struct_property(id_="p4",
                             structdef=P4,
                             configurationkind=("property",),
                             mode="readwrite")
        


