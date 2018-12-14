#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: writeonly_py.spd.xml
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

from ossie.device import Device
from ossie.threadedcomponent import *
from ossie.properties import simple_property
from ossie.properties import simpleseq_property
from ossie.properties import struct_property
from ossie.properties import structseq_property

import Queue, copy, time, threading

class writeonly_py_base(CF__POA.Device, Device, ThreadedComponent):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block

        def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
            Device.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
            ThreadedComponent.__init__(self)

            # self.auto_start is deprecated and is only kept for API compatibility
            # with 1.7.X and 1.8.0 devices.  This variable may be removed
            # in future releases
            self.auto_start = False
            # Instantiate the default implementations for all ports on this device

        def start(self):
            Device.start(self)
            ThreadedComponent.startThread(self, pause=self.PAUSE)

        def stop(self):
            Device.stop(self)
            if not ThreadedComponent.stopThread(self, self.TIMEOUT):
                raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")

        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._log.exception("Error stopping")
            Device.releaseObject(self)

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
        device_kind = simple_property(id_="DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d",
                                      name="device_kind",
                                      type_="string",
                                      mode="readonly",
                                      action="eq",
                                      kinds=("allocation",),
                                      description="""This specifies the device kind""")


        device_model = simple_property(id_="DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb",
                                       name="device_model",
                                       type_="string",
                                       mode="readonly",
                                       action="eq",
                                       kinds=("allocation",),
                                       description=""" This specifies the specific device""")


        foo = simple_property(id_="foo",
                              type_="string",
                              defvalue="something",
                              mode="writeonly",
                              action="external",
                              kinds=("allocation",))


        foo_seq = simpleseq_property(id_="foo_seq",
                                     type_="string",
                                     defvalue=["abc"                                             ],
                                     mode="writeonly",
                                     action="external",
                                     kinds=("allocation",))


        class FooStruct(object):
            abc = simple_property(
                                  id_="abc",
                                  
                                  type_="string",
                                  defvalue="def"
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
                d["abc"] = self.abc
                return str(d)
        
            @classmethod
            def getId(cls):
                return "foo_struct"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("abc",self.abc)]

        foo_struct = struct_property(id_="foo_struct",
                                     structdef=FooStruct,
                                     configurationkind=("allocation",),
                                     mode="writeonly")


        class Ghi(object):
            jkl = simple_property(
                                  id_="jkl",
                                  
                                  type_="string")
        
            def __init__(self, jkl=""):
                self.jkl = jkl
        
            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["jkl"] = self.jkl
                return str(d)
        
            @classmethod
            def getId(cls):
                return "ghi"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("jkl",self.jkl)]

        foo_struct_seq = structseq_property(id_="foo_struct_seq",
                                            structdef=Ghi,
                                            defvalue=[Ghi(jkl="mno")],
                                            configurationkind=("allocation",),
                                            mode="writeonly")




