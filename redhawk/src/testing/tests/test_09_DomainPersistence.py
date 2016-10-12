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

import unittest, os, signal, time, sys
from subprocess import Popen
import scatest
from xml.dom import minidom
import omniORB
from omniORB import URI, any, CORBA
from ossie.cf import CF
import tempfile
import threading
try:
    import CosEventComm,CosEventComm__POA
    import CosEventChannelAdmin
    from ossie.cf import StandardEvent
    hasEvents = True
except:
    hasEvents = False


class Supplier_i(CosEventComm__POA.PushSupplier):
    def disconnect_push_supplier (self):
        pass

class Consumer_i(CosEventComm__POA.PushConsumer):
    def __init__(self, parent):
        self.parent = parent
        self.returnCount = 0
        self.receivelock = threading.Lock()
    
    def push(self, data):
        self.receivelock.acquire()
        if data._v == "bye":
            self.returnCount += 1
            if self.returnCount == 2 or self.returnCount == 3:
                self.parent.eventFlag = True
                self.parent.localEvent.set()
        self.receivelock.release()
    
    def disconnect_push_consumer (self):
        pass

# if SIGKILL is used (simulating a nodeBooter unexpected abort) 
# the next attempt to communicate with the domain manager will
# throw a COMM_FAILURE because the connection died unexpectedly

# Clients that hold references to the DomainManager should 
# include code similar to that below
def comm_failure_retry(cookie, n_retries, exception):
    # For the purposes of the unit test, only allow 1 retry
    if n_retries == 0:
        return True
    else:
        return False
omniORB.installCommFailureExceptionHandler(None, comm_failure_retry)

class DomainPersistenceTest(scatest.CorbaTestCase):
    def setUp(self):
        self._dbfile = tempfile.mktemp()

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)
        try:
            os.remove(self._dbfile)
        except OSError:
            pass

    def test_BasicOperation(self):
        self._nb_domMgr, self._domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)
        self._nb_devMgr, devMgr = self.launchDeviceManager("/nodes/test_PortTestDevice_node/DeviceManager.dcd.xml")
        
        self._fileMgr = self._domMgr._get_fileMgr()
        self._files = self._fileMgr.list("/")

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        self._domMgr.installApplication("/waveforms/PortConnectProvidesPort/PortConnectProvidesPort.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)
        
        # Load on the  device ID
        das = minidom.parse(os.path.join(scatest.getSdrPath(), "dom/waveforms/PortConnectProvidesPort/PortConnectProvidesPort_DAS.xml"))
        ds = []
        deviceAssignmentTypeNodeList = das.getElementsByTagName("deviceassignmenttype")
        for node in deviceAssignmentTypeNodeList:
            componentid = node.getElementsByTagName("componentid")[0].firstChild.data
            assigndeviceid = node.getElementsByTagName("assigndeviceid")[0].firstChild.data
            ds.append( CF.DeviceAssignmentType(str(componentid),str(assigndeviceid)) )

        # Ensure the expected device is available
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 2)
        device1 = None
        device2 = None
        for dev in devMgr._get_registeredDevices():
            if dev._get_identifier() == "DCE:322fb9b2-de57-42a2-ad03-217bcb244262":
                device1 = dev
            elif dev._get_identifier() == "DCE:47dc45d8-19b5-4b7e-bcd4-b165babe5b84":
                device2 = dev
        self.assertNotEqual(device1, None)
        self.assertNotEqual(device2, None)

        # Query the known allocation properties
        memCapacity1 = device1.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips1 = device1.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity1.value._v, 100000000)
        self.assertEqual(bogoMips1.value._v, 100000000)

        memCapacity2 = device2.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips2 = device2.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity2.value._v, 100000000)
        self.assertEqual(bogoMips2.value._v, 100000000)

        appFact = self._domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], ds)

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 1)
        app = self._domMgr._get_applications()[0]

        # cache off some variables for later for testing purposes
        self.assertEqual(len(app._get_componentNamingContexts()), 2)
        self.assertEqual(len(app._get_componentProcessIds()), 2)
        self.assertEqual(len(app._get_componentDevices()), 4)
        self.assertEqual(len(app._get_componentImplementations()), 2)
        
        origNamingContexts = app._get_componentNamingContexts()
        origProcessIds = app._get_componentProcessIds()
        origDevices = app._get_componentDevices()
        origImplementations = app._get_componentImplementations()
        origQuery = app.query([])

        # Verify that capacity was allocated
        memCapacity1 = device1.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips1 = device1.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity1.value._v, 100000000-(5000))
        self.assertEqual(bogoMips1.value._v, 100000000-(1000))

        memCapacity2 = device2.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips2 = device2.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity2.value._v, 100000000-(5000))
        self.assertEqual(bogoMips2.value._v, 100000000-(1000))

        # Kill the domainMgr
        os.kill(self._nb_domMgr.pid, signal.SIGTERM)

        # TODO if SIGKILL is used (simulating a nodeBooter unexpected abort, 
        # the IOR and the newly spawned domain manager do not work
        if not self.waitTermination(self._nb_domMgr):
            self.fail("Domain Manager Failed to Die")
      
        # Start the domainMgr again
        self._nb_domMgr, newDomMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)
        
        # Verify our client reference still is valid
        newDomMgr._get_identifier()
        self.assertEqual(False, newDomMgr._non_existent())
        #self.assertEqual(self._orb.object_to_string(newDomMgr), self._orb.object_to_string(self._domMgr))
        self.assertEqual(False, self._domMgr._non_existent())

        # Verify that the file manager is still valid
        self.new_files = self._fileMgr.list("/")
        self.assertEqual(len(self._files), len(self.new_files))

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 1)
        newDevMgr = self._domMgr._get_deviceManagers()[0]
        self.assertEqual(self._orb.object_to_string(devMgr), self._orb.object_to_string(newDevMgr))

        self.assertEqual(len(devMgr._get_registeredDevices()), 2)
        newDevice1 = None
        newDevice2 = None
        for dev in devMgr._get_registeredDevices():
            if dev._get_identifier() == "DCE:322fb9b2-de57-42a2-ad03-217bcb244262":
                newDevice1 = dev
            elif dev._get_identifier() == "DCE:47dc45d8-19b5-4b7e-bcd4-b165babe5b84":
                newDevice2 = dev
        self.assertNotEqual(newDevice1, None)
        self.assertNotEqual(newDevice2, None)
        self.assertEqual(self._orb.object_to_string(device1), self._orb.object_to_string(newDevice1))
        self.assertEqual(self._orb.object_to_string(device2), self._orb.object_to_string(newDevice2))

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        newAppFact = self._domMgr._get_applicationFactories()[0]
        self.assertEqual(appFact._get_identifier(), newAppFact._get_identifier());
        self.assertEqual(appFact._get_name(), newAppFact._get_name());

        self.assertEqual(len(self._domMgr._get_applications()), 1)
        newApp = self._domMgr._get_applications()[0]
        # Applications are currently not using persistent POAs, so the IORs will change
#        self.assertEqual(self._orb.object_to_string(app), self._orb.object_to_string(newApp))

        newNamingContexts = app._get_componentNamingContexts()
        newProcessIds = app._get_componentProcessIds()
        newDevices = app._get_componentDevices()
        newImplementations = app._get_componentImplementations()

        # We have to compare elements individually since newDevices == origDevices even when the contents are identical
        self.assertEqual(len(newDevices), len(origDevices))
        for x in xrange(len(newDevices)):
            self.assertEqual(newDevices[x].componentId, origDevices[x].componentId)
            self.assertEqual(newDevices[x].assignedDeviceId, origDevices[x].assignedDeviceId)

        self.assertEqual(len(newProcessIds), len(origProcessIds))
        for x in xrange(len(newProcessIds)):
            self.assertEqual(newProcessIds[x].componentId, origProcessIds[x].componentId)
            self.assertEqual(newProcessIds[x].processId, origProcessIds[x].processId)

        self.assertEqual(len(newImplementations), len(origImplementations))
        for x in xrange(len(newImplementations)):
            self.assertEqual(newImplementations[x].componentId, origImplementations[x].componentId)
            self.assertEqual(newImplementations[x].elementId, origImplementations[x].elementId)

        self.assertEqual(len(newNamingContexts), len(origNamingContexts))
        for x in xrange(len(newNamingContexts)):
            self.assertEqual(newNamingContexts[x].componentId, origNamingContexts[x].componentId)
            self.assertEqual(newNamingContexts[x].elementId, origNamingContexts[x].elementId)

        # Verify that the connection between the CF::Application and the assemblyController was 
        # restored.
        newQuery = app.query([])
        self.assertEqual(len(newQuery), len(origQuery))
        for x in xrange(len(newQuery)):
            self.assertEqual(newQuery[x].id, origQuery[x].id)
            self.assertEqual(newQuery[x].value._v, origQuery[x].value._v)
#
        app.stop()
        app.releaseObject()

        # TODO how can we verify that the ports were disconnected as part of release?
        time.sleep(1)

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        # Verify that capacity was deallocated
        memCapacity1 = device1.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips1 = device1.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity1.value._v, 100000000)
        self.assertEqual(bogoMips1.value._v, 100000000)

        memCapacity2 = device2.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips2 = device2.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity2.value._v, 100000000)
        self.assertEqual(bogoMips2.value._v, 100000000)

        newDevMgr.shutdown()
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)

        self._domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)

    def test_EventAppPortConnectionSIGKILL(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._nb_domMgr, domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)
        self._nb_devMgr, devMgr = self.launchDeviceManager("/nodes/test_EventPortTestDevice_node/DeviceManager.dcd.xml")

        domainName = scatest.getTestDomainName()
        domMgr.installApplication("/waveforms/PortConnectFindByDomainFinderEvent/PortConnectFindByDomainFinderEvent.sad.xml")
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        app.start()

        # Kill the domainMgr
        os.kill(self._nb_domMgr.pid, signal.SIGKILL)
        if not self.waitTermination(self._nb_domMgr, 5.0):
            self.fail("Domain Manager Failed to Die")

        # Restart the Domain Manager (which should restore the old channel)
        self._nb_domMgr, domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)

        newappFact = domMgr._get_applicationFactories()[0]
        app2 = newappFact.create(appFact._get_name(), [], [])
        app2.start()
        channelName = URI.stringToName("%s/%s" % (domainName, 'anotherChannel'))
        try:
            appChannel = self._root.resolve(channelName)._narrow(CosEventChannelAdmin.EventChannel)
        except:
            self.assertEqual(False, True)
        else:
            self.assertEqual(True, True)

        # resolve the producer for the event
        supplier_admin = appChannel.for_suppliers()
        _proxy_consumer = supplier_admin.obtain_push_consumer()
        _supplier = Supplier_i()
        _proxy_consumer.connect_push_supplier(_supplier._this())

        # resolve the consumer for the event
        consumer_admin = appChannel.for_consumers()
        _proxy_supplier = consumer_admin.obtain_push_supplier()
        _consumer = Consumer_i(self)
        _proxy_supplier.connect_push_consumer(_consumer._this())

        # a flag is raised only when two responses come back (one for each running app)
        _proxy_consumer.push(any.to_any("hello"))
        self.localEvent.wait(5.0)
        self.assertEqual(self.eventFlag, True)

        self.eventFlag = False
        # this step tests whether the number of subscribers to the channel is restored
        app2.releaseObject()

        self.localEvent.clear()
        _proxy_consumer.push(any.to_any("hello"))
        self.localEvent.wait(5.0)
        self.assertEqual(self.eventFlag, True)
        app.releaseObject()

    def test_EventAppPortConnectionSIGTERM(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._nb_domMgr, domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)
        self._nb_devMgr, devMgr = self.launchDeviceManager("/nodes/test_EventPortTestDevice_node/DeviceManager.dcd.xml")

        domainName = scatest.getTestDomainName()
        domMgr.installApplication("/waveforms/PortConnectFindByDomainFinderEvent/PortConnectFindByDomainFinderEvent.sad.xml")
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        app.start()

        # Kill the domainMgr
        os.kill(self._nb_domMgr.pid, signal.SIGTERM)
        if not self.waitTermination(self._nb_domMgr, 5.0):
            self.fail("Domain Manager Failed to Die")

        # Restart the Domain Manager (which should restore the old channel)
        self._nb_domMgr, domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)

        newappFact = domMgr._get_applicationFactories()[0]
        app2 = newappFact.create(appFact._get_name(), [], [])
        app2.start()
        channelName = URI.stringToName("%s/%s" % (domainName, 'anotherChannel'))
        try:
            appChannel = self._root.resolve(channelName)._narrow(CosEventChannelAdmin.EventChannel)
        except:
            self.assertEqual(False, True)
        else:
            self.assertEqual(True, True)

        # resolve the producer for the event
        supplier_admin = appChannel.for_suppliers()
        _proxy_consumer = supplier_admin.obtain_push_consumer()
        _supplier = Supplier_i()
        _proxy_consumer.connect_push_supplier(_supplier._this())

        # resolve the consumer for the event
        consumer_admin = appChannel.for_consumers()
        _proxy_supplier = consumer_admin.obtain_push_supplier()
        _consumer = Consumer_i(self)
        _proxy_supplier.connect_push_consumer(_consumer._this())

        # a flag is raised only when two responses come back (one for each running app)
        _proxy_consumer.push(any.to_any("hello"))
        self.localEvent.wait(5.0)
        self.assertEqual(self.eventFlag, True)

        self.eventFlag = False
        # this step tests whether the number of subscribers to the channel is restored
        app2.releaseObject()

        self.localEvent.clear()
        _proxy_consumer.push(any.to_any("hello"))
        self.localEvent.wait(5.0)
        self.assertEqual(self.eventFlag, True)
        app.releaseObject()

    def test_EventAppPortConnectionSIGTERMNoPersist(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._nb_domMgr, domMgr = self.launchDomainManager("--nopersist", endpoint="giop:tcp::5679", dbURI=self._dbfile)
        self._nb_devMgr, devMgr = self.launchDeviceManager("/nodes/test_EventPortTestDevice_node/DeviceManager.dcd.xml")

        domainName = scatest.getTestDomainName()
        domMgr.installApplication("/waveforms/PortConnectFindByDomainFinderEvent/PortConnectFindByDomainFinderEvent.sad.xml")
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        app.start()

        # Kill the domainMgr
        os.kill(self._nb_domMgr.pid, signal.SIGTERM)
        if not self.waitTermination(self._nb_domMgr, 5.0):
            self.fail("Domain Manager Failed to Die")

        # Restart the Domain Manager (which should restore the old channel)
        self._nb_domMgr, domMgr = self.launchDomainManager("--nopersist", endpoint="giop:tcp::5679", dbURI=self._dbfile)

        newappFact = domMgr._get_applicationFactories()
        self.assertEqual(len(newappFact), 0)

        apps = domMgr._get_applications()
        self.assertEqual(len(apps), 0)

        devMgrs = domMgr._get_deviceManagers()
        self.assertEqual(len(devMgrs), 0)

    def test_EventAppPortConnectionSIGQUIT(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._nb_domMgr, domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)
        self._nb_devMgr, devMgr = self.launchDeviceManager("/nodes/test_EventPortTestDevice_node/DeviceManager.dcd.xml")

        domainName = scatest.getTestDomainName()
        domMgr.installApplication("/waveforms/PortConnectFindByDomainFinderEvent/PortConnectFindByDomainFinderEvent.sad.xml")
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        app.start()

        # Kill the domainMgr
        os.kill(self._nb_domMgr.pid, signal.SIGQUIT)
        if not self.waitTermination(self._nb_domMgr, 5.0):
            self.fail("Domain Manager Failed to Die")

        # Restart the Domain Manager (which should restore the old channel)
        self._nb_domMgr, domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)

        newappFact = domMgr._get_applicationFactories()[0]
        app2 = newappFact.create(appFact._get_name(), [], [])
        app2.start()
        channelName = URI.stringToName("%s/%s" % (domainName, 'anotherChannel'))
        try:
            appChannel = self._root.resolve(channelName)._narrow(CosEventChannelAdmin.EventChannel)
        except:
            self.assertEqual(False, True)
        else:
            self.assertEqual(True, True)

        # resolve the producer for the event
        supplier_admin = appChannel.for_suppliers()
        _proxy_consumer = supplier_admin.obtain_push_consumer()
        _supplier = Supplier_i()
        _proxy_consumer.connect_push_supplier(_supplier._this())

        # resolve the consumer for the event
        consumer_admin = appChannel.for_consumers()
        _proxy_supplier = consumer_admin.obtain_push_supplier()
        _consumer = Consumer_i(self)
        _proxy_supplier.connect_push_consumer(_consumer._this())

        # a flag is raised only when two responses come back (one for each running app)
        _proxy_consumer.push(any.to_any("hello"))
        self.localEvent.wait(5.0)
        self.assertEqual(self.eventFlag, True)

        self.eventFlag = False
        # this step tests whether the number of subscribers to the channel is restored
        app2.releaseObject()

        self.localEvent.clear()
        _proxy_consumer.push(any.to_any("hello"))
        self.localEvent.wait(5.0)
        self.assertEqual(self.eventFlag, True)
        app.releaseObject()

    def test_EventAppPortConnectionSIGQUITNoPersist(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._nb_domMgr, domMgr = self.launchDomainManager("--nopersist", endpoint="giop:tcp::5679", dbURI=self._dbfile)
        self._nb_devMgr, devMgr = self.launchDeviceManager("/nodes/test_EventPortTestDevice_node/DeviceManager.dcd.xml")

        domainName = scatest.getTestDomainName()
        domMgr.installApplication("/waveforms/PortConnectFindByDomainFinderEvent/PortConnectFindByDomainFinderEvent.sad.xml")
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        app.start()

        # Kill the domainMgr
        os.kill(self._nb_domMgr.pid, signal.SIGQUIT)
        if not self.waitTermination(self._nb_domMgr, 5.0):
            self.fail("Domain Manager Failed to Die")

        # Restart the Domain Manager (which should restore the old channel)
        self._nb_domMgr, domMgr = self.launchDomainManager("--nopersist", endpoint="giop:tcp::5679", dbURI=self._dbfile)

        newappFact = domMgr._get_applicationFactories()
        self.assertEqual(len(newappFact), 0)

        apps = domMgr._get_applications()
        self.assertEqual(len(apps), 0)

        devMgrs = domMgr._get_deviceManagers()
        self.assertEqual(len(devMgrs), 0)

    def test_EventAppPortConnectionSIGINT(self):
        self.localEvent = threading.Event()
        self.eventFlag = False

        self._nb_domMgr, domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)
        self._nb_devMgr, devMgr = self.launchDeviceManager("/nodes/test_EventPortTestDevice_node/DeviceManager.dcd.xml")

        domainName = scatest.getTestDomainName()
        domMgr.installApplication("/waveforms/PortConnectFindByDomainFinderEvent/PortConnectFindByDomainFinderEvent.sad.xml")
        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])
        app.start()

        # Kill the domainMgr
        os.kill(self._nb_domMgr.pid, signal.SIGINT)
        if not self.waitTermination(self._nb_domMgr, 5.0):
            self.fail("Domain Manager Failed to Die")

        # Restart the Domain Manager (which should restore the old channel)
        self._nb_domMgr, domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)

        newappFact = domMgr._get_applicationFactories()
        self.assertEqual(len(newappFact), 0)

        apps = domMgr._get_applications()
        self.assertEqual(len(apps), 0)

        devMgrs = domMgr._get_deviceManagers()
        self.assertEqual(len(devMgrs), 0)

    def test_DeviceManagerDisappear(self):
        self._nb_domMgr, self._domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)
        self._nb_devMgr, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        self._domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        # Ensure the expected device is available
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        # Kill the domainMgr and device manager
        os.kill(self._nb_domMgr.pid, signal.SIGKILL)
        if not self.waitTermination(self._nb_domMgr):
            self.fail("Domain Manager Failed to Die")
      
        os.kill(self._nb_devMgr.pid, signal.SIGTERM)
        if not self.waitTermination(self._nb_devMgr):
            self.fail("Device Manager Failed to Die")

        # Start the domainMgr again
        self._nb_domMgr, newDomMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)
        
        # Verify our client reference still is valid
        self.assertEqual(False, newDomMgr._non_existent())
        self.assertEqual(newDomMgr._get_identifier(),'DCE:5f52f645-110f-4142-8cc9-4d9316ddd958')
        self.assertEqual(self._domMgr._get_identifier(),'DCE:5f52f645-110f-4142-8cc9-4d9316ddd958')
        self.assertEqual(False, self._domMgr._non_existent())

        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)

    def test_ServicesRestored(self):
        domBooter, domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_PortTestDevice_node/DeviceManager.dcd.xml")
        svcBooter, svcMgr = self.launchDeviceManager("/nodes/test_BasicService_node/DeviceManager.dcd.xml")
        
        # Make sure that the service node is up before killing the domain manager
        while len(svcMgr._get_registeredServices()) != 1:
            time.sleep(0.1)

        # Forcibly terminate the domain manager to simulate a crash
        os.kill(domBooter.pid, signal.SIGKILL)
        if not self.waitTermination(domBooter):
            self.fail("DomainManager failed to die")

        # Restart the domain manager
        domBooter, domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)

        # Check that the domain manager reconnected to the device managers.
        self.assertEqual(len(domMgr._get_deviceManagers()), 2)

        # Install the PortConnectServiceName application and try to create an
        # instance to verify that the domain manager is still aware of the services
        # that had previously been registered.
        domMgr.installApplication("/waveforms/PortConnectServiceName/PortConnectServiceName.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]
        try:
            app = appFact.create(appFact._get_name(), [], [])
        except CF.ApplicationFactory.CreateApplicationError:
            self.fail("Unable to create application with service connection")

        # The BasicService provides a PropertySet interface, so verify that some
        # properties are returned from the service test of the PortTest component.
        testResults = app.runTest(1, [])
        self.assertNotEqual(len(testResults), 0)

    def test_ConnectionsRestored(self):
        domBooter, domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)
        self.assertNotEqual(domMgr, None)

        # Start the second node first; it should not report any connections yet
        # because the connection is pending.
        devBooter2, devMgr2 = self.launchDeviceManager("/nodes/test_PortTestDevice2_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr2, None)
        device = devMgr2._get_registeredDevices()[0]
        self.assertEqual(device._get_label(), "PortTestDevice3")
        self.assertEqual(len(device.runTest(0, [])), 0)

        # Forcibly terminate the domain manager to simulate a crash
        os.kill(domBooter.pid, signal.SIGKILL)
        if not self.waitTermination(domBooter):
            self.fail("DomainManager failed to die")

        # Restart the domain manager
        domBooter, domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)
        
        # Start the first node; the pending connection from the second node
        # should be completed now.
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_PortTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        # Verify that the connection is completed, and points to a device on
        # the first node.
        connections = device.runTest(0, [])
        self.assertEqual(len(connections), 1)
        identifier = connections[0].value.value()
        self.assertNotEqual(identifier, None)
        identifier, port = identifier.split('/')
        device2 = None
        for dev in devMgr._get_registeredDevices():
            if identifier == dev._get_identifier():
                device2 = dev
        self.assertNotEqual(device2, None)
        
        # Forcibly terminate the domain manager to simulate a crash (again)
        os.kill(domBooter.pid, signal.SIGKILL)
        if not self.waitTermination(domBooter):
            self.fail("DomainManager failed to die")

        # Terminate the first node. The connection should not get broken yet.
        self.terminateChild(devBooter)
        self.assertEqual(len(device.runTest(0, [])), 1)

        # Restart the domain manager (again)
        domBooter, domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)

        # On restore, the connection should have been disconnected.
        self.assertEqual(len(device.runTest(0, [])), 0)

    def test_DeviceManagerRegisterWhileDomainManagerCrashed(self):
        domBooter, domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)
        self.assertNotEqual(domMgr, None)
        
        # Forcibly terminate the domain manager to simulate a crash
        os.kill(domBooter.pid, signal.SIGKILL)
        if not self.waitTermination(domBooter):
            self.fail("DomainManager failed to die")

        time.sleep(2)
        # Start the node; we cannot get the object reference because the
        # DomainManager is down, so waiting is pointless right now.
        dcdFile = "/nodes/test_PortTestDevice_node/DeviceManager.dcd.xml"
        devBooter, unused = self.launchDeviceManager(dcdFile, wait=False)

        time.sleep(2)
        # Restart the DomainManager
        domBooter, domMgr = self.launchDomainManager(endpoint="giop:tcp::5679", dbURI=self._dbfile)
        self.assertNotEqual(domMgr, None)

        # Wait for the DeviceManager and make sure it registers.
        devMgr = self.waitDeviceManager(devBooter, dcdFile)
        self.assertNotEqual(devMgr, None)
        

# Only run these tests if persistence was enabled at compile time
if not scatest.persistenceEnabled():
    del DomainPersistenceTest

