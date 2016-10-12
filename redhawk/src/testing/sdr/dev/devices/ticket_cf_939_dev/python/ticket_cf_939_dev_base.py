#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: ticket_cf_939_dev.spd.xml
# Generated on: Tue Aug 26 14:21:54 EDT 2014
# REDHAWK IDE
# Version: 1.8.7
# Build id: R201405142058
from ossie.cf import CF, CF__POA
from ossie.utils import uuid

from ossie.device import ExecutableDevice 
from ossie.properties import simple_property

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

class ticket_cf_939_dev_base(CF__POA.ExecutableDevice, ExecutableDevice):
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
        os_version = simple_property(id_="DCE:0f3a9a37-a342-43d8-9b7f-78dc6da74192",
                                          name="os_version", 
                                          type_="string",
                                          defvalue="1.2.3.el6.x86_64",
                                          mode="readwrite",
                                          action="eq",
                                          kinds=("allocation",)
                                          )       
        os_name = simple_property(id_="DCE:80BF17F0-6C7F-11d4-A226-0050DA314CD6",
                                          name="os_name", 
                                          type_="string",
                                          defvalue="Linux",
                                          mode="readwrite",
                                          action="eq",
                                          kinds=("allocation",)
                                          )       
        processor_name = simple_property(id_="DCE:9B445600-6C7F-11d4-A226-0050DA314CD6",
                                          name="processor_name", 
                                          type_="string",
                                          defvalue="x86_64",
                                          mode="readwrite",
                                          action="eq",
                                          kinds=("allocation",)
                                          )
