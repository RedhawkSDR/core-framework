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
import time
from omniORB import CORBA, URI, any
from ossie.cf import CF, CF__POA
import commands, signal

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

def killProcesses(parentPid):
    # kills processes from the top down (children),
    # starting with parentPid

    childPids = getChildren(parentPid)

    try:
        os.kill(parentPid, signal.SIGKILL)
        os.waitpid(parentPid, 0)
    except OSError:
       pass

    for pid in childPids:
        killProcesses(pid)

class HardFailTest(scatest.CorbaTestCase):
    def setUp(self):
        pass

    def test_HardFailDevice(self):
        # test HardFail of a device
        #   - test that when a device fails hard (ex: seg fault or kill -9)
        #     that things still behave as expected

        domBooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        devBooterA, devMgrA = self.launchDeviceManager("/nodes/test_HardFail_nodeA/DeviceManager.dcd.xml", debug=self.debuglevel)
        devBooterB, devMgrB = self.launchDeviceManager("/nodes/test_HardFail_nodeB/DeviceManager.dcd.xml", debug=self.debuglevel)

        devInfo = []
        devInfo.append({"booter":devBooterA, "mgr":devMgrA})
        devInfo.append({"booter":devBooterB, "mgr":devMgrB})

        # ensure both DeviceManagers were started
        self.assertEqual(len(domMgr._get_deviceManagers()), 2)

        # store original capacities of devices
        prop = [CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))]
        for i in devInfo:
            i['dev'] = i['mgr']._get_registeredDevices()[0]
            i['dev_BogoMipsCapacity'] = i['dev'].query(prop)[0].value.value()

        # install first waveform
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperOneDep/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app1 = appFact.create(appFact._get_name(), [], [])
        processIds = app1._get_componentProcessIds()
        componentProcessId = processIds[0].processId

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        # find which device manager it got installed on
        devMgrToKill = None
        devMgrToRelaunchOn = None

        if devInfo[0]['dev'].query(prop)[0].value.value() != devInfo[0]['dev_BogoMipsCapacity']:
            devMgrToKill = 0
            devMgrToRelaunchOn = 1
        elif devInfo[1]['dev'].query(prop)[0].value.value() != devInfo[1]['dev_BogoMipsCapacity']:
            devMgrToKill = 1
            devMgrToRelaunchOn = 0
        else:
            self.fail("Application didn't launch on either device manager")

        # kill the device on which the waveform was launched
        devs = getChildren(devInfo[devMgrToKill]['booter'].pid)
        self.assertEqual(len(devs), 1)

        try:
            os.kill(devs[0], signal.SIGKILL)
            os.waitpid(devs[0], 0)
        except OSError:
           pass

        # check to see if the app and it's components are released
        for x in range(10): # only try for 5 seconds
            if len(domMgr._get_applications()) > 0:
                time.sleep(0.5)

        self.assertEqual(len(domMgr._get_applications()), 0)

        # kill off remaining component process if it is still around
        try:
            os.kill(componentProcessId, signal.SIGKILL)
        except:
            pass

    def test_HardFailComponent(self):
        # test HardFail of the component(s) in and application
        #   - test that when a component fails hard (ex: seg fault or kill -9)
        #     that things still behave as expected

        domBooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        devBooterA, devMgrA = self.launchDeviceManager("/nodes/test_HardFail_nodeA/DeviceManager.dcd.xml", debug=self.debuglevel)
        devBooterB, devMgrB = self.launchDeviceManager("/nodes/test_HardFail_nodeB/DeviceManager.dcd.xml", debug=self.debuglevel)

        devInfo = []
        devInfo.append({"booter":devBooterA, "mgr":devMgrA})
        devInfo.append({"booter":devBooterB, "mgr":devMgrB})

        # ensure both DeviceManagers were started
        self.assertEqual(len(domMgr._get_deviceManagers()), 2)

        # store original capacities of devices
        prop = [CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))]
        for i in devInfo:
            i['dev'] = i['mgr']._get_registeredDevices()[0]
            i['dev_BogoMipsCapacity'] = i['dev'].query(prop)[0].value.value()

        # install first waveform
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperOneDep/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app1 = appFact.create(appFact._get_name(), [], [])

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        # find which device manager it got installed on
        devMgrToKill = None
        devMgrToRelaunchOn = None

        if devInfo[0]['dev'].query(prop)[0].value.value() != devInfo[0]['dev_BogoMipsCapacity']:
            devMgrToKill = 0
            devMgrToRelaunchOn = 1
        elif devInfo[1]['dev'].query(prop)[0].value.value() != devInfo[1]['dev_BogoMipsCapacity']:
            devMgrToKill = 1
            devMgrToRelaunchOn = 0
        else:
            self.fail("Application didn't launch on either device manager")

        # find & kill the component which is part of the waveform that was launched
        devs = getChildren(devInfo[devMgrToKill]['booter'].pid)
        self.assertEqual(len(devs), 1)
        comps = getChildren(devs[0])
        self.failIf(len(comps) < 1, "Should be at least one component in the waveform")

        try:
            os.kill(comps[0], signal.SIGKILL)
            os.waitpid(comps[0], 0)
        except OSError:
           pass

        # try to  stop & release the waveform
        self.assertRaises(CF.Resource.StopError, app1.stop)   # expect not to work - AC is dead

        try:
            app1.releaseObject()
        except:
            self.fail("application->releaseObject should work here")

        self.assertEqual(len(domMgr._get_applications()), 0)


        # make sure capacity was deallocated
        self.assertEqual(devInfo[devMgrToKill]['dev'].query(prop)[0].value.value(), devInfo[devMgrToKill]['dev_BogoMipsCapacity'])


    def test_HardFailDeviceManager(self):
        # test HardFail of a Device Manager
        #   - test that when a Device Manager fails hard (ex: seg fault or kill -9)
        #     that things still behave as expected

        domBooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        devBooterA, devMgrA = self.launchDeviceManager("/nodes/test_HardFail_nodeA/DeviceManager.dcd.xml", debug=self.debuglevel)
        devBooterB, devMgrB = self.launchDeviceManager("/nodes/test_HardFail_nodeB/DeviceManager.dcd.xml", debug=self.debuglevel)

        devInfo = []
        devInfo.append({"booter":devBooterA, "mgr":devMgrA})
        devInfo.append({"booter":devBooterB, "mgr":devMgrB})

        # ensure both DeviceManagers were started
        self.assertEqual(len(domMgr._get_deviceManagers()), 2)

        # store original capacities of devices
        prop = [CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))]
        for i in devInfo:
            i['dev'] = i['mgr']._get_registeredDevices()[0]
            i['dev_BogoMipsCapacity'] = i['dev'].query(prop)[0].value.value()

        # install first waveform
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperOneDep/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app1 = appFact.create(appFact._get_name(), [], [])

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        # find which device manager it got installed on
        devMgrToKill = None
        devMgrToRelaunchOn = None

        if devInfo[0]['dev'].query(prop)[0].value.value() != devInfo[0]['dev_BogoMipsCapacity']:
            devMgrToKill = 0
            devMgrToRelaunchOn = 1
        elif devInfo[1]['dev'].query(prop)[0].value.value() != devInfo[1]['dev_BogoMipsCapacity']:
            devMgrToKill = 1
            devMgrToRelaunchOn = 0
        else:
            self.fail("Application didn't launch on either device manager")

        # get the DeviceManager's children, so we can clean up
        devMgr_children = getChildren(devInfo[devMgrToKill]['booter'].pid)

        # kill the Device Manager on which the waveform was launched
        try:
            os.kill(devInfo[devMgrToKill]['booter'].pid, signal.SIGKILL)
            os.waitpid(devInfo[devMgrToKill]['booter'].pid, 0)
        except OSError:
           pass

        # try to release the waveform from the dead device manager
        try:
            app1.stop()
        except CF.Resource.StopError, msg:
            self.fail("application->stop() should work here; failed with message: " + msg)

        try:
            app1.releaseObject()
        except:
            self.fail("application->releaseObject should work here")

        self.assertEqual(len(domMgr._get_applications()), 0)

        # make sure capacity was deallocated
        self.assertEqual(devInfo[devMgrToKill]['dev'].query(prop)[0].value.value(), devInfo[devMgrToKill]['dev_BogoMipsCapacity'])

        # clean up
        for p in devMgr_children:
            killProcesses(p)

    def test_HardFailDeviceManagerRestart(self):
        # test HardFail of a Device Manager and restart
        #   - test that when a Device Manager fails hard (ex: seg fault or kill -9)
        #     and then it is restarted that things still behave as expected

        domBooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        devBooterA, devMgrA = self.launchDeviceManager("/nodes/test_HardFail_nodeA/DeviceManager.dcd.xml", debug=self.debuglevel)
        devBooterB, devMgrB = self.launchDeviceManager("/nodes/test_HardFail_nodeB/DeviceManager.dcd.xml", debug=self.debuglevel)

        devInfo = []
        devInfo.append({"booter":devBooterA, "mgr":devMgrA})
        devInfo.append({"booter":devBooterB, "mgr":devMgrB})

        # ensure both DeviceManagers were started
        self.assertEqual(len(domMgr._get_deviceManagers()), 2)

        # store original capacities of devices
        prop = [CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))]
        for i in devInfo:
            i['dev'] = i['mgr']._get_registeredDevices()[0]
            i['dev_BogoMipsCapacity'] = i['dev'].query(prop)[0].value.value()

        # install first waveform
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperOneDep/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app1 = appFact.create(appFact._get_name(), [], [])

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        # find which device manager it got installed on
        devMgrToKill = None
        devMgrToRelaunchOn = None

        if devInfo[0]['dev'].query(prop)[0].value.value() != devInfo[0]['dev_BogoMipsCapacity']:
            devMgrToKill = 0
            devMgrToRelaunchOn = 1
        elif devInfo[1]['dev'].query(prop)[0].value.value() != devInfo[1]['dev_BogoMipsCapacity']:
            devMgrToKill = 1
            devMgrToRelaunchOn = 0
        else:
            self.fail("Application didn't launch on either device manager")

        # kill the device manager, devices, and components to simulate a hard failure
        killProcesses(devInfo[devMgrToKill]['booter'].pid)

        # at this point, the domain manager still thinks things are in good shape
        self.assertEqual(len(domMgr._get_applications()), 1)

        # relaunch device manager
        if devMgrToKill == 0:
            devBooterA, devMgrA = self.launchDeviceManager("/nodes/test_HardFail_nodeA/DeviceManager.dcd.xml", debug=self.debuglevel)
            devInfo[0]['booter'] = devBooterA
            devInfo[0]['mgr'] = devMgrA
        else:
            devBooterB, devMgrB = self.launchDeviceManager("/nodes/test_HardFail_nodeB/DeviceManager.dcd.xml", debug=self.debuglevel)
            devInfo[1]['booter'] = devBooterB
            devInfo[1]['mgr'] = devMgrB

        # stale app should have been cleaned up
        self.assertEqual(len(domMgr._get_applications()), 0)

    def test_HardFailAll(self):
        # test HardFail of a node
        #   - test that when a DeviceManager and its devices and apps fail hard (ex: power failure)
        #     that things still behave as expected

        domBooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        devBooterA, devMgrA = self.launchDeviceManager("/nodes/test_HardFail_nodeA/DeviceManager.dcd.xml", debug=self.debuglevel)
        devBooterB, devMgrB = self.launchDeviceManager("/nodes/test_HardFail_nodeB/DeviceManager.dcd.xml", debug=self.debuglevel)

        devInfo = []
        devInfo.append({"booter":devBooterA, "mgr":devMgrA})
        devInfo.append({"booter":devBooterB, "mgr":devMgrB})

        # ensure both DeviceManagers were started
        self.assertEqual(len(domMgr._get_deviceManagers()), 2)

        # store original capacities of devices
        prop = [CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))]
        for i in devInfo:
            i['dev'] = i['mgr']._get_registeredDevices()[0]
            i['dev_BogoMipsCapacity'] = i['dev'].query(prop)[0].value.value()

        # install first waveform
        self.assertEqual(len(domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(domMgr._get_applications()), 0)

        domMgr.installApplication("/waveforms/CommandWrapperOneDep/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)
        appFact = domMgr._get_applicationFactories()[0]
        app1 = appFact.create(appFact._get_name(), [], [])

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        # find which device manager it got installed on
        devMgrToKill = None
        devMgrToRelaunchOn = None

        if devInfo[0]['dev'].query(prop)[0].value.value() != devInfo[0]['dev_BogoMipsCapacity']:
            devMgrToKill = 0
            devMgrToRelaunchOn = 1
        elif devInfo[1]['dev'].query(prop)[0].value.value() != devInfo[1]['dev_BogoMipsCapacity']:
            devMgrToKill = 1
            devMgrToRelaunchOn = 0
        else:
            self.fail("Application didn't launch on either device manager")

        # kill the device manager, devices, and components to simulate a hard failure
        killProcesses(devInfo[devMgrToKill]['booter'].pid)

        # try to stop and release the application (even though everything associated with it is dead)

        self.assertRaises(CF.Resource.StopError, app1.stop)

        try:
            app1.releaseObject()
        except:
            self.fail("releaseObject should have completed successfully here")

        # launch a waveform again on the second device manager
        appFact = domMgr._get_applicationFactories()[0]
        app2 = appFact.create(appFact._get_name(), [], [])

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)
        self.assertNotEqual(devInfo[devMgrToRelaunchOn]['dev'].query(prop)[0].value.value(),devInfo[devMgrToRelaunchOn]['dev_BogoMipsCapacity'])


