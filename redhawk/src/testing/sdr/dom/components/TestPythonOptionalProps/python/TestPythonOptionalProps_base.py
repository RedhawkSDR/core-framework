#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: TestPythonOptionalProps.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

from ossie.component import Component
from ossie.threadedcomponent import *
from ossie.properties import simple_property
from ossie.properties import simpleseq_property
from ossie.properties import struct_property

import Queue, copy, time, threading

class TestPythonOptionalProps_base(CF__POA.Resource, Component, ThreadedComponent):
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
            
        def runTest(self, testid, properties):
            if testid == 0:
                return self.testOptional(properties)
            return []
                
        def testOptional(self, props):
            # Return optional properties that are currently set
            retProps = []
            for prop in props:
                if not prop:
                    retProps.append(prop)
            return retProps

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
        class MyStructName(object):
            struct_octet_name = simple_property(
                                                id_="struct_octet",
                                                name="struct_octet_name",
                                                type_="octet",
                                                optional=True)
        
            struct_short_name = simple_property(
                                                id_="struct_short",
                                                name="struct_short_name",
                                                type_="short",
                                                optional=True)
        
            struct_ushort_name = simple_property(
                                                 id_="struct_ushort",
                                                 name="struct_ushort_name",
                                                 type_="ushort",
                                                 optional=True)
        
            struct_long_name = simple_property(
                                               id_="struct_long",
                                               name="struct_long_name",
                                               type_="long",
                                               optional=True)
        
            struct_ulong_name = simple_property(
                                                id_="struct_ulong",
                                                name="struct_ulong_name",
                                                type_="ulong",
                                                optional=True)
        
            struct_longlong_name = simple_property(
                                                   id_="struct_longlong",
                                                   name="struct_longlong_name",
                                                   type_="longlong",
                                                   optional=True)
        
            struct_ulonglong_name = simple_property(
                                                    id_="struct_ulonglong",
                                                    name="struct_ulonglong_name",
                                                    type_="ulonglong",
                                                    optional=True)
        
            struct_string_name = simple_property(
                                                 id_="struct_string",
                                                 name="struct_string_name",
                                                 type_="string",
                                                 optional=True)
        
            struct_seq_octet_name = simpleseq_property(
                                                       id_="struct_seq_octet",
                                                       name="struct_seq_octet_name",
                                                       type_="octet",
                                                       optional=True)
        
            struct_seq_short_name = simpleseq_property(
                                                       id_="struct_seq_short",
                                                       name="struct_seq_short_name",
                                                       type_="short",
                                                       optional=True)
        
            struct_seq_ushort_name = simpleseq_property(
                                                        id_="struct_seq_ushort",
                                                        name="struct_seq_ushort_name",
                                                        type_="ushort",
                                                        optional=True)
        
            struct_seq_long_name = simpleseq_property(
                                                      id_="struct_seq_long",
                                                      name="struct_seq_long_name",
                                                      type_="long",
                                                      optional=True)
        
            struct_seq_ulong_name = simpleseq_property(
                                                       id_="struct_seq_ulong",
                                                       name="struct_seq_ulong_name",
                                                       type_="ulong",
                                                       optional=True)
        
            struct_seq_longlong_name = simpleseq_property(
                                                          id_="struct_seq_longlong",
                                                          name="struct_seq_longlong_name",
                                                          type_="longlong",
                                                          optional=True)
        
            struct_seq_ulonglong_name = simpleseq_property(
                                                           id_="struct_seq_ulonglong",
                                                           name="struct_seq_ulonglong_name",
                                                           type_="ulonglong",
                                                           optional=True)
        
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
                d["struct_octet_name"] = self.struct_octet_name
                d["struct_short_name"] = self.struct_short_name
                d["struct_ushort_name"] = self.struct_ushort_name
                d["struct_long_name"] = self.struct_long_name
                d["struct_ulong_name"] = self.struct_ulong_name
                d["struct_longlong_name"] = self.struct_longlong_name
                d["struct_ulonglong_name"] = self.struct_ulonglong_name
                d["struct_string_name"] = self.struct_string_name
                d["struct_seq_octet_name"] = self.struct_seq_octet_name
                d["struct_seq_short_name"] = self.struct_seq_short_name
                d["struct_seq_ushort_name"] = self.struct_seq_ushort_name
                d["struct_seq_long_name"] = self.struct_seq_long_name
                d["struct_seq_ulong_name"] = self.struct_seq_ulong_name
                d["struct_seq_longlong_name"] = self.struct_seq_longlong_name
                d["struct_seq_ulonglong_name"] = self.struct_seq_ulonglong_name
                return str(d)
        
            def getId(self):
                return "my_struct"
        
            def isStruct(self):
                return True
        
            def getMembers(self):
                return [("struct_octet_name",self.struct_octet_name),("struct_short_name",self.struct_short_name),("struct_ushort_name",self.struct_ushort_name),("struct_long_name",self.struct_long_name),("struct_ulong_name",self.struct_ulong_name),("struct_longlong_name",self.struct_longlong_name),("struct_ulonglong_name",self.struct_ulonglong_name),("struct_string_name",self.struct_string_name),("struct_seq_octet_name",self.struct_seq_octet_name),("struct_seq_short_name",self.struct_seq_short_name),("struct_seq_ushort_name",self.struct_seq_ushort_name),("struct_seq_long_name",self.struct_seq_long_name),("struct_seq_ulong_name",self.struct_seq_ulong_name),("struct_seq_longlong_name",self.struct_seq_longlong_name),("struct_seq_ulonglong_name",self.struct_seq_ulonglong_name)]
        
        my_struct_name = struct_property(id_="my_struct",
                                         name="my_struct_name",
                                         structdef=MyStructName,
                                         configurationkind=("configure",),
                                         mode="readwrite")
        


