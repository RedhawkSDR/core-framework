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
# Source: svc_a.spd.xml

import sys, signal, copy, os
import logging

from ossie.cf import CF, CF__POA #@UnusedImport
from ossie.service import start_service
from omniORB import CORBA, URI, PortableServer

from ossie.cf import CF
from ossie.cf import CF__POA
import bulkio

class svc_a(CF__POA.PortSupplier):

    def __init__(self, name="svc_a", execparams={}):
        self.name = name
        self._log = logging.getLogger(self.name)
        self.port_dataFloat = bulkio.OutFloatPort("dataFloat")

    def terminateService(self):
        pass

    def getPort(self, name):
        if name == 'dataFloat':
            return self.port_dataFloat._this()
        raise CF.PortSupplier.UnknownPort()


if __name__ == '__main__':
    start_service(svc_a, thread_policy=PortableServer.SINGLE_THREAD_MODEL)
