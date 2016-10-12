#!/usr/bin/env python
#
# AUTO-GENERATED
#
# Source: svc_b.spd.xml

import sys, signal, copy, os
import logging

from ossie.cf import CF, CF__POA #@UnusedImport
from ossie.service import start_service
from omniORB import CORBA, URI, PortableServer

from ossie.cf import CF
from ossie.cf import CF__POA
import bulkio

class svc_b(CF__POA.PortSupplier):

    def __init__(self, name="svc_b", execparams={}):
        self.name = name
        self._log = logging.getLogger(self.name)
        self.port_dataFloat = bulkio.InFloatPort("dataFloat", maxsize=10)

    def terminateService(self):
        pass

    def getPort(self, name):
        if name == 'dataFloat_':
            return self.port_dataFloat._this()
        raise CF.PortSupplier.UnknownPort()


if __name__ == '__main__':
    start_service(svc_b, thread_policy=PortableServer.SINGLE_THREAD_MODEL)
