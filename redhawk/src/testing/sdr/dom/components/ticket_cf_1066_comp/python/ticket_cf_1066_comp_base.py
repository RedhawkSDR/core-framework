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
# Source: ticket_cf_1066_comp.spd.xml
# Generated on: Mon Dec 08 11:40:06 EST 2014
# REDHAWK IDE
# Version: 1.8.7
# Build id: R201405142058
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

class ticket_cf_1066_comp_base(CF__POA.Resource, Resource):
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
        simple_prop = simple_property(id_="simple_prop",
                                          type_="string",
                                          defvalue="default_value",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          ) 
        seq_prop = simpleseq_property(id_="seq_prop",  
                                          type_="short",
                                          defvalue=(111,222,333),
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure",)
                                          )
        class StructProp(object):
            prop_one = simple_property(id_="prop_one",
                                          type_="boolean",
                                          defvalue=True,
                                          )
            prop_two = simple_property(id_="prop_two",
                                          type_="string",
                                          defvalue="This is another default value",
                                          )
            prop_three = simple_property(id_="prop_three",
                                          type_="float",
                                          defvalue=555,
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
                d["prop_one"] = self.prop_one
                d["prop_two"] = self.prop_two
                d["prop_three"] = self.prop_three
                return str(d)

            def getId(self):
                return "struct_prop"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("prop_one",self.prop_one),("prop_two",self.prop_two),("prop_three",self.prop_three)]

        
        struct_prop = struct_property(id_="struct_prop",
                                          structdef=StructProp,
                                          configurationkind=("configure",),
                                          mode="readwrite"
                                          )
        class TmpStruct(object):
            prop_four = simple_property(id_="prop_four",
                                          type_="string",
                                          defvalue="yet another default",
                                          )
            prop_five = simple_property(id_="prop_five",
                                          type_="long",
                                          defvalue=246,
                                          )
            prop_six = simple_property(id_="prop_six",
                                          type_="double",
                                          defvalue=135,
                                          )
        
            def __init__(self, prop_four="", prop_five=0, prop_six=0):
                self.prop_four = prop_four
                self.prop_five = prop_five
                self.prop_six = prop_six

            def __str__(self):
                """Return a string representation of this structure"""
                d = {}
                d["prop_four"] = self.prop_four
                d["prop_five"] = self.prop_five
                d["prop_six"] = self.prop_six
                return str(d)

            def getId(self):
                return "tmp_struct"

            def isStruct(self):
                return True

            def getMembers(self):
                return [("prop_four",self.prop_four),("prop_five",self.prop_five),("prop_six",self.prop_six)]

                
        struct_seq_prop = structseq_property(id_="struct_seq_prop",
                                          structdef=TmpStruct,                          
                                          defvalue=[],
                                          configurationkind=("configure",),
                                          mode="readwrite"
                                          )
