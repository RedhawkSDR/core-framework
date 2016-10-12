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
# Source: TestPythonPropsRange.spd.xml
# Generated on: Wed Dec 19 10:22:38 EST 2012
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

class TestPythonPropsRange_base(CF__POA.Resource, Resource):
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
        my_octet_name = simple_property(id_="my_octet",
                                          name="my_octet_name", 
                                          type_="octet",
                                          defvalue=1,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        my_short_name = simple_property(id_="my_short",
                                          name="my_short_name", 
                                          type_="short",
                                          defvalue=2,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        my_ushort_name = simple_property(id_="my_ushort",
                                          name="my_ushort_name", 
                                          type_="ushort",
                                          defvalue=3,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        my_long_name = simple_property(id_="my_long",
                                          name="my_long_name", 
                                          type_="long",
                                          defvalue=4,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        my_ulong_name = simple_property(id_="my_ulong",
                                          name="my_ulong_name", 
                                          type_="ulong",
                                          defvalue=5,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        my_longlong_name = simple_property(id_="my_longlong",
                                          name="my_longlong_name", 
                                          type_="longlong",
                                          defvalue=6,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )       
        my_ulonglong_name = simple_property(id_="my_ulonglong",
                                          name="my_ulonglong_name", 
                                          type_="ulonglong",
                                          defvalue=7,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        seq_octet_name = simpleseq_property(id_="seq_octet",
                                          name="seq_octet_name",   
                                          type_="octet",
                                          defvalue="\x01\x02",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        seq_short_name = simpleseq_property(id_="seq_short",
                                          name="seq_short_name",   
                                          type_="short",
                                          defvalue=(1,2),
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        seq_ushort_name = simpleseq_property(id_="seq_ushort",
                                          name="seq_ushort_name",   
                                          type_="ushort",
                                          defvalue=(1,2),
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        seq_long_name = simpleseq_property(id_="seq_long",
                                          name="seq_long_name",   
                                          type_="long",
                                          defvalue=(1,2),
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        seq_ulong_name = simpleseq_property(id_="seq_ulong",
                                          name="seq_ulong_name",   
                                          type_="ulong",
                                          defvalue=(1,2),
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        seq_longlong_name = simpleseq_property(id_="seq_longlong",
                                          name="seq_longlong_name",   
                                          type_="longlong",
                                          defvalue=(1,2),
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        seq_ulonglong_name = simpleseq_property(id_="seq_ulonglong",
                                          name="seq_ulonglong_name",   
                                          type_="ulonglong",
                                          defvalue=(1,2),
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        seq_char_name = simpleseq_property(id_="seq_char",
                                          name="seq_char_name",   
                                          type_="char",
                                          defvalue="ab",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )
        class MyStructName(object):
            struct_octet_name = simple_property(id_="struct_octet",
                                          name="struct_octet_name", 
                                          type_="octet",
                                          defvalue=1,
                                          )
            struct_short_name = simple_property(id_="struct_short",
                                          name="struct_short_name", 
                                          type_="short",
                                          defvalue=2,
                                          )
            struct_ushort_name = simple_property(id_="struct_ushort",
                                          name="struct_ushort_name", 
                                          type_="ushort",
                                          defvalue=3,
                                          )
            struct_long_name = simple_property(id_="struct_long",
                                          name="struct_long_name", 
                                          type_="long",
                                          defvalue=4,
                                          )
            struct_ulong_name = simple_property(id_="struct_ulong",
                                          name="struct_ulong_name", 
                                          type_="ulong",
                                          defvalue=5,
                                          )
            struct_longlong_name = simple_property(id_="struct_longlong",
                                          name="struct_longlong_name", 
                                          type_="longlong",
                                          defvalue=6,
                                          )
            struct_ulonglong_name = simple_property(id_="struct_ulonglong",
                                          name="struct_ulonglong_name", 
                                          type_="ulonglong",
                                          defvalue=7,
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
                d["struct_octet_name"] = self.struct_octet_name
                d["struct_short_name"] = self.struct_short_name
                d["struct_ushort_name"] = self.struct_ushort_name
                d["struct_long_name"] = self.struct_long_name
                d["struct_ulong_name"] = self.struct_ulong_name
                d["struct_longlong_name"] = self.struct_longlong_name
                d["struct_ulonglong_name"] = self.struct_ulonglong_name
                return str(d)

            def getId(self):
                return "my_struct"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("struct_octet_name",self.struct_octet_name),("struct_short_name",self.struct_short_name),("struct_ushort_name",self.struct_ushort_name),("struct_long_name",self.struct_long_name),("struct_ulong_name",self.struct_ulong_name),("struct_longlong_name",self.struct_longlong_name),("struct_ulonglong_name",self.struct_ulonglong_name)]

        
        my_struct_name = struct_property(id_="my_struct",
                                          name="my_struct_name", 
                                          structdef=MyStructName,
                                          configurationkind=("configure",),
                                          mode="readwrite"
                                          )
        class SsStructName(object):
            ss_octet_name = simple_property(id_="ss_octet",
                                          name="ss_octet_name", 
                                          type_="octet",
                                          )
            ss_short_name = simple_property(id_="ss_short",
                                          name="ss_short_name", 
                                          type_="short",
                                          )
            ss_ushort_name = simple_property(id_="ss_ushort",
                                          name="ss_ushort_name", 
                                          type_="ushort",
                                          )
            ss_long_name = simple_property(id_="ss_long",
                                          name="ss_long_name", 
                                          type_="long",
                                          )
            ss_ulong_name = simple_property(id_="ss_ulong",
                                          name="ss_ulong_name", 
                                          type_="ulong",
                                          )
            ss_longlong_name = simple_property(id_="ss_longlong",
                                          name="ss_longlong_name", 
                                          type_="longlong",
                                          )
            ss_ulonglong_name = simple_property(id_="ss_ulonglong",
                                          name="ss_ulonglong_name", 
                                          type_="ulonglong",
                                          )
        
            def __init__(self, ss_octet_name=0, ss_short_name=0, ss_ushort_name=0, ss_long_name=0, ss_ulong_name=0, ss_longlong_name=0, ss_ulonglong_name=0):
                self.ss_octet_name = ss_octet_name
                self.ss_short_name = ss_short_name
                self.ss_ushort_name = ss_ushort_name
                self.ss_long_name = ss_long_name
                self.ss_ulong_name = ss_ulong_name
                self.ss_longlong_name = ss_longlong_name
                self.ss_ulonglong_name = ss_ulonglong_name

            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["ss_octet_name"] = self.ss_octet_name
                d["ss_short_name"] = self.ss_short_name
                d["ss_ushort_name"] = self.ss_ushort_name
                d["ss_long_name"] = self.ss_long_name
                d["ss_ulong_name"] = self.ss_ulong_name
                d["ss_longlong_name"] = self.ss_longlong_name
                d["ss_ulonglong_name"] = self.ss_ulonglong_name
                return str(d)

            def getId(self):
                return "ss_struct"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("ss_octet_name",self.ss_octet_name),("ss_short_name",self.ss_short_name),("ss_ushort_name",self.ss_ushort_name),("ss_long_name",self.ss_long_name),("ss_ulong_name",self.ss_ulong_name),("ss_longlong_name",self.ss_longlong_name),("ss_ulonglong_name",self.ss_ulonglong_name)]

                
        my_structseq_name = structseq_property(id_="my_structseq",
                                          name="my_structseq_name", 
                                          structdef=SsStructName,                          
                                          defvalue=[SsStructName(0,1,2,3,4,5,6),SsStructName(7,8,9,10,11,12,13)],
                                          configurationkind=("configure",),
                                          mode="readwrite"
                                          )
