#!/usr/bin/env python
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
