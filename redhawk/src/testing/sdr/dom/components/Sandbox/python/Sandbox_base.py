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
# Source: Sandbox.spd.xml
# Generated on: Thu Jan 03 09:36:31 EST 2013
# Redhawk IDE
# Version:M.1.8.2
# Build id: v201211201139RC3
from ossie.cf import CF, CF__POA
from ossie.utils import uuid

from ossie.resource import Resource
from ossie.properties import simple_property
from ossie.properties import simpleseq_property
from ossie.properties import struct_property
from ossie.properties import structseq_property

import Queue, copy, time, threading

NOOP = -1
NORMAL = 0
FINISH = 1
class ProcessThread(threading.Thread):
    def __init__(self, target, pause=0.0125):
        threading.Thread.__init__(self)
        self.setDaemon(True)
        self.target = target
        self.pause = pause
        self.stop_signal = threading.Event()

    def stop(self):
        self.stop_signal.set()

    def updatePause(self, pause):
        self.pause = pause

    def run(self):
        state = NORMAL
        while (state != FINISH) and (not self.stop_signal.isSet()):
            state = self.target()
            if (state == NOOP):
                # If there was no data to process sleep to avoid spinning
                time.sleep(self.pause)

class Sandbox_base(CF__POA.Resource, Resource):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block
        
        def __init__(self, identifier, execparams):
            loggerName = (execparams['NAME_BINDING'].replace('/', '.')).rsplit("_", 1)[0]
            Resource.__init__(self, identifier, execparams, loggerName=loggerName)
            self.threadControlLock = threading.RLock()
            self.process_thread = None
            # self.auto_start is deprecated and is only kept for API compatability
            # with 1.7.X and 1.8.0 components.  This variable may be removed
            # in future releases
            self.auto_start = False
            
        def initialize(self):
            Resource.initialize(self)
            
            # Instantiate the default implementations for all ports on this component


        def start(self):
            self.threadControlLock.acquire()
            try:
                Resource.start(self)
                if self.process_thread == None:
                    self.process_thread = ProcessThread(target=self.process, pause=self.PAUSE)
                    self.process_thread.start()
            finally:
                self.threadControlLock.release()

        def process(self):
            """The process method should process a single "chunk" of data and then return.  This method will be called
            from the processing thread again, and again, and again until it returns FINISH or stop() is called on the
            component.  If no work is performed, then return NOOP"""
            raise NotImplementedError

        def stop(self):
            self.threadControlLock.acquire()
            try:
                process_thread = self.process_thread
                self.process_thread = None

                if process_thread != None:
                    process_thread.stop()
                    process_thread.join(self.TIMEOUT)
                    if process_thread.isAlive():
                        raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")
                Resource.stop(self)
            finally:
                self.threadControlLock.release()

        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._log.exception("Error stopping")
            self.threadControlLock.acquire()
            try:
                Resource.releaseObject(self)
            finally:
                self.threadControlLock.release()

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
        my_bool_true = simple_property(id_="my_bool_true",
                                          type_="boolean",
                                          defvalue=True,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        my_bool_false = simple_property(id_="my_bool_false",
                                          type_="boolean",
                                          defvalue=False,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        my_bool_empty = simple_property(id_="my_bool_empty",
                                          type_="boolean",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        my_long = simple_property(id_="my_long",
                                          type_="long",
                                          defvalue=10,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        my_long_empty = simple_property(id_="my_long_empty",
                                          type_="long",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        my_str = simple_property(id_="my_str",
                                          type_="string",
                                          defvalue="Hello World!",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        my_str_empty = simple_property(id_="my_str_empty",
                                          type_="string",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        my_long_enum = simple_property(id_="my_long_enum",
                                          type_="long",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        my_str_enum = simple_property(id_="my_str_enum",
                                          type_="string",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        my_bool_enum = simple_property(id_="my_bool_enum",
                                          type_="boolean",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        escape__simple = simple_property(id_="escape::simple",
                                          type_="long",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        readonly_simp = simple_property(id_="readonly_simp",
                                          type_="string",
                                          defvalue="Read only simple prop",
                                          mode="readonly",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        my_seq_bool = simpleseq_property(id_="my_seq_bool",  
                                          type_="boolean",
                                          defvalue=(True,False),
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        my_seq_str = simpleseq_property(id_="my_seq_str",  
                                          type_="string",
                                          defvalue=("one","","three"),
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        readonly_seq = simpleseq_property(id_="readonly_seq",  
                                          type_="string",
                                          defvalue=("read only","sequence property"),
                                          mode="readonly",
                                          action="external",
                                          kinds=("configure",)
                                          )
        class MyStruct(object):
            bool_true = simple_property(id_="bool_true",
                                          type_="boolean",
                                          defvalue=True,
                                          )
            bool_false = simple_property(id_="bool_false",
                                          type_="boolean",
                                          defvalue=False,
                                          )
            bool_empty = simple_property(id_="bool_empty",
                                          type_="boolean",
                                          )
            long_s = simple_property(id_="long_s",
                                          type_="long",
                                          )
            str_s = simple_property(id_="str_s",
                                          type_="string",
                                          )
            enum_bool = simple_property(id_="enum_bool",
                                          type_="boolean",
                                          )
            enum_str = simple_property(id_="enum_str",
                                          type_="string",
                                          )
            enum_long = simple_property(id_="enum_long",
                                          type_="long",
                                          )
            es__3 = simple_property(id_="es::3",
                                          type_="long",
                                          )
        
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
                d["bool_true"] = self.bool_true
                d["bool_false"] = self.bool_false
                d["bool_empty"] = self.bool_empty
                d["long_s"] = self.long_s
                d["str_s"] = self.str_s
                d["enum_bool"] = self.enum_bool
                d["enum_str"] = self.enum_str
                d["enum_long"] = self.enum_long
                d["es__3"] = self.es__3
                return str(d)

            def getId(self):
                return "my_struct"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("bool_true",self.bool_true),("bool_false",self.bool_false),("bool_empty",self.bool_empty),("long_s",self.long_s),("str_s",self.str_s),("enum_bool",self.enum_bool),("enum_str",self.enum_str),("enum_long",self.enum_long),("es__3",self.es__3)]

        
        my_struct = struct_property(id_="my_struct",
                                          structdef=MyStruct,
                                          configurationkind=("configure",),
                                          mode="readwrite"
                                          )
        class EscapeStruct(object):
            es__1 = simple_property(id_="es::1",
                                          type_="long",
                                          )
            es__2 = simple_property(id_="es::2",
                                          type_="long",
                                          )
            normal = simple_property(id_="normal",
                                          type_="long",
                                          )
        
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
                d["es__1"] = self.es__1
                d["es__2"] = self.es__2
                d["normal"] = self.normal
                return str(d)

            def getId(self):
                return "escape::struct"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("es__1",self.es__1),("es__2",self.es__2),("normal",self.normal)]

        
        escape__struct = struct_property(id_="escape::struct",
                                          structdef=EscapeStruct,
                                          configurationkind=("configure",),
                                          mode="readwrite"
                                          )
        class ReadonlyStruct(object):
            readonly_struct_simp = simple_property(id_="readonly_struct_simp",
                                          type_="string",
                                          defvalue="read only struct property",
                                          )
        
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
                d["readonly_struct_simp"] = self.readonly_struct_simp
                return str(d)

            def getId(self):
                return "readonly_struct"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("readonly_struct_simp",self.readonly_struct_simp)]

        
        readonly_struct = struct_property(id_="readonly_struct",
                                          structdef=ReadonlyStruct,
                                          configurationkind=("configure",),
                                          mode="readonly"
                                          )
        class StructGood(object):
            simp__bad = simple_property(id_="simp::bad",
                                          type_="long",
                                          )
            simp_bool = simple_property(id_="simp_bool",
                                          type_="boolean",
                                          )
        
            def __init__(self, simp__bad=0, simp_bool=False):
                self.simp__bad = simp__bad
                self.simp_bool = simp_bool

            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["simp__bad"] = self.simp__bad
                d["simp_bool"] = self.simp_bool
                return str(d)

            def getId(self):
                return "struct_good"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("simp__bad",self.simp__bad),("simp_bool",self.simp_bool)]

                
        my_struct_seq = structseq_property(id_="my_struct_seq",
                                          structdef=StructGood,                          
                                          defvalue=[StructGood(0,False),StructGood(0,False)],
                                          configurationkind=("configure",),
                                          mode="readwrite"
                                          )
        class EsSs(object):
            val__1 = simple_property(id_="val::1",
                                          type_="long",
                                          )
            val_2 = simple_property(id_="val_2",
                                          type_="string",
                                          )
        
            def __init__(self, val__1=0, val_2=""):
                self.val__1 = val__1
                self.val_2 = val_2

            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["val__1"] = self.val__1
                d["val_2"] = self.val_2
                return str(d)

            def getId(self):
                return "es::ss"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("val__1",self.val__1),("val_2",self.val_2)]

                
        escape__structseq = structseq_property(id_="escape::structseq",
                                          structdef=EsSs,                          
                                          defvalue=[EsSs(0,""),EsSs(0,"")],
                                          configurationkind=("configure",),
                                          mode="readwrite"
                                          )
        class ReadonlySs(object):
            readonly_s = simple_property(id_="readonly_s",
                                          type_="string",
                                          )
        
            def __init__(self, readonly_s=""):
                self.readonly_s = readonly_s

            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["readonly_s"] = self.readonly_s
                return str(d)

            def getId(self):
                return "readonly_ss"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("readonly_s",self.readonly_s)]

                
        readonly_structseq = structseq_property(id_="readonly_structseq",
                                          structdef=ReadonlySs,                          
                                          defvalue=[ReadonlySs("read only"),ReadonlySs("struct seq property")],
                                          configurationkind=("configure",),
                                          mode="readonly"
                                          )
