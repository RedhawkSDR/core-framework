
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
from _unitTestHelpers import runtestHelpers

java_support = runtestHelpers.haveJavaSupport('../Makefile')

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

    def test_javaCompNet(self):
        if not java_support:
            return
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
