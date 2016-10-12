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

import threading
import time

# Limit exported symbols to official API; ProcessThread is exported for
# backwards-compatibility
__all__ = ('NOOP', 'NORMAL', 'FINISH', 'ThreadedComponent', 'ProcessThread')

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
        while not self.stop_signal.isSet():
            try:
                state = self.target()
            except Exception, e:
                if hasattr(self.target.__self__,'_log'):
                    self.target.__self__._log.error("Exception detected in process function: "+str(e))
                raise
            if state == FINISH:
                return
            elif state == NOOP:
                # If there was no data to process sleep to avoid spinning
                delay = self.pause
            else:
                delay = 1e-6
            time.sleep(delay)

class ThreadedComponent(object):
    def __init__(self):
        self.threadControlLock = threading.RLock()
        self.process_thread = None
        self._defaultDelay = 0.0125

    def process(self):
        """
        The process method should process a single "chunk" of data and then
        return.  This method will be called from the processing thread again,
        and again, and again until it returns FINISH or stopThread() is called.
        If no work is performed, it should return NOOP; otherwise it should
        return NORMAL.
        """
        raise NotImplementedError

    def startThread(self, pause=None):
        """
        Starts the processing thread, if necessary.
        """
        self.threadControlLock.acquire()
        try:
            if pause is None:
                pause = self._defaultDelay
            if self.process_thread == None:
                self.process_thread = ProcessThread(target=self.process, pause=pause)
                self.process_thread.start()
        finally:
            self.threadControlLock.release()

    def stopThread(self, timeout=5.0):
        """
        Stops the processing thread, if necessary.
        """
        self.threadControlLock.acquire()
        try:
            process_thread = self.process_thread
            self.process_thread = None
        finally:
            self.threadControlLock.release()

        if process_thread:
            process_thread.stop()
            process_thread.join(timeout)
            return not process_thread.isAlive()
        else:
            return True

    def getThreadDelay(self):
        """
        Returns the delay between calls to service function after a NOOP.
        """
        # Generated Python components use a 'PAUSE' attribute, rather than an
        # API call, to set the delay. To maintain compatibility, components
        # generated with 1.10 pass 'PAUSE' to startThread(), so this adapts the
        # ThreadedComponent API to work with the old pattern.
        if hasattr(self, 'PAUSE'):
            return self.PAUSE
        return self._defaultDelay

    def setThreadDelay(self, delay):
        """
        Changes the delay between calls to service function after a NOOP.
        """
        self.threadControlLock.acquire()
        try:
            self._defaultDelay = delay
            if self.process_thread:
                self.process_thread.updatePause(delay)
        finally:
            self.threadControlLock.release()

        # Generated Python components use a 'PAUSE' attribute, rather than an
        # API call, to set the delay. To maintain compatibility, components
        # generated with 1.10 pass 'PAUSE' to startThread(), so this adapts the
        # ThreadedComponent API to work with the old pattern.
        if hasattr(self, 'PAUSE'):
            self.PAUSE = delay

    def isRunning(self):
        return self.process_thread != None
