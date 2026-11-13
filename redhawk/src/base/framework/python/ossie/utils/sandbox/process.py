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

import os
import logging
import signal
import time
import threading
import tempfile
import subprocess
import platform
import zipfile

from ossie.utils import log4py
from ossie import parsers
from ossie.utils.popen import Popen

__all__ = ('LocalProcess')

log = logging.getLogger(__name__)


class LocalProcess(object):
    STOP_SIGNALS = ((signal.SIGINT, 1),
                    (signal.SIGTERM, 5),
                    (signal.SIGKILL, 0))

    def __init__(self, command, arguments, environment=None, stdout=None):
        self.__terminateRequested = False
        self.__command = command
        self.__arguments = arguments
        log.debug('%s %s', command, ' '.join(arguments))
        self.__process = Popen([command]+arguments, executable=command,
                               cwd=os.getcwd(), env=environment,
                               stdout=stdout, stderr=subprocess.STDOUT,
                               preexec_fn=os.setpgrp)
        self.__tracker = None
        self.__callback = None
        self.__children = []
    
    def setTerminationCallback(self, callback):
        if not self.__tracker:
            # Nothing is currently waiting for notification, start monitor.
            name = 'process-%d-tracker' % self.pid()
            self.__tracker = threading.Thread(name=name, target=self._monitorProcess)
            self.__tracker.daemon = True
            self.__tracker.start()
        self.__callback = callback

    def _monitorProcess(self):
        try:
            status = self.__process.wait()
        except:
            # If wait fails, don't bother with notification.
            return
        if self.__callback:
            self.__callback(self.pid(), status)

    def terminate(self):
        for child in self.__children:
            child.terminate()
        self.__children = []

        for sig, timeout in self.STOP_SIGNALS:
            try:
                log.debug('Killing process group %s with signal %s', self.__process.pid, sig)
                os.killpg(self.__process.pid, sig)
            except OSError:
                pass
            giveup_time = time.time() + timeout
            while self.__process.poll() is None:
                if time.time() > giveup_time:
                    break
                time.sleep(0.1)
            if self.__process.poll() is not None:
                break
        self.__process.wait()
        self.__process = None
    
    def requestTermination(self):
        self.__terminateRequested = True
    
    def command(self):
        return self.__command

    def pid(self):
        if self.__process:
            return self.__process.pid
        else:
            return None

    def isAlive(self):
        return self.__process and self.__process.poll() is None

    def addChild(self, process):
        self.__children.append(process)
