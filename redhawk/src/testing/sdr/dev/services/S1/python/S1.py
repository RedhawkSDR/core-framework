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
# AUTO-GENERATED
#
# Source: S1.spd.xml

import sys, signal, copy, os
import logging

from ossie.cf import CF, CF__POA #@UnusedImport
from ossie.service import start_service
from omniORB import CORBA, URI, PortableServer

from ossie.cf import CF
from ossie.cf import CF__POA
import threading
import time

class S1(CF__POA.LogLevels):

    def __init__(self, name="S1", execparams={}):
        self.name = name
        self._log = logging.getLogger(self.name)
        logging.getLogger().setLevel(logging.DEBUG)
        self.pause = 0.5
        self.run=True
        self.thread = threading.Thread(target=self.svcf)
        #self.thread.start()

    def svcf(self):
        while self.run:
            self._log.info("service function INFO")
            self._log.warn("service function WARN")
            self._log.error("service function ERROR")
            self._log.fatal("service function FATAL")
            time.sleep(self.pause)
        pass

    def terminateService(self):
        self.run=False
        if self.thread and self.thread.isAlive() : self.thread.join()
        pass


if __name__ == '__main__':
    start_service(S1, thread_policy=PortableServer.SINGLE_THREAD_MODEL)
