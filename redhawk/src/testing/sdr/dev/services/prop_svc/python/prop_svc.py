#!/usr/bin/env python
#
# AUTO-GENERATED
#
# Source: prop_svc.spd.xml

import sys, signal, copy, os
import logging

from ossie.cf import CF, CF__POA #@UnusedImport
from ossie.service import start_service
from omniORB import CORBA, URI, PortableServer

from ossie.cf import CF
from ossie.cf import CF__POA

class prop_svc(CF__POA.PropertySet):

    def __init__(self, name="prop_svc", execparams={}):
        self.name = name
        self._log = logging.getLogger(self.name)

    def terminateService(self):
        pass

    def configure(self, configProperties):
        # TODO
        pass

    def query(self, configProperties):
        # TODO
        pass


if __name__ == '__main__':
    start_service(prop_svc, thread_policy=PortableServer.SINGLE_THREAD_MODEL)
