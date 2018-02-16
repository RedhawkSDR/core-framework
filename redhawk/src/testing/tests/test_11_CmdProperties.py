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

from omniORB import any
import unittest
from _unitTestHelpers import scatest
from ossie.cf import CF
from omniORB import CORBA
from ossie.utils import redhawk
import struct

class TestAllTypes(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.dom = redhawk.attach(scatest.getTestDomainName())
        self._app = None

    def tearDown(self):
        if self._app:
            self._app.releaseObject()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def preconditions(self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")
        self.assertEquals(self._app, None, "Stale application")

    def test_JustSADOverride(self):
        self.preconditions()
        self._app = self.dom.createApplication('/waveforms/cmdline_def_py_w/cmdline_def_py_w.sad.xml')
        self.assertNotEqual(self._app, None, "Application not created")
        comp_pid = int(self._app._get_componentProcessIds()[0].processId)
        fp=open('/proc/'+str(comp_pid)+'/cmdline','r')
        comp_contents = fp.read()
        fp.close()
        items = comp_contents.split('\x00')
        readonly_cmdline_idx = items.index('readonly_cmdline')
        readonly_cmdline_val = items[readonly_cmdline_idx+1]
        readwrite_cmdline_idx = items.index('readwrite_cmdline')
        readwrite_cmdline_val = items[readwrite_cmdline_idx+1]
        writeonly_cmdline_idx = items.index('writeonly_cmdline')
        writeonly_cmdline_val = items[writeonly_cmdline_idx+1]
        self.assertEquals(readonly_cmdline_val, 'd')
        self.assertEquals(readwrite_cmdline_val, 'e')
        self.assertEquals(writeonly_cmdline_val, 'f')
        self.assertEquals(self._app.comps[0].readonly_cmdline, 'd')
        self.assertEquals(self._app.comps[0].readwrite_cmdline, 'e')

    def test_CreateSADOverride(self):
        self.preconditions()
        self._app = self.dom.createApplication('/waveforms/cmdline_def_py_w/cmdline_def_py_w.sad.xml', 'test_app', {'readonly_cmdline':'g','readwrite_cmdline':'h','writeonly_cmdline':'i'})
        self.assertNotEqual(self._app, None, "Application not created")
        comp_pid = int(self._app._get_componentProcessIds()[0].processId)
        fp=open('/proc/'+str(comp_pid)+'/cmdline','r')
        comp_contents = fp.read()
        fp.close()
        items = comp_contents.split('\x00')
        readonly_cmdline_idx = items.index('readonly_cmdline')
        readonly_cmdline_val = items[readonly_cmdline_idx+1]
        readwrite_cmdline_idx = items.index('readwrite_cmdline')
        readwrite_cmdline_val = items[readwrite_cmdline_idx+1]
        writeonly_cmdline_idx = items.index('writeonly_cmdline')
        writeonly_cmdline_val = items[writeonly_cmdline_idx+1]
        self.assertEquals(readonly_cmdline_val, 'g')
        self.assertEquals(readwrite_cmdline_val, 'h')
        self.assertEquals(writeonly_cmdline_val, 'i')
        self.assertEquals(self._app.comps[0].readonly_cmdline, 'g')
        self.assertEquals(self._app.comps[0].readwrite_cmdline, 'h')

    def test_CleanJustSADOverride(self):
        self.preconditions()
        self._app = self.dom.createApplication('/waveforms/cmdline_py_w/cmdline_py_w.sad.xml')
        self.assertNotEqual(self._app, None, "Application not created")
        comp_pid = int(self._app._get_componentProcessIds()[0].processId)
        fp=open('/proc/'+str(comp_pid)+'/cmdline','r')
        comp_contents = fp.read()
        fp.close()
        items = comp_contents.split('\x00')
        readonly_cmdline_idx = items.index('readonly_cmdline')
        readonly_cmdline_val = items[readonly_cmdline_idx+1]
        readwrite_cmdline_idx = items.index('readwrite_cmdline')
        readwrite_cmdline_val = items[readwrite_cmdline_idx+1]
        writeonly_cmdline_idx = items.index('writeonly_cmdline')
        writeonly_cmdline_val = items[writeonly_cmdline_idx+1]
        self.assertEquals(readonly_cmdline_val, 'd')
        self.assertEquals(readwrite_cmdline_val, 'e')
        self.assertEquals(writeonly_cmdline_val, 'f')
        self.assertEquals(self._app.comps[0].readonly_cmdline, 'd')
        self.assertEquals(self._app.comps[0].readwrite_cmdline, 'e')

    def test_CleanCreateSADOverride(self):
        self.preconditions()
        self._app = self.dom.createApplication('/waveforms/cmdline_py_w/cmdline_py_w.sad.xml', 'test_app', {'readonly_cmdline':'g','readwrite_cmdline':'h','writeonly_cmdline':'i'})
        self.assertNotEqual(self._app, None, "Application not created")
        comp_pid = int(self._app._get_componentProcessIds()[0].processId)
        fp=open('/proc/'+str(comp_pid)+'/cmdline','r')
        comp_contents = fp.read()
        fp.close()
        items = comp_contents.split('\x00')
        readonly_cmdline_idx = items.index('readonly_cmdline')
        readonly_cmdline_val = items[readonly_cmdline_idx+1]
        readwrite_cmdline_idx = items.index('readwrite_cmdline')
        readwrite_cmdline_val = items[readwrite_cmdline_idx+1]
        writeonly_cmdline_idx = items.index('writeonly_cmdline')
        writeonly_cmdline_val = items[writeonly_cmdline_idx+1]
        self.assertEquals(readonly_cmdline_val, 'g')
        self.assertEquals(readwrite_cmdline_val, 'h')
        self.assertEquals(writeonly_cmdline_val, 'i')
        self.assertEquals(self._app.comps[0].readonly_cmdline, 'g')
        self.assertEquals(self._app.comps[0].readwrite_cmdline, 'h')


    def test_CleanCmdlineOverride(self):
        self.preconditions()
        self._app = self.dom.createApplication('/waveforms/no_property_value_w/no_property_value_w.sad.xml', 'test_app', {'readonly_cmdline':'g','readwrite_cmdline':'h','writeonly_cmdline':'i'})
        self.assertNotEqual(self._app, None, "Application not created")
        comp_pid = int(self._app._get_componentProcessIds()[0].processId)
        fp=open('/proc/'+str(comp_pid)+'/cmdline','r')
        comp_contents = fp.read()
        fp.close()
        items = comp_contents.split('\x00')
        readonly_cmdline_idx = items.index('readonly_cmdline')
        readonly_cmdline_val = items[readonly_cmdline_idx+1]
        readwrite_cmdline_idx = items.index('readwrite_cmdline')
        readwrite_cmdline_val = items[readwrite_cmdline_idx+1]
        writeonly_cmdline_idx = items.index('writeonly_cmdline')
        writeonly_cmdline_val = items[writeonly_cmdline_idx+1]
        self.assertEquals(readonly_cmdline_val, 'g')
        self.assertEquals(readwrite_cmdline_val, 'h')
        self.assertEquals(writeonly_cmdline_val, 'i')
        self.assertEquals(self._app.comps[0].readonly_cmdline, 'g')
        self.assertEquals(self._app.comps[0].readwrite_cmdline, 'h')

    def test_CmdlineOverride(self):
        self.preconditions()
        self._app = self.dom.createApplication('/waveforms/PRF_def_property_w/PRF_def_property_w.sad.xml', 'test_app', {'readonly_cmdline':'g','readwrite_cmdline':'h','writeonly_cmdline':'i'})
        self.assertNotEqual(self._app, None, "Application not created")
        comp_pid = int(self._app._get_componentProcessIds()[0].processId)
        fp=open('/proc/'+str(comp_pid)+'/cmdline','r')
        comp_contents = fp.read()
        fp.close()
        items = comp_contents.split('\x00')
        readonly_cmdline_idx = items.index('readonly_cmdline')
        readonly_cmdline_val = items[readonly_cmdline_idx+1]
        readwrite_cmdline_idx = items.index('readwrite_cmdline')
        readwrite_cmdline_val = items[readwrite_cmdline_idx+1]
        writeonly_cmdline_idx = items.index('writeonly_cmdline')
        writeonly_cmdline_val = items[writeonly_cmdline_idx+1]
        self.assertEquals(readonly_cmdline_val, 'g')
        self.assertEquals(readwrite_cmdline_val, 'h')
        self.assertEquals(writeonly_cmdline_val, 'i')
        self.assertEquals(self._app.comps[0].readonly_cmdline, 'g')
        self.assertEquals(self._app.comps[0].readwrite_cmdline, 'h')


    def test_NoCmdlineOverride(self):
        self.preconditions()
        self._app = self.dom.createApplication('/waveforms/PRF_def_property_w/PRF_def_property_w.sad.xml')
        self.assertNotEqual(self._app, None, "Application not created")
        comp_pid = int(self._app._get_componentProcessIds()[0].processId)
        fp=open('/proc/'+str(comp_pid)+'/cmdline','r')
        comp_contents = fp.read()
        fp.close()
        items = comp_contents.split('\x00')
        readonly_cmdline_idx = items.index('readonly_cmdline')
        readonly_cmdline_val = items[readonly_cmdline_idx+1]
        readwrite_cmdline_idx = items.index('readwrite_cmdline')
        readwrite_cmdline_val = items[readwrite_cmdline_idx+1]
        writeonly_cmdline_idx = items.index('writeonly_cmdline')
        writeonly_cmdline_val = items[writeonly_cmdline_idx+1]
        self.assertEquals(readonly_cmdline_val, 'a')
        self.assertEquals(readwrite_cmdline_val, 'b')
        self.assertEquals(writeonly_cmdline_val, 'c')
        self.assertEquals(self._app.comps[0].readonly_cmdline, 'a')
        self.assertEquals(self._app.comps[0].readwrite_cmdline, 'b')

