#!/usr/bin/env python
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
import os
import socket
import time
import signal
import commands
import sys
import threading
import Queue
import shutil
import subprocess, multiprocessing

from omniORB import any, CORBA

import CosEventChannelAdmin, CosEventChannelAdmin__POA
from ossie.utils.sandbox.registrar import ApplicationRegistrarStub
from ossie.utils.sandbox import naming
from ossie.utils import sb, redhawk
from ossie.cf import CF, CF__POA
import ossie.utils.testing
from redhawk import numa

from _unitTestHelpers import scatest, runtestHelpers

def hasNumaSupport():
    return runtestHelpers.haveDefine('../cpp/Makefile', 'HAVE_LIBNUMA')

skipUnless = scatest._skipUnless
def requireNuma(obj):
    return skipUnless(hasNumaSupport(), 'Affinity control is disabled')(obj)

topology = numa.NumaTopology()


class ComponentTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all component implementations in test"""
    def setUp(self):
        super(ComponentTests,self).setUp()
        self.child_pids=[]
        print "\n-----------------------"
        print "Running: ", self.id().split('.')[-1]
        print "-----------------------\n"

    def tearDown(self):
        super(ComponentTests, self).tearDown()
        sproc="./spacely sprockets"
        try:
            os.remove(sproc)
        except:
            pass
        try:
            # kill all busy.py just in case
            os.system('pkill -9 -f busy.py')
        except OSError:
            pass
        for child_p in self.child_pids:
            try:
                os.system('kill -9 '+str(child_p))
            except OSError:
                pass

    def promptToContinue(self):
        if sys.stdout.isatty():
            raw_input("Press enter to continue")
        else:
            pass # For non TTY just continue

    def promptUserInput(self, question, default):
        if sys.stdout.isatty():
            ans = raw_input("%s [%s]?" % (question, default))
            if ans == "":
                return default
            else:
                return ans
        else:
            return default

    def runGPP(self, execparam_overrides={}, initialize=True, configure={}):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        execparams.update(execparam_overrides)
        #execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        #execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        #self.launch(execparams, debugger='valgrind')
        self.launch(execparams, initialize=initialize,configure=configure )
        
        #######################################################################
        # Verify the basic state of the component
        self.assertNotEqual(self.comp_obj, None)
        self.assertEqual(self.comp_obj._non_existent(), False)
        self.assertEqual(self.comp_obj._is_a("IDL:CF/ExecutableDevice:1.0"), True)
        #self.assertEqual(self.spd.get_id(), self.comp_obj._get_identifier())
        
    def testScaBasicBehavior(self):
        #######################################################################
        # Launch the device
        # Use values that could not possibly be true so we can ensure proper behavior
        self.runGPP(initialize=False) # processor_name
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp_obj.configure(configureProps)
        
        #######################################################################
        # Validate that query returns all expected parameters
        # Query of '[]' should return the following set of properties
        expectedProps = []
        expectedProps.extend(self.getPropertySet(kinds=("configure", "execparam"), modes=("readwrite", "readonly"), includeNil=True))
        expectedProps.extend(self.getPropertySet(kinds=("allocate",), action="external", includeNil=True))
        props = self.comp_obj.query([])
        props = dict((x.id, any.from_any(x.value)) for x in props)
        # Query may return more than expected, but not less
        for expectedProp in expectedProps:
            self.assertEquals(props.has_key(expectedProp.id), True)
        
        qr = [CF.DataType(id="DCE:9190eb70-bd1e-4556-87ee-5a259dcfee39", value=any.to_any(None)), # hostName
              CF.DataType(id="DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d", value=any.to_any(None)) # DeviceKind
             ]
        qr = self.comp_obj.query(qr)
        self.assertEqual(qr[0].value.value(), socket.gethostname())
        self.assertEqual(qr[1].value.value(), "GPP")
        
        #######################################################################
        # Verify that all expected ports are available
        for port in self.scd.get_componentfeatures().get_ports().get_uses():
            port_obj = self.comp_obj.getPort(str(port.get_usesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a("IDL:CF/Port:1.0"),  True)
            
        for port in self.scd.get_componentfeatures().get_ports().get_provides():
            port_obj = self.comp_obj.getPort(str(port.get_providesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a(port.get_repid()),  True)
            
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp_obj.start()
        self.comp_obj.stop()
        
        #######################################################################
        # Simulate regular component shutdown
        self.comp_obj.releaseObject()
        
    # Create a test file system
    class FileStub(CF__POA.File):
        def __init__(self):
            self.fobj = open("dat/component_stub.py")
        
        def sizeOf(self):
            return os.path.getsize("dat/component_stub.py")
        
        def read(self, bytes):
            return self.fobj.read(bytes)
        
        def close(self):
            return self.fobj.close()
            
    class FileSystemStub(CF__POA.FileSystem):
        def list(self, path):
            return [CF.FileSystem.FileInformationType(path[1:], CF.FileSystem.PLAIN, 100, [])]
        
        def exists(self, fileName):
            tmp_fileName = './dat/'+fileName
            return os.access(tmp_fileName, os.F_OK)
            
        def open(self, path, readonly):
            file = ComponentTests.FileStub()
            return file._this()

    def testExecute(self):
        self.runGPP()

        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp_obj.configure(configureProps)
        
        fs_stub = ComponentTests.FileSystemStub()
        fs_stub_var = fs_stub._this()
        
        self.comp_obj.load(fs_stub_var, "/component_stub.py", CF.LoadableDevice.EXECUTABLE)
        self.assertEqual(os.path.isfile("component_stub.py"), True) # Technically this is an internal implementation detail that the file is loaded into the CWD of the device
        
        comp_id = "DCE:00000000-0000-0000-0000-000000000000:waveform_1"
        app_id = "waveform_1"
        appReg = ApplicationRegistrarStub(comp_id, app_id)
        appreg_ior = sb.orb.object_to_string(appReg._this())
        pid = self.comp_obj.execute("/component_stub.py", [], [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id)), 
                                                               CF.DataType(id="NAME_BINDING", value=any.to_any("component_stub")),CF.DataType(id="PROFILE_NAME", value=any.to_any("/component_stub/component_stub.spd.xml")),
                                                               CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))])
        self.assertNotEqual(pid, 0)
        
        wait_amount = (self.comp.threshold_cycle_time / 1000.0) * 4
        time.sleep(wait_amount)
        
        component_monitor = self.comp.component_monitor
        self.assertNotEqual(len(component_monitor), 0)
        self.assertEquals(component_monitor[0].pid, pid)
        self.assertEquals(component_monitor[0].component_id, comp_id)
        self.assertEquals(component_monitor[0].waveform_id, comp_id)
        self.assertEquals(component_monitor[0].num_processes, 1)
        self.assertTrue(component_monitor[0].num_threads >= 4)
        
        try:
            os.kill(pid, 0)
        except OSError:
            self.fail("Process failed to execute")
        time.sleep(1)    
        self.comp_obj.terminate(pid)
        try:
            os.kill(pid, 0)
        except OSError:
            pass
        else:
            self.fail("Process failed to terminate")
            
    def testBusy(self):
        self.runGPP()

        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.IDLE)
        cores = multiprocessing.cpu_count()
        sleep_time = 3+cores/10.0
        if sleep_time < 7:
            sleep_time = 7
        procs = []
        for core in range(cores*2):
            procs.append(subprocess.Popen('./busy.py'))
        time.sleep(sleep_time)
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.BUSY)
        br=self.comp.busy_reason.queryValue()
        br_cpu="CPU IDLE" in br.upper() or "LOAD AVG" in br.upper()
        self.assertEqual(br_cpu, True)
        for proc in procs:
            proc.kill()
        time.sleep(sleep_time)
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.IDLE)
        self.assertEqual(self.comp.busy_reason, "")
        
        fs_stub = ComponentTests.FileSystemStub()
        fs_stub_var = fs_stub._this()
        
        self.comp_obj.load(fs_stub_var, "/component_stub.py", CF.LoadableDevice.EXECUTABLE)
        self.assertEqual(os.path.isfile("component_stub.py"), True) # Technically this is an internal implementation detail that the file is loaded into the CWD of the device
        
        comp_id = "DCE:00000000-0000-0000-0000-000000000000:waveform_1"
        app_id = "waveform_1"
        appReg = ApplicationRegistrarStub(comp_id, app_id)
        appreg_ior = sb.orb.object_to_string(appReg._this())
        pid = self.comp_obj.execute("/component_stub.py", [], [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id)), 
                                                               CF.DataType(id="NAME_BINDING", value=any.to_any("component_stub")),CF.DataType(id="PROFILE_NAME", value=any.to_any("/component_stub/component_stub.spd.xml")),
                                                               CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))])
        self.assertNotEqual(pid, 0)
        time.sleep(1)
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.ACTIVE)
        cores = multiprocessing.cpu_count()
        procs = []
        for core in range(cores*2):
            procs.append(subprocess.Popen('./busy.py'))
        time.sleep(sleep_time)
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.BUSY)
        br_cpu="CPU IDLE" in br.upper() or "LOAD AVG" in br.upper()
        for proc in procs:
            proc.kill()
        time.sleep(sleep_time)
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.ACTIVE)
        self.assertEqual(self.comp.busy_reason.queryValue(), "")
        
        try:
            os.kill(pid, 0)
        except OSError:
            self.fail("Process failed to execute")
        time.sleep(1)    
        self.comp_obj.terminate(pid)
        try:
            # kill all busy.py just in case
            os.system('pkill -9 -f busy.py')
            os.kill(pid, 0)
        except OSError:
            pass
        else:
            self.fail("Process failed to terminate")



    def test_busy_allow(self):
        self.runGPP(execparam_overrides={'DEBUG_LEVEL': 3 })

        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.IDLE)
        cores = multiprocessing.cpu_count()
        sleep_time = 3+cores/10.0
        if sleep_time < 7:
            sleep_time = 7
        procs = []
        for core in range(cores*2+5):
            procs.append(subprocess.Popen('./busy.py'))
        time.sleep(sleep_time)
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.BUSY)
        br=self.comp.busy_reason.queryValue()
        br_cpu="CPU IDLE" in br.upper() or "LOAD AVG" in br.upper()
        self.assertEqual(br_cpu, True)

        # turn off check for idle
        self.comp.thresholds.cpu_idle = 0.0
        # wait for busy to be reported... should just be load avg .. takes approx 1 minute
        for i in range(42):
            br=self.comp.busy_reason.queryValue()
            br_cpu="LOAD AVG" in br.upper()
            if br_cpu == True:
                break
            time.sleep(1.5)
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.BUSY)
        self.assertEqual(br_cpu, True)

        # turn off check for load_avg
        self.comp.thresholds.load_avg = 100.0
        for i in range(5):
            br=self.comp.busy_reason.queryValue()
            if br == "":
                break
            time.sleep(1.5)
        # wait for busy to be reported... should just be load avg now
        br=self.comp.busy_reason.queryValue()
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.IDLE)
        self.assertEqual(br, "")

        # turn on check for cpu_idle check
        self.comp.thresholds.cpu_idle = 10.0
        # wait for busy to be reported... should just be load avg now
        for i in range(5):
            br=self.comp.busy_reason.queryValue()
            br_cpu="CPU IDLE" in br.upper()
            if br_cpu == True:
                break
            time.sleep(1.5)
        br_cpu="CPU IDLE" in br.upper()
        self.assertEqual(br_cpu, True)
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.BUSY)

        # turn on check for load_avg check
        self.comp.thresholds.load_avg = 80.0
        # wait for busy to be reported... should just be load avg now
        for i in range(5):
            br=self.comp.busy_reason.queryValue()
            br_cpu="CPU IDLE" in br.upper() or "LOAD AVG" in br.upper()
            if br_cpu == True:
                break
            time.sleep(1.5)

        self.assertEqual(br_cpu, True)
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.BUSY)
        for proc in procs:
            proc.kill()
        for i in range(40):
            br=self.comp.busy_reason.queryValue()
            if br == "":
                break
            time.sleep(1.5)
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.IDLE)
        self.assertEqual(self.comp.busy_reason.queryValue(), "")
        time.sleep(1)    
        try:
            # kill all busy.py just in case
            os.system('pkill -9 -f busy.py')
        except OSError:
            pass


    def visual_testBusy(self):
        self.runGPP()

        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.IDLE)
        cores = multiprocessing.cpu_count()
        sleep_time = 3+cores/10.0
        if sleep_time < 7:
            sleep_time = 7
        procs = []
        for core in range(cores*2):
            procs.append(subprocess.Popen('./busy.py'))
        end_time = time.time() + sleep_time
        while end_time > time.time():
            print str(time.time()) + " busy reason: " + str(self.comp.busy_reason)
            time.sleep(.4)
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.BUSY)
        br=self.comp.busy_reason
        br_cpu="CPU IDLE" in br.upper() or "LOAD AVG" in br.upper()
        self.assertEqual(br_cpu, True)
        for proc in procs:
            proc.kill()
        time.sleep(sleep_time)
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.IDLE)
        self.assertEqual(self.comp.busy_reason, "")

        fs_stub = ComponentTests.FileSystemStub()
        fs_stub_var = fs_stub._this()

        self.comp_obj.load(fs_stub_var, "/component_stub.py", CF.LoadableDevice.EXECUTABLE)
        self.assertEqual(os.path.isfile("component_stub.py"), True) # Technically this is an internal implementation detail that the file is loaded into the CWD of the device

        comp_id = "DCE:00000000-0000-0000-0000-000000000000:waveform_1"
        app_id = "waveform_1"
        appReg = ApplicationRegistrarStub(comp_id, app_id)
        appreg_ior = sb.orb.object_to_string(appReg._this())
        pid = self.comp_obj.execute("/component_stub.py", [], [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id)),
                                                               CF.DataType(id="NAME_BINDING", value=any.to_any("component_stub")),CF.DataType(id="PROFILE_NAME", value=any.to_any("/component_stub/component_stub.spd.xml")),
                                                               CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))])
        self.assertNotEqual(pid, 0)
        time.sleep(1)
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.ACTIVE)
        cores = multiprocessing.cpu_count()
        procs = []
        for core in range(cores*2):
            procs.append(subprocess.Popen('./busy.py'))
        end_time = time.time() + sleep_time
        while end_time > time.time():
            print str(time.time()) + " busy reason: " + str(self.comp.busy_reason)
            time.sleep(.4)

        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.BUSY)
        br_cpu="CPU IDLE" in br.upper() or "LOAD AVG" in br.upper()
        for proc in procs:
            proc.kill()
        time.sleep(sleep_time)
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.ACTIVE)
        self.assertEqual(self.comp.busy_reason, "")

        try:
            os.kill(pid, 0)
        except OSError:
            self.fail("Process failed to execute")
        time.sleep(1)
        self.comp_obj.terminate(pid)
        try:
            # kill all busy.py just in case
            os.system('pkill -9 -f busy.py')
            os.kill(pid, 0)
        except OSError:
            pass
        else:
            self.fail("Process failed to terminate")


    def No_testScreenExecute(self):
        self.runGPP({"DCE:218e612c-71a7-4a73-92b6-bf70959aec45": True})

        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp_obj.configure(configureProps)
        
        qr = self.comp_obj.query([CF.DataType(id="DCE:218e612c-71a7-4a73-92b6-bf70959aec45", value=any.to_any(None))])
        useScreen = qr[0].value.value()
        self.assertEqual(useScreen, True)
        
        fs_stub = ComponentTests.FileSystemStub()
        fs_stub_var = fs_stub._this()
        
        self.comp_obj.load(fs_stub_var, "/component_stub.py", CF.LoadableDevice.EXECUTABLE)
        self.assertEqual(os.path.isfile("component_stub.py"), True) # Technically this is an internal implementation detail that the file is loaded into the CWD of the device
        
        comp_id = "DCE:00000000-0000-0000-0000-000000000000:waveform_1"
        app_id = "waveform_1"
        appReg = ApplicationRegistrarStub(comp_id, app_id)
        appreg_ior = sb.orb.object_to_string(appReg._this())
        pid = self.comp_obj.execute("/component_stub.py", [], [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id)), 
                                                               CF.DataType(id="NAME_BINDING", value=any.to_any("MyComponent")),CF.DataType(id="PROFILE_NAME", value=any.to_any("empty")),
                                                               CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))])
        self.assertNotEqual(pid, 0)
        
        try:
            os.kill(pid, 0)
        except OSError:
            self.fail("Process failed to execute")
        time.sleep(1)
           
        if os.environ.has_key("SCREENDIR"):
            screendir = os.path.expandvars("${SCREENDIR}")
        else:
            screendir = "/var/run/screen/S-%s" % os.environ['USER'] # RHEL specific
        screens = os.listdir(screendir)
        self.assertNotEqual(len(screens), 0)
        
        scrpid = None
        scrname = None
        for screen in screens:
            p, n = screen.split(".", 1)
            if n == "waveform_1.MyComponent":
                scrpid = int(p)
                scrname = n
                break
        self.assertEqual(scrpid, pid)
        self.assertEqual(scrname, "waveform_1.MyComponent")
        
        self.comp_obj.terminate(pid)
        time.sleep(1)
        try:
            os.kill(pid, 0)
        except OSError:
            pass
        else:
            self.fail("Process failed to terminate")
        
        output,status = commands.getstatusoutput('screen -wipe')
            
        screens = os.listdir(screendir)
        self.assertEqual(len(screens), 0)
        
        scrpid = None
        scrname = None
        for screen in screens:
            p, n = screen.split(".", 1)
            if n == "waveform_1.MyComponent":
                scrpid = int(p)
                scrname = n
                break
        self.assertEqual(scrpid, None)
        self.assertEqual(scrname, None)

    def test_scraping_proc(self):

        # start up subprocess with spaces in the name...
        proc="./busy.py"
        sproc="./spacely sprockets"
        shutil.copy(proc,sproc)
        procs = subprocess.Popen(sproc)
        self.assertEqual(procs.poll(), None )

        self.runGPP()
        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.IDLE)
        # wait for an update to occur
        time.sleep(4)

        # basically if we get past here.. and did not crash we are goodx
        self.assertEqual(self.comp._process.isAlive(), True )
        try:
            os.system('pkill -9 -f "'+sproc+'"')
            proc.kill()
        except:
            pass

        
    def testPropertyEvents(self):
        class Consumer_i(CosEventChannelAdmin__POA.ProxyPushConsumer):
            def __init__(self, parent, instance_id):
                self.supplier = None
                self.parent = parent
                self.instance_id = instance_id
                self.existence_lock = threading.Lock()
                
            def push(self, data):
                self.parent.actionQueue.put(data)
            
            def connect_push_supplier(self, supplier):
                self.supplier = supplier
                
            def disconnect_push_consumer(self):
                self.existence_lock.acquire()
                try:
                    self.supplier.disconnect_push_supplier()
                except:
                    pass
                self.existence_lock.release()
            
        class SupplierAdmin_i(CosEventChannelAdmin__POA.SupplierAdmin):
            def __init__(self, parent):
                self.parent = parent
                self.instance_counter = 0
        
            def obtain_push_consumer(self):
                self.instance_counter += 1
                self.parent.consumer_lock.acquire()
                self.parent.consumers[self.instance_counter] = Consumer_i(self.parent,self.instance_counter)
                objref = self.parent.consumers[self.instance_counter]._this()
                self.parent.consumer_lock.release()
                return objref
        
        class EventChannelStub(CosEventChannelAdmin__POA.EventChannel):
            def __init__(self):
                self.consumer_lock = threading.RLock()
                self.consumers = {}
                self.actionQueue = Queue.Queue()
                self.supplier_admin = SupplierAdmin_i(self)

            def for_suppliers(self):
                return self.supplier_admin._this()

        #######################################################################
        # Launch the device
        self.runGPP({"propertyEventRate": 5})
        
        orb = CORBA.ORB_init()
        obj_poa = orb.resolve_initial_references("RootPOA")
        poaManager = obj_poa._get_the_POAManager()
        poaManager.activate()

        eventChannel = EventChannelStub()
        eventChannelId = obj_poa.activate_object(eventChannel)
        eventPort = self.comp_obj.getPort("propEvent")
        eventPort = eventPort._narrow(CF.Port)
        eventPort.connectPort(eventChannel._this(), "eventChannel")

        #configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        configureProps = [CF.DataType(id='DCE:22a60339-b66e-4309-91ae-e9bfed6f0490',value=any.to_any(81))]
        self.comp_obj.configure(configureProps)
        
        # Make sure the background status events are emitted
        time.sleep(0.5)
        
        self.assert_(eventChannel.actionQueue.qsize() > 0)
        
        event = eventChannel.actionQueue.get()
        event = any.from_any(event, keep_structs=True)
        event_dict = ossie.properties.props_to_dict(event.properties)
        self.assert_(self.comp_obj._get_identifier() == event.sourceId)
        self.assert_('DCE:22a60339-b66e-4309-91ae-e9bfed6f0490' == event.properties[0].id)
        self.assert_(81 == any.from_any(event.properties[0].value))


    def test_workingCacheDirView(self):
        # set mcast exec param values for the test
        eparms = { "DCE:4e416acc-3144-47eb-9e38-97f1d24f7700": 'eth0',
                   'DCE:5a41c2d3-5b68-4530-b0c4-ae98c26c77ec': 100,
                   'DCE:442d5014-2284-4f46-86ae-ce17e0749da0': 100 }
        self.runGPP(eparms)
        cwd = os.getcwd()
        self.assertEquals(cwd,self.comp.cacheDirectory)
        self.assertEquals(cwd,self.comp.workingDirectory)


    def test_mcastNicThreshold(self):

        # set mcast exec param values for the test
        eparms = { "DCE:4e416acc-3144-47eb-9e38-97f1d24f7700": 'eth0',
                   'DCE:5a41c2d3-5b68-4530-b0c4-ae98c26c77ec': 100,
                   'DCE:442d5014-2284-4f46-86ae-ce17e0749da0': 100 }
        self.runGPP(eparms)

        # fire off change listener for setting threshold
        cprops = [CF.DataType(id='DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1',value=any.to_any(1))]
        self.comp_obj.configure(cprops)

        # check that values were changed
        cprops = [CF.DataType(id='DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1',value=any.to_any(None)),
                  CF.DataType(id='DCE:506102d6-04a9-4532-9420-a323d818ddec',value=any.to_any(None)) ]
        cprops = self.comp_obj.query(cprops)
        self.assertEquals( cprops[0].value.value(), 1)
        self.assertEquals( cprops[1].value.value(), 1)

        # try failed allocation
        allocProps = [CF.DataType(id='DCE:506102d6-04a9-4532-9420-a323d818ddec',value=any.to_any(200))]
        self.assertRaises( CF.Device.InvalidCapacity, self.comp_obj.allocateCapacity, allocProps)

        # fire off change listener for setting threshold
        cprops = [CF.DataType(id='DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1',value=any.to_any(90))]
        self.comp_obj.configure(cprops)

        # check that values were changed
        cprops = [CF.DataType(id='DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1',value=any.to_any(None)),
                  CF.DataType(id='DCE:506102d6-04a9-4532-9420-a323d818ddec',value=any.to_any(None)) ]
        cprops = self.comp_obj.query(cprops)
        self.assertEquals( cprops[0].value.value(), 90)
        self.assertEquals( cprops[1].value.value(), 90)

        # try good allocation
        allocProps = [CF.DataType(id='DCE:506102d6-04a9-4532-9420-a323d818ddec',value=any.to_any(50))]
        self.comp_obj.allocateCapacity( allocProps)

        cprops = [CF.DataType(id='DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1',value=any.to_any(None)),
                  CF.DataType(id='DCE:506102d6-04a9-4532-9420-a323d818ddec',value=any.to_any(None)) ]
        cprops = self.comp_obj.query(cprops)
        self.assertEquals( cprops[0].value.value(), 90)
        self.assertEquals( cprops[1].value.value(), 40)

        self.comp_obj.deallocateCapacity( allocProps)

        cprops = [CF.DataType(id='DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1',value=any.to_any(None)),
                  CF.DataType(id='DCE:506102d6-04a9-4532-9420-a323d818ddec',value=any.to_any(None)) ]
        cprops = self.comp_obj.query(cprops)
        self.assertEquals( cprops[0].value.value(), 90)
        self.assertEquals( cprops[1].value.value(), 90)


    def test_loadCapacity(self):

        self.runGPP()

        # query current capcity 
        cprops = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(None))]
        cprops=self.comp_obj.query(cprops)

        capacity=cprops[0].value.value()
        req_cap = capacity/2;

        # try allocation
        allocProps = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(req_cap))]
        self.comp_obj.allocateCapacity(allocProps)

        cprops = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(None))]
        cprops=self.comp_obj.query(cprops)

        # make sure capacity was affected
        remaining_cap = capacity-req_cap
        self.assertEquals( cprops[0].value.value(), remaining_cap)

        # now request more... result will be be  0..
        allocProps = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(capacity))]
        self.comp_obj.allocateCapacity(allocProps)

        cprops = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(None))]
        cprops=self.comp_obj.query(cprops)

        # make sure capacity is zero..
        self.assertEquals( cprops[0].value.value(), 0.0)

        # now dealloacte more than requests.. should go back to max.
        allocProps = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(capacity*2))]
        self.comp_obj.deallocateCapacity(allocProps)

        cprops = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(None))]
        cprops=self.comp_obj.query(cprops)

        # make sure capacity is original max.
        self.assertEquals( cprops[0].value.value(), capacity)   

        # now set reservation mode off 
        self.comp.reserved_capacity_per_component=0.0

        # now try to allocate capacity, should fail
        allocProps = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(capacity*2))]
        self.assertRaises( CF.Device.InsufficientCapacity, self.comp_obj.allocateCapacity, allocProps)

    def test_sys_limits(self):

        self.runGPP()
        p=CF.DataType(id='sys_limits',value=any.to_any(None))
        retval = self.comp.query([p])[0].value._v
        ids = []
        for item in retval:
            ids.append(item.id)
        self.assertTrue('sys_limits::current_threads')
        self.assertTrue('sys_limits::max_threads')
        self.assertTrue('sys_limits::current_open_files')
        self.assertTrue('sys_limits::max_open_files')

    def get_single_nic_interface(self):
        import commands
        (exitstatus, ifconfig_info) = commands.getstatusoutput('/sbin/ifconfig -a')
        if exitstatus != 0:
            self._log.debug("Proplem running '/sbin/ifconfig'")
            return

        self.nic_list = []
        # add vlans
        for i in ifconfig_info.splitlines():
            i = i.strip()
            if i.startswith('e') == False or i.find('Link encap') < 0:
                continue

            if len(i.split()) > 0  :
               self.nic_list.append( i.split()[0] )


    def test_threshold_usagestate(self):

        self.get_single_nic_interface()
        if len(self.nic_list)> 1:
            self.runGPP(configure={'nic_interfaces' : [ self.nic_list[0] ]})
        else:
            self.runGPP()

        # set cpu to be 100.00 ... the check busy state..
        orig_thres = self.comp.thresholds.cpu_idle.queryValue()
        self.comp.thresholds.cpu_idle = 100.00
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.BUSY: break
           time.sleep(.5)

        self.assertEquals(ustate, CF.Device.BUSY)

        # set cpu idle  back
        self.comp.thresholds.cpu_idle = orig_thres
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.IDLE: break
           time.sleep(.5)

        self.assertEquals(ustate, CF.Device.IDLE)

        # set mem_free 
        orig_thres = self.comp.thresholds.mem_free.queryValue()
        mem_free = self.comp.memFree.queryValue()
        self.comp.thresholds.mem_free = mem_free+2000
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.BUSY: break
           time.sleep(.5)

        self.assertEquals(ustate, CF.Device.BUSY)

        # set mem_free  back
        self.comp.thresholds.mem_free = orig_thres
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.IDLE: break
           time.sleep(.5)

        self.assertEquals(ustate, CF.Device.IDLE)


        # set load_avg
        orig_thres = self.comp.thresholds.load_avg.queryValue()
        self.comp.thresholds.load_avg=0.0
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.BUSY: break
           time.sleep(.5)

        self.assertEquals(ustate, CF.Device.BUSY)

        # set load_avg  back
        self.comp.thresholds.load_avg = orig_thres
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.IDLE: break
           time.sleep(.5)

        self.assertEquals(ustate, CF.Device.IDLE)


        # set nic_usage
        orig_thres = self.comp.thresholds.nic_usage.queryValue()
        self.comp.thresholds.nic_usage=0.0
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.BUSY: break
           time.sleep(.5)

        self.assertEquals(ustate, CF.Device.BUSY)

        # set nic_usage  back
        self.comp.thresholds.nic_usage = orig_thres
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.IDLE: break
           time.sleep(.5)

        self.assertEquals(ustate, CF.Device.IDLE)

        # set files_available
        orig_thres = self.comp.thresholds.files_available.queryValue()
        self.comp.thresholds.files_available=100.0
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.BUSY: break
           time.sleep(.5)

        self.assertEquals(ustate, CF.Device.BUSY)

        # set files_available back
        self.comp.thresholds.files_available = orig_thres
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.IDLE: break
           time.sleep(.5)

        self.assertEquals(ustate, CF.Device.IDLE)

        # set threads
        orig_thres = self.comp.thresholds.threads.queryValue()
        self.comp.thresholds.threads=100.0
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.BUSY: break
           time.sleep(.5)

        self.assertEquals(ustate, CF.Device.BUSY)

        # set threads back
        self.comp.thresholds.threads = orig_thres
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.IDLE: break
           time.sleep(.5)

        self.assertEquals(ustate, CF.Device.IDLE)


    def test_threshold_usagestate_ignored(self):

        self.get_single_nic_interface()
        if len(self.nic_list)> 1:
            self.runGPP(configure={'nic_interfaces' : [ self.nic_list[0] ]})
        else:
            self.runGPP()

        self.comp.thresholds.ignore = True

        # set cpu to be 100.00 ... the check busy state..
        orig_thres = self.comp.thresholds.cpu_idle.queryValue()
        self.comp.thresholds.cpu_idle = 100.00
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.BUSY: break
           time.sleep(.5)

        self.assertNotEquals(ustate, CF.Device.BUSY)

        # set cpu idle  back
        self.comp.thresholds.cpu_idle = orig_thres
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.IDLE: break
           time.sleep(.5)

        self.assertEquals(ustate, CF.Device.IDLE)

        # set mem_free 
        orig_thres = self.comp.thresholds.mem_free.queryValue()
        mem_free = self.comp.memFree.queryValue()
        self.comp.thresholds.mem_free = mem_free+2000
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.BUSY: break
           time.sleep(.5)

        self.assertNotEquals(ustate, CF.Device.BUSY)

        # set mem_free  back
        self.comp.thresholds.mem_free = orig_thres
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.IDLE: break
           time.sleep(.5)

        self.assertEquals(ustate, CF.Device.IDLE)


        # set load_avg
        orig_thres = self.comp.thresholds.load_avg.queryValue()
        self.comp.thresholds.load_avg=0.0
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.BUSY: break
           time.sleep(.5)

        self.assertNotEquals(ustate, CF.Device.BUSY)

        # set load_avg  back
        self.comp.thresholds.load_avg = orig_thres
        ustate=None
        for i in xrange(6):
           ustate= self.comp._get_usageState()
           if ustate == CF.Device.IDLE: break
           time.sleep(.5)

        self.assertEquals(ustate, CF.Device.IDLE)


    def testReservation(self):
        self.runGPP()
        self.comp.thresholds.cpu_idle = 50
        self.comp.reserved_capacity_per_component = 0.5
        number_reservations = (self.comp.processor_cores / self.comp.reserved_capacity_per_component) * ((100-self.comp.thresholds.cpu_idle)/100.0)
        comp_id = "DCE:00000000-0000-0000-0000-000000000000:waveform_1"
        app_id = "waveform_1"
        appReg = ApplicationRegistrarStub(comp_id, app_id)
        appreg_ior = sb.orb.object_to_string(appReg._this())
        self.assertEquals(self.comp._get_usageState(),CF.Device.IDLE)
        for i in range(int(number_reservations-1)):
            self.child_pids.append(self.comp.ref.execute("/component_stub.py", [], [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id+str(i))), CF.DataType(id="NAME_BINDING", value=any.to_any("component_stub_"+str(i))), CF.DataType(id="PROFILE_NAME", value=any.to_any("/component_stub/component_stub.spd.xml")), CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))]))
            time.sleep(0.1)
        time.sleep(2)
        self.assertEquals(self.comp._get_usageState(),CF.Device.ACTIVE)
        self.child_pids.append(self.comp_obj.execute("/component_stub.py", [], [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id)), 
                                                               CF.DataType(id="NAME_BINDING", value=any.to_any("component_stub")),CF.DataType(id="PROFILE_NAME", value=any.to_any("/component_stub/component_stub.spd.xml")),
                                                               CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))]))
        time.sleep(2)
        self.assertEquals(self.comp._get_usageState(),CF.Device.BUSY)

    def testFloorReservation(self):
        self.runGPP()
        self.comp.thresholds.cpu_idle = 10
        self.comp.reserved_capacity_per_component = 0.5
        number_reservations = (self.comp.processor_cores / self.comp.reserved_capacity_per_component) * ((100-self.comp.thresholds.cpu_idle)/100.0)
        comp_id = "DCE:00000000-0000-0000-0000-000000000000:waveform_1"
        app_id = "waveform_1"
        appReg = ApplicationRegistrarStub(comp_id, app_id)
        appreg_ior = sb.orb.object_to_string(appReg._this())
        self.assertEquals(self.comp._get_usageState(),CF.Device.IDLE)
        self.child_pids.append(self.comp_obj.execute("/component_stub.py", [], [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id+'_1')), CF.DataType(id="NAME_BINDING", value=any.to_any("component_stub_1")), CF.DataType(id="PROFILE_NAME", value=any.to_any("/component_stub/component_stub.spd.xml")), CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))]))
        time.sleep(2.1)
        self.assertEquals(self.comp._get_usageState(),CF.Device.ACTIVE)
        self.child_pids.append(self.comp_obj.execute("/component_stub.py", [], [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id+'_1')), CF.DataType(id="NAME_BINDING", value=any.to_any("component_stub_1")), CF.DataType(id="PROFILE_NAME", value=any.to_any("/component_stub/component_stub.spd.xml")), CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))]))
        time.sleep(2.1)
        self.assertEquals(self.comp._get_usageState(),CF.Device.ACTIVE)
        pid = self.child_pids.pop()
        self.comp_obj.terminate(pid)
        time.sleep(2.1)
        reservation = CF.DataType(id="RH::GPP::MODIFIED_CPU_RESERVATION_VALUE", value=any.to_any(1000.0))
        self.child_pids.append(self.comp_obj.execute("/component_stub.py", [], [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id)), 
                                                               CF.DataType(id="NAME_BINDING", value=any.to_any("component_stub")),CF.DataType(id="PROFILE_NAME", value=any.to_any("/component_stub/component_stub.spd.xml")),
                                                               CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior)), reservation]))
        time.sleep(2)
        self.assertEquals(self.comp._get_usageState(),CF.Device.BUSY)

@requireNuma
class AffinityTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the device.
    SPD_FILE = '../GPP.spd.xml'

    # setUp is run before every function preceded by "test" is executed
    # tearDown is run after every function preceded by "test" is executed
    
    # self.comp is a device using the sandbox API
    # to create a data source, the package sb contains data sources like DataSource or FileSource
    # to create a data sink, there are sinks like DataSink and FileSink
    # to connect the component to get data from a file, process it, and write the output to a file, use the following syntax:
    #  src = sb.FileSource('myfile.dat')
    #  snk = sb.DataSink()
    #  src.connect(self.comp)
    #  self.comp.connect(snk)
    #  sb.start()
    #
    # components/sources/sinks need to be started. Individual components or elements can be started
    #  src.start()
    #  self.comp.start()
    #
    # every component/elements in the sandbox can be started
    #  sb.start()

    def setUp(self):
        print "\n-----------------------"
        print "Running: ", self.id().split('.')[-1]
        print "-----------------------\n"

        # Launch the device, using the selected implementation
        self.comp = sb.launch(self.spd_file, impl=self.impl, properties={'affinity':{'disabled':False}})

        self._pids = []
    
    def tearDown(self):
        # Terminate all launched executables, ignoring errors
        remaining_pids = []
        for pid in self._pids:
            try:
                self.comp.ref.terminate(pid)
            except:
                remaining_pids.append(pid)

        # In case the GPP really failed badly, manually kill the processes
        for pid in remaining_pids:
            os.killpg(pid, 9)

        # Clean up all sandbox artifacts created during test
        sb.release()

    def _getAllowedCpuList(self, pid):
        filename = '/proc/%d/status' % pid
        with open(filename, 'r') as fp:
            for line in fp:
                if not 'Cpus_allowed_list' in line:
                    continue
                cpu_list = line.split()[1]
                return numa.parseValues(cpu_list, ",")
        return []

    def _execute(self, executable, options, parameters):
        if isinstance(options, dict):
            options = [CF.DataType(k, any.to_any(v)) for k, v in options.items()]
        if isinstance(parameters, dict):
            parameters = [CF.DataType(k, any.to_any(v)) for k, v in parameters.items()]
        pid = self.comp.ref.execute(executable, options, parameters)
        if pid != 0:
            self._pids.append(pid)
        return pid

    def _deployWithAffinityOptions(self, name, affinity={}):
        appReg = naming.ApplicationRegistrarStub()
        appreg_ior = sb.orb.object_to_string(appReg._this())
        options = {}
        if affinity:
            options['AFFINITY'] = [CF.DataType(k, any.to_any(v)) for k,v in affinity.items()]
            
        pid = self._execute("/component_stub.py", options,
                            {"COMPONENT_IDENTIFIER": name, 
                             "NAME_BINDING": name,
                             "PROFILE_NAME": "/component_stub/component_stub.spd.xml",
                             "NAMING_CONTEXT_IOR": appreg_ior})
        self.assertNotEqual(pid, 0)

        # There is a delay between when execute() returns and the when the GPP
        # applies the affinity settings that may cause false failures; waiting
        # until the component registers ensures that the affinity is set
        end = time.time() + 2.0
        while time.time() < end:
            comp = appReg.getObject(name)
            if comp is not None:
                break
            time.sleep(0.1)

        self.failIf(comp is None, "component '" + name + "' never registered")

        return pid

    def _getNicAffinity(self, nic):
        cpu_list = []
        with open('/proc/interrupts', 'r') as fp:
            for line in fp:
                # Remove final newline and make sure the line ends with the NIC
                # name (in the unlikely event a machine goes up to "em11")
                line = line.rstrip()
                if not line.endswith(nic):
                    continue
                # Discard the first entry (the IRQ number) and the last two
                # (type and name) to get the CPU IRQ service totals
                cpu_irqs = line.split()[1:-2]
                for cpu, count in enumerate(cpu_irqs):
                    if int(count) > 0:
                        cpu_list.append(cpu)
                break
        return cpu_list

    @skipUnless(len(topology.nodes) > 1, 'At least two NUMA nodes required')
    def testNicAffinity(self):
        # Pick the first NIC and figure out which CPU(s) service its IRQ, then
        # build the list of all CPUs on that node
        self.assertNotEqual(0, len(self.comp.available_nic_interfaces), 'no available NIC interfaces')
        nic = self.comp.available_nic_interfaces[0]
        nodes = set(topology.getNodeForCpu(cpu) for cpu in self._getNicAffinity(nic))
        # Join the CPU lists together
        nic_cpus = sum((node.cpus for node in nodes), [])

        # Launch the component stub with affinity based on the selected NIC;
        # with no CPU blacklist, GPP will assign the component to all of the
        # CPUs on the same socket(s)
        pid = self._deployWithAffinityOptions('nic_affinity_1', {'nic':nic})
        allowed_cpus = self._getAllowedCpuList(pid)
        self.assertEqual(nic_cpus, allowed_cpus)

    def testNicAffinityWithBlackList(self):
        # Pick the first NIC
        self.assertNotEqual(0, len(self.comp.available_nic_interfaces), 'no available NIC interfaces')
        nic = self.comp.available_nic_interfaces[0]
        nic_cpus = self._getNicAffinity(nic)
        if len(nic_cpus) > 1:
            # There's more than one CPU assigned to service NIC interrupts,
            # just blacklist the first one
            blacklist_cpu = nic_cpus.pop(0)
        else:
            # Only one CPU for NIC interrupts, figure out its node and
            # blacklist one of the other CPUs
            cpu = nic_cpus[0]
            node = topology.getNodeForCpu(cpu)
            # Find the CPU in the list and select the next one (wrapping around
            # as necessary) to ensure that we don't blacklist the wrong CPU
            index = node.cpus.index(cpu)
            index = (index + 1) % len(node.cpus)
            blacklist_cpu = node.cpus[index]

        # With a CPU blacklist, only the CPUs that are expliclitly allowed to
        # handle the NIC are in the allowed list, as opposed to all CPUs in the
        # same socket(s)
        self.comp.affinity.blacklist_cpus = str(blacklist_cpu)
        pid = self._deployWithAffinityOptions('nic_affinity_bl_1', {'nic':nic})
        allowed_cpus = self._getAllowedCpuList(pid)
        self.assertEqual(nic_cpus, allowed_cpus)

    def testCpuAffinity(self):
        # Pick the last CPU in the last node; the component should only be
        # allowed to run on that CPU
        cpu = topology.nodes[-1].cpus[-1]
        pid = self._deployWithAffinityOptions('cpu_affinity_1', {'affinity::exec_directive_class': 'cpu',
                                                                 'affinity::exec_directive_value': cpu})
        allowed_cpus = self._getAllowedCpuList(pid)
        self.assertEqual([cpu], allowed_cpus)

    @skipUnless(len(topology.nodes) > 1, 'At least two NUMA nodes are required')
    def testSocketAffinity(self):
        # Pick the last node and deploy to it; the process should be allowed to
        # run on all CPUs from that node
        node = topology.nodes[-1]
        pid = self._deployWithAffinityOptions('socket_affinity_1', {'affinity::exec_directive_class': 'socket',
                                                                    'affinity::exec_directive_value': node.node})
        allowed_cpus = self._getAllowedCpuList(pid)
        self.assertEqual(node.cpus, allowed_cpus)

    @skipUnless(len(topology.nodes) > 1, 'At least two NUMA nodes are required')
    def testSocketAffinityWithBlackList(self):
        # Pick the last node for deployment, but blacklist the first half of
        # its CPUs
        node = topology.nodes[-1]
        cpu_count = len(node.cpus)
        blacklist_cpus = node.cpus[:cpu_count/2]
        cpu_list = node.cpus[cpu_count/2:]

        self.comp.affinity.blacklist_cpus = ','.join(str(cpu) for cpu in blacklist_cpus)
        pid = self._deployWithAffinityOptions('socket_affinity_bl_1', {'affinity::exec_directive_class': 'socket',
                                                                       'affinity::exec_directive_value': node.node})
        allowed_cpus = self._getAllowedCpuList(pid)
        self.assertEqual(cpu_list, allowed_cpus)

    def testForceOverride(self):
        # Configure the GPP to always deploy to CPU 0
        self.comp.affinity.exec_directive_value = '0'
        self.comp.affinity.exec_directive_class = 'cpu'
        self.comp.affinity.force_override = True

        # Set a runtime affinity directive for CPU1; this should be ignored
        pid = self._deployWithAffinityOptions('force_override_1', {'affinity::exec_directive_class': 'cpu',
                                                                   'affinity::exec_directive_value': '1'})
        allowed_cpus = self._getAllowedCpuList(pid)
        self.assertEqual([0], allowed_cpus)

    def testDeployOnSocket(self):
        self.comp.affinity.deploy_per_socket = True

        pid0 = self._deployWithAffinityOptions('deploy_on_socket_1')
        allowed_cpus = self._getAllowedCpuList(pid0)
        self.assertEqual(topology.nodes[0].cpus, allowed_cpus)

        pid1 = self._deployWithAffinityOptions('deploy_on_socket_2')
        allowed_cpus = self._getAllowedCpuList(pid1)
        self.assertEqual(topology.nodes[0].cpus, allowed_cpus)


class DomainSupport(scatest.CorbaTestCase):
    """Test for all component implementations in test"""
    
    def _makeLink(self, src, dest):
        if os.path.exists(dest):
            os.unlink(dest)
        os.symlink(src, dest)

    def launchDomainManager(self, *args, **kwargs):
        domBooter, domMgr = super(DomainSupport,self).launchDomainManager(*args, loggingURI='', nodeBooterPath='nodeBooter', **kwargs)
        if domMgr is None:
            self.dom = None
        else:
            self.dom = redhawk.attach(domMgr._get_name())
        return domBooter, domMgr

    def launchDeviceManager(self, *args, **kwargs):
        return super(DomainSupport,self).launchDeviceManager(*args, loggingURI='', nodeBooterPath='nodeBooter', **kwargs)

    def setUp(self):
        super(DomainSupport,self).setUp()
        self.child_pids=[]
        self.orig_sdrroot=os.environ['SDRROOT']
        os.environ['SDRROOT'] = os.getcwd()+'/sdr'
        print "\n-----------------------"
        print "Running: ", self.id().split('.')[-1]
        print "-----------------------\n"
        self._makeLink(self.orig_sdrroot+'/dom/mgr', 'sdr/dom/mgr')
        self._makeLink(self.orig_sdrroot+'/dev/mgr', 'sdr/dev/mgr')
        print 'done staging DomainManager'

    def tearDown(self):
        super(DomainSupport, self).tearDown()
        try:
            # kill all busy.py just in case
            os.system('pkill -9 -f busy.py')
        except OSError:
            pass
        for child_p in self.child_pids:
            try:
                print "teardown (2)", child_p
                os.system('kill -9 '+str(child_p))
            except OSError:
                pass
        os.environ['SDRROOT'] = self.orig_sdrroot

class ComponentTests_SystemReservations(DomainSupport):
    def setUp(self):
        super(ComponentTests_SystemReservations,self).setUp()

    def tearDown(self):
        super(ComponentTests_SystemReservations, self).tearDown()

    def close(self, value_1, value_2, margin = 0.01):
        if (value_2 * (1-margin)) < value_1 and (value_2 * (1+margin)) > value_1:
            return True
        return False

    def float_eq(self, a,b,eps=0.0000001):
        return abs(a-b) < eps


    def testMonitorComponents(self):
        self.assertEquals(os.path.isfile('sdr/dom/mgr/DomainManager'),True)
        self.assertEquals(os.path.isfile('sdr/dev/mgr/DeviceManager'),True)
        self._domainBooter, domMgr = self.launchDomainManager()
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml")
        self.assertNotEquals(devMgr,None)
        app_1=self.dom.createApplication('/waveforms/load_comp_w/load_comp_w.sad.xml','load_comp_w',[])
        wait_amount = (self.dom.devMgrs[0].devs[0].threshold_cycle_time / 1000.0) * 6
        time.sleep(wait_amount)
        component_monitor = self.dom.devMgrs[0].devs[0].component_monitor[0]
        self.assertNotEqual(len(component_monitor), 0)
        self.assertEquals(component_monitor.num_processes, 2)
        self.assertTrue(component_monitor.cores > 0.75)
        self.assertTrue(component_monitor.cores < 1.75)
        app_1.start()
        time.sleep(wait_amount)
        component_monitor = self.dom.devMgrs[0].devs[0].component_monitor[0]
        self.assertEquals(component_monitor.num_processes, 2)
        self.assertTrue(component_monitor.cores > 1.75)
        app_1.stop()
        time.sleep(wait_amount)
        component_monitor = self.dom.devMgrs[0].devs[0].component_monitor[0]
        self.assertEquals(component_monitor.num_processes, 2)
        self.assertTrue(component_monitor.cores > 0.75)
        self.assertTrue(component_monitor.cores < 1.75)

    def testDeadlock(self):
        self.assertEquals(os.path.isfile('sdr/dom/mgr/DomainManager'),True)
        self.assertEquals(os.path.isfile('sdr/dev/mgr/DeviceManager'),True)
        self._domainBooter, domMgr = self.launchDomainManager()
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml")
        self.assertNotEquals(devMgr,None)
        self.dom.devMgrs[0].devs[0].threshold_cycle_time = 50
        count = 0
        while count < 20:
            try:
               app_1=self.dom.createApplication('/waveforms/empty_comp_w/empty_comp_w.sad.xml','empty_comp_w',[])
               app_1.releaseObject()
               time.sleep(0.1)
            except:
               pass
            count += 1
    
    def testSystemReservation(self):
        self.assertEquals(os.path.isfile('sdr/dom/mgr/DomainManager'),True)
        self.assertEquals(os.path.isfile('sdr/dev/mgr/DeviceManager'),True)
        self._domainBooter, domMgr = self.launchDomainManager()
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml")
        self.assertNotEquals(devMgr,None)
        self.comp= self.dom.devMgrs[0].devs[0]
        cpus = self.dom.devMgrs[0].devs[0].processor_cores
        cpu_thresh = self.dom.devMgrs[0].devs[0].thresholds.cpu_idle
        res_per_comp = self.dom.devMgrs[0].devs[0].reserved_capacity_per_component
        idle_cap_mod = 100.0  * res_per_comp / (cpus*1.0)
        upper_capacity = cpus - (cpus * (cpu_thresh/100))
        wait_amount = (self.dom.devMgrs[0].devs[0].threshold_cycle_time / 1000.0) * 4
        time.sleep(wait_amount)
        self.assertEquals(self.close(upper_capacity, self.dom.devMgrs[0].devs[0].utilization[0]['maximum']), True)
        
        time.sleep(1)
        
        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        subscribed = base_util['subscribed']
        system_load_base = base_util['system_load']
        
        extra_reservation = 1
        _value=any.to_any(extra_reservation)
        _value._t=CORBA.TC_double
        app_1=self.dom.createApplication('/waveforms/busy_w/busy_w.sad.xml','busy_w',[CF.DataType(id='SPECIALIZED_CPU_RESERVATION',value=any.to_any([CF.DataType(id='busy_comp_1',value=_value)]))])
        time.sleep(wait_amount)
        
        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        system_load_now = base_util['system_load']
        sub_now = base_util['subscribed']
        comp_load = base_util['component_load']
        #print "After App1 Create subnow(sub) " , sub_now, " sys_load", system_load_now, " sys_load_base ", system_load_base, " comp_load ", comp_load, " subscribed(base) ", subscribed, " extra ", extra_reservation, " res per", res_per_comp, " idle cap mod ", idle_cap_mod 
        self.assertEquals(self.close(sub_now, extra_reservation), True)

        app_2=self.dom.createApplication('/waveforms/busy_w/busy_w.sad.xml','busy_w',[])
        time.sleep(wait_amount)
        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        system_load_now = base_util['system_load']
        sub_now = base_util['subscribed']
        sub_now_pre = base_util['subscribed']
        comp_load = base_util['component_load']
        #print "After App2 Create subnow(sub) " , sub_now, " sys_load", system_load_now, " sys_load_base ", system_load_base, " comp_load ", comp_load, " subscribed(base) ", subscribed, " extra ", extra_reservation, " res per", res_per_comp, " idle cap mod ", idle_cap_mod 
        self.assertEquals(self.close(sub_now, extra_reservation+res_per_comp), True)

        app_1.start()
        time.sleep(wait_amount)
        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        system_load_now = base_util['system_load']
        sub_now = base_util['subscribed']
        comp_load = base_util['component_load']
        #print "After App1 START subnow(sub) " , sub_now, " sys_load", system_load_now, " sys_load_base ", system_load_base, " comp_load ", comp_load, " subscribed(base) ", subscribed, " extra ", extra_reservation, " res per", res_per_comp, " idle cap mod ", idle_cap_mod 
        gpp_state =  self.comp._get_usageState()
        #print "state:", gpp_state
        if comp_load > extra_reservation :
            self.assertEqual(self.close(sub_now-comp_load,res_per_comp), True)
        else:
            self.assertEqual(self.close(sub_now, extra_reservation+res_per_comp), True)

        app_2.start()
        time.sleep(wait_amount)
        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        system_load_now = base_util['system_load']
        sub_now = base_util['subscribed']
        comp_load = base_util['component_load']
        #print "After App2 START subnow(sub) " , sub_now, " sys_load", system_load_now, " sys_load_base ", system_load_base, " comp_load ", comp_load, " subscribed(base) ", subscribed, " extra ", extra_reservation, " res per", res_per_comp, " idle cap mod ", idle_cap_mod 
        gpp_state =  self.comp._get_usageState()
        #print "state:", gpp_state
        # removed check, no way to correctly gauge load for app2 busy.py...


        app_1.stop()
        time.sleep(wait_amount)
        app_2.stop()
        time.sleep(wait_amount)
        time.sleep(wait_amount)
        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        system_load_now = base_util['system_load']
        sub_now = base_util['subscribed']
        comp_load = base_util['component_load']
        #print "After Stop Both subnow(sub) " , sub_now, " sys_load", system_load_now, " sys_load_base ", system_load_base, " comp_load ", comp_load, " subscribed(base) ", subscribed, " extra ", extra_reservation, " res per", res_per_comp, " idle cap mod ", idle_cap_mod 
        gpp_state =  self.comp._get_usageState()
        #print "state:", gpp_state
        self.assertEquals(self.close(sub_now, extra_reservation+res_per_comp ), True)
        self.assertEquals(self.float_eq(sub_now_pre, sub_now, eps=.01), True)

    def _verifyReservations(self, extra, application, wait_amount):
        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        system_load_now = base_util['system_load']
        sub_now = base_util['subscribed']
        comp_load = base_util['component_load']
        self.assertEquals(self.close(sub_now, extra), True)
        self.assertEquals(comp_load, 0)

        application.start()
        time.sleep(wait_amount)
        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        system_load_now = base_util['system_load']
        sub_now = base_util['subscribed']
        comp_load = base_util['component_load']
        self.assertEquals(self.close(sub_now, extra), True)
        self.assertEquals(self.close(comp_load, 2, margin=0.1), True)

        application.stop()
        time.sleep(wait_amount)
        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        system_load_now = base_util['system_load']
        sub_now = base_util['subscribed']
        comp_load = base_util['component_load']
        self.assertEquals(self.close(sub_now, extra), True)
        self.assertEquals(comp_load, 0)

    def testAppReservation(self):
        self.assertEquals(os.path.isfile('sdr/dom/mgr/DomainManager'),True)
        self.assertEquals(os.path.isfile('sdr/dev/mgr/DeviceManager'),True)
        self._domainBooter, domMgr = self.launchDomainManager()
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml")
        self.assertNotEquals(devMgr,None)
        self.comp= self.dom.devMgrs[0].devs[0]
        cpus = self.dom.devMgrs[0].devs[0].processor_cores
        cpu_thresh = self.dom.devMgrs[0].devs[0].thresholds.cpu_idle
        res_per_comp = self.dom.devMgrs[0].devs[0].reserved_capacity_per_component
        idle_cap_mod = 100.0  * res_per_comp / (cpus*1.0)
        upper_capacity = cpus - (cpus * (cpu_thresh/100))
        wait_amount = (self.dom.devMgrs[0].devs[0].threshold_cycle_time / 1000.0) * 4
        time.sleep(wait_amount)
        self.assertEquals(self.close(upper_capacity, self.dom.devMgrs[0].devs[0].utilization[0]['maximum']), True)

        time.sleep(1)

        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        subscribed = base_util['subscribed']
        system_load_base = base_util['system_load']

        extra_reservation = 3
        _value=any.to_any(extra_reservation)
        _value._t=CORBA.TC_double
        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, self.dom.createApplication, '/waveforms/wav_floor_w/wav_floor_w.sad.xml','busy_w',[CF.DataType(id='SPECIALIZED_CPU_RESERVATION',value=any.to_any([CF.DataType(id='busy_comp_1',value=_value)]))])
        app_1=self.dom.createApplication('/waveforms/wav_floor_w/wav_floor_w.sad.xml','busy_w',[])
        time.sleep(wait_amount)
        self._verifyReservations(extra_reservation, app_1, wait_amount)

        app_1.releaseObject()
        time.sleep(wait_amount)
        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        sub_now = base_util['subscribed']
        comp_load = base_util['component_load']
        self.assertEquals(sub_now, 0)

    def testAppOverloadGenericReservation(self):
        self.assertEquals(os.path.isfile('sdr/dom/mgr/DomainManager'),True)
        self.assertEquals(os.path.isfile('sdr/dev/mgr/DeviceManager'),True)
        self._domainBooter, domMgr = self.launchDomainManager()
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml")
        self.assertNotEquals(devMgr,None)
        self.comp= self.dom.devMgrs[0].devs[0]
        cpus = self.dom.devMgrs[0].devs[0].processor_cores
        cpu_thresh = self.dom.devMgrs[0].devs[0].thresholds.cpu_idle
        res_per_comp = self.dom.devMgrs[0].devs[0].reserved_capacity_per_component
        idle_cap_mod = 100.0  * res_per_comp / (cpus*1.0)
        upper_capacity = cpus - (cpus * (cpu_thresh/100))
        wait_amount = (self.dom.devMgrs[0].devs[0].threshold_cycle_time / 1000.0) * 4
        time.sleep(wait_amount)
        self.assertEquals(self.close(upper_capacity, self.dom.devMgrs[0].devs[0].utilization[0]['maximum']), True)

        time.sleep(1)

        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        subscribed = base_util['subscribed']
        system_load_base = base_util['system_load']

        extra_reservation = 4
        _value=any.to_any(extra_reservation)
        _value._t=CORBA.TC_double
        app_1=self.dom.createApplication('/waveforms/wav_floor_w/wav_floor_w.sad.xml','busy_w',[CF.DataType(id='SPECIALIZED_CPU_RESERVATION',value=any.to_any([CF.DataType(id='',value=_value)]))])
        time.sleep(wait_amount)
        self._verifyReservations(extra_reservation, app_1, wait_amount)

    def testAppOverloadSpecificReservation(self):
        self.assertEquals(os.path.isfile('sdr/dom/mgr/DomainManager'),True)
        self.assertEquals(os.path.isfile('sdr/dev/mgr/DeviceManager'),True)
        self._domainBooter, domMgr = self.launchDomainManager()
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml")
        self.assertNotEquals(devMgr,None)
        self.comp= self.dom.devMgrs[0].devs[0]
        cpus = self.dom.devMgrs[0].devs[0].processor_cores
        cpu_thresh = self.dom.devMgrs[0].devs[0].thresholds.cpu_idle
        res_per_comp = self.dom.devMgrs[0].devs[0].reserved_capacity_per_component
        idle_cap_mod = 100.0  * res_per_comp / (cpus*1.0)
        upper_capacity = cpus - (cpus * (cpu_thresh/100))
        wait_amount = (self.dom.devMgrs[0].devs[0].threshold_cycle_time / 1000.0) * 4
        time.sleep(wait_amount)
        self.assertEquals(self.close(upper_capacity, self.dom.devMgrs[0].devs[0].utilization[0]['maximum']), True)

        time.sleep(1)

        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        subscribed = base_util['subscribed']
        system_load_base = base_util['system_load']

        extra_reservation = 4
        _value=any.to_any(extra_reservation)
        _value._t=CORBA.TC_double
        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, self.dom.createApplication, '/waveforms/wav_floor_w/wav_floor_w.sad.xml','busy_w',[CF.DataType(id='SPECIALIZED_CPU_RESERVATION',value=any.to_any([CF.DataType(id='COLLOC_SET1',value=_value)]))])
        app_1=self.dom.createApplication('/waveforms/wav_floor_w/wav_floor_w.sad.xml','busy_w',[CF.DataType(id='SPECIALIZED_CPU_RESERVATION',value=any.to_any([CF.DataType(id='ID_TEST_SET1',value=_value)]))])
        time.sleep(wait_amount)
        self._verifyReservations(extra_reservation, app_1, wait_amount)

    def testAppOverloadTwoSpecificReservation(self):
        self.assertEquals(os.path.isfile('sdr/dom/mgr/DomainManager'),True)
        self.assertEquals(os.path.isfile('sdr/dev/mgr/DeviceManager'),True)
        self._domainBooter, domMgr = self.launchDomainManager()
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml")
        self.assertNotEquals(devMgr,None)
        self.comp= self.dom.devMgrs[0].devs[0]
        cpus = self.dom.devMgrs[0].devs[0].processor_cores
        cpu_thresh = self.dom.devMgrs[0].devs[0].thresholds.cpu_idle
        res_per_comp = self.dom.devMgrs[0].devs[0].reserved_capacity_per_component
        idle_cap_mod = 100.0  * res_per_comp / (cpus*1.0)
        upper_capacity = cpus - (cpus * (cpu_thresh/100))
        wait_amount = (self.dom.devMgrs[0].devs[0].threshold_cycle_time / 1000.0) * 4
        time.sleep(wait_amount)
        self.assertEquals(self.close(upper_capacity, self.dom.devMgrs[0].devs[0].utilization[0]['maximum']), True)

        time.sleep(1)

        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        subscribed = base_util['subscribed']
        system_load_base = base_util['system_load']

        extra_reservation = 4
        _value=any.to_any(extra_reservation/2)
        _value._t=CORBA.TC_double
        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, self.dom.createApplication, '/waveforms/wav_floor_w/wav_floor_w.sad.xml','busy_w',[CF.DataType(id='SPECIALIZED_CPU_RESERVATION',value=any.to_any([CF.DataType(id='COLLOC_SET1',value=any.to_any(_value)),CF.DataType(id='ID_TEST_SET2',value=_value)]))])
        app_1=self.dom.createApplication('/waveforms/wav_two_floor_w/wav_two_floor_w.sad.xml','busy_w',[CF.DataType(id='SPECIALIZED_CPU_RESERVATION',value=any.to_any([CF.DataType(id='ID_TEST_SET1',value=any.to_any(_value)),CF.DataType(id='ID_TEST_SET2',value=_value)]))])
        time.sleep(wait_amount)
        self._verifyReservations(extra_reservation, app_1, wait_amount)

    def testAppOverloadOneSpecificReservation(self):
        self.assertEquals(os.path.isfile('sdr/dom/mgr/DomainManager'),True)
        self.assertEquals(os.path.isfile('sdr/dev/mgr/DeviceManager'),True)
        self._domainBooter, domMgr = self.launchDomainManager()
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml")
        self.assertNotEquals(devMgr,None)
        self.comp= self.dom.devMgrs[0].devs[0]
        cpus = self.dom.devMgrs[0].devs[0].processor_cores
        cpu_thresh = self.dom.devMgrs[0].devs[0].thresholds.cpu_idle
        res_per_comp = self.dom.devMgrs[0].devs[0].reserved_capacity_per_component
        idle_cap_mod = 100.0  * res_per_comp / (cpus*1.0)
        upper_capacity = cpus - (cpus * (cpu_thresh/100))
        wait_amount = (self.dom.devMgrs[0].devs[0].threshold_cycle_time / 1000.0) * 4
        time.sleep(wait_amount)
        self.assertEquals(self.close(upper_capacity, self.dom.devMgrs[0].devs[0].utilization[0]['maximum']), True)

        time.sleep(1)

        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        subscribed = base_util['subscribed']
        system_load_base = base_util['system_load']

        extra_reservation = 4
        _value=any.to_any(extra_reservation/2)
        _value._t=CORBA.TC_double
        app_1=self.dom.createApplication('/waveforms/wav_one_floor_w/wav_one_floor_w.sad.xml','busy_w',[CF.DataType(id='SPECIALIZED_CPU_RESERVATION',value=any.to_any([CF.DataType(id='ID_TEST_SET1',value=any.to_any(_value)),CF.DataType(id='ID_TEST_SET2',value=_value)]))])
        time.sleep(wait_amount)
        self._verifyReservations(extra_reservation, app_1, wait_amount)


class LoadableDeviceVariableDirectoriesTest(DomainSupport):
    def setUp(self):
        super(LoadableDeviceVariableDirectoriesTest,self).setUp()
        self.launchDomainManager()
        self._testFiles = []

        fp = open('sdr/dev/nodes/test_VarCache_node/DeviceManager.dcd.xml', 'r')
        self.original = fp.read()
        fp.close()

        cwd = os.getcwd()
        self.base_dir = cwd + '/LoadableDeviceVariableDirectoriesTest'
        self.cache_dir = self.base_dir+'/cache'
        self.cwd_dir = self.base_dir+'/cwd'
        modified = self.original.replace('@@@CACHE_DIRECTORY@@@', self.cache_dir)
        modified = modified.replace('@@@CURRENT_WORKING_DIRECTORY@@@', self.cwd_dir)

        fp = open('sdr/dev/nodes/test_VarCache_node/DeviceManager.dcd.xml', 'w')
        fp.write(modified)
        fp.close()

    def tearDown(self):
        fp = open('sdr/dev/nodes/test_VarCache_node/DeviceManager.dcd.xml', 'w')
        fp.write(self.original)
        fp.close()

        super(LoadableDeviceVariableDirectoriesTest, self).tearDown()
        for file in self._testFiles:
            os.unlink(file)

        shutil.rmtree(self.base_dir)

    def test_CheckDEPLOYMENTROOT(self):
        self.assertNotEqual(self.dom, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_VarCache_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        app = self.dom.createApplication('/waveforms/check_cwd_w/check_cwd_w.sad.xml')
        self.assertNotEqual(app, None)
        self.assertEquals(app.comps[0].cwd, self.cwd_dir)
        pid = str(app._get_componentProcessIds()[0].processId)
        fp=open('/proc/'+pid+'/cmdline','r')
        cmdline = fp.read()
        fp.close()
        _args = cmdline.split('\x00')
        idx = _args.index('RH::DEPLOYMENT_ROOT')
        deployment_root=_args[idx+1]
        self.assertEquals(deployment_root, self.dom.devices[0].cacheDirectory)

    def test_CompConfigCacheCWD(self):
        self.assertNotEqual(self.dom, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_VarCache_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        app = self.dom.createApplication('/waveforms/check_cwd_w/check_cwd_w.sad.xml')
        self.assertNotEqual(app, None)
        self.assertEquals(app.comps[0].cwd, self.cwd_dir)
        found_dir = False
        for root, dirs, files in os.walk(self.base_dir):
            if 'check_cwd.py' in files:
                if 'cache/components/check_cwd/python' in root:
                    found_dir = True
                    break
        self.assertTrue(found_dir)

class LoadableDeviceVariableCacheDirTest(DomainSupport):
    def setUp(self):
        super(LoadableDeviceVariableCacheDirTest,self).setUp()
        self.launchDomainManager()
        
        fp = open('sdr/dev/nodes/test_VarCacheOnly_node/DeviceManager.dcd.xml', 'r')
        self.original = fp.read()
        fp.close()
        
        cwd = os.getcwd()
        self.base_dir = cwd + '/LoadableDeviceVariableDirectoriesTest'
        self.cache_dir = self.base_dir+'/cache'
        self.cwd_dir = self.cache_dir
        modified = self.original.replace('@@@CACHE_DIRECTORY@@@', self.cache_dir)
        
        fp = open('sdr/dev/nodes/test_VarCacheOnly_node/DeviceManager.dcd.xml', 'w')
        fp.write(modified)
        fp.close()

    def tearDown(self):
        fp = open('sdr/dev/nodes/test_VarCacheOnly_node/DeviceManager.dcd.xml', 'w')
        fp.write(self.original)
        fp.close()
        
        super(LoadableDeviceVariableCacheDirTest, self).tearDown()
        
        shutil.rmtree(self.base_dir)
            
    def test_CompConfigCache(self):
        self.assertNotEqual(self.dom, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_VarCacheOnly_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        app = self.dom.createApplication('/waveforms/check_cwd_w/check_cwd_w.sad.xml')
        self.assertNotEqual(app, None)
        self.assertEquals(app.comps[0].cwd, self.cwd_dir)
        found_dir = False
        for root, dirs, files in os.walk(self.base_dir):
            if 'check_cwd.py' in files:
                if 'cache/components/check_cwd/python' in root:
                    found_dir = True
        self.assertEquals(found_dir, True)

class LoadableDeviceVariableCWDTest(DomainSupport):
    def setUp(self):
        super(LoadableDeviceVariableCWDTest,self).setUp()
        self.launchDomainManager()
        
        fp = open('sdr/dev/nodes/test_VarCWDOnly_node/DeviceManager.dcd.xml', 'r')
        self.original = fp.read()
        fp.close()
        
        cwd = os.getcwd()
        self.base_dir = cwd + '/LoadableDeviceVariableDirectoriesTest'
        self.cwd_dir = self.base_dir+'/cwd'
        modified = self.original.replace('@@@CURRENT_WORKING_DIRECTORY@@@', self.cwd_dir)
        
        fp = open('sdr/dev/nodes/test_VarCWDOnly_node/DeviceManager.dcd.xml', 'w')
        fp.write(modified)
        fp.close()

    def tearDown(self):
        fp = open('sdr/dev/nodes/test_VarCWDOnly_node/DeviceManager.dcd.xml', 'w')
        fp.write(self.original)
        fp.close()
        
        super(LoadableDeviceVariableCWDTest, self).tearDown()
        
        shutil.rmtree(self.base_dir)
            
    def test_CompConfigCWD(self):
        self.assertNotEqual(self.dom, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_VarCWDOnly_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)
        app = self.dom.createApplication('/waveforms/check_cwd_w/check_cwd_w.sad.xml')
        self.assertNotEqual(app, None)
        self.assertEquals(app.comps[0].cwd, self.cwd_dir)
        found_dir = False
        for root, dirs, files in os.walk(self.base_dir):
            if 'check_cwd.py' in files:
                if 'cwd/components/check_cwd/python' in root:
                    found_dir = True
        self.assertEquals(found_dir, True)


if __name__ == "__main__":
    if False:
        # Debugging support: enable this conditional to dump NUMA topology
        print "NumaSupport %d nodes %d CPUs" % (len(topology.nodes), len(topology.cpus))
        for node in topology.nodes:
            print 'Node', node.node, 'CPUs:', node.cpus
    ossie.utils.testing.main("../GPP.spd.xml") # By default tests all implementations
