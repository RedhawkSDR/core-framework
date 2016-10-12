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

import unittest, os, signal, commands
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
        nodebooter, self._domMgr = self.launchDomainManager(debug=self.debuglevel)
    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)
        killChildProcesses(os.getpid())

    def eventReceived(self, data):
            self.gotData = True

    def test_DeviceFailure(self):
        self.assertNotEqual(self._domMgr, None)
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/test_PythonNodeNoUpdateUsageState_node/DeviceManager.dcd.xml", debug=self.debuglevel)
        self.assertNotEqual(devMgr, None)

        # NOTE These assert check must be kept in-line with the DeviceManager.dcd.xml
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)

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
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/SimpleDevMgr/DeviceManager.dcd.xml", debug=self.debuglevel)
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
        nodebooter, self._domMgr = self.launchDomainManager(debug=self.debuglevel)
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

