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
    
def execute(self, spd, impl, execparams, timeout=None):
    # Find a suitable implementation.
    if impl:
        implementation = self._getImplementation(spd, impl)
    else:
        implementation = self._selectImplementation(spd)

    # Make sure the entry point can be run.
    entry_point = self._getEntryPoint(implementation)
    if not os.access(entry_point, os.X_OK|os.R_OK):
        raise RuntimeError, "Entry point '%s' is not executable" % entry_point

    # Process softpkg dependencies and modify the child environment.
    environment = dict(os.environ.items())

    # Get required execparams based on the component type
    execparams.update(self._getRequiredExecparams())

    # Convert execparams into arguments.
    arguments = []
    for name, value in execparams.iteritems():
        arguments += [name, str(value)]

    # Run the command directly.
    command = entry_point
    default_timeout = 10.0

    # Provided timeout takes precedence
    if timeout is None:
        timeout = default_timeout

    stdout = None
    process = LocalProcess(command, arguments, environment, stdout)

    # Wait for the component to register with the virtual naming service or
    # DeviceManager.
    sleepIncrement = 0.1
    while self.getReference() is None:
        if not process.isAlive():
            raise RuntimeError, "%s '%s' terminated before registering with virtual environment" % (self._getType(), self._name)
        time.sleep(sleepIncrement)
        timeout -= sleepIncrement
        if timeout < 0:
            process.terminate()
            raise RuntimeError, "%s '%s' did not register with virtual environment"  % (self._getType(), self._name)

    # Store the CORBA reference.
    ref = self.getReference()

    return process, ref

class ApplicationRegistrarTest(scatest.CorbaTestCase):
    def setUp(self):
        pass # Nothing to do

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)

    def _getImplementation(self, spd, identifier):
        for implementation in spd.get_implementation():
            if implementation.get_id() == identifier:
                return implementation
        raise KeyError, "Softpkg '%s' has no implementation '%s'" % (spd.get_name(), identifier)
    def _getEntryPoint(self, implementation):
        entry_point = implementation.get_code().get_entrypoint()
        if not entry_point.startswith('/'):
            entry_point = os.path.join(self._xmlpath, entry_point)
        return entry_point
    def _getRequiredExecparams(self):
        return {'COMPONENT_IDENTIFIER': 'some_id',
                'NAMING_CONTEXT_IOR': self.orb.object_to_string(self.__namingContext._this()),
                'PROFILE_NAME': self._profile,
                'NAME_BINDING': self._name}
    def getReference(self):
        return self.__namingContext.getObject(self._name)
    
    def setupStandalone(self, profile, name, impl):
        self.orb = CORBA.ORB_init()
        self.poa = self.orb.resolve_initial_references("RootPOA")
        self.poa._get_the_POAManager().activate()
        self.__namingContext = NamingContextStub()
        self.__namingContextId = self.poa.activate_object(self.__namingContext)
        self._profile = profile
        self._name = name
        self._xmlpath = os.path.dirname(self._profile)
        self.spd = parsers.spd.parse(self._profile)
        self.impl = impl

    def test_cppNamingContextOnly(self):
        self.setupStandalone('sdr/dom/components/cpp_comp/cpp_comp.spd.xml', 'cpp_comp', 'cpp')
        process,ref = execute(self, self.spd, self.impl, {})
        pid = process.pid()
        comp_id = ref._get_identifier()
        self.assertEquals(comp_id, 'some_id')
        ref.start()
        time.sleep(0.5)
        app_id = ref.query([CF.DataType(id='app_id',value=any.to_any(None))])[0].value._v
        number_components = ref.query([CF.DataType(id='number_components',value=any.to_any(None))])[0].value._v
        dom_id = ref.query([CF.DataType(id='dom_id',value=any.to_any(None))])[0].value._v
        self.assertEqual(app_id, "")
        self.assertEqual(number_components, -2)
        self.assertEqual(dom_id, "")
        os.kill(pid, 2)

    def test_pythonNamingContextOnly(self):
        self.setupStandalone('sdr/dom/components/py_comp/py_comp.spd.xml', 'py_comp', 'python')
        process,ref = execute(self, self.spd, self.impl, {})
        pid = process.pid()
        comp_id = ref._get_identifier()
        self.assertEquals(comp_id, 'some_id')
        ref.start()
        time.sleep(0.5)
        app_id = ref.query([CF.DataType(id='app_id',value=any.to_any(None))])[0].value._v
        number_components = ref.query([CF.DataType(id='number_components',value=any.to_any(None))])[0].value._v
        dom_id = ref.query([CF.DataType(id='dom_id',value=any.to_any(None))])[0].value._v
        self.assertEqual(app_id, "")
        self.assertEqual(number_components, -2)
        self.assertEqual(dom_id, "")
        os.kill(pid, 2)

    def test_javaNamingContextOnly(self):
        if not java_support:
            return
        self.setupStandalone('sdr/dom/components/java_comp/java_comp.spd.xml', 'java_comp', 'java')
        process,ref = execute(self, self.spd, self.impl, {})
        pid = process.pid()
        comp_id = ref._get_identifier()
        self.assertEquals(comp_id, 'some_id')
        ref.start()
        time.sleep(0.5)
        app_id = ref.query([CF.DataType(id='app_id',value=any.to_any(None))])[0].value._v
        number_components = ref.query([CF.DataType(id='number_components',value=any.to_any(None))])[0].value._v
        dom_id = ref.query([CF.DataType(id='dom_id',value=any.to_any(None))])[0].value._v
        self.assertEqual(app_id, "")
        self.assertEqual(number_components, -2)
        self.assertEqual(dom_id, "")
        os.kill(pid, 2)

    def test_cppCompBasic(self):
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
        app_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='app_id',value=any.to_any(None))])[0].value._v
        number_components = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='number_components',value=any.to_any(None))])[0].value._v
        dom_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='dom_id',value=any.to_any(None))])[0].value._v
        self.assertEqual(app_id, app._get_identifier())
        self.assertEqual(number_components, 1)
        self.assertEqual(dom_id, domMgr._get_identifier())
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_pyCompBasic(self):
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
        app_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='app_id',value=any.to_any(None))])[0].value._v
        number_components = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='number_components',value=any.to_any(None))])[0].value._v
        dom_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='dom_id',value=any.to_any(None))])[0].value._v
        self.assertEqual(app_id, app._get_identifier())
        self.assertEqual(number_components, 1)
        self.assertEqual(dom_id, domMgr._get_identifier())
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_javaCompBasic(self):
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
        app_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='app_id',value=any.to_any(None))])[0].value._v
        number_components = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='number_components',value=any.to_any(None))])[0].value._v
        dom_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='dom_id',value=any.to_any(None))])[0].value._v
        self.assertEqual(app_id, app._get_identifier())
        self.assertEqual(number_components, 1)
        self.assertEqual(dom_id, domMgr._get_identifier())
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_cppDevBasic(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/cpp_dev_n/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        devmgr_id = devMgr._get_registeredDevices()[0].query([CF.DataType(id='devmgr_id',value=any.to_any(None))])[0].value._v
        dom_id = devMgr._get_registeredDevices()[0].query([CF.DataType(id='dom_id',value=any.to_any(None))])[0].value._v
        self.assertEqual(devmgr_id, devMgr._get_identifier())
        self.assertEqual(dom_id, domMgr._get_identifier())

    def test_pyDevBasic(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/py_dev_n/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        devmgr_id = devMgr._get_registeredDevices()[0].query([CF.DataType(id='devmgr_id',value=any.to_any(None))])[0].value._v
        dom_id = devMgr._get_registeredDevices()[0].query([CF.DataType(id='dom_id',value=any.to_any(None))])[0].value._v
        self.assertEqual(devmgr_id, devMgr._get_identifier())
        self.assertEqual(dom_id, domMgr._get_identifier())

    def test_javaDevBasic(self):
        if not java_support:
            return
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/java_dev_n/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        devmgr_id = devMgr._get_registeredDevices()[0].query([CF.DataType(id='devmgr_id',value=any.to_any(None))])[0].value._v
        dom_id = devMgr._get_registeredDevices()[0].query([CF.DataType(id='dom_id',value=any.to_any(None))])[0].value._v
        self.assertEqual(devmgr_id, devMgr._get_identifier())
        self.assertEqual(dom_id, domMgr._get_identifier())

    def test_cppDevDomainless(self):
        dev = sb.launch('sdr/dev/devices/cpp_dev/cpp_dev.spd.xml')
        self.assertEqual(dev.devmgr_id, "")
        self.assertEqual(dev.dom_id, "")
        dev.releaseObject()

    def test_pyDevDomainless(self):
        dev = sb.launch('sdr/dev/devices/py_dev/py_dev.spd.xml')
        self.assertEqual(dev.devmgr_id, "")
        self.assertEqual(dev.dom_id, "")
        dev.releaseObject()

    def test_javaDevDomainless(self):
        if not java_support:
            return
        dev = sb.launch('sdr/dev/devices/java_dev/java_dev.spd.xml')
        self.assertEqual(dev.devmgr_id, "")
        self.assertEqual(dev.dom_id, "")
        dev.releaseObject()

    def test_cppCompUnaware(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/cpp_comp_w/cpp_comp_w.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]
        
        initconfig = [CF.DataType(id=ExtendedCF.WKP.AWARE_APPLICATION, value=any.to_any(False))]

        app = appFact.create(appFact._get_name(), initconfig, [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.start()
        time.sleep(0.5)
        app_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='app_id',value=any.to_any(None))])[0].value._v
        number_components = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='number_components',value=any.to_any(None))])[0].value._v
        dom_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='dom_id',value=any.to_any(None))])[0].value._v
        self.assertEqual(app_id, app._get_identifier())
        self.assertEqual(number_components, 0)
        self.assertEqual(dom_id, "")
        self.assertEqual(app._get_aware(), False)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_pyCompUnaware(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/py_comp_w/py_comp_w.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]
        
        initconfig = [CF.DataType(id=ExtendedCF.WKP.AWARE_APPLICATION, value=any.to_any(False))]

        app = appFact.create(appFact._get_name(), initconfig, [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.start()
        time.sleep(0.5)
        app_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='app_id',value=any.to_any(None))])[0].value._v
        number_components = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='number_components',value=any.to_any(None))])[0].value._v
        dom_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='dom_id',value=any.to_any(None))])[0].value._v
        self.assertEqual(app_id, app._get_identifier())
        self.assertEqual(number_components, 0)
        self.assertEqual(dom_id, "")
        self.assertEqual(app._get_aware(), False)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_javaCompUnaware(self):
        if not java_support:
            return
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/java_comp_w/java_comp_w.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]
        
        initconfig = [CF.DataType(id=ExtendedCF.WKP.AWARE_APPLICATION, value=any.to_any(False))]

        app = appFact.create(appFact._get_name(), initconfig, [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.start()
        time.sleep(0.5)
        app_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='app_id',value=any.to_any(None))])[0].value._v
        number_components = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='number_components',value=any.to_any(None))])[0].value._v
        dom_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='dom_id',value=any.to_any(None))])[0].value._v
        self.assertEqual(app_id, app._get_identifier())
        self.assertEqual(number_components, 0)
        self.assertEqual(dom_id, "")
        self.assertEqual(app._get_aware(), False)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_cppCompaware(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/cpp_comp_w/cpp_comp_w.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]
        
        initconfig = [CF.DataType(id=ExtendedCF.WKP.AWARE_APPLICATION, value=any.to_any(True))]

        app = appFact.create(appFact._get_name(), initconfig, [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.start()
        time.sleep(0.5)
        app_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='app_id',value=any.to_any(None))])[0].value._v
        dom_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='dom_id',value=any.to_any(None))])[0].value._v
        self.assertEqual(app_id, app._get_identifier())
        self.assertEqual(dom_id, domMgr._get_identifier())
        self.assertEqual(app._get_aware(), True)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_pyCompAware(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/py_comp_w/py_comp_w.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]
        
        initconfig = [CF.DataType(id=ExtendedCF.WKP.AWARE_APPLICATION, value=any.to_any(True))]

        app = appFact.create(appFact._get_name(), initconfig, [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.start()
        time.sleep(0.5)
        app_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='app_id',value=any.to_any(None))])[0].value._v
        dom_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='dom_id',value=any.to_any(None))])[0].value._v
        self.assertEqual(app_id, app._get_identifier())
        self.assertEqual(dom_id, domMgr._get_identifier())
        self.assertEqual(app._get_aware(), True)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_javaCompAware(self):
        if not java_support:
            return
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/java_comp_w/java_comp_w.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]
        
        initconfig = [CF.DataType(id=ExtendedCF.WKP.AWARE_APPLICATION, value=any.to_any(True))]

        app = appFact.create(appFact._get_name(), initconfig, [])
        self.assertEqual(len(domMgr._get_applications()), 1)
        app.start()
        time.sleep(0.5)
        app_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='app_id',value=any.to_any(None))])[0].value._v
        dom_id = app._get_registeredComponents()[0].componentObject.query([CF.DataType(id='dom_id',value=any.to_any(None))])[0].value._v
        self.assertEqual(app_id, app._get_identifier())
        self.assertEqual(dom_id, domMgr._get_identifier())
        self.assertEqual(app._get_aware(), True)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applications()), 0)
