#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK burstioInterfaces.
#
# REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import time
import threading
import traceback

def compare_tasks(lhs, rhs):
    return cmp(lhs[0], rhs[0])

class ExecutorService(object):
    def __init__(self):
        self._cond = threading.Condition(threading.Lock())
        self._queue = []
        self._thread = None
        self._running = False

    def schedule(self, when, func):
        self._cond.acquire()
        try:
            # Add the new task to the end of the queue, then sort the queue
            # in ascending order
            self._queue.append((when, func))
            self._queue.sort(compare_tasks)
            self._cond.notifyAll()
        finally:
            self._cond.release()

    def execute(self, func):
        self.schedule(time.time(), func)

    def start(self):
        self._cond.acquire()
        try:
            if self._running:
                return
            self._running = True
            self._thread = threading.Thread(target=self._run)
            self._thread.start()
        finally:
            self._cond.release()
    
    def stop(self):
        self._cond.acquire()
        try:
            if not self._running:
                return
            self._running = False
            old_thread = self._thread
            self._thread = None
            self._cond.notifyAll()
        finally:
            self._cond.release()
        if old_thread:
            old_thread.join()

    def clear(self):
        self._cond.acquire()
        try:
            self._queue = []
            self._cond.notifyAll()
        finally:
            self._cond.release()

    def _run(self):
        self._cond.acquire()
        try:
            while self._running:
                while self._queue:
                    # Start at the front of the queue every time--a task may
                    # have been added while the lock was released to service
                    # the last task
                    when, func = self._queue[0]
                    if when > time.time():
                        # Head of queue is scheduled in future
                        break

                    # Remove task from the queue
                    self._queue.pop(0)

                    # Run the task with the lock released
                    self._cond.release()
                    try:
                        func()
                    except:
                        traceback.print_exc()
                    self._cond.acquire()

                if not self._queue:
                    self._cond.wait()
                else:
                    when, func = self._queue[0]
                    timeout = time.time() - when
                    self._cond.wait(timeout)
        finally:
            self._cond.release()
