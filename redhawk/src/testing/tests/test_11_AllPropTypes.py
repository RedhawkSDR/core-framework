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
import struct
from _unitTestHelpers import runtestHelpers

java_support = runtestHelpers.haveJavaSupport('../Makefile')

class TestAllTypes(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self._app = None

    def tearDown(self):
        if self._app:
            self._app.stop()
            self._app.releaseObject()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def preconditions(self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")
        self.assertNotEqual(self._app, None, "Application not created")

    def launchApplication(self, language):
        if self._domMgr:
            try:
                sadpath = "/waveforms/Test"+language+"Props/"+language+"AllPropTypes.sad.xml"
                self._domMgr.installApplication(sadpath)
                # Gets the proper appFact
                for curr in self._domMgr._get_applicationFactories():
                    if curr._get_name().find(language) != -1:
                        appFact = curr
                self._app = appFact.create(appFact._get_name(), [], [])
            except:
                pass

    def test_AllPropTypes(self):
        languages = ['Cpp', 'Python']
        if java_support:
            languages.append('Java')
        for lang in languages:
            self.launchApplication(lang)
            self.preconditions()
            res = self._app.query([])
            for r in res:
                self._app.configure([r])

    def checkValue(self, results, prop_id, value):
        for r in results:
            if r.id == prop_id:
                if prop_id == 'struct_vars':
                    self.assertEqual(r.value.value()[0].value.value(), value)
                elif prop_id == 'struct_seq':
                    found_id = False
                    for curr in r.value.value()[0].value():
                        if curr.id == 'struct_seq_string':
                            found_id = True
                            self.assertEqual(curr.value.value(), value)
                    self.assertEqual(found_id, True)
                elif prop_id.find('simple_sequence') != -1:
                    self.assertEqual(r.value.value()[0], value)
                else:
                    self.assertEqual(r.value.value(), value)

    def test_AllPropTypeCallbacks(self):
        languages = ['Cpp', 'Python']
        for lang in languages:
            self.launchApplication(lang)
            self.preconditions()
            res = self._app.query([])
            for r in res:
                self._app.configure([r])
            res = self._app.query([])

            self.checkValue(res, 'simple_string', '42')
            self.checkValue(res, 'simple_boolean', True)
            self.checkValue(res, 'simple_ulong', 43)
            self.checkValue(res, 'simple_objref', '44')
            self.checkValue(res, 'simple_short', 45)
            self.checkValue(res, 'simple_float', 46.0)
            self.checkValue(res, 'simple_octet', 47)
            self.checkValue(res, 'simple_char', struct.pack('b', 48))
            self.checkValue(res, 'simple_ushort', 49)
            self.checkValue(res, 'simple_double', 50.0)
            self.checkValue(res, 'simple_long', 51)
            self.checkValue(res, 'simple_longlong', 52)
            self.checkValue(res, 'simple_ulonglong', 53)
            self.checkValue(res, 'simple_sequence_string', '54')
            self.checkValue(res, 'simple_sequence_boolean', True)
            self.checkValue(res, 'simple_sequence_ulong', 55)
#            self.checkValue(res, 'simple_sequence_objref', '56')   Broken in python
            self.checkValue(res, 'simple_sequence_short', 57)
            self.checkValue(res, 'simple_sequence_float', 58)
#            self.checkValue(res, 'simple_sequence_octet', struct.pack('B', 59))    Broken in python
#            self.checkValue(res, 'simple_sequence_char', struct.pack('b', 60))     Borken in python
            self.checkValue(res, 'simple_sequence_ushort', 61)
            self.checkValue(res, 'simple_sequence_double', 62)
            self.checkValue(res, 'simple_sequence_long', 63)
            self.checkValue(res, 'simple_sequence_longlong', 64)
            self.checkValue(res, 'simple_sequence_ulonglong', 65)
            self.checkValue(res, 'struct_vars', '66')
            self.checkValue(res, 'struct_seq', '67')

