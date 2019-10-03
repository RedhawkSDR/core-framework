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

import copy
from ossie import gcpoa, iterators

class deviterator(iterators.Iterator, CF__POA.DeviceLocationIterator):
    def __init__(self, _list=[]):
        iterators.Iterator.__init__(self, CF.AllocationManager.DeviceLocationType, _list)

class allociterator(iterators.Iterator, CF__POA.AllocationStatusIterator):
    def __init__(self, _list=[]):
        iterators.Iterator.__init__(self, CF.AllocationManager.AllocationStatusType, _list)

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
        blank_devicelocationtype = iterators.construct(CF.AllocationManager.DeviceLocationType)
        for dev_idx in range(self.number_devs):
            self.device_list.append(copy.deepcopy(blank_devicelocationtype))

        self.allocation_list = []
        updated_allocationstatustype = iterators.construct(CF.AllocationManager.AllocationStatusType)
        updated_allocationstatustype.allocationID = 'sample_id'
        source_count = 0
        for dev_idx in range(20):
            source_count += 1
            updated_allocationstatustype.sourceID = str(source_count)
            self.allocation_list.append(copy.deepcopy(updated_allocationstatustype))

        self.deviters = []

    def terminateService(self):
        pass

    def listDevices(self, deviceScope, count):
        devlist = []
        #deviter = iterators.get_list_iterator(self.device_list, deviterator, 0.2)
        deviter = deviterator.get_list_iterator(self.device_list, 0.2)
        if deviter == None:
            return [], None
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
        alloclist = []
        #allociter = iterators.get_list_iterator(self.allocation_list, allociterator, 0.2)
        allociter = allociterator.get_list_iterator(self.allocation_list, 0.2)
        if allociter == None:
            return [], None
        alloclist = allociter.next_n(how_many)[1]
        return alloclist, allociter

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
