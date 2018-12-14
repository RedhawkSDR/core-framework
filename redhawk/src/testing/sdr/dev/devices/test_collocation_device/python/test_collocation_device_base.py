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
# Source: test_collocation_device.spd.xml
# Generated on: Thu Nov 01 08:29:29 EDT 2012
# Redhawk IDE
# Version:@buildLabel@
# Build id: @buildId@
from ossie.cf import CF, CF__POA
from ossie.utils import uuid

from ossie.device import ExecutableDevice 
from ossie.properties import simple_property
from ossie.properties import simpleseq_property
from ossie.properties import struct_property

from ossie.events import PropertyEventSupplier

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

class test_collocation_device_base(CF__POA.ExecutableDevice, ExecutableDevice):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block
        
        def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
            ExecutableDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
            self.threadControlLock = threading.RLock()
            self.process_thread = None
            # self.auto_start is deprecated and is only kept for API compatability
            # with 1.7.X and 1.8.0 components.  This variable may be removed
            # in future releases
            self.auto_start = False
            
        def initialize(self):
            ExecutableDevice.initialize(self)
            
            # Instantiate the default implementations for all ports on this component
            self.port_propEvent = PropertyEventSupplier(self)

        def start(self):
            self.threadControlLock.acquire()
            try:
                ExecutableDevice.start(self)
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
                ExecutableDevice.stop(self)
            finally:
                self.threadControlLock.release()

        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._log.exception("Error stopping")
            self.threadControlLock.acquire()
            try:
                ExecutableDevice.releaseObject(self)
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
        class RedhawkReservationRequest(object):
            redhawk__reservation_request__obj_id = simple_property(
                                                                           id_="redhawk::reservation_request::obj_id",
                                                                           
                                                                           type_="string")
        
            redhawk__reservation_request__kinds = simpleseq_property(
                                                                     id_="redhawk::reservation_request::kinds",
                                                                     
                                                                     type_="string",
                                                                     defvalue=[]
                                                                     )
        
            redhawk__reservation_request__values = simpleseq_property(
                                                                      id_="redhawk::reservation_request::values",
                                                                      
                                                                      type_="string",
                                                                      defvalue=[]
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
                d["redhawk__reservation_request__obj_id"] = self.redhawk__reservation_request__obj_id
                d["redhawk__reservation_request__kinds"] = self.redhawk__reservation_request__kinds
                d["redhawk__reservation_request__values"] = self.redhawk__reservation_request__values
                return str(d)
        
            @classmethod
            def getId(cls):
                return "redhawk::reservation_request"
        
            @classmethod
            def isStruct(cls):
                return True
        
            def getMembers(self):
                return [("redhawk__reservation_request__obj_id",self.redhawk__reservation_request__obj_id),("redhawk__reservation_request__kinds",self.redhawk__reservation_request__kinds),("redhawk__reservation_request__values",self.redhawk__reservation_request__values)]

        redhawk__reservation_request = struct_property(id_="redhawk::reservation_request",
                                                       structdef=RedhawkReservationRequest,
                                                       configurationkind=("allocation",),
                                                       mode="readwrite")
        device_kind = simple_property(id_="DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d",
                                          name="device_kind", 
                                          type_="string",
                                          mode="readonly",
                                          action="eq",
                                          kinds=("configure","allocation"),
                                          description="""This specifies the device kind""" 
                                          )       
        device_model = simple_property(id_="DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb",
                                          name="device_model", 
                                          type_="string",
                                          mode="readonly",
                                          action="eq",
                                          kinds=("configure","allocation"),
                                          description="""This specifies the specific device""" 
                                          )       
        supported_components = simple_property(id_="supported_components",
                                          name="supported_components", 
                                          type_="short",
                                          defvalue=4,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure","allocation")
                                          )       
        additional_supported_components = simple_property(id_="additional_supported_components",
                                          name="additional_supported_components", 
                                          type_="short",
                                          defvalue=4,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure","allocation")
                                          )       
        os_name = simple_property(id_="DCE:4a23ad60-0b25-4121-a630-68803a498f75",
                                          name="os_name", 
                                          type_="string",
                                          defvalue="Linux",
                                          mode="readwrite",
                                          action="eq",
                                          kinds=("configure","allocation")
                                          )
        allocation_attempts = simple_property(id_="allocation_attempts",
                                          name="allocation_attempts", 
                                          type_="ulong",
                                          defvalue=0,
                                          mode="readonly",
                                          action="external",
                                          kinds=("configure",)
                                          )
