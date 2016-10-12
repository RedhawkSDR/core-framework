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
from _unitTestHelpers import scatest

#In testing the persona, allocateCapacity() in PersonaDevice.cpp has allcoateCapacity set to True and 
#the hwLoadRequest() must have the struct CF::Properties request set
class PersonaTest(scatest.CorbaTestCase):
    def setUp(self):
        self._nodebooter, self._domMgr = self.launchDomainManager()

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)

    def launchNode(self, node):
        self._devBooter,self._devMgr = self.launchDeviceManager("/nodes/%s/DeviceManager.dcd.xml" % (node),)

    def checkRegisteredDevices(self,numDevices):
        self.assertEqual(len(self._devMgr._get_registeredDevices()),numDevices)

    def allocate(self):
        deviceNumber = 1
        for device in self._devMgr._get_registeredDevices():
            if "ProgrammableDevice_1" not in device._get_identifier():
                device.allocateCapacity([])
                if deviceNumber == 1:
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[1]._get_usageState()),"IDLE")
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[1]._get_adminState()),"UNLOCKED")
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[2]._get_usageState()),"BUSY")
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[2]._get_adminState()),"LOCKED")
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[3]._get_usageState()),"BUSY")
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[3]._get_adminState()),"LOCKED")
                elif deviceNumber == 2:
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[1]._get_usageState()),"BUSY")
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[1]._get_adminState()),"LOCKED")
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[2]._get_usageState()),"IDLE")
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[2]._get_adminState()),"UNLOCKED")
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[3]._get_usageState()),"BUSY")
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[3]._get_adminState()),"LOCKED")
                elif deviceNumber == 3:
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[1]._get_usageState()),"BUSY")
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[1]._get_adminState()),"LOCKED")
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[2]._get_usageState()),"BUSY")
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[2]._get_adminState()),"LOCKED")
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[3]._get_usageState()),"IDLE")
                    self.assertEquals(str(self._devMgr._get_registeredDevices()[3]._get_adminState()),"UNLOCKED")

                device.deallocateCapacity([])
                self.assertEquals(str(self._devMgr._get_registeredDevices()[1]._get_usageState()),"IDLE")
                self.assertEquals(str(self._devMgr._get_registeredDevices()[1]._get_adminState()),"UNLOCKED")
                self.assertEquals(str(self._devMgr._get_registeredDevices()[2]._get_usageState()),"IDLE")
                self.assertEquals(str(self._devMgr._get_registeredDevices()[2]._get_adminState()),"UNLOCKED")
                self.assertEquals(str(self._devMgr._get_registeredDevices()[3]._get_usageState()),"IDLE")
                self.assertEquals(str(self._devMgr._get_registeredDevices()[3]._get_adminState()),"UNLOCKED")
 
                deviceNumber += 1

    def test_CPP_Persona(self):
        self.launchNode("PersonaNode")
        self.checkRegisteredDevices(4)
        self.allocate()


