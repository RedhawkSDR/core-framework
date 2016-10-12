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
import os
from ossie.utils import sb

class PyPropertiesTest(scatest.CorbaTestCase):
    def setUp(self):
        print "-----------------------------------------"
        print os.getenv('OSSIEHOME')
        print "-----------------------------------------"
        domBooter, self._domMgr = self.launchDomainManager(debug=self.debuglevel)
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=self.debuglevel)
        self._app = None

    def tearDown(self):
        if self._app:
            self._app.stop()
            self._app.releaseObject()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def _launchApp (self, name):
        sadpath = "/waveforms/%s/%s.sad.xml" % (name, name)
        self._domMgr.installApplication(sadpath)
        appFact = self._domMgr._get_applicationFactories()[0]
        return appFact.create(appFact._get_name(), [], [])

    def test_sadPropertyOverride (self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")

        self._app = self._launchApp("TestPythonPropsWithOverride")
        self.assertNotEqual(self._app, None, "Failed to launch app")

        # The TestPyProps component is also the assembly controller, so we can
        # query it via the application.
        prop = self._app.query([CF.DataType("DCE:897a5489-f680-46a8-a698-e36fd8bbae80[]", any.to_any(None))])[0]

        # Check that there is one value, and it matches the expected value from
        # the SAD.
        value = any.from_any(prop.value)
        self.assertEqual(len(value), 1)
        d = dict([(dt['id'], dt['value']) for dt in value[0]])
        self.assertEqual(d, {'ip_address': '127.0.0.1', 'port': 4688})

    def test_pythonPropertyTypes (self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")

        self._app = self._launchApp("TestPythonPropsWithOverride")
        self.assertNotEqual(self._app, None, "Failed to launch app")

        # The TestPyProps component is also the assembly controller, so we can
        # query it via the application.
        props = self._app.query([CF.DataType("DCE:ffe634c9-096d-425b-86cc-df1cce50612f", any.to_any(None)),
                                CF.DataType("test_float", any.to_any(None)),
                                CF.DataType("test_double", any.to_any(None))])

        for prop in props:
            if prop.id == "DCE:ffe634c9-096d-425b-86cc-df1cce50612f":
                prop_struct = prop
            if prop.id == "test_float":
                prop_float = prop
            if prop.id == "test_double":
                prop_double = prop
        self.assertEqual(prop_float.value.typecode(), CORBA.TC_float)
        self.assertEqual(prop_double.value.typecode(), CORBA.TC_double)

        for sub in prop_struct.value._v:
            if sub.id == 'item4':
                sub_type_float = sub.value.typecode()
            if sub.id == 'item3':
                sub_type_double = sub.value.typecode()
        self.assertEqual(sub_type_float, CORBA.TC_float)
        self.assertEqual(sub_type_double, CORBA.TC_double)

    def test_fullQuery(self):
        dev = self._devMgr._get_registeredDevices()[0]
        results = dev.query([])
        self.assertEqual(len(results), 20)

    def test_hexProperty(self):
        dev = self._devMgr._get_registeredDevices()[0]
        prop = CF.DataType(id='hex_props',value=any.to_any(None))
        results = dev.query([prop])
        self.assertEqual(any.from_any(results[0].value)[0], 2)
        self.assertEqual(any.from_any(results[0].value)[1], 3)
        prop = CF.DataType(id='hex_prop',value=any.to_any(None))
        results = dev.query([prop])
        self.assertEqual(any.from_any(results[0].value), 2)

    def test_sequenceOperators (self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")

        self._app = self._launchApp('TestPythonProps')
        self.assertNotEqual(self._app, None, "Application not created")

        # Get the value for the structsequence property. The TestPyProps component
        # is also the assembly controller, so we can query it via the application.
        id = "DCE:897a5489-f680-46a8-a698-e36fd8bbae80"
        prop = self._app.query([CF.DataType(id + '[]', any.to_any(None))])[0]
        value = any.from_any(prop.value)

        # Test the length operator.
        length = self._app.query([CF.DataType(id + '[#]', any.to_any(None))])[0]
        length = any.from_any(length.value)
        self.assertEqual(len(value), length)

        # Test key list.
        keys = self._app.query([CF.DataType(id + '[?]', any.to_any(None))])[0]
        keys = any.from_any(keys.value)
        self.assertEqual(keys, range(length))

        # Test key-value pairs.
        kvpairs = self._app.query([CF.DataType(id + '[@]', any.to_any(None))])[0]
        kvpairs = any.from_any(kvpairs.value)
        for pair in kvpairs:
            index = int(pair['id'])
            self.assertEqual(value[index], pair['value'])

        # Test indexing.
        v1 = self._app.query([CF.DataType(id + '[1]', any.to_any(None))])[0]
        v1 = any.from_any(v1.value)
        self.assertEqual(len(v1), 1)
        self.assertEqual(v1[0], value[1])

    def test_pythonPropsRange(self):
        self._app = self._launchApp('TestPythonPropsRange')

        # Test upper bound
        my_octet = CF.DataType(id='my_octet', value=CORBA.Any(CORBA.TC_long, 255))
        my_short = CF.DataType(id='my_short', value=CORBA.Any(CORBA.TC_long, 32767))
        my_ushort = CF.DataType(id='my_ushort', value=CORBA.Any(CORBA.TC_long, 65535))
        my_long = CF.DataType(id='my_long', value=CORBA.Any(CORBA.TC_longlong, 2147483647))
        my_ulong = CF.DataType(id='my_ulong', value=CORBA.Any(CORBA.TC_longlong, 4294967295))
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

        # Test lower boound
        my_octet = CF.DataType(id='my_octet', value=CORBA.Any(CORBA.TC_long, 0))
        my_short = CF.DataType(id='my_short', value=CORBA.Any(CORBA.TC_long, -32768))
        my_ushort = CF.DataType(id='my_ushort', value=CORBA.Any(CORBA.TC_long, 0))
        my_long = CF.DataType(id='my_long', value=CORBA.Any(CORBA.TC_longlong, -2147483648))
        my_ulong = CF.DataType(id='my_ulong', value=CORBA.Any(CORBA.TC_longlong, 0))
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

        # Test one beyond upper bound
        my_octet = CF.DataType(id='my_octet', value=CORBA.Any(CORBA.TC_long, 256))
        my_short = CF.DataType(id='my_short', value=CORBA.Any(CORBA.TC_long, 32768))
        my_ushort = CF.DataType(id='my_ushort', value=CORBA.Any(CORBA.TC_long, 65536))
        my_long = CF.DataType(id='my_long', value=CORBA.Any(CORBA.TC_longlong, 2147483648))
        my_ulong = CF.DataType(id='my_ulong', value=CORBA.Any(CORBA.TC_longlong, 4294967296))
        my_longlong = CF.DataType(id='my_longlong', value=CORBA.Any(CORBA.TC_ulonglong, 9223372036854775808L))

        # All should fail causing InvalidConfiguration
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure,
                          [my_octet, my_short, my_ushort, my_long, my_ulong, my_longlong])

        # Test one beyond lower bound
        my_octet = CF.DataType(id='my_octet', value=CORBA.Any(CORBA.TC_long, -1))
        my_short = CF.DataType(id='my_short', value=CORBA.Any(CORBA.TC_long, -32769))
        my_ushort = CF.DataType(id='my_ushort', value=CORBA.Any(CORBA.TC_long, -1))
        my_long = CF.DataType(id='my_long', value=CORBA.Any(CORBA.TC_longlong, -2147483649))
        my_ulong = CF.DataType(id='my_ulong', value=CORBA.Any(CORBA.TC_longlong, -1))
        my_ulonglong = CF.DataType(id='my_ulonglong', value=CORBA.Any(CORBA.TC_longlong, -1))

        # All should fail causing InvalidConfiguration
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure,
                          [my_octet, my_short, my_ushort, my_long, my_ulong, my_ulonglong])

        # Make sure Partial Configuration error will occur
        my_octet = CF.DataType(id='my_octet', value=CORBA.Any(CORBA.TC_long, 11))
        my_ushort = CF.DataType(id='my_ushort', value=CORBA.Any(CORBA.TC_long, 22))
        self.assertRaises(CF.PropertySet.PartialConfiguration, self._app.configure,
                          [my_octet, my_short, my_ushort, my_long, my_ulong])
        res = self._app.query([])
        for r in res:
            if r.id == 'my_octet':
                self.assertEquals(r.value.value(), 11)
            elif r.id == 'my_short':
                self.assertEquals(r.value.value(), -32768)
            elif r.id == 'my_ushort':
                self.assertEquals(r.value.value(), 22)
            elif r.id == 'my_long':
                self.assertEquals(r.value.value(), -2147483648)
            elif r.id == 'my_ulong':
                self.assertEquals(r.value.value(), 0)
            elif r.id == 'my_longlong':
                self.assertEquals(r.value.value(), -9223372036854775808)
            elif r.id == 'my_ulonglong':
                self.assertEquals(r.value.value(), 0)

    def test_pythonPropsRangeStruct(self):
        self._app = self._launchApp('TestPythonPropsRange')
        # Test upper bounds
        my_struct = CF.DataType(id='my_struct', value=any.to_any([
                                CF.DataType(id='struct_octet', value=CORBA.Any(CORBA.TC_long, 255)),
                                CF.DataType(id='struct_short', value=CORBA.Any(CORBA.TC_long, 32767)),
                                CF.DataType(id='struct_ushort', value=CORBA.Any(CORBA.TC_long, 65535)),
                                CF.DataType(id='struct_long', value=CORBA.Any(CORBA.TC_longlong, 2147483647)),
                                CF.DataType(id='struct_ulong', value=CORBA.Any(CORBA.TC_longlong, 4294967295)),
                                CF.DataType(id='struct_longlong', value=CORBA.Any(CORBA.TC_longlong, 9223372036854775807L)),
                                CF.DataType(id='struct_ulonglong', value=CORBA.Any(CORBA.TC_ulonglong, 18446744073709551615L))
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

        # Test lower bounds
        my_struct = CF.DataType(id='my_struct', value=any.to_any([
                                CF.DataType(id='struct_octet', value=CORBA.Any(CORBA.TC_long, 0)),
                                CF.DataType(id='struct_short', value=CORBA.Any(CORBA.TC_long, -32768)),
                                CF.DataType(id='struct_ushort', value=CORBA.Any(CORBA.TC_long, 0)),
                                CF.DataType(id='struct_long', value=CORBA.Any(CORBA.TC_longlong, -2147483648)),
                                CF.DataType(id='struct_ulong', value=CORBA.Any(CORBA.TC_longlong, 0)),
                                CF.DataType(id='struct_longlong', value=CORBA.Any(CORBA.TC_longlong, -9223372036854775808L)),
                                CF.DataType(id='struct_ulonglong', value=CORBA.Any(CORBA.TC_ulonglong, 0))
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


        # Loop through each member of the struct to test one beyond the upper bound
        for r in res:
            if r.id == 'my_struct':
                val = r.value.value()
                for v in val:
                    # All default values are valid
                    octet_val = 0
                    short_val = 0
                    ushort_val = 0
                    long_val = 0
                    ulong_val = 0
                    longlong_val = 0
                    ulonglong_val = 0
                    # Creates struct with only 1 invalid member, which will still cause invalid configuration
                    if v.id == 'struct_octet':
                        octet_val = 256
                    elif v.id == 'struct_short':
                        short_val = 32768
                    elif v.id == 'struct_ushort':
                        ushort_val = 65536
                    elif v.id == 'struct_long':
                        long_val = 2147483648
                    elif v.id == 'struct_ulong':
                        ulong_val = 4294967296
                    elif v.id == 'struct_longlong':
                        longlong_val = 9223372036854775808L
                    elif v.id == 'struct_ulonglong':
                        # No value large enough to test outside range of ulonglong
                        continue
                    my_struct = CF.DataType(id='my_struct', value=any.to_any([
                                CF.DataType(id='struct_octet', value=CORBA.Any(CORBA.TC_long, octet_val)),
                                CF.DataType(id='struct_short', value=CORBA.Any(CORBA.TC_long, short_val)),
                                CF.DataType(id='struct_ushort', value=CORBA.Any(CORBA.TC_long, ushort_val)),
                                CF.DataType(id='struct_long', value=CORBA.Any(CORBA.TC_longlong, long_val)),
                                CF.DataType(id='struct_ulong', value=CORBA.Any(CORBA.TC_longlong, ulong_val)),
                                CF.DataType(id='struct_longlong', value=CORBA.Any(CORBA.TC_ulonglong, longlong_val)),
                                CF.DataType(id='struct_ulonglong', value=CORBA.Any(CORBA.TC_ulonglong, ulonglong_val))
                                ]))
                    self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, [my_struct])


        # Loop through each member of the struct to test one beyond the lower bound
        for r in res:
            if r.id == 'my_struct':
                val = r.value.value()
                for v in val:
                    # All default values are valid
                    octet_val = 0
                    short_val = 0
                    ushort_val = 0
                    long_val = 0
                    ulong_val = 0
                    longlong_val = 0
                    ulonglong_val = 0
                    # Creates struct with only 1 invalid member, which will still cause invalid configuration
                    if v.id == 'struct_octet':
                        octet_val = -1
                    elif v.id == 'struct_short':
                        short_val = -32769
                    elif v.id == 'struct_ushort':
                        ushort_val = -1
                    elif v.id == 'struct_long':
                        long_val = -2147483649
                    elif v.id == 'struct_ulong':
                        ulong_val = -1
                    elif v.id == 'struct_longlong':
                        # No value to test below range of longlong
                        continue
                    elif v.id == 'struct_ulonglong':
                        ulonglong_val = -1
                    my_struct = CF.DataType(id='my_struct', value=any.to_any([
                                CF.DataType(id='struct_octet', value=CORBA.Any(CORBA.TC_long, octet_val)),
                                CF.DataType(id='struct_short', value=CORBA.Any(CORBA.TC_long, short_val)),
                                CF.DataType(id='struct_ushort', value=CORBA.Any(CORBA.TC_long, ushort_val)),
                                CF.DataType(id='struct_long', value=CORBA.Any(CORBA.TC_longlong, long_val)),
                                CF.DataType(id='struct_ulong', value=CORBA.Any(CORBA.TC_longlong, ulong_val)),
                                CF.DataType(id='struct_longlong', value=CORBA.Any(CORBA.TC_ulonglong, longlong_val)),
                                CF.DataType(id='struct_ulonglong', value=CORBA.Any(CORBA.TC_longlong, ulonglong_val))
                                ]))
                    self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, [my_struct])

    def test_pythonPropsRangeSeq(self):
        self._app = self._launchApp('TestPythonPropsRange')

        # Test upper and lower bounds
        seq_octet = CF.DataType(id='seq_octet', value=any.to_any([
                                                            CORBA.Any(CORBA.TC_long, 0),
                                                            CORBA.Any(CORBA.TC_long, 255)
                                                            ]))
        seq_short = CF.DataType(id='seq_short', value=any.to_any([-32768, 32767]))
        seq_ushort = CF.DataType(id='seq_ushort', value=any.to_any([0, 65535]))
        seq_long = CF.DataType(id='seq_long', value=any.to_any([-2147483648, 2147483647]))
        seq_ulong = CF.DataType(id='seq_ulong', value=any.to_any([0,4294967295]))
        seq_longlong = CF.DataType(id='seq_longlong', value=any.to_any([-9223372036854775808L, 9223372036854775807L]))
        #seq_ulonglong = CF.DataType(id='seq_ulonglong', value=any.to_any([
        #                                                    CORBA.Any(CORBA.TC_ulonglong, 9223372036854775808L),
        #                                                    CORBA.Any(CORBA.TC_ulonglong, 0)
        #                                                    ]))
        self._app.configure([seq_octet, seq_short, seq_ushort, seq_long, seq_ulong, seq_longlong])    #, seq_ulonglong])

        res = self._app.query([])
        for r in res:
            if r.id == 'seq_octet':
                # Octet sequences are treated like strings, so numbers need to be extracted
                vals = r.value.value().lstrip('[').rstrip(']').split(',')
                self.assertEquals(int(vals[0].strip()), 0)
                self.assertEquals(int(vals[1].strip()), 255)
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

        # Test one beyond upper bound
        seq_octet = CF.DataType(id='seq_octet', value=any.to_any([0, 256]))
        seq_short = CF.DataType(id='seq_short', value=any.to_any([0, 32768]))
        seq_ushort = CF.DataType(id='seq_ushort', value=any.to_any([0, 65536]))
        seq_long = CF.DataType(id='seq_long', value=any.to_any([0, 2147483648]))
        seq_ulong = CF.DataType(id='seq_ulong', value=any.to_any([0, 4294967296]))
        seq_longlong = CF.DataType(id='seq_longlong', value=any.to_any([0,
                                                            CORBA.Any(CORBA.TC_ulonglong, 9223372036854775808L)]))
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure,
                          [seq_octet, seq_short, seq_ushort, seq_long, seq_ulong, seq_longlong])

        # Test one beyond lower bound
        seq_octet = CF.DataType(id='seq_octet', value=any.to_any([-1, 0]))
        seq_short = CF.DataType(id='seq_short', value=any.to_any([-32769, 0]))
        seq_ushort = CF.DataType(id='seq_ushort', value=any.to_any([-1, 0]))
        seq_long = CF.DataType(id='seq_long', value=any.to_any([-2147483649, 0]))
        seq_ulong = CF.DataType(id='seq_ulong', value=any.to_any([-1, 0]))
        seq_ulonglong = CF.DataType(id='seq_ulonglong', value=any.to_any([-1, 0]))
        self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure,
                          [seq_octet, seq_short, seq_ushort, seq_long, seq_ulong, seq_ulonglong])

    def test_pythonPropsRangeStructSeq(self):
        self._app = self._launchApp('TestPythonPropsRange')

        # Struct with upper bound
        upper = CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), [
                            CF.DataType(id='ss_octet', value=CORBA.Any(CORBA.TC_long, 255)),
                            CF.DataType(id='ss_short', value=CORBA.Any(CORBA.TC_long, 32767)),
                            CF.DataType(id='ss_ushort', value=CORBA.Any(CORBA.TC_long, 65535)),
                            CF.DataType(id='ss_long', value=CORBA.Any(CORBA.TC_longlong, 2147483647)),
                            CF.DataType(id='ss_ulong', value=CORBA.Any(CORBA.TC_longlong, 4294967295)),
                            CF.DataType(id='ss_longlong', value=CORBA.Any(CORBA.TC_ulonglong, 9223372036854775807L)),
                            CF.DataType(id='ss_ulonglong', value=CORBA.Any(CORBA.TC_ulonglong, 18446744073709551615L))])
        # Struct with lower bound
        lower = CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), [
                            CF.DataType(id='ss_octet', value=CORBA.Any(CORBA.TC_long, 0)),
                            CF.DataType(id='ss_short', value=CORBA.Any(CORBA.TC_long, -32768)),
                            CF.DataType(id='ss_ushort', value=CORBA.Any(CORBA.TC_long, 0)),
                            CF.DataType(id='ss_long', value=CORBA.Any(CORBA.TC_longlong, -2147483648)),
                            CF.DataType(id='ss_ulong', value=CORBA.Any(CORBA.TC_longlong, 0)),
                            CF.DataType(id='ss_longlong', value=CORBA.Any(CORBA.TC_longlong, -9223372036854775808L)),
                            CF.DataType(id='ss_ulonglong', value=CORBA.Any(CORBA.TC_longlong, 0))])

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

        # Loop through each member of the struct to test one beyond the upper bound
        for r in res:
            if r.id == 'my_structseq':
                val = r.value.value()[1]
                for v in val.value():
                    # All default values are valid
                    octet_val = 0
                    short_val = 0
                    ushort_val = 0
                    long_val = 0
                    ulong_val = 0
                    longlong_val = 0
                    ulonglong_val = 0
                    # Creates struct with only 1 invalid member, which will still cause invalid configuration
                    if v.id == 'ss_octet':
                        octet_val = 256
                    elif v.id == 'ss_short':
                        short_val = 32768
                    elif v.id == 'ss_ushort':
                        ushort_val = 65536
                    elif v.id == 'ss_long':
                        long_val = 2147483648
                    elif v.id == 'ss_ulong':
                        ulong_val = 4294967296
                    elif v.id == 'ss_longlong':
                        longlong_val = 9223372036854775808L
                    elif v.id == 'ss_ulonglong':
                        # No value large enough to test outside range of ulonglong
                        continue
                    bad_struct = CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), [
                            CF.DataType(id='ss_octet', value=CORBA.Any(CORBA.TC_long, octet_val)),
                            CF.DataType(id='ss_short', value=CORBA.Any(CORBA.TC_long, short_val)),
                            CF.DataType(id='ss_ushort', value=CORBA.Any(CORBA.TC_long, ushort_val)),
                            CF.DataType(id='ss_long', value=CORBA.Any(CORBA.TC_longlong, long_val)),
                            CF.DataType(id='ss_ulong', value=CORBA.Any(CORBA.TC_longlong, ulong_val)),
                            CF.DataType(id='ss_longlong', value=CORBA.Any(CORBA.TC_ulonglong, longlong_val)),
                            CF.DataType(id='ss_ulonglong', value=CORBA.Any(CORBA.TC_ulonglong, ulonglong_val))])
                    my_structseq = CF.DataType(id='my_structseq',
                                               value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/AnySeq:1.0"),
                                                               [ upper , bad_struct ]
                                                            ))
                    self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, [my_structseq])

        # Loop through each member of the struct to test one beyond the lower bound
        for r in res:
            if r.id == 'my_structseq':
                val = r.value.value()[0]
                for v in val.value():
                    # All default values are valid
                    octet_val = 0
                    short_val = 0
                    ushort_val = 0
                    long_val = 0
                    ulong_val = 0
                    longlong_val = 0
                    ulonglong_val = 0
                    # Creates struct with only 1 invalid member, which will still cause invalid configuration
                    if v.id == 'ss_octet':
                        octet_val = -1
                    elif v.id == 'ss_short':
                        short_val = -32769
                    elif v.id == 'ss_ushort':
                        ushort_val = -1
                    elif v.id == 'ss_long':
                        long_val = -2147483649
                    elif v.id == 'ss_ulong':
                        ulong_val = -1
                    elif v.id == 'ss_longlong':
                        # No value to test below range of longlong
                        continue
                    elif v.id == 'ss_ulonglong':
                       ulonglong_val = -1
                    bad_struct = CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), [
                            CF.DataType(id='ss_octet', value=CORBA.Any(CORBA.TC_long, octet_val)),
                            CF.DataType(id='ss_short', value=CORBA.Any(CORBA.TC_long, short_val)),
                            CF.DataType(id='ss_ushort', value=CORBA.Any(CORBA.TC_long, ushort_val)),
                            CF.DataType(id='ss_long', value=CORBA.Any(CORBA.TC_longlong, long_val)),
                            CF.DataType(id='ss_ulong', value=CORBA.Any(CORBA.TC_longlong, ulong_val)),
                            CF.DataType(id='ss_longlong', value=CORBA.Any(CORBA.TC_longlong, longlong_val)),
                            CF.DataType(id='ss_ulonglong', value=CORBA.Any(CORBA.TC_longlong, ulonglong_val))])
                    my_structseq = CF.DataType(id='my_structseq',
                                               value=CORBA.Any(CORBA.TypeCode("IDL:omg.org/CORBA/AnySeq:1.0"),
                                                               [ bad_struct, lower ]
                                                ))
                    self.assertRaises(CF.PropertySet.InvalidConfiguration, self._app.configure, [my_structseq])

    def test_QueryBadValue(self):
        """
        Tests that invalid values in Python components do not break query()
        """
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")

        self._app = self._launchApp('TestPythonProps')
        self.assertNotEqual(self._app, None, "Application not created")

        pre_props = self._app.query([])

        # Set the internal variable to an invalid value
        # NB: In the future, we may disallow the ability to set properties from
        #     invalid values; this test will need to be updated
        self._app.configure([CF.DataType('test_float', any.to_any('bad string'))])

        # Try querying the broken property to check that the query doesn't
        # throw an unexpected exception
        self._app.query([CF.DataType('test_float', any.to_any(None))])

        # Try querying all properties to ensure that the invalid value does not
        # derail the entire query
        post_props = self._app.query([])
        self.assertEqual(len(pre_props), len(post_props))

class PyCallbacksTest(scatest.CorbaTestCase):
    def test_Callbacks(self):
        comp = sb.launch('PyCallbacks')

        # Clear callback log
        comp.callbacks_run = []

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
