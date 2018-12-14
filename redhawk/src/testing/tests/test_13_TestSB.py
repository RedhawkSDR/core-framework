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
import Queue
import unittest
import sys
import commands
import cStringIO
import time
import copy
import threading
import warnings
import subprocess
import commands
import struct
import tempfile

from omniORB import CORBA, any, tcInternal

from ossie import properties
from ossie.cf import CF, StandardEvent
from ossie.utils import sb, type_helpers
from ossie.utils.bulkio import bulkio_helpers
from ossie.events import ChannelManager, Subscriber, Publisher
from ossie.utils.bulkio import bulkio_data_helpers

from _unitTestHelpers import scatest, runtestHelpers
import traceback

globalsdrRoot = os.environ['SDRROOT']
try:
    from bulkio.bulkioInterfaces import BULKIO
except:
    pass

def _initSourceAndSink(dataFormat):

    source = sb.DataSource(dataFormat = dataFormat)
    sink   = sb.DataSink()

    usesPortName = source.supportedPorts[dataFormat]["portDict"]["Port Name"]
    providesPortName = sink.supportedPorts[dataFormat]["portDict"]["Port Name"]

    source.connect(
        sink,
        usesPortName     = usesPortName,
        providesPortName = providesPortName)

    source.start()
    sink.start()

    return source, sink


def compareKeywordLists( a, b ):
    for keyA, keyB in zip(a, b):
        if keyA.id  != keyB.id:
            return False
        if keyA.value._t != keyB.value._t:
            if isinstance(keyA.value._t,tcInternal.TypeCode_sequence):
                if keyA.value._t.content_type() != keyB.value._t.content_type():
                    return False
            else:
                return False
        if keyA.value._v != keyB.value._v:
            return False
    return True

@scatest.requireJava
class InteractiveTestJava(scatest.CorbaTestCase):
    def setUp(self):
        self.message = "Interactive mode (-i) no longer supported. Please use the sandbox to run Components/Devices/Services outside the scope of a Domain"

    def tearDown(self):
        pass

    def test_NoInteractiveJavaService(self):
        status, output=commands.getstatusoutput('sdr/dev/services/BasicService_java/java/startJava.sh -i')
        self.assertNotEquals(output.find(self.message),-1)

    def test_NoInteractiveJavaDevice(self):
        status, output=commands.getstatusoutput('sdr/dev/devices/BasicTestDevice_java/java/startJava.sh -i')
        self.assertNotEquals(output.find(self.message),-1)

    def test_NoInteractiveJavaComponent(self):
        status, output=commands.getstatusoutput('sdr/dom/components/ECM_JAVA/java/startJava.sh -i')
        self.assertNotEquals(output.find(self.message),-1)

class InteractiveTestPython(scatest.CorbaTestCase):
    def setUp(self):
        self.message = "Interactive mode (-i) no longer supported. Please use the sandbox to run Components/Devices/Services outside the scope of a Domain"

    def tearDown(self):
        pass

    def test_NoInteractivePythonService(self):
        status, output=commands.getstatusoutput('sdr/dev/services/S1/python/S1.py -i')
        self.assertNotEquals(output.find(self.message),-1)

    def test_NoInteractivePythonDevice(self):
        status, output=commands.getstatusoutput('sdr/dev/devices/BasicTestDevice/BasicTestDevice.py -i')
        self.assertNotEquals(output.find(self.message),-1)

    def test_NoInteractivePythonComponent(self):
        status, output=commands.getstatusoutput('sdr/dom/components/ECM_PY/python/ECM_PY.py -i')
        self.assertNotEquals(output.find(self.message),-1)

class InteractiveTestCpp(scatest.CorbaTestCase):
    def setUp(self):
        self.message = "Interactive mode (-i) no longer supported. Please use the sandbox to run Components/Devices/Services outside the scope of a Domain"

    def tearDown(self):
        pass

    def test_NoInteractiveCppService(self):
        status, output=commands.getstatusoutput('sdr/dev/services/BasicService_cpp/cpp/BasicService_cpp -i')
        self.assertNotEquals(output.find(self.message),-1)

    def test_NoInteractiveCppDevice(self):
        status, output=commands.getstatusoutput('sdr/dev/devices/cpp_dev/cpp/cpp_dev -i')
        self.assertNotEquals(output.find(self.message),-1)

    def test_NoInteractiveCppComponent(self):
        status, output=commands.getstatusoutput('sdr/dom/components/ECM_CPP/cpp/ECM_CPP -i')
        self.assertNotEquals(output.find(self.message),-1)

def wait_on_data(sink, number_timestamps, timeout=1):
    begin_time = time.time()
    estimate = sink.getDataEstimate()
    while estimate.num_timestamps != number_timestamps:
        time.sleep(0.1)
        estimate = sink.getDataEstimate()
        if time.time() - begin_time > timeout:
            break

def wait_for_eos(sink, timeout=10):
    begin_time = time.time()
    while not sink.eos():
        time.sleep(0.1)
        if time.time() - begin_time > timeout:
            break

class SBEventChannelTest(scatest.CorbaTestCase):
    def setUp(self):
        orb = CORBA.ORB_init()
        self.chanMgr = ChannelManager(orb)
        # Force creation
        self.channel = self.chanMgr.createEventChannel("TestChan", force=True)
        sb.setDEBUG(False)

    def tearDown(self):
        try:
            if self.channel:
                self.chanMgr.destroyEventChannel("TestChan")                           
        except:
            pass
        sb.release()
        sb.setDEBUG(False)
        os.environ['SDRROOT'] = globalsdrRoot
        
    def _waitData(self, sub, timeout):
        end = time.time() + timeout
        while time.time() < end:
            data = sub.getData()
            if data:
                return data._v
        return None

    def test_PublishSubscribePull(self):
        sub = Subscriber( self.channel )
        pub = Publisher( self.channel )
        payload = 'hello'
        data = any.to_any(payload)
        pub.push(data)
        rec_data = self._waitData(sub, 1.0)
        self.assertEquals(rec_data, payload)
        pub.terminate()
        sub.terminate()
        
    def test_PublishSubscribeCB(self):
        queue = Queue.Queue()
        sub = Subscriber(self.channel, dataArrivedCB=queue.put)
        pub = Publisher(self.channel)
        payload = 'hello'
        data = any.to_any(payload)
        pub.push(data)
        rec_data = queue.get(timeout=1.0)
        self.assertEquals(rec_data._v, payload)
        pub.terminate()
        sub.terminate()

@scatest.requireLog4cxx
class SBStdOutTest(scatest.CorbaTestCase):
    def setUp(self):
        sb.setDEBUG(False)
        self.test_comp = "Sandbox"
        # Flagrant violation of sandbox API: if the sandbox singleton exists,
        # clean up previous state and dispose of it.
        if sb.domainless._sandbox:
            sb.domainless._sandbox.shutdown()
            sb.domainless._sandbox = None

    def tearDown(self):
        sb.release()
        sb.setDEBUG(False)
        os.environ['SDRROOT'] = globalsdrRoot
        try:
            os.remove(self.tmpfile)
        except:
            pass

    def test_debugCmdExec(self):
        self.tmpfile=tempfile.mktemp()
        fp_tmpfile=open(self.tmpfile, 'w')
        comp = sb.launch('sdr/dom/components/C2/C2.spd.xml', execparams={'DEBUG_LEVEL':5}, stdout=fp_tmpfile)
        sb.start()
        time.sleep(0.4)
        fp_tmpfile.close()
        new_stdout=open(self.tmpfile,'r')
        stdout_contents=new_stdout.read()
        self.assertTrue('serviceFunction() example log message - TRACE' in stdout_contents)
        self.assertTrue('TRACE C2_1.system.Resource' in stdout_contents)
        self.assertTrue('serviceFunction() example log message - DEBUG' in stdout_contents)
        new_stdout.close()

    def test_debugDevCmdExec(self):
        self.tmpfile=tempfile.mktemp()
        fp_tmpfile=open(self.tmpfile, 'w')
        comp = sb.launch('sdr/dev/devices/devcpp/devcpp.spd.xml', execparams={'DEBUG_LEVEL':5}, stdout=fp_tmpfile)
        sb.start()
        time.sleep(0.4)
        fp_tmpfile.close()
        new_stdout=open(self.tmpfile,'r')
        stdout_contents=new_stdout.read()
        self.assertTrue('serviceFunction() example TRACE log message' in stdout_contents)
        self.assertTrue('TRACE devcpp_1.system.Device' in stdout_contents)
        self.assertTrue('serviceFunction() example DEBUG log message' in stdout_contents)
        new_stdout.close()

    def test_debugCmdProp(self):
        self.tmpfile=tempfile.mktemp()
        fp_tmpfile=open(self.tmpfile, 'w')
        comp = sb.launch('sdr/dom/components/C2/C2.spd.xml', properties={'DEBUG_LEVEL':5}, stdout=fp_tmpfile)
        sb.start()
        time.sleep(0.4)
        fp_tmpfile.close()
        new_stdout=open(self.tmpfile,'r')
        stdout_contents=new_stdout.read()
        self.assertTrue('serviceFunction() example log message - TRACE' in stdout_contents)
        self.assertTrue('TRACE C2_1.system.Resource' in stdout_contents)
        self.assertTrue('serviceFunction() example log message - DEBUG' in stdout_contents)
        new_stdout.close()

    def test_debugCmdExecNoMsg(self):
        self.tmpfile=tempfile.mktemp()
        fp_tmpfile=open(self.tmpfile, 'w')
        comp = sb.launch('sdr/dom/components/C2/C2.spd.xml', execparams={'DEBUG_LEVEL':4}, stdout=fp_tmpfile)
        sb.start()
        time.sleep(0.4)
        fp_tmpfile.close()
        new_stdout=open(self.tmpfile,'r')
        stdout_contents=new_stdout.read()
        self.assertFalse('serviceFunction() example log message - TRACE' in stdout_contents)
        self.assertFalse('TRACE C2_1.system.Resource' in stdout_contents)
        self.assertTrue('serviceFunction() example log message - DEBUG' in stdout_contents)
        new_stdout.close()

    def test_debugCmdPropNoMsg(self):
        self.tmpfile=tempfile.mktemp()
        fp_tmpfile=open(self.tmpfile, 'w')
        comp = sb.launch('sdr/dom/components/C2/C2.spd.xml', properties={'DEBUG_LEVEL':4}, stdout=fp_tmpfile)
        sb.start()
        time.sleep(0.4)
        fp_tmpfile.close()
        new_stdout=open(self.tmpfile,'r')
        stdout_contents=new_stdout.read()
        self.assertFalse('serviceFunction() example log message - TRACE' in stdout_contents)
        self.assertFalse('TRACE C2_1.system.Resource' in stdout_contents)
        self.assertTrue('serviceFunction() example log message - DEBUG' in stdout_contents)
        new_stdout.close()

class SBTestTest(scatest.CorbaTestCase):
    def setUp(self):
        sb.setDEBUG(False)
        self.test_comp = "Sandbox"
        # Flagrant violation of sandbox API: if the sandbox singleton exists,
        # clean up previous state and dispose of it.
        if sb.domainless._sandbox:
            sb.domainless._sandbox.shutdown()
            sb.domainless._sandbox = None

    def assertComponentCount(self, count):
        self.assertEquals(len(sb.domainless._getSandbox().getComponents()), count)

    def tearDown(self):
        sb.release()
        sb.setDEBUG(False)
        os.environ['SDRROOT'] = globalsdrRoot

    def test_catalog(self):
        # Store orig sdrroot
        sdrRoot = os.environ.pop('SDRROOT')

        # No SDRROOT env (should default to current directory).
        self.assertEquals(sb.catalog(), [])

        # Bad Sdr root env; dispose of existing sandbox instance to test.
        sb.domainless._sandbox = None
        os.environ["SDRROOT"] = ""
        self.assertEquals(sb.catalog(), [])

        # Good SDRROOT
        sb.setSDRROOT(sdrRoot)
        self.assertNotEquals(len(sb.catalog()), 0)

        # test objType options
        self.assertNotEquals(len(sb.catalog(objType="components")), 0)
        self.assertNotEquals(len(sb.catalog(objType="devices")), 0)
        self.assertNotEquals(len(sb.catalog(objType="services")), 0)
        self.assertNotEquals(len(sb.catalog(objType="all")), 0)

        # Bad search path
        self.assertEquals(len(sb.catalog("my_path")), 0)

        # Search path with no usable files
        self.assertEquals(len(sb.catalog("jackhammer")), 0)

        # Restore sdrroot
        os.environ['SDRROOT'] = sdrRoot

    def test_softpkgDepSingle(self):
        c = sb.launch('ticket_490_single')
        self.assertNotEquals(c, None)

    def test_softpkgDepNone(self):
        c = sb.launch('ticket_490_none')
        self.assertNotEquals(c, None)

    def test_softpkgDepDouble(self):
        c = sb.launch('ticket_490_double')
        self.assertNotEquals(c, None)

    def test_pid(self):
        a = sb.launch('comp_src')
        status,output = commands.getstatusoutput('ps -ww -f | grep comp_src ')
        lines = output.split('\n')
        for line in lines:
          if 'IOR' in line:
            break
        _pid = line.split()[1]
        self.assertEquals(int(_pid), a._pid)

    def test_cleanHeap(self):
        a = sb.launch('alloc_shm')
        ch_pid = a._sandbox._getComponentHost()._pid
        self.assertTrue(os.path.isfile('/dev/shm/heap-'+str(ch_pid)))
        os.kill(ch_pid, 9)
        begin = time.time()
        while time.time()-begin < 1 and os.path.isfile('/dev/shm/heap-'+str(ch_pid)):
            time.sleep(0.1)
        self.assertFalse(os.path.isfile('/dev/shm/heap-'+str(ch_pid)))

    def test_doubleNamedConnection(self):
        a = sb.launch('comp_src')
        b = sb.launch('comp_snk')
        c = sb.launch('comp_src')
        d = sb.launch('comp_snk')
        a.connect(b,connectionId='some_id')
        c.connect(d,connectionId='some_id')
        import ossie.utils.model
        connection_mgr=ossie.utils.model.ConnectionManager.instance()
        connection_ids = connection_mgr.getConnections().keys()
        self.assertEquals(len(connection_ids), 2)

    def test_setSDRROOT(self):
        # None type
        self.assertRaises(TypeError, sb.setSDRROOT, None)

        # Bad dir should not change root
        sdrroot = sb.getSDRROOT()
        self.assertRaises(AssertionError, sb.setSDRROOT, 'TEMP_PATH')
        self.assertEquals(sdrroot, sb.getSDRROOT())

        # Good dir with no dev/dom should not change root
        self.assertRaises(AssertionError, sb.setSDRROOT, 'jackhammer')
        self.assertEquals(sdrroot, sb.getSDRROOT())

        # New root
        sb.setSDRROOT('sdr')
        self.assertEquals(sb.getSDRROOT(), 'sdr')

        # Restore sdrroot
        sb.setSDRROOT(os.environ['SDRROOT'])


    def test_componentInit(self):
        # Bad descriptors
        self.assertRaises(TypeError, sb.launch)
        self.assertRaises(ValueError, sb.launch, "test_name")

        # Make sure only one instance name and refid can be used
        comp = sb.launch(self.test_comp, "comp")
        comp.api(destfile=sys.stdout)
        refid = comp._refid
        self.assertRaises(ValueError, sb.launch, self.test_comp, "comp")
        self.assertRaises(ValueError, sb.launch, self.test_comp, "new_comp", refid)

        # Only 1 component should be running
        self.assertComponentCount(1)

    def test_relativePath(self):
        comp = sb.launch('sdr/dom/components/Sandbox/Sandbox.spd.xml')
        self.assertComponentCount(1)

    def test_LogServiceFunctionException(self):
        file_loc = os.getcwd()
        comp = sb.launch('sdr/dom/components/svc_fn_error/svc_fn_error.spd.xml', execparams={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/logconfig.cfg'})
        comp.start()
        time.sleep(0.5)
        fp = None
        try:
            fp = open('foo/bar/test.log','r')
        except:
            pass
        if fp != None:
            log_contents = fp.read()
            fp.close()
        
        try:
            os.remove('foo/bar/test.log')
        except:
            pass
        try:
            os.rmdir('foo/bar')
        except:
            pass
        try:
            os.rmdir('foo')
        except:
            pass
            
        self.assertTrue('test exception in process()' in log_contents)

    def test_propertyInitialization(self):
        """
        Tests for the correct initialization of 'property' kind properties
        based on whether command line is set, and overrides via launch().
        """
        # First, test with defaults
        comp = sb.launch('sdr/dom/components/property_init/property_init.spd.xml')
        self.assertFalse('initial' in comp.cmdline_args)
        self.assertFalse('cmdline' in comp.initialize_props)
        comp.releaseObject()

        # Test with overrides
        comp = sb.launch('sdr/dom/components/property_init/property_init.spd.xml',
                         properties={'cmdline':'override', 'initial':'override'})
        self.assertFalse('initial' in comp.cmdline_args)
        self.assertFalse('cmdline' in comp.initialize_props)
        self.assertEquals('override', comp.cmdline)
        self.assertEquals('override', comp.initial)
        comp.releaseObject()

        # Test with overrides in deprecated 'execparams' and 'configure' arguments
        with warnings.catch_warnings():
            warnings.simplefilter('ignore', DeprecationWarning)
            comp = sb.launch('sdr/dom/components/property_init/property_init.spd.xml',
                             execparams={'cmdline':'override'}, configure={'initial':'override'})
        self.assertFalse('initial' in comp.cmdline_args)
        self.assertFalse('cmdline' in comp.initialize_props)
        self.assertEquals('override', comp.cmdline)
        self.assertEquals('override', comp.initial)
        comp.releaseObject()

        # Test with misplaced command line property in deprecated 'configure' argument
        comp = sb.launch('sdr/dom/components/property_init/property_init.spd.xml',
                         configure={'cmdline':'override'})
        self.assertFalse('initial' in comp.cmdline_args)
        self.assertFalse('cmdline' in comp.initialize_props)
        self.assertEquals('override', comp.cmdline)
        comp.releaseObject()

    def test_writeOnly(self):
        dev = sb.launch('writeonly_cpp')
        try:
            print dev.foo
            self.assertTrue(False)
        except Exception, e:
            self.assertEquals(e.args[0], 'Could not perform query, "foo" is a writeonly property')
        try:
            print dev.foo_seq
            self.assertTrue(False)
        except Exception, e:
            self.assertEquals(e.args[0], 'Could not perform query, "foo_seq" is a writeonly property')
        try:
            print dev.foo_struct
            self.assertTrue(False)
        except Exception, e:
            self.assertEquals(e.args[0], 'Could not perform query, "foo_struct" is a writeonly property')
        try:
            print dev.foo_struct_seq
            self.assertTrue(False)
        except Exception, e:
            self.assertEquals(e.args[0], 'Could not perform query, "foo_struct_seq" is a writeonly property')

    def test_zeroLengthSeqStruct(self):
        """
        Tests for the correct initialization of 'property' kind properties
        based on whether command line is set, and overrides via launch().
        """
        comp = sb.launch('sdr/dom/components/zero_length/zero_length.spd.xml', impl='cpp')
        self.assertNotEqual(comp, None)
        prop = comp.query([CF.DataType('mystruct', any.to_any(None))])
        found = False
        for p in prop:
            if p.id == 'mystruct':
                val = p.value.value()
                for v in val:
                    if v.id == 'mystruct::mysimpleseq':
                        found = len(v.value.value()) == 0
        self.assertTrue(found)

        comp = sb.launch('sdr/dom/components/zero_length/zero_length.spd.xml', impl='python')
        self.assertNotEqual(comp, None)
        prop = comp.query([CF.DataType('mystruct', any.to_any(None))])
        found = False
        for p in prop:
            if p.id == 'mystruct':
                val = p.value.value()
                for v in val:
                    if v.id == 'mystruct::mysimpleseq':
                        found = len(v.value.value()) == 0
        self.assertTrue(found)

    def test_nestedSoftPkgDeps(self):
        cwd = os.getcwd()
        depLibraryPath = cwd + "/sdr/dom/components/softpkgNestedDep/spdNestedDepLibrary"
        if not depLibraryPath in sys.path:
            sys.path.append(depLibraryPath)
        if os.environ.has_key('PYTHONPATH'):
            os.environ['PYTHONPATH'] = "%s:%s" % (depLibraryPath, os.environ['PYTHONPATH'])
        else:
            os.environ['PYTHONPATH'] = "%s" % (depLibraryPath)

        comp= sb.launch('sdr/dom/components/CommandWrapperNestedSPDDep/CommandWrapperNestedSPDDep.spd.xml')
        self.assertComponentCount(1)

    def initValues(self, comp):
        # Init values
        self.assertEquals(comp.my_bool_true, True)
        self.assertEquals(comp.my_bool_false, False)
        self.assertEquals(comp.my_bool_empty, None)
        self.assertEquals(comp.my_long, 10)
        self.assertEquals(comp.my_long_empty, None)
        self.assertEquals(comp.my_str, "Hello World!")
        self.assertEquals(comp.my_str_empty, None)
        self.assertEquals(comp.my_struct.bool_true, True)
        self.assertEquals(comp.my_struct.bool_false, False)

        self.assertEquals(comp.my_struct.bool_empty, None)
        self.assertEquals(comp.my_struct.long_s, None)
        self.assertEquals(comp.my_struct.str_s, None)
        self.assertEquals(comp.my_seq_bool[0], True)
        self.assertEquals(comp.my_seq_bool[1], False)
        self.assertEquals(comp.my_seq_str[0], "one")
        self.assertEquals(comp.my_seq_str[1], "")
        self.assertEquals(comp.my_seq_str[2], "three")
        self.assertEquals(comp.my_long_enum, None)
        self.assertEquals(comp.my_bool_enum, None)
        self.assertEquals(comp.my_str_enum, None)
        self.assertEquals(comp.my_struct.enum_long, None)
        self.assertEquals(comp.my_struct.enum_bool, None)
        self.assertEquals(comp.my_struct.enum_str, None)


    def test_simpleComp(self):
        comp = sb.launch(self.test_comp)
        comp.api(destfile=sys.stdout)

        # Check the init values
        self.initValues(comp)
        
        # Change values
        comp.my_bool_true = False
        comp.my_bool_false = True
        comp.my_long = 22
        comp.my_str = "new"
        comp.my_struct.bool_true = False
        comp.my_struct.bool_false = True
        comp.my_struct.long_s = 33
        comp.my_seq_bool[0] = False
        comp.my_seq_bool[1] = True
        comp.my_seq_str[0] = "1"
        comp.my_seq_str[1] = "2"
        comp.my_seq_str[2] = "3"
        self.assertEquals(comp.my_bool_true, False)
        self.assertEquals(comp.my_bool_false, True)
        self.assertEquals(comp.my_long, 22)
        self.assertEquals(comp.my_str, "new")
        self.assertEquals(comp.my_struct.bool_true, False)
        self.assertEquals(comp.my_struct.bool_false, True)
        self.assertEquals(comp.my_struct.long_s, 33)
        self.assertEquals(comp.my_seq_bool[0], False)
        self.assertEquals(comp.my_seq_bool[1], True)
        self.assertEquals(comp.my_seq_str[0], "1")
        self.assertEquals(comp.my_seq_str[1], "2")
        self.assertEquals(comp.my_seq_str[2], "3")

        # Checks invalid simple enumeration changes
        comp.my_long_enum = 10
        comp.my_long_enum = "one"
        comp.my_long_enum = "3"
        comp.my_long_enum = 11.1
        comp.my_long_enum = False
        self.assertEquals(comp.my_long_enum, None)
        comp.my_str_enum = 10
        comp.my_str_enum = "one"
        comp.my_str_enum = 11.1
        comp.my_str_enum = False
        self.assertEquals(comp.my_str_enum, None)
        comp.my_bool_enum = 10
        comp.my_bool_enum = "one"
        comp.my_bool_enum = 11.1
        comp.my_bool_enum = "invalid"
        self.assertEquals(comp.my_bool_enum, None)

        # Checks invalid struct enumeration changes
        comp.my_struct.enum_long = 10
        comp.my_struct.enum_long = "one"
        comp.my_struct.enum_long = "3"
        comp.my_struct.enum_long = 11.1
        comp.my_struct.enum_long = False
        self.assertEquals(comp.my_struct.enum_long, None)
        comp.my_struct.enum_str = 10
        comp.my_struct.enum_str = "one"
        comp.my_struct.enum_str = 11.1
        comp.my_struct.enum_str = False
        self.assertEquals(comp.my_struct.enum_str, None)
        comp.my_struct.enum_bool = 10
        comp.my_struct.enum_bool = "one"
        comp.my_struct.enum_bool = 11.1
        comp.my_struct.enum_bool = "valid"
        self.assertEquals(comp.my_struct.enum_bool, None)

        # Checks changes to simple enumerations
        comp.my_long_enum = "ONE"
        self.assertEquals(comp.my_long_enum, 11)
        comp.my_long_enum = "Two"
        self.assertEquals(comp.my_long_enum, 11)
        comp.my_long_enum = 33
        self.assertEquals(comp.my_long_enum, 33)
        comp.my_long_enum = 34
        self.assertEquals(comp.my_long_enum, 33)

        comp.my_str_enum = "ONE"
        self.assertEquals(comp.my_str_enum, "FIRST")
        comp.my_str_enum = "two"
        self.assertEquals(comp.my_str_enum, "FIRST")
        comp.my_str_enum = "SECOND"
        self.assertEquals(comp.my_str_enum, "SECOND")
        comp.my_str_enum = "third"
        self.assertEquals(comp.my_str_enum, "SECOND")

        comp.my_bool_enum = "VALID"
        self.assertEquals(comp.my_bool_enum, True)
        comp.my_bool_enum = "false"
        self.assertEquals(comp.my_bool_enum, True)
        comp.my_bool_enum = False
        self.assertEquals(comp.my_bool_enum, False)
        comp.my_bool_enum = True
        self.assertEquals(comp.my_bool_enum, True)
        comp.my_bool_enum = "INVALID"
        self.assertEquals(comp.my_bool_enum, False)

        # Checks changes to struct enumerations
        comp.my_struct.enum_long = "ONE"
        self.assertEquals(comp.my_struct.enum_long, 1)
        comp.my_struct.enum_long = "Two"
        self.assertEquals(comp.my_struct.enum_long, 1)
        comp.my_struct.enum_long = 3
        self.assertEquals(comp.my_struct.enum_long, 3)
        comp.my_struct.enum_long = 4
        self.assertEquals(comp.my_struct.enum_long, 3)

        comp.my_struct.enum_str = "ONE"
        self.assertEquals(comp.my_struct.enum_str, "first")
        comp.my_struct.enum_str = "two"
        self.assertEquals(comp.my_struct.enum_str, "first")
        comp.my_struct.enum_str = "second"
        self.assertEquals(comp.my_struct.enum_str, "second")
        comp.my_struct.enum_str = "THIRD"
        self.assertEquals(comp.my_struct.enum_str, "second")

        comp.my_struct.enum_bool = "VALID"
        self.assertEquals(comp.my_struct.enum_bool, True)
        comp.my_struct.enum_bool = "false"
        self.assertEquals(comp.my_struct.enum_bool, True)
        comp.my_struct.enum_bool = False
        self.assertEquals(comp.my_struct.enum_bool, False)
        comp.my_struct.enum_bool = True
        self.assertEquals(comp.my_struct.enum_bool, True)
        comp.my_struct.enum_bool = "INVALID"
        self.assertEquals(comp.my_struct.enum_bool, False)
        
        #Components which have properties with the same id where one is
        #enumerated and one is not. checks the id's can not be confused and enum 
        #values can not given to the non enumerated property
        sandbox = sb.launch("Sandbox")
        testProp = sb.launch("TestComponentProperty")
        self.assertRaises(TypeError, testProp.my_long_enum.configureValue, "TWO")
        self.assertEquals(testProp.my_long_enum, 11)
        testProp.my_struct.long_s = 1
        self.assertRaises(TypeError, testProp.my_struct.enum_long.configureValue, "THREE")
        self.assertEquals(testProp.my_struct.long_s,1)
        sandbox.my_struct.enum_str = "TWO"
        self.assertEquals(sandbox.my_struct.enum_str,"second")
        sandbox.my_struct.enum_str = "fourth"
        self.assertEquals(sandbox.my_struct.enum_str,"second")

        #Test _properties query works for getting a properties
        #kinds, action, type, value, defualt value, typecode and id
        self.assertEquals(testProp.simpleExecparam.kinds,['execparam'])
        self.assertEquals(testProp.simpleExecparam.action,"eq")
        self.assertEquals(testProp.seqTest.kinds,["configure"])
        self.assertEquals(testProp.structSeqTest[0].simpleShort,3)
        self.assertEquals(testProp.structSeqTest[1].simpleFloat.defValue, 2)
        self.assertEquals(testProp.simpleExecparam.id,"simpleExecparam")
        self.assertEquals(testProp.structSeqTest.type,"structSeq")
        self.assertEquals(testProp.simpleExecparam.typecode,CORBA.TC_float)



        # Make sure reset works properly
        sb.reset()
        self.initValues(comp)

    def test_illegalPropertyNames(self):
        comp = sb.launch(self.test_comp)
        comp.api(destfile=sys.stdout)

        self.initValues(comp)

        # Makes sure that no getters cause exceptions
        comp.escape__simple
        comp.escape__struct.es__1
        comp.escape__struct.es__2
        comp.escape__struct.normal
        comp.my_struct.es__3
        comp.my_struct_seq
        comp.escape__structseq
        comp.my_struct_seq[0].simp__bad
        comp.my_struct_seq[1].simp_bool
        comp.escape__structseq[0].val__1
        comp.escape__structseq[1].val_2

        # Makes sure that no setters cause exceptions
        comp.escape__simple = 1234
        comp.escape__struct.es__1 = 5678
        comp.escape__struct.es__2 = 4321
        comp.escape__struct.normal = 8765
        comp.my_struct.es__3 = 333
        comp.my_struct_seq[0].simp_bool = True
        comp.my_struct_seq[1].simp__bad = 11
        comp.escape__structseq[0].val_2 = "test"
        comp.escape__structseq[1].val__1 = 22

        # Makes sure that values got set properly
        self.assertEquals(comp.escape__simple, 1234)
        self.assertEquals(comp.escape__struct.es__1, 5678)
        self.assertEquals(comp.escape__struct.es__2, 4321)
        self.assertEquals(comp.escape__struct.normal, 8765)
        self.assertEquals(comp.my_struct.es__3, 333)
        self.assertEquals(comp.my_struct_seq[0].simp_bool, True)
        self.assertEquals(comp.my_struct_seq[1].simp__bad, 11)
        self.assertEquals(comp.escape__structseq[0].val_2, "test")
        self.assertEquals(comp.escape__structseq[1].val__1, 22)

    def test_loadSADFile(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/ticket_462_w/ticket_462_w.sad.xml')
        self.assertEquals(retval, True)
        comp_ac = sb.getComponent('ticket_462_ac_1')
        self.assertNotEquals(comp_ac, None)
        comp = sb.getComponent('ticket_462_1')
        comp_id = comp._get_identifier()
        self.assertEquals(len(comp_id.split(':')), 2)
        self.assertEquals(comp_id.split(':')[0], 'ticket_462_1')
        self.assertEquals(comp_id.split(':')[1], 'ticket_462_w')
        self.assertNotEquals(comp, None)
        self.assertEquals(comp_ac.my_simple, "foo")
        self.assertEquals(comp_ac.my_seq, ["initial value"])
        self.assertEquals(comp_ac.basic_struct.some_simple, '4')
        self.assertEquals(comp.over_simple, "override")
        self.assertEquals(comp.over_struct_seq, [{'a_word': 'something', 'a_number': 1}])

    def test_connectPortSADFile(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/PortConnectProvidesPort/PortConnectProvidesPort.sad.xml')
        sad=sb.generateSADXML('hello')

        uses_string = '\n      <usesport>\n        <usesidentifier>@__PORTNAME__@</usesidentifier>\n        <componentinstantiationref refid="@__COMPONENTINSTANCE__@"/>\n      </usesport>\n'
        uses_string = uses_string.replace('@__PORTNAME__@', 'resource_out')
        uses_string = uses_string.replace('@__COMPONENTINSTANCE__@', 'DCE:5faf296f-3193-49cc-8751-f8a64b315fdf')

        provides_string = '\n      <providesport>\n        <providesidentifier>@__PORTNAME__@</providesidentifier>\n        <componentinstantiationref refid="@__COMPONENTINSTANCE__@"/>\n      </providesport>\n'
        provides_string = provides_string.replace('@__PORTNAME__@', 'resource_in')
        provides_string = provides_string.replace('@__COMPONENTINSTANCE__@', 'DCE:12ab27fb-01bd-4189-8d1d-0043b87c4f74')

        self.assertNotEqual(sad.find(uses_string), -1)
        self.assertNotEqual(sad.find(provides_string), -1)
        self.assertEquals(sad.find('DCE:DCE'), -1)

    def test_connectSupportedInterfaceSADFile(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/PortConnectComponentSupportedInterface/PortConnectComponentSupportedInterface.sad.xml')
        sad=sb.generateSADXML('hello')

        uses_string = '\n      <usesport>\n        <usesidentifier>@__PORTNAME__@</usesidentifier>\n        <componentinstantiationref refid="@__COMPONENTINSTANCE__@"/>\n      </usesport>\n'
        uses_string = uses_string.replace('@__PORTNAME__@', 'resource_out')
        uses_string = uses_string.replace('@__COMPONENTINSTANCE__@', 'DCE:5faf296f-3193-49cc-8751-f8a64b315fdf')

        provides_string = '\n      <componentsupportedinterface>\n        <supportedidentifier>@__PORTINTERFACE__@</supportedidentifier>\n        <componentinstantiationref refid="@__COMPONENTINSTANCE__@"/>\n      </componentsupportedinterface>\n'
        provides_string = provides_string.replace('@__PORTINTERFACE__@', 'IDL:CF/Resource:1.0')
        provides_string = provides_string.replace('@__COMPONENTINSTANCE__@', 'DCE:12ab27fb-01bd-4189-8d1d-0043b87c4f74')

        self.assertNotEqual(sad.find(uses_string), -1)
        self.assertNotEqual(sad.find(provides_string), -1)
        self.assertEquals(sad.find('DCE:DCE'), -1)

    def test_connectSandbox(self):
        src=sb.launch('PortTest')
        snk=sb.launch('PortTest')
        src.connect(snk, usesPortName='resource_out')
        sad=sb.generateSADXML('hello')

        uses_string = '\n      <usesport>\n        <usesidentifier>@__PORTNAME__@</usesidentifier>\n        <componentinstantiationref refid="@__COMPONENTINSTANCE__@"/>\n      </usesport>\n'
        uses_string = uses_string.replace('@__PORTNAME__@', 'resource_out')
        uses_string = uses_string.replace('@__COMPONENTINSTANCE__@', src._id)

        provides_string = '\n      <componentsupportedinterface>\n        <supportedidentifier>@__PORTINTERFACE__@</supportedidentifier>\n        <componentinstantiationref refid="@__COMPONENTINSTANCE__@"/>\n      </componentsupportedinterface>\n'
        provides_string = provides_string.replace('@__PORTINTERFACE__@', 'IDL:CF/Resource:1.0')
        provides_string = provides_string.replace('@__COMPONENTINSTANCE__@', snk._id)

        non_colon_connectionid = '<connectinterface id="DCE_'

        self.assertNotEqual(sad.find(uses_string), -1)
        self.assertNotEqual(sad.find(provides_string), -1)
        self.assertNotEqual(sad.find(non_colon_connectionid), -1)
        self.assertEquals(sad.find('DCE:DCE'), -1)

    def test_loadSADFile_startorder(self):
        maxpid=32768
        try:
            out=subprocess.Popen(['cat', '/proc/sys/kernel/pid_max'], stdout=subprocess.PIPE)
            res=out.communicate()
            maxpid=int(res[0].strip())
        except:
            pass
        retval = sb.loadSADFile('sdr/dom/waveforms/ticket_462_w/ticket_462_w.sad.xml')
        self.assertEquals(retval, True)
        comp_ac = sb.getComponent('ticket_462_ac_1')
        self.assertNotEquals(comp_ac, None)
        comp = sb.getComponent('ticket_462_1')
        self.assertNotEquals(comp, None)
        if comp_ac._pid <= maxpid-1:
            isless= comp_ac._pid < comp._pid
        else:
            isless=comp._pid < comp_ac._pid 
        self.assertTrue(isless)


    def test_SDDS_SRI(self):
        c = sb.launch('sdds_src')
        self.assertNotEquals(c, None)
        snk = sb.DataSinkSDDS()
        c.connect(snk)
        sb.start()
        count = 0
        sri = None
        while count < 4:
            sri = snk.sri()
            if sri != None:
                break
            count += 1
            time.sleep(0.5)
        self.assertNotEquals(sri, None)


    def test_SDDS_Incompatable_Port(self):
        from ossie.utils.model import NoMatchingPorts
        c = sb.launch('sdds_src')
        self.assertNotEquals(c, None)
        snk = sb.DataSink()
        self.assertRaises(NoMatchingPorts, c.connect,snk)


    def test_loadSADFileSpecialChar(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/comp_prop_special_char_w/comp_prop_special_char_w.sad.xml')
        self.assertEquals(retval, True)

    def test_loadSADFileUses_p(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/SADUsesDeviceWave/SADUsesDeviceWaveConnectionDevProvides.sad.xml')
        self.assertEquals(retval, True)

    def test_loadSADFileUses_u(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/SADUsesDeviceWave/SADUsesDeviceWaveConnectionDevUses.sad.xml')
        self.assertEquals(retval, True)

    def test_loadSADFilePartialOverrideStruct(self):
        # This tests partially overriding a struct or struct seq property in sad
        retval = sb.loadSADFile('sdr/dom/waveforms/more_ticket_462/more_ticket_462.sad.xml')
        comp = sb.getComponent('another_ticket_462_1')
        self.assertNotEquals(comp, None)
        self.assertEquals(comp.simple_prop, "This is a string")
        self.assertEquals(comp.seq_prop, [9,8,7])
        self.assertEquals(comp.struct_prop.prop_two, 'string override')
        self.assertEquals(comp.struct_seq_prop[0].prop_five, 1)
        self.assertEquals(comp.struct_seq_prop[0].prop_six, 123.0)
        self.assertEquals(comp.struct_seq_prop[1].prop_four, "anotherString")
        self.assertEquals(comp.struct_seq_prop[1].prop_six, 345.0)
        self.assertEquals(comp.struct_seq_prop[2].prop_four, "string_override")
        self.assertEquals(comp.struct_seq_prop[2].prop_five, 3)
        self.assertEquals(comp.struct_seq_prop[2].prop_six, 678.0)
        self.assertEquals(comp.struct_seq_prop[3].prop_six, 987.0)

    def test_loadSADFileACPropertyWithDefaultValueOverride(self):
        # Tests overriding a property in the sad file for an assembly controller property that has a default value
        retval = sb.loadSADFile('sdr/dom/waveforms/ticket_cf_1066_wf/ticket_cf_1066_wf.sad.xml')
        comp = sb.getComponent('ticket_cf_1066_comp_1')
        self.assertNotEquals(comp, None)
        self.assertEquals(comp.simple_prop, "This is a string")
        self.assertEquals(comp.seq_prop, [9,8,7])
        self.assertEquals(comp.struct_prop.prop_two, 'string override')
        self.assertEquals(comp.struct_seq_prop[0].prop_five, 1)
        self.assertEquals(comp.struct_seq_prop[0].prop_six, 123.0)
        self.assertEquals(comp.struct_seq_prop[1].prop_four, "anotherString")
        self.assertEquals(comp.struct_seq_prop[1].prop_six, 345.0)
        self.assertEquals(comp.struct_seq_prop[2].prop_four, "string_override")
        self.assertEquals(comp.struct_seq_prop[2].prop_five, 3)
        self.assertEquals(comp.struct_seq_prop[2].prop_six, 678.0)
        self.assertEquals(comp.struct_seq_prop[3].prop_six, 987.0)

    def test_loadSADFileOverrideWithExecparam(self):
        # Tests overriding a property in the sad file for a component that also has an execparam property
        retval = sb.loadSADFile('sdr/dom/waveforms/ticket_cf_1067_wf/ticket_cf_1067_wf.sad.xml')
        self.assertEquals(retval, True)
        comp = sb.getComponent('ticket_cf_1067_comp_1')
        self.assertNotEquals(comp, None)
        self.assertEquals(comp.simple_exec_param, "default_value")

    def test_loadSADFilePassingInOverrideOfExecparamWithOverrideInSadFile(self):
        # Tests passing in an execparam property value for a property that is overridden in the sad file
        retval = sb.loadSADFile('sdr/dom/waveforms/override_execparam_wf/override_execparam_wf.sad.xml',props={'simple_exec_param':'another_override_value'})
        self.assertEquals(retval, True)
        comp = sb.getComponent('ticket_cf_1067_comp_1')
        self.assertNotEquals(comp, None)
        self.assertEquals(comp.simple_exec_param, "another_override_value")

    def test_loadSADFileMorePassingInOverrideOfExecparamWithDefaultValue(self):
        # Tests overriding an execparam property for an assembly controller by passing value in, and the execparam is not overridden in the sad file 
        retval = sb.loadSADFile('sdr/dom/waveforms/ticket_cf_1067_wf/ticket_cf_1067_wf.sad.xml',props={'simple_exec_param':'another_override_value'})
        self.assertEquals(retval, True)
        comp = sb.getComponent('ticket_cf_1067_comp_1')
        self.assertNotEquals(comp, None)
        self.assertEquals(comp.simple_exec_param, "another_override_value")

    def test_loadSADFileNoOverriddenProperties(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/ticket_841_and_854/ticket_841_and_854.sad.xml')
        self.assertEquals(retval, True)
        comp_ac = sb.getComponent('Sandbox_1')
        self.assertNotEquals(comp_ac, None)
        self.assertEquals(comp_ac.my_long, 10)

    def test_loadSADFile_overload_create(self):
        self.assertRaises(Warning, sb.loadSADFile, 'sdr/dom/waveforms/ticket_462_w/ticket_462_w.sad.xml', props={'my_simple':'not foo','over_simple':'not override'})
        #retval = sb.loadSADFile('sdr/dom/waveforms/ticket_462_w/ticket_462_w.sad.xml', props=[{'my_simple':'not foo'},{'over_simple':'not override'}])
        comp_ac = sb.getComponent('ticket_462_ac_1')
        self.assertNotEquals(comp_ac, None)
        comp = sb.getComponent('ticket_462_1')
        self.assertNotEquals(comp, None)
        self.assertEquals(comp_ac.my_simple, "not foo")
        self.assertEquals(comp_ac.my_seq, ["initial value"])
        self.assertEquals(comp_ac.basic_struct.some_simple, '4')
        self.assertEquals(comp.over_simple, "override")
        self.assertEquals(comp.over_struct_seq, [{'a_word': 'something', 'a_number': 1}])

    def test_loadSADFile_overload_external_props(self):
        retval = sb.loadSADFile('sdr/dom/waveforms/cf_1535/cf_1535.sad.xml', props={'freq_3': '2222', 's1_3': [ 1,2,3 ], 'st1_3' : { 'st1::b1' : 2 , 'st1::a1' : 'setting st1::a1' } , 'ss1_3' : [ { 'b1' : 3, 'a1' : 'struct1 a1' }, { 'b1' : 4 , 'ss1::st1::a1' : 'struct2 a1' }]} )
        self.assertEquals(retval, True)
        comp = sb.getComponent('P1_3')
        self.assertNotEquals(comp, None)
        self.assertAlmostEquals(comp.freq, 2222.0)
        self.assertEquals(comp.s1, [1,2,3])
        self.assertEquals(comp.st1.a1, 'setting st1::a1' )
        self.assertEquals(comp.st1.b1,  2.0  )
        self.assertEquals(comp.ss1, [ { 'ss1::st1::a1': 'struct1 a1', 'ss1::st1::b1': 3.0  }, { 'ss1::st1::a1': 'struct2 a1', 'ss1::st1::b1': 4.0  } ] )


    def test_simplePropertyRange(self):
        comp = sb.launch('TestPythonPropsRange')
        comp.api(destfile=sys.stdout)

        # Test upper range
        comp.my_octet_name = 255
        comp.my_short_name = 32767
        comp.my_ushort_name = 65535
        comp.my_long_name = 2147483647
        comp.my_ulong_name = 4294967295
        comp.my_longlong_name = 9223372036854775807
        comp.my_ulonglong_name = 18446744073709551615
        self.assertEquals(comp.my_octet_name, 255)
        self.assertEquals(comp.my_short_name, 32767)
        self.assertEquals(comp.my_ushort_name, 65535)
        self.assertEquals(comp.my_long_name, 2147483647)
        self.assertEquals(comp.my_ulong_name, 4294967295)
        self.assertEquals(comp.my_longlong_name, 9223372036854775807)
        self.assertEquals(comp.my_ulonglong_name, 18446744073709551615)

        # Test lower range
        comp.my_octet_name = 0
        comp.my_short_name = -32768
        comp.my_ushort_name = 0
        comp.my_long_name = -2147483648
        comp.my_ulong_name = 0
        comp.my_longlong_name = -9223372036854775808
        comp.my_ulonglong_name = 0
        self.assertEquals(comp.my_octet_name, 0)
        self.assertEquals(comp.my_short_name, -32768)
        self.assertEquals(comp.my_ushort_name, 0)
        self.assertEquals(comp.my_long_name, -2147483648)
        self.assertEquals(comp.my_ulong_name, 0)
        self.assertEquals(comp.my_longlong_name, -9223372036854775808)
        self.assertEquals(comp.my_ulonglong_name, 0)

        # Test one beyond upper bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_octet_name.configureValue, 256)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_short_name.configureValue, 32768)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ushort_name.configureValue, 65536)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_long_name.configureValue, 2147483648)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ulong_name.configureValue, 4294967296)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_longlong_name.configureValue, 9223372036854775808)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ulonglong_name.configureValue, 18446744073709551616)

        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_octet_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_short_name.configureValue, -32769)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ushort_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_long_name.configureValue, -2147483649)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ulong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_longlong_name.configureValue, -9223372036854775809)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_ulonglong_name.configureValue, -1)


    def test_structPropertyRange(self):
        comp = sb.launch('TestPythonPropsRange')
        comp.api(destfile=sys.stdout)

        # Test upper range
        comp.my_struct_name.struct_octet_name = 255
        comp.my_struct_name.struct_short_name = 32767
        comp.my_struct_name.struct_ushort_name = 65535
        comp.my_struct_name.struct_long_name = 2147483647
        comp.my_struct_name.struct_ulong_name = 4294967295
        comp.my_struct_name.struct_longlong_name = 9223372036854775807
        comp.my_struct_name.struct_ulonglong_name = 18446744073709551615
        comp.my_struct_name.struct_seq_octet_name[1] = 255
        comp.my_struct_name.struct_seq_short_name[1] = 32767
        comp.my_struct_name.struct_seq_ushort_name[1] = 65535
        comp.my_struct_name.struct_seq_long_name[1] = 2147483647
        comp.my_struct_name.struct_seq_ulong_name[1] = 4294967295
        comp.my_struct_name.struct_seq_longlong_name[1] = 9223372036854775807
        #comp.my_struct_name.struct_seq_ulonglong_name[1] = 18446744073709551615
        self.assertEquals(comp.my_struct_name.struct_octet_name, 255)
        self.assertEquals(comp.my_struct_name.struct_short_name, 32767)
        self.assertEquals(comp.my_struct_name.struct_ushort_name, 65535)
        self.assertEquals(comp.my_struct_name.struct_long_name, 2147483647)
        self.assertEquals(comp.my_struct_name.struct_ulong_name, 4294967295)
        self.assertEquals(comp.my_struct_name.struct_longlong_name, 9223372036854775807)
        self.assertEquals(comp.my_struct_name.struct_ulonglong_name, 18446744073709551615)
        self.assertEquals(comp.my_struct_name.struct_seq_octet_name[1], 255)
        self.assertEquals(comp.my_struct_name.struct_seq_short_name[1], 32767)
        self.assertEquals(comp.my_struct_name.struct_seq_ushort_name[1], 65535)
        self.assertEquals(comp.my_struct_name.struct_seq_long_name[1], 2147483647)
        self.assertEquals(comp.my_struct_name.struct_seq_ulong_name[1], 4294967295)
        self.assertEquals(comp.my_struct_name.struct_seq_longlong_name[1], 9223372036854775807)
        #self.assertEquals(comp.my_struct_name.struct_seq_ulonglong_name[1], 18446744073709551615)

        # Test lower range
        comp.my_struct_name.struct_octet_name = 0
        comp.my_struct_name.struct_short_name = -32768
        comp.my_struct_name.struct_ushort_name = 0
        comp.my_struct_name.struct_long_name = -2147483648
        comp.my_struct_name.struct_ulong_name = 0
        comp.my_struct_name.struct_longlong_name = -9223372036854775808
        comp.my_struct_name.struct_ulonglong_name = 0
        comp.my_struct_name.struct_seq_octet_name[0] = 0
        comp.my_struct_name.struct_seq_short_name[0] = -32768
        comp.my_struct_name.struct_seq_ushort_name[0] = 0
        comp.my_struct_name.struct_seq_long_name[0] = -2147483648
        comp.my_struct_name.struct_seq_ulong_name[0] = 0
        comp.my_struct_name.struct_seq_longlong_name[0] = -9223372036854775808
        comp.my_struct_name.struct_seq_ulonglong_name[0] = 0
        self.assertEquals(comp.my_struct_name.struct_octet_name, 0)
        self.assertEquals(comp.my_struct_name.struct_short_name, -32768)
        self.assertEquals(comp.my_struct_name.struct_ushort_name, 0)
        self.assertEquals(comp.my_struct_name.struct_long_name, -2147483648)
        self.assertEquals(comp.my_struct_name.struct_ulong_name, 0)
        self.assertEquals(comp.my_struct_name.struct_longlong_name, -9223372036854775808)
        self.assertEquals(comp.my_struct_name.struct_ulonglong_name, 0)
        self.assertEquals(comp.my_struct_name.struct_seq_octet_name[0], 0)
        self.assertEquals(comp.my_struct_name.struct_seq_short_name[0], -32768)
        self.assertEquals(comp.my_struct_name.struct_seq_ushort_name[0], 0)
        self.assertEquals(comp.my_struct_name.struct_seq_long_name[0], -2147483648)
        self.assertEquals(comp.my_struct_name.struct_seq_ulong_name[0], 0)
        self.assertEquals(comp.my_struct_name.struct_seq_longlong_name[0], -9223372036854775808)
        self.assertEquals(comp.my_struct_name.struct_seq_ulonglong_name[0], 0)

        # Test one beyond upper bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_octet_name.configureValue, 256)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_short_name.configureValue, 32768)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ushort_name.configureValue, 65536)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_long_name.configureValue, 2147483648)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ulong_name.configureValue, 4294967296)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_longlong_name.configureValue, 9223372036854775808)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ulonglong_name.configureValue, 18446744073709551616)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_seq_octet_name.configureValue, [0, 256])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_seq_short_name.configureValue, [0, 32768])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_seq_ushort_name.configureValue, [0, 65536])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_seq_long_name.configureValue, [0, 2147483648])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_seq_ulong_name.configureValue, [0, 4294967296])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_seq_longlong_name.configureValue, [0, 9223372036854775808])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_seq_ulonglong_name.configureValue, [0, 18446744073709551616])

        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_octet_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_short_name.configureValue, -32769)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ushort_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_long_name.configureValue, -2147483649)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ulong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_longlong_name.configureValue, -9223372036854775809)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_ulonglong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_seq_octet_name.configureValue, [-1, 0])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_seq_short_name.configureValue, [-32769, 0])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_seq_ushort_name.configureValue, [-1, 0])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_seq_long_name.configureValue, [-2147483649, 0])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_seq_ulong_name.configureValue, [-1, 0])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_seq_longlong_name.configureValue, [-9223372036854775809, 0])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_struct_name.struct_seq_ulonglong_name.configureValue, [-1, 0])

        # Makes sure the struct can be set without error
        # NB: This test used to use names instead of ids, which silently failed in 1.8.
        new_value = {'struct_octet': 100, 'struct_short': 101, 'struct_ushort': 102, 'struct_long': 103,
                     'struct_ulong': 104, 'struct_longlong': 105, 'struct_ulonglong': 106, 'struct_seq_octet': [100, 101],
                     'struct_seq_short': [102, 103], 'struct_seq_ushort': [104, 105], 'struct_seq_long': [106L, 107L],
                     'struct_seq_ulong': [108L, 109L], 'struct_seq_longlong': [110L, 111L], 'struct_seq_ulonglong': [112L, 113L]}
        comp.my_struct_name = new_value
        self.assertEquals(comp.my_struct_name, new_value)

    def test_seqPropertyRange(self):
        comp = sb.launch('TestPythonPropsRange')
        comp.api(destfile=sys.stdout)

        # Test upper and lower bounds
        comp.seq_octet_name[0] = 0
        comp.seq_octet_name[1] = 255
        comp.seq_short_name[0] = -32768
        comp.seq_short_name[1] = 32767
        comp.seq_ushort_name[0] = 0
        comp.seq_ushort_name[1] = 65535
        comp.seq_long_name[0] = -2147483648
        comp.seq_long_name[1] = 2147483647
        comp.seq_ulong_name[0] = 0
        comp.seq_ulong_name[1] = 4294967295
        comp.seq_longlong_name[0] = -9223372036854775808
        comp.seq_longlong_name[1] = 9223372036854775807
        comp.seq_ulonglong_name[0] = 0
        #comp.seq_ulonglong_name[1] = 18446744073709551615
        self.assertEquals(comp.seq_octet_name[0], 0)
        self.assertEquals(comp.seq_octet_name[1], 255)
        self.assertEquals(comp.seq_short_name[0], -32768)
        self.assertEquals(comp.seq_short_name[1], 32767)
        self.assertEquals(comp.seq_ushort_name[0], 0)
        self.assertEquals(comp.seq_ushort_name[1], 65535)
        self.assertEquals(comp.seq_long_name[0], -2147483648)
        self.assertEquals(comp.seq_long_name[1], 2147483647)
        self.assertEquals(comp.seq_ulong_name[0], 0)
        self.assertEquals(comp.seq_ulong_name[1], 4294967295)
        self.assertEquals(comp.seq_longlong_name[0], -9223372036854775808)
        self.assertEquals(comp.seq_longlong_name[1], 9223372036854775807)
        self.assertEquals(comp.seq_ulonglong_name[0], 0)
        #self.assertEquals(comp.seq_ulonglong_name[1], 18446744073709551615)

        # Test one beyond upper bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_octet.configureValue, [0, 256])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_short.configureValue, [0, 32768])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ushort.configureValue, [0, 65536])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_long.configureValue, [0, 2147483648])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ulong.configureValue, [0, 4294967296])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_longlong.configureValue, [0, 9223372036854775808])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ulonglong.configureValue, [0, 18446744073709551616])

        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_octet.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_short.configureValue, [-32769, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ushort.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_long.configureValue, [-2147483649, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ulong.configureValue, [-1, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_longlong.configureValue, [-9223372036854775809, 0])
        self.assertRaises(type_helpers.OutOfRangeException, comp.seq_ulonglong.configureValue, [-1, 0])

        # Tests char and octet sequences
        self.assertRaises(TypeError, comp.seq_char_name.configureValue, ['A','BB'])
        self.assertRaises(TypeError, comp.seq_char_name.configureValue, ['a', 1])
        self.assertRaises(TypeError, comp.seq_octet_name.configureValue, [1, 'a'])

        comp.seq_char_name[0] = 'X'
        comp.seq_char_name[1] = 'Y'
        self.assertEquals(comp.seq_char_name[0], 'X')
        self.assertEquals(comp.seq_char_name[1], 'Y')


    def test_structSeqPropertyRange(self):
        comp = sb.launch('TestPythonPropsRange')
        comp.api(destfile=sys.stdout)

        # Test upper and lower bounds
        comp.my_structseq_name[0].ss_octet_name = 255
        comp.my_structseq_name[1].ss_octet_name = 0
        comp.my_structseq_name[0].ss_short_name = 32767
        comp.my_structseq_name[1].ss_short_name = -32768
        comp.my_structseq_name[0].ss_ushort_name = 65535
        comp.my_structseq_name[1].ss_ushort_name = 0
        comp.my_structseq_name[0].ss_long_name = 2147483647
        comp.my_structseq_name[1].ss_long_name = -2147483648
        comp.my_structseq_name[0].ss_ulong_name = 4294967295
        comp.my_structseq_name[1].ss_ulong_name = 0
        comp.my_structseq_name[0].ss_longlong_name = 9223372036854775807
        comp.my_structseq_name[1].ss_longlong_name = -9223372036854775808
        #comp.my_structseq_name[0].ss_ulonglong_name = 18446744073709551615
        comp.my_structseq_name[1].ss_ulonglong_name = 0
        comp.my_structseq_name[0].ss_seq_octet_name[1] = 255
        comp.my_structseq_name[1].ss_seq_octet_name[0] = 0
        comp.my_structseq_name[0].ss_seq_short_name[1] = 32767
        comp.my_structseq_name[1].ss_seq_short_name[0] = -32768
        comp.my_structseq_name[0].ss_seq_ushort_name[1] = 65535
        comp.my_structseq_name[1].ss_seq_ushort_name[0] = 0
        comp.my_structseq_name[0].ss_seq_long_name[1] = 2147483647
        comp.my_structseq_name[1].ss_seq_long_name[0] = -2147483648
        comp.my_structseq_name[0].ss_seq_ulong_name[1] = 4294967295
        comp.my_structseq_name[1].ss_seq_ulong_name[0] = 0
        comp.my_structseq_name[0].ss_seq_longlong_name[1] = 9223372036854775807
        comp.my_structseq_name[1].ss_seq_longlong_name[0] = -9223372036854775808
        #comp.my_structseq_name[0].ss_seq_ulonglong_name[1] = 18446744073709551615
        comp.my_structseq_name[1].ss_seq_ulonglong_name[0] = 0
        self.assertEquals(comp.my_structseq_name[0].ss_octet_name, 255)
        self.assertEquals(comp.my_structseq_name[1].ss_octet_name, 0)
        self.assertEquals(comp.my_structseq_name[0].ss_short_name, 32767)
        self.assertEquals(comp.my_structseq_name[1].ss_short_name, -32768)
        self.assertEquals(comp.my_structseq_name[0].ss_ushort_name, 65535)
        self.assertEquals(comp.my_structseq_name[1].ss_ushort_name, 0)
        self.assertEquals(comp.my_structseq_name[0].ss_long_name, 2147483647)
        self.assertEquals(comp.my_structseq_name[1].ss_long_name, -2147483648)
        self.assertEquals(comp.my_structseq_name[0].ss_ulong_name, 4294967295)
        self.assertEquals(comp.my_structseq_name[1].ss_ulong_name, 0)
        self.assertEquals(comp.my_structseq_name[0].ss_longlong_name, 9223372036854775807)
        self.assertEquals(comp.my_structseq_name[1].ss_longlong_name, -9223372036854775808)
        #self.assertEquals(comp.my_structseq_name[0].ss_ulonglong_name, 18446744073709551615)
        self.assertEquals(comp.my_structseq_name[1].ss_ulonglong_name, 0)
        self.assertEquals(comp.my_structseq_name[0].ss_seq_octet_name[1], 255)
        self.assertEquals(comp.my_structseq_name[1].ss_seq_octet_name[0], 0)
        self.assertEquals(comp.my_structseq_name[0].ss_seq_short_name[1], 32767)
        self.assertEquals(comp.my_structseq_name[1].ss_seq_short_name[0], -32768)
        self.assertEquals(comp.my_structseq_name[0].ss_seq_ushort_name[1], 65535)
        self.assertEquals(comp.my_structseq_name[1].ss_seq_ushort_name[0], 0)
        self.assertEquals(comp.my_structseq_name[0].ss_seq_long_name[1], 2147483647)
        self.assertEquals(comp.my_structseq_name[1].ss_seq_long_name[0], -2147483648)
        self.assertEquals(comp.my_structseq_name[0].ss_seq_ulong_name[1], 4294967295)
        self.assertEquals(comp.my_structseq_name[1].ss_seq_ulong_name[0], 0)
        self.assertEquals(comp.my_structseq_name[0].ss_seq_longlong_name[1], 9223372036854775807)
        self.assertEquals(comp.my_structseq_name[1].ss_seq_longlong_name[0], -9223372036854775808)
        #self.assertEquals(comp.my_structseq_name[0].ss_seq_ulonglong_name[1], 18446744073709551615)
        self.assertEquals(comp.my_structseq_name[1].ss_seq_ulonglong_name[0], 0)

        # Test one beyond upper bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_octet_name.configureValue, 256)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_short_name.configureValue, 32768)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_ushort_name.configureValue, 65536)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_long_name.configureValue, 2147483648)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_ulong_name.configureValue, 4294967296)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_longlong_name.configureValue, 9223372036854775808)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_ulonglong_name.configureValue, 18446744073709551616)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_seq_octet_name.configureValue, [0, 256])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_seq_short_name.configureValue, [0, 32768])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_seq_ushort_name.configureValue, [0, 65536])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_seq_long_name.configureValue, [0, 2147483648])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_seq_ulong_name.configureValue, [0, 4294967296])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_seq_longlong_name.configureValue, [0, 9223372036854775808])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[0].ss_seq_ulonglong_name.configureValue, [0, 18446744073709551616])

        # Test one beyond lower bound
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_octet_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_short_name.configureValue, -32769)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_ushort_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_long_name.configureValue, -2147483649)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_ulong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_longlong_name.configureValue, -9223372036854775809)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_ulonglong_name.configureValue, -1)
        self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_seq_octet_name.configureValue, [-1, 0])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_seq_short_name.configureValue, [-32769, 0])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_seq_ushort_name.configureValue, [-1, 0])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_seq_long_name.configureValue, [-2147483649, 0])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_seq_ulong_name.configureValue, [-1, 0])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_seq_longlong_name.configureValue, [-9223372036854775809, 0])
	self.assertRaises(type_helpers.OutOfRangeException, comp.my_structseq_name[1].ss_seq_ulonglong_name.configureValue, [-1, 0])

        # Make sure entire struct seq can be set without error
        new_value = [{'ss_octet': 100, 'ss_short': 101, 'ss_ushort': 102, 'ss_long': 103,
                      'ss_ulong': 104, 'ss_longlong': 105, 'ss_ulonglong': 106, 'ss_seq_octet': [100, 101],
                      'ss_seq_short': [102, 103], 'ss_seq_ushort': [104, 105], 'ss_seq_long': [106L, 107L],
                      'ss_seq_ulong': [108L, 109L], 'ss_seq_longlong': [110L, 111L], 'ss_seq_ulonglong': [112L, 113L]},
                     {'ss_octet': 107, 'ss_short': 108, 'ss_ushort': 109, 'ss_long': 110,
                      'ss_ulong': 111, 'ss_longlong': 112, 'ss_ulonglong': 113, 'ss_seq_octet': [114, 115],
                      'ss_seq_short': [116, 117], 'ss_seq_ushort': [118, 119], 'ss_seq_long': [120L, 121L],
                      'ss_seq_ulong': [122L, 123L], 'ss_seq_longlong': [124L, 125L], 'ss_seq_ulonglong': [126L, 127L]}]
        comp.my_structseq_name = new_value
        self.assertEqual(comp.my_structseq_name, new_value)

        # Make sure individual structs can be set without error
        # NB: This test used to use names instead of ids, which silently failed in 1.8.
        for item in new_value:
            for name in item.iterkeys():
                if isinstance(item[name], list):
		    for i in item[name]:
			i += 100
                else:
                    item[name] = item[name] + 100
        comp.my_structseq_name[0] = new_value[0]
        comp.my_structseq_name[1] = new_value[1]
        self.assertEqual(comp.my_structseq_name, new_value)

    def test_readOnlyProps(self):
        comp = sb.launch('Sandbox')
        comp.api(destfile=sys.stdout)

        # Properties should be able to be read, but not set, and all should throw the saem exception
        exception = None
        comp.readonly_simp
        try:
            comp.readonly_simp = 'bad'
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')

        # Makes sure both ways to set struct members throws same error
        comp.readonly_struct.readonly_struct_simp
        try:
            comp.readonly_struct.readonly_struct_simp = 'bad'
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')
        try:
            comp.readonly_struct = {'readonly_struct_simp':'bad'}
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')

        # Makes sure both ways to set seqs throws same error
        comp.readonly_seq[0]
        comp.readonly_seq[1]
        try:
            comp.readonly_seq[1] = 'bad'
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')
        try:
            comp.readonly_seq = ['bad1', 'bad2']
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')

        # Make sure all ways to set structseq throws the same error
        comp.readonly_structseq[0].readonly_s
        comp.readonly_structseq[1].readonly_s
        try:
            comp.readonly_structseq[0].readonly_s = 'bad'
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')
        try:
            comp.readonly_structseq[1] = {'readonly_s':'bad'}
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')
        try:
            comp.readonly_structseq = [{'readonly_s':'bad1'}, {'readonly_s':'bad2'}]
        except Exception, e:
            self.assertEquals(e.__class__, Exception)
        else:
            self.fail('Expected exception to be thrown for read only property')

        # Nothing should have changed
        self.assertEquals(comp.readonly_simp, 'Read only simple prop')
        self.assertEquals(comp.readonly_struct.readonly_struct_simp, 'read only struct property')
        self.assertEquals(comp.readonly_seq[0], 'read only')
        self.assertEquals(comp.readonly_seq[1], 'sequence property')
        self.assertEquals(comp.readonly_structseq[0].readonly_s, 'read only')
        self.assertEquals(comp.readonly_structseq[1].readonly_s, 'struct seq property')

    def test_DuplicateNames(self):
        """
        Tests that duplicate property names that belong to different scopes
        (e.g., fields in different structs) can be accessed by the base name,
        rather than the "uniquified" version.
        """
        comp = sb.launch('struct_fields')

        comp.first.first = -1
        comp.first.second = 'TEST'
        comp.second.first = 1e6
        comp.second.second = True

        for prop in comp.query([]):
            if prop.id == 'first':
                value = comp.first.fromAny(prop.value)
                self.assertEqual(value['first::first'], -1)
                self.assertEqual(value['first::second'], 'TEST')
            elif prop.id == 'second':
                value = comp.second.fromAny(prop.value)
                self.assertEqual(value['second::first'], 1e6)
                self.assertEqual(value['second::second'], True)

    def test_StructSetFromDict(self):
        """
        Tests that setting struct properties from dictionaries supports both
        names and IDs for the keys.
        """
        comp = sb.launch('struct_fields')

        # The first property gets names, the second gets IDs
        props = {'first':{'first':-1, 'second':'TEST'},
                 'second':{'second::first':1e6, 'second::second':True}}
        comp.configure(props)

        for prop in comp.query([]):
            if prop.id == 'first':
                value = comp.first.fromAny(prop.value)
                self.assertEqual(value['first::first'], props['first']['first'])
                self.assertEqual(value['first::second'], props['first']['second'])
            elif prop.id == 'second':
                value = comp.second.fromAny(prop.value)
                first = 'second::first'
                self.assertEqual(value[first], props['second'][first])
                second = 'second::second'
                self.assertEqual(value[second], props['second'][second])

    def test_DefaultPropertyKinds(self):
        """
        Try to set a property of each type (simple, struct, etc.) to ensure
        that if the PRF does not specify a kind, it defaults to configure.
        """
        comp = sb.launch('default_kinds')

        comp.long_prop = 1
        comp.floatseq_prop = [1,2,3]
        comp.struct_prop.string_field = 'TEST'
        comp.endpoints = [{}]

    @scatest.requireJava
    def test_Services(self):
        service = sb.launch(sb.getSDRROOT() + '/dev/services/BasicService_java/BasicService_java.spd.xml')
        comp = sb.launch('ServiceComponent')
        comp.connect(service)
        self.assertEquals(len(sb.domainless.ConnectionManager.instance().getConnections().keys()), 1)

        # Check that all the parameters got set correctly
        props = service.query([])
        d = dict([(p.id, any.from_any(p.value)) for p in props])
        self.assertEqual(d["SERVICE_NAME"], "BasicService_java_1")
        self.assertEqual(d["PARAM1"], "ABCD")
        self.assertEqual(d["PARAM2"], 42)
        self.assertAlmostEqual(d["PARAM3"], 3.1459)
        self.assertEqual(d["PARAM4"], False)
        self.assertEqual(d["PARAM5"], "Hello World")
        self.assertEqual(d.has_key("PARAM6"), False)


    def test_IDMChannel(self):
        device_spd = sb.getSDRROOT() + '/dev/devices/BasicTestDevice/BasicTestDevice.spd.xml'
        device = sb.launch(device_spd)

        channel = sb.getEventChannel('IDM_Channel')

        # Push all events onto a queue
        event_queue = Queue.Queue()
        channel.eventReceived.addListener(event_queue.put)

        # Lock and unlock the device; this should create an event
        device._set_adminState(CF.Device.LOCKED)

        # Wait up to a second, just in case
        try:
            event = event_queue.get(True, 1.0)
        except Queue.Empty:
            self.fail('Device admin state change message was never received')

        # Unpack the event and check that it matches our expectations
        event = event.value()
        self.assertEqual(event.producerId, device._refid)
        self.assertEqual(event.sourceId, device._refid)
        self.assertEqual(event.stateChangeCategory, StandardEvent.ADMINISTRATIVE_STATE_EVENT)
        self.assertEqual(event.stateChangeFrom, StandardEvent.UNLOCKED)
        self.assertEqual(event.stateChangeTo, StandardEvent.LOCKED)

    def test_EventChannels(self):
        channel_name = 'properties'

        # Check that getEventChannel raises an exception when the channel
        # doesn't exist (and it shouldn't yet)
        self.assertRaises(NameError, sb.getEventChannel, channel_name)

        # Create the event channel and ensure that a duplicate create call
        # raises an exception if exclusive is True
        channel = sb.createEventChannel(channel_name)
        self.assertRaises(NameError, sb.createEventChannel, channel_name, True)

        # Make sure getEventChannel works now
        self.assertEqual(channel, sb.getEventChannel(channel_name))

        # Push all events onto a queue
        event_queue = Queue.Queue()
        channel.eventReceived.addListener(event_queue.put)

        # Connect a component that emits events to the channel
        comp = sb.launch('PropertyChangeEvents')
        comp.connect(channel)
        self.assertEqual(channel.supplier_count, 1)

        # Trigger a property event and wait up to a second for it to be
        # received
        comp.myprop = 2
        try:
            event = event_queue.get(True, 1.0)
        except Queue.Empty:
            self.fail('Property change event not received')

        # Check that the event matches our expectations
        event = event.value()
        self.assertEqual(event.sourceId, comp._refid)
        self.assertEqual(event.sourceName, comp._instanceName)
        self.assertEqual(len(event.properties), 1)
        prop = event.properties[0]
        self.assertEqual(prop.id, 'myprop')
        self.assertEqual(any.from_any(prop.value), 2)

        # Disconnect the event channel
        comp.disconnect(channel)
        self.assertEqual(channel.supplier_count, 0)

        # Clean up the channel
        channel.destroy()

        # Should no longer be able to use the channel--even a simple attribute
        # reference throws an exception
        self.assertRaises(ReferenceError, getattr, channel, 'destroy')

        # Check that the channel is really gone from the sandbox
        self.assertRaises(NameError, sb.getEventChannel, channel_name)

    def test_LaunchTimeout(self):
        """
        Test that the launch timeout can be adjusted to accomodate components
        or devices that take longer than 10 seconds to register with the
        sandbox.
        """
        try:
            comp = sb.launch('SlowComponent', properties={'CREATE_DELAY': 15}, timeout=20)
        except RuntimeError:
            self.fail('Launch timeout was not honored')

    def test_ComplexSequenceFromRealSequence(self):
        """
        Test that assigning a sequence of real values to a complex sequence
        property works correctly.
        """
        # NB: The default value for 'complexCharProp' raises a non-fatal
        #     exception during launch. Overriding it avoids the error message,
        #     though in general, complex char properties should be avoided.
        comp = sb.launch('TestComplexProps', properties={'complexCharProp':('a','b')})
        value = range(4)
        try:
            comp.complexFloatSequence = value
        except:
            self.fail('Could not assign real sequence to complex sequence property')
        self.assertEqual(value, comp.complexFloatSequence)

    def _callback(self, _id, _data):
        self.message = _id, _data

    def test_MessageFormat(self):
        src = sb.MessageSource(messageId='foo',messageFormat={'val':'short','seq':'[float]'})
        snk = sb.MessageSink(messageCallback = self._callback)
        sb.start()
        src.connect(snk)
        self.assertRaises(Exception, src.sendMessage, {'val':5,'seq':'[1,2]'})
        self.message = None
        begin = time.time()
        src.sendMessage({'val':5,'seq':[1,2]})
        while not self.message and time.time()-begin < 1:
            time.sleep(0.1)
        self.assertEquals(self.message[1].id, 'foo')
        msg = None
        for _msg in self.message[1].value._v:
            if _msg.id == 'seq':
                msg = _msg.value
        self.assertEquals(msg._t.content_type(), CORBA.TC_float)
        msg = None
        for _msg in self.message[1].value._v:
            if _msg.id == 'val':
                msg = _msg.value
        self.assertEquals(msg._t, CORBA.TC_short)

    def test_DeviceAllocation(self):
        """
        Tests device allocation/deallocation using both dictionaries and lists
        of CF.DataTypes.
        """
        spd = os.path.join(sb.getSDRROOT(), 'dev/devices/CppTestDevice/CppTestDevice.spd.xml')
        dev = sb.launch(spd)

        # Save the initial state for checking that allocation is working
        load_average = dev.load_average.queryValue()
        shared_memory = dev.shared_memory.queryValue()

        # Allocate via dictionary
        dict_props = {'load_average': 1.25,
                      'memory_allocation': {'contiguous': False,
                                            'capacity': 1024,
                                            'memory_type': 'SHARED'} }
        self.assertTrue(dev.allocateCapacity(dict_props))
        self.assertEqual(dev.load_average, load_average+dict_props['load_average'])
        self.assertEqual(dev.shared_memory, shared_memory-dict_props['memory_allocation']['capacity'])

        dev.deallocateCapacity(dict_props)
        self.assertEqual(dev.load_average, load_average)
        self.assertEqual(dev.shared_memory, shared_memory)

        # Allocate with a list of CF.DataTypes
        cf_props = [self._propertyToDataType(dev, name, value) for name, value in dict_props.iteritems()]
        self.assertTrue(dev.allocateCapacity(cf_props))

        self.assertEqual(dev.load_average, load_average+dict_props['load_average'])
        self.assertEqual(dev.shared_memory, shared_memory-dict_props['memory_allocation']['capacity'])

        dev.deallocateCapacity(cf_props)
        self.assertEqual(dev.load_average, load_average)
        self.assertEqual(dev.shared_memory, shared_memory)

    def _propertyToDataType(self, comp, name, value):
        prop = getattr(comp, name)
        return CF.DataType(prop.id, prop.toAny(value))

    def test_Services(self):
        """
        Tests for the ability to launch and interact with services from the
        sandbox.
        """
        # Launch first instance by path to SPD
        spd = os.path.join(sb.getSDRROOT(), 'dev/services/BasicService/BasicService.spd.xml')
        service1 = sb.launch(spd)
        service1_name = service1._instanceName
        service1_id = service1._refid

        # Launch second instance by ID (added in 1.10)
        service2 = sb.launch('BasicService', properties={'PARAM4':True,'PARAM5':'Message'})
        service2_name = service2._instanceName
        service2_id = service2._refid

        # Check that the generated service names and IDs are unique
        self.assertNotEqual(service1_name, service2_name)
        self.assertNotEqual(service1_id, service2_id)

        # Try the CORBA API (BasicService is a property set)
        props = service1.query([])

        # Check that the execparams actually got overridden
        props = dict((str(prop.id), prop.value._v) for prop in service2.query([]))
        self.assertEqual(props['PARAM4'], True)
        self.assertEqual(props['PARAM5'], 'Message')

        # Make sure you can retrieve them by name
        serviceA = sb.getService(service1_name)
        serviceB = sb.getService(service2_name)

        # Make sure we got the same ones back
        self.assertEqual(service1, serviceA)
        self.assertEqual(service2, serviceB)

    def test_ComplexListConversions(self):
        # Test interleaved-to-complex
        inData = range(4)
        outData = bulkio_helpers.bulkioComplexToPythonComplexList(inData)
        self.assertEqual(outData,[complex(0,1),complex(2,3)])

        # Test complex-to-interleaved
        cxData = [complex(x+0.5,0) for x in xrange(4)]
        outData = bulkio_helpers.pythonComplexListToBulkioComplex(cxData)
        self.assertEqual(outData, [0.5,0.0,1.5,0.0,2.5,0.0,3.5,0.0])

        # Ensure that conversion does not modify the original list
        self.assertTrue(isinstance(cxData[0],complex))

        # Test inline type conversion (should truncate)
        outDataInt = bulkio_helpers.pythonComplexListToBulkioComplex(cxData, int)
        self.assertEqual(outDataInt[0], 0)
        self.assertEqual(outDataInt, [int(x) for x in outData])
    
    def test_apiBeforeLaunch(self):
        try:
            sb.api("TestCppProps", destfile=sys.stdout)
            sb.api("SimpleDevice", destfile=sys.stdout)
            # Building Java support is not necessary to test sb.api()
            sb.api("BasicService_java", destfile=sys.stdout)
        except:
            self.fail("sb.api(<objectName>) failure")

    def test_SISuffix(self):
        test=sb.launch("Sandbox")
        test.my_long=100
        self.assertEqual(test.my_long,100)
        test.my_long="1K"
        self.assertEqual(test.my_long,1000)
        test.my_long="2KB"
        self.assertEqual(test.my_long,2048)

    def test_MessageSource(self):
        source = sb.MessageSource(messageId='test_message')
        comp = sb.launch('MessageReceiverPy')
        source.connect(comp)
        sb.start()

        # As a pre-condition, there should have been no messages yet
        self.assertEqual(comp.received_messages, [])

        source.sendMessage({'item_float':0.0, 'item_string':'first'})

        # Wait until the component has time to process the message
        timeout = time.time() + 1.0
        while len(comp.received_messages) == 0 and time.time() < timeout:
            time.sleep(0.1)

        self.assertEqual(len(comp.received_messages), 1)
        self.assertEqual(comp.received_messages[0], "test_message,0.0,'first'")

    def test_BasicSharedComponent(self):
        """
        Test that two shared library components launched from the sandbox have
        the same process ID.
        """
        comp1 = sb.launch('BasicShared')
        comp2 = sb.launch('BasicShared')
        self.assertEqual(int(comp1.pid), int(comp2.pid))

    def test_NotSharedComponent(self):
        """
        Test that forcing a shared library component to run in a non-shared
        context reports a different process ID.
        """
        comp1 = sb.launch('BasicShared')
        comp2 = sb.launch('BasicShared', shared=False)
        self.assertNotEqual(int(comp1.pid), int(comp2.pid))


class Test_DataSDDSSource(unittest.TestCase):

    def test_CreateSDDSSource(self):
        source = sb.DataSourceSDDS()
        self.assertNotEquals(source, None)



class BulkioTest(unittest.TestCase):
    XMLDATA = """<body>
  <element tag=value/>
</body>"""

    TEMPFILE = 'testout.xml'

    def setUp(self):
        try:
            import bulkio
        except ImportError:
            raise ImportError('BULKIO is required for this test')

    def tearDown(self):
        # Clean up sources and sinks
        sb.release()
        try:
            os.unlink(self.TEMPFILE)
        except:
            pass

    def readFile(self, filename, strip=False):
        infile = open(filename, 'r')
        try:
            data = infile.read()
            if strip:
                data = data.strip()
            return data
        finally:
            infile.close()

    def _pushSRIThroughSourceAndSink(
        self,
        EOS          = True,
        dataFormat   = "float",
        streamID     = "testStreamID",
        sampleRate   = 2.0,
        delay        = 0.1,
        SRIKeywords  = []):
        # Note that the default parameters should be different
        # than the defaults in the DataSource/Sink to verify
        # that we are not just getting DataSource/Sink defaults.

        source, sink = _initSourceAndSink(dataFormat)

        data = range(10)
        source.push(
            data,
            EOS         = EOS,
            streamID    = streamID,
            sampleRate  = sampleRate,
            SRIKeywords = SRIKeywords)

        # give the data sink time to buffer the pushed data
        time.sleep(delay)

        returnedSRI = sink.sri()
        self.assertEquals(sink.eos(), EOS)
        self.assertEquals(returnedSRI.streamID, streamID)
        self.assertEquals(returnedSRI.xdelta, 1./sampleRate)
        self.assertEquals(returnedSRI.keywords, SRIKeywords)

    def _pushDataThroughSourceAndSink(
        self,
        data,
        dataFormat  = "float",
        delay       = 0.1,
        complexData = False):
        """
        Push data from DataSource to DataSink and verify that the data out
        of the DataSink is identical to the data being sent into the
        DataSource.
        """
        source, sink = _initSourceAndSink(dataFormat)

        originalData = copy.deepcopy(data)
        source.push(
            data,
            complexData = complexData)

        # give the data sink time to buffer the pushed data
        time.sleep(delay)
        receivedData = sink.getData()

        # Detect if we were passed a data list that had python complex types
        # in it
        def isComplex(x) : return type(x) == type(complex())
        if len(filter(isComplex, originalData)):
            # in this case, the DataSink will return bulkio complex data
            # convert the originalData to the bulkio data format.
            originalData = bulkio_helpers.pythonComplexListToBulkioComplex(originalData)

            # make sure the mode flag was automatically set
            self.assertEquals(sink.sri().mode, True)

        self.assertEquals(receivedData, originalData)

    def test_DataSourceWithFormatConnect(self):
        src = sb.DataSource(dataFormat='short')
        sink = sb.DataSink()
        try:
            src.connect(sink)
        except Exception, e:
            self.fail('Automatic connect failed for source that had dataFormat passed in')
      
    def test_DataSourceAndSink(self):
        dummySource = sb.DataSource()
        supportedPorts = dummySource.supportedPorts

        self._pushSRIThroughSourceAndSink()

        scalarDataList  = range(8)
        complexDataList = [0, 1, 2+2j]

        for format in supportedPorts.keys():
            if format == "file":
                # TODO: add test for file
                continue
            elif format == "xml":
                # TODO: add test for xml
                continue
            elif format == "char":
                # TODO: add test for char
                continue
            elif format == "octet":
                # TODO: add test for octet
                continue
            elif format == "sdds":
                # TODO: add test for sdds
                continue
            else:
                # test scalar data
                dataCopy = copy.deepcopy(scalarDataList)
                self._pushDataThroughSourceAndSink(
                    data         = dataCopy,
                    dataFormat   = format)

                # test complex data (both methods)
                dataCopy = copy.deepcopy(scalarDataList)
                self._pushDataThroughSourceAndSink(
                    data         = dataCopy,
                    dataFormat   = format,
                    complexData  = True)
                dataCopy = copy.deepcopy(complexDataList)
                self._pushDataThroughSourceAndSink(
                    data         = dataCopy,
                    dataFormat   = format)

    def test_DataSinkBadTimeStamp(self):
        datasink = sb.DataSink()
        port=datasink.getPort('shortIn')
        sb.start()

        t_good=BULKIO.PrecisionUTCTime(1,BULKIO.TCS_VALID,0,0,0)
        t_bad=BULKIO.PrecisionUTCTime(1,BULKIO.TCS_INVALID,0,0,0)

        # Retrieve the data
        # push good timestamp and data
        port.pushPacket([1,2,3,4],t_good,False,'hello')
        # push bad timestamp and data (should get the bad timestamp)
        port.pushPacket([1,2,3],t_bad,False,'hello')
        # push bad timestamp and no data (should not get the bad timestamp)
        port.pushPacket([],t_bad,False,'hello')
        # push bad timestamp, no data, and EOS (should not get the bad timestamp)
        port.pushPacket([],t_bad,True,'hello')
        data, tstamps = datasink.getData(eos_block=True, tstamps=True)
        self.assertEquals(len(tstamps), 2)

    def test_XMLDataSource(self):
        source = sb.DataSource(dataFormat='xml')
        datasink = sb.DataSink()
        source.connect(datasink)
        sb.start()

        source.push(self.XMLDATA, EOS=True)

        # Retrieve the data
        data = datasink.getData(eos_block=True)
        self.assertEqual(len(data), 1)
        data = data[0]
        self.assertEqual(self.XMLDATA, data)

    def test_XMLDataSink(self):
        """
        Test DataSink with XML data.
        """
        sink = sb.DataSink()
        sb.start()

        # Push directly into the port to test without a source
        port = sink.getPort('xmlIn')
        port.pushPacket(self.XMLDATA, True, 'xml')

        # Retrieve the data
        data = sink.getData(eos_block=True)
        self.assertEqual(len(data), 1)
        data = data[0]
        self.assertEqual(self.XMLDATA, data)

    def test_XMLFileSink(self):
        sink = sb.FileSink(self.TEMPFILE)
        sb.start()

        # Push directly into the port to test without a source
        port = sink.getPort('xmlIn')
        port.pushPacket(self.XMLDATA, True, 'xml')

        sink.waitForEOS()

        # Compare the output against the original data
        data = self.readFile(self.TEMPFILE, strip=True)
        self.assertEqual(self.XMLDATA, data)

    def test_XMLFileSource(self):
        infile = os.path.join(sb.getSDRROOT(), 'dom/mgr/DomainManager.spd.xml')

        source = sb.FileSource(infile, dataFormat='xml')
        sink = sb.FileSink(self.TEMPFILE)
        source.connect(sink)
        sb.start()

        sink.waitForEOS()
        
        # Check that the input and output files match
        xmldata = self.readFile(infile)
        data = self.readFile(self.TEMPFILE)
        self.assertEqual(xmldata, data)

    def test_DataSourceEOS(self):
        """
        Verify that DataSource sends EOS properly for pushes that exceed
        bytesPerPush.
        """
        source = sb.DataSource(dataFormat='float', bytesPerPush=1024)
        sink = sb.DataSink()
        source.connect(sink)
        sb.start()
        # Use an integer multiple of bytesPerPush (in this case 4X) to check
        # that exact boundaries don't break EOS
        source.push([float(x) for x in xrange(1024)], EOS=True)

        # Wait up to 2 seconds for EOS to be received
        start = time.time()
        while not sink.eos() and (time.time() - start) < 2.0:
            time.sleep(0.1)
        self.assertTrue(sink.eos())

        #TODO if BULKIO ever gets folded into core framework these tests can be used
        # to add them proper components must be created
        # 1 with multiple good connections
        # 1 with no good connections
        # 1 with a good connection

    def test_DataSourceChunkedTimeStamp(self):
        """
        Verify that DataSource sends timestamp properly for pushes that exceed
        bytesPerPush.
        """
        _timeout = 1
        _startTime = 10
        _sampleRate = 100
        _xdelta = 1/_sampleRate
        source = sb.DataSource(startTime=_startTime)
        sink = sb.DataSink()
        source.connect(sink, usesPortName='floatOut')
        sb.start()
        
        # test default sample rate
        _srcData = range(500000)
        source.push(_srcData, sampleRate=_sampleRate)
        estimate = sink.getDataEstimate()
        begin_time = time.time()
        while estimate.num_timestamps != 4:
            time.sleep(0.1)
            estimate = sink.getDataEstimate()
            if time.time() - begin_time > _timeout:
                break
        (_data, _tstamps) = sink.getData(tstamps=True)
        xdelta = sink.sri().xdelta
        print _tstamps
        self.assertEquals(_tstamps[0][1].twsec, _startTime)
        self.assertEquals(_tstamps[1][1].twsec, _tstamps[1][0]*sink.sri().xdelta+_startTime)
        self.assertEquals(_tstamps[2][1].twsec, _tstamps[2][0]*sink.sri().xdelta+_startTime)
        self.assertEquals(_tstamps[3][1].twsec, _tstamps[3][0]*sink.sri().xdelta+_startTime)

    def test_DataSourceSampleRateInt(self):
        """
        Verify that DataSource handles integer values for sampleRate when
        updating the timestamp for pushes that exceed bytesPerPush
        """
        source = sb.DataSource(startTime=0.0, bytesPerPush=4096)
        sink = sb.DataSink()
        source.connect(sink, usesPortName='floatOut')
        sb.start()

        # Use an integral sample rate larger than the packet size to make sure
        # that floating point math is used to calculate the time stamps
        sampleRate = 48000
        source.push(range(9600), sampleRate=sampleRate, EOS=True)
        data, tstamps = sink.getData(eos_block=True, tstamps=True)
        xdelta = sink.sri().xdelta
        self.assertAlmostEquals(xdelta, 1.0/sampleRate)

        # Check all of the received time stamps, only using the precision of a
        # float (instead of a PrecisionUTCTime) because that's all DataSource
        # uses
        for offset, ts in tstamps:
            actual = ts.twsec + ts.tfsec
            expected = offset * xdelta
            self.assertAlmostEquals(actual, expected)

    def test_DataSourceTimeStamp(self):
        """
        Verify that timestamps are correct across multiple pushes.
        """
        _timeout = 1
        _startTime = 10
        source = sb.DataSource(startTime=_startTime)
        sink = sb.DataSink()
        source.connect(sink, usesPortName='floatOut')
        sb.start()

        # test default sample rate
        _srcData = [1,2,3,4]
        source.push(_srcData)
        source.push(_srcData)
        estimate = sink.getDataEstimate()
        begin_time = time.time()
        while estimate.num_timestamps != 2:
            time.sleep(0.1)
            estimate = sink.getDataEstimate()
            if time.time() - begin_time > _timeout:
                break
        (_data, _tstamps) = sink.getData(tstamps=True)
        self.assertEquals(len(_data), len(_srcData)*2)
        self.assertEquals(sink.sri().xdelta, 1)
        self.assertEquals(_tstamps[0][1].twsec, _startTime)
        self.assertEquals(_tstamps[1][1].twsec, _startTime+len(_srcData))
        
        toffset = _startTime+len(_data)
        
        # test modified sample rate
        _sampleRate = 10.0
        source.push(_srcData,sampleRate=_sampleRate)
        source.push(_srcData)
        begin_time = time.time()
        estimate = sink.getDataEstimate()
        while estimate.num_timestamps != 2:
            time.sleep(0.1)
            estimate = sink.getDataEstimate()
            if time.time() - begin_time > _timeout:
                break
        (_data, _tstamps) = sink.getData(tstamps=True)
        self.assertEquals(len(_data), len(_srcData)*2)
        self.assertEquals(sink.sri().xdelta, 1/_sampleRate)
        _orig_time = _tstamps[0][1].twsec+_tstamps[0][1].tfsec
        _round_time = int(round(_orig_time*10))/10.0
        self.assertEquals(_round_time, toffset)
        _orig_time = _tstamps[1][1].twsec+_tstamps[1][1].tfsec
        _round_time = int(round(_orig_time*10))/10.0
        self.assertEquals(_round_time, toffset+len(_srcData)/_sampleRate)
        
        toffset = toffset+len(_data)/_sampleRate
        
        # revert sample rate
        _sampleRate = 1.0
        source.push(_srcData,sampleRate=_sampleRate)
        source.push(_srcData)
        begin_time = time.time()
        estimate = sink.getDataEstimate()
        while estimate.num_timestamps != 2:
            time.sleep(0.1)
            estimate = sink.getDataEstimate()
            if time.time() - begin_time > _timeout:
                break
        (_data, _tstamps) = sink.getData(tstamps=True)
        self.assertEquals(len(_data), len(_srcData)*2)
        self.assertEquals(sink.sri().xdelta, 1/_sampleRate)
        _orig_time = _tstamps[0][1].twsec+_tstamps[0][1].tfsec
        _round_time = int(round(_orig_time*10))/10.0
        self.assertEquals(_round_time, toffset)
        _orig_time = _tstamps[1][1].twsec+_tstamps[1][1].tfsec
        _round_time = int(round(_orig_time*10))/10.0
        self.assertEquals(_round_time, toffset+len(_srcData)/_sampleRate)
        
        toffset = toffset+len(_data)/_sampleRate
        
        # test complex data
        _sampleRate = 1.0
        _srcData = [complex(1),complex(2),complex(3),complex(4)]
        source.push(_srcData,sampleRate=_sampleRate)
        source.push(_srcData)
        begin_time = time.time()
        estimate = sink.getDataEstimate()
        while estimate.num_timestamps != 2:
            time.sleep(0.1)
            estimate = sink.getDataEstimate()
            if time.time() - begin_time > _timeout:
                break
        (_data, _tstamps) = sink.getData(tstamps=True)
        self.assertEquals(len(_data), len(_srcData)*4)
        self.assertEquals(sink.sri().xdelta, 1/_sampleRate)
        _orig_time = _tstamps[0][1].twsec+_tstamps[0][1].tfsec
        _round_time = int(round(_orig_time*10))/10.0
        self.assertEquals(_round_time, toffset)
        _orig_time = _tstamps[1][1].twsec+_tstamps[1][1].tfsec
        _round_time = int(round(_orig_time*10))/10.0
        self.assertEquals(_round_time, toffset+len(_srcData)/_sampleRate)


    def test_DataSourceTimeStampParam(self):
        """
        Verify that the time stamp param is honored
        """
        _timeout = 1
        _startTime = 10
        _sampleRate = 1.0
        source = sb.DataSource(startTime=_startTime)
        sink = sb.DataSink()
        source.connect(sink, usesPortName='floatOut')
        sb.start()

        # test default sample rate
        _srcData = [1,2,3,4]
        source.push(_srcData)
        source.push(_srcData)
        estimate = sink.getDataEstimate()
        begin_time = time.time()
        while estimate.num_timestamps != 2:
            time.sleep(0.1)
            estimate = sink.getDataEstimate()
            if time.time() - begin_time > _timeout:
                break
        (_data, _tstamps) = sink.getData(tstamps=True)
        self.assertEquals(len(_data), len(_srcData)*2)
        self.assertEquals(sink.sri().xdelta, 1)
        self.assertEquals(_tstamps[0][1].twsec, _startTime)
        self.assertEquals(_tstamps[1][1].twsec, _startTime+len(_srcData))
        
        _ts = sb.createTimeStamp()
        begin_time = _ts.twsec+_ts.tfsec
        _toffset =begin_time
        source.push(_srcData, ts=_ts)
        source.push(_srcData)
        estimate = sink.getDataEstimate()
        while estimate.num_timestamps != 2:
            time.sleep(0.1)
            estimate = sink.getDataEstimate()
            if time.time() - begin_time > _timeout:
                break
        (_data, _tstamps) = sink.getData(tstamps=True)
        self.assertEquals(len(_data), len(_srcData)*2)
        self.assertEquals(sink.sri().xdelta, 1/_sampleRate)
        _pkt_time = _tstamps[0][1].twsec+_tstamps[0][1].tfsec
        _rnd_pkt_time = int(round(_pkt_time*10))/10.0
        _rnd_toffset = int(round(_toffset*10))/10.0
        self.assertEquals(_rnd_pkt_time,_rnd_toffset )
        _pkt_time = _tstamps[1][1].twsec+_tstamps[1][1].tfsec
        _rnd_pkt_time = int(round(_pkt_time*10))/10.0
        _rnd_toffset = int(round( (_toffset+len(_srcData)/_sampleRate) *10))/10.0
        self.assertEquals(_rnd_pkt_time,_rnd_toffset )


        # test modified sample rate
        _sampleRate = 10.0
        _ts = sb.createTimeStamp()
        begin_time = _ts.twsec+_ts.tfsec
        _toffset =begin_time
        source.push(_srcData,sampleRate=_sampleRate, ts=_ts)
        source.push(_srcData)
        estimate = sink.getDataEstimate()
        while estimate.num_timestamps != 2:
            time.sleep(0.1)
            estimate = sink.getDataEstimate()
            if time.time() - begin_time > _timeout:
                break
        (_data, _tstamps) = sink.getData(tstamps=True)
        self.assertEquals(len(_data), len(_srcData)*2)
        self.assertEquals(sink.sri().xdelta, 1/_sampleRate)
        _pkt_time = _tstamps[0][1].twsec+_tstamps[0][1].tfsec
        _rnd_pkt_time = int(round(_pkt_time*10))/10.0
        _rnd_toffset = int(round(_toffset*10))/10.0
        self.assertEquals(_rnd_pkt_time,_rnd_toffset )
        _pkt_time = _tstamps[1][1].twsec+_tstamps[1][1].tfsec
        _rnd_pkt_time = int(round(_pkt_time*10))/10.0
        _rnd_toffset = int(round( (_toffset+len(_srcData)/_sampleRate) *10))/10.0
        self.assertEquals(_rnd_pkt_time,_rnd_toffset )


        # test modified sample rate
        _sampleRate=5.0
        _sri=source.sri()
        _sri.xdelta = 1.0/_sampleRate
        _ts = sb.createTimeStamp()
        begin_time = _ts.twsec+_ts.tfsec
        _toffset =begin_time
        source.push(_srcData,sri=_sri, ts=_ts)
        source.push(_srcData)
        estimate = sink.getDataEstimate()
        while estimate.num_timestamps != 2:
            time.sleep(0.1)
            estimate = sink.getDataEstimate()
            if time.time() - begin_time > _timeout:
                break
        (_data, _tstamps) = sink.getData(tstamps=True)
        self.assertEquals(len(_data), len(_srcData)*2)
        self.assertEquals(sink.sri().xdelta, 1/_sampleRate)
        _pkt_time = _tstamps[0][1].twsec+_tstamps[0][1].tfsec
        _rnd_pkt_time = int(round(_pkt_time*10))/10.0
        _rnd_toffset = int(round(_toffset*10))/10.0
        self.assertEquals(_rnd_pkt_time,_rnd_toffset )
        _pkt_time = _tstamps[1][1].twsec+_tstamps[1][1].tfsec
        _rnd_pkt_time = int(round(_pkt_time*10))/10.0
        _rnd_toffset = int(round( (_toffset+len(_srcData)/_sampleRate) *10))/10.0
        self.assertEquals(_rnd_pkt_time,_rnd_toffset )

    def _fileSourceThrottle(self, _file, rate):
        fp=open(_file, 'r')
        contents = fp.read()
        fp.close()
        source = sb.FileSource(_file, dataFormat='octet', sampleRate=rate, throttle=True)
        sink = sb.DataSink()
        source.connect(sink)
        time_estimate = len(contents)/float(rate)
        sb.start()
        begin_time = time.time()
        wait_for_eos(sink)
        time_diff = time.time()-begin_time
        self.assertTrue(time_diff<time_estimate*1.1)
        self.assertTrue(time_diff>time_estimate*0.9)
        sb.stop()

    def test_FileSourceThrottle(self):
        infile = os.path.join(sb.getSDRROOT(), 'dom/mgr/DomainManager.spd.xml')
        self._fileSourceThrottle(infile, 1000)
        self._fileSourceThrottle(infile, 1500)

    def test_DataSourceThrottle(self):
        src = sb.DataSource(dataFormat='float', throttle=True)
        snk = sb.DataSink()
        src.connect(snk)
        sb.start()
        _sampleRate = 500
        _dataLength = 100
        time_estimate = (3.0*_dataLength)/(_sampleRate)
        begin_time = time.time()
        src.push([float(x) for x in range(100)],sampleRate=_sampleRate)
        src.push([float(x) for x in range(100)],sampleRate=_sampleRate)
        src.push([float(x) for x in range(100)],sampleRate=_sampleRate)
        wait_on_data(snk, 3, 5)
        end_time = time.time()
        time_diff = end_time-begin_time
        self.assertTrue(time_diff<time_estimate*1.1)
        self.assertTrue(time_diff>time_estimate*0.9)

        data=snk.getData()
        self.assertEquals(len(data), 3.0*_dataLength)

        _sampleRate = 300
        _dataLength = 100
        time_estimate = (3.0*_dataLength)/(_sampleRate)
        begin_time = time.time()
        src.push([float(x) for x in range(100)],sampleRate=_sampleRate)
        src.push([float(x) for x in range(100)],sampleRate=_sampleRate)
        src.push([float(x) for x in range(100)],sampleRate=_sampleRate)
        wait_on_data(snk, 3, 5)
        end_time = time.time()
        time_diff = end_time-begin_time
        self.assertTrue(time_diff<time_estimate*1.1)
        self.assertTrue(time_diff>time_estimate*0.9)

        data=snk.getData()
        self.assertEquals(len(data), 3.0*_dataLength)

    class customSink(bulkio_data_helpers.ArraySink):
        def __init__(self, porttype):
            bulkio_data_helpers.ArraySink.__init__(self, porttype)

        def pushSRI(self, H):
            _H = H
            _H.xdelta = H.xdelta * 2
            self.sri = _H

    def test_CustomDataSink(self):
        src = sb.DataSource(dataFormat='float')
        snk = sb.DataSink(sinkClass=self.customSink)
        src.connect(snk)
        sb.start()
        src.push([1,2,3,4,5],sampleRate=100)
        src.push([1,2,3,4,5],sampleRate=1000)
        src.push([1,2,3,4,5],sampleRate=10000)
        wait_on_data(snk, 3)
        data=snk.getData(tstamps=True)
        self.assertEquals(snk._sink.sri.xdelta, 0.0002)

    def test_DataSourceSRI(self):
        _timeout = 1
        _startTime = 10
        source = sb.DataSource(startTime=_startTime)
        sink = sb.DataSink()
        source.connect(sink, usesPortName='floatOut')
        sb.start()

        # get an sri
        _sri = source.sri()
        
        sid = 'test-sri-1'
        _sri.streamID=sid
        _sri.xdelta = 0.1234

        # push samples down stream, with custom sri
        _srcData = [1,2,3,4]
        source.push(_srcData, sri=_sri )
        wait_on_data(sink, 1)
        data=sink.getData()
        rsri=sink.sri()
        self.assertEquals(rsri.streamID, sid )
        self.assertAlmostEquals(rsri.xdelta, 0.1234)

        # add keywords as a param
        kws=[]
        kws.append(sb.SRIKeyword('kw1',1000,'long'))
        kws.append(sb.SRIKeyword('kw2',12456.0,'float'))
        kws.append(sb.SRIKeyword('kw3',16,'short'))
        kws.append(sb.SRIKeyword('kw4', 200,'octet'))
        kws.append(sb.SRIKeyword('kw5','this is a test','string'))
        kws.append(sb.SRIKeyword('kw6',[1,2],'[short]'))

        expectedType = properties.getTypeCode('short')
        expectedTypeCode = tcInternal.createTypeCode((tcInternal.tv_sequence, expectedType._d, 0))
        kw6 = CORBA.Any(expectedTypeCode, [1,2])

        matchkws=[ CF.DataType(id='kw1', value=CORBA.Any(CORBA.TC_long, 1000)), 
                   CF.DataType(id='kw2', value=CORBA.Any(CORBA.TC_float, 12456.0)), 
                   CF.DataType(id='kw3', value=CORBA.Any(CORBA.TC_short, 16)), 
                   CF.DataType(id='kw4', value=CORBA.Any(CORBA.TC_octet, 200)), 
                   CF.DataType(id='kw5', value=CORBA.Any(CORBA.TC_string, 'this is a test')),
                   CF.DataType(id='kw6', value=kw6)
                   ]
        _srcData = [1,2,3,4]
        source.push(_srcData, SRIKeywords=kws )
        begin_time = time.time()
        wait_on_data(sink, 1)
        data=sink.getData()
        rsri=sink.sri()
        self.assertEquals(rsri.streamID, sid )
        self.assertAlmostEquals(rsri.xdelta, 0.1234)
        self.assertEqual(True, compareKeywordLists( rsri.keywords, matchkws) )

        # Repeat, making sure that a second push with keywords does not fail
        source.push(_srcData, SRIKeywords=kws)
        wait_on_data(sink, 1)
        data=sink.getData()
        self.assertTrue(data)

        # add new keywords to sri
        matchkws=[ CF.DataType(id='kw1-1', value=CORBA.Any(CORBA.TC_long, 1000)), 
                   CF.DataType(id='kw2-1', value=CORBA.Any(CORBA.TC_float, 12456.0)), 
                   CF.DataType(id='kw3-1', value=CORBA.Any(CORBA.TC_short, 16)), 
                   CF.DataType(id='kw4-1', value=CORBA.Any(CORBA.TC_octet, 200)), 
                   CF.DataType(id='kw5-1', value=CORBA.Any(CORBA.TC_string, 'this is a test'))
                   ]
        _sri.keywords=copy.copy(matchkws)
        _srcData = [1,2,3,4]
        source.push(_srcData, sri=_sri )
        wait_on_data(sink, 1)
        data=sink.getData()
        rsri=sink.sri()
        self.assertEquals(rsri.streamID, sid )
        self.assertAlmostEquals(rsri.xdelta, 0.1234)
        self.assertEqual(True, compareKeywordLists( rsri.keywords, matchkws) )

        # try pushing using the same sri object with changing attributes
        _sri = sb.createSRI()
        _sri.streamID=sid
        _srcData = [1,2,3,4]
        source.push(_srcData, sri=_sri )
        wait_on_data(sink, 1)
        data=sink.getData()
        rsri=sink.sri()
        self.assertEquals(rsri.streamID, sid )

        _sri.streamID='anewsri'
        _srcData = [1,2,3,4]
        source.push(_srcData, sri=_sri )
        wait_on_data(sink, 1)
        data=sink.getData()
        rsri=sink.sri()
        self.assertEquals(rsri.streamID, 'anewsri' )

        _sri.mode=1
        _srcData = [1,2,3,4]
        source.push(_srcData, sri=_sri )
        wait_on_data(sink, 1)
        data=sink.getData()
        rsri=sink.sri()
        self.assertEquals(rsri.mode, 1 )

        _sri.mode=0
        _srcData = [1,2,3,4]
        source.push(_srcData, sri=_sri )
        wait_on_data(sink, 1)
        data=sink.getData()
        rsri=sink.sri()
        self.assertEquals(rsri.mode, 0 )

        _sri.hversion=100
        _srcData = [1,2,3,4]
        source.push(_srcData, sri=_sri )
        wait_on_data(sink, 1)
        data=sink.getData()
        rsri=sink.sri()
        self.assertEquals(rsri.hversion, 100 )


    def test_DataSinkSubsize(self):
        src=sb.DataSource(dataFormat='short',subsize=5)
        snk=sb.DataSink()
        src.connect(snk)
        sb.start()
        src.push([1,2,3,4,5,6,7,8,9,10], EOS=True)
        start = time.time()
        while not snk.eos() and (time.time() - start) < 2.0:
            time.sleep(0.1)
        data=snk.getData()
        self.assertTrue(snk.eos())
        self.assertEquals(len(data),2)
        self.assertEquals(len(data[0]),5)
        self.assertEquals(len(data[1]),5)

    def test_SubsizeComplex(self):
        # Test interleaved-to-complex
        _subsize = 10
        _frames = 4
        inData = range(_subsize * _frames)
        src=sb.DataSource(dataFormat='short',subsize=_subsize)
        snk=sb.DataSink()
        sb.start()
        src.connect(snk)
        src.push(inData,EOS=True,complexData=True)
        recData = snk.getData(eos_block=True)
        self.assertEqual(len(recData),_frames/2)
        self.assertEqual(len(recData[0]),_subsize*2)

    def test_SubsizeComplexNoEOS(self):
        # Test interleaved-to-complex
        _subsize = 10
        _frames = 4
        inData = range(_subsize * _frames)
        src=sb.DataSource(dataFormat='short',subsize=_subsize)
        snk=sb.DataSink()
        sb.start()
        src.connect(snk)
        src.push(inData,complexData=True)

        wait_on_data(snk, 1)
        recData = snk.getData()

        self.assertEqual(len(recData),_frames/2)
        self.assertEqual(len(recData[0]),_subsize*2)

    def test_DataSinkChar(self):
        src=sb.DataSource(dataFormat='char')
        snk=sb.DataSink()
        src.connect(snk)
        sb.start()
        indata = range(16)
        src.push(indata, EOS=True)
        start = time.time()
        while not snk.eos() and (time.time() - start) < 2.0:
            time.sleep(0.1)
        outdata = snk.getData()
        self.assertEquals(len(outdata), len(indata))
        self.assertEquals(outdata, indata)

    def _formatOctet(self, data):
        return list(struct.pack('%dB' % len(data), *data))

    def test_DataSinkOctet(self):
        src=sb.DataSource(dataFormat='octet')
        snk=sb.DataSink()
        src.connect(snk)
        sb.start()
        indata = range(16)
        src.push(indata, EOS=True)
        start = time.time()
        while not snk.eos() and (time.time() - start) < 2.0:
            time.sleep(0.1)
        outdata = snk.getData()
        self.assertEquals(len(outdata), len(indata))
        self.assertEquals(outdata, self._formatOctet(indata))

    def test_DataSinkCharSubsize(self):
        subsize = 8
        frames = 4
        src=sb.DataSource(dataFormat='char',subsize=subsize)
        snk=sb.DataSink()
        src.connect(snk)
        sb.start()
        src.push(range(subsize*frames), EOS=True)
        start = time.time()
        while not snk.eos() and (time.time() - start) < 2.0:
            time.sleep(0.1)
        outdata = snk.getData()
        self.assertEquals(len(outdata), frames)
        for frame in outdata:
            self.assertEquals(len(frame), subsize)

    def test_DataSinkCharSignedData(self):
        src=sb.DataSource(dataFormat='char')
        snk=sb.DataSink()
        src.connect(snk,usesPortName='charOut', providesPortName='charIn')
        sb.start()
        data=[0, 1, 2, 3, 4, 5, -1, -2, -3, -4, -5]
        src.push(data,EOS=True)
        start = time.time()
        while not snk.eos() and (time.time() - start) < 2.0:
            time.sleep(0.1)
        outdata = snk.getData()
        self.assertEquals(data,outdata)

    def test_DataSinkOctetSignedData(self):
        src=sb.DataSource(dataFormat='octet')
        snk=sb.DataSink()
        src.connect(snk,usesPortName='octetOut', providesPortName='octetIn')
        sb.start()
        data=[0, 1, 2, 3, 4, 5, -1, -2, -3, -4, -5]
        data1=[0, 1, 2, 3, 4, 5, 255,254,253,252,251]
        src.push(data,EOS=True)
        start = time.time()
        while not snk.eos() and (time.time() - start) < 2.0:
            time.sleep(0.1)
        outdata = snk.getData()
        self.assertEquals([],outdata)
        src.push(data1,EOS=True)
        start = time.time()
        while not snk.eos() and (time.time() - start) < 2.0:
            time.sleep(0.1)
        outdata = snk.getData()
        self.assertEquals(self._formatOctet(data1), outdata)

#    def test_connections(self):
#        a = sb.launch(self.test_comp)
#        b = sb.launch("SandBoxTest2")
#        no_connections = sb.launch("SimpleComponent")
#        multiple_connections = sb.launch("SandBoxTestMultipleConnections")
#
#        names = [a._componentName,              \
#                 b._componentName,              \
#                 no_connections._componentName, \
#                 multiple_connections._componentName]
#
#        # Poor args
#        self.assertRaises(AssertionError, a.connect, b, "")
#        self.assertRaises(AssertionError, a.connect, b, None, "")
#        self.assertRaises(AssertionError, a.connect, b, "", "")
#        self.assertRaises(AssertionError, a.connect, b, "long_in", "")
#        self.assertRaises(AssertionError, a.connect, b, "", "long_out")
#        self.assertRaises(AssertionError, a.connect, no_connections)
#
#        # Good connections
#        self.assertEquals(a.connect(b), True)
#        self.assertEquals(a.connect(b, "long_in"), True)
#        self.assertEquals(a.connect(b, None, "long_out"), True)
#        self.assertEquals(len(sb.domainless._currentState['Component Connections'].keys()), 3)
#
#        # Disconnect
#        a.disconnect(no_connections)
#        a.disconnect(None)
#        self.assertEquals(len(sb.domainless._currentStatep['Component Connections'].keys()), 3)
#        a.disconnect(b)
#        self.assertEquals(len(sb.domainless._currentState['Component Connections'].keys()), 0)
#
#        # Makes sure that connection keys are unique
#        self.assertEquals(a.connect(b, "long_in", "long_out", "my_conn"), True)
#        self.assertEquals(len(sb.domainless._currentState['Component Connections'].keys()), 1)
#        self.assertRaises(AssertionError, a.connect, b, "long_in", "long_out", "my_conn")
#        self.assertEquals(len(sb.domainless._currentState['Component Connections'].keys()), 1)
#
#        a.disconnect(b)
#
#        # Multiple good connections
#        self.assertRaises(AssertionError, a.connect, multiple_connections)
#
#        #### TEST for a.connectd diff ports BUG
#        ###TODO this test will fail until Issue #149 is fixed
#        #self.assertRaises(AssertionError, a.connect, multiple_connections, "long_in", "short_out")
#
#        # Tests getComponents()
#        self.assertEquals(sb.getComponent(None), None)
#        self.assertEquals(sb.getComponent(""), None)
#        for key in sb.domainless._currentState['Components Running'].keys():
#            temp = sb.getComponent(key)
#            self.assertNotEquals(temp, None)
#            self.assertEquals(temp._componentName in names, True)


class MessagePortTest(scatest.CorbaTestCase):
    def setUp(self):
        sb.setDEBUG(True)

    def tearDown(self):
        sb.release()
        sb.setDEBUG(False)
        os.environ['SDRROOT'] = globalsdrRoot

    def test_MessageSink(self):
        
        class MCB:
           def __init__(self, cond):
               self.cond = cond
               self.msg=None
               self.count=0

           def msgCallback(self, id, msg):
               self.msg = properties.prop_to_dict(msg)
               self.count = self.count + 1
               self.cond.acquire()
               self.cond.notify()
               self.cond.release()

           def reset(self):
               self.msg=None
               self.count=0

        def wait_for_msg(cond, timeout=2.0):       
            cond.acquire()
            cond.wait(timeout)
            cond.release()
                        
        msrc = sb.MessageSource()
        cond = threading.Condition()
        mcb = MCB(cond)
        msink = sb.MessageSink(messageCallback=mcb.msgCallback, storeMessages = True)
        msrc.connect(msink)
        # Simple messages come across properties list which translates into the following
        # {'sb_struct': {'sb': 'testing 1'}}

        msrc.sendMessage("testing 1")
        wait_for_msg(cond)
        self.assertEquals( mcb.msg, None )
        sb.start()
        msrc.sendMessage("testing 2")
        wait_for_msg(cond)
        msg = mcb.msg['sb_struct']['sb']
        self.assertEquals( msg, "testing 2")
        rcv_msg = msink.getMessages()
        self.assertEquals(len(rcv_msg), 1)
        self.assertEquals(rcv_msg[0], mcb.msg)
        self.assertEquals(len(msink.getMessages()), 0)
        sb.stop()

        # terminate this sink object
        msink.releaseObject()

        # create new sink and connect to source 
        msink = sb.MessageSink( messageCallback=mcb.msgCallback )
        msrc.connect(msink)

        # try and send message....wait should expire and msg == none
        mcb.reset()
        msrc.sendMessage("testing 3")
        wait_for_msg(cond)
        self.assertEquals( mcb.msg, None)

        sb.start()
        msrc.sendMessage("testing 4")
        wait_for_msg(cond)
        msg = mcb.msg['sb_struct']['sb']
        self.assertEquals( msg, "testing 4")
        mcb.reset()
        msrc.sendMessage("testing 5")
        wait_for_msg(cond)
        msg = mcb.msg['sb_struct']['sb']
        self.assertEquals( msg, "testing 5")
        self.assertEquals(len(msink.getMessages()), 0)
        sb.stop()

        # terminate this sink object
        msink.releaseObject()

        # create new sink and connect to source 
        msink = sb.MessageSink(messageCallback=None, storeMessages = True)
        msrc.connect(msink)
        sb.start()
        msrc.sendMessage("testing 4")
        msrc.sendMessage("testing 5")
        time.sleep(2)
        self.assertEquals(len(msink.getMessages()), 2)
        sb.stop()

        #  reset receiver and cycle sandbox state
        mcb.reset()
        sb.start()
        sb.stop()
        self.assertEquals( mcb.msg, None)
        msink.releaseObject()


    def test_SendMessage(self):
        
        class MCB:
           def __init__(self, cond):
               self.cond = cond
               self.count=0
               self.msg=None


           def msgCallback(self, id, msg):
               self.msg = properties.prop_to_dict(msg)
               self.count = self.count + 1
               self.cond.acquire()
               self.cond.notify()
               self.cond.release()

           def reset(self):
               self.msg=None
               self.count=0

        def wait_for_msg(cond, timeout=2.0):       
            cond.acquire()
            cond.wait(timeout)
            cond.release()
                        
        cond = threading.Condition()
        c=sb.launch('MsgPort_P')
        mcb = MCB(cond)
        msink = sb.MessageSink( messageCallback=mcb.msgCallback )
        c.connect(msink)
        src_data={ 'field1': 'testing', 'field2': 100 }
        c.sendMessage( src_data )
        wait_for_msg(cond)
        self.assertEquals( mcb.msg, None )
        sb.start()
        src_data={ 'field1': 'testing 2', 'field2': 100 }
        c.sendMessage( src_data )
        wait_for_msg(cond)
        res_data = mcb.msg['msg_out']
        self.assertEquals( src_data, res_data )        
        src_data={ 'field1': 'testing 2-1', 'field2': 101 }
        c.sendMessage( src_data, msgId='msg_out' )
        wait_for_msg(cond)
        res_data = mcb.msg['msg_out']
        self.assertEquals( src_data, res_data )        
        src_data={ 'field1': 'testing 2-2', 'field2': 102 }
        c.sendMessage( src_data, msgId='msg_out', msgPort='msg_out' )
        wait_for_msg(cond)
        res_data = mcb.msg['msg_out']
        self.assertEquals( src_data, res_data )        
        sb.stop()

        # terminate this sink object
        msink.releaseObject()

        # create new sink and connect to source 
        msink = sb.MessageSink( messageCallback=mcb.msgCallback )
        c.connect(msink, usesPortName='msg_out' )

        # try and send message....wait should expire and msg == none
        mcb.reset()
        src_data={ 'field1': 'testing 3', 'field2': 100 }
        ret=c.sendMessage( src_data, msgId='test_msg', msgPort='msg_out')
        wait_for_msg(cond)
        self.assertEquals( mcb.msg, None)

        src_data={ 'field1': 'testing 4', 'field2': 400 }
        sb.start()
        ret=c.sendMessage( src_data, msgId='test_msg', msgPort='msg_out')        
        wait_for_msg(cond)
        self.assertEquals( ret, False )
        ret=c.sendMessage( src_data, msgId='test_msg', msgPort='msg_out', restrict=False)        
        wait_for_msg(cond)
        res_data = mcb.msg['test_msg']
        self.assertEquals( src_data, res_data )        
        sb.stop()

        #  reset receiver and cycle sandbox state
        mcb.reset()
        sb.start()
        sb.stop()
        self.assertEquals( mcb.msg, None)
        c.releaseObject()
        msink.releaseObject()
