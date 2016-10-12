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
import time
from ossie.utils import sb
from ossie.utils import redhawk

class CppPropertiesTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self._app = None
        if self._domMgr:
            try:
                sadpath = "/waveforms/TestCppProps/TestCppProps.sad.xml"
                self._domMgr.installApplication(sadpath)
                appFact = self._domMgr._get_applicationFactories()[0]
                self._app = appFact.create(appFact._get_name(), [], [])
            except:
                pass

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

    def test_LegacyPropertyCallbacks(self):
        self.preconditions()

        # The TestCppProps component is also the assembly controller, so we can
        # use the TestableObject interface via the Application object to run
        # the property callback tests for member functions...
        props = self._app.query([])
        for result in self._app.runTest(0, props):
            self.assert_(result.value._v)

        # ...and static functions.
        for result in self._app.runTest(1, props):
            self.assert_(result.value._v)

    def test_NilProperty(self):
        self.preconditions()

        # Use the prop_str property for this test.
        prop = CF.DataType("DCE:4e7c1977-5f53-4061-bae7-cb8c1072f4b7", any.to_any(None))

        # Check that the default value is not 'None'.
        propVal = self._app.query([prop])[0]
        self.assertNotEqual(propVal.value.value(), None)

        # Enable nil values for the property.
        prop.value = any.to_any(True)
        self._app.runTest(2, [prop])

        # Explicitly configure the value to None and verify that it reports back
        # as 'None'.
        prop.value = any.to_any(None)
        self._app.configure([prop])
        propVal = self._app.query([prop])[0]
        self.assertEqual(propVal.value.value(), None)

        # Clear the property's nil status and check that it no longer returns
        # 'None'.
        prop.value = any.to_any(False)
        self._app.runTest(3, [prop])
        propVal = self._app.query([prop])[0]
        self.assertNotEqual(propVal.value.value(), None)

    def test_fullQuery(self):
        devBooter_2, self._devMgr_2 = self.launchDeviceManager("/nodes/props_test_node/DeviceManager.dcd.xml")
        dev = self._devMgr_2._get_registeredDevices()[0]
        results = dev.query([])
        ids = []
        for result in results:
            ids.append(result.id)
        self.assertEqual("DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb" in ids, True)
        self.assertEqual("DCE:1d9bed5e-1e6c-4607-9391-0e692f8fd1ae" in ids, True)
        self.assertEqual("DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d" in ids, True)
        self.assertEqual("execparam_readwrite" in ids, True)
        self.assertEqual("simple_config" in ids, True)
        self.assertEqual(len(results), 5)

    def test_NilRead(self):
        #nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        #self.assertNotEqual(devMgr, None)

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 1)

        self._domMgr.installApplication("/waveforms/ticket_2093/ticket_2093.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 2)
        self.assertEqual(len(self._domMgr._get_applications()), 1)

        factories = dict((af._get_name(), af) for af in self._domMgr._get_applicationFactories())
        appFact = factories['ticket_2093']
        app = appFact.create(appFact._get_name(), [], [])

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 2)
        self.assertEqual(len(self._domMgr._get_applications()), 2)

        props = app.query([])
        # Sequences cannot be nil, just a zero-length array
        #self.assertEqual(props[0].value._t, CORBA.TC_null)
        self.assertEqual(props[1].value._t, CORBA.TC_null)
        self.assertEqual(props[2].value._t, CORBA.TC_null)
        # Sequences cannot be nil, just a zero-length array
        #self.assertEqual(props[3].value._t, CORBA.TC_null)

        ## Clean-up
        app.stop()
        app.releaseObject()

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 2)
        self.assertEqual(len(self._domMgr._get_applications()), 1)

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_InvalidConfigureProperty(self):
        self.preconditions()

        # Use the prop_str property for this test.
        props = [CF.DataType("not valide property id", any.to_any(None))]
        props.append(CF.DataType("DCE:4e7c1977-5f53-4061-bae7-cb8c1072f4b7", any.to_any(None)))

        # Make sure that the appropriate exception is raised
        self.assertRaises(CF.PropertySet.PartialConfiguration, self._app.configure, props)

        # Make sure that the app is still good
        prop = CF.DataType("DCE:4e7c1977-5f53-4061-bae7-cb8c1072f4b7", any.to_any(None))
        # Check that the default value is not 'None'.
        propVal = self._app.query([prop])[0]
        self.assertNotEqual(propVal.value.value(), None)

    def test_OptionalPropertiesInStruct(self):
        comp = sb.launch('TestCppOptionalProps')
        octet_val = struct.pack('B', 0) + struct.pack('B', 255)
        my_struct = CF.DataType(id='my_struct', value=any.to_any([
                                CF.DataType(id='struct_octet', value=CORBA.Any(CORBA.TC_octet, 255)),
                                CF.DataType(id='struct_short', value=CORBA.Any(CORBA.TC_short, 32767)),
                                CF.DataType(id='struct_ushort', value=CORBA.Any(CORBA.TC_ushort, 65535)),
                                CF.DataType(id='struct_long', value=CORBA.Any(CORBA.TC_long, 2147483647)),
                                CF.DataType(id='struct_ulong', value=CORBA.Any(CORBA.TC_ulong, 4294967295)),
                                CF.DataType(id='struct_longlong', value=CORBA.Any(CORBA.TC_longlong, 9223372036854775807L)),
                                CF.DataType(id='struct_ulonglong', value=CORBA.Any(CORBA.TC_ulonglong, 18446744073709551615L)),
 				CF.DataType(id='struct_string', value=CORBA.Any(CORBA.TC_string, "new string")),
				CF.DataType(id='struct_seq_octet', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/OctetSeq:1.0"), octet_val)),
                                CF.DataType(id='struct_seq_short', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/ShortSeq:1.0"), [0, 32767])),
                                CF.DataType(id='struct_seq_ushort', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/UShortSeq:1.0"), [0, 65535])),
                                CF.DataType(id='struct_seq_long', value=any.to_any([0, 2147483647])),
                                CF.DataType(id='struct_seq_ulong', value=any.to_any([0, 4294967295])),
                                CF.DataType(id='struct_seq_longlong', value=any.to_any([0, 9223372036854775807L])),
                                #CF.DataType(id='struct_seq_ulonglong', value=any.to_any([0, 9223372036854775807L]))
                                ]))
        comp.configure([my_struct])
        res = comp.query([])
        for r in res:
            if r.id == 'my_struct':
                val = r.value.value()
                for v in val:
                    if v.id == 'struct_octet':
                        self.assertEquals(v.value.value(), 255)
                    elif v.id == 'struct_short':
                        self.assertEquals(v.value.value(), 32767)
                    elif v.id == 'struct_ushort':
                        self.assertEquals(v.value.value(), 65535)
                    elif v.id == 'struct_long':
                        self.assertEquals(v.value.value(), 2147483647)
                    elif v.id == 'struct_ulong':
                        self.assertEquals(v.value.value(), 4294967295)
                    elif v.id == 'struct_longlong':
                        self.assertEquals(v.value.value(), 9223372036854775807L)
                    elif v.id == 'struct_ulonglong':
                        self.assertEquals(v.value.value(), 18446744073709551615L)
		    elif v.id == 'struct_seq_octet':
			# Octets need to be unpacked
                	stored_vals = v.value.value()
                	vals = []
                	for num in stored_vals:
                    	    curr = struct.unpack('B', num)
                    	    vals.append(curr[0])
                	self.assertEquals(vals[0], 0)
                	self.assertEquals(vals[1], 255)
		    elif v.id == 'struct_seq_short':
			self.assertEquals(v.value.value(), [0, 32767])
		    elif v.id == 'struct_seq_ushort':
			self.assertEquals(v.value.value(), [0, 65535])
		    elif v.id == 'struct_seq_long':
			self.assertEquals(v.value.value(), [0, 2147483647])
		    elif v.id == 'struct_seq_ulong':
			self.assertEquals(v.value.value(), [0, 4294967295])
		    elif v.id == 'struct_seq_longlong':
			self.assertEquals(v.value.value(), [0, 9223372036854775807L])
		    #elif v.id == 'struct_seq_ulonglong':
		#	self.assertEquals(v.value.value(), [0, 9223372036854775807L])

        # Configure only certain properties
        my_struct = CF.DataType(id='my_struct', value=any.to_any([
                                CF.DataType(id='struct_octet', value=CORBA.Any(CORBA.TC_octet, 255)),
                                CF.DataType(id='struct_short', value=CORBA.Any(CORBA.TC_short, 32767)),
                                CF.DataType(id='struct_ushort', value=CORBA.Any(CORBA.TC_ushort, 65535)),
                                #CF.DataType(id='struct_long', value=CORBA.Any(CORBA.TC_long, 2147483647)),
                                CF.DataType(id='struct_ulong', value=CORBA.Any(CORBA.TC_ulong, 4294967295)),
                                CF.DataType(id='struct_longlong', value=CORBA.Any(CORBA.TC_longlong, 9223372036854775807L)),
                                #CF.DataType(id='struct_ulonglong', value=CORBA.Any(CORBA.TC_ulonglong, 18446744073709551615L)),
 				CF.DataType(id='struct_string', value=CORBA.Any(CORBA.TC_string, "new string")),
				CF.DataType(id='struct_seq_octet', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/OctetSeq:1.0"), octet_val)),
                                CF.DataType(id='struct_seq_short', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/ShortSeq:1.0"), [0, 32767])),
                                #CF.DataType(id='struct_seq_ushort', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/UShortSeq:1.0"), [0, 65535])),
                                CF.DataType(id='struct_seq_long', value=any.to_any([0, 2147483647])),
                                CF.DataType(id='struct_seq_ulong', value=any.to_any([0, 4294967295])),
                                CF.DataType(id='struct_seq_longlong', value=any.to_any([0, 9223372036854775807L])),
                                #CF.DataType(id='struct_seq_ulonglong', value=any.to_any([0, 9223372036854775807L]))
                                ]))
        comp.configure([my_struct])

        # Run a test inside the component
        comp.runTest(0, comp.query([]))


class CppPropertiesRangeTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self._app = None
        if self._domMgr:
            try:
                sadpath = "/waveforms/TestCppPropsRange/TestCppPropsRange.sad.xml"
                self._domMgr.installApplication(sadpath)
                appFact = self._domMgr._get_applicationFactories()[0]
                self._app = appFact.create(appFact._get_name(), [], [])
            except:
                pass

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

    def test_cppPropsRangeSimple(self):
        self.preconditions()

        # Test upper bound
        my_octet = CF.DataType(id='my_octet', value=CORBA.Any(CORBA.TC_octet, 255))
        my_short = CF.DataType(id='my_short', value=CORBA.Any(CORBA.TC_short, 32767))
        my_ushort = CF.DataType(id='my_ushort', value=CORBA.Any(CORBA.TC_ushort, 65535))
        my_long = CF.DataType(id='my_long', value=CORBA.Any(CORBA.TC_long, 2147483647))
        my_ulong = CF.DataType(id='my_ulong', value=CORBA.Any(CORBA.TC_ulong, 4294967295))
        my_longlong = CF.DataType(id='my_longlong', value=CORBA.Any(CORBA.TC_longlong, 9223372036854775807L))
        my_ulonglong = CF.DataType(id='my_ulonglong', value=CORBA.Any(CORBA.TC_ulonglong, 18446744073709551615L))
        self._app.configure([my_octet, my_short, my_ushort, my_long, my_ulong, my_longlong, my_ulonglong])
        res = self._app.query([])
        for r in res:
            if r.id == 'my_octet':
                self.assertEquals(r.value.value(), 255)
            elif r.id == 'my_short':
                self.assertEquals(r.value.value(), 32767)
            elif r.id == 'my_ushort':
                self.assertEquals(r.value.value(), 65535)
            elif r.id == 'my_long':
                self.assertEquals(r.value.value(), 2147483647)
            elif r.id == 'my_ulong':
                self.assertEquals(r.value.value(), 4294967295)
            elif r.id == 'my_longlong':
                self.assertEquals(r.value.value(), 9223372036854775807)
            elif r.id == 'my_ulonglong':
                self.assertEquals(r.value.value(), 18446744073709551615)

        # Test lower bound
        my_octet = CF.DataType(id='my_octet', value=CORBA.Any(CORBA.TC_octet, 0))
        my_short = CF.DataType(id='my_short', value=CORBA.Any(CORBA.TC_short, -32768))
        my_ushort = CF.DataType(id='my_ushort', value=CORBA.Any(CORBA.TC_ushort, 0))
        my_long = CF.DataType(id='my_long', value=CORBA.Any(CORBA.TC_long, -2147483648))
        my_ulong = CF.DataType(id='my_ulong', value=CORBA.Any(CORBA.TC_ulong, 0))
        my_longlong = CF.DataType(id='my_longlong', value=CORBA.Any(CORBA.TC_longlong, -9223372036854775808L))
        my_ulonglong = CF.DataType(id='my_ulonglong', value=CORBA.Any(CORBA.TC_ulonglong, 0))
        self._app.configure([my_octet, my_short, my_ushort, my_long, my_ulong, my_longlong, my_ulonglong])
        res = self._app.query([])
        for r in res:
            if r.id == 'my_octet':
                self.assertEquals(r.value.value(), 0)
            elif r.id == 'my_short':
                self.assertEquals(r.value.value(), -32768)
            elif r.id == 'my_ushort':
                self.assertEquals(r.value.value(), 0)
            elif r.id == 'my_long':
                self.assertEquals(r.value.value(), -2147483648)
            elif r.id == 'my_ulong':
                self.assertEquals(r.value.value(), 0)
            elif r.id == 'my_longlong':
                self.assertEquals(r.value.value(), -9223372036854775808)
            elif r.id == 'my_ulonglong':
                self.assertEquals(r.value.value(), 0)

    def test_cppPropsRangeStruct(self):
        self.preconditions()

        # Test upper bounds
	octet_val = struct.pack('B', 0) + struct.pack('B', 255)
        my_struct = CF.DataType(id='my_struct', value=any.to_any([
                                CF.DataType(id='struct_octet', value=CORBA.Any(CORBA.TC_octet, 255)),
                                CF.DataType(id='struct_short', value=CORBA.Any(CORBA.TC_short, 32767)),
                                CF.DataType(id='struct_ushort', value=CORBA.Any(CORBA.TC_ushort, 65535)),
                                CF.DataType(id='struct_long', value=CORBA.Any(CORBA.TC_long, 2147483647)),
                                CF.DataType(id='struct_ulong', value=CORBA.Any(CORBA.TC_ulong, 4294967295)),
                                CF.DataType(id='struct_longlong', value=CORBA.Any(CORBA.TC_longlong, 9223372036854775807L)),
                                CF.DataType(id='struct_ulonglong', value=CORBA.Any(CORBA.TC_ulonglong, 18446744073709551615L)),
				CF.DataType(id='struct_seq_octet', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/OctetSeq:1.0"), octet_val)),
                                CF.DataType(id='struct_seq_short', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/ShortSeq:1.0"), [0, 32767])),
                                CF.DataType(id='struct_seq_ushort', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/UShortSeq:1.0"), [0, 65535])),
                                CF.DataType(id='struct_seq_long', value=any.to_any([0, 2147483647])),
                                CF.DataType(id='struct_seq_ulong', value=any.to_any([0, 4294967295])),
                                CF.DataType(id='struct_seq_longlong', value=any.to_any([0, 9223372036854775807L])),
                                #CF.DataType(id='struct_seq_ulonglong', value=any.to_any([0, 9223372036854775807L]))
                                ]))
        self._app.configure([my_struct])
        res = self._app.query([])
        for r in res:
            if r.id == 'my_struct':
                val = r.value.value()
                for v in val:
                    if v.id == 'struct_octet':
                        self.assertEquals(v.value.value(), 255)
                    elif v.id == 'struct_short':
                        self.assertEquals(v.value.value(), 32767)
                    elif v.id == 'struct_ushort':
                        self.assertEquals(v.value.value(), 65535)
                    elif v.id == 'struct_long':
                        self.assertEquals(v.value.value(), 2147483647)
                    elif v.id == 'struct_ulong':
                        self.assertEquals(v.value.value(), 4294967295)
                    elif v.id == 'struct_longlong':
                        self.assertEquals(v.value.value(), 9223372036854775807L)
                    elif v.id == 'struct_ulonglong':
                        self.assertEquals(v.value.value(), 18446744073709551615L)
		    elif v.id == 'struct_seq_octet':
			# Octets need to be unpacked
                	stored_vals = v.value.value()
                	vals = []
                	for num in stored_vals:
                    	    curr = struct.unpack('B', num)
                    	    vals.append(curr[0])
                	self.assertEquals(vals[0], 0)
                	self.assertEquals(vals[1], 255)
		    elif v.id == 'struct_seq_short':
			self.assertEquals(v.value.value(), [0, 32767])
		    elif v.id == 'struct_seq_ushort':
			self.assertEquals(v.value.value(), [0, 65535])
		    elif v.id == 'struct_seq_long':
			self.assertEquals(v.value.value(), [0, 2147483647])
		    elif v.id == 'struct_seq_ulong':
			self.assertEquals(v.value.value(), [0, 4294967295])
		    elif v.id == 'struct_seq_longlong':
			self.assertEquals(v.value.value(), [0, 9223372036854775807L])
		    #elif v.id == 'struct_seq_ulonglong':
		#	self.assertEquals(v.value.value(), [0, 9223372036854775807L])

        # Test lower bounds
        my_struct = CF.DataType(id='my_struct', value=any.to_any([
                                CF.DataType(id='struct_octet', value=CORBA.Any(CORBA.TC_octet, 0)),
                                CF.DataType(id='struct_short', value=CORBA.Any(CORBA.TC_short, -32768)),
                                CF.DataType(id='struct_ushort', value=CORBA.Any(CORBA.TC_ushort, 0)),
                                CF.DataType(id='struct_long', value=CORBA.Any(CORBA.TC_long, -2147483648)),
                                CF.DataType(id='struct_ulong', value=CORBA.Any(CORBA.TC_ulong, 0)),
                                CF.DataType(id='struct_longlong', value=CORBA.Any(CORBA.TC_longlong, -9223372036854775808L)),
                                CF.DataType(id='struct_ulonglong', value=CORBA.Any(CORBA.TC_ulonglong, 0)),
				CF.DataType(id='struct_seq_octet', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/OctetSeq:1.0"), octet_val)),
				CF.DataType(id='struct_seq_short', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/ShortSeq:1.0"), [-32768, 32767])),
                                CF.DataType(id='struct_seq_ushort', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/UShortSeq:1.0"), [0, 65535])),
                                CF.DataType(id='struct_seq_long', value=any.to_any([-2147483648, 2147483647])),
                                CF.DataType(id='struct_seq_ulong', value=any.to_any([0, 4294967295])),
                                CF.DataType(id='struct_seq_longlong', value=any.to_any([-9223372036854775808L, 9223372036854775807L])),
                                #CF.DataType(id='struct_seq_ulonglong', value=any.to_any([0, 9223372036854775807]))
                                ]))
        self._app.configure([my_struct])
        res = self._app.query([])
        for r in res:
            if r.id == 'my_struct':
                val = r.value.value()
                for v in val:
                    if v.id == 'struct_octet':
                        self.assertEquals(v.value.value(), 0)
                    elif v.id == 'struct_short':
                        self.assertEquals(v.value.value(), -32768)
                    elif v.id == 'struct_ushort':
                        self.assertEquals(v.value.value(), 0)
                    elif v.id == 'struct_long':
                        self.assertEquals(v.value.value(), -2147483648)
                    elif v.id == 'struct_ulong':
                        self.assertEquals(v.value.value(), 0)
                    elif v.id == 'struct_longlong':
                        self.assertEquals(v.value.value(), -9223372036854775808L)
                    elif v.id == 'struct_ulonglong':
                        self.assertEquals(v.value.value(), 0)
		    elif v.id == 'struct_seq_octet':
			# Octets need to be unpacked
                	stored_vals = v.value.value()
                	vals = []
                	for num in stored_vals:
                    	    curr = struct.unpack('B', num)
                    	    vals.append(curr[0])
                	self.assertEquals(vals[0], 0)
                	self.assertEquals(vals[1], 255)
		    elif v.id == 'struct_seq_short':
			self.assertEquals(v.value.value(), [-32768, 32767])
		    elif v.id == 'struct_seq_ushort':
			self.assertEquals(v.value.value(), [0, 65535])
		    elif v.id == 'struct_seq_long':
			self.assertEquals(v.value.value(), [-2147483648, 2147483647])
		    elif v.id == 'struct_seq_ulong':
			self.assertEquals(v.value.value(), [0, 4294967295])
		    elif v.id == 'struct_seq_longlong':
			self.assertEquals(v.value.value(), [-9223372036854775808L, 9223372036854775807L])
		    #elif v.id == 'struct_seq_ulonglong':
		#	self.assertEquals(v.value.value(), [0, 9223372036854775807L])

    def test_cppPropsRangeSeq(self):
        self.preconditions()

        # Test upper and lower bounds
        octet_val = struct.pack('B', 0) + struct.pack('B', 255)
        seq_octet = CF.DataType(id='seq_octet', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/OctetSeq:1.0"),
                                                            octet_val
                                                            ))
        seq_short = CF.DataType(id='seq_short', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/ShortSeq:1.0"),
                                                            [-32768, 32767]
                                                            ))
        seq_ushort = CF.DataType(id='seq_ushort', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/UShortSeq:1.0"),
                                                            [0, 65535]
                                                            ))
        seq_long = CF.DataType(id='seq_long', value=any.to_any([-2147483648, 2147483647]))
        seq_ulong = CF.DataType(id='seq_ulong', value=any.to_any([0,4294967295]))
        seq_longlong = CF.DataType(id='seq_longlong', value=any.to_any([-9223372036854775808L, 9223372036854775807L]))
        self._app.configure([seq_octet, seq_short, seq_ushort, seq_long, seq_ulong, seq_longlong])

        res = self._app.query([])
        for r in res:
            if r.id == 'seq_octet':
                # Octets need to be unpacked
                stored_vals = r.value.value()
                vals = []
                for num in stored_vals:
                    curr = struct.unpack('B', num)
                    vals.append(curr[0])
                self.assertEquals(vals[0], 0)
                self.assertEquals(vals[1], 255)
            elif r.id == 'seq_short':
                self.assertEquals(r.value.value()[0], -32768)
                self.assertEquals(r.value.value()[1], 32767)
            elif r.id == 'seq_ushort':
                self.assertEquals(r.value.value()[0], 0)
                self.assertEquals(r.value.value()[1], 65535)
            elif r.id == 'seq_long':
                self.assertEquals(r.value.value()[0], -2147483648)
                self.assertEquals(r.value.value()[1], 2147483647)
            elif r.id == 'seq_ulong':
                self.assertEquals(r.value.value()[0], 0)
                self.assertEquals(r.value.value()[1], 4294967295)
            elif r.id == 'seq_longlong':
                self.assertEquals(r.value.value()[0], -9223372036854775808L)
                self.assertEquals(r.value.value()[1], 9223372036854775807L)

    def test_cppPropsRangeStructSeq(self):
        self.preconditions()

        # Struct with upper bound
	octet_val = struct.pack('B', 0) + struct.pack('B', 255)
        upper = CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), [
                            CF.DataType(id='ss_octet', value=CORBA.Any(CORBA.TC_octet, 255)),
                            CF.DataType(id='ss_short', value=CORBA.Any(CORBA.TC_short, 32767)),
                            CF.DataType(id='ss_ushort', value=CORBA.Any(CORBA.TC_ushort, 65535)),
                            CF.DataType(id='ss_long', value=CORBA.Any(CORBA.TC_long, 2147483647)),
                            CF.DataType(id='ss_ulong', value=CORBA.Any(CORBA.TC_ulong, 4294967295)),
                            CF.DataType(id='ss_longlong', value=CORBA.Any(CORBA.TC_longlong, 9223372036854775807L)),
                            CF.DataType(id='ss_ulonglong', value=CORBA.Any(CORBA.TC_ulonglong, 18446744073709551615L)),
			    CF.DataType(id='ss_seq_octet', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/OctetSeq:1.0"), octet_val)),
                            CF.DataType(id='ss_seq_short', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/ShortSeq:1.0"), [0, 32767])),
                            CF.DataType(id='ss_seq_ushort', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/UShortSeq:1.0"), [0, 65535])),
                	    CF.DataType(id='ss_seq_long', value=any.to_any([0, 2147483647])),
                	    CF.DataType(id='ss_seq_ulong', value=any.to_any([0, 4294967295])),
                	    CF.DataType(id='ss_seq_longlong', value=any.to_any([0, 9223372036854775807L])),
                	    #CF.DataType(id='ss_seq_ulonglong', value=any.to_any([0, 9223372036854775807L]))
			])
        # Struct with lower bound
        lower = CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), [
                            CF.DataType(id='ss_octet', value=CORBA.Any(CORBA.TC_octet, 0)),
                            CF.DataType(id='ss_short', value=CORBA.Any(CORBA.TC_short, -32768)),
                            CF.DataType(id='ss_ushort', value=CORBA.Any(CORBA.TC_ushort, 0)),
                            CF.DataType(id='ss_long', value=CORBA.Any(CORBA.TC_long, -2147483648)),
                            CF.DataType(id='ss_ulong', value=CORBA.Any(CORBA.TC_ulong, 0)),
                            CF.DataType(id='ss_longlong', value=CORBA.Any(CORBA.TC_longlong, -9223372036854775808L)),
                            CF.DataType(id='ss_ulonglong', value=CORBA.Any(CORBA.TC_ulonglong, 0)),
			    CF.DataType(id='ss_seq_octet', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/OctetSeq:1.0"), octet_val)),
                            CF.DataType(id='ss_seq_short', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/ShortSeq:1.0"), [-32768, 32767])),
                            CF.DataType(id='ss_seq_ushort', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/UShortSeq:1.0"), [0, 65535])),
                            CF.DataType(id='ss_seq_long', value=any.to_any([-2147483648, 2147483647])),
                            CF.DataType(id='ss_seq_ulong', value=any.to_any([0, 4294967295])),
                            CF.DataType(id='ss_seq_longlong', value=any.to_any([-9223372036854775808L, 9223372036854775807L])),
                            #CF.DataType(id='ss_seq_ulonglong', value=any.to_any([0, 9223372036854775807L]))
			])

        my_structseq = CF.DataType(id='my_structseq',
                value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/AnySeq:1.0"),
                        [ upper , lower ]
                        ))
        self._app.configure([my_structseq])

        # Make sure values all got set
        res = self._app.query([])
        for r in res:
            if r.id == 'my_structseq':
                upper = r.value.value()[0]
                lower = r.value.value()[1]
                for v in upper.value():
                    if v.id == 'ss_octet':
                        self.assertEquals(v.value.value(), 255)
                    elif v.id == 'ss_short':
                        self.assertEquals(v.value.value(), 32767)
                    elif v.id == 'ss_ushort':
                        self.assertEquals(v.value.value(), 65535)
                    elif v.id == 'ss_long':
                        self.assertEquals(v.value.value(), 2147483647)
                    elif v.id == 'ss_ulong':
                        self.assertEquals(v.value.value(), 4294967295)
                    elif v.id == 'ss_longlong':
                        self.assertEquals(v.value.value(), 9223372036854775807L)
                    elif v.id == 'ss_ulonglong':
                        self.assertEquals(v.value.value(), 18446744073709551615L)
		    elif v.id == 'ss_seq_octet':
			# Octets need to be unpacked
                	stored_vals = v.value.value()
                	vals = []
                	for num in stored_vals:
                    	    curr = struct.unpack('B', num)
                    	    vals.append(curr[0])
                	self.assertEquals(vals[0], 0)
                	self.assertEquals(vals[1], 255)
		    elif v.id == 'ss_seq_short':
			self.assertEquals(v.value.value(), [0, 32767])
		    elif v.id == 'ss_seq_ushort':
			self.assertEquals(v.value.value(), [0, 65535])
		    elif v.id == 'ss_seq_long':
			self.assertEquals(v.value.value(), [0, 2147483647])
		    elif v.id == 'ss_seq_ulong':
			self.assertEquals(v.value.value(), [0, 4294967295])
		    elif v.id == 'ss_seq_longlong':
			self.assertEquals(v.value.value(), [0, 9223372036854775807L])
		    #elif v.id == 'ss_seq_ulonglong':
		#	self.assertEquals(v.value.value(), [0, 9223372036854775807L])
                for v in lower.value():
                    if v.id == 'ss_octet':
                        self.assertEquals(v.value.value(), 0)
                    elif v.id == 'ss_short':
                        self.assertEquals(v.value.value(), -32768)
                    elif v.id == 'ss_ushort':
                        self.assertEquals(v.value.value(), 0)
                    elif v.id == 'ss_long':
                        self.assertEquals(v.value.value(), -2147483648)
                    elif v.id == 'ss_ulong':
                        self.assertEquals(v.value.value(), 0)
                    elif v.id == 'ss_longlong':
                        self.assertEquals(v.value.value(), -9223372036854775808L)
                    elif v.id == 'ss_ulonglong':
                        self.assertEquals(v.value.value(), 0)
		    elif v.id == 'ss_seq_octet':
			# Octets need to be unpacked
                	stored_vals = v.value.value()
                	vals = []
                	for num in stored_vals:
                    	    curr = struct.unpack('B', num)
                    	    vals.append(curr[0])
                	self.assertEquals(vals[0], 0)
                	self.assertEquals(vals[1], 255)
		    elif v.id == 'ss_seq_short':
			self.assertEquals(v.value.value(), [-32768, 32767])
		    elif v.id == 'ss_seq_ushort':
			self.assertEquals(v.value.value(), [0, 65535])
		    elif v.id == 'ss_seq_long':
			self.assertEquals(v.value.value(), [-2147483648, 2147483647])
		    elif v.id == 'ss_seq_ulong':
			self.assertEquals(v.value.value(), [0, 4294967295])
		    elif v.id == 'ss_seq_longlong':
			self.assertEquals(v.value.value(), [-9223372036854775808L, 9223372036854775807L])
		    #elif v.id == 'ss_seq_ulonglong':
			#self.assertEquals(v.value.value(), [0, 9223372036854775807L])

    def checkPropValues(self, expected_short, expected_long, expected_struct_short, expected_struct_long):
        # Gets the property values
        res = self._app.query([])

        # Checks for expected prop values
        for r in res:
            if r.id == 'my_short':
                self.assertEquals(r.value.value(), expected_short)
            elif r.id == 'my_long':
                self.assertEquals(r.value.value(), expected_long)
            elif r.id == 'my_struct':
                val = r.value.value()
                for v in val:
                    if v.id == 'struct_short':
                        self.assertEquals(v.value.value(), expected_struct_short)
                    elif v.id == 'struct_long':
                        self.assertEquals(v.value.value(), expected_struct_long)

    def test_bug208(self):
        self.preconditions()

        # Makes sure that a query will change the property values
        my_short = CF.DataType(id='my_short', value=CORBA.Any(CORBA.TC_short, 11))
        my_long = CF.DataType(id='my_long', value=CORBA.Any(CORBA.TC_long, 22))
        my_struct = CF.DataType(id='my_struct',value=any.to_any([
                                CF.DataType(id='struct_short',value=CORBA.Any(CORBA.TC_short, 33)),
                                CF.DataType(id='struct_long',value=CORBA.Any(CORBA.TC_long, 44))]))

        # Configures and makes sure properties were set correctly
        self._app.configure([my_short, my_long, my_struct])
        self.checkPropValues(11, 22, 33, 44)

        # Make sure correct exceptions are raised
        bad_short = CF.DataType(id='my_short', value=CORBA.Any(CORBA.TC_string, 'invalid'))
        bad_long = CF.DataType(id='my_long' , value=CORBA.Any(CORBA.TC_string, 'invalid'))
        bad_struct = CF.DataType(id='my_struct', value=any.to_any([
                                CF.DataType(id='struct_short', value=CORBA.Any(CORBA.TC_string, 'invalid')),
                                CF.DataType(id='struct_long', value=CORBA.Any(CORBA.TC_string, 'invalid'))]))

        # Properties should not have changed
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, [bad_short])
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, [bad_short, bad_long])
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, [bad_short, bad_struct])
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, [bad_long])
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, [bad_long, bad_struct])
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, [bad_struct])
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, [bad_short, bad_long, bad_struct])
        self.checkPropValues(11, 22, 33, 44)

        new_short = CF.DataType(id='my_short', value=CORBA.Any(CORBA.TC_short, 111))
        new_long = CF.DataType(id='my_long', value=CORBA.Any(CORBA.TC_long, 222))
        new_struct = CF.DataType(id='my_struct',value=any.to_any([
                                CF.DataType(id='struct_short',value=CORBA.Any(CORBA.TC_short, 333)),
                                CF.DataType(id='struct_long',value=CORBA.Any(CORBA.TC_long, 444))]))

        # Only long should change
        self.assertRaises(CF.PropertySet.PartialConfiguration, self._app.configure, [bad_short, new_long, bad_struct])
        self.checkPropValues(11, 222, 33, 44)

        # Only short should change
        self.assertRaises(CF.PropertySet.PartialConfiguration, self._app.configure, [new_short, bad_long, bad_struct])
        self.checkPropValues(111, 222, 33, 44)

        # Only struct should change
        self.assertRaises(CF.PropertySet.PartialConfiguration, self._app.configure, [bad_short, bad_long, new_struct])
        self.checkPropValues(111, 222, 333, 444)

        bad_short_name = CF.DataType(id='wrong_short', value=CORBA.Any(CORBA.TC_short, 66))
        bad_long_name = CF.DataType(id='wrong_long' , value=CORBA.Any(CORBA.TC_long, 66))
        bad_struct_name = CF.DataType(id='wrong_struct',value=any.to_any([
                                CF.DataType(id='struct_short',value=CORBA.Any(CORBA.TC_short, 333)),
                                CF.DataType(id='struct_long',value=CORBA.Any(CORBA.TC_long, 444))]))
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, [bad_short_name, bad_long_name, bad_struct_name])
        self.checkPropValues(111, 222, 333, 444)

        # Check random partial configs
        self.assertRaises(CF.PropertySet.PartialConfiguration, self._app.configure, [bad_short_name, my_long, bad_struct])
        self.checkPropValues(111, 22, 333, 444)
        self.assertRaises(CF.PropertySet.PartialConfiguration, self._app.configure, [my_short, bad_long_name, bad_struct])
        self.checkPropValues(11, 22, 333, 444)
        self.assertRaises(CF.PropertySet.PartialConfiguration, self._app.configure, [bad_short, my_long, my_struct])
        self.checkPropValues(11, 22, 33, 44)

        # Makes sure that all values can successfully be changed at once
        self._app.configure([new_short, new_struct, new_long])
        self.checkPropValues(111, 222, 333, 444)

    def test_propertyExceptionsSeq(self):
        self.preconditions()

        seq_short = CF.DataType(id='seq_short', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/ShortSeq:1.0"),
                                                            [11, 22]
                                                            ))
        self._app.configure([seq_short])

        seq_short = CF.DataType(id='seq_short', value=any.to_any([33,44]))
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, [seq_short])

        seq_long = CF.DataType(id='seq_long', value=any.to_any([55, 66]))
        self.assertRaises(CF.PropertySet.PartialConfiguration, self._app.configure, [seq_short, seq_long])

        # Long sequence should have changed and short did not
        for r in self._app.query([]):
            if r.id == 'seq_long':
                self.assertEquals(r.value.value()[0], 55)
                self.assertEquals(r.value.value()[1], 66)
            elif r.id =='seq_short':
                self.assertEquals(r.value.value()[0], 11)
                self.assertEquals(r.value.value()[1], 22)

    def test_SimpleSequenceFromNil(self):
        self.preconditions()

        # Make sure that seq_long has at least 1 value to start
        seq_long = CF.DataType(id='seq_long', value=any.to_any(None))
        props = self._app.query([seq_long])
        self.assertNotEqual(len(props[0].value.value()), 0)

        # Set the sequence to 'nil' and check that it now has no values
        self._app.configure([seq_long])
        props = self._app.query([seq_long])
        self.assertEqual(len(props[0].value.value()), 0)

    def _queryValue(self, propid):
        prop = self._app.query([CF.DataType(propid, any.to_any(None))])
        return prop[0].value.value()

    def _tryStringConversion(self, propid):
        value = self._queryValue(propid) + 1
        try:
            self._app.configure([CF.DataType(propid, any.to_any(str(value)))])
        except:
            self.fail("Configure of '"+propid+"' from string value raised an exception")

        new_value = self._queryValue(propid)
        self.assertEqual(value, new_value)

    def test_SimpleStringConversion(self):
        self.preconditions()

        self._tryStringConversion('my_octet')
        self._tryStringConversion('my_short')
        self._tryStringConversion('my_ushort')
        self._tryStringConversion('my_long')
        self._tryStringConversion('my_ulong')
        self._tryStringConversion('my_longlong')
        self._tryStringConversion('my_ulonglong')
        

class CppCallbacksTest(scatest.CorbaTestCase):
    def test_Callbacks(self):
        comp = sb.launch('CppCallbacks')
        self.assertEqual(comp.callbacks_run, [])

        # Simple
        count = comp.count + 1
        comp.count = count
        self.assertEqual(comp.callbacks_run.count('count'), 1)

        # Simple sequence
        constellation = comp.constellation[::2]
        comp.constellation = constellation
        self.assertEqual(comp.callbacks_run.count('constellation'), 1)

        # Struct
        station = {'name': 'WYPR', 'frequency': 88.1}
        comp.station = station
        self.assertEqual(comp.callbacks_run.count('station'), 1)

        # Struct sequence
        servers = comp.servers + [{'host': 'localhost', 'port':8080}]
        comp.servers = servers
        self.assertEqual(comp.callbacks_run.count('servers'), 1)

        # Clear callback tracking and set the same values to ensure that the
        # callbacks do not get triggered
        comp.callbacks_run = []
        comp.count = count
        comp.constellation = constellation
        comp.station = station
        comp.servers = servers
        self.assertEqual(comp.callbacks_run, [])


class CPPPropertyTest(scatest.CorbaTestCase):
    def setUp(self):
        self._domBooter, self._domMgr = self.launchDomainManager()

    def tearDown(self):
        try:
            self._app.stop()
            self._app.releaseObject()
        except AttributeError:
            pass

        try:
            self._devMgr.shutdown()
        except AttributeError:
            pass

        try:
            self.terminateChild(self._devBooter)
        except AttributeError:
            pass

        try:
            self.terminateChild(self._domBooter)
        except AttributeError:
            pass

        # Do all application and node booter shutdown before calling the base
        # class tearDown, or failures will occur.
        scatest.CorbaTestCase.tearDown(self)

    def test_triggerTiming(self):
        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        dom=redhawk.attach(self._domMgr._get_name())
        app = self._domMgr.createApplication("/waveforms/prop_trigger_timing_w/prop_trigger_timing_w.sad.xml", "prop_trigger_timing_w", [], [])
        self.assertEquals(dom.apps[0].comps[0].prop_1_trigger, False)
        self.assertEquals(dom.apps[0].comps[0].prop_2_trigger, False)
        self.assertEquals(dom.apps[0].comps[0].prop_3_trigger, False)
        self.assertEquals(dom.apps[0].comps[0].prop_4_trigger, False)

    def test_Property_CPP(self):
        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/Property_T1/Property_T1.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]
        self.assertNotEqual(appFact, None)
        app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(app, None)
        app.start()
        time.sleep(1)

        ps=None
        c=None
        d=redhawk.attach(scatest.getTestDomainName())
        a=d.apps[0]
        c=filter( lambda c : c.name == 'Property_CPP', a.comps )[0]
        self.assertNotEqual(c,None)
        ps = c.ref._narrow(CF.PropertySet)
        self.assertNotEqual(ps,None)
        
        self.assertEquals(c.p1,"prop1")
        self.assertAlmostEquals(c.p2,123.4)
        self.assertEquals(c.p3,567)
        self.assertEquals(c.p4.p4sub1,"prop2")
        t1=int(c.p4.p4sub2)
        self.assertEquals(t1,890)

        c.p1 = "testing"
        c.p2 = 100.0
        c.p3 = 100
        c.p4.p4sub1="testing2"
        c.p4.p4sub2=200.0

        self.assertEquals(c.p1,"testing")
        self.assertAlmostEquals(c.p2,100.0)
        self.assertEquals(c.p3,100)
        self.assertEquals(c.p4.p4sub1,"testing2")
        t1=int(c.p4.p4sub2)
        self.assertEquals(t1,200)


        app.releaseObject()

