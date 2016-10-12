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
# Source: TestAllPropTypes.spd.xml
# Generated on: Tue May 07 10:59:44 EDT 2013
# REDHAWK IDE
# Version: R.1.8.3
# Build id: v201303122306
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

class TestAllPropTypes_base(CF__POA.Resource, Resource):
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
        simple_string = simple_property(id_="simple_string",
                                          type_="string",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        simple_boolean = simple_property(id_="simple_boolean",
                                          type_="boolean",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        simple_ulong = simple_property(id_="simple_ulong",
                                          type_="ulong",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
#        simple_objref = simple_property(id_="simple_objref",
#                                          type_="objref",
#                                          mode="readwrite",
#                                          action="external",
#                                          kinds=("configure",)
#                                          )       
        simple_short = simple_property(id_="simple_short",
                                          type_="short",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        simple_float = simple_property(id_="simple_float",
                                          type_="float",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        simple_octet = simple_property(id_="simple_octet",
                                          type_="octet",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        simple_char = simple_property(id_="simple_char",
                                          type_="char",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        simple_ushort = simple_property(id_="simple_ushort",
                                          type_="ushort",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        simple_double = simple_property(id_="simple_double",
                                          type_="double",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        simple_long = simple_property(id_="simple_long",
                                          type_="long",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        simple_longlong = simple_property(id_="simple_longlong",
                                          type_="longlong",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        simple_ulonglong = simple_property(id_="simple_ulonglong",
                                          type_="ulonglong",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        simple_sequence_string = simpleseq_property(id_="simple_sequence_string",  
                                          type_="string",
                                          defvalue=None,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        simple_sequence_boolean = simpleseq_property(id_="simple_sequence_boolean",  
                                          type_="boolean",
                                          defvalue=None,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        simple_sequence_ulong = simpleseq_property(id_="simple_sequence_ulong",  
                                          type_="ulong",
                                          defvalue=None,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
#        simple_sequence_objref = simpleseq_property(id_="simple_sequence_objref",  
#                                          type_="objref",
#                                          defvalue=None,
#                                          mode="readwrite",
#                                          action="external",
#                                          kinds=("configure",)
#                                          ) 
        simple_sequence_short = simpleseq_property(id_="simple_sequence_short",  
                                          type_="short",
                                          defvalue=None,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        simple_sequence_float = simpleseq_property(id_="simple_sequence_float",  
                                          type_="float",
                                          defvalue=None,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        simple_sequence_octet = simpleseq_property(id_="simple_sequence_octet",  
                                          type_="octet",
                                          defvalue=None,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        simple_sequence_char = simpleseq_property(id_="simple_sequence_char",  
                                          type_="char",
                                          defvalue=None,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        simple_sequence_ushort = simpleseq_property(id_="simple_sequence_ushort",  
                                          type_="ushort",
                                          defvalue=None,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        simple_sequence_double = simpleseq_property(id_="simple_sequence_double",  
                                          type_="double",
                                          defvalue=None,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        simple_sequence_long = simpleseq_property(id_="simple_sequence_long",  
                                          type_="long",
                                          defvalue=None,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        simple_sequence_longlong = simpleseq_property(id_="simple_sequence_longlong",  
                                          type_="longlong",
                                          defvalue=None,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        simple_sequence_ulonglong = simpleseq_property(id_="simple_sequence_ulonglong",  
                                          type_="ulonglong",
                                          defvalue=None,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )
        class StructVars(object):
            struct_string = simple_property(id_="struct_string",
                                          type_="string",
                                          )
            struct_boolean = simple_property(id_="struct_boolean",
                                          type_="boolean",
                                          )
            struct_ulong = simple_property(id_="struct_ulong",
                                          type_="ulong",
                                          )
#            struct_objref = simple_property(id_="struct_objref",
#                                          type_="objref",
#                                          )
            struct_short = simple_property(id_="struct_short",
                                          type_="short",
                                          )
            struct_float = simple_property(id_="struct_float",
                                          type_="float",
                                          )
            struct_octet = simple_property(id_="struct_octet",
                                          type_="octet",
                                          )
            struct_char = simple_property(id_="struct_char",
                                          type_="char",
                                          )
            struct_ushort = simple_property(id_="struct_ushort",
                                          type_="ushort",
                                          )
            struct_double = simple_property(id_="struct_double",
                                          type_="double",
                                          )
            struct_long = simple_property(id_="struct_long",
                                          type_="long",
                                          )
            struct_longlong = simple_property(id_="struct_longlong",
                                          type_="longlong",
                                          )
            struct_ulonglong = simple_property(id_="struct_ulonglong",
                                          type_="ulonglong",
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
                d["struct_string"] = self.struct_string
                d["struct_boolean"] = self.struct_boolean
                d["struct_ulong"] = self.struct_ulong
#                d["struct_objref"] = self.struct_objref
                d["struct_short"] = self.struct_short
                d["struct_float"] = self.struct_float
                d["struct_octet"] = self.struct_octet
                d["struct_char"] = self.struct_char
                d["struct_ushort"] = self.struct_ushort
                d["struct_double"] = self.struct_double
                d["struct_long"] = self.struct_long
                d["struct_longlong"] = self.struct_longlong
                d["struct_ulonglong"] = self.struct_ulonglong
                return str(d)

            def getId(self):
                return "struct_vars"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("struct_string",self.struct_string),("struct_boolean",self.struct_boolean),("struct_ulong",self.struct_ulong),#("struct_objref",self.struct_objref),
("struct_short",self.struct_short),("struct_float",self.struct_float),("struct_octet",self.struct_octet),("struct_char",self.struct_char),("struct_ushort",self.struct_ushort),("struct_double",self.struct_double),("struct_long",self.struct_long),("struct_longlong",self.struct_longlong),("struct_ulonglong",self.struct_ulonglong)]

        
        struct_vars = struct_property(id_="struct_vars",
                                          structdef=StructVars,
                                          configurationkind=("configure",),
                                          mode="readwrite"
                                          )
        class StructSeqVars(object):
            struct_seq_string = simple_property(id_="struct_seq_string",
                                          type_="string",
                                          )
            struct_seq_boolean = simple_property(id_="struct_seq_boolean",
                                          type_="boolean",
                                          )
            struct_seq_ulong = simple_property(id_="struct_seq_ulong",
                                          type_="ulong",
                                          )
#            struct_seq_objref = simple_property(id_="struct_seq_objref",
#                                          type_="objref",
#                                          )
            struct_seq_short = simple_property(id_="struct_seq_short",
                                          type_="short",
                                          )
            struct_seq_float = simple_property(id_="struct_seq_float",
                                          type_="float",
                                          )
            struct_seq_octet = simple_property(id_="struct_seq_octet",
                                          type_="octet",
                                          )
            struct_seq_char = simple_property(id_="struct_seq_char",
                                          type_="char",
                                          )
            struct_seq_ushort = simple_property(id_="struct_seq_ushort",
                                          type_="ushort",
                                          )
            struct_seq_double = simple_property(id_="struct_seq_double",
                                          type_="double",
                                          )
            struct_seq_long = simple_property(id_="struct_seq_long",
                                          type_="long",
                                          )
            struct_seq_longlong = simple_property(id_="struct_seq_longlong",
                                          type_="longlong",
                                          )
            struct_seq_ulonglong = simple_property(id_="struct_seq_ulonglong",
                                          type_="ulonglong",
                                          )
        
            def __init__(self, struct_seq_string="", struct_seq_boolean=False, struct_seq_ulong=0, struct_seq_objref="", struct_seq_short=0, struct_seq_float=0, struct_seq_octet=0, struct_seq_char="", struct_seq_ushort=0, struct_seq_double=0, struct_seq_long=0, struct_seq_longlong=0, struct_seq_ulonglong=0):
                self.struct_seq_string = struct_seq_string
                self.struct_seq_boolean = struct_seq_boolean
                self.struct_seq_ulong = struct_seq_ulong
                self.struct_seq_objref = struct_seq_objref
                self.struct_seq_short = struct_seq_short
                self.struct_seq_float = struct_seq_float
                self.struct_seq_octet = struct_seq_octet
                self.struct_seq_char = struct_seq_char
                self.struct_seq_ushort = struct_seq_ushort
                self.struct_seq_double = struct_seq_double
                self.struct_seq_long = struct_seq_long
                self.struct_seq_longlong = struct_seq_longlong
                self.struct_seq_ulonglong = struct_seq_ulonglong

            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["struct_seq_string"] = self.struct_seq_string
                d["struct_seq_boolean"] = self.struct_seq_boolean
                d["struct_seq_ulong"] = self.struct_seq_ulong
                d["struct_seq_objref"] = self.struct_seq_objref
                d["struct_seq_short"] = self.struct_seq_short
                d["struct_seq_float"] = self.struct_seq_float
                d["struct_seq_octet"] = self.struct_seq_octet
                d["struct_seq_char"] = self.struct_seq_char
                d["struct_seq_ushort"] = self.struct_seq_ushort
                d["struct_seq_double"] = self.struct_seq_double
                d["struct_seq_long"] = self.struct_seq_long
                d["struct_seq_longlong"] = self.struct_seq_longlong
                d["struct_seq_ulonglong"] = self.struct_seq_ulonglong
                return str(d)

            def getId(self):
                return "struct_seq_vars"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("struct_seq_string",self.struct_seq_string),("struct_seq_boolean",self.struct_seq_boolean),("struct_seq_ulong",self.struct_seq_ulong),("struct_seq_objref",self.struct_seq_objref),("struct_seq_short",self.struct_seq_short),("struct_seq_float",self.struct_seq_float),("struct_seq_octet",self.struct_seq_octet),("struct_seq_char",self.struct_seq_char),("struct_seq_ushort",self.struct_seq_ushort),("struct_seq_double",self.struct_seq_double),("struct_seq_long",self.struct_seq_long),("struct_seq_longlong",self.struct_seq_longlong),("struct_seq_ulonglong",self.struct_seq_ulonglong)]

                
        struct_seq = structseq_property(id_="struct_seq",
                                          structdef=StructSeqVars,                          
                                          defvalue=[],
                                          configurationkind=("configure",),
                                          mode="readwrite"
                                          )
