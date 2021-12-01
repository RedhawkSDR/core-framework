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

import unittest, os, signal, time, platform
from _unitTestHelpers import scatest
from omniORB import URI, any, CORBA
from ossie.cf import CF
from ossie import properties
import subprocess
import CosNaming
import tempfile
import subprocess
import shutil

def getChildren(parentPid):
    process_listing = subprocess.getoutput('ls /proc').split('\n')
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

class DeviceManagerCacheTest(scatest.CorbaTestCase):
    def setUp(self):
        nodebooter, self._domMgr = self.launchDomainManager()
        self._domBooter = nodebooter

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)
        killChildProcesses(os.getpid())
        (status,output) = subprocess.getstatusoutput('chmod 755 '+os.getcwd()+'/sdr/cache/.BasicTestDevice_node')
        (status,output) = subprocess.getstatusoutput('rm -rf devmgr_runtest.props')

    def test_NoWriteCache(self):
        cachedir = os.getcwd()+'/sdr/cache/.BasicTestDevice_node'
        (status,output) = subprocess.getstatusoutput('mkdir -p '+cachedir)
        (status,output) = subprocess.getstatusoutput('chmod 000 '+cachedir)
        self.assertFalse(os.access(cachedir, os.R_OK|os.W_OK|os.X_OK), 'Current user can still access directory')
        devmgr_nb = None
        while not devmgr_nb:
            try:
                devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
            except Exception as e:
                print(e)
            if not devmgr_nb:
                time.sleep(2) # pause before trying again
        begin_time = time.time()
        while time.time()-begin_time < 5 and devmgr_nb.returncode == None:
            devmgr_nb.poll()
            time.sleep(0.1)
        self.assertEqual(255, devmgr_nb.returncode)

class DeviceManagerTest(scatest.CorbaTestCase):
    def setUp(self):
        nodebooter, self._domMgr = self.launchDomainManager()
        self._domBooter = nodebooter

    def tearDown(self):

        try:
            os.kill(self.device_pids[1],15)
        except:
            pass

        try:
            os.kill(self.service_pids[1],15)
        except:
            pass

        scatest.CorbaTestCase.tearDown(self)

        killChildProcesses(os.getpid())

    def test_CleanShutDown(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        self.assertTrue(self._domMgr._is_equivalent(devMgr._get_domMgr()))

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        devMgr.shutdown()

        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)


    def test_BadReleaseShutDown(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BadReleaseDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 5)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        try:
            devMgr.shutdown()
        except:
            self.assertEqual(True, False)
        else:
            self.assertEqual(True, True)


        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)



    def test_deadDeviceShutdownNode(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_SelfTerminatingDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        scatest.verifyDeviceLaunch(self, devMgr, 1)
        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml

        devs = devMgr._get_registeredDevices()
        devs[0].start()
        time.sleep(0.5)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        devMgr.shutdown()

        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)

    def test_deadDeviceManager(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_SelfTerminatingDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 1)

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

        time.sleep(1.0)
        self.assertNotEqual(devmgr_nb.poll(), None, "Nodebooter did not die after shutdown")

    def test_CatatrophicUnregister(self):
        # Test that if a device manager dies unexpectedly and then re-registers there are no problems
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_SelfTerminatingDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        devs = devMgr._get_registeredDevices()
        pids = getChildren(devmgr_nb.pid)
        for devpid in pids:
            os.kill(devpid, signal.SIGKILL)
        os.kill(devmgr_nb.pid, signal.SIGKILL)

        self.waitTermination(devmgr_nb)

        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_SelfTerminatingDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        devMgr.shutdown()

        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

    def test_IgnoreDevMgrDuplicate(self):
        # These two nodes use the same identifier, but have different names to distinguish them
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        # Because the DeviceManager has the same identifier, we cannot use the launchDeviceManager
        # method; however, to get automatic cleanup in the event of a failure, we manually add the
        # nodebooter to the list of DeviceManager nodebooters.
        devmgr2_nb = scatest.spawnNodeBooter(dcdFile="/nodes/test_BasicTestDeviceSameDevMgrId_node/DeviceManager.dcd.xml")
        self._deviceBooters.append(devmgr2_nb)
        time.sleep(2)

        # Verify that the second DeviceManager is no longer alive,
        # This is REDHAWK specific, the spec would have let this go without
        # giving the user clear warning that something was wrong
        self.assertNotEqual(devmgr2_nb.poll(), None)
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        devMgr = self._domMgr._get_deviceManagers()[0]
        self.assertEqual(devMgr._get_label(), "BasicTestDevice_node") # If the second one won, it would be DeviceManager2
        scatest.verifyDeviceLaunch(self, devMgr, 1)

    def test_IgnoreDeviceDuplicate(self):
        # These two nodes use the same identifier, but have different names to distinguish them
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_DuplicateTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        scatest.verifyDeviceLaunch(self, devMgr, 1)

    def test_DeviceBadOverload(self):
        # This device manager fails to launch because of a bad overloaded value
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/dev_props_bad_numbers_node/DeviceManager.dcd.xml")
        if devMgr:
            begin_time = time.time()
            while time.time()-begin_time < 5 and devMgr:
                poll = devmgr_nb.poll()
                if poll == None:
                    time.sleep(0.5)
                    continue
                else:
                    devMgr = None
                    break
        self.assertEqual(devMgr, None)

    def test_DeviceInitializeFail(self):
        # These two nodes use the same identifier, but have different names to distinguish them
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/bad_init_device_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # Device initialization failed, so remove the Device from the registered Device list
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        scatest.verifyDeviceLaunch(self, devMgr, 0)

    def test_ReRegDevMgrDuplicate(self):
        # These two nodes use the same identifier, but have different names to distinguish them
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        self.terminateChild(devmgr_nb, signals=(signal.SIGKILL,))
        self.assertNotEqual(devmgr_nb.poll(), None)

        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDeviceSameDevMgrId_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        # Verify that the second DeviceManager is no longer alive,
        # This is REDHAWK specific, the spec would have let this go without
        # giving the user clear warning that something was wrong
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        devMgr = self._domMgr._get_deviceManagers()[0]
        self.assertEqual(devMgr._get_label(), "BasicTestDeviceSameDevMgrId_node") # If the second one won, it would be DeviceManager2
        scatest.verifyDeviceLaunch(self, devMgr, 1)

    def test_IgnoreDevDuplicate(self):
        # These two nodes have devices with the same identifier, but have different names to distinguish them
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        scatest.verifyDeviceLaunch(self, devMgr, 1)
        dev = devMgr._get_registeredDevices()[0]
        self.assertEqual(dev._get_label(), "BasicTestDevice1")

        devmgr2_nb, devMgr2 = self.launchDeviceManager("/nodes/test_BasicTestDeviceSameDevId_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 2)
        scatest.verifyDeviceLaunch(self, devMgr2, 1)
        dev2 = devMgr2._get_registeredDevices()[0]
        self.assertEqual(dev2._get_label(), "BasicTestDeviceSameDevId")

        # TODO how do we test that this indeed worked?

    def test_ReRegDevDuplicate(self):
        # These two nodes have devices with the same identifier, but have different names to distinguish them
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        scatest.verifyDeviceLaunch(self, devMgr, 1)
        dev = devMgr._get_registeredDevices()[0]
        self.assertEqual(dev._get_label(), "BasicTestDevice1")

        self.terminateChildren(devmgr_nb, signals=(signal.SIGKILL,))
        self.assertNotEqual(devmgr_nb.poll(), None)

        devmgr2_nb, devMgr2 = self.launchDeviceManager("/nodes/test_BasicTestDeviceSameDevId_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 2)
        scatest.verifyDeviceLaunch(self, devMgr2, 1)
        dev2 = devMgr2._get_registeredDevices()[0]
        self.assertEqual(dev2._get_label(), "BasicTestDeviceSameDevId")

        # TODO how do we test that this indeed worked?

    def test_ComponentPlacementPropertyOverride(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 1)

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

    def test_cmdline_props(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/cmdline_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        
        status,output = subprocess.getstatusoutput('ps -ww -f | grep cmdline_dev')
        lines = output.split('\n')
        for line in lines:
          if 'IOR' in line:
            break
        
        items = line.split(' ')
        self.assertEqual(items.count('testprop'),1)
        props=[CF.DataType(id='testprop',value=any.to_any(None))]
        retprops = devMgr._get_registeredDevices()[0].query(props)
        self.assertEqual('abc',retprops[0].value._v)

    def test_nocmdline_props(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/nocmdline_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        
        status,output = subprocess.getstatusoutput('ps -ww -f | grep cmdline_dev')
        lines = output.split('\n')
        for line in lines:
          if 'IOR' in line:
            break
        
        items = line.split(' ')
        self.assertEqual(items.count('testprop'),0)
        props=[CF.DataType(id='testprop',value=any.to_any(None))]
        retprops = devMgr._get_registeredDevices()[0].query(props)
        self.assertEqual('abc',retprops[0].value._v)

    def test_deviceWithDepInSPD(self):
        nodebooter, devMgr = self.launchDeviceManager("/nodes/dev_comp_softpkg_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

    def test_ComponentPlacementSimpleSeqPropertyOverride(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_DCDSimpleSeq_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        device = devMgr._get_registeredDevices()[0]
        propId = "testsimpleseq"
        prop = CF.DataType(id=propId, value=any.to_any(None))
        result = device.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(len(any.from_any(result[0].value)), 2)


    def test_ConfigureNotCalledProperty(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/configure_call_property_dev_n/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        device = devMgr._get_registeredDevices()[0]
        # this device implementation will put a non-nil value on the property (even if a nil has been passed in a configure call)
        configure_called = device.query([CF.DataType(id='configure_called',value=any.to_any(None))])
        self.assertEqual(configure_called[0].value._v, False)

    def test_ComponentPlacementNoPropOverride(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_DCDSimpleSeq_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        device = devMgr._get_registeredDevices()[0]
        # this device implementation will put a non-nil value on the property (even if a nil has been passed in a configure call)
        propId = "nil_property"
        prop = CF.DataType(id=propId, value=any.to_any(None))
        result = device.query([prop])
        self.assertEqual(any.from_any(result[0].value), None)
        prop.value = any.to_any(2.0)
        device.configure([prop])
        result = device.query([prop])
        self.assertEqual(any.from_any(result[0].value), -1.0)


    def test_ComponentPlacementNoPropertyOverride(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDeviceNoOverrides_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 1)

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
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_ExecParamOverride_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 1)

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
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        scatest.verifyDeviceLaunch(self, devMgr, 1)
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

    def test_AllocateCapacities_python_callbacks(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/ticket_1502_node/DeviceManager.dcd.xml", debug=2)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        device = devMgr._get_registeredDevices()[0]
        self.assertNotEqual(device, None)

        props = [CF.DataType(id='simple_prop', value=any.to_any(123.0)),
                 CF.DataType(id='prop with spaces', value=any.to_any(456)),
                 CF.DataType(id='prop::with_colons', value=any.to_any(789))
                ]

        self.assertEqual(device.allocateCapacity(props),True)

    def test_ZeroLengthDev(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/zero_length_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        dev = devMgr._get_registeredDevices()[0]
        prop = dev.query([])
        for p in prop:
            if p.id == 'mystruct':
                val = p.value.value()
                for v in val:
                    if v.id == 'mystruct::mysimpleseq':
                        found = len(v.value.value()) == 0

        self.assertTrue(found)

    def test_ComponentPropertyOverride_cpp(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/SimpleDevMgr/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        scatest.verifyDeviceLaunch(self, devMgr, 1)
        device = devMgr._get_registeredDevices()[0]
        self.assertNotEqual(device, None)


        # Now trying the component
        if self._domMgr:
            try:
                sadpath = "/waveforms/SimpleWaveform/SimpleWaveform.sad.xml"
                self._domMgr.installApplication(sadpath)
                appFact = self._domMgr._get_applicationFactories()[0]
                self._app = appFact.create(appFact._get_name(), [], [])
            except:
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
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_MultipleExecutableDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        devices = []
        begin_time = time.time()
        while len(devices) != 4:
            time.sleep(0.1)
            if (time.time() - begin_time) > 15.0:
                break
            devices = devMgr._get_registeredDevices()

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 4)

    def test_BadDeviceManagerName(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BadDeviceManagerName_node/DeviceManager.dcd.xml")
        self.assertEqual(devMgr, None)
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)

    def test_AbsPaths(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_AbsPathNode/DeviceManager.dcd.xml")

    def test_MultipleDevicesWithSameName(self):
        # Test that two different device managers can start devices with the same name

        # TODO We should check that these two actually have a device that has the same name
        # because, if someone changes the XML unwittingly then this test will pass
        # without really testing anything
        nb1, devMgr1 = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        nb2, devMgr2 = self.launchDeviceManager("/nodes/test_BasicTestDevice2_node/DeviceManager.dcd.xml")

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 2)
        scatest.verifyDeviceLaunch(self, devMgr1, 1)
        scatest.verifyDeviceLaunch(self, devMgr2, 1)

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
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_PortUsesTestDevice_node/DeviceManager.dcd.xml")

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        for device in devMgr._get_registeredDevices():
            DeviceManager_id = device.runTest(1,[])
            self.assertEqual(DeviceManager_id[0].value._v, 'DCE:5e51ac08-ada2-41fc-8996-0270bf05c236')

    def test_DeviceExecParamOverride(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)


        props = devMgr.query([])

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 1)

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
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/SimpleDevMgr/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        scatest.verifyDeviceLaunch(self, devMgr, 1)
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
            except Exception as e:
                pass
        try:
            self._app.start()
        except Exception as e:
            pass


        dev_mgrs = len(self._domMgr._get_deviceManagers())
        self.assertEqual(dev_mgrs, 1)

        self._app.stop()
        self._app.releaseObject()

    def test_NoConfigureNilDeviceProperties(self):
        # This test is pretty limited and should be expanded to other complex property types
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDeviceNoOverrides_node/DeviceManager.dcd.xml")
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

    def test_PythonService(self):
        self._test_BasicService('test_BasicService_node', 'BasicService1')

    @scatest.requireJava
    def test_JavaService(self):
        self._test_BasicService('test_BasicService_java_node', 'BasicService_java_1')

    def _test_BasicService(self, node, expected_name):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/"+node+"/DeviceManager.dcd.xml", debug=1)
        begin_time = time.time()
        while len(devMgr._get_registeredServices()) != 1 and time.time()-begin_time < 10:
            time.sleep(0.5)
        self.assertEqual(len(devMgr._get_registeredServices()), 1)
        svc = devMgr._get_registeredServices()[0]
        self.assertNotEqual(svc, None)
        self.assertEqual(svc.serviceName, expected_name)
        self.assertNotEqual(svc.serviceObject, None)
        obj = svc.serviceObject
        obj = obj._narrow(CF.PropertySet)
        self.assertNotEqual(obj, None)

        # Check the name service to ensure the service is properly bound
        svcName = URI.stringToName(scatest.getTestDomainName() + "/" + expected_name)
        svcobj = self._root.resolve(svcName)._narrow(CF.PropertySet)
        self.assertNotEqual(svcobj, None)
        self.assertTrue(obj._is_equivalent(svcobj))

        # Check that all the parameters got set correctly
        props = obj.query([])
        d = dict([(p.id, any.from_any(p.value)) for p in props])
        self.assertEqual(d["SERVICE_NAME"], expected_name)
        self.assertEqual(d["DEVICE_MGR_IOR"], self._orb.object_to_string(devMgr))
        self.assertEqual(d["PARAM1"], "ABCD")
        self.assertEqual(d["PARAM2"], 42)
        self.assertAlmostEqual(d["PARAM3"], 3.1459)
        self.assertEqual(d["PARAM4"], False)
        self.assertEqual(d["PARAM5"], "Hello World")
        self.assertEqual("PARAM6" in d, False)

        # Check that we unregister correctly
        os.kill(devmgr_nb.pid, signal.SIGTERM)

        svcName = URI.stringToName(scatest.getTestDomainName() + "/" + expected_name)
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

    def setDelayTimeOut(self, src,  disableDelay=False, timeo=5000, dest=None ):
        import re
        if dest == None: dest = src.replace(".GOLD","")
        dest_f = open(dest,'w')
        lines = [line.rstrip() for line in open(src)]
        for l in lines:
            newline=l
            newline = re.sub(r'XXX', r''+str(disableDelay), newline )
            newline = re.sub(r'YYY', r''+str(timeo), newline )
            dest_f.write(newline+'\n')


    def test_blockStartupResponseDevice(self):
        from ossie.utils import redhawk
        d=redhawk.attach(self._domainManager._get_name())
        self.setDelayTimeOut("./sdr/dev/nodes/LongDeviceCalls/DeviceManager.dcd.xml.GOLD", False, 0)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/LongDeviceCalls/DeviceManager.dcd.xml")
        dev_devices= len(devMgr._get_registeredDevices())
        self.assertEqual(dev_devices, 0)

        # now clean up the test....
        devMgr.shutdown()
        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

    def test_longStartup1ResponseDevice(self):
        from ossie.utils import redhawk
        d=redhawk.attach(self._domainManager._get_name())
        self.setDelayTimeOut("./sdr/dev/nodes/LongDeviceCalls/DeviceManager.dcd.xml.GOLD", False, 5000)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/LongDeviceCalls/DeviceManager.dcd.xml")
        dev_devices= len(devMgr._get_registeredDevices())
        self.assertEqual(dev_devices, 1)

        # now clean up the test....
        devMgr.shutdown()
        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

    def test_longStartup2ResponseDevice(self):
        from ossie.utils import redhawk
        d=redhawk.attach(self._domainManager._get_name())
        self.setDelayTimeOut("./sdr/dev/nodes/LongDeviceCalls/DeviceManager.dcd.xml.GOLD", False, 12000)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/LongDeviceCalls/DeviceManager.dcd.xml")
        dev_devices= len(devMgr._get_registeredDevices())
        self.assertEqual(dev_devices, 0)

        # now clean up the test....
        devMgr.shutdown()
        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")


    def test_blockResponseDevice(self):
        from ossie.utils import redhawk
        d=redhawk.attach(self._domainManager._get_name())
        self.setDelayTimeOut("./sdr/dev/nodes/LongDeviceCalls/DeviceManager.dcd.xml.GOLD", True, 3000)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/LongDeviceCalls/DeviceManager.dcd.xml")
        dev_devices= len(devMgr._get_registeredDevices())
        self.assertEqual(dev_devices, 1)
        ld=d.devices[0]
        # have device block on corba calls used during shutdown
        ld.disable_delay=False
        ld.delay=0
        # now clean up the test....
        devMgr.shutdown()
        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

    def test_normalResponseDevice(self):
        from ossie.utils import redhawk
        d=redhawk.attach(self._domainManager._get_name())
        self.setDelayTimeOut("./sdr/dev/nodes/LongDeviceCalls/DeviceManager.dcd.xml.GOLD", True, 3000)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/LongDeviceCalls/DeviceManager.dcd.xml")
        dev_devices= len(devMgr._get_registeredDevices())
        self.assertEqual(dev_devices, 1)

        # now clean up the test....
        devMgr.shutdown()
        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

    def test_processGroupDevSvc(self):
        from ossie.utils import redhawk
        d=redhawk.attach(self._domainManager._get_name())
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/node_fork/DeviceManager.dcd.xml")
        self.device_pids = [0,0]
        for entry in d.devices[0].query([]):
            if entry.id == 'child_pid':
                self.device_pids[1] = entry.value._v
            if entry.id == 'self_pid':
                self.device_pids[0] = entry.value._v
        self.service_pids = [0,0]
        for entry in d.services[0].query([]):
            if entry.id == 'child_pid':
                self.service_pids[1] = entry.value._v
            if entry.id == 'self_pid':
                self.service_pids[0] = entry.value._v

        devMgr.shutdown()

        timeout = 5

        item_shutdown = False
        begin_time = time.time()
        while not item_shutdown and not (time.time()-begin_time > timeout):
            try:
                os.kill(self.device_pids[0],0)
            except OSError:
                item_shutdown = True
        self.assertTrue(item_shutdown)

        item_shutdown = False
        begin_time = time.time()
        while not item_shutdown and not (time.time()-begin_time > timeout):
            try:
                os.kill(self.device_pids[1],0)
            except OSError:
                item_shutdown = True
        self.assertTrue(item_shutdown)

        item_shutdown = False
        begin_time = time.time()
        while not item_shutdown and not (time.time()-begin_time > timeout):
            try:
                os.kill(self.service_pids[0],0)
            except OSError:
                item_shutdown = True
        self.assertTrue(item_shutdown)

        item_shutdown = False
        begin_time = time.time()
        while not item_shutdown and not (time.time()-begin_time > timeout):
            try:
                os.kill(self.service_pids[1],0)
            except OSError:
                item_shutdown = True
        self.assertTrue(item_shutdown)

    def test_ExternalServices(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_MultipleService_node/DeviceManager.dcd.xml")
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

            names.remove(svc.serviceName)
            obj = svc.serviceObject
            obj = obj._narrow(CF.PropertySet)
            self.assertNotEqual(obj, None)

            # Check the name service to ensure the service is properly bound
            svcName = URI.stringToName(scatest.getTestDomainName() + "/" + svc.serviceName)
            svcobj = self._root.resolve(svcName)._narrow(CF.PropertySet)
            self.assertNotEqual(svcobj, None)
            self.assertTrue(obj._is_equivalent(svcobj))

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
            self.assertEqual("PARAM6" in d, False)

        # Make sure a component can communicate with the Service
        self._domMgr.installApplication("/waveforms/ServiceConnection/ServiceConnection.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        factory = self._domMgr._get_applicationFactories()[0]
        app = factory.create(factory._get_name(), [], [])
        self.assertEqual(len(self._domMgr._get_applications()), 1)

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
        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

        # Don't use assertRaises, so we can simplify things
        for name in svcNames:
            try:
                self._root.resolve(name)._narrow(CF.PropertySet)
            except CosNaming.NamingContext.NotFound:
                pass
            else:
                self.fail("Expected service to not exist in the naming service")

        # Makes sure all children are cleaned
        self.assertEqual(len(getChildren(devmgr_nb.pid)), 0)

    def test_RogueService(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        import ossie.utils.popen as _popen
        from ossie.utils import redhawk
        rhdom= redhawk.attach(scatest.getTestDomainName())

        serviceName = "fake_1"
        args = []
        args.append("sdr/dev/services/fake/python/fake.py")
        args.append("DEVICE_MGR_IOR")
        args.append(self._orb.object_to_string(devMgr))
        args.append("SERVICE_NAME")
        args.append(serviceName)
        exec_file = "sdr/dev/services/fake/python/fake.py"
        external_process = _popen.Popen(args, executable=exec_file, cwd=os.getcwd(), preexec_fn=os.setpgrp)

        time.sleep(2)

        names=[serviceName]
        for svc in devMgr._get_registeredServices():
            self.assertNotEqual(svc, None)
            self.assertEqual(svc.serviceName in names, True)

        for svc in rhdom.services:
            self.assertNotEqual(svc, None)
            self.assertEqual(svc._instanceName in names, True)

        # Kill the external services
        os.kill(external_process.pid, signal.SIGINT)

        time.sleep(1)

        # check rogue service is removed
        self.assertEqual(len(devMgr._get_registeredServices()), 0)
        self.assertEqual(len(rhdom.services), 0)


    def test_ServiceShutdown_DomMgr(self):
        num_services = 5
        num_devices = 1
        # This test makes sure that services are unregistered from the naming service upon shutdown of the DomainManager
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_MultipleService_node/DeviceManager.dcd.xml")
        timeout = 5
        begin_time = time.time()
        done = False
        while not done:
            current_time = time.time()
            if (current_time - begin_time) > timeout:
                break
            if len(devMgr._get_registeredServices()) == num_services:
                break
        self.assertEqual(len(devMgr._get_registeredServices()), num_services)

        # Makes sure that the correct number of processes forked
        self.assertEqual(len(getChildren(devmgr_nb.pid)), num_services + num_devices)

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
            self.assertTrue(obj._is_equivalent(svcobj))

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
            self.assertEqual("PARAM6" in d, False)

        # Check that we unregister correctly
        os.kill(self._domBooter.pid, signal.SIGINT)

        names = ["BasicService1", "BasicService2", "BasicService3", "BasicService4", "BasicService5"]
        svcNames = []
        for n in names:
            svcNames.append(URI.stringToName(scatest.getTestDomainName() + "/" + n))

        # Needs to allow time for unregistering
        self.assertTrue(self.waitTermination(self._domBooter), "Nodebooter did not die after shutdown")

        # Don't use assertRaises, so we can simplify things
        for name in svcNames:
            try:
                self._root.resolve(name)._narrow(CF.PropertySet)
            except CosNaming.NamingContext.NotFound:
                pass
            else:
                self.fail("Expected service to not exist in the naming service: " + str(name))

        # Makes sure that all children are dead; on slower systems, the service
        # processes may take a while to exit, so wait for the DeviceManager to
        # exit first
        self.assertTrue(self.waitTermination(devmgr_nb), "DeviceManager did not exit after shutdown")
        self.assertEqual(len(getChildren(devmgr_nb.pid)), 0)

    def test_ServiceShutdown_DevMgr(self):
        num_services = 5
        num_devices = 1

        # This test makes sure that services are unregistered from the naming service upon shutdown of the DeviceManager
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_MultipleService_node/DeviceManager.dcd.xml")
        begin_time = time.time()
        while len(devMgr._get_registeredServices()) != num_services and time.time()-begin_time < 10:
            time.sleep(0.5)
        self.assertEqual(len(devMgr._get_registeredServices()), num_services)

        # Makes sure that the correct number of processes forked
        self.assertEqual(len(getChildren(devmgr_nb.pid)), num_services + num_devices)

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
            self.assertTrue(obj._is_equivalent(svcobj))

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
            self.assertEqual("PARAM6" in d, False)

        # Check that we unregister correctly
        os.kill(devmgr_nb.pid, signal.SIGINT)

        names = ["BasicService1", "BasicService2", "BasicService3", "BasicService4", "BasicService5"]
        svcNames = []
        for n in names:
            svcNames.append(URI.stringToName(scatest.getTestDomainName() + "/" + n))

        # Needs to allow time for unregistering
        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

        # Don't use assertRaises, so we can simplify things
        for name in svcNames:
            try:
                self._root.resolve(name)._narrow(CF.PropertySet)
            except CosNaming.NamingContext.NotFound:
                pass
            else:
                self.fail("Expected service to not exist in the naming service: " + str(name))

        # Makes sure that all children are dead
        self.assertEqual(len(getChildren(devmgr_nb.pid)), 0)

    def test_ServicePort_DevMgrShutdown(self):
        num_services = 4
        num_devices = 2

        # This test makes sure that services are unregistered from the naming service upon shutdown of the DeviceManager
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/svc_port_node/DeviceManager.dcd.xml")
        begin_time = time.time()
        while len(devMgr._get_registeredServices()) != num_services and time.time()-begin_time < 10:
            time.sleep(0.5)
        self.assertEqual(len(devMgr._get_registeredServices()), num_services)

        # Makes sure that the correct number of processes forked
        self.assertEqual(len(getChildren(devmgr_nb.pid)), num_devices)

        # Check that we unregister correctly
        os.kill(devmgr_nb.pid, signal.SIGTERM)

        # Needs to allow time for unregistering
        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

        # Makes sure that all children are dead
        self.assertEqual(len(getChildren(devmgr_nb.pid)), 0)

    def _test_Valgrind(self, valgrind):
        # Clear the device cache to prevent false positives
        deviceCacheDir = os.path.join(scatest.getSdrCache(), ".ExecutableDevice_node", "ExecutableDevice1")
        shutil.rmtree(deviceCacheDir, ignore_errors=True)

        os.environ['VALGRIND'] = valgrind
        try:
            # Checking that the node and device launch as expected
            nb, devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        finally:
            del os.environ['VALGRIND']

        self.assertFalse(devMgr is None)
        scatest.verifyDeviceLaunch(self, devMgr, 1, msg='device failed to launch with valgrind')
        children = getChildren(nb.pid)
        self.assertEqual(len(children), 1)
        devMgr.shutdown()

        # Check that a valgrind logfile exists
        logfile = os.path.join(deviceCacheDir, 'valgrind.%s.log' % children[0])
        self.assertTrue(os.path.exists(logfile))

    def test_ValgrindOption(self):
        # Make sure that valgrind exists and is in the path
        valgrind = scatest.which('valgrind')
        if not valgrind:
            raise RuntimeError('Valgrind is not installed')

        # Let the device manager find valgrind on the path
        self._test_Valgrind('')

        # Set an explicit path to valgrind, using a symbolic link to a non-path
        # location as an additional check
        altpath = os.path.join(scatest.getSdrPath(), 'valgrind')
        os.symlink(valgrind, altpath)

        # patch for ubuntu valgrind script
        ub_patch=False
        try:
           if 'UBUNTU' in platform.linux_distribution()[0].upper():
               ub_patch=True
               valgrind_bin = scatest.which('valgrind.bin')
               os.symlink(valgrind_bin, altpath+'.bin')
        except:
             pass

        try:
            self._test_Valgrind(altpath)
        finally:
            os.unlink(altpath)
            if ub_patch:
                os.unlink(altpath+'.bin')

    def test_Service_Startup(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_service_startup_node/DeviceManager.dcd.xml")
        from ossie.utils import redhawk
        d=redhawk.attach(self._domainManager._get_name())

        svc=None
        svc_pre=None
        for s in d.services:
            if s._id == 'S2_1':
                svc = s
            if s._id == 'S2_pre_1':
                svc_pre = s
        self.assertNotEqual(svc, None)
        self.assertNotEqual(svc_pre, None)

        # get p1 from service
        res=svc.query([CF.DataType(id='p1', value=any.to_any(None))])
        p1=properties.props_to_dict(res)
        self.assertEqual(p1['p1'],'p1 set by DCD file')

        # get p2 from service
        res=svc.query([CF.DataType(id='p2', value=any.to_any(None))])
        p1=properties.props_to_dict(res)
        self.assertEqual(p1['p2'],123456)


        # get p1 from service
        res=svc_pre.query([CF.DataType(id='p1', value=any.to_any(None))])
        p1=properties.props_to_dict(res)
        self.assertEqual(p1['p1'],'pre p1 set by DCD file')

        # get p2 from service
        res=svc_pre.query([CF.DataType(id='p2', value=any.to_any(None))])
        p1=properties.props_to_dict(res)
        self.assertEqual(p1['p2'],654321)

        self.assertRaises(CF.PropertySet.InvalidConfiguration, svc.configure, [CF.DataType(id='fake', value=any.to_any(None))] )

        self.assertRaises(CF.PropertySet.InvalidConfiguration, svc_pre.configure, [CF.DataType(id='fake', value=any.to_any(None))] )

    def _test_DeviceExecParamReadonly_(self, dcdfile, true_or_false ):
        devmgr_nb, devMgr = self.launchDeviceManager(dcdfile)
        self.assertNotEqual(devMgr, None)

        from ossie.utils import redhawk
        d=redhawk.attach(scatest.getTestDomainName())

        dev=d.devices[0]
        self.assertNotEqual(dev, None)

        # check exec params are changed
        val = dev.config_prop_normal.queryValue()
        self.assertEqual(val,"prop_ok_normal")

        val = dev.config_prop_read_only.queryValue()
        self.assertEqual(val,"good")

        val = dev.exec_read_only.queryValue()
        self.assertEqual(val,"cmd_ok_read_only")

        val = dev.exec_read_only_nochange.queryValue()
        self.assertEqual(val,"nochange")

        val = dev.exec_read_only_empty.queryValue()
        self.assertEqual(val,None)

        val = dev.exec_read_only_number.queryValue()
        self.assertEqual(val,12345)

        val = dev.exec_read_only_empty_number.queryValue()
        self.assertEqual(val,0)

        val = dev.exec_read_only_bool.queryValue()
        self.assertEqual(val,true_or_false)

        val = dev.exec_cmd.queryValue()
        self.assertEqual(val,"cmd_ok")

        val = dev.exec_cmd_empty.queryValue()
        self.assertEqual(val,None)


    def test_DeviceExecParamReadonly_0(self):
        self._test_DeviceExecParamReadonly_("/nodes/node_exec_params/node_exec_params/DeviceManager.dcd.xml", False )

    def test_DeviceExecParamReadonly_1(self):
        self._test_DeviceExecParamReadonly_("/nodes/node_exec_params/node_exec_params/DeviceManager.true-true.dcd.xml", True )

    def test_DeviceExecParamReadonly_1_1(self):
        self._test_DeviceExecParamReadonly_("/nodes/node_exec_params/node_exec_params/DeviceManager.true-tRuE.dcd.xml", True )

    def test_DeviceExecParamReadonly_2(self):
        self._test_DeviceExecParamReadonly_("/nodes/node_exec_params/node_exec_params/DeviceManager.true-1.dcd.xml", True )

    def test_DeviceExecParamReadonly_3(self):
        self._test_DeviceExecParamReadonly_("/nodes/node_exec_params/node_exec_params/DeviceManager.false-0.dcd.xml", False )

    def test_DeviceExecParamReadonly_4(self):
        self._test_DeviceExecParamReadonly_("/nodes/node_exec_params/node_exec_params/DeviceManager.false-false.dcd.xml", False )

    def test_DeviceExecParamReadonly_4_1(self):
        self._test_DeviceExecParamReadonly_("/nodes/node_exec_params/node_exec_params/DeviceManager.false-fAlSe.dcd.xml", False )


    def test_DeviceExecParamReadonly_NoEmpty(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/node_exec_params/node_exec_params_no_empty/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        from ossie.utils import redhawk
        d=redhawk.attach(scatest.getTestDomainName())

        dev=d.devices[0]
        self.assertNotEqual(dev, None)

        # check exec params are changed
        val = dev.config_prop_normal.queryValue()
        self.assertEqual(val,"prop_ok_normal")

        val = dev.config_prop_read_only.queryValue()
        self.assertEqual(val,"good")

        val = dev.exec_read_only.queryValue()
        self.assertEqual(val,"cmd_ok_read_only")

        val = dev.exec_read_only_nochange.queryValue()
        self.assertEqual(val,"nochange")

        val = dev.exec_read_only_empty.queryValue()
        self.assertEqual(val,"cmd_ok_read_only_empty")

        val = dev.exec_read_only_number.queryValue()
        self.assertEqual(val,54321)

        val = dev.exec_read_only_empty_number.queryValue()
        self.assertEqual(val,654321)

        val = dev.exec_cmd.queryValue()
        self.assertEqual(val,"cmd_ok")

        val = dev.exec_cmd_empty.queryValue()
        self.assertEqual(val,"cmd_ok_empty")



    def _find_exec_param( self,lines, pname, match_str ):
        p=None
        for l in lines:
            if l.find(pname) != -1 :
                if l.find(match_str) != -1:
                    p=l
                    break

        return p

    def _get_match_exec(self):
        return { "exec_cmd" : "exec_cmd cmd_ok",
                     "exec_cmd_empty" : "exec_cmd_empty cmd_ok_empty",
                     "exec_read_only" : "exec_read_only cmd_ok_read_only",
                     "exec_read_only_nochange" :  "exec_read_only_nochange nochange",
                     "exec_read_only_empty" :  "exec_read_only_empty cmd_ok_empty",
                     "exec_read_only_number" : "exec_read_only_number 54321",
                     "exec_read_only_empty_number" : "exec_read_only_empty_number 654321",
                     "exec_read_only_bool" : "exec_read_only_bool FALSE",
                     "exec_read_only_bool_missing" : None };

    def _test_ServiceExecParamReadonly_(self, dcd_file, match_exec):
        devmgr_nb, devMgr = self.launchDeviceManager(dcd_file)
        self.assertNotEqual(devMgr, None)

        from ossie.utils import redhawk
        d=redhawk.attach(scatest.getTestDomainName())

        svc=d.services[0]
        self.assertNotEqual(svc, None)
        
        lines = [ line.rstrip() for line in os.popen('ps -ww -ef | grep "services/py_svc_exec_params" | grep  -v "grep"')]
        
        k="exec_cmd"
        if  match_exec[k]:
            p=self._find_exec_param(lines, "py_svc_exec_params",  match_exec[k] )
            self.assertNotEqual(p, None)
        else:
            p=self._find_exec_param(lines, "py_svc_exec_params",  k+" " )
            self.assertEqual(p, None)

        k="exec_cmd_empty"
        if  match_exec[k]:
            p=self._find_exec_param(lines, "py_svc_exec_params", match_exec[k])
            self.assertNotEqual(p, None)
        else:
            p=self._find_exec_param(lines, "py_svc_exec_params",  k+" " )
            self.assertEqual(p, None)

        k="exec_read_only"
        if  match_exec[k]:
            p=self._find_exec_param(lines, "py_svc_exec_params",  match_exec[k])
            self.assertNotEqual(p, None)
        else:
            p=self._find_exec_param(lines, "py_svc_exec_params",  k+" " )
            self.assertEqual(p, None)

        k="exec_read_only_empty"
        if  match_exec[k]:
            p=self._find_exec_param(lines, "py_svc_exec_params",  match_exec[k])
            self.assertNotEqual(p, None)
        else:
            p=self._find_exec_param(lines, "py_svc_exec_params",  k+" " )
            self.assertEqual(p, None)

        k="exec_read_only_nochange"
        if  match_exec[k]:
            p=self._find_exec_param(lines, "py_svc_exec_params",  match_exec[k])
            self.assertNotEqual(p, None)
        else:
            p=self._find_exec_param(lines, "py_svc_exec_params",  k+" " )
            self.assertEqual(p, None)

        k="exec_read_only_number"
        if  match_exec[k]:
            p=self._find_exec_param(lines, "py_svc_exec_params",  match_exec[k])
            self.assertNotEqual(p, None)
        else:
            p=self._find_exec_param(lines, "py_svc_exec_params",  k+" " )
            self.assertEqual(p, None)

        k="exec_read_only_empty_number"
        if  match_exec[k]:
            p=self._find_exec_param(lines, "py_svc_exec_params",  match_exec[k])
            self.assertNotEqual(p, None)
        else:
            p=self._find_exec_param(lines, "py_svc_exec_params",  k+" " )
            self.assertEqual(p, None)

        k="exec_read_only_bool"
        if  match_exec[k]:
            p=self._find_exec_param(lines, "py_svc_exec_params",  match_exec[k])
            self.assertNotEqual(p, None)
        else:
            p=self._find_exec_param(lines, "py_svc_exec_params",  k+" " )
            self.assertEqual(p, None)

        k="exec_read_only_bool_missing"
        if  match_exec[k]:
            p=self._find_exec_param(lines, "py_svc_exec_params",  match_exec[k])
            self.assertNotEqual(p, None)
        else:
            p=self._find_exec_param(lines, "py_svc_exec_params",  k+" " )
            self.assertEqual(p, None)

    def test_ServiceExecParamReadonly_0(self):
        match_exec=self._get_match_exec()
        match_exec["exec_read_only_bool"]=None
        self._test_ServiceExecParamReadonly_("/nodes/node_exec_params/node_svc_exec_params/DeviceManager.dcd.xml", match_exec)

    def test_ServiceExecParamReadonly_1(self):
        match_exec=self._get_match_exec()
        match_exec["exec_read_only_bool"]="exec_read_only_bool true"
        self._test_ServiceExecParamReadonly_("/nodes/node_exec_params/node_svc_exec_params/DeviceManager.true-true.dcd.xml", match_exec)

    def test_ServiceExecParamReadonly_1_1(self):
        match_exec=self._get_match_exec()
        match_exec["exec_read_only_bool"]="exec_read_only_bool tRuE"
        self._test_ServiceExecParamReadonly_("/nodes/node_exec_params/node_svc_exec_params/DeviceManager.true-tRuE.dcd.xml", match_exec)

    def test_ServiceExecParamReadonly_2(self):
        match_exec=self._get_match_exec()
        match_exec["exec_read_only_bool"]="exec_read_only_bool 1"
        self._test_ServiceExecParamReadonly_("/nodes/node_exec_params/node_svc_exec_params/DeviceManager.true-1.dcd.xml", match_exec)

    def test_ServiceExecParamReadonly_3(self):
        match_exec=self._get_match_exec()
        match_exec["exec_read_only_bool"]="exec_read_only_bool 0"
        match_exec["exec_read_only_bool_missing"]="exec_read_only_bool_missing FaLsE"
        self._test_ServiceExecParamReadonly_("/nodes/node_exec_params/node_svc_exec_params/DeviceManager.false-0.dcd.xml", match_exec)

    def test_ServiceExecParamReadonly_4(self):
        match_exec=self._get_match_exec()
        match_exec["exec_read_only_bool"]="exec_read_only_bool False"
        self._test_ServiceExecParamReadonly_("/nodes/node_exec_params/node_svc_exec_params/DeviceManager.false-false.dcd.xml", match_exec)

    def test_ServiceExecParamReadonly_4_1(self):
        match_exec=self._get_match_exec()
        match_exec["exec_read_only_bool"]="exec_read_only_bool fAlSe"
        self._test_ServiceExecParamReadonly_("/nodes/node_exec_params/node_svc_exec_params/DeviceManager.false-fAlSe.dcd.xml", match_exec)

    def test_ServiceExecParamReadonly_5(self):
        match_exec=self._get_match_exec()
        match_exec["exec_cmd"] = "exec_cmd changeme"
        match_exec["exec_cmd_empty"] = None
        match_exec["exec_read_only"] = "exec_read_only changeme"
        match_exec["exec_read_only_empty"] = None
        match_exec["exec_read_only_nochange"] = "exec_read_only_nochange nochange"
        match_exec["exec_read_only_number"] = "exec_read_only_number 1000"
        match_exec["exec_read_only_empty_number"] = None
        match_exec["exec_read_only_bool"]="exec_read_only_bool FALSE"
        match_exec["exec_read_only_bool_missing"]=None
        self._test_ServiceExecParamReadonly_("/nodes/node_exec_params/node_svc_exec_params/DeviceManager.missing.dcd.xml", match_exec)


    def test_ServiceExecParamReadonly_NoEmpty(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/node_exec_params/node_svc_exec_params_no_empty/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        from ossie.utils import redhawk
        d=redhawk.attach(scatest.getTestDomainName())

        svc=d.services[0]
        self.assertNotEqual(svc, None)
        
        lines = [ line.rstrip() for line in os.popen('ps -ww -ef | grep "services/py_svc_exec_params"')]
        
        p=self._find_exec_param(lines, "py_svc_exec_params", "exec_cmd cmd_ok")
        self.assertNotEqual(p, None)

        p=self._find_exec_param(lines, "py_svc_exec_params", "exec_cmd_empty ")
        self.assertEqual(p, None)

        p=self._find_exec_param(lines, "py_svc_exec_params", "exec_read_only cmd_ok_read_only")
        self.assertNotEqual(p, None)

        p=self._find_exec_param(lines, "py_svc_exec_params", "exec_read_only_nochange nochange")
        self.assertNotEqual(p, None)

        p=self._find_exec_param(lines, "py_svc_exec_params", "exec_read_only_empty ")
        self.assertEqual(p, None)

        p=self._find_exec_param(lines, "py_svc_exec_params", "exec_read_only_number 12345")
        self.assertNotEqual(p, None)

        p=self._find_exec_param(lines, "py_svc_exec_params", "exec_read_only_empty_number 0")
        self.assertNotEqual(p, None)

        p=self._find_exec_param(lines, "py_svc_exec_params", "exec_read_only_bool True")
        self.assertNotEqual(p, None)


    def test_DuplicateService(self):
        # The first node provides the service
        nb1, devMgr1 = self.launchDeviceManager('/nodes/test_BasicService_node/DeviceManager.dcd.xml')

        # Check that the same service is reported via the DeviceManager and the
        # naming service
        services = devMgr1._get_registeredServices()
        self.assertEqual(1, len(services))
        service_name = URI.stringToName(scatest.getTestDomainName() + '/BasicService1')
        service = self._root.resolve(service_name)
        self.assertTrue(service._is_equivalent(services[0].serviceObject))

        # Launching the second node, it should time out after about 5 seconds
        # waiting for its service to show up in the registered services, which
        # should never happen because the DomainManager should reject it
        nb2, devMgr2 = self.launchDeviceManager('/nodes/DuplicateService_node/DeviceManager.dcd.xml')

        # The first node's service was registered first, so it should be the
        # only one we find, fetching it again to be sure
        services = devMgr1._get_registeredServices()
        self.assertEqual(1, len(services))
        self.assertEqual('BasicService1', services[0].serviceName)
        self.assertEqual(0, len(devMgr2._get_registeredServices()))
        service = self._root.resolve(service_name)
        self.assertTrue(service._is_equivalent(services[0].serviceObject))

        # The duplicate service should have been terminated
        self.assertEqual(0, len(getChildren(nb2.pid)))

        # Launch an executable device for the test application
        nb3, devMgr3 = self.launchDeviceManager("/nodes/test_PortTestDevice2_node/DeviceManager.dcd.xml")

        # Use the service name connection test waveform to make sure that the
        # DomainManager is connecting it correctly to the first service
        sad_file = '/waveforms/PortConnectServiceName/PortConnectServiceName.sad.xml'
        app = self._domMgr.createApplication(sad_file, 'good', [], [])
        components = app._get_registeredComponents()
        comp = components[0].componentObject
        self.assertEqual(1, len(components))
        port = comp.getPort('propset_out')
        connections = port._get_connections()
        self.assertEqual(1, len(connections))
        service = connections[0].port
        props = service.query([CF.DataType('PARAM1', any.to_any(None))])
        value = any.from_any(props[0].value)
        self.assertEqual('ABCD', value)
        app.releaseObject()

        # Terminate the BasicService node, which should prevent the waveform
        # from launching (one more way to verify that it's the first node's
        # service that's being used)
        devMgr1.shutdown()
        self.assertTrue(self.waitTermination(nb1), "Nodebooter did not die after shutdown")

        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, self._domMgr.createApplication, sad_file, 'fail', [], [])

    def test_BadCompositeDevice(self):
        '''
        One programmable and one persona (shared library) in the node.  The persona is not part
        of a composite device relationship and should fail to load leaving us with 1 device on
        startup, the programmable.
        '''
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/BadPersonaNode/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        num_devices = 1
        self.assertEqual(len(getChildren(devmgr_nb.pid)), num_devices)

class DeviceManagerDepsTest(scatest.CorbaTestCase):
    def setUp(self):
        nodebooter, self._domMgr = self.launchDomainManager()
        self._domBooter = nodebooter

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)

        killChildProcesses(os.getpid())

    def test_DevCppDeps(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/node_device_deps/DeviceManager.dcd.xml.cpp")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        devMgr.shutdown()

        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)

    def test_DevPyDeps(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/node_device_deps/DeviceManager.dcd.xml.py")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        devMgr.shutdown()

        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)


    def test_DevCppPyDeps(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/node_device_deps/DeviceManager.dcd.xml.cpp.py")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        scatest.verifyDeviceLaunch(self, devMgr, 2)

        devMgr.shutdown()

        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)

    def test_DevCppPyDeps2(self):
        devmgr_nb_1, devMgr_1 = self.launchDeviceManager("/nodes/node_device_deps/DeviceManager.dcd.xml.cpp")
        self.assertNotEqual(devMgr_1, None)
        devmgr_nb_2, devMgr_2 = self.launchDeviceManager("/nodes/node_device_deps/DeviceManager.dcd.xml.py")
        self.assertNotEqual(devMgr_2, None)

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 2)
        scatest.verifyDeviceLaunch(self, devMgr_1, 1)
        scatest.verifyDeviceLaunch(self, devMgr_2, 1)

        devMgr_1.shutdown()
        devMgr_2.shutdown()

        self.assertTrue(self.waitTermination(devmgr_nb_1), "Nodebooter did not die after shutdown")
        self.assertTrue(self.waitTermination(devmgr_nb_2), "Nodebooter did not die after shutdown")

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)


    @scatest.requireJava
    def test_DevJavaDeps(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/node_device_deps/DeviceManager.dcd.xml.java")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        scatest.verifyDeviceLaunch(self, devMgr, 1)

        devMgr.shutdown()

        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)

    @scatest.requireJava
    def test_DevCppPyJavaDeps(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/node_device_deps/DeviceManager.dcd.xml.cpp.py.java")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        scatest.verifyDeviceLaunch(self, devMgr, 3)

        devMgr.shutdown()

        self.assertTrue(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)

