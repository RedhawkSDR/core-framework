#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: TestComplexProps.spd.xml
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

class TestComplexProps_base(CF__POA.Resource, Component, ThreadedComponent):
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
        complexBooleanProp = simple_property(id_="complexBooleanProp",
                                             type_="boolean",
                                             defvalue=complex(0,1),
                                             complex=True,
                                             mode="readwrite",
                                             action="external",
                                             kinds=("property",))


        complexULongProp = simple_property(id_="complexULongProp",
                                           type_="ulong",
                                           defvalue=complex(4,5),
                                           complex=True,
                                           mode="readwrite",
                                           action="external",
                                           kinds=("property",))


        complexShortProp = simple_property(id_="complexShortProp",
                                           type_="short",
                                           defvalue=complex(4,5),
                                           complex=True,
                                           mode="readwrite",
                                           action="external",
                                           kinds=("property",))


        complexFloatProp = simple_property(id_="complexFloatProp",
                                           type_="float",
                                           defvalue=complex(4.0,5.0),
                                           complex=True,
                                           mode="readwrite",
                                           action="external",
                                           kinds=("property",))


        complexOctetProp = simple_property(id_="complexOctetProp",
                                           type_="octet",
                                           defvalue=complex(4,5),
                                           complex=True,
                                           mode="readwrite",
                                           action="external",
                                           kinds=("property",))


        complexUShort = simple_property(id_="complexUShort",
                                        type_="ushort",
                                        defvalue=complex(4,5),
                                        complex=True,
                                        mode="readwrite",
                                        action="external",
                                        kinds=("property",))


        complexDouble = simple_property(id_="complexDouble",
                                        type_="double",
                                        defvalue=complex(4.0,5.0),
                                        complex=True,
                                        mode="readwrite",
                                        action="external",
                                        kinds=("property",))


        complexLong = simple_property(id_="complexLong",
                                      type_="long",
                                      defvalue=complex(4,5),
                                      complex=True,
                                      mode="readwrite",
                                      action="external",
                                      kinds=("property",))


        complexLongLong = simple_property(id_="complexLongLong",
                                          type_="longlong",
                                          defvalue=complex(4,5),
                                          complex=True,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("property",))


        complexULongLong = simple_property(id_="complexULongLong",
                                           type_="ulonglong",
                                           defvalue=complex(4,5),
                                           complex=True,
                                           mode="readwrite",
                                           action="external",
                                           kinds=("property",))


        complexFloatSequence = simpleseq_property(id_="complexFloatSequence",
                                                  type_="float",
                                                  defvalue=[complex(6.0,7.0), complex(4.0,5.0), complex(4.0,5.0)                                             ],
                                                  complex=True,
                                                  mode="readwrite",
                                                  action="external",
                                                  kinds=("property",))


        class _Float(object):
            FloatStructMember = simple_property(
                                                id_="FloatStructMember",
                                                
                                                type_="float",
                                                defvalue=6.0
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
                d["FloatStructMember"] = self.FloatStructMember
                return str(d)
        
            @classmethod
            def getId(cls):
                return "FloatStruct"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("FloatStructMember",self.FloatStructMember)]

        FloatStruct = struct_property(id_="FloatStruct",
                                      structdef=_Float,
                                      configurationkind=("property",),
                                      mode="readwrite")


        class ComplexFloat(object):
            complexFloatStructMember = simple_property(
                                                       id_="complexFloatStructMember",
                                                       
                                                       type_="float",
                                                       defvalue=complex(6.0,7.0)
                                                       ,
                                                       complex=True)
        
            complex_float_seq = simpleseq_property(
                                                   id_="complexFloatStruct::complex_float_seq",
                                                   
                                                   name="complex_float_seq",
                                                   type_="float",
                                                   defvalue=[complex(3.0,2.0)]
                                                   ,
                                                   complex=True)
        
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
                d["complexFloatStructMember"] = self.complexFloatStructMember
                d["complex_float_seq"] = self.complex_float_seq
                return str(d)
        
            @classmethod
            def getId(cls):
                return "complexFloatStruct"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("complexFloatStructMember",self.complexFloatStructMember),("complex_float_seq",self.complex_float_seq)]

        complexFloatStruct = struct_property(id_="complexFloatStruct",
                                             structdef=ComplexFloat,
                                             configurationkind=("property",),
                                             mode="readwrite")


        class _FloatStructSequenceMember(object):
            FloatStructSequenceMemberMemember = simple_property(
                                                                id_="FloatStructSequenceMemberMemember",
                                                                
                                                                type_="float",
                                                                defvalue=6.0
                                                                )
        
            float_seq = simpleseq_property(
                                           id_="FloatStructSequence::float_seq",
                                           
                                           name="float_seq",
                                           type_="float",
                                           defvalue=[3.0]
                                           )
        
            def __init__(self, FloatStructSequenceMemberMemember=6.0, float_seq=[3.0]):
                self.FloatStructSequenceMemberMemember = FloatStructSequenceMemberMemember
                self.float_seq = float_seq
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["FloatStructSequenceMemberMemember"] = self.FloatStructSequenceMemberMemember
                d["float_seq"] = self.float_seq
                return str(d)
        
            @classmethod
            def getId(cls):
                return "FloatStructSequenceMember"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("FloatStructSequenceMemberMemember",self.FloatStructSequenceMemberMemember),("float_seq",self.float_seq)]

        FloatStructSequence = structseq_property(id_="FloatStructSequence",
                                                 structdef=_FloatStructSequenceMember,
                                                 defvalue=[],
                                                 configurationkind=("property",),
                                                 mode="readwrite")


        class ComplexFloatStructSequenceMember(object):
            complexFloatStructSequenceMemberMemember = simple_property(
                                                                       id_="complexFloatStructSequenceMemberMemember",
                                                                       
                                                                       type_="float",
                                                                       defvalue=complex(6.0,5.0)
                                                                       ,
                                                                       complex=True)
        
            complex_float_seq = simpleseq_property(
                                                   id_="complexFloatStructSequence::complex_float_seq",
                                                   
                                                   name="complex_float_seq",
                                                   type_="float",
                                                   defvalue=[complex(3.0,2.0)]
                                                   ,
                                                   complex=True)
        
            def __init__(self, complexFloatStructSequenceMemberMemember=complex(6.0,5.0), complex_float_seq=[complex(3.0,2.0)]):
                self.complexFloatStructSequenceMemberMemember = complexFloatStructSequenceMemberMemember
                self.complex_float_seq = complex_float_seq
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["complexFloatStructSequenceMemberMemember"] = self.complexFloatStructSequenceMemberMemember
                d["complex_float_seq"] = self.complex_float_seq
                return str(d)
        
            @classmethod
            def getId(cls):
                return "complexFloatStructSequenceMember"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("complexFloatStructSequenceMemberMemember",self.complexFloatStructSequenceMemberMemember),("complex_float_seq",self.complex_float_seq)]

        complexFloatStructSequence = structseq_property(id_="complexFloatStructSequence",
                                                        structdef=ComplexFloatStructSequenceMember,
                                                        defvalue=[],
                                                        configurationkind=("property",),
                                                        mode="readwrite")




