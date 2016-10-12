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

import unittest
from _unitTestHelpers import scatest, allocMgrHelpers
from omniORB import any as _any
from ossie.cf import CF
from ossie.cf import ExtendedCF
from ossie import properties

class AllocationManagerTest(scatest.CorbaTestCase):
    def setUp(self):
        super(AllocationManagerTest,self).setUp()
        nb, self._domMgr = self.launchDomainManager()
        self._allocMgr = self._domMgr._get_allocationMgr()

    def _tryAllocation(self, props):
        request = [allocMgrHelpers.createRequest('test', properties.props_from_dict(props))]
        response = self._allocMgr.allocate(request)
        if response:
            self._allocMgr.deallocate([r.allocationID for r in response])
        return len(response) == len(request)

    def _tryAllocation2(self, am, props):
        request = [allocMgrHelpers.createRequest('test', properties.props_from_dict(props))]
        response = am.allocate(request)
        if response:
            am.deallocate([r.allocationID for r in response])
        return len(response) == len(request)


    def test_MatchingProperties(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_Matching_node/DeviceManager.dcd.xml')
        self.assertNotEqual(devMgr, None)
        self.assertTrue(len(devMgr._get_registeredDevices()) > 0)
        dev = devMgr._get_registeredDevices()[0]

        self.assertTrue(self._tryAllocation({'test_eq': 'default'}))
        self.assertFalse(self._tryAllocation({'test_eq': 'other'}))

        self.assertTrue(self._tryAllocation({'test_ne': 1}))
        self.assertFalse(self._tryAllocation({'test_ne': 0}))

        self.assertTrue(self._tryAllocation({'test_lt': 1}))
        self.assertFalse(self._tryAllocation({'test_lt': 0}))
        self.assertFalse(self._tryAllocation({'test_lt': -1}))

        self.assertTrue(self._tryAllocation({'test_le': 1}))
        self.assertTrue(self._tryAllocation({'test_le': 0}))
        self.assertFalse(self._tryAllocation({'test_le': -1}))

        self.assertFalse(self._tryAllocation({'test_gt': 1}))
        self.assertFalse(self._tryAllocation({'test_gt': 0}))
        self.assertTrue(self._tryAllocation({'test_gt': -1}))

        self.assertFalse(self._tryAllocation({'test_ge': 1}))
        self.assertTrue(self._tryAllocation({'test_ge': 0}))
        self.assertTrue(self._tryAllocation({'test_ge': -1}))

        self.assertTrue(self._tryAllocation({ExtendedCF.WKP.OS_NAME: 'OS/2'}))
        self.assertFalse(self._tryAllocation({ExtendedCF.WKP.OS_NAME: 'Linux'}))

        # Check that implementation-specific properties are overridden
        self.assertFalse(self._tryAllocation({'impl_defined': 'unknown'}))
        self.assertTrue(self._tryAllocation({'impl_defined': 'python'}))

    def test_MatchingPropertiesDCDOverride(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_MatchingDCDOverride_node/DeviceManager.dcd.xml')
        self.assertNotEqual(devMgr, None)
        self.assertTrue(len(devMgr._get_registeredDevices()) > 0)
        dev = devMgr._get_registeredDevices()[0]

        self.assertFalse(self._tryAllocation({'test_eq': 'default'}))
        self.assertTrue(self._tryAllocation({'test_eq': 'override'}))

        self.assertFalse(self._tryAllocation({'test_ne': 1}))
        self.assertTrue(self._tryAllocation({'test_ne': 0}))

        self.assertTrue(self._tryAllocation({'test_lt': 2}))
        self.assertFalse(self._tryAllocation({'test_lt': 1}))
        self.assertFalse(self._tryAllocation({'test_lt': 0}))

        self.assertTrue(self._tryAllocation({'test_le': 2}))
        self.assertTrue(self._tryAllocation({'test_le': 1}))
        self.assertFalse(self._tryAllocation({'test_le': 0}))

        self.assertFalse(self._tryAllocation({'test_gt': 2}))
        self.assertFalse(self._tryAllocation({'test_gt': 1}))
        self.assertTrue(self._tryAllocation({'test_gt': 0}))

        self.assertFalse(self._tryAllocation({'test_ge': 2}))
        self.assertTrue(self._tryAllocation({'test_ge': 1}))
        self.assertTrue(self._tryAllocation({'test_ge': 0}))

        self.assertFalse(self._tryAllocation({ExtendedCF.WKP.OS_NAME: 'OS/2'}))
        self.assertTrue(self._tryAllocation({ExtendedCF.WKP.OS_NAME: 'Linux'}))

    def test_ExternalProperties(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_collocation_good_node/DeviceManager.dcd.xml')

        #self._tryAllocation({ExtendedCF.WKP.OS_VERSION:'1'})
        #self._tryAllocation({'os_name':'Linux'})
        #self._tryAllocation({'supported_components': 5})
        #self._tryAllocation({'supported_components': 5, 'os_name':'Linux'})

        props = [('supported_components', 1), ('supported_components', 1)]
        allocProps = [CF.DataType(key, _any.to_any(value)) for key, value in props]
        request = [allocMgrHelpers.createRequest('test', allocProps)]
        response = self._allocMgr.allocate(request)
        if response:
            self._allocMgr.deallocate([r.allocationID for r in response])
        return len(response) == len(request)

    def test_MultipleRequests(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_SADUsesDevice/DeviceManager.dcd.xml')

        # Try two requests that should succeed
        props = properties.props_from_dict({'simple_alloc': 1})
        request = [allocMgrHelpers.createRequest('test1', props), allocMgrHelpers.createRequest('test2', props)]
        response = self._allocMgr.allocate(request)
        self.assertEqual(len(request), len(response))
        self._allocMgr.deallocate([r.allocationID for r in response])

        # The second request should fail
        props = properties.props_from_dict({'simple_alloc': 8})
        request = [allocMgrHelpers.createRequest('test1', props), allocMgrHelpers.createRequest('test2', props)]
        response = self._allocMgr.allocate(request)
        good_requests = [r.requestID for r in response]
        self.assertTrue(len(request) > len(response))
        self.assertTrue('test1' in good_requests)
        self.assertFalse('test2' in good_requests)
        self._allocMgr.deallocate([r.allocationID for r in response])

        # The first and second requests should fail, but the third should succeed
        bad_props = {'simple_alloc': 12}
        good_props = {'simple_alloc': 8}
        request = [('test1', bad_props), ('test2', bad_props), ('test3', good_props)]
        request = [allocMgrHelpers.createRequest(k, properties.props_from_dict(v)) for k, v in request]
        response = self._allocMgr.allocate(request)
        good_requests = [r.requestID for r in response]
        self.assertTrue(len(request) > len(response))
        self.assertEqual(good_requests, ['test3'])
        self._allocMgr.deallocate([r.allocationID for r in response])

        # Ensure that different requests can be allocated to different devices
        request = [('external', {'simple_alloc': 1}),
                   ('matching', {'DCE:ac73446e-f935-40b6-8b8d-4d9adb6b403f':2,
                                 'DCE:7f36cdfb-f828-4e4f-b84f-446e17f1a85b':'BasicTestDevice'})]
        request = [allocMgrHelpers.createRequest(k, properties.props_from_dict(v)) for k, v in request]
        response = dict((r.requestID, r) for r in self._allocMgr.allocate(request))
        self.assertEqual(len(request), len(response))
        self.assertFalse(response['external'].allocatedDevice._is_equivalent(response['matching'].allocatedDevice))
        self._allocMgr.deallocate([r.allocationID for r in response.values()])

    def test_allocationsMethod(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_SADUsesDevice/DeviceManager.dcd.xml')

        # Check that there are no allocations reported
        allocs = self._allocMgr.allocations([])
        self.assertEqual(len(allocs), 0)

        # Make a single allocation request and check that it looks right
        props = properties.props_from_dict({'simple_alloc': 1})
        request = [allocMgrHelpers.createRequest('test1', props)]
        response = self._allocMgr.allocate(request)
        self.assertEqual(len(request), len(response))
        self.assertEqual(request[0].requestID, response[0].requestID)
        
        # Save allocation IDs for later checks
        allocIDs = [resp.allocationID for resp in response]

        # Check that the reported allocations match expectations
        allocs = self._allocMgr.allocations([])
        self.assertEqual(len(allocs), 1)
        self.assertEqual(allocs[0].allocationID, allocIDs[0])

        # Make two more allocation requests
        request = [('external', {'simple_alloc': 1}),
                   ('matching', {'DCE:ac73446e-f935-40b6-8b8d-4d9adb6b403f':2,
                                 'DCE:7f36cdfb-f828-4e4f-b84f-446e17f1a85b':'BasicTestDevice'})]
        request = [allocMgrHelpers.createRequest(k, properties.props_from_dict(v)) for k, v in request]
        response = self._allocMgr.allocate(request)
        self.assertEqual(len(request), len(response))
        allocIDs.extend(resp.allocationID for resp in response)

        allocs = self._allocMgr.allocations([])
        self.assertEqual(len(allocs), 3)

        # Try to retrieve an invalid allocation ID, making sure it throws an
        # exception
        self.assertRaises(CF.AllocationManager.InvalidAllocationId, self._allocMgr.allocations, ['missing'])

        # Check that we can retrieve a specific allocation
        allocs = self._allocMgr.allocations(allocIDs[-1:])
        self.assertEqual(len(allocs), 1)

    def test_BasicOperations(self):
        # Check that the domain manager back link is correct
        domMgr = self._allocMgr._get_domainMgr()
        self.assert_(self._domMgr._is_equivalent(domMgr))

        # Check that the device list attributes work as expected (with no
        # devices), and do not throw exceptions
        self.assertEqual(self._allocMgr._get_localDevices(), [])
        self.assertEqual(self._allocMgr._get_allDevices(), [])
        self.assertEqual(self._allocMgr._get_authorizedDevices(), [])

        # Check that the allocation list functions work as expected (with no
        # devices), and do not throw exceptions
        self.assertEqual(self._allocMgr.allocations([]), [])
        self.assertEqual(self._allocMgr.localAllocations([]), [])

    def test_DeviceLists(self):
        """
        Tests the operation of the device list attributes (allDevices,
        authorizedDevices, localDevices).
        """
        # Keep adding nodes and testing the device lists to ensure that the
        # state is up-to-date
        devCount = 0
        for node in ('test_collocation_good_node', 'test_SADUsesDevice', 'test_MultipleExecutableDevice_node'):
            nb, devMgr = self.launchDeviceManager('/nodes/'+node+'/DeviceManager.dcd.xml')

            # Collect the complete set of device IDs, making sure new devices
            # are added every time through the loop
            devices = allocMgrHelpers.parseDomainDevices(self._domMgr)
            self.assert_(len(devices) > devCount)
            devCount = len(devices)

            # Make sure localDevices matches our known state
            localDevices = allocMgrHelpers.parseDeviceLocations(self._allocMgr._get_localDevices())
            self.assertEqual(devices, localDevices)

            # No policy is applied in default implementation, so authorized devices
            # should be the complete set of local devices
            authDevices = allocMgrHelpers.parseDeviceLocations(self._allocMgr._get_authorizedDevices())
            self.assertEqual(devices, authDevices)

            # Make sure allDevices matches our known state; since there are no
            # remote domains, it should be the same as localDevices
            allDevices = allocMgrHelpers.parseDeviceLocations(self._allocMgr._get_allDevices())
            self.assertEqual(devices, allDevices)

    def test_DeviceIterators(self):
        """
        Tests the operation of the device list iterators.
        """
        nb, devMgr = self.launchDeviceManager('/nodes/test_MultipleExecutableDevice_node/DeviceManager.dcd.xml')
        devices = allocMgrHelpers.parseDomainDevices(self._domMgr)

        # First, try to list more devices than there are in the system, to make
        # sure no iterator is returned
        devlist, deviter = self._allocMgr.listDevices(CF.AllocationManager.LOCAL_DEVICES, 10)
        self.assertEqual(allocMgrHelpers.parseDeviceLocations(devlist), devices)
        self.assertEqual(deviter, None)

        # Next, start with fewer devices and fetch one-by-one via the iterator
        devlist, deviter = self._allocMgr.listDevices(CF.AllocationManager.LOCAL_DEVICES, 1)
        self.assertEqual(len(devlist), 1)
        self.assertNotEqual(deviter, None)
        try:
            # There has to be at least one more device
            status, item = deviter.next_one()
            self.assertTrue(status)
            self.assertNotEqual(item, None)
            devlist.append(item)

            # Fetch the remainder
            while status:
                status, item = deviter.next_one()
                if status:
                    devlist.append(item)
        finally:
            deviter.destroy()
        # Check the resulting list
        self.assertEqual(allocMgrHelpers.parseDeviceLocations(devlist), devices)

        # Then try fetching by a higher count
        devlist, deviter = self._allocMgr.listDevices(CF.AllocationManager.LOCAL_DEVICES, 1)
        self.assertEqual(len(devlist), 1)
        self.assertNotEqual(deviter, None)
        try:
            # Try to fetch 2 more, which ought to succeed in full
            status, items = deviter.next_n(2)
            self.assertTrue(status)
            self.assertEqual(len(items), 2)
            devlist.extend(items)

            # Try 2 more, which should return only 1
            status, items = deviter.next_n(2)
            self.assertTrue(status)
            self.assertEqual(len(items), 1)
            devlist.extend(items)

            # Finally, the next fetch should fail
            status, items = deviter.next_n(2)
            self.assertFalse(status)
            self.assertEqual(len(items), 0)
        finally:
            deviter.destroy()
        # Check the resulting list
        self.assertEqual(allocMgrHelpers.parseDeviceLocations(devlist), devices)

    def test_AllocationIterators(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_SADUsesDevice/DeviceManager.dcd.xml')
        # Set initial state to 4 allocations
        request = [('test1', {'simple_alloc': 1}),
                   ('test2', {'simple_alloc': 1}),
                   ('external', {'simple_alloc': 1}),
                   ('matching', {'DCE:ac73446e-f935-40b6-8b8d-4d9adb6b403f':2,
                                 'DCE:7f36cdfb-f828-4e4f-b84f-446e17f1a85b':'BasicTestDevice'})]
        request = [allocMgrHelpers.createRequest(k, properties.props_from_dict(v)) for k, v in request]
        response = self._allocMgr.allocate(request)
        self.assertEqual(len(request), len(response))

        localAllocs = self._allocMgr.localAllocations([])

        # First, try to list more allocations than have been made, to make sure
        # no iterator is returned
        allocs, allociter = self._allocMgr.listAllocations(CF.AllocationManager.LOCAL_ALLOCATIONS, 10)
        self.assertTrue(allocMgrHelpers.compareAllocationStatusSequence(allocs, localAllocs))
        self.assertEqual(allociter, None)

        # Next, start with fewer allocations and fetch one-by-one via the iterator
        allocs, allociter = self._allocMgr.listAllocations(CF.AllocationManager.LOCAL_ALLOCATIONS, 1)
        self.assertEqual(len(allocs), 1)
        self.assertNotEqual(allociter, None)
        try:
            # There has to be at least one more allocation
            status, item = allociter.next_one()
            self.assertTrue(status)
            self.assertNotEqual(item, None)
            allocs.append(item)

            # Fetch the remainder
            while status:
                status, item = allociter.next_one()
                if status:
                    allocs.append(item)
        finally:
            allociter.destroy()
        # Check the resulting list
        self.assertTrue(allocMgrHelpers.compareAllocationStatusSequence(allocs, localAllocs))

        # Then try fetching by a higher count
        allocs, allociter = self._allocMgr.listAllocations(CF.AllocationManager.LOCAL_ALLOCATIONS, 1)
        self.assertEqual(len(allocs), 1)
        self.assertNotEqual(allociter, None)
        try:
            # Try to fetch 2 more, which ought to succeed in full
            status, items = allociter.next_n(2)
            self.assertTrue(status)
            self.assertEqual(len(items), 2)
            allocs.extend(items)

            # Try 2 more, which should return only 1
            status, items = allociter.next_n(2)
            self.assertTrue(status)
            self.assertEqual(len(items), 1)
            allocs.extend(items)

            # Finally, the next fetch should fail
            status, items = allociter.next_n(2)
            self.assertFalse(status)
            self.assertEqual(len(items), 0)
        finally:
            allociter.destroy()
        # Check the resulting list
        self.assertTrue(allocMgrHelpers.compareAllocationStatusSequence(allocs, localAllocs))



class AllocationManagerTestRedhawkUtils(scatest.CorbaTestCase):
    def setUp(self):
        super(AllocationManagerTestRedhawkUtils,self).setUp()
        nb, self._domMgr = self.launchDomainManager()
        self._allocMgr = self._domMgr._get_allocationMgr()
        from ossie.utils import redhawk
        d=None
        d = redhawk.Domain(self._domMgr._get_name())
        self.assertNotEqual(d, None )

        # Get AllocationManager
        self.am=None
        self.am = d.getAllocationMgr()
        self.assertNotEqual(self.am, None )


    def _tryAllocation(self, props):
        request = [allocMgrHelpers.createRequest('test', properties.props_from_dict(props))]
        response = self.am.allocate(request)
        if response:
            self.am.deallocate([r.allocationID for r in response])
        return len(response) == len(request)


    def test_redhawkutils_MatchingProperties(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_Matching_node/DeviceManager.dcd.xml')
        self.assertNotEqual(devMgr, None)
        self.assertTrue(len(devMgr._get_registeredDevices()) > 0)
        dev = devMgr._get_registeredDevices()[0]

        self.assertTrue(self._tryAllocation({'test_eq': 'default'}))
        self.assertFalse(self._tryAllocation({'test_eq': 'other'}))

        self.assertTrue(self._tryAllocation({'test_ne': 1}))
        self.assertFalse(self._tryAllocation({'test_ne': 0}))

        self.assertTrue(self._tryAllocation({'test_lt': 1}))
        self.assertFalse(self._tryAllocation({'test_lt': 0}))
        self.assertFalse(self._tryAllocation({'test_lt': -1}))

        self.assertTrue(self._tryAllocation({'test_le': 1}))
        self.assertTrue(self._tryAllocation({'test_le': 0}))
        self.assertFalse(self._tryAllocation({'test_le': -1}))

        self.assertFalse(self._tryAllocation({'test_gt': 1}))
        self.assertFalse(self._tryAllocation({'test_gt': 0}))
        self.assertTrue(self._tryAllocation({'test_gt': -1}))

        self.assertFalse(self._tryAllocation({'test_ge': 1}))
        self.assertTrue(self._tryAllocation({'test_ge': 0}))
        self.assertTrue(self._tryAllocation({'test_ge': -1}))

        self.assertTrue(self._tryAllocation({ExtendedCF.WKP.OS_NAME: 'OS/2'}))
        self.assertFalse(self._tryAllocation({ExtendedCF.WKP.OS_NAME: 'Linux'}))

        # Check that implementation-specific properties are overridden
        self.assertFalse(self._tryAllocation({'impl_defined': 'unknown'}))
        self.assertTrue(self._tryAllocation({'impl_defined': 'python'}))

    def test_redhawkutils_MatchingPropertiesDCDOverride(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_MatchingDCDOverride_node/DeviceManager.dcd.xml')
        self.assertNotEqual(devMgr, None)
        self.assertTrue(len(devMgr._get_registeredDevices()) > 0)
        dev = devMgr._get_registeredDevices()[0]

        self.assertFalse(self._tryAllocation({'test_eq': 'default'}))
        self.assertTrue(self._tryAllocation({'test_eq': 'override'}))

        self.assertFalse(self._tryAllocation({'test_ne': 1}))
        self.assertTrue(self._tryAllocation({'test_ne': 0}))

        self.assertTrue(self._tryAllocation({'test_lt': 2}))
        self.assertFalse(self._tryAllocation({'test_lt': 1}))
        self.assertFalse(self._tryAllocation({'test_lt': 0}))

        self.assertTrue(self._tryAllocation({'test_le': 2}))
        self.assertTrue(self._tryAllocation({'test_le': 1}))
        self.assertFalse(self._tryAllocation({'test_le': 0}))

        self.assertFalse(self._tryAllocation({'test_gt': 2}))
        self.assertFalse(self._tryAllocation({'test_gt': 1}))
        self.assertTrue(self._tryAllocation({'test_gt': 0}))

        self.assertFalse(self._tryAllocation({'test_ge': 2}))
        self.assertTrue(self._tryAllocation({'test_ge': 1}))
        self.assertTrue(self._tryAllocation({'test_ge': 0}))

        self.assertFalse(self._tryAllocation({ExtendedCF.WKP.OS_NAME: 'OS/2'}))
        self.assertTrue(self._tryAllocation({ExtendedCF.WKP.OS_NAME: 'Linux'}))


    def test_redhawkutils_ExternalProperties(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_collocation_good_node/DeviceManager.dcd.xml')

        #self._tryAllocation({ExtendedCF.WKP.OS_VERSION:'1'})
        #self._tryAllocation({'os_name':'Linux'})
        #self._tryAllocation({'supported_components': 5})
        #self._tryAllocation({'supported_components': 5, 'os_name':'Linux'})

        props = [('supported_components', 1), ('supported_components', 1)]
        allocProps = [CF.DataType(key, _any.to_any(value)) for key, value in props]
        request = [self.am.createRequest('test', allocProps)]
        response = self.am.allocate(request)
        if response:
            self.am.deallocate([r.allocationID for r in response])
        return len(response) == len(request)

    def test_redhawkutils_MultipleRequests(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_SADUsesDevice/DeviceManager.dcd.xml')

        # Try two requests that should succeed
        props = properties.props_from_dict({'simple_alloc': 1})
        request = [self.am.createRequest('test1', props), self.am.createRequest('test2', props)]
        response = self.am.allocate(request)
        self.assertEqual(len(request), len(response))
        self.am.deallocate([r.allocationID for r in response])

        # The second request should fail
        props = properties.props_from_dict({'simple_alloc': 8})
        request = [self.am.createRequest('test1', props), self.am.createRequest('test2', props)]
        response = self.am.allocate(request)
        good_requests = [r.requestID for r in response]
        self.assertTrue(len(request) > len(response))
        self.assertTrue('test1' in good_requests)
        self.assertFalse('test2' in good_requests)
        self.am.deallocate([r.allocationID for r in response])

        # The first and second requests should fail, but the third should succeed
        bad_props = {'simple_alloc': 12}
        good_props = {'simple_alloc': 8}
        request = [('test1', bad_props), ('test2', bad_props), ('test3', good_props)]
        request = [self.am.createRequest(k, properties.props_from_dict(v)) for k, v in request]
        response = self.am.allocate(request)
        good_requests = [r.requestID for r in response]
        self.assertTrue(len(request) > len(response))
        self.assertEqual(good_requests, ['test3'])
        self.am.deallocate([r.allocationID for r in response])

        # Ensure that different requests can be allocated to different devices
        request = [('external', {'simple_alloc': 1}),
                   ('matching', {'DCE:ac73446e-f935-40b6-8b8d-4d9adb6b403f':2,
                                 'DCE:7f36cdfb-f828-4e4f-b84f-446e17f1a85b':'BasicTestDevice'})]
        request = [self.am.createRequest(k, properties.props_from_dict(v)) for k, v in request]
        response = dict((r.requestID, r) for r in self.am.allocate(request))
        self.assertEqual(len(request), len(response))
        self.assertFalse(response['external'].allocatedDevice._is_equivalent(response['matching'].allocatedDevice))
        self.am.deallocate([r.allocationID for r in response.values()])

    def test_redhawkutils_allocationsMethod(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_SADUsesDevice/DeviceManager.dcd.xml')

        # Check that there are no allocations reported
        allocs = self.am.allocations([])
        self.assertEqual(len(allocs), 0)

        # Make a single allocation request and check that it looks right
        props = properties.props_from_dict({'simple_alloc': 1})
        request = [self.am.createRequest('test1', props)]
        response = self.am.allocate(request)
        self.assertEqual(len(request), len(response))
        self.assertEqual(request[0].requestID, response[0].requestID)
        
        # Save allocation IDs for later checks
        allocIDs = [resp.allocationID for resp in response]

        # Check that the reported allocations match expectations
        allocs = self.am.allocations([])
        self.assertEqual(len(allocs), 1)
        self.assertEqual(allocs[0].allocationID, allocIDs[0])

        # Make two more allocation requests
        request = [('external', {'simple_alloc': 1}),
                   ('matching', {'DCE:ac73446e-f935-40b6-8b8d-4d9adb6b403f':2,
                                 'DCE:7f36cdfb-f828-4e4f-b84f-446e17f1a85b':'BasicTestDevice'})]
        request = [self.am.createRequest(k, properties.props_from_dict(v)) for k, v in request]
        response = self.am.allocate(request)
        self.assertEqual(len(request), len(response))
        allocIDs.extend(resp.allocationID for resp in response)

        allocs = self.am.allocations([])
        self.assertEqual(len(allocs), 3)

        # Try to retrieve an invalid allocation ID, making sure it throws an
        # exception
        self.assertRaises(CF.AllocationManager.InvalidAllocationId, self.am.allocations, ['missing'])

        # Check that we can retrieve a specific allocation
        allocs = self.am.allocations(allocIDs[-1:])
        self.assertEqual(len(allocs), 1)



    def test_redhawkutils_AllocationIterators(self):
        nb, devMgr = self.launchDeviceManager('/nodes/test_SADUsesDevice/DeviceManager.dcd.xml')
        # Set initial state to 4 allocations
        request = [('test1', {'simple_alloc': 1}),
                   ('test2', {'simple_alloc': 1}),
                   ('external', {'simple_alloc': 1}),
                   ('matching', {'DCE:ac73446e-f935-40b6-8b8d-4d9adb6b403f':2,
                                 'DCE:7f36cdfb-f828-4e4f-b84f-446e17f1a85b':'BasicTestDevice'})]

        request = [ self.am.createRequest(k, properties.props_from_dict(v)) for k, v in request]
        response = self.am.allocate(request)
        self.assertEqual(len(request), len(response))

        localAllocs = self.am.allocations([], True)

        # First, try to list more allocations than have been made, to make sure
        # no iterator is returned
        allocs, allociter = self.am.listAllocations( self.am.LOCAL_ALLOCATIONS, 10)
        self.assertTrue(allocMgrHelpers.compareAllocationStatusSequence(allocs, localAllocs))
        self.assertEqual(allociter, None)

        # Next, start with fewer allocations and fetch one-by-one via the iterator
        allocs, allociter = self.am.listAllocations( self.am.LOCAL_ALLOCATIONS, 1)
        self.assertEqual(len(allocs), 1)
        self.assertNotEqual(allociter, None)
        try:
            # There has to be at least one more allocation
            item = allociter.next_one()
            self.assertNotEqual(item, None)
            allocs.append(item)

            # Fetch the remainder
            while item:
                item = allociter.next_one()
                if item:
                    allocs.append(item)
        finally:
            pass

        # Check the resulting list
        self.assertTrue(allocMgrHelpers.compareAllocationStatusSequence(allocs, localAllocs))

        # Then try fetching by a higher count
        allocs, allociter = self.am.listAllocations( self.am.LOCAL_ALLOCATIONS, 1)
        self.assertEqual(len(allocs), 1)
        self.assertNotEqual(allociter, None)
        try:
            # Try to fetch 2 more, which ought to succeed in full
            items = allociter.next_n(2)
            self.assertEqual(len(items), 2)
            allocs.extend(items)

            # Try 2 more, which should return only 1
            items = allociter.next_n(2)
            self.assertEqual(len(items), 1)
            allocs.extend(items)

            # Finally, the next fetch should fail
            items = allociter.next_n(2)
            self.assertEqual(items, None)
        finally:
            pass

        # Check the resulting list
        self.assertTrue(allocMgrHelpers.compareAllocationStatusSequence(allocs, localAllocs))


    def test_redhawkutils_DeviceIterators(self):
        """
        Tests the operation of the device list iterators.
        """
        nb, devMgr = self.launchDeviceManager('/nodes/test_MultipleExecutableDevice_node/DeviceManager.dcd.xml')
        devices = allocMgrHelpers.parseDomainDevices(self._domMgr)

        # First, try to list more devices than there are in the system, to make
        # sure no iterator is returned
        devlist, deviter = self.am.listDevices( self.am.LOCAL_DEVICES, 10)
        self.assertEqual(allocMgrHelpers.parseDeviceLocations(devlist), devices)
        self.assertEqual(deviter, None)

        # Next, start with fewer devices and fetch one-by-one via the iterator
        devlist, deviter = self.am.listDevices( self.am.LOCAL_DEVICES, 1)
        self.assertEqual(len(devlist), 1)
        self.assertNotEqual(deviter, None)
        try:
            # There has to be at least one more device
            item = deviter.next_one()
            self.assertNotEqual(item, None)
            devlist.append(item)

            # Fetch the remainder
            while item:
                item = deviter.next_one()
                if item:
                    devlist.append(item)
        finally:
            pass

        # Check the resulting list
        self.assertEqual(allocMgrHelpers.parseDeviceLocations(devlist), devices)

        # Then try fetching by a higher count
        devlist, deviter = self.am.listDevices( self.am.LOCAL_DEVICES, 1)
        self.assertEqual(len(devlist), 1)
        self.assertNotEqual(deviter, None)
        try:
            # Try to fetch 2 more, which ought to succeed in full
            items = deviter.next_n(2)
            self.assertEqual(len(items), 2)
            devlist.extend(items)

            # Try 2 more, which should return only 1
            items = deviter.next_n(2)
            self.assertEqual(len(items), 1)
            devlist.extend(items)

            # Finally, the next fetch should fail
            items = deviter.next_n(2)
            self.assertEqual( items, None )
        finally:
            pass

        # Check the resulting list
        self.assertEqual(allocMgrHelpers.parseDeviceLocations(devlist), devices)



    def test_redhawkutils_DeviceLists(self):
        """
        Tests the operation of the device list attributes (allDevices,
        authorizedDevices, localDevices).
        """
        # Keep adding nodes and testing the device lists to ensure that the
        # state is up-to-date
        devCount = 0
        for node in ('test_collocation_good_node', 'test_SADUsesDevice', 'test_MultipleExecutableDevice_node'):
            nb, devMgr = self.launchDeviceManager('/nodes/'+node+'/DeviceManager.dcd.xml')

            # Collect the complete set of device IDs, making sure new devices
            # are added every time through the loop
            devices = allocMgrHelpers.parseDomainDevices(self._domMgr)
            self.assert_(len(devices) > devCount)
            devCount = len(devices)

            # Make sure localDevices matches our known state
            localDevices = allocMgrHelpers.parseDeviceLocations( self.am.localDevices )
            self.assertEqual(devices, localDevices)

            # No policy is applied in default implementation, so authorized devices
            # should be the complete set of local devices
            authDevices = allocMgrHelpers.parseDeviceLocations( self.am.authorizedDevices )
            self.assertEqual(devices, authDevices)

            # Make sure allDevices matches our known state; since there are no
            # remote domains, it should be the same as localDevices
            allDevices = allocMgrHelpers.parseDeviceLocations( self.am.allDevices)
            self.assertEqual(devices, allDevices)


    def test_redhawkutils_BasicOperations(self):
        # Check that the domain manager back link is correct
        domMgr = self.am.getDomainMgr
        self.assert_(self._domMgr._is_equivalent(domMgr))

        # Check that the device list attributes work as expected (with no
        # devices), and do not throw exceptions
        self.assertEqual( self.am.localDevices, [])
        self.assertEqual( self.am.allDevices, [])
        self.assertEqual( self.am.authorizedDevices, [])

        # Check that the allocation list functions work as expected (with no
        # devices), and do not throw exceptions
        self.assertEqual( self.am.allocations([]), [])
        self.assertEqual( self.am.allocations([],True), [])
