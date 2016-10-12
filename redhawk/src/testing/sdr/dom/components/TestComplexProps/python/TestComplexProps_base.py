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
# Source: TestComplexProps.spd.xml
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

class TestComplexProps_base(CF__POA.Resource, Resource):
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
        complexBooleanProp = simple_property(id_="complexBooleanProp",
                                             type_="boolean",
                                             defvalue=complex(0,1),
                                             complex=True,
                                             mode="readwrite",
                                             action="external",
                                             kinds=("configure",)
                                             )
        complexULongProp = simple_property(id_="complexULongProp",
                                           type_="ulong",
                                           defvalue=complex(4,5),
                                           complex=True,
                                           mode="readwrite",
                                           action="external",
                                           kinds=("configure",)
                                           )
        complexShortProp = simple_property(id_="complexShortProp",
                                           type_="short",
                                           defvalue=complex(4,5),
                                           complex=True,
                                           mode="readwrite",
                                           action="external",
                                           kinds=("configure",)
                                           )
        complexFloatProp = simple_property(id_="complexFloatProp",
                                           type_="float",
                                           defvalue=complex(4.0,5.0),
                                           complex=True,
                                           mode="readwrite",
                                           action="external",
                                           kinds=("configure",)
                                           )
        complexOctetProp = simple_property(id_="complexOctetProp",
                                           type_="octet",
                                           defvalue=complex(4,5),
                                           complex=True,
                                           mode="readwrite",
                                           action="external",
                                           kinds=("configure",)
                                           )
        complexCharProp = simple_property(id_="complexCharProp",
                                          type_="char",
                                          defvalue=complex(4,5),
                                          complex=True,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )
        complexUShort = simple_property(id_="complexUShort",
                                        type_="ushort",
                                        defvalue=complex(4,5),
                                        complex=True,
                                        mode="readwrite",
                                        action="external",
                                        kinds=("configure",)
                                        )
        complexDouble = simple_property(id_="complexDouble",
                                        type_="double",
                                        defvalue=complex(4.0,5.0),
                                        complex=True,
                                        mode="readwrite",
                                        action="external",
                                        kinds=("configure",)
                                        )
        complexLong = simple_property(id_="complexLong",
                                      type_="long",
                                      defvalue=complex(4,5),
                                      complex=True,
                                      mode="readwrite",
                                      action="external",
                                      kinds=("configure",)
                                      )
        complexLongLong = simple_property(id_="complexLongLong",
                                          type_="longlong",
                                          defvalue=complex(4,5),
                                          complex=True,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )
        complexULongLong = simple_property(id_="complexULongLong",
                                           type_="ulonglong",
                                           defvalue=complex(4,5),
                                           complex=True,
                                           mode="readwrite",
                                           action="external",
                                           kinds=("configure",)
                                           )
        complexFloatSequence = simpleseq_property(id_="complexFloatSequence",
                                                  type_="float",
                                                  defvalue=[
                                                      complex(4.0,5.0),
                                                      complex(4.0,5.0),
                                                      complex(4.0,5.0),
                                                      ],
                                                  complex=True,
                                                  mode="readwrite",
                                                  action="external",
                                                  kinds=("configure",)
                                                  )
        class Float(object):
            FloatStructMember = simple_property(id_="FloatStructMember",
                                                type_="float",
                                                defvalue=4.0,
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
                d["FloatStructMember"] = self.FloatStructMember
                return str(d)
        
            def getId(self):
                return "FloatStruct"
        
            def isStruct(self):
                return True
        
            def getMembers(self):
                return [("FloatStructMember",self.FloatStructMember)]

        FloatStruct = struct_property(id_="FloatStruct",
                                      structdef=Float,
                                      configurationkind=("configure",),
                                      mode="readwrite"
                                      )
        class ComplexFloat(object):
            complexFloatStructMember = simple_property(id_="complexFloatStructMember",
                                                       type_="float",
                                                       defvalue=complex(4.0,5.0),
                                                       complex=True,
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
                d["complexFloatStructMember"] = self.complexFloatStructMember
                return str(d)
        
            def getId(self):
                return "complexFloatStruct"
        
            def isStruct(self):
                return True
        
            def getMembers(self):
                return [("complexFloatStructMember",self.complexFloatStructMember)]

        complexFloatStruct = struct_property(id_="complexFloatStruct",
                                             structdef=ComplexFloat,
                                             configurationkind=("configure",),
                                             mode="readwrite"
                                             )

        class FloatStructSequenceMember(object):
            FloatStructSequenceMemberMemember = simple_property(id_="FloatStructSequenceMemberMemember",
                                                                type_="float",
                                                                defvalue=4.0,
                                                                )
        
            def __init__(self, FloatStructSequenceMemberMemember=4.0):
                self.FloatStructSequenceMemberMemember = FloatStructSequenceMemberMemember
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["FloatStructSequenceMemberMemember"] = self.FloatStructSequenceMemberMemember
                return str(d)
        
            def getId(self):
                return "FloatStructSequenceMember"
        
            def isStruct(self):
                return True
        
            def getMembers(self):
                return [("FloatStructSequenceMemberMemember",self.FloatStructSequenceMemberMemember)]

        FloatStructSequence = structseq_property(id_="FloatStructSequence",
                                                 structdef=FloatStructSequenceMember,
                                                 defvalue=[],
                                                 configurationkind=("configure",),
                                                 mode="readwrite"
                                                 )
        class ComplexFloatStructSequenceMember(object):
            complexFloatStructSequenceMemberMemember = simple_property(id_="complexFloatStructSequenceMemberMemember",
                                                                       type_="float",
                                                                       defvalue=complex(4.0,5.0),
                                                                       complex=True,
                                                                       )
        
            def __init__(self, complexFloatStructSequenceMemberMemember=complex(4.0,5.0)):
                self.complexFloatStructSequenceMemberMemember = complexFloatStructSequenceMemberMemember
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["complexFloatStructSequenceMemberMemember"] = self.complexFloatStructSequenceMemberMemember
                return str(d)
        
            def getId(self):
                return "complexFloatStructSequenceMember"
        
            def isStruct(self):
                return True
        
            def getMembers(self):
                return [("complexFloatStructSequenceMemberMemember",self.complexFloatStructSequenceMemberMemember)]

        complexFloatStructSequence = structseq_property(id_="complexFloatStructSequence",
                                                        structdef=ComplexFloatStructSequenceMember,
                                                        defvalue=[],
                                                        configurationkind=("configure",),
                                                        mode="readwrite"
                                                        )

