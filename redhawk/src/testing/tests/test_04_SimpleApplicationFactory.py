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
from xml.dom import minidom
from omniORB import any
from ossie.cf import CF

class ApplicationFactoryTest(scatest.CorbaTestCase):
    def setUp(self):
        pass # Nothing to do

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)

    def test_BasicOperation(self):
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
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
        nodebooter, devMgr = self.launchDeviceManager(dcdFile="/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=self.debuglevel)
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(ds), 1)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]
        self.assertEqual(device._get_identifier(), ds[0].assignedDeviceId)

        # Query the known allocation properties
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000)
        self.assertEqual(bogoMips.value._v, 100000000)
        # query all properties, make sure that only 'external' properties show up
        allProps = device.query([])

        # convert props to a dict
        allProps = dict([(x.id, x.value) for x in allProps])
        # This isn't all of them...but a fair amount
        self.assertEqual(allProps.has_key("DCE:4a23ad60-0b25-4121-a630-68803a498f75"), False)  # os_name
        self.assertEqual(allProps.has_key("DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b"), False) # processor_name
        self.assertEqual(allProps.has_key("DCE:7f36cdfb-f828-4e4f-b84f-446e17f1a85b"), False) # DeviceKind
        self.assertEqual(allProps.has_key("DCE:64303822-4c67-4c04-9a5c-bf670f27cf39"), False) # RunsAs
        self.assertEqual(allProps.has_key("DCE:021f10cf-7a05-46ec-a507-04b513b84bd4"), False) # HasXMIDAS

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], ds)

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 1)

        self.assertEqual(app._get_started(), False)
        app.start()
        self.assertEqual(app._get_started(), True)

        # Verify that capacity was allocated
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000-5000)
        self.assertEqual(bogoMips.value._v, 100000000-1000)

        app.stop()
        self.assertEqual(app._get_started(), False)
        app.releaseObject()
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        # Verify that capacity was deallocated
        memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000)
        self.assertEqual(bogoMips.value._v, 100000000)

        domMgr.uninstallApplication(appFact._get_identifier())

