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

import unittest, os, signal, commands, shutil
from _unitTestHelpers import scatest
from xml.dom import minidom
from ossie.cf import CF
import time
from omniORB import URI, any
import CosNaming
import time
import CosEventComm, CosEventComm__POA
import CosEventChannelAdmin, CosEventChannelAdmin__POA
from ossie.cf import StandardEvent
from ossie.events import ChannelManager

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

# create a class for consuming events
class Consumer_i(CosEventComm__POA.PushConsumer):
    def __init__(self, parent):
        #self.event = threading.Event()
        self.parent = parent

    def push(self, data_obj):
        data = data_obj.value()
        self.parent.eventReceived(data_obj)

    def disconnect_push_consumer (self):
        pass

class DomainManagerTest(scatest.CorbaTestCase):
    def setUp(self):
        nodebooter, self._domMgr = self.launchDomainManager()
    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)
        killChildProcesses(os.getpid())

    def eventReceived(self, data):
            self.gotData = True

    def test_NSCleanup(self):
        domain_name = self._domMgr._get_name()
        ns_domMgr = URI.stringToName("%s/%s" % (domain_name, domain_name))
        ns_domMgrCtx = URI.stringToName("%s" % (domain_name))
        ns_ODM = URI.stringToName("%s/%s" % (domain_name, "ODM_Channel"))
        ns_IDM = URI.stringToName("%s/%s" % (domain_name, "IDM_Channel"))
        domCtx_ref = self._root.resolve(ns_domMgrCtx)
        domMgr_ref = self._root.resolve(ns_domMgr)
        ODM_ref = self._root.resolve(ns_ODM)
        IDM_ref = self._root.resolve(ns_IDM)
        self.assertNotEqual(domCtx_ref, None)
        self.assertNotEqual(domMgr_ref, None)
        self.assertNotEqual(ODM_ref, None)
        self.assertNotEqual(IDM_ref, None)
        os.kill(self._domainBooter.pid, signal.SIGINT)
        self.waitTermination(self._domainBooter)
        self.assertRaises(CosNaming.NamingContext.NotFound, self._root.resolve, ns_domMgrCtx)

    def test_DeviceFailure(self):
        self.assertNotEqual(self._domMgr, None)
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_PythonNodeNoUpdateUsageState_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 0)

        # Test that the DCD file componentproperties get pushed to configure()
        # as per DeviceManager requirement SR:482
        devMgr.shutdown()

        self.assert_(self.waitTermination(devmgr_nb), "Nodebooter did not die after shutdown")
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)
        self.assertNotEqual(self._domMgr, None)
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)


    def test_DomainManagerPropertyOverride(self):
        # in order to test the nodeBooter execparam first we need to set the execparams
        self.tearDown()
        print "Waiting to give tearDown some time"
        time.sleep(2)

        # the id used to set the COMPONENT_BINDING_TIMEOUT
        propId = 'COMPONENT_BINDING_TIMEOUT'
        # the '--' is required to tell the nodeBooter that the following arguments are execparams
        execparams = ['--', propId,'120']
        self._execparams = " ".join(execparams)
        self.setUp()

        prop = CF.DataType(id=propId, value=any.to_any(None))
        result = self._domainManager.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, propId)
        # This should have been overrided by the dcd
        self.assertEqual(result[0].value._v, 120)

    def test_DomainManagerFileMgr(self):
        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._domMgr._get_fileMgr(), None, msg="Violation of SR:210 and/or SR:219")

    def test_DomainManagerApplicationLifecycle(self):
        self.assertNotEqual(self._domMgr, None)
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        # This filename isn't in compliance with SCA, but it is necessary for OSSIE
        self._domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        appFact = self._domMgr._get_applicationFactories()[0]
        dom = minidom.parse(os.path.join(scatest.getSdrPath(), "dom/waveforms/CommandWrapper/CommandWrapper.sad.xml"))
        expectedId = dom.getElementsByTagName("softwareassembly")[0].getAttribute("id")
        providedId = appFact._get_identifier()
        self.assertEqual(providedId, expectedId, msg="Violation of SR:155 and/or SR:156")

        expectedName = dom.getElementsByTagName("softwareassembly")[0].getAttribute("name")
        providedName = appFact._get_name()
        self.assertEqual(providedName, expectedName, msg="Violation of SR:153")

        self._domMgr.uninstallApplication(providedId)
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

    def test_DomainManagerExceptionCase(self):
        return # NOT IMPLEMENDTED CORRECTLY YET
        self.assertRaises(CF.DomainManager.InvalidIdentifier, self._domMgr.uninstallApplication, "DCE:00000000-0000-0000-0000-000000000000")

        # Test SR:269
        self.assertRaises(CF.InvalidFileName, self._domMgr.installApplication, "bad_application.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

    def test_DomainManagerId(self):
        self.assertNotEqual(self._domMgr, None)

        # Load the ID from the XML file
        dom = minidom.parse(os.path.join(scatest.getSdrPath(), "dom/domain/DomainManager.dmd.xml"))
        expectedId = dom.getElementsByTagName("domainmanagerconfiguration")[0].getAttribute("id")
        providedId = self._domMgr._get_identifier()
        self.assertEqual(providedId, expectedId, msg="Violation of SR:213 and/or SR:214")

        # According to SCA section D.8.1, the id is supposed to be a DCE UUID
        self.assertIsDceUUID(expectedId, msg="Violation of SCA D.8.1")

    def test_DomainManagerBadSadFile(self):
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/SimpleDevMgr/DeviceManager.dcd.xml")
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
                # the sad file contains an invalid assembly controller
                # reference ID that should causes an error
                sadpath = "/waveforms/SimpleWaveform/BadSimpleWaveform.sad.xml"
                self._domMgr.installApplication(sadpath)
                self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
                appFact = self._domMgr._get_applicationFactories()[0]
                self._app = appFact.create(appFact._get_name(), [], [])
            except:
                # exception is expected as the SAD file does not contain a
                # valid assembly controller
                pass

            # making sure the domain manager still alive
            self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)

    def test_registerWithEventChannel_creation(self):
        # launch DomainManager
        nodebooter, self._domMgr = self.launchDomainManager()
        self.assertNotEqual(self._domMgr, None)
        self.gotData = False
        # set up consumer
        _consumer = Consumer_i(self)
        channelName = 'testChannel'
        self._domMgr.registerWithEventChannel(_consumer._this(), 'some_id', channelName)
        domainName = scatest.getTestDomainName()
        eventChannelURI = URI.stringToName("%s/%s" % (domainName, channelName))
        channel = self._root.resolve(eventChannelURI)._narrow(CosEventChannelAdmin.EventChannel)
        supplier_admin = channel.for_suppliers()
        proxy_consumer = supplier_admin.obtain_push_consumer()
        proxy_consumer.connect_push_supplier(None)
        proxy_consumer.push(any.to_any(True))
        begin_time = time.time()
        timeout = 5 # maximum of 5 seconds
        while ((time.time() - begin_time) < timeout) and not self.gotData:
            time.sleep(0.1)
        self.assertEqual(self.gotData, True)
        self._domMgr.unregisterFromEventChannel('some_id', channelName)


class DomainManager_ApplicationInstall(scatest.CorbaTestCase):
    def setUp(self):
        self.nodebooter, self._domMgr = self.launchDomainManager()
        self.devmgr_nb, self.devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self.devMgr, None)
        self.wf_name="cpp_deps_wf"
        self.sadfile="/waveforms/"+self.wf_name+"/"+self.wf_name+".sad.xml"
        self.comp_name="cpp_with_deps"

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)
        killChildProcesses(os.getpid())

    def test_installApplicationFailures_sad_invalid(self):
        self.assertRaises( CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, "/waveforms/cpp_deps_wf.OUCH/cpp_deps_wf.sad.xml")

    def test_installApplicationFailures_sad_compref_test1(self):
        wf_name=self.wf_name
        sadfile=self.sadfile
        
        ## copy bad compref test 1 to sad file
        wf_sad=scatest.getSdrPath()+"/dom"+sadfile
        shutil.copy( wf_sad+".TEST.bad.compref", wf_sad)
        self.assertRaises( CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, sadfile)

        ## reset sad file
        shutil.copy( wf_sad+".ORIG", wf_sad)

    def test_installApplicationFailures_sad_compref_test2(self):
        wf_name=self.wf_name
        sadfile=self.sadfile
        
        ## copy bad compref test 1 to sad file
        wf_sad=scatest.getSdrPath()+"/dom"+sadfile
        shutil.copy( wf_sad+".TEST.bad.comp.file", wf_sad)
        self.assertRaises( CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, sadfile)

        ## reset sad file
        shutil.copy( wf_sad+".ORIG", wf_sad)


    def test_installApplicationFailures_comp_missing(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        ## remove component spd file
        comp_spd=scatest.getSdrPath()+"/dom/components/cpp_with_deps/cpp_with_deps.spd.xml"
        try:
            os.remove(comp_spd)
        except:
            pass
        self.assertRaises( CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, sadfile)

        ## reset spd file
        shutil.copy(comp_spd+".ORIG", comp_spd)


    def test_installApplicationFailures_comp_missing2(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        ## remove component spd file
        comp_scd=scatest.getSdrPath()+"/dom/components/cpp_with_deps/cpp_with_deps.scd.xml"
        try:
            os.remove(comp_scd)
        except:
            pass
        self.assertRaises( CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, sadfile)

        ## reset file
        shutil.copy(comp_scd+".ORIG", comp_scd)

    def test_installApplicationFailures_comp_missing3(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        ## remove component spd file
        comp_prf=scatest.getSdrPath()+"/dom/components/cpp_with_deps/cpp_with_deps.prf.xml"
        try:
            os.remove(comp_prf)
        except:
            pass
        self.assertRaises( CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, sadfile)

        ## reset file
        shutil.copy(comp_prf+".ORIG", comp_prf)

    def test_installApplicationFailures_comp_bad_prf_file(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        comp_spd=scatest.getSdrPath()+"/dom/components/cpp_with_deps/cpp_with_deps.spd.xml"
        shutil.copy(comp_spd+".TEST.bad.prf", comp_spd)
        self.assertRaises( CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, sadfile)

        ## reset file
        shutil.copy(comp_spd+".ORIG", comp_spd)

    def test_installApplicationFailures_comp_bad_scd_file(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        comp_spd=scatest.getSdrPath()+"/dom/components/cpp_with_deps/cpp_with_deps.spd.xml"
        shutil.copy(comp_spd+".TEST.bad.scd", comp_spd)
        self.assertRaises( CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, sadfile)

        ## reset file
        shutil.copy(comp_spd+".ORIG", comp_spd)

    def test_installApplicationFailures_comp_bad_dep_ref(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        comp_spd=scatest.getSdrPath()+"/dom/components/cpp_with_deps/cpp_with_deps.spd.xml"
        shutil.copy(comp_spd+".TEST.bad.dep", comp_spd)
        self.assertRaises( CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, sadfile)

        ## reset file
        shutil.copy(comp_spd+".ORIG", comp_spd)

    def test_installApplicationFailures_dep_missing(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        dep_dir=scatest.getSdrPath()+"/dom/deps/cpp_dep1"
        shutil.move(dep_dir, dep_dir+".XXX")
        self.assertRaises( CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, sadfile)

        ## reset file
        shutil.move(dep_dir+".XXX", dep_dir)

    def test_installApplicationFailures_dep_missing_dep_dir(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        dep_dir=scatest.getSdrPath()+"/dom/deps/cpp_dep1/cpp"
        shutil.move(dep_dir, dep_dir+".XXX")
        self.assertRaises( CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, sadfile)

        ## reset file
        shutil.move( dep_dir+".XXX", dep_dir)


    def test_installApplicationFailures_dep_bad_rec_dep_file(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        dep_spd=scatest.getSdrPath()+"/dom/deps/cpp_dep1/cpp_dep1.spd.xml"
        shutil.copy(dep_spd+".TEST.bad.rec.dep.file", dep_spd)
        self.assertRaises( CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, sadfile)

        ## reset file
        shutil.copy(dep_spd+".ORIG", dep_spd)


    def test_installApplicationFailures_recdep_missing_dir(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        dep_spd=scatest.getSdrPath()+"/dom/deps/cpp_dep2"
        shutil.move(dep_spd, dep_spd+".XXX")
        self.assertRaises( CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, sadfile)

        ## reset file
        shutil.move(dep_spd+".XXX", dep_spd)

    def test_installApplicationFailures_recdep_missing_file(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        dep_spd=scatest.getSdrPath()+"/dom/deps/cpp_dep2/cpp_dep2.spd.xml"
        shutil.move(dep_spd, dep_spd+".XXX")
        self.assertRaises( CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, sadfile)

        ## reset file
        shutil.move(dep_spd+".XXX", dep_spd)


    def test_installApplicationFailures_recdep_bad_dir(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        dep_spd=scatest.getSdrPath()+"/dom/deps/cpp_dep2/cpp_dep2.spd.xml"
        shutil.copy( dep_spd+".TEST.bad.dir", dep_spd)
        self.assertRaises( CF.DomainManager.ApplicationInstallationError, self._domMgr.installApplication, sadfile)

        ## reset file
        shutil.copy(dep_spd+".ORIG", dep_spd)

    def test_installApplication_recdep_typeiscompliant(self):
        wf_name="cpp_deps_wf2"
        sadfile="/waveforms/"+wf_name+"/"+wf_name+".sad.xml"

        dep_spd=scatest.getSdrPath()+"/dom/deps/cpp_dep2/cpp_dep2.spd.xml"
        shutil.copy( dep_spd+".ORIG", dep_spd)
        self._domMgr.installApplication( sadfile )
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual( appFact, None )
        self._domMgr.uninstallApplication(appFact._get_identifier())

        ## reset file
        shutil.copy(dep_spd+".ORIG", dep_spd)


    def test_installApplication_recdep_typeisnoncompliant(self):
        wf_name="cpp_deps_wf2"
        sadfile="/waveforms/"+wf_name+"/"+wf_name+".sad.xml"

        dep_spd=scatest.getSdrPath()+"/dom/deps/cpp_dep2/cpp_dep2.spd.xml"
        shutil.copy( dep_spd+".NON_COMP", dep_spd)
        self._domMgr.installApplication( sadfile )
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual( appFact, None )
        self._domMgr.uninstallApplication(appFact._get_identifier())

        ## reset file
        shutil.copy(dep_spd+".ORIG", dep_spd)



class DomainManager_CreateApplication(scatest.CorbaTestCase):
    def setUp(self):
        self.nodebooter, self._domMgr = self.launchDomainManager()
        self.devmgr_nb, self.devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self.devMgr, None)
        self.wf_name="cpp_deps_wf"
        self.sadfile="/waveforms/"+self.wf_name+"/"+self.wf_name+".sad.xml"
        self.comp_name="cpp_with_deps"

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)
        killChildProcesses(os.getpid())

    def test_createApplicationFailures_sad_invalid(self):
        self.assertRaises( CF.InvalidProfile, self._domMgr.createApplication, "/waveforms/cpp_deps_wf.OUCH/cpp_deps_wf.sad.xml","",[],[])

    def test_createApplicationFailures_sad_compref_refid(self):
        wf_name=self.wf_name
        sadfile=self.sadfile
        
        ## copy bad compref test 1 to sad filex
        wf_sad=scatest.getSdrPath()+"/dom"+sadfile
        shutil.copy( wf_sad+".TEST.bad.compref", wf_sad)
        self.assertRaises( CF.InvalidProfile, self._domMgr.createApplication, sadfile, "", [], [])

        ## reset sad file
        shutil.copy( wf_sad+".ORIG", wf_sad)

    def test_createApplicationFailures_sad_compref_file(self):
        wf_name=self.wf_name
        sadfile=self.sadfile
        
        ## copy bad compref test 1 to sad file
        wf_sad=scatest.getSdrPath()+"/dom"+sadfile
        shutil.copy( wf_sad+".TEST.bad.comp.file", wf_sad)
        self.assertRaises( CF.InvalidProfile, self._domMgr.createApplication, sadfile, "", [], [])

        ## reset sad file
        shutil.copy( wf_sad+".ORIG", wf_sad)


    def test_createApplicationFailures_comp_missing(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        ## remove component spd file
        comp_spd=scatest.getSdrPath()+"/dom/components/cpp_with_deps/cpp_with_deps.spd.xml"
        try:
            os.remove(comp_spd)
        except:
            pass
        self.assertRaises( CF.InvalidProfile, self._domMgr.createApplication, sadfile, "", [], [])

        ## reset spd file
        shutil.copy(comp_spd+".ORIG", comp_spd)


    def test_createApplicationFailures_comp_missing2(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        ## remove component spd file
        comp_scd=scatest.getSdrPath()+"/dom/components/cpp_with_deps/cpp_with_deps.scd.xml"
        try:
            os.remove(comp_scd)
        except:
            pass
        self.assertRaises( CF.InvalidProfile, self._domMgr.createApplication, sadfile, "", [], [])

        ## reset file
        shutil.copy(comp_scd+".ORIG", comp_scd)

    def test_createApplicationFailures_comp_missing3(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        ## remove component spd file
        comp_prf=scatest.getSdrPath()+"/dom/components/cpp_with_deps/cpp_with_deps.prf.xml"
        try:
            os.remove(comp_prf)
        except:
            pass
        self.assertRaises( CF.InvalidProfile, self._domMgr.createApplication, sadfile, "", [], [])

        ## reset file
        shutil.copy(comp_prf+".ORIG", comp_prf)

    def test_createApplicationFailures_comp_bad_prf_file(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        comp_spd=scatest.getSdrPath()+"/dom/components/cpp_with_deps/cpp_with_deps.spd.xml"
        shutil.copy(comp_spd+".TEST.bad.prf", comp_spd)
        self.assertRaises( CF.InvalidProfile, self._domMgr.createApplication, sadfile, "", [], [])

        ## reset file
        shutil.copy(comp_spd+".ORIG", comp_spd)

    def test_createApplicationFailures_comp_bad_scd_file(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        comp_spd=scatest.getSdrPath()+"/dom/components/cpp_with_deps/cpp_with_deps.spd.xml"
        shutil.copy(comp_spd+".TEST.bad.scd", comp_spd)
        self.assertRaises( CF.InvalidProfile, self._domMgr.createApplication, sadfile, "", [], [])

        ## reset file
        shutil.copy(comp_spd+".ORIG", comp_spd)

    def test_createApplicationFailures_comp_bad_dep_ref(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        comp_spd=scatest.getSdrPath()+"/dom/components/cpp_with_deps/cpp_with_deps.spd.xml"
        shutil.copy(comp_spd+".TEST.bad.dep", comp_spd)
        self.assertRaises( CF.InvalidProfile, self._domMgr.createApplication, sadfile, "", [], [])

        ## reset file
        shutil.copy(comp_spd+".ORIG", comp_spd)

    def test_createApplicationFailures_dep_missing(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        dep_dir=scatest.getSdrPath()+"/dom/deps/cpp_dep1"
        shutil.move(dep_dir, dep_dir+".XXX")
        self.assertRaises( CF.InvalidProfile, self._domMgr.createApplication, sadfile, "", [], [])

        ## reset file
        shutil.move(dep_dir+".XXX", dep_dir)

    def test_createApplicationFailures_dep_missing_dep_dir(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        dep_dir=scatest.getSdrPath()+"/dom/deps/cpp_dep1/cpp"
        shutil.move(dep_dir, dep_dir+".XXX")
        self.assertRaises( CF.InvalidProfile, self._domMgr.createApplication, sadfile, "", [], [])

        ## reset file
        shutil.move( dep_dir+".XXX", dep_dir)


    def test_createApplicationFailures_dep_bad_rec_dep_file(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        dep_spd=scatest.getSdrPath()+"/dom/deps/cpp_dep1/cpp_dep1.spd.xml"
        shutil.copy(dep_spd+".TEST.bad.rec.dep.file", dep_spd)
        self.assertRaises( CF.InvalidProfile, self._domMgr.createApplication, sadfile, "", [], [])

        ## reset file
        shutil.copy(dep_spd+".ORIG", dep_spd)


    def test_createApplicationFailures_recdep_missing_dir(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        dep_spd=scatest.getSdrPath()+"/dom/deps/cpp_dep2"
        shutil.move(dep_spd, dep_spd+".XXX")
        self.assertRaises( CF.InvalidProfile, self._domMgr.createApplication, sadfile, "", [], [])

        ## reset file
        shutil.move(dep_spd+".XXX", dep_spd)

    def test_createApplicationFailures_recdep_missing_file(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        dep_spd=scatest.getSdrPath()+"/dom/deps/cpp_dep2/cpp_dep2.spd.xml"
        shutil.move(dep_spd, dep_spd+".XXX")
        self.assertRaises( CF.InvalidProfile, self._domMgr.createApplication, sadfile, "", [], [])

        ## reset file
        shutil.move(dep_spd+".XXX", dep_spd)


    def test_createApplicationFailures_recdep_bad_dir(self):
        wf_name=self.wf_name
        sadfile=self.sadfile

        dep_spd=scatest.getSdrPath()+"/dom/deps/cpp_dep2/cpp_dep2.spd.xml"
        shutil.copy( dep_spd+".TEST.bad.dir", dep_spd)
        self.assertRaises( CF.InvalidProfile, self._domMgr.createApplication, sadfile, "", [], [])

        ## reset file
        shutil.copy(dep_spd+".ORIG", dep_spd)


    def test_createApplication_recdep_typeiscompliant(self):
        wf_name="cpp_deps_wf2"
        sadfile="/waveforms/"+wf_name+"/"+wf_name+".sad.xml"

        dep_spd=scatest.getSdrPath()+"/dom/deps/cpp_dep2/cpp_dep2.spd.xml"
        shutil.copy( dep_spd+".ORIG", dep_spd)
        app = self._domMgr.createApplication( sadfile, "some_app", [], [])
        self.assertNotEqual( app, None )
        app.releaseObject()

        ## reset file
        shutil.copy(dep_spd+".ORIG", dep_spd)


    def test_createApplication_recdep_typeisnoncompliant(self):
        wf_name="cpp_deps_wf2"
        sadfile="/waveforms/"+wf_name+"/"+wf_name+".sad.xml"

        dep_spd=scatest.getSdrPath()+"/dom/deps/cpp_dep2/cpp_dep2.spd.xml"
        shutil.copy( dep_spd+".NON_COMP", dep_spd)
        app = self._domMgr.createApplication( sadfile, "some_app", [], [])
        self.assertNotEqual( app, None )
        app.releaseObject()

        ## reset file
        shutil.copy(dep_spd+".ORIG", dep_spd)
