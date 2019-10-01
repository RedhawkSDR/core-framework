#!/usr/bin/env python
#
# AUTO-GENERATED
#
# Source: allocmgr_svc.spd.xml

import sys, signal, copy, os
import logging

from ossie.cf import CF, CF__POA #@UnusedImport
from ossie.service import start_service
from omniORB import CORBA, URI, PortableServer

from ossie.cf import CF
from ossie.cf import CF__POA
from ossie import iterators

from ossie.utils import weakobj
import gc
from ossie import gcpoa

class deviterator(iterators.Iterator, CF__POA.DeviceLocationIterator):
    def __init__(self, _list=[]):
        blank = CF.AllocationManager.DeviceLocationType("", [], None, None)
        iterators.Iterator.__init__(self, _list, blank)

class allocmgr_svc(CF__POA.AllocationManager):

    def __init__(self, name="allocmgr_svc", execparams={}):

        orb = CORBA.ORB_init()

        # get the POA
        obj_poa = orb.resolve_initial_references("RootPOA")
        poaManager = obj_poa._get_the_POAManager()

        poaManager.hold_requests(1)
        activator_servant = gcpoa.POACreator()
        activator = activator_servant._this()
        obj_poa.the_activator = activator
        poaManager.activate()

        self.name = name
        self._baseLog = logging.getLogger(self.name)
        self._log = logging.getLogger(self.name)
        self.number_devs = 50000
        self.device_list = []
        for dev_idx in range(self.number_devs):
            self.device_list.append(CF.AllocationManager.DeviceLocationType("", [], None, None))
        self.deviters = []

    def terminateService(self):
        pass

    def listDevices(self, deviceScope, count):
        devlist = []
        blank = CF.AllocationManager.DeviceLocationType("", [], None, None)
        deviter = iterators.get_list(10, self.device_list, deviterator, blank)
        devlist = deviter.next_n(count)[1]
        return devlist, deviter

    def allocate(self, requests):
        pass

    def allocateLocal(self, requests, domainName):
        # TODO
        pass

    def deallocate(self, allocationIDs):
        # TODO
        pass

    def allocations(self, allocationIDs):
        # TODO
        pass

    def localAllocations(self, allocationIDs):
        # TODO
        pass

    def listAllocations(self, allocScope, how_many):
        pass

    def _get_allDevices(self):
        # TODO
        pass

    def _get_authorizedDevices(self):
        # TODO
        pass

    def _get_localDevices(self):
        # TODO
        pass

    def _get_domainMgr(self):
        # TODO
        pass


if __name__ == '__main__':
    start_service(allocmgr_svc, thread_policy=PortableServer.SINGLE_THREAD_MODEL)
