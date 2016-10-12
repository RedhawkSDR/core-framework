#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: comp_prop_special_char.spd.xml
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

class comp_prop_special_char_base(CF__POA.Resource, Component, ThreadedComponent):
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
        class MyBase(object):
            my_element = simple_property(
                                         id_="my:element",
                                         type_="string",
                                         defvalue="foo"
                                         )
        
            my_stuff = simple_property(
                                       id_="my.stuff",
                                       type_="string",
                                       defvalue="bar"
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
                d["my_element"] = self.my_element
                d["my_stuff"] = self.my_stuff
                return str(d)
        
            @classmethod
            def getId(cls):
                return "my:base"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("my_element",self.my_element),("my_stuff",self.my_stuff)]
        
        my_base = struct_property(id_="my:base",
                                  structdef=MyBase,
                                  configurationkind=("property",),
                                  mode="readwrite")
        
        class MyStr(object):
            my_seq_my_str_my_prop = simple_property(
                                                    id_="my_seq:my_str:my_prop",
                                                    type_="string",
                                                    defvalue="qwe"
                                                    )
        
            def __init__(self, my_seq_my_str_my_prop="qwe"):
                self.my_seq_my_str_my_prop = my_seq_my_str_my_prop
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["my_seq_my_str_my_prop"] = self.my_seq_my_str_my_prop
                return str(d)
        
            @classmethod
            def getId(cls):
                return "my:str"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("my_seq_my_str_my_prop",self.my_seq_my_str_my_prop)]
        
        my_seq = structseq_property(id_="my:seq",
                                    structdef=MyStr,
                                    defvalue=[],
                                    configurationkind=("property",),
                                    mode="readwrite")
        


