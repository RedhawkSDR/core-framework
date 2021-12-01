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
# Source: simpleseqdev.spd.xml
# Generated on: Mon Nov 14 14:53:26 EST 2011
# Redhawk IDE
# Version:T.1.X.X
# Build id: v201110201144-r6330
from ossie.cf import CF, CF__POA
from ossie.utils import uuid

from ossie.device import Device 
from ossie.properties import simple_property
from ossie.properties import simpleseq_property

import queue, copy, time, threading

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

class simpleseqdev_python_impl1_base(CF__POA.Device, Device):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block
        
        def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
            Device.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
            self.process_thread = None
            self.auto_start = False
            
        def initialize(self):
            Device.initialize(self)
            # Instantiate the default implementations for all ports on this component


        def start(self):
            Device.start(self)
            if self.process_thread == None:
                self.process_thread = ProcessThread(target=self.process, pause=self.PAUSE)
                self.process_thread.start()

        def process(self):
            """The process method should process a single "chunk" of data and then return.  This method will be called
            from the processing thread again, and again, and again until it returns FINISH or stop() is called on the
            component.  If no work is performed, then return NOOP"""
            raise NotImplementedError

        def stop(self):
            # Technically not thread-safe but close enough for now
            process_thread = self.process_thread
            self.process_thread = None

            if process_thread != None:
                process_thread.stop()
                process_thread.join(self.TIMEOUT)
                if process_thread.isAlive():
                    raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")
            Device.stop(self)

        def releaseObject(self):
            try:
                self.stop()
            except Exception as e:
                self._log.exception("Error stopping: " + str(e))
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
        nil_property = simple_property(id_="nil_property",
                                          name="nil_property", 
                                          type_="float",
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure")
                                          ) 
        testsimpleseq = simpleseq_property(id_="testsimpleseq",
                                          name="testsimpleseq",   
                                          type_="float",
                                          defvalue=None,
                                          mode="readwrite",
                                          action="external",
                                          kinds=("configure")
                                          )