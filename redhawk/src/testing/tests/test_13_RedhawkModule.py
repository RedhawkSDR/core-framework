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
import time
from omniORB import CORBA
from ossie.cf import CF
from ossie.utils import redhawk
from ossie.utils import type_helpers

class RedhawkModuleTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self._rhDom = redhawk.attach(scatest.getTestDomainName())
        self.assertEquals(len(self._rhDom._get_applications()), 0)

    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        redhawk.core._cleanUpLaunchedApps()
        scatest.CorbaTestCase.tearDown(self)
        # need to let event service clean up event channels...... 
        # cycle period is 10 milliseconds
        time.sleep(0.1)

    def preconditions(self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")

    def test_createApplication1(self):
        # Create Application from $SDRROOT path
        app = self._rhDom.createApplication("/waveforms/TestCppProps/TestCppProps.sad.xml")
        self.assertNotEqual(app, None, "Application not created")
        self.assertEquals(len(self._rhDom._get_applications()), 1)
        self.assertEquals(len(self._rhDom.apps), 1)

        # Ensure that api() works.
        try:
            app.api()
        except:
            self.fail('App.api() raised an exception')

        app2 = self._rhDom.createApplication("TestCppProps")
        self.assertNotEqual(app2, None, "Application not created")
        self.assertEquals(len(self._rhDom._get_applications()), 2)
        self.assertEquals(len(self._rhDom.apps), 2)

        app.releaseObject()
        self.assertEquals(len(self._rhDom._get_applications()), 1)
        self.assertEquals(len(self._rhDom.apps), 1)

        # Use exit functions from module to release other launched app
        redhawk.core._cleanUpLaunchedApps()
        self.assertEquals(len(self._rhDom.apps), 0)
        self.assertEquals(len(self._rhDom._get_applications()), 0)

    def test_cleanup_multiple_waves(self):
        # Create Application from $SDRROOT path
        app = self._rhDom.createApplication("/waveforms/TestCppProps/TestCppProps.sad.xml")
        self.assertNotEqual(app, None, "Application not created")
        self.assertEquals(len(self._rhDom._get_applications()), 1)
        self.assertEquals(len(self._rhDom.apps), 1)

        app2 = self._rhDom.createApplication("TestCppProps")
        self.assertNotEqual(app2, None, "Application not created")
        self.assertEquals(len(self._rhDom._get_applications()), 2)
        self.assertEquals(len(self._rhDom.apps), 2)

        app3 = self._rhDom.createApplication("TestCppProps")
        self.assertNotEqual(app3, None, "Application not created")
        self.assertEquals(len(self._rhDom._get_applications()), 3)
        self.assertEquals(len(self._rhDom.apps), 3)

        # Use exit functions from module to release launched apps
        redhawk.core._cleanUpLaunchedApps()
        self.assertEquals(len(self._rhDom.apps), 0)
        self.assertEquals(len(self._rhDom._get_applications()), 0)

    def test_largeShutdown(self):
        for i in range(16):
            self._rhDom.createApplication('TestCppProps')

        self.assertEquals(len(self._rhDom._get_applications()), 16)
        self.assertEquals(len(self._rhDom.apps), 16)

        apps = self._rhDom.apps

        for a in apps:
            a.start()
            a.stop()
            a.releaseObject()

        self.assertEquals(len(self._rhDom._get_applications()), 0)
        self.assertEquals(len(self._rhDom.apps), 0)

    def test_apiHostCollocation(self):
        app = self._rhDom.createApplication("/waveforms/through_w/through_w.sad.xml")
        provides_ports = object.__getattribute__(app,'_providesPortDict')
        self.assertEquals(provides_ports, {})
        uses_ports = object.__getattribute__(app,'_usesPortDict')
        self.assertEquals(uses_ports, {})
        app.api()
        provides_ports = object.__getattribute__(app,'_providesPortDict')
        self.assertEquals(len(provides_ports), 1)
        self.assertEquals(provides_ports.keys()[0], 'input')
        self.assertEquals(provides_ports['input']['Port Interface'], 'IDL:CF/LifeCycle:1.0')
        self.assertEquals(provides_ports['input']['Port Name'], 'input')
        uses_ports = object.__getattribute__(app,'_usesPortDict')
        self.assertEquals(uses_ports.keys()[0], 'output')
        self.assertEquals(uses_ports['output']['Port Interface'], 'IDL:CF/LifeCycle:1.0')
        self.assertEquals(uses_ports['output']['Port Name'], 'output')
        
    def test_appListSync(self):
        app = self._rhDom.createApplication("/waveforms/TestCppProps/TestCppProps.sad.xml")
        self.assertNotEqual(app, None, "Application not created")
        self.assertEquals(len(self._rhDom._get_applications()), 1)
        self.assertEquals(len(self._rhDom.apps), 1)

        # Make sure that an app created outside of the redhawk module still updates the app list inside
        self._domMgr.installApplication('/waveforms/TestCppProps/TestCppProps.sad.xml')
        appFact = self._domMgr._get_applicationFactories()[0]
        app2 = appFact.create(appFact._get_name(), [], [])
        # Give the domain a moment to process the ODM event
        time.sleep(0.1)
        self.assertEquals(len(self._rhDom.apps), 2)

        app2.releaseObject()
        app.releaseObject()
        # Give the domain a moment to process the ODM event
        time.sleep(0.1)
        self.assertEquals(len(self._rhDom.apps), 0)
        self.assertEquals(len(self._rhDom._get_applications()), 0)

    def test_simplePropertyRange(self):
        # Make sure setters and getters all work for simples
        app = self._rhDom.createApplication('/waveforms/TestPythonPropsRange/TestPythonPropsRange.sad.xml')

        # Test upper range
        app.my_octet_name = 255
        app.my_short_name = 32767
        app.my_ushort_name = 65535
        app.my_long_name = 2147483647
        app.my_ulong_name = 4294967295
        app.my_longlong_name = 9223372036854775807
        app.my_ulonglong_name = 18446744073709551615
        self.assertEquals(app.my_octet_name, 255)
        self.assertEquals(app.my_short_name, 32767)
        self.assertEquals(app.my_ushort_name, 65535)
        self.assertEquals(app.my_long_name, 2147483647)
        self.assertEquals(app.my_ulong_name, 4294967295)
        self.assertEquals(app.my_longlong_name, 9223372036854775807)
        self.assertEquals(app.my_ulonglong_name, 18446744073709551615)

        # Test lower range
        app.my_octet_name = 0
        app.my_short_name = -32768
        app.my_ushort_name = 0
        app.my_long_name = -2147483648
        app.my_ulong_name = 0
        app.my_longlong_name = -9223372036854775808
        app.my_ulonglong_name = 0
        self.assertEquals(app.my_octet_name, 0)
        self.assertEquals(app.my_short_name, -32768)
        self.assertEquals(app.my_ushort_name, 0)
        self.assertEquals(app.my_long_name, -2147483648)
        self.assertEquals(app.my_ulong_name, 0)
        self.assertEquals(app.my_longlong_name, -9223372036854775808)
        self.assertEquals(app.my_ulonglong_name, 0)

        # Test one beyond upper bound
        self.assertRaises(type_helpers.OutOfRangeException, app.my_octet_name.configureValue, 256)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_short_name.configureValue, 32768)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_ushort_name.configureValue, 65536)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_long_name.configureValue, 2147483648)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_ulong_name.configureValue, 4294967296)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_longlong_name.configureValue, 9223372036854775808)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_ulonglong_name.configureValue, 18446744073709551616)

        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, app.my_octet_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_short_name.configureValue, -32769)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_ushort_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_long_name.configureValue, -2147483649)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_ulong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_longlong_name.configureValue, -9223372036854775809)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_ulonglong_name.configureValue, -1)


    def test_structPropertyRange(self):
        # Make sure setters and getters all work for structs
        app = self._rhDom.createApplication('/waveforms/TestPythonPropsRange/TestPythonPropsRange.sad.xml')

        # Test upper range
        app.my_struct_name.struct_octet_name = 255
        app.my_struct_name.struct_short_name = 32767
        app.my_struct_name.struct_ushort_name = 65535
        app.my_struct_name.struct_long_name = 2147483647
        app.my_struct_name.struct_ulong_name = 4294967295
        app.my_struct_name.struct_longlong_name = 9223372036854775807
        app.my_struct_name.struct_ulonglong_name = 18446744073709551615
        app.my_struct_name.struct_seq_octet_name[1] = 255
        app.my_struct_name.struct_seq_short_name[1] = 32767
        app.my_struct_name.struct_seq_ushort_name[1] = 65535
        app.my_struct_name.struct_seq_long_name[1] = 2147483647
        app.my_struct_name.struct_seq_ulong_name[1] = 4294967295
        app.my_struct_name.struct_seq_longlong_name[1] = 9223372036854775807
        #app.my_struct_name.struct_seq_ulonglong_name[1] = 18446744073709551615
        self.assertEquals(app.my_struct_name.struct_octet_name, 255)
        self.assertEquals(app.my_struct_name.struct_short_name, 32767)
        self.assertEquals(app.my_struct_name.struct_ushort_name, 65535)
        self.assertEquals(app.my_struct_name.struct_long_name, 2147483647)
        self.assertEquals(app.my_struct_name.struct_ulong_name, 4294967295)
        self.assertEquals(app.my_struct_name.struct_longlong_name, 9223372036854775807)
        self.assertEquals(app.my_struct_name.struct_ulonglong_name, 18446744073709551615)
        self.assertEquals(app.my_struct_name.struct_seq_octet_name[1], 255)
        self.assertEquals(app.my_struct_name.struct_seq_short_name[1], 32767)
        self.assertEquals(app.my_struct_name.struct_seq_ushort_name[1], 65535)
        self.assertEquals(app.my_struct_name.struct_seq_long_name[1], 2147483647)
        self.assertEquals(app.my_struct_name.struct_seq_ulong_name[1], 4294967295)
        self.assertEquals(app.my_struct_name.struct_seq_longlong_name[1], 9223372036854775807)
        #self.assertEquals(app.my_struct_name.struct_seq_ulonglong_name[1], 18446744073709551615)

        # Test lower range
        app.my_struct_name.struct_octet_name = 0
        app.my_struct_name.struct_short_name = -32768
        app.my_struct_name.struct_ushort_name = 0
        app.my_struct_name.struct_long_name = -2147483648
        app.my_struct_name.struct_ulong_name = 0
        app.my_struct_name.struct_longlong_name = -9223372036854775808
        app.my_struct_name.struct_ulonglong_name = 0
        app.my_struct_name.struct_seq_octet_name[0] = 0
        app.my_struct_name.struct_seq_short_name[0] = -32768
        app.my_struct_name.struct_seq_ushort_name[0] = 0
        app.my_struct_name.struct_seq_long_name[0] = -2147483648
        app.my_struct_name.struct_seq_ulong_name[0] = 0
        app.my_struct_name.struct_seq_longlong_name[0] = -9223372036854775808
        app.my_struct_name.struct_seq_ulonglong_name[0] = 0
        self.assertEquals(app.my_struct_name.struct_octet_name, 0)
        self.assertEquals(app.my_struct_name.struct_short_name, -32768)
        self.assertEquals(app.my_struct_name.struct_ushort_name, 0)
        self.assertEquals(app.my_struct_name.struct_long_name, -2147483648)
        self.assertEquals(app.my_struct_name.struct_ulong_name, 0)
        self.assertEquals(app.my_struct_name.struct_longlong_name, -9223372036854775808)
        self.assertEquals(app.my_struct_name.struct_ulonglong_name, 0)
        self.assertEquals(app.my_struct_name.struct_seq_octet_name[0], 0)
        self.assertEquals(app.my_struct_name.struct_seq_short_name[0], -32768)
        self.assertEquals(app.my_struct_name.struct_seq_ushort_name[0], 0)
        self.assertEquals(app.my_struct_name.struct_seq_long_name[0], -2147483648)
        self.assertEquals(app.my_struct_name.struct_seq_ulong_name[0], 0)
        self.assertEquals(app.my_struct_name.struct_seq_longlong_name[0], -9223372036854775808)
        self.assertEquals(app.my_struct_name.struct_seq_ulonglong_name[0], 0)

        # Test one beyond upper bound
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_octet_name.configureValue, 256)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_short_name.configureValue, 32768)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_ushort_name.configureValue, 65536)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_long_name.configureValue, 2147483648)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_ulong_name.configureValue, 4294967296)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_longlong_name.configureValue, 9223372036854775808)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_ulonglong_name.configureValue, 18446744073709551616)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_seq_octet_name.configureValue, [0, 256])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_seq_short_name.configureValue, [0, 32768])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_seq_ushort_name.configureValue, [0, 65536])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_seq_long_name.configureValue, [0, 2147483648])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_seq_ulong_name.configureValue, [0, 4294967296])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_seq_longlong_name.configureValue, [0, 9223372036854775808])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_seq_ulonglong_name.configureValue, [0, 18446744073709551616])

        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_octet_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_short_name.configureValue, -32769)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_ushort_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_long_name.configureValue, -2147483649)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_ulong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_longlong_name.configureValue, -9223372036854775809)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_ulonglong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_seq_octet_name.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_seq_short_name.configureValue, [-32769, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_seq_ushort_name.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_seq_long_name.configureValue, [-2147483649, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_seq_ulong_name.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_seq_longlong_name.configureValue, [-9223372036854775809, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_struct_name.struct_seq_ulonglong_name.configureValue, [-1, 0])

        # Makes sure the struct can be set without error
        # NB: This test used to use names instead of ids, which silently failed in 1.8.
        new_value = {'struct_octet': 100, 'struct_short': 101, 'struct_ushort': 102, 'struct_long': 103,
                     'struct_ulong': 104, 'struct_longlong': 105, 'struct_ulonglong': 106, 'struct_seq_octet': [100, 101],
                     'struct_seq_short': [102, 103], 'struct_seq_ushort': [104, 105], 'struct_seq_long': [106L, 107L],
                     'struct_seq_ulong': [108L, 109L], 'struct_seq_longlong': [110L, 111L], 'struct_seq_ulonglong': [112L, 113L]}
        app.my_struct_name = new_value
        self.assertEquals(app.my_struct_name, new_value)


    def test_seqPropertyRange(self):
        # Make sure setters and getters all work for sequences
        app = self._rhDom.createApplication('/waveforms/TestPythonPropsRange/TestPythonPropsRange.sad.xml')

        # Test upper and lower bounds
        app.seq_octet_name[0] = 0
        app.seq_octet_name[1] = 255
        app.seq_short_name[0] = -32768
        app.seq_short_name[1] = 32767
        app.seq_ushort_name[0] = 0
        app.seq_ushort_name[1] = 65535
        app.seq_long_name[0] = -2147483648
        app.seq_long_name[1] = 2147483647
        app.seq_ulong_name[0] = 0
        app.seq_ulong_name[1] = 4294967295
        app.seq_longlong_name[0] = -9223372036854775808
        app.seq_longlong_name[1] = 9223372036854775807
        app.seq_ulonglong_name[0] = 0
        #app.seq_ulonglong_name[1] = 18446744073709551615
        self.assertEquals(app.seq_octet_name[0], 0)
        self.assertEquals(app.seq_octet_name[1], 255)
        self.assertEquals(app.seq_short_name[0], -32768)
        self.assertEquals(app.seq_short_name[1], 32767)
        self.assertEquals(app.seq_ushort_name[0], 0)
        self.assertEquals(app.seq_ushort_name[1], 65535)
        self.assertEquals(app.seq_long_name[0], -2147483648)
        self.assertEquals(app.seq_long_name[1], 2147483647)
        self.assertEquals(app.seq_ulong_name[0], 0)
        self.assertEquals(app.seq_ulong_name[1], 4294967295)
        self.assertEquals(app.seq_longlong_name[0], -9223372036854775808)
        self.assertEquals(app.seq_longlong_name[1], 9223372036854775807)
        self.assertEquals(app.seq_ulonglong_name[0], 0)
        #self.assertEauals(app.seq_ulonglong_name[1], 18446744073709551615)

        # Test one beyond upper bound
        self.assertRaises(type_helpers.OutOfRangeException, app.seq_octet.configureValue, [0, 256])
        self.assertRaises(type_helpers.OutOfRangeException, app.seq_short.configureValue, [0, 32768])
        self.assertRaises(type_helpers.OutOfRangeException, app.seq_ushort.configureValue, [0, 65536])
        self.assertRaises(type_helpers.OutOfRangeException, app.seq_long.configureValue, [0, 2147483648])
        self.assertRaises(type_helpers.OutOfRangeException, app.seq_ulong.configureValue, [0, 4294967296])
        self.assertRaises(type_helpers.OutOfRangeException, app.seq_longlong.configureValue, [0, 9223372036854775808])
        self.assertRaises(type_helpers.OutOfRangeException, app.seq_ulonglong.configureValue, [0, 18446744073709551616])

        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, app.seq_octet.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.seq_short.configureValue, [-32769, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.seq_ushort.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.seq_long.configureValue, [-2147483649, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.seq_ulong.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.seq_longlong.configureValue, [-9223372036854775809, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.seq_ulonglong.configureValue, [-1, 0])

        # Tests char and octet sequences
        self.assertRaises(TypeError, app.seq_char_name.configureValue, ['A','BB'])
        self.assertRaises(TypeError, app.seq_char_name.configureValue, ['a', 1])
        self.assertRaises(TypeError, app.seq_octet_name.configureValue, [1, 'a'])

        app.seq_char_name[0] = 'X'
        app.seq_char_name[1] = 'Y'
        self.assertEquals(app.seq_char_name[0], 'X')
        self.assertEquals(app.seq_char_name[1], 'Y')


    def test_structSeqPropertyRange(self):
        # Make sure setters and getters all work for struct sequences
        app = self._rhDom.createApplication('/waveforms/TestPythonPropsRange/TestPythonPropsRange.sad.xml')

        # Test upper and lower bounds
        app.my_structseq_name[0].ss_octet_name = 255
        app.my_structseq_name[1].ss_octet_name = 0
        app.my_structseq_name[0].ss_short_name = 32767
        app.my_structseq_name[1].ss_short_name = -32768
        app.my_structseq_name[0].ss_ushort_name = 65535
        app.my_structseq_name[1].ss_ushort_name = 0
        app.my_structseq_name[0].ss_long_name = 2147483647
        app.my_structseq_name[1].ss_long_name = -2147483648
        app.my_structseq_name[0].ss_ulong_name = 4294967295
        app.my_structseq_name[1].ss_ulong_name = 0
        app.my_structseq_name[0].ss_longlong_name = 9223372036854775807
        app.my_structseq_name[1].ss_longlong_name = -9223372036854775808
        app.my_structseq_name[0].ss_ulonglong_name = 18446744073709551615
        app.my_structseq_name[1].ss_ulonglong_name = 0
        app.my_structseq_name[0].ss_seq_octet_name[1] = 255
        app.my_structseq_name[1].ss_seq_octet_name[0] = 0
        app.my_structseq_name[0].ss_seq_short_name[1] = 32767
        app.my_structseq_name[1].ss_seq_short_name[0] = -32768
        app.my_structseq_name[0].ss_seq_ushort_name[1] = 65535
        app.my_structseq_name[1].ss_seq_ushort_name[0] = 0
        app.my_structseq_name[0].ss_seq_long_name[1] = 2147483647
        app.my_structseq_name[1].ss_seq_long_name[0] = -2147483648
        app.my_structseq_name[0].ss_seq_ulong_name[1] = 4294967295
        app.my_structseq_name[1].ss_seq_ulong_name[0] = 0
        app.my_structseq_name[0].ss_seq_longlong_name[1] = 9223372036854775807
        app.my_structseq_name[1].ss_seq_longlong_name[0] = -9223372036854775808
        #app.my_structseq_name[0].ss_seq_ulonglong_name[1] = 18446744073709551615
        app.my_structseq_name[1].ss_seq_ulonglong_name[0] = 0
        self.assertEquals(app.my_structseq_name[0].ss_octet_name, 255)
        self.assertEquals(app.my_structseq_name[1].ss_octet_name, 0)
        self.assertEquals(app.my_structseq_name[0].ss_short_name, 32767)
        self.assertEquals(app.my_structseq_name[1].ss_short_name, -32768)
        self.assertEquals(app.my_structseq_name[0].ss_ushort_name, 65535)
        self.assertEquals(app.my_structseq_name[1].ss_ushort_name, 0)
        self.assertEquals(app.my_structseq_name[0].ss_long_name, 2147483647)
        self.assertEquals(app.my_structseq_name[1].ss_long_name, -2147483648)
        self.assertEquals(app.my_structseq_name[0].ss_ulong_name, 4294967295)
        self.assertEquals(app.my_structseq_name[1].ss_ulong_name, 0)
        self.assertEquals(app.my_structseq_name[0].ss_longlong_name, 9223372036854775807)
        self.assertEquals(app.my_structseq_name[1].ss_longlong_name, -9223372036854775808)
        self.assertEquals(app.my_structseq_name[0].ss_ulonglong_name, 18446744073709551615)
        self.assertEquals(app.my_structseq_name[1].ss_ulonglong_name, 0)
        self.assertEquals(app.my_structseq_name[0].ss_seq_octet_name[1], 255)
        self.assertEquals(app.my_structseq_name[1].ss_seq_octet_name[0], 0)
        self.assertEquals(app.my_structseq_name[0].ss_seq_short_name[1], 32767)
        self.assertEquals(app.my_structseq_name[1].ss_seq_short_name[0], -32768)
        self.assertEquals(app.my_structseq_name[0].ss_seq_ushort_name[1], 65535)
        self.assertEquals(app.my_structseq_name[1].ss_seq_ushort_name[0], 0)
        self.assertEquals(app.my_structseq_name[0].ss_seq_long_name[1], 2147483647)
        self.assertEquals(app.my_structseq_name[1].ss_seq_long_name[0], -2147483648)
        self.assertEquals(app.my_structseq_name[0].ss_seq_ulong_name[1], 4294967295)
        self.assertEquals(app.my_structseq_name[1].ss_seq_ulong_name[0], 0)
        self.assertEquals(app.my_structseq_name[0].ss_seq_longlong_name[1], 9223372036854775807)
        self.assertEquals(app.my_structseq_name[1].ss_seq_longlong_name[0], -9223372036854775808)
        #self.assertEquals(app.my_structseq_name[0].ss_seq_ulonglong_name[1], 18446744073709551615)
        self.assertEquals(app.my_structseq_name[1].ss_seq_ulonglong_name[0], 0)

        # Test one beyond upper bound
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[0].ss_octet_name.configureValue, 256)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[0].ss_short_name.configureValue, 32768)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[0].ss_ushort_name.configureValue, 65536)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[0].ss_long_name.configureValue, 2147483648)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[0].ss_ulong_name.configureValue, 4294967296)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[0].ss_longlong_name.configureValue, 9223372036854775808)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[0].ss_ulonglong_name.configureValue, 18446744073709551616)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[0].ss_seq_octet_name.configureValue, [0, 256])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[0].ss_seq_short_name.configureValue, [0, 32768])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[0].ss_seq_ushort_name.configureValue, [0, 65536])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[0].ss_seq_long_name.configureValue, [0, 2147483648])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[0].ss_seq_ulong_name.configureValue, [0, 4294967296])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[0].ss_seq_longlong_name.configureValue, [0, 9223372036854775808])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[0].ss_seq_ulonglong_name.configureValue, [0, 18446744073709551616])

        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[1].ss_octet_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[1].ss_short_name.configureValue, -32769)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[1].ss_ushort_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[1].ss_long_name.configureValue, -2147483649)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[1].ss_ulong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[1].ss_longlong_name.configureValue, -9223372036854775809)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[1].ss_ulonglong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[1].ss_seq_octet_name.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[1].ss_seq_short_name.configureValue, [-32769, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[1].ss_seq_ushort_name.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[1].ss_seq_long_name.configureValue, [-2147483649, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[1].ss_seq_ulong_name.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[1].ss_seq_longlong_name.configureValue, [-9223372036854775809, 0])
        self.assertRaises(type_helpers.OutOfRangeException, app.my_structseq_name[1].ss_seq_ulonglong_name.configureValue, [-1, 0])

        # Make sure entire struct seq can be set without error
        new_value = [{'ss_octet': 100, 'ss_short': 101, 'ss_ushort': 102, 'ss_long': 103,
                      'ss_ulong': 104, 'ss_longlong': 105, 'ss_ulonglong': 106, 'ss_seq_octet': [100, 101],
                      'ss_seq_short': [102, 103], 'ss_seq_ushort': [104, 105], 'ss_seq_long': [106L, 107L],
                      'ss_seq_ulong': [108L, 109L], 'ss_seq_longlong': [110L, 111L], 'ss_seq_ulonglong': [112L, 113L]},
                     {'ss_octet': 107, 'ss_short': 108, 'ss_ushort': 109, 'ss_long': 110,
                      'ss_ulong': 111, 'ss_longlong': 112, 'ss_ulonglong': 113, 'ss_seq_octet': [114, 115],
                      'ss_seq_short': [116, 117], 'ss_seq_ushort': [118, 119], 'ss_seq_long': [120L, 121L],
                      'ss_seq_ulong': [122L, 123L], 'ss_seq_longlong': [124L, 125L], 'ss_seq_ulonglong': [126L, 127L]}]
        app.my_structseq_name = new_value
        self.assertEqual(app.my_structseq_name, new_value)

        # Make sure individual structs can be set without error
        # NB: This test used to use names instead of ids, which silently failed in 1.8.
        for item in new_value:
            for name in item.iterkeys():
                if isinstance(item[name], list):
                    for i in item[name]:
                        i += 100
                else:
                    item[name] = item[name] + 100
        app.my_structseq_name[0] = new_value[0]
        app.my_structseq_name[1] = new_value[1]
        self.assertEqual(app.my_structseq_name, new_value)

    def test_createApplicationName(self):
        """
        Tests that createAppliction() allows overriding the instance name of
        the application
        """
        sadfile = '/waveforms/TestCppProps/TestCppProps.sad.xml'
        name = 'TestWave'

        # Create an app and make sure that the given name was used as a prefix
        app1 = self._rhDom.createApplication(sadfile, name)
        self.assertNotEqual(app1, None, 'Application not created')
        self.assert_(app1.ns_name.startswith(name))

        # Create a second instance, and verify that the names are unique
        app2 = self._rhDom.createApplication(sadfile, name)
        self.assertNotEqual(app2, None, 'Application not created')
        self.assert_(app2.ns_name.startswith(name))
        self.assertNotEqual(app1.ns_name, app2.ns_name)

    def test_createApplication1InitConfig(self):
        """
        Tests that createAppliction() allows overriding the initial
        configuration of the application
        """
        sadfile = '/waveforms/TestPythonPropsRange/TestPythonPropsRange.sad.xml'

        # Use a dictionary to override the properties; at present, this only
        # supports the use of property IDs
        props = {'my_long':1000}
        app1 = self._rhDom.createApplication(sadfile, initConfiguration=props)
        for name, value in props.iteritems():
            self.assertEqual(getattr(app1, name), value)

        # Use CF.Properties to override the properties
        props = [CF.DataType('my_short', CORBA.Any(CORBA.TC_short, -300))]
        app2 = self._rhDom.createApplication(sadfile, initConfiguration=props)
        for dt in props:
            value = dt.value.value()
            self.assertEqual(getattr(app2, dt.id), value)

    def test_connect(self):
        """
        Tests that applications can make connections between their external ports
        """
        self.launchDeviceManager('/nodes/test_PortTestDevice_node/DeviceManager.dcd.xml')

        app1 = self._rhDom.createApplication('/waveforms/PortConnectExternalPort/PortConnectExternalPort.sad.xml')
        app2 = self._rhDom.createApplication('/waveforms/PortConnectExternalPort/PortConnectExternalPort.sad.xml')

        # Tally up the connections prior to making an app-to-app connection;
        # the PortTest component's runTest method returns the identifiers of
        # any connected Resources
        pre = []
        for comp in app1.comps + app2.comps:
            pre.extend(comp.runTest(0, []))

        app1.connect(app2)

        # Tally up the connections to check that a new one has been made
        post = []
        for comp in app1.comps + app2.comps:
            post.extend(comp.runTest(0, []))

        self.assertTrue(len(post) > len(pre))
