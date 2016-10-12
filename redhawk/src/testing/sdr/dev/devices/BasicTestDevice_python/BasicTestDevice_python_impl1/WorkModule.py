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

import Queue
import threading
import os, time

class WorkClass:
    """
    This class provides a place for the main processing of the
       component to reside.
    """

    def __init__(self, parent):
        self.parent = parent
        
        # Initialize variables for input data processing
        self.data_queue = Queue.Queue()
        self.empty_queue = False
        
        # variables for thread management
        self.is_running = True
        self.timeout_check_period = 0.1 # this is in seconds
        self.process_thread_released = False
        
        # create mutex locks for handling issues with Reset
        self.reset_lock = threading.Lock()
        self.reset_signal = threading.Event()
        
        # create and start the main thread
        self.process_thread = threading.Thread(target=self.Process)
        self.process_thread.start()
        
    def __del__(self):
        """
        Destructor
        """
        self.Release()
        
    def Release(self):
        self.reset_signal.clear()
        self.reset_lock.acquire()

        self.is_running = False
        
        self.reset_signal.set()
        self.reset_lock.release()
        
        while not self.process_thread_released:
            time.sleep(0.01)
            
            
    def Reset(self, external_flag=True):
        """
        Reset the module to a known state.
        """
        
        if external_flag:
            self.reset_signal.clear()
            self.reset_lock.acquire()
            
            # Empty out the data queue if desired
            if self.empty_queue:
                try:
                    while self.data_queue.get_nowait():
                        pass
                except Queue.Empty:
                    pass
            
        #
        # Clear out any data variables
        #
        
        
        if external_flag:    
            self.reset_signal.set()
            self.reset_lock.release()

    def queueData(self, data, T, EOS, streamID):
        """
        Add data to main processing queue.
        """
        tmpmsg = (data, T, EOS, streamID)
        self.data_queue.put(tmpmsg)
        
    # Main processing thread
    def Process(self):
        new_data_flag = False
        new_data = None

        # set signal initially to enter main loop
        self.reset_signal.set()
        
        # main processing loop
        while self.is_running:
            # make's sure it is OK to process data from the queue
            self.reset_signal.wait()
            self.reset_lock.acquire()
            
            # check to see if component has been released
            if not self.is_running:
                self.reset_lock.release()
                continue

            # get new message from queue if available
            try:
                new_data = self.data_queue.get(timeout=self.timeout_check_period)
                new_data_flag = True
            except Queue.Empty:
                new_data_flag = False

            if new_data_flag:
                # extract info from new data
                data, T, EOS, streamID = new_data

                #
                # deal with new data here
                #
                
            # ensures that nothing is reset while processing
            self.reset_lock.release()
            
        # Let Release function know it's now OK to close
        self.process_thread_released = True
