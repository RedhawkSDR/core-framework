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
import os
import re
import time

from _unitTestHelpers import scatest

def is_regexp_in_file_lines(fpath, regexp):
    lines = open(fpath).readlines()  # let it raise
    for line in lines:
        match = re.search(regexp, line)
        if match is not None:
            return True


#In testing the persona, allocateCapacity() in PersonaDevice.cpp has allcoateCapacity set to True and 
#the hwLoadRequest() must have the struct CF::Properties request set
class PersonaTest(scatest.CorbaTestCase):
    def setUp(self):
        self._nodebooter, self._domMgr = self.launchDomainManager()
        self._tempfiles = []

    def tearDown(self):
        self._delete_temp_files()
        scatest.CorbaTestCase.tearDown(self)

    def launchNode(self, node, debug=-1, loggingURI=None):
        self._devBooter,self._devMgr = self.launchDeviceManager("/nodes/%s/DeviceManager.dcd.xml" % (node), debug=debug, loggingURI=loggingURI)

    def checkRegisteredDevices(self,numDevices):
        scatest.verifyDeviceLaunch(self, self._devMgr, numDevices)

    def _delete_temp_files(self):
        for tempfile in self._tempfiles:
            try:
                os.unlink(tempfile)
            except:
                pass

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

    def _test_Persona_log(self, fpath_log, log_level):
        """
        These checks are simplified by knowing what log messages will occur, and their order.
        For both ProgrammableDevice and PersonaDevice, several DEBUG messages happen first,
        followed closely by these INFO messages:
            for ProgrammableDevice_1:
                yyyy-mm-dd hh:mm:ss INFO  ProgrammableDevice_1.system.Device:1188 - DEV-ID:test_Persona_cpp_log_debug:ProgrammableDevice_1 Requesting IDM CHANNEL IDM_Channel
            for PersonaDevice_1:
                yyyy-mm-dd hh:mm:ss INFO  PersonaDevice_1.system.Device:1188 - DEV-ID:test_Persona_cpp_log_debug:PersonaDevice_1 Requesting IDM CHANNEL IDM_Channel
        """
        self.launchNode('test_Persona_log', debug=log_level, loggingURI='dev/nodes/test_Persona_log/Persona.cfg')
        found_prog_dbg = False
        found_pers_dbg = False
        found_prog_inf = False
        found_pers_inf = False
        # These regexes must match the format defined in the log config file.
        re_prog_dbg = r'^.{20}' + r'{0}'.format('DEBUG ProgrammableDevice_1')
        re_pers_dbg = r'^.{20}' + r'{0}'.format('DEBUG PersonaDevice_1')
        re_prog_inf = r'^.{20}' + r'{0}'.format('INFO  ProgrammableDevice_1')
        re_pers_inf = r'^.{20}' + r'{0}'.format('INFO  PersonaDevice_1')
        time_limit = 5
        start = time.time()
        while not (found_prog_inf and found_pers_inf):
            if time.time() > start + time_limit:
                break
            found_prog_dbg = is_regexp_in_file_lines(fpath_log, re_prog_dbg)
            found_pers_dbg = is_regexp_in_file_lines(fpath_log, re_pers_dbg)
            found_prog_inf = is_regexp_in_file_lines(fpath_log, re_prog_inf)
            found_pers_inf = is_regexp_in_file_lines(fpath_log, re_pers_inf)
            time.sleep(0.1)
        return found_prog_dbg, found_pers_dbg, found_prog_inf, found_pers_inf

    @scatest.requireLog4cxx
    def test_Persona_log_config_file(self):
        """
        Check that for both ProgrammableDevice and PersonaDevice:
        - a FileAppender is created via logging config file
        - log messages are written to the FileAppender's output file
        """
        fpath_log = '/var/tmp/Persona.log'  # Must match what is defined in log config file.
        self._tempfiles.append(fpath_log)
        self._delete_temp_files()  # ensure no leftover log file exists

        log_level = 4  # DEBUG
        found_prog_dbg, found_pers_dbg, found_prog_inf, found_pers_inf = self._test_Persona_log(fpath_log, log_level=log_level)
        self.assertTrue(found_prog_dbg)
        self.assertTrue(found_pers_dbg)
        self.assertTrue(found_prog_inf)
        self.assertTrue(found_pers_inf)

    @scatest.requireLog4cxx
    def test_Persona_log_nodebooter_commandline(self):
        """
        Check that for both ProgrammableDevice and PersonaDevice:
        - the `-debug` flag of nodeBooter's command line affects the log level
        """
        fpath_log = '/var/tmp/Persona.log'  # Must match what is defined in log config file.
        self._tempfiles.append(fpath_log)
        self._delete_temp_files()  # ensure no leftover log file exists

        log_level = 3  # INFO
        found_prog_dbg, found_pers_dbg, found_prog_inf, found_pers_inf = self._test_Persona_log(fpath_log, log_level=log_level)
        self.assertFalse(found_prog_dbg)
        self.assertFalse(found_pers_dbg)
        self.assertTrue(found_prog_inf)
        self.assertTrue(found_pers_inf)

class PPTest(scatest.CorbaTestCase):
    def setUp(self):
        self._nodebooter, self._domMgr = self.launchDomainManager()

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)

    def launchNode(self, node):
        self._devBooter,self._devMgr = self.launchDeviceManager("/nodes/%s/DeviceManager.dcd.xml" % (node),)

    def test_CPP_ProgrammableAndPersona(self):
        self.launchNode("agg_test")
        devs = self._devMgr._get_registeredDevices()
        self.assertEquals(devs[0]._get_identifier(), 'agg_test:base_programmable_1')
        self.assertEquals(devs[1]._get_identifier(), 'agg_test:base_persona_1')
        self.assertEquals(devs[0]._get_compositeDevice(), None)
        self.assertEquals(devs[1]._get_compositeDevice()._get_identifier(), 'agg_test:base_programmable_1')
