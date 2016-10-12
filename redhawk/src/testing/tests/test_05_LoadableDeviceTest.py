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

import unittest, os
from _unitTestHelpers import scatest
from omniORB import CORBA, URI, any
from ossie.cf import CF

class LoadableDeviceTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        self._testFiles = []

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)
        for file in self._testFiles:
            os.unlink(file)

    def test_cpp_BasicOperation(self):
        self.assertNotEqual(self._domMgr, None)

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        self._domMgr.installApplication("/waveforms/CommandWrapperOsProcessor/CommandWrapper.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        # Ensure the expected device is available
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        appFact = self._domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], []) # LOOK MA, NO DAS!

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 1)

        # Verify that properties have been changed from their defaults
        self.assertEqual(len(app._get_componentNamingContexts()), 1)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        cmd = comp.query([CF.DataType(id="DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e", value=any.to_any(None))])[0]
        args = comp.query([CF.DataType(id="DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53", value=any.to_any(None))])[0]
        self.assertEqual(cmd.value._v, "/bin/echo")
        self.assertEqual(args.value._v, ["Hello World"])

        app.stop()
        app.releaseObject()
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_cpp_BigFiles(self):
        self.assertNotEqual(self._domMgr, None)

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        self._domMgr.installApplication("/waveforms/CommandWrapperOsProcessor/CommandWrapper.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        # Ensure the expected device is available
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]
        device.load(devMgr._get_fileSys(), '/data/big_file.txt', CF.LoadableDevice.SHARED_LIBRARY)
        source_file = open('sdr/dev/data/big_file.txt','r')
        destination_file = open('sdr/cache/.ExecutableDevice_node/ExecutableDevice1/data/big_file.txt','r')
        source_data = source_file.read()
        destination_data = destination_file.read()
        source_file.close()
        destination_file.close()
        self.assertEqual(source_data,destination_data)
        device.unload('/data/big_file.txt')

    def test_EmptyDir(self):
        self.assertNotEqual(self._domMgr, None)

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        self._domMgr.installApplication("/waveforms/CommandWrapperEmptyDir/CommandWrapperEmptyDir.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        # Ensure the expected device is available
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        appFact = self._domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], []) # LOOK MA, NO DAS!

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 1)

        app.start()
        app.stop()
        tmpdir = os.path.join(scatest.getSdrCache(), '.BasicTestDevice_node/BasicTestDevice1/components/CommandWrapperEmptyDir/cmd_dir/tmp')
        self.assert_(os.path.isdir(tmpdir))
        app.releaseObject()
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_cpp_RefreshSubdir(self):
        self.assertNotEqual(self._domMgr, None)

        # Ensure the expected device is available
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]
        device.load(devMgr._get_fileSys(), "/data/subdir_test", CF.LoadableDevice.EXECUTABLE)
        self.assertEqual(os.listdir('sdr/cache/.ExecutableDevice_node/ExecutableDevice1/data/subdir_test/subdir_two'),['second_file.txt'])
        os.remove('sdr/cache/.ExecutableDevice_node/ExecutableDevice1/data/subdir_test/subdir_two/second_file.txt')
        self.assertEqual(os.listdir('sdr/cache/.ExecutableDevice_node/ExecutableDevice1/data/subdir_test/subdir_two'),[])
        device.load(devMgr._get_fileSys(), "/data/subdir_test", CF.LoadableDevice.EXECUTABLE)
        self.assertEqual(os.listdir('sdr/cache/.ExecutableDevice_node/ExecutableDevice1/data/subdir_test/subdir_two'),['second_file.txt'])
        device.unload("/data/subdir_test")

    def test_cpp_DirectoryLoad(self):
        self.assertNotEqual(self._domMgr, None)

        # Verify in the devices cache is emtpy
        componentDir = os.path.join(scatest.getSdrPath(), "dom", "components", "CommandWrapperWithDirectoryLoad")
        deviceCacheDir = os.path.join(scatest.getSdrCache(), ".ExecutableDevice_node", "ExecutableDevice1", "components", "CommandWrapperWithDirectoryLoad")
        if os.path.exists(deviceCacheDir):
            os.system("rm -rf %s" % deviceCacheDir)

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        self._domMgr.installApplication("/waveforms/CommandWrapperWithDirectoryLoad/CommandWrapper.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        # Ensure the expected device is available
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        appFact = self._domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], []) # LOOK MA, NO DAS!

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 1)

        # Verify that properties have been changed from their defaults
        self.assertEqual(len(app._get_componentNamingContexts()), 1)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        cmd = comp.query([CF.DataType(id="DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e", value=any.to_any(None))])[0]
        args = comp.query([CF.DataType(id="DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53", value=any.to_any(None))])[0]
        self.assertEqual(cmd.value._v, "/bin/echo")
        self.assertEqual(args.value._v, ["Hello World"])

        app.stop()
        app.releaseObject()
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_py_BasicOperation(self):
        self.assertNotEqual(self._domMgr, None)

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        self._domMgr.installApplication("/waveforms/CommandWrapperOsProcessor/CommandWrapper.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        # Ensure the expected device is available
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        appFact = self._domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], []) # LOOK MA, NO DAS!

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 1)

        # Verify that properties have been changed from their defaults
        self.assertEqual(len(app._get_componentNamingContexts()), 1)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        cmd = comp.query([CF.DataType(id="DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e", value=any.to_any(None))])[0]
        args = comp.query([CF.DataType(id="DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53", value=any.to_any(None))])[0]
        self.assertEqual(cmd.value._v, "/bin/echo")
        self.assertEqual(args.value._v, ["Hello World"])

        app.stop()
        app.releaseObject()
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_py_RefreshSubdir(self):
        self.assertNotEqual(self._domMgr, None)

        # Ensure the expected device is available
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]
        device.load(devMgr._get_fileSys(), "/data/subdir_test", CF.LoadableDevice.EXECUTABLE)
        self.assertEqual(os.listdir('sdr/cache/.BasicTestDevice_node/BasicTestDevice1/data/subdir_test/subdir_two'),['second_file.txt'])
        os.remove('sdr/cache/.BasicTestDevice_node/BasicTestDevice1/data/subdir_test/subdir_two/second_file.txt')
        self.assertEqual(os.listdir('sdr/cache/.BasicTestDevice_node/BasicTestDevice1/data/subdir_test/subdir_two'),[])
        device.load(devMgr._get_fileSys(), "/data/subdir_test", CF.LoadableDevice.EXECUTABLE)
        self.assertEqual(os.listdir('sdr/cache/.BasicTestDevice_node/BasicTestDevice1/data/subdir_test/subdir_two'),['second_file.txt'])
        device.unload("/data/subdir_test")

    def test_py_DirectoryLoad(self):
        self.assertNotEqual(self._domMgr, None)

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        # Verify in the devices cache is emtpy
        componentDir = os.path.join(scatest.getSdrPath(), "dom", "components", "CommandWrapperWithDirectoryLoad")
        deviceCacheDir = os.path.join(scatest.getSdrCache(), ".BasicTestDevice_node", "BasicTestDevice1", "components", "CommandWrapperWithDirectoryLoad")
        if os.path.exists(deviceCacheDir):
            os.system("rm -rf %s" % deviceCacheDir)

        self._domMgr.installApplication("/waveforms/CommandWrapperWithDirectoryLoad/CommandWrapper.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        # Ensure the expected device is available
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        appFact = self._domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], []) # LOOK MA, NO DAS!

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 1)

        # Verify that properties have been changed from their defaults
        self.assertEqual(len(app._get_componentNamingContexts()), 1)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        cmd = comp.query([CF.DataType(id="DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e", value=any.to_any(None))])[0]
        args = comp.query([CF.DataType(id="DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53", value=any.to_any(None))])[0]
        self.assertEqual(cmd.value._v, "/bin/echo")
        self.assertEqual(args.value._v, ["Hello World"])

        # Verify in the devices cache that the directory structure was mirrored exactly
        for root, dirs, files in os.walk(componentDir):
            # turn the abs path in to a relative path
            rel_root = root[len(componentDir)+1:]
            for dir in dirs:
                # Hidden files aren't loaded
                if dir[0] != ".":
                    expectedDir = os.path.join(deviceCacheDir, rel_root, dir)
                    self.assertEqual(os.path.isdir(expectedDir), True, "Dir %s not found at %s" % (dir, expectedDir))
                else:
                    # Don't descend into hidden sub-dirs
                    dirs.remove(dir)
            for f in files:
                # Hidden files aren't loaded
                if f[0] != ".":
                    expectedFile = os.path.join(deviceCacheDir, rel_root, f)
                    self.assertEqual(os.path.isfile(expectedFile), True, "File %s not found at %s" % (f, expectedFile))

        app.stop()
        app.releaseObject()
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_py_UnloadOnRelease(self):
        # Test that releasing the device unloads all files
        deviceCacheDir = os.path.join(scatest.getSdrCache(), ".BasicTestDevice_node", "BasicTestDevice1")
        if os.path.exists(deviceCacheDir):
            os.system("rm -rf %s" % deviceCacheDir)

        self.assertNotEqual(self._domMgr, None)

        # Ensure the expected device is available
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        self.assert_(not os.path.exists(deviceCacheDir + "/components/CommandWrapper"))
        self.assert_(not os.path.exists(deviceCacheDir + "/components/CapacityUser"))

        # Load a some files and directories
        device.load(self._domMgr._get_fileMgr(), "/components/CommandWrapper", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.py", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.prf.xml", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.scd.xml", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.spd.xml", CF.LoadableDevice.EXECUTABLE)

        # Simply check that the cache dir isn't empty and has the correct number of files
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 4)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CapacityUser")), 4)

        device.releaseObject()

        # Wait for the device to unregister.
        self.assert_(self._waitRegisteredDevices(devMgr, 0))

        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 0)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CapacityUser")), 0)

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)

    def test_py_LoadUnload(self):
        # Test that releasing the device unloads all files
        deviceCacheDir = os.path.join(scatest.getSdrCache(), ".BasicTestDevice_node", "BasicTestDevice1")
        if os.path.exists(deviceCacheDir):
            os.system("rm -rf %s" % deviceCacheDir)

        self.assertNotEqual(self._domMgr, None)

        # Ensure the expected device is available
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        self.assert_(not os.path.exists(deviceCacheDir + "/components/CommandWrapper"))
        self.assert_(not os.path.exists(deviceCacheDir + "/components/CapacityUser"))

        # Load a some files and directories
        device.load(self._domMgr._get_fileMgr(), "/components/CommandWrapper", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.py", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.prf.xml", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.scd.xml", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.spd.xml", CF.LoadableDevice.EXECUTABLE)

        # Simply check that the cache dir isn't empty and has the correct number of files
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 4)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CapacityUser")), 4)

        # Check that a second load doesn't do anything weird
        device.load(self._domMgr._get_fileMgr(), "/components/CommandWrapper", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.py", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.prf.xml", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.scd.xml", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.spd.xml", CF.LoadableDevice.EXECUTABLE)

        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 4)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CapacityUser")), 4)

        # Clear out the deviceCache
        os.system("rm -rf %s" % (deviceCacheDir + "/components/CommandWrapper"))
        os.system("rm -rf %s" % (deviceCacheDir + "/components/CapacityUser/CapacityUser.py"))

        self.assertEqual(os.path.exists(deviceCacheDir + "/components/CommandWrapper"), False)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CapacityUser")), 3)

        # Load files and directories, this should copy the files over because they don't exist, even if though the refCnt > 0
        device.load(self._domMgr._get_fileMgr(), "/components/CommandWrapper", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.py", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.prf.xml", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.scd.xml", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.spd.xml", CF.LoadableDevice.EXECUTABLE)

        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 4)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CapacityUser")), 4)

        # Now we need to unload 3 times
        for i in xrange(3):
            self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 4)
            self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CapacityUser")), 4)

            device.unload("/components/CommandWrapper")
            device.unload("/components/CapacityUser/CapacityUser.py")
            device.unload("/components/CapacityUser/CapacityUser.prf.xml")
            device.unload("/components/CapacityUser/CapacityUser.scd.xml")
            device.unload("/components/CapacityUser/CapacityUser.spd.xml")

        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 0)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CapacityUser")), 0)

        device.releaseObject()

        # Wait for the device to unregister.
        self.assert_(self._waitRegisteredDevices(devMgr, 0))

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)

    def test_cpp_LoadUnload(self):
        # Test that releasing the device unloads all files
        deviceCacheDir = os.path.join(scatest.getSdrCache(), ".ExecutableDevice_node", "ExecutableDevice1")
        if os.path.exists(deviceCacheDir):
            os.system("rm -rf %s" % deviceCacheDir)

        self.assertNotEqual(self._domMgr, None)

        # Ensure the expected device is available
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        self.assert_(not os.path.exists(deviceCacheDir + "/components/CommandWrapper"))
        self.assert_(not os.path.exists(deviceCacheDir + "/components/CapacityUser"))

        # Load a some files and directories
        device.load(self._domMgr._get_fileMgr(), "/components/CommandWrapper", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.py", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.prf.xml", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.scd.xml", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.spd.xml", CF.LoadableDevice.EXECUTABLE)

        # Simply check that the cache dir isn't empty and has the correct number of files
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 4)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CapacityUser")), 4)

        # Check that a second load doesn't do anything weird
        device.load(self._domMgr._get_fileMgr(), "/components/CommandWrapper", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.py", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.prf.xml", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.scd.xml", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.spd.xml", CF.LoadableDevice.EXECUTABLE)

        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 4)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CapacityUser")), 4)

        # Clear out the deviceCache
        os.system("rm -rf %s" % (deviceCacheDir + "/components/CommandWrapper"))
        os.system("rm -rf %s" % (deviceCacheDir + "/components/CapacityUser/CapacityUser.py"))

        self.assertEqual(os.path.exists(deviceCacheDir + "/components/CommandWrapper"), False)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CapacityUser")), 3)

        # Load files and directories, this should copy the files over because they don't exist, even if though the refCnt > 0
        device.load(self._domMgr._get_fileMgr(), "/components/CommandWrapper", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.py", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.prf.xml", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.scd.xml", CF.LoadableDevice.EXECUTABLE)
        device.load(self._domMgr._get_fileMgr(), "/components/CapacityUser/CapacityUser.spd.xml", CF.LoadableDevice.EXECUTABLE)

        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 4)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CapacityUser")), 4)

        # Now we need to unload 3 times
        print os.listdir(deviceCacheDir + "/components")
        for i in xrange(3):
            self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 4)
            self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CapacityUser")), 4)

            device.unload("/components/CommandWrapper")
            device.unload("/components/CapacityUser/CapacityUser.py")
            device.unload("/components/CapacityUser/CapacityUser.prf.xml")
            device.unload("/components/CapacityUser/CapacityUser.scd.xml")
            device.unload("/components/CapacityUser/CapacityUser.spd.xml")


        #self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 0)
        # Empty directories get deleted
        self.assert_(not os.path.exists(deviceCacheDir + "/components/CommandWrapper"))
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CapacityUser")), 0)

        device.releaseObject()

        # Wait for the device to unregister.
        self.assert_(self._waitRegisteredDevices(devMgr, 0))

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)

    def _test_FileChanged(self, nodeName, deviceName):
        # Test that updating a file in the SCA filesystem causes a device to reload that file
        # in its cache.
        deviceCacheDir = os.path.join(scatest.getSdrCache(), "." + nodeName, deviceName)
        if os.path.exists(deviceCacheDir):
            os.system("rm -rf %s" % deviceCacheDir)

        self.assertNotEqual(self._domMgr, None)
        fileMgr = self._domMgr._get_fileMgr()

        # Ensure the expected device is available
        devBooter, devMgr = self.launchDeviceManager(dcdFile="/nodes/test_%s/DeviceManager.dcd.xml" % nodeName)
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        # Create the initial file we'll be loading.
        testFile = 'test.out'
        scaPath = '/' + testFile
        srcFile = os.path.join(os.environ['SDRROOT'], 'dom', testFile)
        f = open(srcFile, 'w')
        f.write('Pre')
        f.close()
        self._testFiles.append(srcFile)

        # Load the initial file onto the device and verify that it contains the expected text.
        device.load(fileMgr, scaPath, CF.LoadableDevice.EXECUTABLE)
        cacheFile = os.path.join(deviceCacheDir, testFile)
        f = open(cacheFile, 'r')
        self.assertEqual(f.readline(), 'Pre')
        f.close()

        # Update the file, making sure that the modification time is more recent than the cached file.
        f = open(srcFile, 'w')
        f.write('Post')
        f.close()
        os.utime(srcFile, (os.path.getatime(cacheFile), os.path.getmtime(cacheFile)+1))

        # Load the file again and verify that the cache has been updated.
        device.load(fileMgr, scaPath, CF.LoadableDevice.EXECUTABLE)
        f = open(cacheFile, 'r')
        self.assertEqual(f.readline(), 'Post')
        f.close()

    def test_DeviceBadLoadable(self):
        devBooter, devMgr = self.launchDeviceManager("/nodes/SimpleDevMgr/DeviceManager.dcd.xml")
        device = devMgr._get_registeredDevices()[0]
        fileSys = devMgr._get_fileSys()
        self.assertRaises(CF.LoadableDevice.InvalidLoadKind, device.load, fileSys, '/nodes/SimpleDevMgr/DeviceManager.dcd.xml',CF.LoadableDevice.DRIVER)

    def test_cpp_FileChanged(self):
        self._test_FileChanged("ExecutableDevice_node", "ExecutableDevice1")

    def test_py_FileChanged(self):
        self._test_FileChanged("BasicTestDevice_node", "BasicTestDevice1")

    def test_cpp_SharedLibraryLoad(self):
        # Ensure the expected device is available
        devBooter, devMgr = self.launchDeviceManager(dcdFile="/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        # Load a "shared library" with a very short basename to check that the
        # C++ LoadableDevice base class does not regress; previously, it tried
        # some string operations that assumed the filename was at least 4
        # characters long.
        try:
            device.load(self._domMgr._get_fileMgr(), "/mgr", CF.LoadableDevice.SHARED_LIBRARY)
        except CORBA.COMM_FAILURE:
            self.fail('Device died loading shared library with short path')

    def _failIfOpen(self, fileSys, path):
        for info in fileSys.list(path):
            for prop in info.fileProperties:
                if prop.id != 'IOR_AVAILABLE':
                    continue
                ior_list = prop.value.value()
                self.assertEqual(len(ior_list), 0, 'File was not closed on copy')

    def _test_LoadedFileClosed(self, nodeName):
        devBooter, devMgr = self.launchDeviceManager("/nodes/%s/DeviceManager.dcd.xml" % nodeName)
        device = devMgr._get_registeredDevices()[0]
        fileSys = devMgr._get_fileSys()

        # Load a single file
        scaPath = '/devices/ExecutableDevice/ExecutableDevice'
        device.load(fileSys, scaPath, CF.LoadableDevice.EXECUTABLE)
        self._failIfOpen(fileSys, scaPath)

        # Load an entire directory
        scaPath = '/devices/ExecutableDevice'
        device.load(fileSys, scaPath, CF.LoadableDevice.EXECUTABLE)
        self._failIfOpen(fileSys, scaPath+'/')

    def test_cpp_LoadedFileClosed(self):
        self._test_LoadedFileClosed('test_ExecutableDevice_node')

    def test_py_LoadedFileClosed(self):
        self._test_LoadedFileClosed('test_BasicTestDevice_node')

    def _query(self, comp, propid):
        dt = comp.query([CF.DataType(propid, any.to_any(None))])[0]
        return any.from_any(dt.value)

    def _checkCachePath(self, cachePath, targetPath):
        for path in cachePath.split(':'):
            # The Python device may add a trailing slash, which is harmless
            path = path.rstrip('/')
            if path.endswith(targetPath):
                return True
        return False

    def _test_LoadSoftpkgDir(self, nodeName):
        devBooter, devMgr = self.launchDeviceManager("/nodes/%s/DeviceManager.dcd.xml" % nodeName)
        device = devMgr._get_registeredDevices()[0]
        fileSys = self._domMgr._get_fileMgr()

        # Use a directory known to contain .so files
        scaPath = '/components/linkedLibraryTest/.libs'

        # Get LD_LIBRARY_PATH prior to the load
        path_pre = self._query(device, 'LD_LIBRARY_PATH')
        self.failIf(self._checkCachePath(path_pre, scaPath), 'Library directory already in LD_LIBRARY_PATH')

        device.load(fileSys, scaPath, CF.LoadableDevice.SHARED_LIBRARY)

        # Check that LD_LIBRARY has been augmented with the directory
        path_post = self._query(device, 'LD_LIBRARY_PATH')
        self.assert_(self._checkCachePath(path_post, scaPath), 'Library directory not added to LD_LIBRARY_PATH')

    def test_cpp_LoadSoftpkgDir(self):
        self._test_LoadSoftpkgDir('test_ExecutableDevice_node')

    def test_py_LoadSoftpkgDir(self):
        self._test_LoadSoftpkgDir('test_ExecutableDevicePy_node')

    def _test_LoadSoftpkgLib(self, nodeName):
        devBooter, devMgr = self.launchDeviceManager("/nodes/%s/DeviceManager.dcd.xml" % nodeName)
        device = devMgr._get_registeredDevices()[0]
        fileSys = self._domMgr._get_fileMgr()

        # Use a directory known to contain .so files
        scaPath = '/components/linkedLibraryTest/.libs'
        scaFile = os.path.join(scaPath, 'liblinkedLibraryTest.so')

        # Get LD_LIBRARY_PATH prior to the load
        path_pre = self._query(device, 'LD_LIBRARY_PATH')
        self.failIf(self._checkCachePath(path_pre, scaPath), 'Library directory already in LD_LIBRARY_PATH')

        device.load(fileSys, scaFile, CF.LoadableDevice.SHARED_LIBRARY)

        # Check that LD_LIBRARY has been augmented with the directory
        path_post = self._query(device, 'LD_LIBRARY_PATH')
        self.assert_(self._checkCachePath(path_post, scaPath), 'Library directory not added to LD_LIBRARY_PATH')

    def test_cpp_LoadSoftpkgLib(self):
        self._test_LoadSoftpkgLib('test_ExecutableDevice_node')

    def test_py_LoadSoftpkgLib(self):
        self._test_LoadSoftpkgLib('test_ExecutableDevicePy_node')

    def _test_LoadOctaveDir(self, nodeName):
        devBooter, devMgr = self.launchDeviceManager("/nodes/%s/DeviceManager.dcd.xml" % nodeName)
        device = devMgr._get_registeredDevices()[0]
        fileSys = self._domMgr._get_fileMgr()

        # Use a directory; the current implementations don't check that there
        # are any .m files, so any directory will work
        scaPath = '/components/linkedLibraryTest/.libs'

        # Get OCTAVE_PATH prior to the load
        path_pre = self._query(device, 'OCTAVE_PATH')
        self.failIf(self._checkCachePath(path_pre, scaPath), 'Library directory already in OCTAVE_PATH')

        device.load(fileSys, scaPath, CF.LoadableDevice.SHARED_LIBRARY)

        # Check that OCTAVE_PATH has been augmented with the directory
        path_post = self._query(device, 'OCTAVE_PATH')
        self.assert_(self._checkCachePath(path_post, scaPath), 'Library directory not added to OCTAVE_PATH')

    def test_cpp_LoadOctaveDir(self):
        self._test_LoadOctaveDir('test_ExecutableDevice_node')

    def test_py_LoadOctaveDir(self):
        self._test_LoadOctaveDir('test_ExecutableDevicePy_node')
