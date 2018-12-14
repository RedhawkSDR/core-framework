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

import unittest, os, signal, time, sys, shutil
from subprocess import Popen
from _unitTestHelpers import scatest
from xml.dom import minidom
from omniORB import CORBA, URI, any
from ossie.cf import CF, CF__POA, ExtendedCF
import commands
from ossie.utils.sandbox.launcher import LocalProcess
from ossie import parsers
from ossie.utils.sandbox.naming import NamingContextStub
from ossie.utils import sb

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

def getProcessName(pid):
    str_pid = str(pid)
    process_listing = commands.getoutput('ls /proc').split('\n')
    Name = ''
    for entry in process_listing:
        if entry == str_pid:
            try:
                filename = '/proc/'+entry+'/status'
                fp = open(filename,'r')
                stuff=fp.read()
                fp.close()
                lines = stuff.split('\n')
                for line in lines:
                    if line[:4]=='Name':
                        Name = line.split(':')[1][1:]
                        return Name
            except:
                continue
        else:
            continue
    return Name

def pidExists(pid):
    process_listing = commands.getoutput('ls /proc').split('\n')
    return str(pid) in process_listing

class AwarenessTest(scatest.CorbaTestCase):
    def setUp(self):
        pass # Nothing to do

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)

    def test_cppCompNet(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/cpp_comp_w/cpp_comp_w.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.start()
        time.sleep(0.5)
        nic_name = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='nic_name',value=any.to_any(None))])[0].value._v
        nic_names = os.listdir('/sys/class/net')
        self.assertTrue(nic_name in nic_names)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_pyCompNet(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/py_comp_w/py_comp_w.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.start()
        time.sleep(0.5)
        nic_name = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='nic_name',value=any.to_any(None))])[0].value._v
        nic_names = os.listdir('/sys/class/net')
        self.assertTrue(nic_name in nic_names)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

    @scatest.requireJava
    def test_javaCompNet(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/java_comp_w/java_comp_w.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.start()
        time.sleep(0.5)
        nic_name = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='nic_name',value=any.to_any(None))])[0].value._v
        nic_names = os.listdir('/sys/class/net')
        self.assertTrue(nic_name in nic_names)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

class NicAllocTest(scatest.CorbaTestCase):
    def setUp(self):
        nodebooter, self.domMgr = self.launchDomainManager()
        self.assertNotEqual(self.domMgr, None)
        nodebooter, self.devMgr = self.launchDeviceManager("/nodes/test_NicAllocation_node/DeviceManager.dcd.xml")
        self.assertNotEqual(self.devMgr, None)
        self.dev = self.devMgr._get_registeredDevices()[0]
        props = self.dev.query([CF.DataType('nic_list', any.to_any(None))])
        self.nicNames = any.from_any(props[0].value)

    def _testNicAlloc(self, waveform, cmdline=True):
        sad_file = '/waveforms/NicAllocWave/%s.sad.xml' % waveform
        app = self.domMgr.createApplication(sad_file, waveform, [], [])

        if cmdline:
            for comp in app._get_componentProcessIds():
                with open('/proc/%d/cmdline' % comp.processId, 'r') as fp:
                    args = fp.read().split('\0')
                    self.failUnless('NIC' in args, "%s did not get NIC command line argument" % comp.componentId)

        for comp in app._get_registeredComponents():
            props = comp.componentObject.query([CF.DataType(id='nic_name',value=any.to_any(None))])
            nic_name = any.from_any(props[0].value)
            self.assertTrue(nic_name in self.nicNames, "%s has invalid nic '%s'" % (comp.identifier, nic_name))

    def test_CppNicAlloc(self):
        self._testNicAlloc('NicAllocWaveCpp')

    def test_CppNicAllocIdentifier(self):
        self._testNicAlloc('NicAllocWaveCppIdentifier')

    def test_CppNicAllocCollocated(self):
        self._testNicAlloc('NicAllocWaveCppCollocated')

    def test_CppSharedNicAlloc(self):
        self._testNicAlloc('NicAllocWaveCppShared', cmdline=False)

    def test_CppSharedNicAllocIdentifier(self):
        self._testNicAlloc('NicAllocWaveCppSharedIdentifier', cmdline=False)

    def test_CppSharedNicAllocCollocated(self):
        self._testNicAlloc('NicAllocWaveCppSharedCollocated', cmdline=False)

    def test_PyNicAlloc(self):
        self._testNicAlloc('NicAllocWavePy')

    def test_PyNicAllocIdentifier(self):
        self._testNicAlloc('NicAllocWavePyIdentifier')

    def test_PyNicAllocCollocated(self):
        self._testNicAlloc('NicAllocWavePyCollocated')

    @scatest.requireJava
    def test_JavaNicAlloc(self):
        self._testNicAlloc('NicAllocWaveJava')

    @scatest.requireJava
    def test_JavaNicAllocIdentifier(self):
        self._testNicAlloc('NicAllocWaveJavaIdentifier')

    @scatest.requireJava
    def test_JavaNicAllocCollocated(self):
        self._testNicAlloc('NicAllocWaveJavaCollocated')
