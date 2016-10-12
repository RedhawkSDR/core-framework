#!/usr/bin/env python
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
