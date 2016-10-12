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
# Source: PythonExecDev_python_impl1.spd.xml
# Generated on: Mon May 23 10:08:36 EDT 2011
from ossie.cf import CF, CF__POA
from ossie.utils import uuid
from ossie.device import ExecutableDevice 


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

    def run(self):
        state = NORMAL
        while (state != FINISH) and (not self.stop_signal.isSet()):
            state = self.target()
            if (state == NOOP):
                # If there was no data to process sleep to avoid spinning
                time.sleep(self.pause)

class PythonExecDev_python_impl1_base(CF__POA.ExecutableDevice, ExecutableDevice):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block
        
        def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
            ExecutableDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
            self.process_thread = None
            
        def initialize(self):
            ExecutableDevice.initialize(self)


        def start(self):
            ExecutableDevice.start(self)
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
            ExecutableDevice.stop(self)

        def releaseObject(self):
            try:
                self.stop()
            except Exception, e:
                self._log.exception("Error stopping: ", e)
            ExecutableDevice.releaseObject(self)

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
