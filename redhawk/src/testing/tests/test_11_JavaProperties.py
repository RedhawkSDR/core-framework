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

import unittest
from _unitTestHelpers import scatest
from ossie.cf import CF
from omniORB import CORBA, any
import struct
import time
from ossie.utils import sb
from ossie.utils import redhawk


class JavaPropertiesTest(scatest.CorbaTestCase):
    """
    Test new-style 1.9+ Java properties.
    """
    def setUp(self):
        self.comp = sb.launch('TestJavaProps')

    def tearDown(self):
        self.comp.releaseObject()

    def test_EmptyQuery (self):
        results = self.comp.query([])
        self.assertEqual(len(results), 5)

        ids = set(r.id for r in results)
        expected = ("ulong_prop",
                    "long_seq",
                    "struct_prop",
                    "exec_param",
                    "structseq_prop")
        for prop in expected:
            self.assert_(prop in ids, "'%s' not in default query" % prop)

    def test_SimpleProps(self):
        # Check expected default
        self.assertEqual(self.comp.ulong_prop, None)

        # Change the value (to a value that exceeds signed long range) and
        # check that it is updated
        newvalue = 2**32-1
        self.comp.ulong_prop = newvalue
        self.assertEqual(self.comp.ulong_prop, newvalue)

        # Clear to nil
        self.comp.ulong_prop = None
        self.assertEqual(self.comp.ulong_prop, None)

    def test_SequenceProps(self):
        # Check expected default
        self.assertEqual(len(self.comp.long_seq), 0)

        # Change the value and check that it is updated
        newvalue = [1,2,3,4]
        self.comp.long_seq = newvalue
        self.assertEqual(self.comp.long_seq, newvalue)

        # Check that assigning "None" clears the list (but it remains a list)
        # NB: Only valid for 1.9+ Java properties.
        self.comp.long_seq = None
        self.assertEqual(len(self.comp.long_seq), 0)

    def test_StructProps(self):
        # Check expected default
        self.assertEqual(self.comp.struct_prop, {'item_string':'default', 'item_long':0})

        # Change the value and check that it is updated
        newvalue = {'item_string':'changed', 'item_long': 1000}
        self.comp.struct_prop = newvalue
        self.assertEqual(self.comp.struct_prop, newvalue)

        # Working directly with the CORBA values, add an invalid field to the
        # current value and check that it does not throw an exception.
        anyval = self.comp.query([CF.DataType('struct_prop', any.to_any(None))])[0]
        anyval.value._v.append(CF.DataType('invalid_field', any.to_any(None)))
        try:
            self.comp.configure([anyval])
        except CF.PropertySet.InvalidConfiuration:
            self.fail('Extra struct fields were not silently ignored')

    def test_StructSequenceProps(self):
        # Check that the struct sequence matches the expected value from the PRF.
        self.assertEqual(self.comp.structseq_prop, [{'item_string':'default', 'item_long':0}, {'item_string':'default', 'item_long':0}])

        # Try configuring a new structsequence value and checking that it comes back correct.
        newvalue = [{"item_long":12, "item_string":"twelve"}, {"item_long":9, "item_string":"nine"}]
        self.comp.structseq_prop = newvalue
        self.assertEqual(self.comp.structseq_prop, newvalue)

    def test_OptionalPropertiesInStruct(self):
        comp = sb.launch('TestJavaOptionalProps')
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
                    elif v.id == 'struct_string':
                        self.assertEquals(v.value.value(), "new string")
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
	
        res = comp.query([])
        for r in res:
            if r.id == 'my_struct':
                val = r.value.value()
                valIds = [v.id for v in val]
                self.assertTrue('struct_long' not in valIds)
                self.assertTrue('struct_ulonglong' not in valIds)
                self.assertTrue('struct_seq_ushort' not in valIds)
                self.assertTrue('struct_seq_ulonglong' not in valIds)
                for v in val:
                    if v.id == 'struct_octet':
                        self.assertEquals(v.value.value(), 255)
                    if v.id == 'struct_short':
                        self.assertEquals(v.value.value(), 32767)
                    elif v.id == 'struct_ushort':
                        self.assertEquals(v.value.value(), 65535)
                    elif v.id == 'struct_ulong':
                        self.assertEquals(v.value.value(), 4294967295)
                    elif v.id == 'struct_longlong':
                        self.assertEquals(v.value.value(), 9223372036854775807L)
                    elif v.id == 'struct_string':
                        self.assertEquals(v.value.value(), "new string")
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
		    elif v.id == 'struct_seq_long':
		        self.assertEquals(v.value.value(), [0, 2147483647])
		    elif v.id == 'struct_seq_ulong':
			self.assertEquals(v.value.value(), [0, 4294967295])
		    elif v.id == 'struct_seq_longlong':
			self.assertEquals(v.value.value(), [0, 9223372036854775807L])


class LegacyJavaPropertiesTest(scatest.CorbaTestCase):
    """
    Test backwards compatibility with pre-1.9 Java properties.
    """
    def setUp(self):
        self.comp = sb.launch('TestLegacyJavaProps')

    def tearDown(self):
        self.comp.releaseObject()

    def test_EmptyQuery (self):
        results = self.comp.query([])
        self.assertEqual(len(results), 5)

        ids = set(r.id for r in results)
        expected = ("DCE:3303ee57-70bb-4325-84ad-fb7fd333c44a",
                    "DCE:dd8d450f-d377-4c2c-8c3c-207e42dae017",
                    "DCE:d933f8ba-9e79-4d2c-a1b5-9fb9da0ea740",
                    "exec_param",
                    "DCE:23a6d333-55fb-4425-a102-185e6e998782")
        for prop in expected:
            self.assert_(prop in ids, "'%s' not in default query" % prop)

    def test_SimpleProps(self):
        # Check expected default
        self.assertEqual(self.comp.ulong_prop, None)

        # Change the value (to a value that exceeds signed long range) and
        # check that it is updated
        newvalue = 2**32-1
        self.comp.ulong_prop = newvalue
        self.assertEqual(self.comp.ulong_prop, newvalue)

        # Clear to nil
        self.comp.ulong_prop = None
        self.assertEqual(self.comp.ulong_prop, None)

    def test_SequenceProps(self):
        # Check expected default
        self.assertEqual(len(self.comp.long_seq), 0)

        # Change the value and check that it is updated
        newvalue = [1,2,3,4]
        self.comp.long_seq = newvalue
        self.assertEqual(self.comp.long_seq, newvalue)

    def test_StructProps(self):
        # Check expected default
        self.assertEqual(self.comp.struct_prop, {'item_string':'default', 'item_long':0})

        # Change the value and check that it is updated
        newvalue = {'item_string':'changed', 'item_long': 1000}
        self.comp.struct_prop = newvalue
        self.assertEqual(self.comp.struct_prop, newvalue)

    def test_StructSequenceProps(self):
        # Check that the struct sequence matches the expected value from the PRF.
        self.assertEqual(self.comp.structseq_prop, [{'item_string':'default', 'item_long':0}, {'item_string':'default', 'item_long':0}])

        # Try configuring a new structsequence value and checking that it comes back correct.
        newvalue = [{"item_long":12, "item_string":"twelve"}, {"item_long":9, "item_string":"nine"}]
        self.comp.structseq_prop = newvalue
        self.assertEqual(self.comp.structseq_prop, newvalue)


class JavaPropertiesRangeTest(scatest.CorbaTestCase):

    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self._app = None
        if self._domMgr:
            try:
                sadpath = "/waveforms/TestJavaPropsRange/TestJavaPropsRange.sad.xml"
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

    def test_javaPropsRangeSimple(self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")
        self.assertNotEqual(self._app, None, "Failed to launch app")

        # Test upper bound
        my_octet = CF.DataType(id='my_octet', value=CORBA.Any(CORBA.TC_octet, 255))
        my_short = CF.DataType(id='my_short', value=CORBA.Any(CORBA.TC_short, 32767))
        my_ushort = CF.DataType(id='my_ushort', value=CORBA.Any(CORBA.TC_ushort, 65535))
        my_long = CF.DataType(id='my_long', value=CORBA.Any(CORBA.TC_long, 2147483647))
        my_ulong = CF.DataType(id='my_ulong', value=CORBA.Any(CORBA.TC_ulong, 4294967295))
        my_longlong = CF.DataType(id='my_longlong', value=CORBA.Any(CORBA.TC_longlong, 9223372036854775807L))
        self._app.configure([my_octet, my_short, my_ushort, my_long, my_ulong, my_longlong])
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

        # Test lower bound
        my_octet = CF.DataType(id='my_octet', value=CORBA.Any(CORBA.TC_long, 0))
        my_short = CF.DataType(id='my_short', value=CORBA.Any(CORBA.TC_short, -32768))
        my_ushort = CF.DataType(id='my_ushort', value=CORBA.Any(CORBA.TC_ushort, 0))
        my_long = CF.DataType(id='my_long', value=CORBA.Any(CORBA.TC_long, -2147483648))
        my_ulong = CF.DataType(id='my_ulong', value=CORBA.Any(CORBA.TC_ulong, 0))
        my_longlong = CF.DataType(id='my_longlong', value=CORBA.Any(CORBA.TC_longlong, -9223372036854775808L))
        self._app.configure([my_octet, my_short, my_ushort, my_long, my_ulong, my_longlong])
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

    def test_javaPropsRangeStruct(self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")
        self.assertNotEqual(self._app, None, "Failed to launch app")

        # Test upper bounds
	octet_val = struct.pack('B', 0) + struct.pack('B', 255)
        my_struct = CF.DataType(id='my_struct', value=any.to_any([
                                CF.DataType(id='struct_octet', value=CORBA.Any(CORBA.TC_octet, 255)),
                                CF.DataType(id='struct_short', value=CORBA.Any(CORBA.TC_short, 32767)),
                                CF.DataType(id='struct_ushort', value=CORBA.Any(CORBA.TC_ushort, 65535)),
                                CF.DataType(id='struct_long', value=CORBA.Any(CORBA.TC_long, 2147483647)),
                                CF.DataType(id='struct_ulong', value=CORBA.Any(CORBA.TC_ulong, 4294967295)),
                                CF.DataType(id='struct_longlong', value=CORBA.Any(CORBA.TC_longlong, 9223372036854775807L)),
				CF.DataType(id='struct_seq_octet', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/OctetSeq:1.0"), octet_val)),
                                CF.DataType(id='struct_seq_short', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/ShortSeq:1.0"), [0, 32767])),
                                CF.DataType(id='struct_seq_ushort', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/UShortSeq:1.0"), [0, 65535])),
                                CF.DataType(id='struct_seq_long', value=any.to_any([0, 2147483647])),
                                CF.DataType(id='struct_seq_ulong', value=any.to_any([0, 4294967295])),
                                CF.DataType(id='struct_seq_longlong', value=any.to_any([0, 9223372036854775807L])),
                                CF.DataType(id='struct_seq_ulonglong', value=any.to_any([0, 9223372036854775807L]))
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
		    elif v.id == 'struct_seq_ulonglong':
			self.assertEquals(v.value.value(), [0, 9223372036854775807L])

        # Test lower bounds
        my_struct = CF.DataType(id='my_struct', value=any.to_any([
                                CF.DataType(id='struct_octet', value=CORBA.Any(CORBA.TC_octet, 0)),
                                CF.DataType(id='struct_short', value=CORBA.Any(CORBA.TC_short, -32768)),
                                CF.DataType(id='struct_ushort', value=CORBA.Any(CORBA.TC_ushort, 0)),
                                CF.DataType(id='struct_long', value=CORBA.Any(CORBA.TC_long, -2147483648)),
                                CF.DataType(id='struct_ulong', value=CORBA.Any(CORBA.TC_ulong, 0)),
                                CF.DataType(id='struct_longlong', value=CORBA.Any(CORBA.TC_longlong, -9223372036854775808L)),
				CF.DataType(id='struct_seq_octet', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/OctetSeq:1.0"), octet_val)),
                                CF.DataType(id='struct_seq_short', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/ShortSeq:1.0"), [-32768, 32767])),
                                CF.DataType(id='struct_seq_ushort', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/UShortSeq:1.0"), [0, 65535])),
                                CF.DataType(id='struct_seq_long', value=any.to_any([-2147483648, 2147483647])),
                                CF.DataType(id='struct_seq_ulong', value=any.to_any([0, 4294967295])),
                                CF.DataType(id='struct_seq_longlong', value=any.to_any([-9223372036854775808L, 9223372036854775807L])),
                                CF.DataType(id='struct_seq_ulonglong', value=any.to_any([0, 9223372036854775807]))
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
		    elif v.id == 'struct_seq_ulonglong':
			self.assertEquals(v.value.value(), [0, 9223372036854775807L])


    def test_javaPropsRangeSeq(self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")
        self.assertNotEqual(self._app, None, "Failed to launch app")

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


    def test_javaPropsRangeStructSeq(self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")
        self.assertNotEqual(self._app, None, "Failed to launch app")

        # Struct with upper bound
	octet_val = struct.pack('B', 0) + struct.pack('B', 255)
        upper = CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), [
			    CF.DataType(id='ss_octet', value=CORBA.Any(CORBA.TC_octet, 255)),
                	    CF.DataType(id='ss_short', value=CORBA.Any(CORBA.TC_short, 32767)),
	                    CF.DataType(id='ss_ushort', value=CORBA.Any(CORBA.TC_ushort, 65535)),
            	  	    CF.DataType(id='ss_long', value=CORBA.Any(CORBA.TC_long, 2147483647)),
                	    CF.DataType(id='ss_ulong', value=CORBA.Any(CORBA.TC_ulong, 4294967295)),
                	    CF.DataType(id='ss_longlong', value=CORBA.Any(CORBA.TC_longlong, 9223372036854775807L)),
                	    CF.DataType(id='ss_seq_octet', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/OctetSeq:1.0"), octet_val)),
                            CF.DataType(id='ss_seq_short', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/ShortSeq:1.0"), [0, 32767])),
                            CF.DataType(id='ss_seq_ushort', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/UShortSeq:1.0"), [0, 65535])),
                	    CF.DataType(id='ss_seq_long', value=any.to_any([0, 2147483647])),
                	    CF.DataType(id='ss_seq_ulong', value=any.to_any([0, 4294967295])),
                	    CF.DataType(id='ss_seq_longlong', value=any.to_any([0, 9223372036854775807L])),
                	    CF.DataType(id='ss_seq_ulonglong', value=any.to_any([0, 9223372036854775807L]))
                ])
        # Struct with lower bound
        lower = CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), [
                            CF.DataType(id='ss_octet', value=CORBA.Any(CORBA.TC_octet, 0)),
                            CF.DataType(id='ss_short', value=CORBA.Any(CORBA.TC_short, -32768)),
                            CF.DataType(id='ss_ushort', value=CORBA.Any(CORBA.TC_ushort, 0)),
                            CF.DataType(id='ss_long', value=CORBA.Any(CORBA.TC_long, -2147483648)),
                            CF.DataType(id='ss_ulong', value=CORBA.Any(CORBA.TC_ulong, 0)),
                            CF.DataType(id='ss_longlong', value=CORBA.Any(CORBA.TC_longlong, -9223372036854775808L)),
			    CF.DataType(id='ss_seq_octet', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/OctetSeq:1.0"), octet_val)),
                            CF.DataType(id='ss_seq_short', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/ShortSeq:1.0"), [-32768, 32767])),
                            CF.DataType(id='ss_seq_ushort', value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/UShortSeq:1.0"), [0, 65535])),
                            CF.DataType(id='ss_seq_long', value=any.to_any([-2147483648, 2147483647])),
                            CF.DataType(id='ss_seq_ulong', value=any.to_any([0, 4294967295])),
                            CF.DataType(id='ss_seq_longlong', value=any.to_any([-9223372036854775808L, 9223372036854775807L])),
                            CF.DataType(id='ss_seq_ulonglong', value=any.to_any([0, 9223372036854775807L]))
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
		    elif v.id == 'ss_seq_ulonglong':
			self.assertEquals(v.value.value(), [0, 9223372036854775807L])
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
		    elif v.id == 'ss_seq_ulonglong':
			self.assertEquals(v.value.value(), [0, 9223372036854775807L])




class JAVAPropertyTest(scatest.CorbaTestCase):
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

    def test_Property_JAVA(self):
        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", self._domMgr)
        self.assertNotEqual(self._devBooter, None)
        self._domMgr.installApplication("/waveforms/Property_T2/Property_T2.sad.xml")
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
        c=filter( lambda c : c.name == 'Property_JAVA', a.comps )[0]
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
