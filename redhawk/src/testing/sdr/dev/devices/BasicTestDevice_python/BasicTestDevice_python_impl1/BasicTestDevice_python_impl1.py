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

from ossie.cf import CF, CF__POA
from ossie.device import ExecutableDevice, AggregateDevice,start_device
import commands, os, sys
import logging
import WorkModule
from BasicTestDevice_python_impl1Props import PROPERTIES
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

class BasicTestDevice_python_impl1(CF__POA.AggregateExecutableDevice,ExecutableDevice, AggregateDevice):
    def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
        ExecutableDevice.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams, PROPERTIES)
        AggregateDevice.__init__(self)

        self.process_thread = None
        self.__MAX_MEMORY   = 2048
        self.__MAX_BOGOMIPS = 1024
        self.allocated_mem  = 0
        self.allocated_bog  = 0
        self.__mem_name     = 'memCapacity'
        self.__bog_name     = 'BogoMipsCapacity'
        self.__mem_id       = 'DCE:7aeaace8-350e-48da-8d77-f97c2e722e06'
        self.__bog_id       = 'DCE:bbdf708f-ce05-469f-8aed-f5c93e353e14'
        
        

    ###########################################
    # CF::LifeCycle
    ###########################################
    def initialize(self):
        ExecutableDevice.initialize(self)


    def start(self):
        ExecutableDevice.start(self)
        self.process_thread = ProcessThread(target=self.process, pause=self.PAUSE)
        self.process_thread.start()

    def process(self):
        return NOOP

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
            if self._log != None :
                self._log.exception("Error stopping: ", e)
        ExecutableDevice.releaseObject(self)


    ###########################################
    # CF::PropertySet
    ###########################################
    ###########################################
    # CF::TestableObject
    ###########################################
    def runTest(self, properties, testid):
        pass
       
    ###########################################
    # CF::Device
    ###########################################
    
    # overrides allocateCapacity for memCapacity
    def allocate_memCapacity(self, value):
        try:
            tmp_val = self.allocated_mem + value
            if tmp_val > self.__MAX_MEMORY:
                raise Exception('Cannot allocate more than %d' % 
                                self.__MAX_MEMORY)
            
            self.allocated_mem = tmp_val
            self._props[self.__mem_id] = self.allocated_mem
            return True 
        except:
            print "\tAlloc Failed: Allocated Memory: %d" % self.allocated_mem
            print "\tAlloc Failed: Current Memory Usage: %d" % self._props[self.__mem_id]
            return False
        
    # overrides allocateCapacity for BogoMipsCapacity
    def allocate_BogoMipsCapacity(self, value):
        try:
            tmp_val = self.allocated_bog + value
            if tmp_val > self.__MAX_BOGOMIPS:
                raise Exception('Cannot allocate more than %d' % 
                                self.__MAX_BOGOMIPS)

            self.allocated_bog = tmp_val
            self._props[self.__bog_id] = self.allocated_bog
            return True
        except:
            print "\tAlloc Failed: Allocated BogoMips: %d" % self.allocated_bog
            print "\tAlloc Failed: Current BogoMips Usage: %d" % self._props[self.__bog_id]
        
            return False

    def allocateCapacities(self, propDict):
        try:
            successfulAllocations = []
            keys = propDict.keys()
            if self.__mem_id in keys:
                val = propDict[self.__mem_id]
                success = self.allocate_memCapacity(val)
                if success:
                    successfulAllocations.append((self.__mem_name, val))
    
            if self.__bog_id in keys:
                val = propDict[self.__bog_id]
                success = self.allocate_BogoMipsCapacity(propDict[self.__bog_id])
                if success:
                    successfulAllocations.append((self.__bog_name, val))
            
            success = len(successfulAllocations) == len(propDict)
            if not success:
                for key, val in successfulAllocations:
                    self._deallocateCapacity(key, val)
                return False  
            return True
        except:
            return False
            

    # overrides allocateCapacity for memCapacity
    def deallocate_memCapacity(self, value):
        try:
            tmp_val = self.allocated_mem - value
            if tmp_val < 0:
                raise Exception('Cannot deallocate more than %d' %
                                 self.__MAX_MEMORY)
            
            self.allocated_mem = tmp_val
            self._props[self.__mem_id] = self.allocated_mem
            return True 
            
        except Exception, e:
            print "\tGot an error while deallocationg Memory: %s" % str(e)
            return False
        
    # overrides allocateCapacity for BogoMipsCapacity
    def deallocate_BogoMipsCapacity(self, value):
        try:
            tmp_val = self.allocated_bog - value
            if tmp_val < 0:
                raise Exception('Cannot deallocate more than %d' %
                                self.__MAX_BOGOMIPS)
            
            self.allocated_bog = tmp_val
            self._props[self.__bog_id] = self.allocated_bog
            return True
        except Exception, e:
            print "\tGot an error while deallocationg BogoMips: %s" % str(e)
            return False
                        
    def updateUsageState(self):
        """
        This is called automatically after allocateCapacity or deallocateCapacity are called.
        Your implementation should determine the current state of the device:
           self._usageState = CF.Device.IDLE   # not in use
           self._usageState = CF.Device.ACTIVE # in use, with capacity remaining for allocation
           self._usageState = CF.Device.BUSY   # in use, with no capacity remaining for allocation
        """
        pass
            
###########################################                    
# program execution
###########################################
if __name__ == "__main__":
    logging.getLogger().setLevel(logging.WARN)
    logging.debug("Starting Device")
    start_device(BasicTestDevice_python_impl1)


