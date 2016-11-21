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
# Source: fake.spd.xml

import sys, signal, copy, os
import logging

from ossie.cf import CF, CF__POA #@UnusedImport
from ossie.service import start_service
from omniORB import CORBA, URI, PortableServer

from ossie.cf import ExtendedEvent
from ossie.cf import ExtendedEvent__POA

class fake(ExtendedEvent__POA.MessageEvent):

    def __init__(self, name="fake", execparams={}):
        self.name = name
        self._log = logging.getLogger(self.name)

    def terminateService(self):
        pass

    def for_consumers(self):
        # TODO
        pass

    def for_suppliers(self):
        # TODO
        pass

    def destroy(self):
        # TODO
        pass

    def connectPort(self, connection, connectionId):
        # TODO
        pass

    def disconnectPort(self, connectionId):
        # TODO
        pass


if __name__ == '__main__':
    start_service(fake, thread_policy=PortableServer.SINGLE_THREAD_MODEL)
