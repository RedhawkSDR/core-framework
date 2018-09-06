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

import unittest, os, signal, time, sys, shutil, platform
from subprocess import Popen
from _unitTestHelpers import scatest
from xml.dom import minidom
from omniORB import CORBA, URI, any
import omniORB
from ossie.cf import CF, CF__POA, ExtendedCF
import commands
from ossie.utils import redhawk
from ossie import properties
import threading

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

# This test suite requires log4cxx support because it checks the domain's log
# output
@scatest.requireLog4cxx
class ApplicationExceptionTest(scatest.CorbaTestCase):
    def setUp(self):
        cfg = "log4j.rootLogger=DEBUG,STDOUT,FILE\n " + \
            "# Direct log messages to FILE\n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.FILE=org.apache.log4j.FileAppender\n" + \
            "log4j.appender.FILE.File=tmp_logfile.log\n" + \
            "log4j.appender.CONSOLE.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.CONSOLE.layout.ConversionPattern=%p:%c - %m [%F:%L]%n\n" + \
            "log4j.appender.FILE.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.FILE.layout.ConversionPattern=%d %p:%c - %m [%F:%L]%n\n"
            
        fp = open('tmp_logfile.config','w')
        fp.write(cfg)
        fp.close()
        self.domBooter, self._domMgr = self.launchDomainManager(loggingURI=os.getcwd()+'/tmp_logfile.config')
        self.devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self._app = None

    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)
        if os.path.exists('tmp_logfile.config'):
            os.remove('tmp_logfile.config')
        if os.path.exists('sdr/tmp_logfile.log'):
            os.remove('sdr/tmp_logfile.log')

    def preconditions(self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")
        self.assertNotEqual(self._app, None, "Application not created")

    def test_description_runTest_props(self):
        dom=redhawk.attach(self._domMgr._get_name())
        sadpath = "/waveforms/python_runtest_w/python_runtest_w.sad.xml"
        self._app = dom.createApplication(sadpath, 'appname', [], [])
        self.preconditions()
        self.assertRaises(CF.UnknownProperties, dom.apps[0].runTest, 0,properties.props_from_dict({'hello':5, 'hey':9}))
        fp = open('sdr/tmp_logfile.log','r')
        contents = fp.read()
        fp.close()
        self.assertNotEquals(contents.find('Run test failed with CF::UnknownProperties for Test ID 0 for properties: hello, hey.'),-1)


class ApplicationFactoryTest(scatest.CorbaTestCase):
    def setUp(self):
        pass # Nothing to do

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)

    def test_BasicOperation(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Load on the  device ID
        das = minidom.parse(os.path.join(scatest.getSdrPath(), "dom/waveforms/CommandWrapper/CommandWrapper_DAS.xml"))
        ds = []
        deviceAssignmentTypeNodeList = das.getElementsByTagName("deviceassignmenttype")
        for node in deviceAssignmentTypeNodeList:
            componentid = node.getElementsByTagName("componentid")[0].firstChild.data
            assigndeviceid = node.getElementsByTagName("assigndeviceid")[0].firstChild.data
            ds.append( CF.DeviceAssignmentType(str(componentid),str(assigndeviceid)) )

        # Ensure the expected device is available
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(ds), 1)
        self.assertEqual(len(domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]
        self.assertEqual(device._get_identifier(), ds[0].assignedDeviceId)

        # Query the known allocation properties
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        nicCapacity = device.query([CF.DataType(id="DCE:4f9a57fc-8fb3-47f6-b779-3c2692f52cf9", value=any.to_any(None))])[0]
        fakeCapacity = device.query([CF.DataType(id="DCE:0cfccc59-7853-4b19-9110-29dccc443374", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000)
        self.assertEqual(bogoMips.value._v, 100000000)
        self.assertEqual(nicCapacity.value._v, 100.0)
        self.assertEqual(fakeCapacity.value._v, 3)

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(),
                             [CF.DataType(id="DCE:6ad84383-49cf-4017-b7ca-0ec4c4917952", value=any.to_any(1.5)),
                              CF.DataType(id="SOMEOBJREF", value=any.to_any(self._orb.object_to_string(appFact)))],
                             ds)

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)
        apps = domMgr._get_applications()
        registeredComponents = app._get_registeredComponents()
        componentIdentifier = registeredComponents[0].componentObject._get_identifier()
        self.assertEqual(registeredComponents[0].identifier,componentIdentifier)
        self.assertEqual(apps[0]._get_registeredComponents()[0].identifier,componentIdentifier)
        componentProfile = registeredComponents[0].componentObject._get_softwareProfile()
        self.assertEqual("/components/CommandWrapper/CommandWrapper.spd.xml",componentProfile)

        # Verify that capacity was allocated
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        # Mem and Nic are __MATH__ properties
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        nicCapacity = device.query([CF.DataType(id="DCE:4f9a57fc-8fb3-47f6-b779-3c2692f52cf9", value=any.to_any(None))])[0]
        fakeCapacity = device.query([CF.DataType(id="DCE:0cfccc59-7853-4b19-9110-29dccc443374", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000-5000)
        self.assertEqual(bogoMips.value._v, 100000000-1000)
        self.assertEqual(nicCapacity.value._v, 50.0)
        self.assertEqual(fakeCapacity.value._v, 1)

        # Verify the various sequences
        self.assertEqual(len(app._get_componentNamingContexts()), 1)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)
        self.assertEqual(compName.componentId, comp._get_identifier())

        # make sure that allocations based on __MATH__ correctly deallocate even if the property value changes
        newCapacityOffset = comp.query([CF.DataType(id="DCE:6ad84383-49cf-4017-b7ca-0ec4c4917952", value=any.to_any(None))])[0]
        self.assertEqual(newCapacityOffset.value._v, 1.5)
        comp.configure([CF.DataType(id="DCE:6ad84383-49cf-4017-b7ca-0ec4c4917952", value=any.to_any(2.5))])
        newCapacityOffset = comp.query([CF.DataType(id="DCE:6ad84383-49cf-4017-b7ca-0ec4c4917952", value=any.to_any(None))])[0]
        self.assertEqual(newCapacityOffset.value._v, 2.5)

        self.assertEqual(len(app._get_componentProcessIds()), 1)
        compProc = app._get_componentProcessIds()[0]
        self.assertNotEqual(compProc, None)
        self.assertEqual(compProc.componentId, comp._get_identifier())

        self.assertEqual(len(app._get_componentDevices()), 1)
        compDev = app._get_componentDevices()[0]
        self.assertNotEqual(compDev, None)
        self.assertEqual(compDev.componentId, comp._get_identifier())

        self.assertEqual(len(app._get_componentImplementations()), 1)
        compImpl = app._get_componentImplementations()[0]
        self.assertNotEqual(compImpl, None)
        self.assertEqual(compImpl.componentId, comp._get_identifier())

        # Per SR:169 verify that the identifier is instantiationid:waveformname
        self.assertEqual(comp._get_identifier()[0:40], "DCE:a39e37bc-280e-406f-9952-5beee6575fb4")
        self.assertEqual(comp._get_identifier()[41:], appFact._get_name() + "_1") # We actually violate the spec in OSSIE

        # Verify the component properties
        cmd = comp.query([CF.DataType(id="DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e", value=any.to_any(None))])[0]
        args = comp.query([CF.DataType(id="DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53", value=any.to_any(None))])[0]
        commandAlive = comp.query([CF.DataType(id="DCE:95f19cb8-679e-48fb-bece-dc199ef45f20", value=any.to_any(None))])[0]
        self.assertEqual(cmd.value._v, "/bin/echo")
        self.assertEqual(args.value._v, ["Hello World"])
        self.assertEqual(commandAlive.value._v, False)
        execparams = comp.query([CF.DataType(id="DCE:85d133fd-1658-4e4d-b3ff-1443cd44c0e2", value=any.to_any(None))])[0]
        args = any.from_any(execparams.value)
        execparams = {}
        for b in args:
            a=eval(b)
            name = a[0]
            value = a[1]
            execparams[name] = value
        self.assert_(execparams.has_key("NAMING_CONTEXT_IOR"))
        self.assert_(execparams.has_key("NAME_BINDING"))
        self.assert_(execparams.has_key("COMPONENT_IDENTIFIER"))
        self.assert_(execparams.has_key("EXEC_PARAM_1"))
        self.assert_(execparams.has_key("EXEC_PARAM_2"))
        self.assert_(execparams.has_key("EXEC_PARAM_4"))
        self.assert_(execparams.has_key("SOMEOBJREF"))
        self.assert_(not execparams.has_key("EXEC_PARAM_3"))
        self.assertEqual(execparams["EXEC_PARAM_1"], "Test1")
        self.assertEqual(execparams["EXEC_PARAM_2"], "2")
        self.assertEqual(execparams["EXEC_PARAM_4"], "True")
        self.assertEqual(execparams["SOMEOBJREF"], self._orb.object_to_string(appFact))

        alonglong = comp.query([CF.DataType(id="DCE:a7de97ee-1e78-45e9-8e2b-204c141656fc", value=any.to_any(None))])[0]
        self.assertEqual(alonglong.value._v, 12345678901)
        alonglong2 = comp.query([CF.DataType(id="DCE:9ec6e2ff-6a4f-4452-8f38-4df47d6eebc1", value=any.to_any(None))])[0]
        self.assertEqual(alonglong2.value._v, 22222222222)

        # Test that we have struct properties
        structprop = comp.query([CF.DataType(id="DCE:ffe634c9-096d-425b-86cc-df1cce50612f", value=any.to_any(None))])[0]
        struct_propseq = any.from_any(structprop.value)
        d = dict([(d["id"], d["value"]) for d in struct_propseq])
        self.assertEqual(d, {"item1": "value1", "item2": 100, "item3": 3.14156})

        app.stop()
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Verify that capacity was deallocated
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        nicCapacity = device.query([CF.DataType(id="DCE:4f9a57fc-8fb3-47f6-b779-3c2692f52cf9", value=any.to_any(None))])[0]
        fakeCapacity = device.query([CF.DataType(id="DCE:0cfccc59-7853-4b19-9110-29dccc443374", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000)
        self.assertEqual(bogoMips.value._v, 100000000)
        self.assertEqual(nicCapacity.value._v, 100.0)
        self.assertEqual(fakeCapacity.value._v, 3)

        domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

    def test_CollocationMATH(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        device = devMgr._get_registeredDevices()[0]

        # Query the known allocation properties
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0].value._v

        self.assertEqual(len(domMgr._get_applications()), 0)

        app = domMgr.createApplication("/waveforms/CommandWrapperColloc/CommandWrapperColloc.sad.xml", 'commandcolloc', [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)

        new_memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0].value._v
        self.assertEqual(new_memCapacity, memCapacity-5000)

    def test_ConfigureNotCalledProperty(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        # Ensure the expected device is available
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        app = domMgr.createApplication("/waveforms/configure_call_property_w/configure_call_property_w.sad.xml", 'configure_call_property', [], [])
        self.assertNotEqual(app, None)
        configure_called = app.query([CF.DataType(id='configure_called',value=any.to_any(None))])
        self.assertEquals(configure_called[0].value._v, False)

    def _test_NamespacedWaveform(self, name):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/foo/wav/%s/%s.sad.xml" % (name,name))
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        number_components = len(app._get_registeredComponents())
        self.assertEqual(number_components, 1)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_NamespacedWaveformCpp(self):
        self._test_NamespacedWaveform('cppwave')

    def test_NamespacedWaveformPython(self):
        self._test_NamespacedWaveform('pywave')

    @scatest.requireJava
    def test_NamespacedWaveformJava(self):
        self._test_NamespacedWaveform('javawave')

    def test_BadOverload(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, domMgr.createApplication, "/waveforms/props_bad_numbers_w/props_bad_numbers_w.sad.xml", "props_app", [], [], )
        try:
            app = domMgr.createApplication("/waveforms/props_bad_numbers_w/props_bad_numbers_w.sad.xml", "props_app", [], [])
        except Exception, e:
            pass
        self.assertNotEqual(e.msg.find('Unable to perform conversion'), -1)
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_PartialStructConfiguration(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/TestComponentProperty_w/TestComponentProperty_w.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        number_components = len(app._get_registeredComponents())
        self.assertEqual(number_components, 1)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_DependencyActionDefaultKind(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.createApplication("/waveforms/math_py_w/math_py_w.sad.xml", 'some_app', [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)

    def test_PartialStructProp(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, domMgr.createApplication, "/waveforms/struct_prop_check_w/struct_prop_check_w.sad.xml", 'some_app', [], [], )

    def test_commandline_props(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        app = domMgr.createApplication("/waveforms/commandline_prop_w/commandline_prop_w.sad.xml", 'some_app', [], [])
        comp_pid = int(app._get_componentProcessIds()[0].processId)
        fp=open('/proc/'+str(comp_pid)+'/cmdline','r')
        comp_contents = fp.read()
        fp.close()
        items = comp_contents.split('\x00')
        self.assertEqual(len(domMgr._get_applications()), 1)
        
        self.assertEqual(items.count('emptytestprop'),0)
        self.assertEqual(items.count('testprop'),1)
        props=[CF.DataType(id='testprop',value=any.to_any(None))]
        retprops = app._get_registeredComponents()[0].componentObject.query(props)
        self.assertEqual('abc',retprops[0].value._v)
        
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_nocommandline_props(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        app = domMgr.createApplication("/waveforms/nocommandline_prop_w/nocommandline_prop_w.sad.xml", 'some_app', [], [])
        comp_pid = int(app._get_componentProcessIds()[0].processId)
        fp=open('/proc/'+str(comp_pid)+'/cmdline','r')
        comp_contents = fp.read()
        fp.close()
        items = comp_contents.split('\x00')
        self.assertEqual(len(domMgr._get_applications()), 1)
        
        self.assertEqual(items.count('testprop'),0)
        props=[CF.DataType(id='testprop',value=any.to_any(None))]
        retprops = app._get_registeredComponents()[0].componentObject.query(props)
        self.assertEqual('abc',retprops[0].value._v)
        
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_NoTimeout(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        app = domMgr.createApplication("/waveforms/start_timeout_w/start_timeout_w.sad.xml", 'my_application', [], [])
        begin_time = time.time()
        app.start()
        end_time = time.time()
        self.assertTrue(end_time-begin_time >= 9.5)
        app.releaseObject()

    def test_InvalidFile(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertRaises(CF.DomainManager.ApplicationInstallationError, domMgr.createApplication, "AppDoesNotExist", 'my_application', [], [])

    def test_NonScaCompliant(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/NonScaCompliant/NonScaCompliant.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        # Verify the various sequences
        self.assertEqual(len(app._get_componentNamingContexts()), 0)
        self.assertEqual(len(app._get_componentProcessIds()), 1)
        self.assertEqual(len(app._get_componentDevices()), 1)
        self.assertEqual(len(app._get_componentImplementations()), 1)
        
        self.assertRaises(CF.Resource.StartError, app.start, )

        # Give the application a moment to run, since there is no programmatic
        # feedback to let us know that the non-compliant component is running
        # other than the pid
        time.sleep(0.5)

        # Clean-up
        self.assertRaises(CF.Resource.StopError, app.stop, )
        app.releaseObject()

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_StructMathDependency(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/TestStructDepWave/TestStructDepWave.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [CF.DataType(id="magicword", value=any.to_any("abracadabra"))], [])

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        ## Clean-up
        app.stop()
        app.releaseObject()

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_MultipleAppFactory(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/NonScaCompliant/NonScaCompliant.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 2)
        domMgr.uninstallApplication(domMgr._get_applicationFactories()[0]._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        domMgr.uninstallApplication(domMgr._get_applicationFactories()[0]._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

    def test_BasicEmptyDAS(self):
        # test basic operation of launching an application and checking allocation capacities

        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Ensure the expected device is available
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        # Query the known allocation properties
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000)
        self.assertEqual(bogoMips.value._v, 100000000)

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], []) # LOOK MA, NO DAS!

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        # Verify that capacity was allocated
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000-5000)
        self.assertEqual(bogoMips.value._v, 100000000-1000)

        app.stop()
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Verify that capacity was deallocated
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000)
        self.assertEqual(bogoMips.value._v, 100000000)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_MalformedComponentFile(self):
        # test basic operation of launching an application and checking allocation capacities

        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        self.assertRaises( CF.DomainManager.ApplicationInstallationError, domMgr.installApplication,"/waveforms/MalformedComponentFile/MalformedComponentFile.sad.xml" )
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Ensure the expected device is available
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]


    def test_MalformedACFile(self):
        # test basic operation of launching an application and checking allocation capacities

        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        self.assertRaises(CF.DomainManager.ApplicationInstallationError, domMgr.installApplication, "/waveforms/MalformedACFile/MalformedACFile.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_MultipleSubprocess(self):
        # test basic operation of launching an application and checking allocation capacities

        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperSubProcess/CommandWrapperSubProcess.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Ensure the expected device is available
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], []) # LOOK MA, NO DAS!

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        componentPid = app._get_componentProcessIds()[0].processId

        # Predicate class to fetch the first child of a process, for use with
        # assertPredicateWithWait()
        class child_finder(object):
            def __init__(self, pid):
                self.pid = pid
                self.children = []
            def __call__(self):
                if not self.children:
                    self.children = getChildren(self.pid)
                if self.children:
                    return self.children[0]
                else:
                    return None

        # Wait for the component process to have at least one child process
        pred = child_finder(componentPid)
        self.assertPredicateWithWait(pred, 'could not find first child process', 1.0)
        firstChild = pred()

        # Wait for the first child process to have at least one child process
        pred = child_finder(firstChild)
        self.assertPredicateWithWait(pred, 'could not find second child process', 1.0)
        secondChild = pred()

        self.assertEqual(pidExists(componentPid), True)
        self.assertEqual(pidExists(firstChild), True)
        self.assertEqual(pidExists(secondChild), True)
        app.releaseObject()

        # Wait up to a second for all of the children to exit
        def children_exited():
            for pid in (componentPid, firstChild, secondChild):
                if pidExists(pid):
                    return False
            return True
        self.assertPredicateWithWait(children_exited, wait=1.0)

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_checkOsVersion(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/ticket_cf_939_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/ticket_cf_939_wave/ticket_cf_939_wave.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

    def test_StackSizeAndPriority(self):
        # test basic operation of launching an application and checking allocation capacities

        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")

        # Ensure the expected device is available
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        device = devMgr._get_registeredDevices()[0]

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], []) # LOOK MA, NO DAS!

        # Query the STACK SIZE and PRIORITY values
        stackSize = device.query([CF.DataType(id="check_STACK_SIZE", value=any.to_any(None))])[0]
        priority = device.query([CF.DataType(id="check_PRIORITY", value=any.to_any(None))])[0]
        self.assertEqual(stackSize.value._v, 8192)
        self.assertEqual(priority.value._v, 15)

        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_NonUUIDInstanceId(self):
        # test basic operation of launching an application and checking allocation capacities

        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperNonUUID/CommandWrapper.sad.xml")

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Ensure the expected device is available
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        device = devMgr._get_registeredDevices()[0]

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], []) # LOOK MA, NO DAS!

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        app.releaseObject()

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_sadPropertyOverride(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperWithPropertyOverride/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Ensure the expected device is available
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        appFact = domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], []) # LOOK MA, NO DAS!

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        # Verify that properties have been changed from their defaults
        self.assertEqual(len(app._get_componentNamingContexts()), 1)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        cmd = comp.query([CF.DataType(id="DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e", value=any.to_any(None))])[0]
        args = comp.query([CF.DataType(id="DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53", value=any.to_any(None))])[0]
        self.assertEqual(cmd.value._v, "/bin/date")
        self.assertEqual(args.value._v, ["-u"])
        someprop3 = comp.query([CF.DataType(id="DCE:6ad84383-49cf-4017-b7ca-0ec4c4917952", value=any.to_any(None))])[0]
        self.assertEqual(someprop3.value._v, 1.0)

        structprop = comp.query([CF.DataType(id="DCE:ffe634c9-096d-425b-86cc-df1cce50612f", value=any.to_any(None))])[0]
        struct_propseq = any.from_any(structprop.value)
        d = dict([(d["id"], d["value"]) for d in struct_propseq])
        self.assertEqual(d, {"item1": "a new string", "item2": 500, "item3": 3.33})

        execparams = comp.query([CF.DataType(id="DCE:85d133fd-1658-4e4d-b3ff-1443cd44c0e2", value=any.to_any(None))])[0]
        self.assertNotEqual(execparams.value._v, None)
        args = any.from_any(execparams.value)
        execparams = {}
        for b in args:
            a = eval(b)
            name = a[0]
            value = a[1]
            execparams[name] = value
        self.assert_(execparams.has_key("NAMING_CONTEXT_IOR"))
        self.assert_(execparams.has_key("NAME_BINDING"))
        self.assert_(execparams.has_key("COMPONENT_IDENTIFIER"))
        self.assert_(execparams.has_key("EXEC_PARAM_1"))
        self.assert_(execparams.has_key("EXEC_PARAM_2"))
        self.assert_(not execparams.has_key("EXEC_PARAM_3"))
        self.assertEqual(execparams["EXEC_PARAM_1"], "New1")
        self.assertEqual(execparams["EXEC_PARAM_2"], "-2")

        app.stop()
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_sadReadOnlyPropertyOverride(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperWithPropertyOverride/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Ensure the expected device is available
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        appFact = domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], []) # LOOK MA, NO DAS!

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        self.assertEqual(len(app._get_componentNamingContexts()), 1)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        failedConfigure = False
        try:
            comp.configure([CF.DataType(id="DCE:95f19cb8-679e-48fb-bece-dc199ef45f20", value=any.to_any(True))])
        except:
            failedConfigure = True

        self.assertEqual(failedConfigure, True)

        # Verify that properties have been changed from their defaults

        app.stop()
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_createPropertyOverride(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Ensure the expected device is available
        self.assertEqual(len(domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        appFact = domMgr._get_applicationFactories()[0]

        initProps = []
        initProps.append(CF.DataType(id="DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e", value=any.to_any("/bin/uname")))
        initProps.append(CF.DataType(id="DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53", value=any.to_any(["-a"])))
        initProps.append(CF.DataType(id="EXEC_PARAM_1", value=any.to_any("NewValue")))

        newvalue = [CF.DataType(id="item1", value=any.to_any("testing 1,2,3")),
                    CF.DataType(id="item2", value=any.to_any(44)),
                    CF.DataType(id="item3", value=any.to_any(4.25))]
        initProps.append(CF.DataType(id="DCE:ffe634c9-096d-425b-86cc-df1cce50612f", value=any.to_any(newvalue)))

        app = appFact.create(appFact._get_name(), initProps, []) # LOOK MA, NO DAS!

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        # Verify that properties have been changed from their defaults
        self.assertEqual(len(app._get_componentNamingContexts()), 1)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        cmd = comp.query([CF.DataType(id="DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e", value=any.to_any(None))])[0]
        args = comp.query([CF.DataType(id="DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53", value=any.to_any(None))])[0]
        self.assertEqual(cmd.value._v, "/bin/uname")
        self.assertEqual(args.value._v, ["-a"])
        execp = comp.query([CF.DataType(id="EXEC_PARAM_1", value=any.to_any(None))])[0]
        self.assertEqual(execp.value._v, "NewValue")

        structprop = comp.query([CF.DataType(id="DCE:ffe634c9-096d-425b-86cc-df1cce50612f", value=any.to_any(None))])[0]
        struct_propseq = any.from_any(structprop.value)
        d = dict([(d["id"], d["value"]) for d in struct_propseq])
        self.assertEqual(d, {"item1": "testing 1,2,3", "item2": 44, "item3": 4.25})

        app.stop()
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Test that an empty string exec-param doesn't mess things up totally
        initProps = []
        initProps.append(CF.DataType(id="EXEC_PARAM_1", value=any.to_any("")))

        app = appFact.create(appFact._get_name(), initProps, []) # LOOK MA, NO DAS!

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        # Verify that properties have been changed from their defaults
        self.assertEqual(len(app._get_componentNamingContexts()), 1)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        execp = comp.query([CF.DataType(id="EXEC_PARAM_1", value=any.to_any(None))])[0]
        self.assertEqual(execp.value._v, "")

        app.stop()
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_relativeXmlPaths(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperWithRelativePaths/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        self.assertEqual(len(app._get_componentNamingContexts()), 1)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        app.stop()
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_directoryLoad(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperWithDirectoryLoad/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])

        # TODO Test the that the files actually got copied to the cache

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        self.assertEqual(len(app._get_componentNamingContexts()), 1)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        app.stop()
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_AC_order_test(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/ac_prop_order_test/ac_prop_order_test.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        props = app.query([])
        for prop in props:
            if prop.id == 'other_component':
                self.assertEqual(any.from_any(prop.value), 2.0)

        # TODO Test the that the files actually got copied to the cache

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        self.assertEqual(len(app._get_componentNamingContexts()), 2)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        app.stop()
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_appFailResource(self):
        # test case where launching of an app fails (due to a failed configure call
        # in CommandWrapperBad)
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        dev = devMgr._get_registeredDevices()[0]

        props = [CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None)), CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))]
        retval = dev.query(props)
        original_mem = int(retval[0].value.value())
        original_mips = int(retval[1].value.value())

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperBadSad/CommandWrapperBadSad.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]

        app = None

        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, appFact.create, appFact._get_name(), [], [])

        props = [CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None)), CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))]
        retval = dev.query(props)
        # verify that all capacities were returned
        self.assertEqual(original_mem, int(retval[0].value.value()))
        self.assertEqual(original_mips, int(retval[1].value.value()))

        # TODO Test the that the files actually got copied to the cache

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        self.assertEqual(app, None)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_appFailAssembly(self):
        # Test that a component with no deps can be loaded on a device with
        # no properties
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        dev = devMgr._get_registeredDevices()[0]

        props = [CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None)), CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))]
        retval = dev.query(props)
        original_mem = int(retval[0].value.value())
        original_mips = int(retval[1].value.value())

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperBadSadAssembly/CommandWrapperBadSadAssembly.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]

        app = None

        try:
            app = appFact.create(appFact._get_name(), [], [])
        except:
            pass

        props = [CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None)), CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))]
        retval = dev.query(props)
        self.assertEqual(original_mem, int(retval[0].value.value()))
        self.assertEqual(original_mips, int(retval[1].value.value()))

        # TODO Test the that the files actually got copied to the cache

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        self.assertEqual(app, None)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_appFailDAS(self):
        # Test that a waveform with an invalid DAS fails 
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        dev = devMgr._get_registeredDevices()[0]

        props = [CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None)), CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))]
        retval = dev.query(props)
        original_mem = int(retval[0].value.value())
        original_mips = int(retval[1].value.value())

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]

        app = None


        das = [ CF.DeviceAssignmentType("DCE:a39e37bc-280e-406f-9952-5beee6575fb4", "DCE:bad_id") ]
        try:
            app = appFact.create(appFact._get_name(), [], das)
        except:
            pass

        props = [CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None)), CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))]
        retval = dev.query(props)
        self.assertEqual(original_mem, int(retval[0].value.value()))
        self.assertEqual(original_mips, int(retval[1].value.value()))

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        self.assertEqual(app, None)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_appComponentToDeviceMatchingFail(self):
        # Test that a component that has device dependencies won't
        # be matched to a device with no allocation properties
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDeviceNoAllocDeps_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        dev = devMgr._get_registeredDevices()[0]

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperOneDep/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]

        app = None

        try:
            app = appFact.create(appFact._get_name(), [], [])
        except:
            pass

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        self.assertEqual(app, None)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_appComponentToDeviceMatchOverride(self):
        # Test that a component that has device dependencies won't
        # be matched to a device with no allocation properties
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDeviceAllocOverride/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        dev = devMgr._get_registeredDevices()[0]

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperMatchDep/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]

        app = None

        try:
            app = appFact.create(appFact._get_name(), [], [])
        except:
            pass

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        self.assertNotEqual(app, None)

        domMgr.uninstallApplication(appFact._get_identifier())


    def test_usesDevicePass(self):
        # Test that a component with no deps can be loaded on a device with
        # no properties
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_MultipleSimpleDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        devUses = None
        for dev in devMgr._get_registeredDevices():
            if dev._get_identifier() == 'DCE:fe4fee1e-f305-454b-aa96-9f6e7d960cde':
                devUses = dev
        self.assertNotEqual(devUses, None)

        props = [CF.DataType(id="DCE:8cad8ca5-c155-4d1d-ae40-e194aa1d855f", value=any.to_any(None))]
        retval = devUses.query(props)
        original_bw = int(retval[0].value.value())

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperUsesDevice/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]

        app = None

        try:
            app = appFact.create(appFact._get_name(), [], [])
        except:
            pass

        props = [CF.DataType(id="DCE:8cad8ca5-c155-4d1d-ae40-e194aa1d855f", value=any.to_any(None))]
        retval = devUses.query(props)
        middle_bw = int(retval[0].value.value())

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        das = app._get_componentDevices()
        devs=domMgr._get_deviceManagers()[0]._get_registeredDevices()
        dev_0_id = devs[0]._get_identifier()
        dev_1_id = ''
        if dev_0_id == 'DCE:fe4fee1e-f305-454b-aa96-9f6e7d960cde':
            dev_1_id = devs[1]._get_identifier()
        else:
            dev_0_id = devs[1]._get_identifier()
            dev_1_id = devs[0]._get_identifier()
        self.assertEqual(dev_0_id,'DCE:fe4fee1e-f305-454b-aa96-9f6e7d960cde')
        self.assertEqual(dev_1_id,'DCE:8f3478e3-626e-45c3-bd01-0a8117dbe59b')
        pid = app._get_componentProcessIds()[0].processId
        status,output = commands.getstatusoutput('kill -0 '+str(pid))
        self.assertEqual(status,0)

        app.stop()
        app.releaseObject()

        status,output = commands.getstatusoutput('kill -0 '+str(pid))
        self.assertNotEqual(status,0)

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        props = [CF.DataType(id="DCE:8cad8ca5-c155-4d1d-ae40-e194aa1d855f", value=any.to_any(None))]
        retval = devUses.query(props)
        final_bw = int(retval[0].value.value())

        self.assertEqual(original_bw, middle_bw+150000)
        self.assertEqual(original_bw, final_bw)

        domMgr.uninstallApplication(appFact._get_identifier())

        devs = devMgr._get_registeredDevices()
        foundProperty = False
        for dev in devs:
            if dev._get_label() == "BasicUsesDevice1":
                foundProperty = True
                props = dev.query([CF.DataType(id="number_connections",value=any.to_any(None))])
                self.assertEqual(props[0].value._v, 0, "Device connection was not broken")
        self.assertEqual(foundProperty, True)


    def test_usesDevicePassProperty(self):
        # Test that a component with no deps can be loaded on a device with
        # no properties
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_MultipleSimpleDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        devUses = None
        for dev in devMgr._get_registeredDevices():
            if dev._get_identifier() == 'DCE:fe4fee1e-f305-454b-aa96-9f6e7d960cde':
                devUses = dev
        self.assertNotEqual(devUses, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperUsesDeviceProperty/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]

        app = None

        try:
            app = appFact.create(appFact._get_name(), [], [])
        except:
            pass

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        app.stop()
        app.releaseObject()

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        domMgr.uninstallApplication(appFact._get_identifier())



    def test_usesDeviceFail(self):
        # Test that a component with no deps can be loaded on a device with
        # no properties
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_MultipleSimpleDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        devUses = None
        for dev in devMgr._get_registeredDevices():
            if dev._get_identifier() == 'DCE:fe4fee1e-f305-454b-aa96-9f6e7d960cde':
                devUses = dev

        self.assertNotEqual(devUses, None)
        props = [CF.DataType(id="DCE:8cad8ca5-c155-4d1d-ae40-e194aa1d855f", value=any.to_any(None))]
        retval = devUses.query(props)
        original_bw = int(retval[0].value.value())

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperUsesDeviceFail/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]

        app = None

        try:
            app = appFact.create(appFact._get_name(), [], [])
        except:
            app = None

        props = [CF.DataType(id="DCE:8cad8ca5-c155-4d1d-ae40-e194aa1d855f", value=any.to_any(None))]
        retval = devUses.query(props)
        middle_bw = int(retval[0].value.value())

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        if app != None:
            app.stop()
            app.releaseObject()

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        props = [CF.DataType(id="DCE:8cad8ca5-c155-4d1d-ae40-e194aa1d855f", value=any.to_any(None))]
        retval = devUses.query(props)
        final_bw = int(retval[0].value.value())

        self.assertEqual(original_bw, middle_bw)
        self.assertEqual(original_bw, final_bw)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_usesDeviceCrash(self):
        # Test that releasing an application that includes usesdevice relationships
        # does not crash the DomainManager on future application creation.
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_MultipleSimpleDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CommandWrapperUsesDevice/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]

        # Create the first instance of the application. If this fails, other things
        # are wrong.
        try:
            app = appFact.create(appFact._get_name(), [], [])
        except:
            self.fail("Failure creating initial application")

        # Release the application. This will decrement the reference counts that
        # led to the original crash.
        app.releaseObject()

        # Create a new instance of the application. This would crash the DomainManager
        # because it was holding references that had been deleted; check that this is
        # no longer the case.
        try:
            app = appFact.create(appFact._get_name(), [], [])
        except:
            self.fail("Failure creating second application")

        # Clean up, just to make sure there are no remaining reference problems.
        app.releaseObject()


    def test_hostCollocationFail(self):
        # Test that creating an application that uses host collocation fails
        # if all the components cannot be allocated on the same device.
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_MultipleBasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CollocationTest/CollocationTest.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        # Allocate enough capacity on the devices that the components cannot
        # be collocated.
        for device in devMgr._get_registeredDevices():
            device.allocateCapacity([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(1000))])

        appFact = domMgr._get_applicationFactories()[0]

        try:
            app = appFact.create(appFact._get_name(), [], [])
        except:
            pass
        else:
            app.stop()
            app.releaseObject()
            self.fail('Application creation should fail')

    def test_NoAssemblyController(self):
        # Test that installing an application without an assembly controller fails
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        sadFile = "/waveforms/CommandWrapperNoAssembly/CommandWrapper.sad.xml"
        self.assertRaises(CF.DomainManager.ApplicationInstallationError, domMgr.installApplication, sadFile)

    def test_hostCollocationDAS(self):
        # Test that creating an application that uses host collocation with
        # a DAS will place all collocated components on the same device.
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_MultipleBasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CollocationTest/CollocationTest.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        deviceID = "DCE:f6ee5832-6a99-4ff8-bacf-d2f9c7574a72"

        # Create a hard-coded DAS to put the second component on the second
        # device, to make sure that both components get placed correctly.
        das = [ CF.DeviceAssignmentType("DCE:8eebcfae-9a42-47cb-b645-35eccf7a69a0", deviceID) ]
        try:
            app = appFact.create(appFact._get_name(), [], das)
        except:
            self.fail("Did not create application with collocated components")

        # Check that all the components are on the expected device.
        for device in app._get_componentDevices():
            self.assertEqual(device.assignedDeviceId, deviceID)

        app.stop()
        app.releaseObject()

    def test_MultipleImplementations(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperMultipleImplementations/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Ensure the expected device is available
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        # Query the known allocation properties
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000)
        self.assertEqual(bogoMips.value._v, 100000000)

        ds = []
        ds.append( CF.DeviceAssignmentType("DCE:a39e37bc-280e-406f-9952-5beee6575fb4",str(device._get_identifier())) )

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], ds)

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        # Verify that capacity was allocated
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000-1000000-5000)
        self.assertEqual(bogoMips.value._v, 100000000-1000-1000)

        # Verify the component properties
        self.assertEqual(len(app._get_componentNamingContexts()), 2)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        cmd = comp.query([CF.DataType(id="DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e", value=any.to_any(None))])[0]
        args = comp.query([CF.DataType(id="DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53", value=any.to_any(None))])[0]
        commandAlive = comp.query([CF.DataType(id="DCE:95f19cb8-679e-48fb-bece-dc199ef45f20", value=any.to_any(None))])[0]
        self.assertEqual(cmd.value._v, "/bin/echo")
        self.assertEqual(args.value._v, ["Hello World"])
        self.assertEqual(commandAlive.value._v, False)
        execparams = comp.query([CF.DataType(id="DCE:85d133fd-1658-4e4d-b3ff-1443cd44c0e2", value=any.to_any(None))])[0]
        args = execparams.value._v.split()
        execparams = {}
        while len(args) > 0:
            name = args.pop(0)
            value = args.pop(0)
            execparams[name] = value
        self.assert_(execparams.has_key("NAMING_CONTEXT_IOR"))
        self.assert_(execparams.has_key("NAME_BINDING"))
        self.assert_(execparams.has_key("COMPONENT_IDENTIFIER"))
        self.assert_(execparams.has_key("EXEC_PARAM_1"))
        self.assert_(execparams.has_key("EXEC_PARAM_2"))
        self.assert_(execparams.has_key("EXEC_PARAM_3"))
        self.assertEqual(execparams["EXEC_PARAM_1"], "Test1")
        self.assertEqual(execparams["EXEC_PARAM_2"], "2")
        self.assertEqual(execparams["EXEC_PARAM_3"], "3.3333")

        app.stop()
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Verify that capacity was deallocated
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000)
        self.assertEqual(bogoMips.value._v, 100000000)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_MultipleImplementationsWithDAS(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperMultipleImplementations/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Ensure the expected device is available
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        # Query the known allocation properties
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000)
        self.assertEqual(bogoMips.value._v, 100000000)

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        # Verify that capacity was allocated
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000-1000000-5000)
        self.assertEqual(bogoMips.value._v, 100000000-1000-1000)

        # Verify the component properties
        self.assertEqual(len(app._get_componentNamingContexts()), 2)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        cmd = comp.query([CF.DataType(id="DCE:a4e7b230-1d17-4a86-aeff-ddc6ea3df26e", value=any.to_any(None))])[0]
        args = comp.query([CF.DataType(id="DCE:5d8bfe8d-bc25-4f26-8144-248bc343aa53", value=any.to_any(None))])[0]
        commandAlive = comp.query([CF.DataType(id="DCE:95f19cb8-679e-48fb-bece-dc199ef45f20", value=any.to_any(None))])[0]
        self.assertEqual(cmd.value._v, "/bin/echo")
        self.assertEqual(args.value._v, ["Hello World"])
        self.assertEqual(commandAlive.value._v, False)
        execparams = comp.query([CF.DataType(id="DCE:85d133fd-1658-4e4d-b3ff-1443cd44c0e2", value=any.to_any(None))])[0]
        args = execparams.value._v.split()
        execparams = {}
        while len(args) > 0:
            name = args.pop(0)
            value = args.pop(0)
            execparams[name] = value
        self.assert_(execparams.has_key("NAMING_CONTEXT_IOR"))
        self.assert_(execparams.has_key("NAME_BINDING"))
        self.assert_(execparams.has_key("COMPONENT_IDENTIFIER"))
        self.assert_(execparams.has_key("EXEC_PARAM_1"))
        self.assert_(execparams.has_key("EXEC_PARAM_2"))
        self.assert_(execparams.has_key("EXEC_PARAM_3"))
        self.assertEqual(execparams["EXEC_PARAM_1"], "Test1")
        self.assertEqual(execparams["EXEC_PARAM_2"], "2")
        self.assertEqual(execparams["EXEC_PARAM_3"], "3.3333")

        app.stop()
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Verify that capacity was deallocated
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000)
        self.assertEqual(bogoMips.value._v, 100000000)

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_cacheCleanup(self):
        deviceCacheDir = os.path.join(scatest.getSdrCache(), ".BasicTestDevice_node", "BasicTestDevice1")
        if os.path.exists(deviceCacheDir):
            os.system("rm -rf %s" % deviceCacheDir)

        # test that if we load/unload an application, after the final unload the cache is cleared
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        domMgr.installApplication("/waveforms/CommandWrapperWithDirectoryLoad/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 2)
        self.assertEqual(len(domMgr._get_applications()), 0)

        apps = []
        for i in xrange(2):
            for appFact in domMgr._get_applicationFactories():
                app = appFact.create(appFact._get_name(), [], [])
                apps.append(app)

        self.assertEqual(len(domMgr._get_applications()), 4)
        # TODO Test the that the files actually got copied to the cache
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 1)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapperWithDirectoryLoad")), 4)

        for app in apps:
            app.stop()
            app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 0)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapperWithDirectoryLoad")), 0)

        apps = []
        for appFact in domMgr._get_applicationFactories():
            app = appFact.create(appFact._get_name(), [], [])
            apps.append(app)

        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 1)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapperWithDirectoryLoad")), 4)

        for app in apps:
            app.stop()
            app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapper")), 0)
        self.assertEqual(len(os.listdir(deviceCacheDir + "/components/CommandWrapperWithDirectoryLoad")), 0)

    def test_FailStartup(self):
        # Verify that if a component fails to start, any allocated resources are restored
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        # Set the component name binding timeout to a more reasonable 2 seconds, since the
        # failures are pretty quick.
        id = "COMPONENT_BINDING_TIMEOUT"
        value = CORBA.Any(CORBA.TC_ulong, 2)
        domMgr.configure([CF.DataType(id, value)])

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/FailStartup/FailStartup.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]
        self.assertEqual(len(domMgr._get_applications()), 0)
        
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        # Query the known allocation properties
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        nicCapacity = device.query([CF.DataType(id="DCE:4f9a57fc-8fb3-47f6-b779-3c2692f52cf9", value=any.to_any(None))])[0]
        fakeCapacity = device.query([CF.DataType(id="DCE:0cfccc59-7853-4b19-9110-29dccc443374", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000)
        self.assertEqual(bogoMips.value._v, 100000000)
        self.assertEqual(nicCapacity.value._v, 100.0)
        self.assertEqual(fakeCapacity.value._v, 3)

        for failurePos in ("constructor", "initializeProperties", "initialize"):
            self.assertRaises(CF.ApplicationFactory.CreateApplicationError, appFact.create, appFact._get_name(), [CF.DataType(id="FAIL_AT", value=any.to_any(failurePos))], [])
            self.assertEqual(len(domMgr._get_applications()), 0)
            # Verify that capacity was not allocated
            bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
            memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
            nicCapacity = device.query([CF.DataType(id="DCE:4f9a57fc-8fb3-47f6-b779-3c2692f52cf9", value=any.to_any(None))])[0]
            fakeCapacity = device.query([CF.DataType(id="DCE:0cfccc59-7853-4b19-9110-29dccc443374", value=any.to_any(None))])[0]
            self.assertEqual(memCapacity.value._v, 100000000)
            self.assertEqual(bogoMips.value._v, 100000000)
            self.assertEqual(nicCapacity.value._v, 100.0)
            self.assertEqual(fakeCapacity.value._v, 3)

        # Now check that things are working normally
        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        # Verify that capacity was not allocated
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        nicCapacity = device.query([CF.DataType(id="DCE:4f9a57fc-8fb3-47f6-b779-3c2692f52cf9", value=any.to_any(None))])[0]
        fakeCapacity = device.query([CF.DataType(id="DCE:0cfccc59-7853-4b19-9110-29dccc443374", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000-5000)
        self.assertEqual(bogoMips.value._v, 100000000-1000)
        self.assertEqual(nicCapacity.value._v, 100.0)
        self.assertEqual(fakeCapacity.value._v, 3)

        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)
        # Verify that capacity was not allocated
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        nicCapacity = device.query([CF.DataType(id="DCE:4f9a57fc-8fb3-47f6-b779-3c2692f52cf9", value=any.to_any(None))])[0]
        fakeCapacity = device.query([CF.DataType(id="DCE:0cfccc59-7853-4b19-9110-29dccc443374", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000)
        self.assertEqual(bogoMips.value._v, 100000000)
        self.assertEqual(nicCapacity.value._v, 100.0)
        self.assertEqual(fakeCapacity.value._v, 3)

    def test_fileProblems(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        cmdWrapperDir = os.path.join(scatest.getSdrPath(), "dom", "components", "CommandWrapper")
        shutil.copy(cmdWrapperDir + "/CommandWrapper.spd.xml", cmdWrapperDir + "/CommandWrapper.spd.xml.bak")

        domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        os.remove(cmdWrapperDir + "/CommandWrapper.spd.xml")
        os.system("ls %s" % cmdWrapperDir)

        try:
            appFact = domMgr._get_applicationFactories()[0]
            # Since we deleted the file in question, we expect a graceful error condition...not an outright crash or
            # failure
            self.assertRaises(CF.ApplicationFactory.CreateApplicationError, appFact.create, appFact._get_name(), [], [])
            self.assertEqual(len(domMgr._get_applicationFactories()), 1)
            self.assertEqual(len(domMgr._get_applications()), 0)

            # Now make sure things aren't totatally busted
            shutil.copy(cmdWrapperDir + "/CommandWrapper.spd.xml.bak", cmdWrapperDir + "/CommandWrapper.spd.xml")
            app = appFact.create(appFact._get_name(), [], [])

            self.assertEqual(len(app._get_componentNamingContexts()), 1)
            compName = app._get_componentNamingContexts()[0]
            comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
            self.assertNotEqual(comp, None)

            app.stop()
            app.releaseObject()
            self.assertEqual(len(domMgr._get_applicationFactories()), 1)
            self.assertEqual(len(domMgr._get_applications()), 0)

            domMgr.uninstallApplication(appFact._get_identifier())
        finally:
            shutil.copy(cmdWrapperDir + "/CommandWrapper.spd.xml.bak", cmdWrapperDir + "/CommandWrapper.spd.xml")
            try:
                os.remove(cmdWrapperDir + "/CommandWrapper.spd.xml.bak")
            except OSError:
                pass

    def test_NoDepsFail(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        self.assertRaises(CF.DomainManager.ApplicationInstallationError, domMgr.installApplication, "/waveforms/CommandWrapperWithNoDeps/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_PrfParsingErrors(self):
        # Inject multiple validation errors into CommandWrapper.prf.xml to ensure that the system tolerates it
        # The tests in here are based off specific errors that have been seen in the wild and are not exhaustive
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]

        cmdWrapperDir = os.path.join(scatest.getSdrPath(), "dom", "components", "CommandWrapper")
        cmdWrapperFile = os.path.join(cmdWrapperDir, "CommandWrapper.prf.xml")
        cmdWrapperBakFile = os.path.join(cmdWrapperDir, "CommandWrapper.prf.xml.bak")
        shutil.copy(cmdWrapperFile, cmdWrapperBakFile)

        try:
            prf = minidom.parse(cmdWrapperFile)

            # Create a simple with invalid structure for enumerations
            simple = prf.createElement("simple")
            simple.setAttribute("id", "bad_enum_prop")
            simple.setAttribute("type", "string")

            enum1 = prf.createElement("enumeration")
            enum1.setAttribute("label", "a")
            enum1.setAttribute("value", "1")
            simple.appendChild(enum1)

            enum2 = prf.createElement("enumeration")
            enum2.setAttribute("label", "a")
            enum2.setAttribute("value", "2")
            simple.appendChild(enum2)

            enum3 = prf.createElement("enumeration")
            enum3.setAttribute("label", "a")
            enum3.setAttribute("value", "3")
            simple.appendChild(enum3)

            prf.getElementsByTagName("properties")[0].appendChild(simple)

            f = open(cmdWrapperFile, "w")
            f.write(prf.toprettyxml())
            f.close()

            self.assertRaises(CF.ApplicationFactory.CreateApplicationError, appFact.create, appFact._get_name(), [], [])

            self.assertEqual(len(domMgr._get_applicationFactories()), 1)
            self.assertEqual(len(domMgr._get_applications()), 0)
        finally:
            shutil.copy(cmdWrapperBakFile, cmdWrapperFile)
            try:
                os.remove(cmdWrapperBakFile)
            except OSError:
                pass

    def test_softpkgDependency_colliding_PyDev(self):
        dommgr_nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CommandWrapperSPDDep/CommandWrapperSPDDep.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        domMgr.installApplication("/waveforms/CommandWrapperSPDDep_collide/CommandWrapperSPDDep_collide.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 2)
        self.assertEqual(len(domMgr._get_applications()), 1)
        appFact_2 = domMgr._get_applicationFactories()[1]
        app_2 = appFact_2.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 2)
        app.releaseObject()
        app_2.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        domMgr.uninstallApplication(appFact_2._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

    def test_softpkgDependency_PyDev(self):
        dommgr_nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CommandWrapperSPDDep/CommandWrapperSPDDep.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

    def test_softpkgDependency_automatic(self):
        dommgr_nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CommandWrapperSPDDepAutomatic/CommandWrapperSPDDepAutomatic.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

    def test_softpkgDependency_processormatch(self):
        dommgr_nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CommandWrapperSPDDepProcessorMatch/CommandWrapperSPDDepProcessorMatch.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        try:
            app = appFact.create(appFact._get_name(), [], [])
        except:
            pass
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

    def test_nestedsoftpkgDependency(self):
        dommgr_nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CommandWrapperNestedSPDDep/CommandWrapperNestedSPDDep.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

    def test_softpkgDependency_colliding_CppDev(self):
        dommgr_nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CommandWrapperSPDDep/CommandWrapperSPDDep.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        domMgr.installApplication("/waveforms/CommandWrapperSPDDep_collide/CommandWrapperSPDDep_collide.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 2)
        self.assertEqual(len(domMgr._get_applications()), 1)
        appFact_2 = domMgr._get_applicationFactories()[1]
        app_2 = appFact_2.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 2)
        app.releaseObject()
        app_2.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        domMgr.uninstallApplication(appFact_2._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

    def test_softpkgDependency_CppDev(self):
        dommgr_nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CommandWrapperSPDDep/CommandWrapperSPDDep.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

    def test_CppsoftpkgDependency_PyDev(self):
        dommgr_nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CppsoftpkgDep/CppsoftpkgDep.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

    def test_CppsoftpkgDependency_CppDev(self):
        dommgr_nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CppsoftpkgDep/CppsoftpkgDep.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

    def test_CppsoftpkgDependency_CppDev_with_hostcollocation(self):
        dommgr_nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CppsoftpkgDep_with_hostcollocation/CppsoftpkgDep_with_hostcollocation.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

    @scatest.requireJava
    def test_JavasoftpkgDependency(self):
        dommgr_nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/JavasoftpkgDep/JavasoftpkgDep.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

    @scatest.requireJava
    def test_JavasoftpkgDependency_CppDev(self):
        dommgr_nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/JavasoftpkgDep/JavasoftpkgDep.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

    def test_deviceManagerDeath(self):
        dommgr_nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        devmgr2_nb, devMgr2 = self.launchDeviceManager("/nodes/test_BasicTestDevice2_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 2)
        self.assertEqual(len(devMgr2._get_registeredDevices()), 1)
        device2 = devMgr2._get_registeredDevices()[0]

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(),
                             [CF.DataType(id="DCE:6ad84383-49cf-4017-b7ca-0ec4c4917952", value=any.to_any(1.5)),
                              CF.DataType(id="SOMEOBJREF", value=any.to_any(self._orb.object_to_string(appFact)))],
                             [])

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        app.stop()
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Kill the device as brutally as possible
        #os.kill(devmgr_nb.pid, signal.SIGKILL)
        pids = getChildren(devmgr_nb.pid)
        for devpid in pids:
            os.kill(devpid, signal.SIGKILL)

        for i in xrange(10):
            if len(devMgr._get_registeredDevices()) ==  0:
                break
            time.sleep(1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 0)
        self.assertEqual(len(devMgr2._get_registeredDevices()), 1)

        # Try again
        app = appFact.create(appFact._get_name(),
                             [CF.DataType(id="DCE:6ad84383-49cf-4017-b7ca-0ec4c4917952", value=any.to_any(1.5)),
                              CF.DataType(id="SOMEOBJREF", value=any.to_any(self._orb.object_to_string(appFact)))],
                             [])
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        app.stop()
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)


    def test_skipBusyDevices(self):

        # creates a new DomainManager
        nodebooter, domMgr = self.launchDomainManager(debug=5)
        self.assertNotEqual(domMgr, None)

        # checking that I don't have a factory yet (no application)
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # once is generated I need to install an application in order to create an ApplicationFactory
        domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicAlwaysBusyDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        
        nodebooter2, devMgr2 = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr2, None)

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(),[],[])

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        invalid_node_id = "DCE:8f3478e3-626e-45c3-bd01-0a8117dbe59f"
        valid_node_id = "DCE:8f3478e3-626e-45c3-bd01-0a8117dbe59b"
        devices = app._get_componentDevices()

        # making sure it has only one component
        self.assertEqual(len(devices), 1)
        dev_id = devices[0].assignedDeviceId

        # checking the id to assure is NOT the busy one
        self.assertEqual(dev_id, valid_node_id)

        # Stop and release the application
        app.stop()
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)


    def test_MyPlayground(self):

        # creates a new DomainManager
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        # checking that I don't have a factory yet (no application)
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # once is generated I need to install an application in order to create an ApplicationFactory
        domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicAlwaysBusyDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        
        nodebooter2, devMgr2 = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr2, None)

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(),[],[])

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        invalid_node_id = "DCE:8f3478e3-626e-45c3-bd01-0a8117dbe59f"
        valid_node_id = "DCE:8f3478e3-626e-45c3-bd01-0a8117dbe59b"
        devices = app._get_componentDevices()

        # making sure it has only one component
        self.assertEqual(len(devices), 1)
        dev_id = devices[0].assignedDeviceId

        # checking the id to assure is NOT the busy one
        self.assertEqual(dev_id, valid_node_id)

        app.start()

        # Stop and release the application
        app.stop()
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)


    def test_structAllocationProps(self):
        # Test that struct properties can be used for allocation, both via
        # component dependencies and 'usesdevice' relationships.
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_MultipleBasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/StructAllocTest/StructAllocTest.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]
        
        appmgr = domMgr._get_allocationMgr()

        app = appFact.create(appFact._get_name(), [], [])
        #allocations = appmgr.allocations([])
        #self.assertEqual(len(appmgr.allocations([])), 1)

        # StructAllocTest has a dependency and usesdevice that both reference
        # the same struct allocation property; in aggregate, the required
        # capacity exceeds the maximum of any one device, so the assigned devices
        # must differ.
        devs = app._get_componentDevices()
        self.assertEqual(devs[0].componentId, devs[1].componentId)
        self.assertNotEqual(devs[0].assignedDeviceId, devs[1].assignedDeviceId)

        app.stop()
        app.releaseObject()        
        self.assertEqual(len(appmgr.allocations([])), 0)

    def test_dontConfigureNilProps(self):
        # Test that struct properties can be used for allocation, both via
        # component dependencies and 'usesdevice' relationships.
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/TestPythonPropsNoDefaults/TestPythonProps.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], [])

        props = app.query([])
        props = dict( [(p.id, any.from_any(p.value)) for p in props] )
        self.assertNotEqual(props["DCE:b8f43ac8-26b5-40b3-9102-d127b84f9e4b"], None)
        self.assertNotEqual(props["DCE:10b3364d-f035-4639-8e7f-02ac4706f5c7[]"], None)
        self.assertNotEqual(props["DCE:ffe634c9-096d-425b-86cc-df1cce50612f"], None)
        self.assertNotEqual(props["DCE:897a5489-f680-46a8-a698-e36fd8bbae80[]"], None)

        app.stop()
        app.releaseObject()

    def test_noStartOrder(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CommandWrapperStartOrderTests/CommandWrapperWithoutOrder.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], [])

        count = 0
        for x in app._get_registeredComponents():
            props = x.componentObject.query([])
            props = dict( [(p.id, any.from_any(p.value)) for p in props] )
            if props["startCounter"] == 1:
                count = count + 1
        self.assertEqual(count, 0)

        app.start()

        props = app.query([])
        props = dict( [(p.id, any.from_any(p.value)) for p in props] )
        self.assertEqual(props["startCounter"], 1)

        count = 0
        for x in app._get_registeredComponents():
            props = x.componentObject.query([])
            props = dict( [(p.id, any.from_any(p.value)) for p in props] )
            if props["startCounter"] == 1:
                count = count + 1
        self.assertEqual(count, 1)

        app.stop()
        app.releaseObject()

    def test_normalStartOrder(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CommandWrapperStartOrderTests/CommandWrapperWithOrder.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], [])

        count = 0
        for x in app._get_registeredComponents():
            props = x.componentObject.query([])
            props = dict( [(p.id, any.from_any(p.value)) for p in props] )
            if props["startCounter"] == 1:
                count = count + 1
        self.assertEqual(count, 0)

        app.start()

        count = 0
        for x in app._get_registeredComponents():
            props = x.componentObject.query([])
            props = dict( [(p.id, any.from_any(p.value)) for p in props] )
            if props["startCounter"] == 1:
                count = count + 1
        self.assertEqual(count, 4)

        app.stop()
        app.releaseObject()

    def test_badStartOrder(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/CommandWrapperStartOrderTests/CommandWrapperWithBadOrder.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], [])

        count = 0
        for x in app._get_registeredComponents():
            props = x.componentObject.query([])
            props = dict( [(p.id, any.from_any(p.value)) for p in props] )
            if props["startCounter"] == 1:
                count = count + 1
        self.assertEqual(count, 0)

        app.start()

        count = 0
        for x in app._get_registeredComponents():
            props = x.componentObject.query([])
            props = dict( [(p.id, any.from_any(p.value)) for p in props] )
            if props["startCounter"] == 1:
                count = count + 1
        self.assertEqual(count, 3)

        app.stop()
        app.releaseObject()

    def test_sadULongLongPropertyOverride(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/ulonglong_override/ulonglong_override.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Ensure the expected device is available
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(domMgr._get_deviceManagers()), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        appFact = domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], [])

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        # Verify that the ulonglong property has been overridden with the SAD value
        prop = app.query([CF.DataType('my_ulonglong', any.to_any(None))])[0]
        self.assertEqual(any.from_any(prop.value), 1000)

        app.releaseObject()                

        domMgr.uninstallApplication(appFact._get_identifier())

    def test_OrphanProcesses(self):
        """
        Tests that all child processes associated with a component get
        terminated with that component.
        """
        nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        nb, devMgr = self.launchDeviceManager('/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml')
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication('/waveforms/orphaned_child/orphaned_child.sad.xml')

        appFact = domMgr._get_applicationFactories()[0]
        self.assertEqual(appFact._get_name(), 'orphaned_child')

        app = appFact.create(appFact._get_name(), [], [])
        pid = app._get_componentProcessIds()[0].processId
        children = [int(line) for line in commands.getoutput('ps --ppid %d --no-headers -o pid' % (pid,)).split()]

        app.releaseObject()

        orphans = 0
        for pid in children:
            try:
                os.kill(pid, 9)
                orphans += 1
            except OSError:
                pass

        self.assertEqual(orphans, 0)

    def test_CppGppOrphanProcesses(self):
        """
        Tests that all child processes associated with a component get
        terminated with that component.
        """
        nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        nb, devMgr = self.launchDeviceManager('/nodes/test_GPP_node/DeviceManager.dcd.xml')
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication('/waveforms/orphaned_child/orphaned_child.sad.xml')

        appFact = domMgr._get_applicationFactories()[0]
        self.assertEqual(appFact._get_name(), 'orphaned_child')

        app = appFact.create(appFact._get_name(), [], [])
        pid = app._get_componentProcessIds()[0].processId
        children = [int(line) for line in commands.getoutput('ps --ppid %d --no-headers -o pid' % (pid,)).split()]

        app.releaseObject()

        orphans = 0
        for pid in children:
            try:
                os.kill(pid, 9)
                orphans += 1
            except OSError:
                pass

        self.assertEqual(orphans, 0)

    def test_HangingStopAndRelease(self):
        """
        Tests that all child processes associated with a component get
        terminated with that component.
        """
        nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        nb, devMgr = self.launchDeviceManager('/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml')
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication('/waveforms/hang_release/hang_release.sad.xml')

        appFact = domMgr._get_applicationFactories()[0]
        self.assertEqual(appFact._get_name(), 'hang_release')

        app = appFact.create(appFact._get_name(), [], [])

        timeout=40 # this is in seconds
        omniORB.setClientCallTimeout(app,timeout*1000)
        app.releaseObject()
        apps = domMgr._get_applications()
        self.assertEqual(len(apps),0)

    def _releaseThread(self, app):
        # Wait for all of the threads to get kicked off.
        timeout=40 # this is in seconds
        omniORB.setClientCallTimeout(app,timeout*1000)
        app.releaseObject()

    def test_CallsWhileRelease(self):
        """
        Tests that all child processes associated with a component get
        terminated with that component.
        """
        nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        nb, devMgr = self.launchDeviceManager('/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml')
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication('/waveforms/hang_release/hang_release.sad.xml')

        appFact = domMgr._get_applicationFactories()[0]
        self.assertEqual(appFact._get_name(), 'hang_release')

        app = appFact.create(appFact._get_name(), [], [])

        #timeout=40 # this is in seconds
        #omniORB.setClientCallTimeout(app,timeout*1000)
        release_thread = threading.Thread(target=self._releaseThread, args=(app,))
        release_thread.start()
        time.sleep(3.5)
        app.start()
        app.stop()
        comps = app._get_registeredComponents()
        ncs = app._get_componentNamingContexts()
        cpids = app._get_componentProcessIds()
        cdevs = app._get_componentDevices()
        cimpls = app._get_componentImplementations()
        app.initialize()
        app.runTest(0,[])
        app.initializeProperties([])
        
        release_thread.join()

        apps = domMgr._get_applications()
        self.assertEqual(len(apps),0)

    def test_NoDefaultExecParam(self):
        """
        Tests that all child processes associated with a component get
        terminated with that component.
        """
        nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        nb, devMgr = self.launchDeviceManager('/nodes/test_GPP_node/DeviceManager.dcd.xml')
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication('/waveforms/execcheck_w/execcheck_w.sad.xml')

        appFact = domMgr._get_applicationFactories()[0]
        self.assertEqual(appFact._get_name(), 'execcheck_w')

        app = appFact.create(appFact._get_name(), [], [])
        comp_pid = int(app._get_componentProcessIds()[0].processId)
        fp=open('/proc/'+str(comp_pid)+'/cmdline','r')
        comp_contents = fp.read()
        fp.close()
        items = comp_contents.split('\x00')
        self.assertEqual('b' in items, False)
        self.assertEqual('Kind: 0' in items, False)

        app.releaseObject()
        apps = domMgr._get_applications()
        self.assertEqual(len(apps),0)

    def test_StopAllComponents(self):
        nb, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)

        nb, devMgr = self.launchDeviceManager('/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml', debug=self.debuglevel)
        self.assertNotEqual(devMgr, None)

        # Create the application, which is pre-configured such that the last
        # component in the start order (and hence, first in stop) will fail on
        # stop().
        domMgr.installApplication('/waveforms/fail_stop/fail_stop.sad.xml')
        appFact = domMgr._get_applicationFactories()[0]
        self.assertEqual(appFact._get_name(), 'fail_stop')
        app = appFact.create(appFact._get_name(), [], [])
        app.start()

        # Pre-condition: check that all the components are started
        components = app._get_registeredComponents()
        for comp in components:
            self.assertTrue(comp.componentObject._get_started())

        self.assertRaises(CF.Resource.StopError, app.stop)

        # Count the number of components that still report as started to make
        # sure that the rest were stopped
        started_count = 0
        for comp in components:
            if comp.componentObject._get_started():
                started_count += 1
        self.assertEqual(started_count, len(components) - 1)

        # Turn off fail on stop so that releaseObject() doesn't complain
        props = [CF.DataType('fail_stop', any.to_any(False))]
        for comp in components:
            comp.componentObject.configure(props)
        app.releaseObject()

    def test_StopTimeout(self):
        nb, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)

        nb, devMgr = self.launchDeviceManager('/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml', debug=self.debuglevel)
        self.assertNotEqual(devMgr, None)

        # Create the application, which is pre-configured such that the last
        # component in the start order (and hence, first in stop) will fail on
        # stop().
        app = domMgr.createApplication('/waveforms/long_stop/long_stop.sad.xml', 'long_stop', [], [])
        app.start()
        begin_stop = time.time()
        try:
            app.stop()
        except:
            pass
        end_stop = time.time()
        stop_time = end_stop-begin_stop
        app.releaseObject()
        end_release = time.time()
        release_time = end_release-end_stop
        self.assertTrue(16<=stop_time<=17)
        self.assertTrue(20<=release_time<=22)

    def test_StopTimeoutBuiltinDefault(self):
        nb, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)

        nb, devMgr = self.launchDeviceManager('/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml', debug=self.debuglevel)
        self.assertNotEqual(devMgr, None)

        # Create the application, which is pre-configured such that the last
        # component in the start order (and hence, first in stop) will fail on
        # stop().
        app = domMgr.createApplication('/waveforms/long_stop_builtin_def/long_stop_builtin_def.sad.xml', 'long_stop', [], [])
        app.start()
        begin_stop = time.time()
        try:
            app.stop()
        except:
            pass
        end_stop = time.time()
        stop_time = end_stop-begin_stop
        app.releaseObject()
        end_release = time.time()
        release_time = end_release-end_stop
        self.assertTrue(6<=stop_time<=7)
        self.assertTrue(20<=release_time<=22)

    def test_StopTimeoutLiveOverride(self):
        nb, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)

        nb, devMgr = self.launchDeviceManager('/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml', debug=self.debuglevel)
        self.assertNotEqual(devMgr, None)

        # Create the application, which is pre-configured such that the last
        # component in the start order (and hence, first in stop) will fail on
        # stop().
        initconfig = [CF.DataType(id=ExtendedCF.WKP.STOP_TIMEOUT, value=any.to_any(4))]
        app = domMgr.createApplication('/waveforms/long_stop/long_stop.sad.xml', 'long_stop', initconfig, [])
        app.start()
        begin_stop = time.time()
        try:
            app.stop()
        except:
            pass
        end_stop = time.time()
        stop_time = end_stop-begin_stop
        app.releaseObject()
        end_release = time.time()
        release_time = end_release-end_stop
        self.assertTrue(8<=stop_time<=9)
        self.assertTrue(20<=release_time<=22)

    def test_StopTimeoutChange(self):
        nb, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)

        nb, devMgr = self.launchDeviceManager('/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml', debug=self.debuglevel)
        self.assertNotEqual(devMgr, None)

        # Create the application, which is pre-configured such that the last
        # component in the start order (and hence, first in stop) will fail on
        # stop().
        initconfig = [CF.DataType(id=ExtendedCF.WKP.STOP_TIMEOUT, value=any.to_any(4))]
        app = domMgr.createApplication('/waveforms/long_stop/long_stop.sad.xml', 'long_stop', initconfig, [])
        app.start()
        curr_stoptimeout = app._get_stopTimeout()
        self.assertEquals(curr_stoptimeout, 4.0)
        app._set_stopTimeout(5)
        begin_stop = time.time()
        try:
            app.stop()
        except:
            pass
        end_stop = time.time()
        stop_time = end_stop-begin_stop
        app.releaseObject()
        end_release = time.time()
        release_time = end_release-end_stop
        self.assertTrue(10<=stop_time<=11)
        self.assertTrue(20<=release_time<=22)

    def test_StopTimeoutIndefinite(self):
        nb, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)

        nb, devMgr = self.launchDeviceManager('/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml', debug=self.debuglevel)
        self.assertNotEqual(devMgr, None)

        # Create the application, which is pre-configured such that the last
        # component in the start order (and hence, first in stop) will fail on
        # stop().
        app = domMgr.createApplication('/waveforms/slow_stop_w/slow_stop_w.sad.xml', 'slow_stop', [], [])
        app.start()
        curr_stoptimeout = app._get_stopTimeout()
        self.assertEquals(curr_stoptimeout, -1)
        begin_stop = time.time()
        try:
            app.stop()
        except:
            pass
        end_stop = time.time()
        stop_time = end_stop-begin_stop
        app.releaseObject()
        end_release = time.time()
        release_time = end_release-end_stop
        self.assertTrue(5<=stop_time<=6)
        self.assertTrue(8<=release_time<=9)

    def _test_ValgrindCppDevice(self, appFact, valgrind):
        # Clear the device cache to prevent false positives
        deviceCacheDir = os.path.join(scatest.getSdrCache(), ".ExecutableDevice_node", "ExecutableDevice1")
        shutil.rmtree(deviceCacheDir, ignore_errors=True)

        os.environ['VALGRIND'] = valgrind
        try:
            nb, devMgr = self.launchDeviceManager('/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml')
        finally:
            del os.environ['VALGRIND']
        self.assertNotEqual(devMgr, None)

        # Create the application and create a lookup table of component IDs to
        # PIDs, then check that there is a log file for that PID in the device
        # cache next to the profile (this only works as long as the entry point
        # is in the same directory as the XML profile)
        app = appFact.create(appFact._get_name(), [], [])
        components = dict((c.componentId, c.processId) for c in app._get_componentProcessIds())
        for comp in app._get_registeredComponents():
            # The software profile starts with '/' (because it's relative to
            # SDRROOT), so use concatenation instead of os.path.join()
            cacheDir = deviceCacheDir + os.path.dirname(comp.softwareProfile)
            logfile = os.path.join(cacheDir, 'valgrind.%d.log' % components[comp.identifier])
            self.assertTrue(os.path.exists(logfile))

        app.releaseObject()

        devMgr.shutdown()

    def test_ValgrindCppDevice(self):
        """
        Checks that the 'VALGRIND' environment variable can be used to run
        components launched by an ExecutableDevice with Valgrind.
        """
        # Make sure that valgrind exists and is in the path
        valgrind = scatest.which('valgrind')
        if not valgrind:
            raise RuntimeError('Valgrind is not installed')

        nb, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        # For best results, use an application with C++ components
        domMgr.installApplication("/waveforms/CppsoftpkgDep/CppsoftpkgDep.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]

        # Let the executable device find valgrind on the path
        self._test_ValgrindCppDevice(appFact, '')

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
            self._test_ValgrindCppDevice(appFact, altpath)
        finally:
            os.unlink(altpath)
            if ub_patch:
                os.unlink(altpath+'.bin')

