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
from _unitTestHelpers import scatest
from omniORB import URI, any
from ossie.cf import CF
import ossie.parsers.dcd
import threading


class WaveformWorker:
    def __init__(self, appFact, debug=False, num=None):
        self.debug = debug
        self.appFact = appFact
        self.thread_num = num
        self.data_signal_1 = threading.Event()
        self.data_signal_2 = threading.Event()
        self.process_thread = threading.Thread(target=self.Process)

    def getApp(self):
        # attempt to get the app and try until an arbitrary timeout period is reached
        begin_time = time.time()
        current_time = time.time()
        retval = "invalid"
        while (current_time-begin_time)<10:
            try:
                retval = self.app
                break
            except:
                pass
            current_time = time.time()
        return retval

    def Process(self):
        if self.thread_num and self.debug:
            print 'starting thread: ' + str(self.thread_num)
        try:
            self.app = self.appFact.create(self.appFact._get_name(), [], [])
        except:
            self.app = "failure: %s %s" % sys.exc_info()[0:2]
            return
        self.data_signal_1.wait()
        if self.app != None:
            self.app.stop()
            self.app.releaseObject()
            if self.thread_num and self.debug:
                print 'releasing thread: ' + str(self.thread_num)
        self.data_signal_2.wait()


def waitAppCount (domMgr, count, timeout, pause=1.0):
    while len(domMgr._get_applications()) != count and timeout > 0.0:
        timeout -= pause
        time.sleep(pause)
    return len(domMgr._get_applications()) == count


class StressTest(scatest.CorbaTestCase):
    def __init__(self, methodName='runTest'):
        scatest.CorbaTestCase.__init__(self, methodName, orbArgs=["-ORBoneCallPerConnection", "0"])

    def setUp(self):
        self.devRoot = os.path.join(scatest.getSdrPath(), 'dev')


    def _threadCheckin(self):
        self.countCondition.acquire()
        self.count -= 1
        if self.count == 0:
            self.countCondition.notify()
        self.countCondition.release()

    def _threadFastCycle(self, dcd):
        # Wait for all of the threads to get kicked off.
        self.startSignal.wait()

        # Launch the device manager; if it fails for some reason, return early.
        # The main thread will notice the incorrect count and report that a
        # failure occurred.
        devBooter, devMgr = self.launchDeviceManager(dcdFile=dcd)
        if not devMgr:
            return

        # Report back to the main thread.
        self._threadCheckin()

        # Wait for all threads to start their DeviceManagers, then shutdown.
        self.stopSignal.wait()
        devMgr.shutdown()

        # Report back to the main thread again.
        self._threadCheckin()

    def _createNode(self, nodeName, outfile, numDevices, device="BasicTestDevice"):
        # Set the node name and ID
        nodeId = "DCE:" + scatest.uuidgen()
        dcd = ossie.parsers.dcd.deviceconfiguration(id_=nodeId, name=nodeName)

        # Point to the DeviceManager softpkg
        softpkg = ossie.parsers.dcd.devicemanagersoftpkg()
        softpkg.set_localfile(ossie.parsers.dcd.localfile(name="/mgr/DeviceManager.spd.xml"))
        dcd.set_devicemanagersoftpkg(softpkg)

        # Define the device componentfile
        componentfiles = ossie.parsers.dcd.componentfiles()
        filerefid = "%s_%s" % (device, scatest.uuidgen())
        componentfile = ossie.parsers.dcd.componentfile(id_=filerefid, type_="SPD")
        deviceSpd = "/devices/%s/%s.spd.xml" % (device, device)
        componentfile.set_localfile(ossie.parsers.dcd.localfile(name=deviceSpd))
        componentfiles.add_componentfile(componentfile)
        dcd.set_componentfiles(componentfiles)
        fileref = ossie.parsers.dcd.componentfileref(filerefid)

        # Create devices
        partitioning = ossie.parsers.dcd.partitioning()
        for ii in range(numDevices):
            devName = "%s%d" % (device, ii+1)
            devId = "DCE:" + scatest.uuidgen()
            instantiation = ossie.parsers.dcd.componentinstantiation(id_=devId, usagename=devName)
            placement = ossie.parsers.dcd.componentplacement(componentfileref=fileref)
            placement.add_componentinstantiation(instantiation)
            partitioning.add_componentplacement(placement)

        dcd.set_partitioning(partitioning)

        domainmanager = ossie.parsers.dcd.domainmanager()
        domainmanager.set_namingservice(ossie.parsers.dcd.namingservice(name=""))
        dcd.set_domainmanager(domainmanager)

        # Write the output XML
        dcd.export(open(outfile, 'w'), 0)


    def test_FastCycle(self):
        # Make sure our base node directory exists.
        nodeDir = os.path.join(self.devRoot, 'nodes', 'test_Concurrent_nodes')
        if not os.path.isdir(nodeDir):
            os.mkdir(nodeDir)

        # Starts many nodes with many devices each simulataneously
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        numDevMgr = 10
        self.count = numDevMgr
        self.countCondition = threading.Condition()

        self.startSignal = threading.Event()
        self.stopSignal = threading.Event()

        # Generate a DCD file per node and launch a thread to start and stop it.
        for ii in range(1, numDevMgr+1):
            name = "ConcurrentNode%d" % (ii,)
            dcdFile = "/nodes/test_Concurrent_nodes/DeviceManager_%d.dcd.xml" % (ii,)
            self._createNode(name, self.devRoot + dcdFile, 5)
            t = threading.Thread(target=self._threadFastCycle, args=(dcdFile,))
            t.start()

        # Tell all the threads to start their DeviceManagers.
        self.startSignal.set()

        failures = []

        # Wait up to a second per node for all of the DeviceManagers to start.
        self.countCondition.acquire()
        if self.count > 0:
            self.countCondition.wait(numDevMgr * 1.0)
        if self.count > 0:
            # Not everybody made it; mark the failure and adjust expected count.
            failures.append("start")
            numDevMgr -= self.count
        self.countCondition.release()

        # Reset the counter and tell the threads to shutdown.
        self.count = numDevMgr
        self.stopSignal.set()

        # Wait up to a second per node for all of the DeviceManagers to shutdown.
        self.countCondition.acquire()
        if self.count > 0:
            self.countCondition.wait(numDevMgr * 1.0)
        if self.count > 0:
            failures.append("shutdown")
        self.countCondition.release()

        # Report if either phase had any failures.
        if len(failures) > 0:
            self.fail("Not all DeviceManagers completed %s" % ', '.join(failures))

        # Double check that there are no DeviceManagers left (this also verifies
        # that the DomainManager is still callable)
        self.assertEqual(len(domMgr._get_deviceManagers()), 0)

    def test_WaveformCreation(self):
        """A test to check the ApplicationFactory's ability to handle
           many simulataneous creation and tear down events of an application"""

        num_waveforms = 100  # number of simultaneous waveforms to launch

        processThreads = []
        domNb, domMgr = self.launchDomainManager()
        devNb1, devMgr1 = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        devNb2, devMgr2 = self.launchDeviceManager("/nodes/test_BasicTestDevice3_node/DeviceManager.dcd.xml")

        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/SlowStart/SlowStart.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]

        # create the worker classes - these will each create a new app (waveform) in a new thread
        for i in xrange(num_waveforms):
            processThreads.append(WaveformWorker(appFact, False, i))

        # start the thread (app/waveform)
        for entry in processThreads:
            entry.process_thread.start()

        # Give it time to start
        waitAppCount(domMgr, num_waveforms, 30)

        # make sure each waveform has started (or timed out while starting)
        for entry in processThreads:
            app = entry.getApp()
            if isinstance(app, str):
                self.fail("App %d: %s" % (entry.thread_num, app))
            entry.data_signal_1.set()

        # Give it time to shut down, and make sure they all finish
        self.assert_(waitAppCount(domMgr, 0, 30))

        # kill the threads
        for entry in processThreads:
            entry.data_signal_2.set()

        time.sleep(5)

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.uninstallApplication(appFact._get_identifier())
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)

