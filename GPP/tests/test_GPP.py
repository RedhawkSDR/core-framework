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

import unittest
import ossie.utils.testing
import os
import socket
import time
import commands
import sys
import threading
import Queue
from omniORB import any
from ossie.cf import CF, CF__POA
from ossie.cf import ExtendedEvent
from omniORB import CORBA
import CosEventChannelAdmin, CosEventChannelAdmin__POA

class ComponentTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all component implementations in test"""

    def promptToContinue(self):
        if sys.stdout.isatty():
            raw_input("Press enter to continue")
        else:
            pass # For non TTY just continue

    def promptUserInput(self, question, default):
        if sys.stdout.isatty():
            ans = raw_input("%s [%s]?" % (question, default))
            if ans == "":
                return default
            else:
                return ans
        else:
            return default
        
    def runGPP(self, execparam_overrides={}):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        execparams.update(execparam_overrides)
        self.launch(execparams)
        
        #######################################################################
        # Verify the basic state of the component
        self.assertNotEqual(self.comp_obj, None)
        self.assertEqual(self.comp_obj._non_existent(), False)
        self.assertEqual(self.comp_obj._is_a("IDL:CF/ExecutableDevice:1.0"), True)
        self.assertEqual(self.spd.get_id(), self.comp_obj._get_identifier())
        
    def testScaBasicBehavior(self):
        #######################################################################
        # Launch the device
        # Use values that could not possibly be true so we can ensure proper behavior
        self.runGPP({"DCE:4a23ad60-0b25-4121-a630-68803a498f75": "Windows", # os_name
                     "DCE:0f3a9a37-a342-43d8-9b7f-78dc6da74192": "3.1.1", # os_version
                     "DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b": "arm"}) # processor_name
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp_obj.configure(configureProps)
        
        #######################################################################
        # Validate that query returns all expected parameters
        # Query of '[]' should return the following set of properties
        expectedProps = []
        expectedProps.extend(self.getPropertySet(kinds=("configure", "execparam"), modes=("readwrite", "readonly"), includeNil=True))
        expectedProps.extend(self.getPropertySet(kinds=("allocate",), action="external", includeNil=True))
        props = self.comp_obj.query([])
        props = dict((x.id, any.from_any(x.value)) for x in props)
        # Query may return more than expected, but not less
        for expectedProp in expectedProps:
            self.assertEquals(props.has_key(expectedProp.id), True)
        
        qr = [CF.DataType(id="DCE:9190eb70-bd1e-4556-87ee-5a259dcfee39", value=any.to_any(None)), # hostName
              CF.DataType(id="DCE:7f36cdfb-f828-4e4f-b84f-446e17f1a85b", value=any.to_any(None)), # DeviceKind
              CF.DataType(id="DCE:4a23ad60-0b25-4121-a630-68803a498f75", value=any.to_any(None)), # os_name
              CF.DataType(id="DCE:0f3a9a37-a342-43d8-9b7f-78dc6da74192", value=any.to_any(None)), # os_version
              CF.DataType(id="DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b", value=any.to_any(None)), # processor_name
             ]
        qr = self.comp_obj.query(qr)
        self.assertEqual(qr[0].value.value(), socket.gethostname())
        self.assertEqual(qr[1].value.value(), "GPP")
        self.assertEqual(qr[2].value.value(), "Windows")
        self.assertEqual(qr[3].value.value(), "3.1.1")
        self.assertEqual(qr[4].value.value(), "arm")
        
        #######################################################################
        # Verify that all expected ports are available
        for port in self.scd.get_componentfeatures().get_ports().get_uses():
            port_obj = self.comp_obj.getPort(str(port.get_usesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a("IDL:CF/Port:1.0"),  True)
            
        for port in self.scd.get_componentfeatures().get_ports().get_provides():
            port_obj = self.comp_obj.getPort(str(port.get_providesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a(port.get_repid()),  True)
            
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp_obj.start()
        self.comp_obj.stop()
        
        #######################################################################
        # Simulate regular component shutdown
        self.comp_obj.releaseObject()
        
    def testMemoryCapacityAllocation(self):
        #######################################################################
        # Launch the device
        self.runGPP()
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp_obj.configure(configureProps)
        
        #######################################################################
        # Test memory capacity
        qr = [CF.DataType(id="DCE:329d9304-839e-4fec-a36f-989e3b4d311d", value=any.to_any(None)), # memTotal
              CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None)), # memCapacity
              CF.DataType(id="DCE:fc24e19d-eda9-4200-ae96-8ba2ed953128", value=any.to_any(None)), # memThreshold
              CF.DataType(id="DCE:6565bffd-cb09-4927-9385-2ecac68035c7", value=any.to_any(None)), # memFree
             ]
        qr = self.comp_obj.query(qr)
        memTotal = qr[0].value.value()
        memCapacity = qr[1].value.value()
        memThreshold = qr[2].value.value()
        memFree = qr[3].value.value()
        self.assertEqual(int(memCapacity), int(memTotal*(memThreshold/100.0)))
        self.assert_(memFree > 0)
        self.assert_(memFree < memTotal)
        
        allocateMemCapacity = memCapacity / 4
        for i in xrange(1,5):
            # All four should succeed
            allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(allocateMemCapacity))])
            self.assertEqual(allocated, True)
            qr = [CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))]
            qr = self.comp_obj.query(qr)
            self.assertEqual(qr[0].value.value(), memCapacity - (i*allocateMemCapacity))
            
        # The next allocation should fail
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(allocateMemCapacity))])
        self.assertEqual(allocated, False)
        
        # Deallocate and try again
        self.comp_obj.deallocateCapacity([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(allocateMemCapacity))])
        qr = [CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))]
        qr = self.comp_obj.query(qr)
        self.assertEqual(qr[0].value.value(), memCapacity - (3*allocateMemCapacity))
             
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(allocateMemCapacity))])
        self.assertEqual(allocated, True)
        r = [CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))]
        qr = self.comp_obj.query(qr)
        self.assertEqual(qr[0].value.value(), memCapacity - (4*allocateMemCapacity))
        
        # Deallocate all
        for i in xrange(3,-1,-1):
            # All four should succeed
            self.comp_obj.deallocateCapacity([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(allocateMemCapacity))])
            qr = [CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))]
            qr = self.comp_obj.query(qr)
            self.assertEqual(qr[0].value.value(), memCapacity - (i*allocateMemCapacity))
    
    def testLoadCapacityAllocation(self):
        #######################################################################
        # Launch the device
        self.runGPP()
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp_obj.configure(configureProps)
        
        #######################################################################
        # Test bogomips capacity
        qr = [CF.DataType(id="DCE:2df4cfe4-675c-41ec-9cc8-84dff2f468b3", value=any.to_any(None)), # processor_cores
              CF.DataType(id="DCE:3bf07b37-0c00-4e2a-8275-52bd4e391f07", value=any.to_any(None)), # loadCapacityPerCore
              CF.DataType(id="DCE:28b23bc8-e4c0-421b-9c52-415a24715209", value=any.to_any(None)), # loadTotal
              CF.DataType(id="DCE:6c000787-6fea-4765-8686-2e051e6c24b0", value=any.to_any(None)), # loadFree
              CF.DataType(id="DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056", value=any.to_any(None)), # loadCapacity
              CF.DataType(id="DCE:22a60339-b66e-4309-91ae-e9bfed6f0490", value=any.to_any(None)), # loadThreshold
              CF.DataType(id="DCE:9da85ebc-6503-48e7-af36-b77c7ad0c2b4", value=any.to_any(None)), # loadAverage
             ]
        qr = self.comp_obj.query(qr)
        processor_cores = qr[0].value.value()
        loadCapacityPerCore = qr[1].value.value()
        loadTotal = qr[2].value.value()
        loadFree = qr[3].value.value()
        loadCapacity = qr[4].value.value()
        loadThreshold = qr[5].value.value()
        loadAverage = dict([(x.id, x.value.value()) for x in qr[6].value.value()])
        
        self.assertAlmostEqual(loadCapacityPerCore, 1.0)
        self.assertAlmostEqual(loadTotal, processor_cores*loadCapacityPerCore)
        self.assertAlmostEqual(loadCapacity, loadTotal*(loadThreshold/100.0), places=0)
        self.assertAlmostEqual(loadFree, loadTotal*(loadThreshold/100.0), places=0)
        self.assertNotEqual(loadAverage['onemin'], 0.0)
        self.assertNotEqual(loadAverage['fivemin'], 0.0)
        self.assertNotEqual(loadAverage['fifteenmin'], 0.0)
        self.assertNotEqual(loadAverage['onemin'], None)
        self.assertNotEqual(loadAverage['fivemin'], None)
        self.assertNotEqual(loadAverage['fifteenmin'], None)
        
        #=======================================================================
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:2df4cfe4-675c-41ec-9cc8-84dff2f468b3", value=any.to_any(processor_cores))])
        self.assertEqual(allocated, True)
        self.comp_obj.deallocateCapacity([CF.DataType(id="DCE:2df4cfe4-675c-41ec-9cc8-84dff2f468b3", value=any.to_any(processor_cores))])
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:2df4cfe4-675c-41ec-9cc8-84dff2f468b3", value=any.to_any(processor_cores+1))])
        self.assertEqual(allocated, False)
         
               
        allocateLoadCapacity = loadCapacity / 4
        for i in xrange(1,5):
           # All four should succeed
           allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056", value=any.to_any(allocateLoadCapacity))])
           self.assertEqual(allocated, True)
           qr = [CF.DataType(id="DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056", value=any.to_any(None))]
           qr = self.comp_obj.query(qr)
           self.assertAlmostEqual(qr[0].value.value(), loadCapacity - (i*allocateLoadCapacity))
           qr = [CF.DataType(id="DCE:6c000787-6fea-4765-8686-2e051e6c24b0", value=any.to_any(None))]
           qr = self.comp_obj.query(qr)
           self.assertAlmostEqual(qr[0].value.value(), loadTotal*(loadThreshold/100.0) - (i*allocateLoadCapacity))
           
        # The next allocation should fail
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056", value=any.to_any(allocateLoadCapacity))])
        self.assertEqual(allocated, False)
        
        # Deallocate and try again
        self.comp_obj.deallocateCapacity([CF.DataType(id="DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056", value=any.to_any(allocateLoadCapacity))])
        qr = [CF.DataType(id="DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056", value=any.to_any(None))]
        qr = self.comp_obj.query(qr)
        self.assertAlmostEqual(qr[0].value.value(), loadCapacity - (3*allocateLoadCapacity))
             
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056", value=any.to_any(allocateLoadCapacity))])
        self.assertEqual(allocated, True)
        r = [CF.DataType(id="DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056", value=any.to_any(None))]
        qr = self.comp_obj.query(qr)
        self.assertAlmostEqual(qr[0].value.value(), loadCapacity - (4*allocateLoadCapacity))
         
        # Deallocate all
        for i in xrange(3,-1,-1):
           # All four should succeed
           self.comp_obj.deallocateCapacity([CF.DataType(id="DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056", value=any.to_any(allocateLoadCapacity))])
           qr = [CF.DataType(id="DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056", value=any.to_any(None))]
           qr = self.comp_obj.query(qr)
           self.assertAlmostEqual(qr[0].value.value(), loadCapacity - (i*allocateLoadCapacity))
           
        # The next allocation should fail is "saftey valve is turned on"
        #allocateLoadCapacity = (loadTotal - loadAverage['onemin']) + 1.0
        #allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056", value=any.to_any(allocateLoadCapacity))])
        #self.assertEqual(allocated, False)
        
        #=======================================================================
            
    def testDiskCapacityAllocation(self):
        #######################################################################
        # Launch the device
        self.runGPP()
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp_obj.configure(configureProps)
        
        # Test allocation against BogoMipsPerCPU and processor_cores
        #######################################################################
        # Test disk capacity
        qr = [CF.DataType(id="DCE:f5f78038-b7d4-4fcd-8294-344369c8a74f", value=any.to_any(None)), # fileSystems
              CF.DataType(id="DCE:6786dd11-1e30-4910-aaac-a92b8b82614c", value=any.to_any(None)), # diskCapacity
              CF.DataType(id="DCE:8c79aea8-479c-4b9b-98ab-efbb89305750", value=any.to_any(None)), # diskrateCapacity
             ]
        qr = self.comp_obj.query(qr)
        
        # Validate the fileSystems property
        self.assertNotEqual(len(qr[0].value.value()), 0)
        for fileSystemStruct in qr[0].value.value():
            fileSystem = dict([(x.id, x.value.value()) for x in any.from_any(fileSystemStruct, True)])
            # Don't actually validate the output against "df"..just check the structure
            self.assert_(fileSystem.has_key("device"))
            self.assertNotEqual(fileSystem["device"], None)
            self.assertNotEqual(fileSystem["device"], "")
            self.assert_(fileSystem.has_key("mount"))
            self.assertNotEqual(fileSystem["mount"], None)
            self.assertNotEqual(fileSystem["mount"], "")
            self.assert_(fileSystem.has_key("total"))
            self.assertNotEqual(fileSystem["total"], None)
            self.assertNotEqual(fileSystem["total"], 0)
            self.assert_(fileSystem.has_key("available"))
            self.assertNotEqual(fileSystem["available"], None)
            self.assertNotEqual(fileSystem["available"], 0)
            self.assert_(fileSystem.has_key("used"))
            self.assertNotEqual(fileSystem["used"], None)
        
        # TODO
        diskrateCapacity = qr[2].value.value()
        print "RRR", diskrateCapacity
        
        # Check that we can non-writable paths cause an allocation failure
        diskCapacityRequestStruct = [CF.DataType(id='diskCapacityPath', value=any.to_any("/")), CF.DataType(id='diskCapacity', value=any.to_any(100))]
        diskCapacityRequestStructSeq = [any.to_any(diskCapacityRequestStruct)]
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:6786dd11-1e30-4910-aaac-a92b8b82614c", value=any.to_any(diskCapacityRequestStructSeq))])
        self.assertEqual(allocated, False)
        
        # Check that non-existant paths fail allocation
        diskCapacityRequestStruct = [CF.DataType(id='diskCapacityPath', value=any.to_any("/tmp/certianly-not-there")), CF.DataType(id='diskCapacity', value=any.to_any(100))]
        diskCapacityRequestStructSeq = [any.to_any(diskCapacityRequestStruct)]
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:6786dd11-1e30-4910-aaac-a92b8b82614c", value=any.to_any(diskCapacityRequestStructSeq))])
        self.assertEqual(allocated, False)
        
        # Check that writable paths are allocatable (also check that $TMPDIR expansion works)
        diskCapacityRequestStruct = [CF.DataType(id='diskCapacityPath', value=any.to_any("${TMPDIR}")), CF.DataType(id='diskCapacity', value=any.to_any(100))]
        diskCapacityRequestStructSeq = [any.to_any(diskCapacityRequestStruct)]
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:6786dd11-1e30-4910-aaac-a92b8b82614c", value=any.to_any(diskCapacityRequestStructSeq))])
        self.assertEqual(allocated, True)
        
        # Check that not having enough space is valid (do that by just picking a massive value)
        diskCapacityRequestStruct = [CF.DataType(id='diskCapacityPath', value=any.to_any("${TMPDIR}")), CF.DataType(id='diskCapacity', value=any.to_any(int(100e6)))]
        diskCapacityRequestStructSeq = [any.to_any(diskCapacityRequestStruct)]
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:6786dd11-1e30-4910-aaac-a92b8b82614c", value=any.to_any(diskCapacityRequestStructSeq))])
        self.assertEqual(allocated, False)
        
        # Check that we can allocate from multiple directories at a time
        diskCapacityRequestStruct1 = [CF.DataType(id='diskCapacityPath', value=any.to_any("${TMPDIR}")), CF.DataType(id='diskCapacity', value=any.to_any(100))]
        diskCapacityRequestStruct2 = [CF.DataType(id='diskCapacityPath', value=any.to_any("/var/tmp")), CF.DataType(id='diskCapacity', value=any.to_any(100))]
        diskCapacityRequestStructSeq = [any.to_any(diskCapacityRequestStruct1), any.to_any(diskCapacityRequestStruct2)]
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:6786dd11-1e30-4910-aaac-a92b8b82614c", value=any.to_any(diskCapacityRequestStructSeq))])
        self.assertEqual(allocated, True)
        
        # Check that we can allocate from multiple directories at a time
        diskCapacityRequestStruct1 = [CF.DataType(id='diskCapacityPath', value=any.to_any("${TMPDIR}")), CF.DataType(id='diskCapacity', value=any.to_any(100))]
        diskCapacityRequestStruct2 = [CF.DataType(id='diskCapacityPath', value=any.to_any("/var/tmp")), CF.DataType(id='diskCapacity', value=any.to_any(int(100e6)))]
        diskCapacityRequestStructSeq = [any.to_any(diskCapacityRequestStruct1), any.to_any(diskCapacityRequestStruct2)]
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:6786dd11-1e30-4910-aaac-a92b8b82614c", value=any.to_any(diskCapacityRequestStructSeq))])
        self.assertEqual(allocated, False)
    
    def testMulticastNicCapacityAllocation(self):
        mcastIface = self.promptUserInput("Specify a NIC to be used for the multicast allocation", None)
        if mcastIface == None:
            return
        
        #######################################################################
        # Launch the device
        self.runGPP({"DCE:4e416acc-3144-47eb-9e38-97f1d24f7700": mcastIface,
                     "DCE:5a41c2d3-5b68-4530-b0c4-ae98c26c77ec": int(1000),
                     "DCE:442d5014-2284-4f46-86ae-ce17e0749da0": int(100)})
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp_obj.configure(configureProps)
        
        # Test allocation against BogoMipsPerCPU and processor_cores
        #######################################################################
        # Test disk capacity
        qr = [CF.DataType(id="DCE:4e416acc-3144-47eb-9e38-97f1d24f7700", value=any.to_any(None)), # mcastnicInterface
              CF.DataType(id="DCE:5a41c2d3-5b68-4530-b0c4-ae98c26c77ec", value=any.to_any(None)), # mcastnicIngressTotal
              CF.DataType(id="DCE:442d5014-2284-4f46-86ae-ce17e0749da0", value=any.to_any(None)), # mcastnicEgressTotal
              CF.DataType(id="DCE:506102d6-04a9-4532-9420-a323d818ddec", value=any.to_any(None)), # mcastnicIngressCapacity
              CF.DataType(id="DCE:eb08e43f-11c7-45a0-8750-edff439c8b24", value=any.to_any(None)), # mcastnicEgressCapacity)
              CF.DataType(id="DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1", value=any.to_any(None)), # mcastnicThreshold
              CF.DataType(id="DCE:65544aad-4c73-451f-93de-d4d76984025a", value=any.to_any(None)), # mcastnicVLANs
              CF.DataType(id="DCE:9190eb70-bd1e-4556-87ee-5a259dcfee39", value=any.to_any(None)), # hostName
             ]
        qr = self.comp_obj.query(qr)
        
        mcastnicInterface = qr[0].value.value()
        mcastnicIngressTotal = qr[1].value.value()
        mcastnicEgressTotal = qr[2].value.value()
        mcastnicIngressCapacity = qr[3].value.value()
        mcastnicEgressCapacity = qr[4].value.value()
        mcastnicThreshold = qr[5].value.value()
        mcastnicVLANs = qr[6].value.value()
        hostName = qr[7].value.value()
        
        self.assertEqual(mcastnicInterface, mcastIface)
        self.assertEqual(mcastnicIngressTotal, 1000)
        self.assertEqual(mcastnicEgressTotal, 100)
        self.assertEqual(hostName, socket.gethostname())
        
        self.assertNotEqual(len(mcastnicVLANs), 0)

        self.assertAlmostEqual(mcastnicIngressCapacity, int(mcastnicIngressTotal*float(mcastnicThreshold/100.0)), places=2)
        self.assertAlmostEqual(mcastnicEgressCapacity, int(mcastnicEgressTotal*float(mcastnicThreshold/100.0)), places=2)
        
        allocateIngressCapacity = mcastnicIngressCapacity / 4
        for i in xrange(1,5):
            # All four should succeed
            allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:506102d6-04a9-4532-9420-a323d818ddec", value=any.to_any(allocateIngressCapacity))])
            self.assertEqual(allocated, True)
            qr = [CF.DataType(id="DCE:506102d6-04a9-4532-9420-a323d818ddec", value=any.to_any(None))]
            qr = self.comp_obj.query(qr)
            self.assertEqual(qr[0].value.value(), mcastnicIngressCapacity - (i*allocateIngressCapacity), msg="Failed on %s allocation" % i)
            
        # The next allocation should fail
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:506102d6-04a9-4532-9420-a323d818ddec", value=any.to_any(allocateIngressCapacity))])
        self.assertEqual(allocated, False)
        
        # Deallocate and try again
        self.comp_obj.deallocateCapacity([CF.DataType(id="DCE:506102d6-04a9-4532-9420-a323d818ddec", value=any.to_any(allocateIngressCapacity))])
        qr = [CF.DataType(id="DCE:506102d6-04a9-4532-9420-a323d818ddec", value=any.to_any(None))]
        qr = self.comp_obj.query(qr)
        self.assertEqual(qr[0].value.value(), mcastnicIngressCapacity - (3*allocateIngressCapacity))
             
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:506102d6-04a9-4532-9420-a323d818ddec", value=any.to_any(allocateIngressCapacity))])
        self.assertEqual(allocated, True)
        r = [CF.DataType(id="DCE:506102d6-04a9-4532-9420-a323d818ddec", value=any.to_any(None))]
        qr = self.comp_obj.query(qr)
        self.assertEqual(qr[0].value.value(), mcastnicIngressCapacity - (4*allocateIngressCapacity))
        
        # Deallocate all
        for i in xrange(3,-1,-1):
            # All four should succeed
            self.comp_obj.deallocateCapacity([CF.DataType(id="DCE:506102d6-04a9-4532-9420-a323d818ddec", value=any.to_any(allocateIngressCapacity))])
            qr = [CF.DataType(id="DCE:506102d6-04a9-4532-9420-a323d818ddec", value=any.to_any(None))]
            qr = self.comp_obj.query(qr)
            self.assertEqual(qr[0].value.value(), mcastnicIngressCapacity - (i*allocateIngressCapacity))
            
        allocateEgressCapacity = mcastnicEgressCapacity / 4
        for i in xrange(1,5):
            # All four should succeed
            allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:eb08e43f-11c7-45a0-8750-edff439c8b24", value=any.to_any(allocateEgressCapacity))])
            self.assertEqual(allocated, True)
            qr = [CF.DataType(id="DCE:eb08e43f-11c7-45a0-8750-edff439c8b24", value=any.to_any(None))]
            qr = self.comp_obj.query(qr)
            self.assertEqual(qr[0].value.value(), mcastnicEgressCapacity - (i*allocateEgressCapacity))
            
        # The next allocation should fail
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:eb08e43f-11c7-45a0-8750-edff439c8b24", value=any.to_any(allocateEgressCapacity))])
        self.assertEqual(allocated, False)
        
        # Deallocate and try again
        self.comp_obj.deallocateCapacity([CF.DataType(id="DCE:eb08e43f-11c7-45a0-8750-edff439c8b24", value=any.to_any(allocateEgressCapacity))])
        qr = [CF.DataType(id="DCE:eb08e43f-11c7-45a0-8750-edff439c8b24", value=any.to_any(None))]
        qr = self.comp_obj.query(qr)
        self.assertEqual(qr[0].value.value(), mcastnicEgressCapacity - (3*allocateEgressCapacity))
             
        allocated = self.comp_obj.allocateCapacity([CF.DataType(id="DCE:eb08e43f-11c7-45a0-8750-edff439c8b24", value=any.to_any(allocateEgressCapacity))])
        self.assertEqual(allocated, True)
        r = [CF.DataType(id="DCE:eb08e43f-11c7-45a0-8750-edff439c8b24", value=any.to_any(None))]
        qr = self.comp_obj.query(qr)
        self.assertEqual(qr[0].value.value(), mcastnicEgressCapacity - (4*allocateEgressCapacity))
        
        # Deallocate all
        for i in xrange(3,-1,-1):
            # All four should succeed
            self.comp_obj.deallocateCapacity([CF.DataType(id="DCE:eb08e43f-11c7-45a0-8750-edff439c8b24", value=any.to_any(allocateEgressCapacity))])
            qr = [CF.DataType(id="DCE:eb08e43f-11c7-45a0-8750-edff439c8b24", value=any.to_any(None))]
            qr = self.comp_obj.query(qr)
            self.assertEqual(qr[0].value.value(), mcastnicEgressCapacity - (i*allocateEgressCapacity))    
    
    
    # Create a test file system
    class FileStub(CF__POA.File):
        def __init__(self):
            self.fobj = open("dat/component_stub.py")
        
        def sizeOf(self):
            return os.path.getsize("dat/component_stub.py")
        
        def read(self, bytes):
            return self.fobj.read(bytes)
        
        def close(self):
            return self.fobj.close()
            
    class FileSystemStub(CF__POA.FileSystem):
        def list(self, path):
            return [CF.FileSystem.FileInformationType(path[1:], CF.FileSystem.PLAIN, 100, [])]
            
        def open(self, path, readonly):
            file = ComponentTests.FileStub()
            return file._this()
            
    def testExecute(self):
        self.runGPP()
        self.comp_obj.initialize()
        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp_obj.configure(configureProps)
        
        fs_stub = ComponentTests.FileSystemStub()
        fs_stub_var = fs_stub._this()
        
        self.comp_obj.load(fs_stub_var, "/component_stub.py", CF.LoadableDevice.EXECUTABLE)
        self.assertEqual(os.path.isfile("component_stub.py"), True) # Technically this is an internal implementation detail that the file is loaded into the CWD of the device
        
        pid = self.comp_obj.execute("/component_stub.py", [], [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any("DCE:00000000-0000-0000-0000-000000000000:waveform_1")), 
                                                               CF.DataType(id="NAME_BINDING", value=any.to_any("MyComponent"))])
        self.assertNotEqual(pid, 0)
        
        try:
            os.kill(pid, 0)
        except OSError:
            self.fail("Process failed to execute")
        time.sleep(2)    
        self.comp_obj.terminate(pid)
        try:
            os.kill(pid, 0)
        except OSError:
            pass
        else:
            self.fail("Process failed to terminate")
            
        # Test both parameter and environment variable expansion
        componentOutputLog = "${TMPDIR}/comp_@COMPONENT_IDENTIFIER@.log"
        realLogPath = "/tmp/comp_DCE:00000000-0000-0000-0000-000000000000.log"
        
        self.comp_obj.configure([CF.DataType(id="DCE:c80f6c5a-e3ea-4f57-b0aa-46b7efac3176", value=any.to_any(componentOutputLog))])
        
        try:
            os.unlink(realLogPath)
        except OSError:
            pass
        self.assertEqual(os.path.isfile(realLogPath), False)
        
        pid = self.comp_obj.execute("/component_stub.py", [], [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any("DCE:00000000-0000-0000-0000-000000000000")), 
                                                               CF.DataType(id="NAME_BINDING", value=any.to_any("MyComponent"))])
        time.sleep(2)
        self.assertEqual(os.path.isfile(realLogPath), True)
        self.assertNotEqual(os.path.getsize(realLogPath), 0)
        self.comp_obj.terminate(pid)
        
        self.comp_obj.unload("/component_stub.py")
        self.assertEqual(os.path.isfile("component_stub.py"), False) # Technically this is an internal implementation detail that the file is loaded into the CWD of the device
        
    def testScreenExecute(self):
        self.runGPP({"DCE:218e612c-71a7-4a73-92b6-bf70959aec45": True})
        self.comp_obj.initialize()
        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp_obj.configure(configureProps)
        
        qr = self.comp_obj.query([CF.DataType(id="DCE:218e612c-71a7-4a73-92b6-bf70959aec45", value=any.to_any(None))])
        useScreen = qr[0].value.value()
        self.assertEqual(useScreen, True)
        
        fs_stub = ComponentTests.FileSystemStub()
        fs_stub_var = fs_stub._this()
        
        self.comp_obj.load(fs_stub_var, "/component_stub.py", CF.LoadableDevice.EXECUTABLE)
        self.assertEqual(os.path.isfile("component_stub.py"), True) # Technically this is an internal implementation detail that the file is loaded into the CWD of the device
        
        pid = self.comp_obj.execute("/component_stub.py", [], [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any("DCE:00000000-0000-0000-0000-000000000000:waveform_1")), 
                                                               CF.DataType(id="NAME_BINDING", value=any.to_any("MyComponent"))])
        self.assertNotEqual(pid, 0)
        
        try:
            os.kill(pid, 0)
        except OSError:
            self.fail("Process failed to execute")
        time.sleep(2)
           
        if os.environ.has_key("SCREENDIR"):
            screendir = os.path.expandvars("${SCREENDIR}")
        else:
            screendir = "/var/run/screen/S-%s" % os.environ['USER'] # RHEL specific
        screens = os.listdir(screendir)
        self.assertNotEqual(len(screens), 0)
        
        scrpid = None
        scrname = None
        for screen in screens:
            p, n = screen.split(".", 1)
            if n == "waveform_1.MyComponent":
                scrpid = int(p)
                scrname = n
                break
        self.assertEqual(scrpid, pid)
        self.assertEqual(scrname, "waveform_1.MyComponent")
        
        self.comp_obj.terminate(pid)
        try:
            os.kill(pid, 0)
        except OSError:
            pass
        else:
            self.fail("Process failed to terminate")
            
        screens = os.listdir(screendir)
        self.assertEqual(len(screens), 0)
        
        scrpid = None
        scrname = None
        for screen in screens:
            p, n = screen.split(".", 1)
            if n == "waveform_1.MyComponent":
                scrpid = int(p)
                scrname = n
                break
        self.assertEqual(scrpid, None)
        self.assertEqual(scrname, None)
        
    def testPropertyEvents(self):
        class Consumer_i(CosEventChannelAdmin__POA.ProxyPushConsumer):
            def __init__(self, parent, instance_id):
                self.supplier = None
                self.parent = parent
                self.instance_id = instance_id
                self.existence_lock = threading.Lock()
                
            def push(self, data):
                self.parent.actionQueue.put(data)
            
            def connect_push_supplier(self, supplier):
                self.supplier = supplier
                
            def disconnect_push_consumer(self):
                self.existence_lock.acquire()
                try:
                    self.supplier.disconnect_push_supplier()
                except:
                    pass
                self.existence_lock.release()
            
        class SupplierAdmin_i(CosEventChannelAdmin__POA.SupplierAdmin):
            def __init__(self, parent):
                self.parent = parent
                self.instance_counter = 0
        
            def obtain_push_consumer(self):
                self.instance_counter += 1
                self.parent.consumer_lock.acquire()
                self.parent.consumers[self.instance_counter] = Consumer_i(self.parent,self.instance_counter)
                objref = self.parent.consumers[self.instance_counter]._this()
                self.parent.consumer_lock.release()
                return objref
        
        class EventChannelStub(CosEventChannelAdmin__POA.EventChannel):
            def __init__(self):
                self.consumer_lock = threading.RLock()
                self.consumers = {}
                self.actionQueue = Queue.Queue()
                self.supplier_admin = SupplierAdmin_i(self)

            def for_suppliers(self):
                return self.supplier_admin._this()

        #######################################################################
        # Launch the device
        self.runGPP({"propertyEventRate": 5})
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp_obj.configure(configureProps)
        
        eventChannel = EventChannelStub()
        eventPort = self.comp_obj.getPort("propEvent")
        eventPort = eventPort._narrow(CF.Port)
        eventPort.connectPort(eventChannel._this(), "eventChannel")
        
        orb = CORBA.ORB_init()
        obj_poa = orb.resolve_initial_references("RootPOA")
        poaManager = obj_poa._get_the_POAManager()
        poaManager.activate()
        
        # Make sure the background status events are emitted
        time.sleep(12)
        
        self.assert_(eventChannel.actionQueue.qsize() > 0)
        
        event = eventChannel.actionQueue.get()
        event = any.from_any(event, keep_structs=True)
        event_dict = ossie.properties.props_to_dict(event.properties)
        
        # Check that all expected properties exist
        eventPropIds = set([prop.id for prop in self.getPropertySet(kinds=("event",), includeNil=True)])
        receivedEventIds = set(event_dict.keys())
        self.assert_(eventPropIds ^ receivedEventIds == set([]))
        
    # TODO Add additional tests here
    #
    # See:
    #   ossie.utils.testing.bulkio_helpers,
    #   ossie.utils.testing.bluefile_helpers
    # for modules that will assist with testing components with BULKIO ports
    
if __name__ == "__main__":
    ossie.utils.testing.main("../GPP.spd.xml") # By default tests all implementations
