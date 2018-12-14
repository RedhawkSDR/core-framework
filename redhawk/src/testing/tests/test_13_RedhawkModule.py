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
import contextlib
import cStringIO
import tempfile
import re
import sys as _sys
from omniORB import CORBA
from omniORB import any as _any
from xml.dom import minidom
import os as _os
import Queue
import StringIO
from ossie.cf import CF
from ossie.utils import redhawk
from ossie.utils import type_helpers
from ossie.utils import rhconnection
from ossie.utils import allocations
from ossie.utils import sb
from ossie.utils.model import NoMatchingPorts
from ossie.events import Subscriber, Publisher
from ossie.cf import CF
import traceback

class RedhawkModuleEventChannelTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        self.ecm = self._domMgr._get_eventChannelMgr()
        self.channelName = 'TestChan'
        try:
            self.channel = self.ecm.create(self.channelName)
        except CF.EventChannelManager.ChannelAlreadyExists:
            pass
        self.channel = self.ecm.get(self.channelName)

    def tearDown(self):
        try:
            self.ecm.release(self.channelName)
        except CF.EventChannelManager.ChannelDoesNotExist:
            pass
        scatest.CorbaTestCase.tearDown(self)

    def _waitData(self, sub, timeout):
        end = time.time() + timeout
        while time.time() < end:
            data = sub.getData()
            if data:
                return data._v
        return None

    def test_eventChannelPull(self):
        sub = Subscriber(self._domMgr, self.channelName)
        pub = Publisher(self._domMgr, self.channelName)
        payload = 'hello'
        data = _any.to_any(payload)
        pub.push(data)
        rec_data = self._waitData(sub, 1.0)
        self.assertEquals(rec_data, payload)
        pub.terminate()
        sub.terminate()

    def test_eventChannelForceDelete(self):
        sub = Subscriber(self._domMgr, self.channelName)
        pub = Publisher(self._domMgr, self.channelName)
        payload = 'hello'
        data = _any.to_any(payload)
        pub.push(data)
        rec_data = self._waitData(sub, 1.0)
        self.assertEquals(rec_data, payload)
        self.ecm.forceRelease(self.channelName)
        self.assertRaises(CF.EventChannelManager.ChannelDoesNotExist, self.ecm.release, self.channelName)
        
    def test_eventChannelCB(self):
        queue = Queue.Queue()
        sub = Subscriber(self._domMgr, self.channelName, dataArrivedCB=queue.put)
        pub = Publisher(self._domMgr, self.channelName)
        payload = 'hello'
        data = _any.to_any(payload)
        pub.push(data)
        rec_data = queue.get(timeout=1.0)
        self.assertEquals(rec_data._v, payload)
        pub.terminate()
        sub.terminate()

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
        redhawk.setTrackApps(False)

    def preconditions(self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")

    def test_API_remap(self):
        import _omnipy
        v=int(_omnipy.__version__[0])
        orig_api = dir(self._rhDom.ref)
        remap_api = dir(self._rhDom)
        not_remap = ['_NP_RepositoryId','_Object__release','__getattribute__','__getstate__','__hash__','__setattr__','__setstate__','__weakref__',
                     '__methods__','_duplicate','_dynamic_op','_hash','_is_a','_is_equivalent','_narrow','_nil','_obj',
                     '__del__','__omni_obj','_release','_unchecked_narrow', '_non_existent',
                     'retrieve_records', 'retrieve_records_by_date', 'retrieve_records_from_date'  ]
        if  v > 3 :  not_remap += ['log_level']
        for entry in orig_api:
            if entry in not_remap:
                continue
            self.assertEquals(entry in remap_api, True)
            
        orig_api = dir(self._rhDom.devMgrs[0].ref)
        remap_api = dir(self._rhDom.devMgrs[0])
        for entry in orig_api:
            if entry in not_remap:
                continue
            self.assertEquals(entry in remap_api, True)
            
        app = self._rhDom.createApplication("/waveforms/TestCppProps/TestCppProps.sad.xml")
        self.assertNotEqual(app, None, "Application not created")
        self.assertEquals(len(self._rhDom._get_applications()), 1)
        self.assertEquals(len(self._rhDom.apps), 1)

        orig_api = dir(self._rhDom.apps[0].comps[0].ref)
        remap_api = dir(self._rhDom.apps[0].comps[0])
        for entry in orig_api:
            if entry in not_remap:
                continue
            self.assertEquals(entry in remap_api, True)

        orig_api = dir(self._rhDom.apps[0].ref)
        remap_api = dir(self._rhDom.apps[0])
        for entry in orig_api:
            if entry in not_remap:
                continue
            self.assertEquals(entry in remap_api, True)

        orig_api = dir(self._rhDom.devMgrs[0].devs[0].ref)
        remap_api = dir(self._rhDom.devMgrs[0].devs[0])
        for entry in orig_api:
            if entry in not_remap:
                continue
            self.assertEquals(entry in remap_api, True)

    def test_createBadCompApplication(self):
        # Automatically clean up
        redhawk.setTrackApps(True)
        # Create Application from $SDRROOT path
        app = self._rhDom.createApplication("/waveforms/svc_fn_error_cpp_w/svc_fn_error_cpp_w.sad.xml")
        app_2 = self._rhDom.createApplication("/waveforms/svc_one_error_w/svc_one_error_w.sad.xml")
        self.assertNotEqual(app, None, "Application not created")
        self.assertEquals(len(self._rhDom._get_applications()), 2)
        self.assertEquals(len(self._rhDom.apps), 2)

        app.start()
        app_2.start()

        time.sleep(0.5)

        self.assertEquals(app.comps, [])
        self.assertEquals(len(app_2.comps), 2)

    def test_createApplication1(self):
        # Automatically clean up
        redhawk.setTrackApps(True)
        # Create Application from $SDRROOT path
        app = self._rhDom.createApplication("/waveforms/TestCppProps/TestCppProps.sad.xml")
        self.assertNotEqual(app, None, "Application not created")
        self.assertEquals(len(self._rhDom._get_applications()), 1)
        self.assertEquals(len(self._rhDom.apps), 1)

        # Ensure that api() works.
        _destfile=StringIO.StringIO()
        try:
            app.api(destfile=_destfile)
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


    def test_createApplication_namespaced(self):
        # Automatically clean up
        redhawk.setTrackApps(True)
        # Create Application from $SDRROOT path
        app = self._rhDom.createApplication("simple_ns.c2_comp")
        self.assertNotEqual(app, None, "Application not created")
        self.assertEquals(len(self._rhDom._get_applications()), 1)
        self.assertEquals(len(self._rhDom.apps), 1)


        app2 = self._rhDom.createApplication("c2_comp")
        self.assertNotEqual(app2, None, "Application not created")
        self.assertEquals(len(self._rhDom._get_applications()), 2)
        self.assertEquals(len(self._rhDom.apps), 2)

        app.releaseObject()
        self.assertEquals(len(self._rhDom._get_applications()), 1)
        self.assertEquals(len(self._rhDom.apps), 1)

        self.assertRaises(CF.DomainManager.ApplicationInstallationError,self._rhDom.createApplication, "this.should.fail.c2_comp")

        # Use exit functions from module to release other launched app
        redhawk.core._cleanUpLaunchedApps()
        self.assertEquals(len(self._rhDom.apps), 0)
        self.assertEquals(len(self._rhDom._get_applications()), 0)

    def test_createApplicationNoCleanup(self):
        # Create Application from $SDRROOT path
        app = self._rhDom.createApplication("/waveforms/TestCppProps/TestCppProps.sad.xml")
        self.assertNotEqual(app, None, "Application not created")
        self.assertEquals(len(self._rhDom._get_applications()), 1)
        self.assertEquals(len(self._rhDom.apps), 1)

        # Ensure that api() works.
        _destfile=StringIO.StringIO()
        try:
            app.api(destfile=_destfile)
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
        self.assertEquals(len(self._rhDom.apps), 1)
        self.assertEquals(len(self._rhDom._get_applications()), 1)

    def test_cleanup_multiple_waves(self):
        # Automatically clean up
        redhawk.setTrackApps(True)
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
        _destfile=StringIO.StringIO()
        app.api(destfile=_destfile)
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
        #self.assertEquals(app.seq_ulonglong_name[1], 18446744073709551615)

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

    def test_createApplicationDeviceAssignment(self):
        """
        Tests that createApplication() allows the definition of a Device Assignment sequence
        """
        sadfile = '/waveforms/TestCppProps/TestCppProps.sad.xml'
        name = 'TestWave'
        initConfig = {}
        devAssignment = []
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node_2/DeviceManager.dcd.xml")
        
        devAssignment.append(CF.DeviceAssignmentType('DCE:87f4687e-6b67-458c-a32d-bef8f99a1064',''))
        self.assertEqual(len(self._rhDom._get_deviceManagers()), 2)
        dev_1 = self._rhDom.devMgrs[0].devs[0]._get_identifier()
        dev_2 = self._rhDom.devMgrs[1].devs[0]._get_identifier()
        
        devAssignment[0].assignedDeviceId = dev_1
        app = self._rhDom.createApplication(sadfile, name, {}, devAssignment)
        self.assertNotEqual(app, None, 'Application not created')
        self.assertEquals(app._get_componentDevices()[0].assignedDeviceId, dev_1)
        app.releaseObject()
        
        devAssignment[0].assignedDeviceId = dev_2
        app = self._rhDom.createApplication(sadfile, name, {}, devAssignment)
        self.assertNotEqual(app, None, 'Application not created')
        self.assertEquals(app._get_componentDevices()[0].assignedDeviceId, dev_2)
        app.releaseObject()

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

        app1.connect(app2, usesPortName="resource_out")

        # Tally up the connections to check that a new one has been made
        post = []
        for comp in app1.comps + app2.comps:
            post.extend(comp.runTest(0, []))

        self.assertTrue(len(post) > len(pre))

    def test_connectionMgrApp(self):
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
        
        ep1=rhconnection.makeEndPoint(app1, 'resource_out')
        ep2=rhconnection.makeEndPoint(app2, '')
        cMgr = self._rhDom._get_connectionMgr()
        cMgr.connect(ep1,ep2)

        # Tally up the connections to check that a new one has been made
        post = []
        for comp in app1.comps + app2.comps:
            post.extend(comp.runTest(0, []))

        self.assertTrue(len(post) > len(pre))

    def test_connectionMgrComp(self):
        """
        Tests that applications can make connections between their external ports
        """
        self.launchDeviceManager('/nodes/test_PortTestDevice_node/DeviceManager.dcd.xml')

        app1 = self._rhDom.createApplication('/waveforms/PortConnectExternalPort/PortConnectExternalPort.sad.xml')
        app2 = self._rhDom.createApplication('/waveforms/PortConnectExternalPort/PortConnectExternalPort.sad.xml')

        foundcomp=False
        for _comp in app1.comps:
            if _comp._id[:33] == 'PortTest1:PortConnectExternalPort':
                self.assertEquals(_comp.instanceName, "PortTest1")
                foundcomp = True
                break
        self.assertTrue(foundcomp)

        for _port in _comp.ports:
            if _port.name == 'resource_out':
                break

        self.assertEquals(len(_port._get_connections()), 0)

        ep1=rhconnection.makeEndPoint(_comp, 'resource_out')
        print ep1
        ep2=rhconnection.makeEndPoint(app2, '')
        print ep2
        cMgr = self._rhDom._get_connectionMgr()
        cMgr.connect(ep1,ep2)

        self.assertEquals(len(_port._get_connections()), 1)

class RedhawkModuleAllocationMgrTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/dev_alloc_node/DeviceManager.dcd.xml")
        self._rhDom = redhawk.attach(scatest.getTestDomainName())
        self.am=self._rhDom._get_allocationMgr()
        self.assertEquals(len(self._rhDom._get_applications()), 0)

    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        redhawk.core._cleanUpLaunchedApps()
        scatest.CorbaTestCase.tearDown(self)
        # need to let event service clean up event channels...... 
        # cycle period is 10 milliseconds
        time.sleep(0.1)
        redhawk.setTrackApps(False)

    def test_allocMgrSimple(self):
        """
        Tests that applications can make connections between their external ports
        """
        prop = allocations.createProps({'si_prop':3})
        rq=self.am.createRequest('foo',prop)
        resp = self.am.allocate([rq])
        self.assertEquals(len(resp),1)
        self.assertEquals(self.am.listAllocations(CF.AllocationManager.LOCAL_ALLOCATIONS, 100)[0][0].allocationID, resp[0].allocationID)
        self.am.deallocate([resp[0].allocationID])

    def test_allocMgrSimSeq(self):
        """
        Tests that applications can make connections between their external ports
        """
        prop = allocations.createProps({'se_prop':[1.0,2.0]})
        rq=self.am.createRequest('foo',prop)
        resp = self.am.allocate([rq])
        self.assertEquals(len(resp),1)
        self.am.deallocate([resp[0].allocationID])
        prop = allocations.createProps({'se_prop':[1.0,2.0]}, prf='sdr/dev/devices/dev_alloc_cpp/dev_alloc_cpp.prf.xml')
        rq=self.am.createRequest('foo',prop)
        resp = self.am.allocate([rq])
        self.assertEquals(len(resp),1)
        self.assertEquals(self.am.listAllocations(CF.AllocationManager.LOCAL_ALLOCATIONS, 100)[0][0].allocationID, resp[0].allocationID)
        self.am.deallocate([resp[0].allocationID])

    def test_allocMgrStruct(self):
        """
        Tests that applications can make connections between their external ports
        """
        prop = allocations.createProps({'s_prop':{'s_prop::a':'hello','s_prop::b':5}})
        rq=self.am.createRequest('foo',prop)
        resp = self.am.allocate([rq])
        self.assertEquals(len(resp),1)
        self.am.deallocate([resp[0].allocationID])
        prop = allocations.createProps({'s_prop':{'s_prop::a':'hello','s_prop::b':5}}, prf='sdr/dev/devices/dev_alloc_cpp/dev_alloc_cpp.prf.xml')
        rq=self.am.createRequest('foo',prop)
        resp = self.am.allocate([rq])
        self.assertEquals(len(resp),1)
        self.assertEquals(self.am.listAllocations(CF.AllocationManager.LOCAL_ALLOCATIONS, 100)[0][0].allocationID, resp[0].allocationID)
        self.am.deallocate([resp[0].allocationID])

    def test_allocMgrStrSeq(self):
        """
        Tests that applications can make connections between their external ports
        """
        prop = allocations.createProps({'sq_prop':[{'sq_prop::b':'hello','sq_prop::a':5},{'sq_prop::b':'another','sq_prop::a':7}]})
        rq=self.am.createRequest('foo',prop)
        resp = self.am.allocate([rq])
        self.assertEquals(len(resp),1)
        self.am.deallocate([resp[0].allocationID])
        prop = allocations.createProps({'sq_prop':[{'sq_prop::b':'hello','sq_prop::a':5},{'sq_prop::b':'another','sq_prop::a':7}]}, prf='sdr/dev/devices/dev_alloc_cpp/dev_alloc_cpp.prf.xml')
        rq=self.am.createRequest('foo',prop)
        resp = self.am.allocate([rq])
        self.assertEquals(len(resp),1)
        self.assertEquals(self.am.listAllocations(CF.AllocationManager.LOCAL_ALLOCATIONS, 100)[0][0].allocationID, resp[0].allocationID)
        self.am.deallocate([resp[0].allocationID])

class MixedRedhawkSandboxTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self._rhDom = redhawk.attach(scatest.getTestDomainName())

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)
        sb.release()

    def test_BadApplicationConnection(self):
        """
        Tests that attempting to connect a Sandbox source or sink to an
        application that does not have matching ports throws the correct
        exception type (as opposed to an AttributeError).
        """
        app = self._rhDom.createApplication("/waveforms/TestCppProps/TestCppProps.sad.xml")

        source = sb.DataSource()
        self.assertRaises(NoMatchingPorts, source.connect, app)

        sink = sb.DataSink()
        self.assertRaises(NoMatchingPorts, app.connect, source)


class DomainMgrLoggingAPI(scatest.CorbaTestCase):
    def setUp(self):
        self.lcfg=_os.environ['OSSIEUNITTESTSLOGCONFIG']
        _os.environ['OSSIEUNITTESTSLOGCONFIG']=""
        domBooter, self._domMgr = self.launchDomainManager()
        self.dom = redhawk.attach(scatest.getTestDomainName())

    def tearDown(self):
        redhawk.core._cleanUpLaunchedApps()
        scatest.CorbaTestCase.tearDown(self)
        time.sleep(0.1)
        _os.environ['OSSIEUNITTESTSLOGCONFIG']=self.lcfg

    def test123log_level(self):
        """
        Tests set debug level api is working
        """
        from ossie.cf import CF
        self.assertNotEqual( self.dom, None )

        self.dom.ref._set_log_level( CF.LogLevels.TRACE  )
        ret=self.dom.ref._get_log_level( )
        self.assertEqual( ret, CF.LogLevels.TRACE )

        self.dom.ref._set_log_level( CF.LogLevels.DEBUG )
        ret=self.dom.ref._get_log_level()
        self.assertEqual( ret, CF.LogLevels.DEBUG )

        self.dom.ref._set_log_level( CF.LogLevels.INFO )
        ret=self.dom.ref._get_log_level()
        self.assertEqual( ret, CF.LogLevels.INFO )

        self.dom.ref._set_log_level( CF.LogLevels.WARN )
        ret=self.dom.ref._get_log_level()
        self.assertEqual( ret, CF.LogLevels.WARN )

        self.dom.ref._set_log_level( CF.LogLevels.ERROR )
        ret=self.dom.ref._get_log_level()
        self.assertEqual( ret, CF.LogLevels.ERROR )

        self.dom.ref._set_log_level( CF.LogLevels.FATAL )
        ret=self.dom.ref._get_log_level()
        self.assertEqual( ret, CF.LogLevels.FATAL )

    def test_default_logconfig(self):
        cfg = "log4j.rootLogger=INFO,STDOUT\n" + \
              "# Direct log messages to STDOUT\n" + \
              "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
              "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
              "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{3}:%L - %m%n\n"

        c_cfg=self.dom.ref.getLogConfig()

        ## remove extra white space
        cfg=cfg.replace(" ","")
        c_cfg=c_cfg.replace(" ","")
        self.assertEquals( cfg, c_cfg)


    def test_logconfig(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT\n" + \
            "# Direct log messages to STDOUT\n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c{1}:%L - %m%n\n"

        self.dom.ref.setLogConfig(cfg)

        c_cfg=self.dom.ref.getLogConfig()
        cfg=cfg.replace(" ","")
        c_cfg=c_cfg.replace(" ","")
        self.assertEquals( cfg, c_cfg)


    def test_macro_config(self):
        cfg = "log4j.rootLogger=ERROR,STDOUT\n " + \
            "# Direct log messages to STDOUT\n" + \
            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.STDOUT.layout.ConversionPattern=@@@DOMAIN.NAME@@@\n"

        self.dom.ref.setLogConfig(cfg)

        c_cfg=self.dom.ref.getLogConfig()

        res=c_cfg.find(scatest.getTestDomainName())

        self.assertNotEquals( res, -1 )

    def test_macro_config2(self):
        cfg = "@@@DOMAIN.NAME@@@"
        self.dom.ref.setLogConfig(cfg)
        c_cfg=self.dom.ref.getLogConfig()
        res=c_cfg.find(scatest.getTestDomainName())
        self.assertNotEquals( res, -1 )




class RedhawkStartup(scatest.CorbaTestCase):
    def setUp(self):
        pass

    def tearDown(self):
        redhawk.base._cleanup_domain()
        scatest.CorbaTestCase.tearDown(self)
        import commands
        try:
            s,o = commands.getstatusoutput('pkill -9 -f nodeBooter ')
            s,o = commands.getstatusoutput('pkill -9 -f dev/devices ')
            s,o = commands.getstatusoutput('pkill -9 -f DomainManager ')
            s,o = commands.getstatusoutput('pkill -9 -f DeviceManager ')
        except:
            pass

    def _try_kick_domain(self, logcfg, epatterns, debug_level=None,  kick_device_managers=False, dev_mgrs=[], dev_mgr_levels=[] ):

        tmpfile=tempfile.mktemp()
        self._rhDom = redhawk.kickDomain(domain_name=scatest.getTestDomainName(),
                                         logfile=scatest.getSdrPath()+'/dom/logcfg/'+logcfg,
                                         kick_device_managers=kick_device_managers,
                                         device_managers = dev_mgrs,
                                         stdout=tmpfile,
                                         debug_level=debug_level,
                                         device_managers_debug_levels = dev_mgr_levels )

        if kick_device_managers and len(dev_mgrs)> 0 :
            for devm in dev_mgrs:
                try:
                    self.waitForDeviceManager(devm)
                except:
                    traceback.print_exc()
                    pass

	time.sleep(2)
        new_stdout=open(tmpfile,'r')
        for k, epat in epatterns.iteritems():
            epat.setdefault('results',[])

        for x in new_stdout.readlines():
            #print "Line -> ", x
            for k, pat in epatterns.iteritems():
                for epat in pat['patterns' ]:
                    m=re.search( epat, x )
                    if m :
                        #print "MATCH  -> ", epat, " LINE ", x
                        pat['results'].append(True)

        for k,pat in epatterns.iteritems():
            if type(pat['match']) == list:
                lmatch = len(pat['results']) == len(pat['match']) and pat['results'] == pat['match']
                self.assertEqual(lmatch, True )

            if type(pat['match']) == tuple:
                   reslen=len(pat['results'])
                   c=pat['match']
	           res=eval("'" + str(reslen) + " " + str(c[0]) + " " + str(c[1]) + "'")
                   self.assertTrue(res)

            if type(pat['match']) == int:
                # ignore
                if pat['match'] == -1 :
                    pass
                else:
                   self.assertEqual( pat['match'] , len(pat['results']) )

    @scatest.requireLog4cxx
    def test_kick_trace(self):
        self._try_kick_domain('log4j.kickdomain.cfg',
                              epatterns={
                                          "yes" :  { 'patterns':  [" TRACE ", " DEBUG ", " INFO ", " WARN ", " ERROR ", " FATAL " ], 'match': -1 },
                                          },
                              debug_level="TRACE")

    @scatest.requireLog4cxx
    def test_kick_trace_both(self):
        self._try_kick_domain('log4j.kickdomain.cfg',
                              epatterns={
                                          "yes" :  { 'patterns':  [" TRACE ", " DEBUG ", " INFO ", " WARN ", " ERROR ", " FATAL " ], 'match': -1 },
                                          },
                              debug_level="TRACE",
                              kick_device_managers=True,
                              dev_mgrs = [ 'test_BasicTestDevice_node' ],
                              dev_mgr_levels= [ "TRACE" ],
                                )

    @scatest.requireLog4cxx
    def test_kick_debug(self):
        self._try_kick_domain('log4j.kickdomain.cfg',
                              epatterns={ "no" :  { 'patterns':  [" TRACE " ], 'match': [] },
                                          "yes" :  { 'patterns':  [" DEBUG ", " INFO ", " WARN ", " ERROR ", " FATAL " ], 'match': -1 },
                                          },
                              debug_level="DEBUG")


    @scatest.requireLog4cxx
    def test_kick_debug_both(self):
        self._try_kick_domain('log4j.kickdomain.cfg',
                              epatterns={ "no" :  { 'patterns':  [" TRACE " ], 'match': [] },
                                          "yes" :  { 'patterns':  [ " DEBUG ", " INFO ", " WARN ", " ERROR ", " FATAL " ], 'match': -1 },
                                          },
                              debug_level="DEBUG",
                              kick_device_managers=True,
                              dev_mgrs = [ 'test_BasicTestDevice_node', "test_BasicTestDevice2_node" ],
                              dev_mgr_levels= [ "DEBUG", "DEBUG" ]
                                )

    @scatest.requireLog4cxx
    def test_kick_info(self):
        self._try_kick_domain('log4j.kickdomain.cfg',
                              epatterns={ "no" :  { 'patterns':  [" TRACE ", " DEBUG " ], 'match': [] },
                                          "yes" :  { 'patterns':  [ " INFO ", " WARN ", " ERROR ", " FATAL " ], 'match': -1 },
                                          },
                              debug_level="INFO")

    @scatest.requireLog4cxx
    def test_kick_info_both(self):
        self._try_kick_domain('log4j.kickdomain.cfg',
                              epatterns={ "no" :  { 'patterns':  [" TRACE ", " DEBUG " ], 'match': [] },
                                          "yes" :  { 'patterns':  [ " INFO ", " WARN ", " ERROR ", " FATAL " ], 'match': -1 },
                                          },
                              debug_level="INFO",
                              kick_device_managers=True,
                              dev_mgrs = [ 'test_BasicTestDevice_node', "test_BasicTestDevice2_node" ],
                              dev_mgr_levels= [ "INFO", "INFO" ]
                                )

    @scatest.requireLog4cxx
    def test_kick_warn(self):
        self._try_kick_domain('log4j.kickdomain.cfg',
                              epatterns={ "no" :  { 'patterns':  [" TRACE ", " DEBUG ", " INFO ",   ], 'match': [] },
                                          "yes" :  { 'patterns':  [" WARN ", " ERROR ", " FATAL " ], 'match': -1 },
                                          },
                              debug_level="WARN")

    @scatest.requireLog4cxx
    def test_kick_warn_both(self):
        self._try_kick_domain('log4j.kickdomain.cfg',
                              epatterns={ "no" :  { 'patterns':  [" TRACE ", " DEBUG "   ], 'match': [] },
                                          "no2" :  { 'patterns':  [ " INFO ",   ], 'match': ("<=", 12 ) },
                                          "yes" :  { 'patterns':  [ " WARN ", " ERROR ", " FATAL " ], 'match': -1 },
                                          },
                              debug_level="WARN",
                              kick_device_managers=True,
                              dev_mgrs = [ 'test_BasicTestDevice_node', "test_BasicTestDevice2_node" ],
                              dev_mgr_levels= [ "WARN", "WARN" ]
                                )

    @scatest.requireLog4cxx
    def test_kick_error(self):
        self._try_kick_domain('log4j.kickdomain.cfg',
                              epatterns={ "no" :  { 'patterns':  [" TRACE ", " DEBUG ", " INFO ", " WARN "  ], 'match': [] },
                                          "yes" :  { 'patterns':  [ " ERROR ", " FATAL " ], 'match': -1 },
                                          },
                              debug_level="ERROR")

    @scatest.requireLog4cxx
    def test_kick_error_both(self):
        self._try_kick_domain('log4j.kickdomain.cfg',
                              epatterns={ "no" :  { 'patterns':  [" TRACE ", " DEBUG " ] , 'match': [] },
                                          "no2" :  { 'patterns':  [ " INFO ", " WARN "  ], 'match': ('<=', 18) },
                                          "yes" :  { 'patterns':  [ " ERROR ", " FATAL " ], 'match': -1 },
                                          },
                              debug_level="ERROR",
                              kick_device_managers=True,
                              dev_mgrs = [ 'test_BasicTestDevice_node', "test_BasicTestDevice2_node" ],
                              dev_mgr_levels= [ "ERROR", "ERROR"  ]
                                )

    @scatest.requireLog4cxx
    def test_kick_fatal(self):
        self._try_kick_domain('log4j.kickdomain.cfg',
                              epatterns={ "no" :  { 'patterns':  [" TRACE ", " DEBUG ", " INFO ", " WARN ", " ERROR "  ], 'match': [] },
                                          "yes" :  { 'patterns':  [" FATAL " ], 'match': -1 },
                                          },
                              debug_level="ERROR")

    @scatest.requireLog4cxx
    def test_kick_fatal_both(self):
        self._try_kick_domain('log4j.kickdomain.cfg',
                              epatterns={ "no" :  { 'patterns':  [" TRACE ", " DEBUG ", " ERROR "  ], 'match': [] },
                                          "no2" :  { 'patterns':  [ " INFO ", " WARN "  ], 'match': ('<=', 18 ) },
                                          "yes" :  { 'patterns':  [ " FATAL " ], 'match': -1 },
                                          },
                              debug_level="FATAL",
                              kick_device_managers=True,
                              dev_mgrs = [ 'test_BasicTestDevice_node', "test_BasicTestDevice2_node" ],
                              dev_mgr_levels= [ "FATAL", "FATAL"  ]
                                )

    def test_kick_devmgr_path(self):
        """
        Test $SDRROOT/dev relative paths for DeviceManagers in kickDomain()
        """
        dom = redhawk.kickDomain(domain_name=scatest.getTestDomainName(),
                                 kick_device_managers=True,
                                 device_managers=['/nodes/test_GPP_node/DeviceManager.dcd.xml'])

        def check_devmgr():
            return len(dom.devMgrs) == 1

        self.assertPredicateWithWait(check_devmgr, 'test_GPP_node did not launch')
        self.assertEqual(dom.devMgrs[0].label, 'test_GPP_node')

    def test_kick_devmgr_name(self):
        """
        Test using node names for DeviceManagers in kickDomain()
        """
        dom = redhawk.kickDomain(domain_name=scatest.getTestDomainName(),
                                 kick_device_managers=True,
                                 device_managers=['test_GPP_node'])

        def check_devmgr():
            return len(dom.devMgrs) == 1

        self.assertPredicateWithWait(check_devmgr, 'test_GPP_node did not launch')
        self.assertEqual(dom.devMgrs[0].label, 'test_GPP_node')
