#!/usr/bin/env python
#
# AUTO-GENERATED
#
# Source: py_svc_exec_params.spd.xml

import sys, signal, copy, os
import logging

from ossie.cf import CF, CF__POA #@UnusedImport
from ossie.service import start_service
from omniORB import CORBA, URI, PortableServer
from ossie import properties
from ossie.properties import simple_property


from ossie.cf import CF
from ossie.cf import CF__POA

class py_svc_exec_params(CF__POA.TestableObject):

    def __init__(self, name="py_svc_exec_params", execparams={}):
        self.name = name
        self._log = logging.getLogger(self.name)

    def terminateService(self):
        pass

    def runTest(self, testid, testValues):
	pass


if __name__ == '__main__':
    start_service(py_svc_exec_params, thread_policy=PortableServer.SINGLE_THREAD_MODEL)
