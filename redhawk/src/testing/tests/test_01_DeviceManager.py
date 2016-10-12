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

import unittest, os, signal, time
import scatest
from omniORB import URI, any, CORBA
from ossie.cf import CF
import commands
import CosNaming
import tempfile

def getChildren(parentPid): 
    process_listing = commands.getoutput('ls /proc').split('\n') 
    children = [] 
    for entry in process_listing: 
        try: 
            filename = '/proc/'+entry+'/status' 
            fp = open(filename,'r') 
            stuff=fp.read() 
            fp.close() 
            rows = stuff.split('\n') 
            for row in rows: 
                if row[:4]=='PPid': 
                    PPid = int(row.split(':')[1][1:])
                    if PPid == parentPid: 
                        children.append(int(entry)) 
                        break 
        except: 
            continue 
    return children

def killChildProcesses(parentPid):
    childPids = getChildren(parentPid)
    for pid in childPids:
        killChildProcesses(pid)
        try:
            os.kill(pid, signal.SIGKILL)
        except OSError:
            pass
    for pid in childPids:
        try:
            os.waitpid(pid, 0)
        except OSError:
            pass

class DeviceManagerTest(scatest.CorbaTestCase):
    def setUp(self):
        nodebooter, self._domMgr = self.launchDomainManager(debug=9)
        self._domBooter = nodebooter

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)
        
        killChildProcesses(os.getpid())

    def test_CleanShutDown(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        devMgr.shutdown()

        self.assert_(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)


    def test_BadReleaseShutDown(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BadReleaseDevice_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 5)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        try:
            devMgr.shutdown()
        except:
            self.assertEqual(True, False)
        else:
            self.assertEqual(True, True)


        self.assert_(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)


        
    def test_deadDeviceShutdownNode(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_SelfTerminatingDevice_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
            
        devs = devMgr._get_registeredDevices()
        devs[0].start()
        time.sleep(0.5)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        devMgr.shutdown()

        self.assert_(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)
        
    def test_deadDeviceManager(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_SelfTerminatingDevice_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
            
        devs = devMgr._get_registeredDevices()
        pids = getChildren(os.getpid())
        devMgrPid = 0
        for entry in pids:
            tmp_pids = getChildren(entry)
            if len(tmp_pids)>0:
                devMgrPid = entry
        self.assertNotEqual(devMgrPid, 0)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        devMgr.shutdown()

        time.sleep(0.5)
        self.assertNotEqual(devmgr_nb.poll(), None, "Nodebooter did not die after shutdown")

    def test_CatatrophicUnregister(self):
        # Test that if a device manager dies unexpectedly and then re-registers there are no problems
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_SelfTerminatingDevice_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
            
        devs = devMgr._get_registeredDevices()
        pids = getChildren(devmgr_nb.pid)
        for devpid in pids:
            os.kill(devpid, signal.SIGKILL)
        os.kill(devmgr_nb.pid, signal.SIGKILL)

        self.waitTermination(devmgr_nb)

        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_SelfTerminatingDevice_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        devMgr.shutdown()

        self.assert_(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

    def test_IgnoreDevMgrDuplicate(self):
        # These two nodes use the same identifier, but have different names to distinguish them
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

        # Because the DeviceManager has the same identifier, we cannot use the launchDeviceManager
        # method; however, to get automatic cleanup in the event of a failure, we manually add the
        # nodebooter to the list of DeviceManager nodebooters.
        devmgr2_nb = scatest.spawnNodeBooter(dcdFile="/nodes/test_BasicTestDeviceSameDevMgrId_node/DeviceManager.dcd.xml", debug=9)
        self._deviceBooters.append(devmgr2_nb)
        time.sleep(2)

        # Verify that the second DeviceManager is no longer alive, 
        # This is REDHAWK specific, the spec would have let this go without 
        # giving the user clear warning that something was wrong
        self.assertNotEqual(devmgr2_nb.poll(), None)
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        devMgr = self._domMgr._get_deviceManagers()[0]
        self.assertEqual(devMgr._get_label(), "BasicTestDevice_node") # If the second one won, it would be DeviceManager2
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

    def test_IgnoreDeviceDuplicate(self):
        # These two nodes use the same identifier, but have different names to distinguish them
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_DuplicateTestDevice_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

    def test_DeviceInitializeFail(self):
        # These two nodes use the same identifier, but have different names to distinguish them
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/bad_init_device_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
    
    def test_ReRegDevMgrDuplicate(self):
        # These two nodes use the same identifier, but have different names to distinguish them
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

        self.terminateChild(devmgr_nb, signals=(signal.SIGKILL,))
        self.assertNotEqual(devmgr_nb.poll(), None)

        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDeviceSameDevMgrId_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

        # Verify that the second DeviceManager is no longer alive, 
        # This is REDHAWK specific, the spec would have let this go without 
        # giving the user clear warning that something was wrong
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        devMgr = self._domMgr._get_deviceManagers()[0]
        self.assertEqual(devMgr._get_label(), "BasicTestDeviceSameDevMgrId_node") # If the second one won, it would be DeviceManager2
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

    def test_IgnoreDevDuplicate(self):
        # These two nodes have devices with the same identifier, but have different names to distinguish them
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        dev = devMgr._get_registeredDevices()[0]
        self.assertEqual(dev._get_label(), "BasicTestDevice1")

        devmgr2_nb, devMgr2 = self.launchDeviceManager("/nodes/test_BasicTestDeviceSameDevId_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 2)
        self.assertEqual(len(devMgr2._get_registeredDevices()), 1)
        dev2 = devMgr2._get_registeredDevices()[0]
        self.assertEqual(dev2._get_label(), "BasicTestDeviceSameDevId")
        
        # TODO how do we test that this indeed worked?

    def test_ReRegDevDuplicate(self):
        # These two nodes have devices with the same identifier, but have different names to distinguish them
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        dev = devMgr._get_registeredDevices()[0]
        self.assertEqual(dev._get_label(), "BasicTestDevice1")

        self.terminateChildren(devmgr_nb, signals=(signal.SIGKILL,))
        self.assertNotEqual(devmgr_nb.poll(), None)

        devmgr2_nb, devMgr2 = self.launchDeviceManager("/nodes/test_BasicTestDeviceSameDevId_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 2)
        self.assertEqual(len(devMgr2._get_registeredDevices()), 1)
        dev2 = devMgr2._get_registeredDevices()[0]
        self.assertEqual(dev2._get_label(), "BasicTestDeviceSameDevId")

        # TODO how do we test that this indeed worked?

    def test_ComponentPlacementPropertyOverride(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        device = devMgr._get_registeredDevices()[0]
        propId = "DCE:6b298d70-6735-43f2-944d-06f754cd4eb9"
        prop = CF.DataType(id=propId, value=any.to_any(None))
        result = device.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, propId)
        # This should have been overrided by the dcd 
        self.assertEqual(result[0].value._v, "BasicTestDevice1_no_default_prop")

        propId = "DCE:456310b2-7d2f-40f5-bfef-9fdf4f3560ea" 
        prop = CF.DataType(id=propId, value=any.to_any(None))
        result = device.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, propId)
        # This should have been overrided by the dcd 
        self.assertEqual(result[0].value._v, "BasicTestDevice1_default_prop")

        structprop = device.query([CF.DataType(id="DCE:ffe634c9-096d-425b-86cc-df1cce50612f", value=any.to_any(None))])[0]
        struct_propseq = any.from_any(structprop.value)
        d = dict([(d["id"], d["value"]) for d in struct_propseq])
        self.assertEqual(d, {"item1": "the new value", "item2": 400, "item3": 1.414})


    def test_ComponentPlacementSimpleSeqPropertyOverride(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_DCDSimpleSeq_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        device = devMgr._get_registeredDevices()[0]
        propId = "testsimpleseq"
        prop = CF.DataType(id=propId, value=any.to_any(None))
        result = device.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(len(any.from_any(result[0].value)), 2)


    def test_ComponentPlacementNoPropOverride(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_DCDSimpleSeq_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

        device = devMgr._get_registeredDevices()[0]
        # this device implementation will put a non-nil value on the property (even if a nil has been passed in a configure call)
        propId = "nil_property"
        prop = CF.DataType(id=propId, value=any.to_any(None))
        result = device.query([prop])
        self.assertEqual(any.from_any(result[0].value), None)
        device.configure([prop])
        result = device.query([prop])
        self.assertEqual(any.from_any(result[0].value), -1.0)


    def test_ComponentPlacementNoPropertyOverride(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDeviceNoOverrides_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        device = devMgr._get_registeredDevices()[0]

        propId = "modified_default"
        prop = CF.DataType(id=propId, value=any.to_any(None))
        result = device.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, propId)
        # This should have been overrided by the dcd 
        self.assertEqual(result[0].value._v, "modified_default_value")


    def test_DevicePlacementPropertyOverride_cpp(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_ExecParamOverride_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        device = devMgr._get_registeredDevices()[0]
                
        propId = "DCE:68dc0d3b-deb2-4fae-b898-62273b74614b"                  
        prop = CF.DataType(id=propId, value=any.to_any(None))
        result = device.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, propId)
        # This should have been overrided by the dcd 
        self.assertEqual(result[0].value._v, "new execparam value")

        propId = "DCE:07350439-e917-45ef-b71f-e387a737fd9c"
        prop = CF.DataType(id=propId, value=any.to_any(None))
        result = device.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, propId)
        # This should have been overrided by the dcd 
        self.assertEqual(result[0].value._v, "new configured value")
        
        try:
            propId = "DCE:07350439-e917-45ef-b71f-a-bad-id"
            prop = CF.DataType(id=propId, value=any.to_any(None))
            result = self._app.query([prop])
            self.assertTrue(False)
        except:
            self.assertTrue(True)        

    def test_AllocateCapacities(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_AllocateBasicDevice_python_node/DeviceManager.dcd.xml", debug=2)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]
        self.assertNotEqual(device, None)
        
        mem_id = 'DCE:7aeaace8-350e-48da-8d77-f97c2e722e06'
        bog_id = 'DCE:bbdf708f-ce05-469f-8aed-f5c93e353e14'
        
        # first we don't have the properties set
        result = device.query([])
        for prop in result:
            self.assertEqual(prop.value._v, None)

        # Set the BasicTestDevice to have 2048 MB of memory capacity and 1024
        # BogoMips.  Attempting to allocate more than that it raises an error
        # at the BasicTestDevice level        
        props = [CF.DataType(id=mem_id, value=any.to_any(1024)),
                 CF.DataType(id=bog_id, value=any.to_any(512))
                ]
        
        # allocating half of maximum capacity
        device.allocateCapacity(props)                      
        result = device.query([])
        for prop in result:
            if prop.id == mem_id:
                self.assertEqual(prop.value._v, 1024)
            elif prop.id == bog_id:
                self.assertEqual(prop.value._v, 512)
            else:
                self.assertTrue(False)

        # allocating full capacity
        device.allocateCapacity(props)                      
        result = device.query([])
        for prop in result:
            if prop.id == mem_id:
                self.assertEqual(prop.value._v, 2048)
            elif prop.id == bog_id:
                self.assertEqual(prop.value._v, 1024)
            else:
                self.assertTrue(False)
        
        # exceeding capacity
        props = [CF.DataType(id=mem_id, value=any.to_any(1)),
                 CF.DataType(id=bog_id, value=any.to_any(0))
                ]        
        device.allocateCapacity(props)                      
        result = device.query([])
        for prop in result:
            if prop.id == mem_id:
                self.assertEqual(prop.value._v, 2048)
            elif prop.id == bog_id:
                self.assertEqual(prop.value._v, 1024)
            else:
                self.assertTrue(False)
                        
        # deallocating capacity
        props = [CF.DataType(id=mem_id, value=any.to_any(1024)),
                 CF.DataType(id=bog_id, value=any.to_any(512))
                ]        
        
        device.deallocateCapacity(props)                      
        result = device.query([])
        for prop in result:
            if prop.id == mem_id:
                self.assertEqual(prop.value._v, 1024)
            elif prop.id == bog_id:
                self.assertEqual(prop.value._v, 512)
            else:
                self.assertTrue(False)
        
                                      
        # allocating capacity
        props = [CF.DataType(id=mem_id, value=any.to_any(1025)),
                 CF.DataType(id=bog_id, value=any.to_any(512))
                ]        
        
        # exceeding capacity so no changes should occur
        device.allocateCapacity(props)                      
        result = device.query([])
        
        for prop in result:
            if prop.id == mem_id:
                self.assertEqual(prop.value._v, 1024)
            elif prop.id == bog_id:
                self.assertEqual(prop.value._v, 512)
            else:
                self.assertTrue(False)
         


    def test_ComponentPropertyOverride_cpp(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/SimpleDevMgr/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]
        self.assertNotEqual(device, None)
        

        # Now trying the component
        if self._domMgr:
            try:
                sadpath = "/waveforms/SimpleWaveform/SimpleWaveform.sad.xml"
                self._domMgr.installApplication(sadpath)
                appFact = self._domMgr._get_applicationFactories()[0]
                self._app = appFact.create(appFact._get_name(), [], [])
            except Exception, e:
                pass
                        
        propId = "DCE:c709f95e-6b05-439a-9db9-dba95e70888e"                  
        prop = CF.DataType(id=propId, value=any.to_any(None))
        result = self._app.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, propId)
        # This should have been overrided by the sad file
        self.assertEqual(result[0].value._v, "new execparam value")

        propId = "DCE:6ea8108d-76ea-4532-9255-01684ad68429"
        prop = CF.DataType(id=propId, value=any.to_any(None))
        result = self._app.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, propId)
        # This should have been overrided by the sad file
        self.assertEqual(result[0].value._v, "new configured value")
                
        try:
            propId = "DCE:07350439-e917-45ef-b71f-a-bad-id"
            prop = CF.DataType(id=propId, value=any.to_any(None))
            result = self._app.query([prop])
            self.assertTrue(False)
        except:
            self.assertTrue(True)


        # testing the Octect
        octetId = 'DCE:10add64d-1160-4de0-885b-46a991f52f1d'
        self._app.configure([CF.DataType(id=octetId, value=CORBA.Any(CORBA.TC_octet, 254))])
        prop = CF.DataType(id=octetId, value=any.to_any(None))
        result = self._app.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, octetId)
        self.assertEqual(result[0].value._v, 254)
        
                
    def test_MultipleComponentPlacements(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_MultipleExecutableDevice_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 4)
    
    def test_BadDeviceManagerName(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BadDeviceManagerName_node/DeviceManager.dcd.xml", debug=9)
        self.assertEqual(devMgr, None)
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)

    def test_AbsPaths(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_AbsPathNode/DeviceManager.dcd.xml", debug=9)

    def test_BasicService(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicService_node/DeviceManager.dcd.xml", debug=9)
        self.assertEqual(len(devMgr._get_registeredServices()), 1)
        svc = devMgr._get_registeredServices()[0]
        self.assertNotEqual(svc, None)
        self.assertEqual(svc.serviceName, "BasicService1")
        self.assertNotEqual(svc.serviceObject, None)
        obj = svc.serviceObject
        obj = obj._narrow(CF.PropertySet)
        self.assertNotEqual(obj, None)

        # Check the name service to ensure the service is properly bound
        svcName = URI.stringToName(scatest.getTestDomainName() + "/BasicService1")
        svcobj = self._root.resolve(svcName)._narrow(CF.PropertySet)
        self.assertNotEqual(svcobj, None)
        self.assert_(obj._is_equivalent(svcobj))

        # Check that all the parameters got set correctly
        props = obj.query([])
        d = dict([(p.id, any.from_any(p.value)) for p in props])
        self.assertEqual(d["SERVICE_NAME"], "BasicService1")
        self.assertEqual(d["DEVICE_MGR_IOR"], self._orb.object_to_string(devMgr))
        self.assertEqual(d["PARAM1"], "ABCD")
        self.assertEqual(d["PARAM2"], 42)
        self.assertEqual(d["PARAM3"], 3.1459)
        self.assertEqual(d["PARAM4"], False)
        self.assertEqual(d["PARAM5"], "Hello World")
        self.assertEqual(d.has_key("PARAM6"), False)

        # Check that we unregister correctly
        os.kill(devmgr_nb.pid, signal.SIGTERM)

        svcName = URI.stringToName(scatest.getTestDomainName() + "/BasicService1")
        time_begin = time.time()
        time_end = time.time()
        name_found = True
        while ((time_end - time_begin) < 2) and name_found:
            time_end = time.time()
            # Don't use assertRaises, so we can simplify things
            try:
                self._root.resolve(svcName)._narrow(CF.PropertySet)
                time.sleep(0.1)
            except CosNaming.NamingContext.NotFound:
                name_found = False
        if name_found:
            self.fail("Expected service to not exist in the naming service")

    def test_MultipleDevicesWithSameName(self):
        # Test that two different device managers can start devices with the same name

        # TODO We should check that these two actually have a device that has the same name
        # because, if someone changes the XML unwittingly then this test will pass
        # without really testing anything
        nb1, devMgr1 = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=9)
        nb2, devMgr2 = self.launchDeviceManager("/nodes/test_BasicTestDevice2_node/DeviceManager.dcd.xml", debug=9)

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 2)
        for devMgr in self._domMgr._get_deviceManagers():
            self.assertEqual(len(devMgr._get_registeredDevices()), 1) 

        # Although nothing in the spec specifies where a device is to be bound
        # ossie does /DomainName/DevicemgrName/DeviceName, so let's check that now
        # to ensure the behavior remains consistent
        dev1Name = URI.stringToName(scatest.getTestDomainName() + "/BasicTestDevice_node/BasicTestDevice1")
        dev2Name = URI.stringToName(scatest.getTestDomainName() + "/BasicTestDevice2_node/BasicTestDevice1")

        dev1 = self._root.resolve(dev1Name)._narrow(CF.Device)
        dev2 = self._root.resolve(dev2Name)._narrow(CF.Device)
        self.assertNotEqual(dev1, None)
        self.assertNotEqual(dev2, None)

    def test_connectToDeviceManager(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_PortUsesTestDevice_node/DeviceManager.dcd.xml", debug=9)

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        for device in devMgr._get_registeredDevices():
            DeviceManager_id = device.runTest(1,[])
            self.assertEqual(DeviceManager_id[0].value._v, 'DCE:5e51ac08-ada2-41fc-8996-0270bf05c236')

    def test_DeviceExecParamOverride(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)


        props = devMgr.query([])

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

        # Test that the DCD file execparam get's sent to Device
        device = devMgr._get_registeredDevices()[0]
        propId = "DCE:c03e148f-e9f9-4d70-aa00-6e23d33fa648"
        prop = CF.DataType(id=propId, value=any.to_any(None))
        try:
            result = device.query([prop])
        except:
            pass
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, propId)
        # This should have been overrided by the dcd 
        self.assertEqual(result[0].value._v, "path/to/some/config/file")

        # Test that the DCD file execparam that are readonly get sent the default value in the PRF file
        device = devMgr._get_registeredDevices()[0]
        propId = "DCE:6f5881b3-433e-434b-8204-d39c89ff4be2"
        prop = CF.DataType(id=propId, value=any.to_any(None))
        try:
            result = device.query([prop])
        except:
            pass
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, propId)
        # This should not have been overrided by the dcd 
        self.assertEqual(result[0].value._v, "DefaultValueGood")

        # Test whether the execparam "ImplementationSpecificProperty" was overloaded properly from the implementation-specific PRF file
        propId = "DCE:dc4289a8-bb98-435b-b914-305ffaa7594f"
        prop = CF.DataType(id=propId, value=any.to_any(None))
        try:
            result = device.query([prop])
        except:
            pass
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, propId)
        # This should have been overrided by the implementation-specific PRF file
        self.assertEqual(result[0].value._v, "NewLinuxx86Value")

        # Test whether the implementation-specific execparam property get's overloaded by the DCD
        propId = "DCE:716ea1c4-059a-4b18-8b66-74804bd8d435"
        prop = CF.DataType(id=propId, value=any.to_any(None))
        try:
            result = device.query([prop])
        except:
            pass
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, propId)
        # This should have been overrided by the implementation-specific PRF file
        self.assertEqual(result[0].value._v, "OverloadedTheImplementationSpecific")
        self.assertNotEqual(result[0].value._v, "NewLinuxx86Value2")
        
        # Test whether the implementation-specific property get's overloaded by the DCD
        #
        # if there is ever a way to override implemenation specific allocation properties
        # then use this check
        #
#             propId = "DCE:f6fb9770-cfd9-4e14-a337-2234f7f3317b"
#             prop = CF.DataType(id=propId, value=any.to_any(None))
#             result = [CF.DataType(id='', value=any.to_any(None)),]
#             try:
#                 result = device.query([prop])
#             except:
#                 pass
#             self.assertEqual(len(result), 1)
#             self.assertEqual(result[0].id, propId)
#             # This should have been overrided by the implementation-specific PRF file
#             self.assertEqual(result[0].value._v, "ImplementationSpecificAllocationProp")


    def test_ReleaseDevice(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/SimpleDevMgr/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]
        self.assertNotEqual(device, None)

        # Now trying the component
        if self._domMgr:
            try:
                sadpath = "/waveforms/SimpleWaveform/SimpleWaveform.sad.xml"
                #sadpath = "/waveforms/PyWave/PyWave.sad.xml"
                self._domMgr.installApplication(sadpath)
                appFact = self._domMgr._get_applicationFactories()[0]
                self._app = appFact.create(appFact._get_name(), [], [])
            except Exception, e:
                pass
        try:
            self._app.start()
        except Exception, e:
            pass
            

        dev_mgrs = len(self._domMgr._get_deviceManagers())
        self.assertEqual(dev_mgrs, 1)

        self._app.stop()
        self._app.releaseObject()

    def test_NoConfigureNilDeviceProperties(self):
        # This test is pretty limited and should be expanded to other complex property types
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDeviceNoOverrides_node/DeviceManager.dcd.xml", debug=9)
        self.assertNotEqual(devMgr, None)

        # Test that the DCD file execparam get's sent to Device
        device = devMgr._get_registeredDevices()[0]
        propId = "DCE:6b298d70-6735-43f2-944d-06f754cd4eb9"
        prop = CF.DataType(id=propId, value=any.to_any(None))
        try:
            result = device.query([prop])
        except:
            pass
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, propId)
        self.assertEqual(result[0].value._v, "not_none")

    def test_ExternalServices(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_MultipleService_node/DeviceManager.dcd.xml", debug=9)
        
        import ossie.utils.popen as _popen

        serviceName = "BasicService10"

        args = []
        args.append("sdr/dev/services/BasicService/BasicService.py")
        args.append("DEVICE_MGR_IOR")
        args.append(self._orb.object_to_string(devMgr))
        args.append("SERVICE_NAME")
        args.append(serviceName)
        args.append("LOGGING_CONFIG_URI")
        args.append("runtest.props")
        args.append("PARAM1")
        args.append("ABCD")
        args.append("PARAM2")
        args.append("42")
        args.append("PARAM3")
        args.append("3.1459")
        args.append("PARAM4")
        args.append("False")
        args.append("PARAM5")
        args.append("Hello World")
        exec_file = "sdr/dev/services/BasicService/BasicService.py"
        external_process = _popen.Popen(args, executable=exec_file, cwd=os.getcwd(), preexec_fn=os.setpgrp)

        serviceName2 = "BasicService11"
        args[4] = serviceName2
        exec_file = "sdr/dev/services/BasicService/BasicService.py"
        external_process2 = _popen.Popen(args, executable=exec_file, cwd=os.getcwd(), preexec_fn=os.setpgrp)

        time.sleep(1)

        names = ["BasicService1", "BasicService2", "BasicService3", "BasicService4", "BasicService5", serviceName, serviceName2]   
        
        # Makes sure external service registered
        for svc in devMgr._get_registeredServices():
            self.assertNotEqual(svc, None)
            self.assertEqual(svc.serviceName in names, True)
            print svc.serviceName
            names.remove(svc.serviceName)
            obj = svc.serviceObject
            obj = obj._narrow(CF.PropertySet)
            self.assertNotEqual(obj, None)
            
            # Check the name service to ensure the service is properly bound
            svcName = URI.stringToName(scatest.getTestDomainName() + "/" + svc.serviceName)
            svcobj = self._root.resolve(svcName)._narrow(CF.PropertySet)
            self.assertNotEqual(svcobj, None)
            self.assert_(obj._is_equivalent(svcobj))
            
            # Check that all the parameters got set correctly
            props = obj.query([])
            d = dict([(p.id, any.from_any(p.value)) for p in props])
            self.assertEqual(d["SERVICE_NAME"], svc.serviceName)
            self.assertEqual(d["DEVICE_MGR_IOR"], self._orb.object_to_string(devMgr))
            self.assertEqual(d["PARAM1"], "ABCD")
            self.assertEqual(d["PARAM2"], 42)
            self.assertEqual(d["PARAM3"], 3.1459)
            self.assertEqual(d["PARAM4"], False)
            self.assertEqual(d["PARAM5"], "Hello World")
            self.assertEqual(d.has_key("PARAM6"), False)

        # Make sure a component can communicate with the Service
        self._domMgr.installApplication("/waveforms/ServiceConnection/ServiceConnection.sad.xml")
        self.assertEquals(len(self._domMgr._get_applicationFactories()), 1)
        factory = self._domMgr._get_applicationFactories()[0]
        app = factory.create(factory._get_name(), [], [])
        self.assertEquals(len(self._domMgr._get_applications()), 1)    
              
        # Make sure an app that uses external services can launch
        app.start()
        port = app._get_registeredComponents()[0].componentObject.getPort("output") 
        app.releaseObject()

        # Kill the 2 external services
        os.kill(external_process.pid, signal.SIGINT)
        os.kill(external_process2.pid, signal.SIGINT)
        
        # Give time for kill signals to be caught
        time.sleep(1)
        os.kill(devmgr_nb.pid, signal.SIGINT)
                
        # Make sure services are no longer register with the domain manager
        names = ["BasicService1", "BasicService2", "BasicService3", "BasicService4", "BasicService5", serviceName, serviceName2]  
        svcNames = []
        for n in names:
            svcNames.append(URI.stringToName(scatest.getTestDomainName() + "/" + n))
        
        # Needs to allow time for unregistering
        self.assert_(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")
        
        # Don't use assertRaises, so we can simplify things
        for name in svcNames:
            try:
                #print "!!!!!!!!!!!" + name
                self._root.resolve(name)._narrow(CF.PropertySet)
            except CosNaming.NamingContext.NotFound:
                pass
            else:
                self.fail("Expected service to not exist in the naming service") 
        
        # Makes sure all children are cleaned
        self.assertEquals(len(getChildren(devmgr_nb.pid)), 0)
        
    def test_ServiceShutdown_DomMgr(self):
        num_services = 5
        num_devices = 1
        # This test makes sure that services are unregistered from the naming service upon shutdown of the DomainManager
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_MultipleService_node/DeviceManager.dcd.xml", debug=9)
        self.assertEqual(len(devMgr._get_registeredServices()), num_services)
        
        # Makes sure that the correct number of processes forked 
        self.assertEquals(len(getChildren(devmgr_nb.pid)), num_services + num_devices)
        
        svcName = URI.stringToName(scatest.getTestDomainName() + "/BasicService1")
        self._root.resolve(svcName)._narrow(CF.PropertySet)

        names = ["BasicService1", "BasicService2", "BasicService3", "BasicService4", "BasicService5"]

        for svc in devMgr._get_registeredServices():
            self.assertNotEqual(svc, None)
            self.assertEqual(svc.serviceName in names, True)
            names.remove(svc.serviceName)
            obj = svc.serviceObject
            obj = obj._narrow(CF.PropertySet)
            self.assertNotEqual(obj, None)
            
            # Check the name service to ensure the service is properly bound
            svcName = URI.stringToName(scatest.getTestDomainName() + "/" + svc.serviceName)
            svcobj = self._root.resolve(svcName)._narrow(CF.PropertySet)
            self.assertNotEqual(svcobj, None)
            self.assert_(obj._is_equivalent(svcobj))

            # Check that all the parameters got set correctly
            props = obj.query([])
            d = dict([(p.id, any.from_any(p.value)) for p in props])
            self.assertEqual(d["SERVICE_NAME"], svc.serviceName)
            self.assertEqual(d["DEVICE_MGR_IOR"], self._orb.object_to_string(devMgr))
            self.assertEqual(d["PARAM1"], "ABCD")
            self.assertEqual(d["PARAM2"], 42)
            self.assertEqual(d["PARAM3"], 3.1459)
            self.assertEqual(d["PARAM4"], False)
            self.assertEqual(d["PARAM5"], "Hello World")
            self.assertEqual(d.has_key("PARAM6"), False)

        # Check that we unregister correctly
        os.kill(self._domBooter.pid, signal.SIGINT)
        
        names = ["BasicService1", "BasicService2", "BasicService3", "BasicService4", "BasicService5"]
        svcNames = []
        for n in names:
            svcNames.append(URI.stringToName(scatest.getTestDomainName() + "/" + n))
        
        # Needs to allow time for unregistering
        self.assert_(self.waitTermination(self._domBooter), "Nodebooter did not die after shutdown")
        
        # Don't use assertRaises, so we can simplify things
        for name in svcNames:
            try:
                self._root.resolve(name)._narrow(CF.PropertySet)
            except CosNaming.NamingContext.NotFound:
                pass
            else:
                self.fail("Expected service to not exist in the naming service: " + str(name))
                
        # Makes sure that all children are dead
        self.assertEquals(len(getChildren(devmgr_nb.pid)), 0)
        
    def test_ServiceShutdown_DevMgr(self):
        num_services = 5
        num_devices = 1
        
        # This test makes sure that services are unregistered from the naming service upon shutdown of the DeviceManager
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_MultipleService_node/DeviceManager.dcd.xml", debug=9)
        self.assertEqual(len(devMgr._get_registeredServices()), num_services)
        
        # Makes sure that the correct number of processes forked
        self.assertEquals(len(getChildren(devmgr_nb.pid)), num_services + num_devices)
    
        svcName = URI.stringToName(scatest.getTestDomainName() + "/BasicService1")
        self._root.resolve(svcName)._narrow(CF.PropertySet)
        
        names = ["BasicService1", "BasicService2", "BasicService3", "BasicService4", "BasicService5"]

        for svc in devMgr._get_registeredServices():
            self.assertNotEqual(svc, None)
            self.assertEqual(svc.serviceName in names, True)
            names.remove(svc.serviceName)
            obj = svc.serviceObject
            obj = obj._narrow(CF.PropertySet)
            self.assertNotEqual(obj, None)

            # Check the name service to ensure the service is properly bound
            svcName = URI.stringToName(scatest.getTestDomainName() + "/" + svc.serviceName)
            svcobj = self._root.resolve(svcName)._narrow(CF.PropertySet)
            self.assertNotEqual(svcobj, None)
            self.assert_(obj._is_equivalent(svcobj))

            # Check that all the parameters got set correctly
            props = obj.query([])
            d = dict([(p.id, any.from_any(p.value)) for p in props])
            self.assertEqual(d["SERVICE_NAME"], svc.serviceName)
            self.assertEqual(d["DEVICE_MGR_IOR"], self._orb.object_to_string(devMgr))
            self.assertEqual(d["PARAM1"], "ABCD")
            self.assertEqual(d["PARAM2"], 42)
            self.assertEqual(d["PARAM3"], 3.1459)
            self.assertEqual(d["PARAM4"], False)
            self.assertEqual(d["PARAM5"], "Hello World")
            self.assertEqual(d.has_key("PARAM6"), False)

        # Check that we unregister correctly
        os.kill(devmgr_nb.pid, signal.SIGINT)

        names = ["BasicService1", "BasicService2", "BasicService3", "BasicService4", "BasicService5"]
        svcNames = []
        for n in names:
            svcNames.append(URI.stringToName(scatest.getTestDomainName() + "/" + n))
        
        # Needs to allow time for unregistering
        self.assert_(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

        # Don't use assertRaises, so we can simplify things
        for name in svcNames:
            try:
                self._root.resolve(name)._narrow(CF.PropertySet)
            except CosNaming.NamingContext.NotFound:
                pass
            else:
                self.fail("Expected service to not exist in the naming service: " + str(name))
            
        # Makes sure that all children are dead
        self.assertEquals(len(getChildren(devmgr_nb.pid)), 0)

