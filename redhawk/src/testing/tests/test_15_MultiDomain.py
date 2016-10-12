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

import signal
import tempfile
import unittest
from _unitTestHelpers import scatest, allocMgrHelpers
from omniORB import URI, any
from ossie.cf import CF
from ossie import properties
from xml.dom import minidom
import os
import time

def launchDomain(number, root):
    domainName = scatest.getTestDomainName() + '_' + str(number)
    _domainBooter = scatest.spawnNodeBooter(dmdFile='', domainname=domainName)
    while _domainBooter.poll() == None:
        _domainManager = None
        try:
            _domainManager = root.resolve(URI.stringToName("%s/%s" % (domainName, domainName)))._narrow(CF.DomainManager)
        except:
            _domainManager = None
        if _domainManager:
            break
        time.sleep(0.1)
    return (_domainBooter, _domainManager)

def _iteratorFetch(result, count=1):
    listitems, listiter = result
    if listiter:
        try:
            status = True
            while status:
                status, items = listiter.next_n(count)
                listitems.extend(items)
        finally:
            listiter.destroy()
    return listitems

class MultiDomainTest(scatest.CorbaTestCase):
    def setUp(self):
        (self._domainBooter_1, self._domainManager_1) = launchDomain(1, self._root)
        (self._domainBooter_2, self._domainManager_2) = launchDomain(2, self._root)

    def tearDown(self):
        if self._domainBooter_1:
            self.terminateChild(self._domainBooter_1)
        if self._domainBooter_2:
            self.terminateChild(self._domainBooter_2)

    def test_MultipleDomainDeployment(self):
        self.assertEqual(len(self._domainManager_1._get_applicationFactories()), 0)
        self.assertEqual(len(self._domainManager_2._get_applicationFactories()), 0)
        
        nb1, execDevNode1 = self.launchDeviceManager("/nodes/test_multiDomain_exec/DeviceManager.dcd.xml", domainManager=self._domainManager_1)
        self.assertNotEqual(execDevNode1, None)

        self._domainManager_1.installApplication("/waveforms/CommandWrapperUsesDevice/CommandWrapper.sad.xml")
        appFact = self._domainManager_1._get_applicationFactories()[0]
        
        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, appFact.create, appFact._get_name(), [], [])

        nb2, basicDevNode1 = self.launchDeviceManager("/nodes/test_multiDomain_uses/DeviceManager.dcd.xml", domainManager=self._domainManager_2)
        self.assertNotEqual(basicDevNode1, None)
        
        self.assertEqual(len(self._domainManager_1._get_deviceManagers()),1)
        self.assertEqual(len(self._domainManager_2._get_deviceManagers()),1)
        
        props=[CF.DataType(id='DCE:8cad8ca5-c155-4d1d-ae40-e194aa1d855f',value=any.to_any(None))]
        value_2 = any.from_any(self._domainManager_2._get_deviceManagers()[0]._get_registeredDevices()[0].query(props)[0].value)
        
        self.assertEqual(len(self._domainManager_1._get_remoteDomainManagers()), 0)
        self._domainManager_1.registerRemoteDomainManager(self._domainManager_2)
        self.assertEqual(len(self._domainManager_1._get_remoteDomainManagers()), 1)

        app = appFact.create(appFact._get_name(), [], []) # LOOK MA, NO DAS!
        self.assertEqual(len(self._domainManager_1._get_applications()), 1)
        app.stop()
        app.releaseObject()

        self.assertEqual(len(self._domainManager_1._get_applicationFactories()), 1)
        self.assertEqual(len(self._domainManager_1._get_applications()), 0)
        self._domainManager_1.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(self._domainManager_1._get_applicationFactories()), 0)

        value_1 = any.from_any(self._domainManager_2._get_deviceManagers()[0]._get_registeredDevices()[0].query(props)[0].value)
        self.assertEquals(value_1, value_2)

    def test_AllocationManagerDevices(self):
        """
        Test to verify that AllocationManagers report the correct devices for
        both localDevices() and allDevices().
        """
        nb1, execDevNode1 = self.launchDeviceManager("/nodes/test_multiDomain_exec/DeviceManager.dcd.xml", domainManager=self._domainManager_1)
        self.assertNotEqual(execDevNode1, None)

        nb2, basicDevNode1 = self.launchDeviceManager("/nodes/test_multiDomain_uses/DeviceManager.dcd.xml", domainManager=self._domainManager_2)
        self.assertNotEqual(basicDevNode1, None)

        # Retrieve the full list of devices in each domain
        devices_1 = allocMgrHelpers.parseDomainDevices(self._domainManager_1)
        devices_2 = allocMgrHelpers.parseDomainDevices(self._domainManager_2)

        # Combine both domains' devices
        devices = {}
        devices.update(devices_1)
        devices.update(devices_2)

        # Ensure that there are no duplicate IDs
        self.assertEqual(len(devices), len(devices_1)+len(devices_2))

        # Connect the domains to each other
        self._domainManager_1.registerRemoteDomainManager(self._domainManager_2)
        allocMgr_1 = self._domainManager_1._get_allocationMgr()
        allocMgr_2 = self._domainManager_2._get_allocationMgr()

        # Check that the first AllocationManager's local devices exactly match
        # the devices found by walking through the domain
        local_1 = allocMgrHelpers.parseDeviceLocations(allocMgr_1._get_localDevices())
        self.assertEqual(devices_1, local_1)

        # Check that all of the devices known to the first AllocationManager
        # matches the full set found by walking through the domains
        all_1 = allocMgrHelpers.parseDeviceLocations(allocMgr_1._get_allDevices())
        self.assertEqual(devices, all_1)

        # The second domain is not linked back to the first, so its idea of
        # all devices should be the same as its local devices
        local_2 = allocMgrHelpers.parseDeviceLocations(allocMgr_2._get_localDevices())
        self.assertEqual(devices_2, local_2)
        all_2 = allocMgrHelpers.parseDeviceLocations(allocMgr_2._get_allDevices())
        self.assertEqual(local_2, all_2)

    def test_AllocationManagerAllocationIterators(self):
        """
        Verifiers that the AllocationManager's allocation iterators return the
        same sets of allocations as the corresponding attributes.
        """
        nb1, execDevNode1 = self.launchDeviceManager("/nodes/test_multiDomain_exec/DeviceManager.dcd.xml", domainManager=self._domainManager_1)
        self.assertNotEqual(execDevNode1, None)

        nb2, basicDevNode1 = self.launchDeviceManager("/nodes/test_multiDomain_uses/DeviceManager.dcd.xml", domainManager=self._domainManager_2)
        self.assertNotEqual(basicDevNode1, None)

        # Connect the domains to each other
        self._domainManager_1.registerRemoteDomainManager(self._domainManager_2)
        allocMgr = self._domainManager_1._get_allocationMgr()

        # Make a couple of allocation requests that we know will have to be
        # split across the two domains
        execcap = {'DCE:8dcef419-b440-4bcf-b893-cab79b6024fb':1000,
                   'DCE:4f9a57fc-8fb3-47f6-b779-3c2692f52cf9':50.0}
        usescap = {'DCE:8cad8ca5-c155-4d1d-ae40-e194aa1d855f':1}
        requests = [allocMgrHelpers.createRequest('exec', properties.props_from_dict(execcap)),
                    allocMgrHelpers.createRequest('uses', properties.props_from_dict(usescap))]
        results = allocMgr.allocate(requests)
        self.assertEqual(len(requests), len(results))

        # Check local allocations
        local_iter = _iteratorFetch(allocMgr.listAllocations(CF.AllocationManager.LOCAL_ALLOCATIONS, 1))
        local_list = allocMgr.localAllocations([])
        self.assertTrue(allocMgrHelpers.compareAllocationStatusSequence(local_iter, local_list))

        # Check all allocations
        all_iter = _iteratorFetch(allocMgr.listAllocations(CF.AllocationManager.ALL_ALLOCATIONS, 1))
        all_list = allocMgr.allocations([])
        self.assertTrue(allocMgrHelpers.compareAllocationStatusSequence(all_iter, all_list))

    def test_AllocationManagerDeviceIterators(self):
        """
        Verifiers that the AllocationManager's device list iterators return the
        same sets of devices as the corresponding attributes.
        """
        nb1, execDevNode1 = self.launchDeviceManager("/nodes/test_multiDomain_exec/DeviceManager.dcd.xml", domainManager=self._domainManager_1)
        self.assertNotEqual(execDevNode1, None)

        nb2, basicDevNode1 = self.launchDeviceManager("/nodes/test_multiDomain_uses/DeviceManager.dcd.xml", domainManager=self._domainManager_2)
        self.assertNotEqual(basicDevNode1, None)

        # Connect the domains to each other
        self._domainManager_1.registerRemoteDomainManager(self._domainManager_2)
        allocMgr = self._domainManager_1._get_allocationMgr()

        # Check local devices
        local_iter = _iteratorFetch(allocMgr.listDevices(CF.AllocationManager.LOCAL_DEVICES, 1))
        local_attr = allocMgr._get_localDevices()
        self.assertEqual(allocMgrHelpers.parseDeviceLocations(local_iter), allocMgrHelpers.parseDeviceLocations(local_attr))

        # Check all devices
        all_iter = _iteratorFetch(allocMgr.listDevices(CF.AllocationManager.ALL_DEVICES, 1))
        all_attr = allocMgr._get_allDevices()
        self.assertEqual(allocMgrHelpers.parseDeviceLocations(all_iter), allocMgrHelpers.parseDeviceLocations(all_attr))

        # Check authorized devices
        auth_iter = _iteratorFetch(allocMgr.listDevices(CF.AllocationManager.AUTHORIZED_DEVICES, 1))
        auth_attr = allocMgr._get_authorizedDevices()
        self.assertEqual(allocMgrHelpers.parseDeviceLocations(auth_iter), allocMgrHelpers.parseDeviceLocations(auth_attr))

    def test_RemoteAllocations(self):
        nb1, execDevNode1 = self.launchDeviceManager("/nodes/test_multiDomain_exec/DeviceManager.dcd.xml", domainManager=self._domainManager_1)
        nb2, basicDevNode1 = self.launchDeviceManager("/nodes/test_multiDomain_uses/DeviceManager.dcd.xml", domainManager=self._domainManager_2)

        # Register second domain with first (no need to do both directions)
        self._domainManager_1.registerRemoteDomainManager(self._domainManager_2)

        allocMgr_1 = self._domainManager_1._get_allocationMgr()
        allocMgr_2 = self._domainManager_2._get_allocationMgr()

        # Check that the initial state of all allocations is empty
        self.assertEqual(allocMgr_1.allocations([]), [])
        self.assertEqual(allocMgr_1.localAllocations([]), [])
        self.assertEqual(allocMgr_2.allocations([]), [])
        self.assertEqual(allocMgr_2.localAllocations([]), [])

        # Make a couple of allocation requests that we know will have to be
        # split across the two domains
        execcap = {'DCE:8dcef419-b440-4bcf-b893-cab79b6024fb':1000,
                   'DCE:4f9a57fc-8fb3-47f6-b779-3c2692f52cf9':50.0}
        usescap = {'DCE:8cad8ca5-c155-4d1d-ae40-e194aa1d855f':1}
        requests = [allocMgrHelpers.createRequest('exec', properties.props_from_dict(execcap),sourceId='TestId'),
                    allocMgrHelpers.createRequest('uses', properties.props_from_dict(usescap))]
        results = dict((r.requestID, r) for r in allocMgr_1.allocate(requests))
        self.assertEqual(len(requests), len(results))
        usesId = results['uses'].allocationID
        execId = results['exec'].allocationID
        allocations = allocMgr_1.listAllocations(CF.AllocationManager.ALL_ALLOCATIONS,100)[0]
        number_checks = 0
        for allocation in allocations:
            if allocation.allocationID == results['uses'].allocationID:
                self.assertEqual(allocation.sourceID,'')
                number_checks += 1
            else:
                self.assertEqual(allocation.sourceID,'TestId')
                number_checks += 1
        self.assertEqual(number_checks,2)

        # The first domain should report the full set of allocations, with only
        # the "exec" allocation showing up in the local allocations
        allocs = allocMgr_1.allocations([])
        self.assertEqual(len(allocs), len(requests))
        for status in allocs:
            if status.allocationID == usesId:
                resId = 'uses'
            elif status.allocationID == execId:
                resId = 'exec'
            else:
                self.fail('Unexpected allocation in results')
            self.assert_(allocMgrHelpers.compareAllocationStatus(status, results[resId]))

        # Try to retrieve a local and remote allocation via allocations
        allocs = allocMgr_1.allocations([execId])
        self.assertEqual(len(allocs), 1)
        self.assert_(allocMgrHelpers.compareAllocationStatus(allocs[0], results['exec']))
        allocs = allocMgr_1.allocations([usesId])
        self.assertEqual(len(allocs), 1)
        self.assert_(allocMgrHelpers.compareAllocationStatus(allocs[0], results['uses']))

        # Make sure we can retrieve the local allocation via localAllocations
        allocs = allocMgr_1.localAllocations([execId])
        self.assertEqual(len(allocs), 1)
        self.assert_(allocMgrHelpers.compareAllocationStatus(allocs[0], results['exec']))

        # Try to retrieve a remote allocation via localAllocations, and make
        # sure that the invalid ID causes an exception
        self.assertRaises(CF.AllocationManager.InvalidAllocationId, allocMgr_1.localAllocations, [usesId])

        # Check the second domain, which should report just the 'uses'
        # allocation, but via both allocations and localAllocations
        allocs = allocMgr_2.allocations([])
        self.assertEqual(len(allocs), 1)
        self.assert_(allocMgrHelpers.compareAllocationStatus(allocs[0], results['uses']))
        allocs = allocMgr_2.localAllocations([])
        self.assertEqual(len(allocs), 1)
        self.assert_(allocMgrHelpers.compareAllocationStatus(allocs[0], results['uses']))

        # The second domain shouldn't know about the local 'exec' allocation
        self.assertRaises(CF.AllocationManager.InvalidAllocationId, allocMgr_2.allocations, [execId])
        self.assertRaises(CF.AllocationManager.InvalidAllocationId, allocMgr_2.localAllocations, [execId])

        # Deallocate the remote allocation and check that only the local 'exec'
        # allocation remains
        allocMgr_1.deallocate([usesId])
        allocs = allocMgr_1.allocations([])
        self.assertEqual(len(allocs), 1)
        self.assert_(allocMgrHelpers.compareAllocationStatus(allocs[0], results['exec']))
        self.assertRaises(CF.AllocationManager.InvalidAllocationId, allocMgr_1.allocations, [usesId])
        allocs = allocMgr_1.localAllocations([])
        self.assertEqual(len(allocs), 1)
        self.assert_(allocMgrHelpers.compareAllocationStatus(allocs[0], results['exec']))

        # The remote domain should have nothing left
        self.assertEqual(allocMgr_2.allocations([]), [])
        self.assertEqual(allocMgr_2.localAllocations([]), [])

class MultiDomainPersistenceTest(scatest.CorbaTestCase):
    def tearDown(self):
        if self._domainBooter_1:
            self.terminateChild(self._domainBooter_1)

    def setUp(self):
        self._dbfile = tempfile.mktemp()
        self._domainBooter_1, self._domainManager_1 = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)
        self._domainBooter_2, self._domainManager_2 = launchDomain(2, self._root)

    def tearDown(self):
        self.terminateChild(self._domainBooter_2)
        scatest.CorbaTestCase.tearDown(self)
        try:
            os.remove(self._dbfile)
        except OSError:
            pass

    def test_AllocationPersistence(self):
        self.launchDeviceManager("/nodes/test_multiDomain_exec/DeviceManager.dcd.xml", domainManager=self._domainManager_1)
        self.launchDeviceManager("/nodes/test_multiDomain_uses/DeviceManager.dcd.xml", domainManager=self._domainManager_2)
        self._domainManager_1.registerRemoteDomainManager(self._domainManager_2)

        allocMgr_1 = self._domainManager_1._get_allocationMgr()

        # Make a couple of allocation requests that we know will have to be
        # split across the two domains
        execcap = {'DCE:8dcef419-b440-4bcf-b893-cab79b6024fb':1000,
                   'DCE:4f9a57fc-8fb3-47f6-b779-3c2692f52cf9':50.0}
        usescap = {'DCE:8cad8ca5-c155-4d1d-ae40-e194aa1d855f':1}
        requests = [allocMgrHelpers.createRequest('exec', properties.props_from_dict(execcap)),
                    allocMgrHelpers.createRequest('uses', properties.props_from_dict(usescap))]
        results = dict((r.requestID, r) for r in allocMgr_1.allocate(requests))
        self.assertEqual(len(requests), len(results))
        usesId = results['uses'].allocationID
        execId = results['exec'].allocationID

        # Save the current allocation state
        pre = dict((al.allocationID, al) for al in allocMgr_1.allocations([]))

        # Kill the DomainManager
        os.kill(self._domainBooter_1.pid, signal.SIGTERM)
        if not self.waitTermination(self._domainBooter_1):
            self.fail("Domain Manager Failed to Die")

        # Re-launch and check that the allocation state remains the same
        self.launchDomainManager(endpoint='giop:tcp::5679', dbURI=self._dbfile)
        post = dict((al.allocationID, al) for al in allocMgr_1.allocations([]))
        self.assertEqual(len(pre), len(post))
        self.assertEqual(pre.keys(), post.keys())
        for allocId, status in pre.iteritems():
            self.assert_(allocMgrHelpers.compareAllocationStatus(status, post[allocId]))

# Only run these tests if persistence was enabled at compile time
if not scatest.persistenceEnabled():
    del MultiDomainPersistenceTest

if __name__ == "__main__":
  # Run the unittests
  unittest.main()
