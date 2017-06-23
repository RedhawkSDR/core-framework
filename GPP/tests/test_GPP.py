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
from omniORB import any
from ossie.cf import ExtendedEvent
from ossie.parsers import DCDParser
from omniORB import CORBA
import omniORB
import CosEventChannelAdmin, CosEventChannelAdmin__POA
from ossie.utils.sandbox.registrar import ApplicationRegistrarStub
import subprocess, multiprocessing
from ossie.utils import sb, redhawk
from ossie.cf import CF, CF__POA
import ossie.utils.testing
from shutil import copyfile
import os
import shutil

# numa layout: node 0 cpus, node 1 cpus, node 0 cpus sans cpuid=0

maxcpus=32
maxnodes=2
all_cpus='0-'+str(maxcpus-1)
all_cpus_sans0='1-'+str(maxcpus-1)
numa_match={ "all" : "0-31",
             "sock0": "0-7,16-23",
             "sock1": "8-15,24-31", 
             "sock0sans0": "1-7,16-23", 
             "sock1sans0": "1-7,16-23", 
             "5" : "5",
             "8-10" : "8-10" }
numa_layout=[ "0-7,16-23", "8-15,24-31" ]

affinity_test_src={ "all" : "0-31",
                 "sock0": "0",
                 "sock1": "1", 
                 "sock0sans0": "0", 
                 "5" : "5",
                 "8-10" : "8,9,10",
                 "eface" : "em1" }

def get_match( key="all" ):
    if key and  key in numa_match:
        return numa_match[key]
    return numa_match["all"]

def spawnNodeBooter(dmdFile=None, 
                    dcdFile=None, 
                    debug=0, 
                    domainname=None, 
                    loggingURI=None, 
                    endpoint=None, 
                    dbURI=None, 
                    execparams="", 
                    nodeBooterPath=os.getenv('OSSIEHOME')+"/bin/nodeBooter",
                    sdrroot = None):
    args = []
    if dmdFile != None:
        args.extend(["-D", dmdFile])
    if dcdFile != None:
        args.extend(["-d", dcdFile])
    if domainname == None:
        # Always use the --domainname argument because
        # we don't want to have to read the DCD files or regnerate them
        args.extend(["--domainname", 'sample_domain'])
    else:
        args.extend(["--domainname", domainname])

    if endpoint == None:
        args.append("--nopersist")
    else:
        args.extend(["-ORBendPoint", endpoint])

    if dbURI:
        args.extend(["--dburl", dbURI])
    
    if sdrroot == None:
        sdrroot = os.getenv('SDRROOT')

    args.extend(["-debug", str(debug)])
    args.extend(execparams.split(" "))
    args.insert(0, nodeBooterPath)

    print '\n-------------------------------------------------------------------'
    print 'Launching nodeBooter', " ".join(args)
    print '-------------------------------------------------------------------'
    nb = ossie.utils.Popen(args, cwd=sdrroot, shell=False, preexec_fn=os.setpgrp)

    return nb

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

    def check_affinity(self, pname, affinity_match="0-31", use_pidof=True, pid_in=None):
        try:
            if pid_in:
                pid=pid_in
                o2=os.popen('cat /proc/'+str(pid)+'/status | grep Cpus_allowed_list')
            else:
                if use_pidof == True:
                    o1=os.popen('pidof -x '+pname )
                else:
                    o1=os.popen('pgrep -f '+pname )
                pid=o1.read()
                o2=os.popen('cat /proc/'+pid.split('\n')[0]+'/status | grep Cpus_allowed_list')
            cpus_allowed=o2.read().split()
        except:
            cpus_allowed=[]

        #print pname, cpus_allowed
        self.assertEqual(cpus_allowed[1],affinity_match)
        return

        
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
        self.comp.thresholds.cpu_idle = -1.0
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
        self.comp.thresholds.load_avg = -1.0
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



    def DeployWithAffinityOptions(self, options_list, numa_layout_test, bl_cpus ):
        self.runGPP()

        # enable affinity processing..
        props=[ossie.cf.CF.DataType(id='affinity', value=CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), 
                       [ ossie.cf.CF.DataType(id='affinity::exec_directive_value', value=CORBA.Any(CORBA.TC_string, '')), 
                         ossie.cf.CF.DataType(id='affinity::exec_directive_class', value=CORBA.Any(CORBA.TC_string, 'socket')), 
                         ossie.cf.CF.DataType(id='affinity::force_override', value=CORBA.Any(CORBA.TC_boolean, False)), 
                         ossie.cf.CF.DataType(id='affinity::blacklist_cpus', value=CORBA.Any(CORBA.TC_string, bl_cpus)), 
                         ossie.cf.CF.DataType(id='affinity::deploy_per_socket', value=CORBA.Any(CORBA.TC_boolean, False)), 
                         ossie.cf.CF.DataType(id='affinity::disabled', value=CORBA.Any(CORBA.TC_boolean, False))  ## enable affinity
                       ] ))]

        self.comp_obj.configure(props)

        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.IDLE)
        
        fs_stub = ComponentTests.FileSystemStub()
        fs_stub_var = fs_stub._this()

        ## Run a component with NIC based affinity
        self.comp_obj.load(fs_stub_var, "/component_stub.py", CF.LoadableDevice.EXECUTABLE)
        self.assertEqual(os.path.isfile("component_stub.py"), True) # Technically this is an internal implementation detail that the file is loaded into the CWD of the device
        
        comp_id = "DCE:00000000-0000-0000-0000-000000000000:waveform_1"
        app_id = "waveform_1"
        appReg = ApplicationRegistrarStub(comp_id, app_id)
        appreg_ior = sb.orb.object_to_string(appReg._this())
        pid = self.comp_obj.execute("/component_stub.py", [
                CF.DataType(id="AFFINITY", value=any.to_any( options_list ) ) ],
                [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id)), 
                 CF.DataType(id="NAME_BINDING", value=any.to_any("component_stub")),CF.DataType(id="PROFILE_NAME", value=any.to_any("/component_stub/component_stub.spd.xml")),
                 CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))])
        self.assertNotEqual(pid, 0)

        self.check_affinity( 'component_stub.py', get_match(numa_layout_test), False)
        
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


    def testNicAffinity(self):
        self.DeployWithAffinityOptions( [ CF.DataType(id='nic',value=any.to_any(affinity_test_src['eface'])) ], "sock0", '' )

    def testNicAffinityWithBlackList(self):
        self.DeployWithAffinityOptions( [ CF.DataType(id='nic',value=any.to_any(affinity_test_src['eface'])) ], "sock0sans0", '0' )

    def testCpuAffinity(self):
        if maxcpus > 6:
            self.DeployWithAffinityOptions( [ CF.DataType(id='affinity::exec_directive_class',value=any.to_any('cpu')),
                                              CF.DataType(id='affinity::exec_directive_value',value=any.to_any(affinity_test_src['5'])) ], "5", '' )

    def testSocketAffinity(self):
        self.DeployWithAffinityOptions( [ CF.DataType(id='affinity::exec_directive_class',value=any.to_any('socket')),
                               CF.DataType(id='affinity::exec_directive_value',value=any.to_any(affinity_test_src['sock1'])) ], 
                                        "sock1sans0", '0' )

    def testDeployOnSocket(self):
        self.runGPP()

        # enable affinity processing..
        props=[ossie.cf.CF.DataType(id='affinity', value=CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), 
                       [ ossie.cf.CF.DataType(id='affinity::exec_directive_value', value=CORBA.Any(CORBA.TC_string, '')), 
                         ossie.cf.CF.DataType(id='affinity::exec_directive_class', value=CORBA.Any(CORBA.TC_string, 'socket')), 
                         ossie.cf.CF.DataType(id='affinity::force_override', value=CORBA.Any(CORBA.TC_boolean, False)), 
                         ossie.cf.CF.DataType(id='affinity::blacklist_cpus', value=CORBA.Any(CORBA.TC_string, '')), 
                         ossie.cf.CF.DataType(id='affinity::deploy_per_socket', value=CORBA.Any(CORBA.TC_boolean, True)),   ## enable deploy_on 
                         ossie.cf.CF.DataType(id='affinity::disabled', value=CORBA.Any(CORBA.TC_boolean, False))  ## enable affinity
                       ] ))]

        self.comp_obj.configure(props)

        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.IDLE)
        
        fs_stub = ComponentTests.FileSystemStub()
        fs_stub_var = fs_stub._this()

        ## Run a component with NIC based affinity
        self.comp_obj.load(fs_stub_var, "/component_stub.py", CF.LoadableDevice.EXECUTABLE)
        self.assertEqual(os.path.isfile("component_stub.py"), True) # Technically this is an internal implementation detail that the file is loaded into the CWD of the device
        
        comp_id = "DCE:00000000-0000-0000-0000-000000000000:waveform_1"
        app_id = "waveform_1"
        appReg = ApplicationRegistrarStub(comp_id, app_id)
        appreg_ior = sb.orb.object_to_string(appReg._this())
        pid0 = self.comp_obj.execute("/component_stub.py", [],
                [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id)), 
                 CF.DataType(id="NAME_BINDING", value=any.to_any("component_stub")),CF.DataType(id="PROFILE_NAME", value=any.to_any("/component_stub/component_stub.spd.xml")),
                 CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))])
        self.assertNotEqual(pid0, 0)

        comp_id = "DCE:00000000-0000-0000-0000-000000000001:waveform_1"
        app_id = "waveform_1"
        appReg = ApplicationRegistrarStub(comp_id, app_id)
        appreg_ior = sb.orb.object_to_string(appReg._this())
        pid1 = self.comp_obj.execute("/component_stub.py", [],
                [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id)), 
                 CF.DataType(id="NAME_BINDING", value=any.to_any("component_stub")),CF.DataType(id="PROFILE_NAME", value=any.to_any("/component_stub/component_stub.spd.xml")),
                 CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))])

        self.assertNotEqual(pid1, 0)

        self.check_affinity( 'component_stub.py', get_match("sock0"), False, pid0)
        self.check_affinity( 'component_stub.py', get_match("sock0"), False, pid1)
        
        for pid in [ pid0, pid1 ]:
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

    def testForceOverride(self):
        self.runGPP()

        # enable affinity processing..
        props=[ossie.cf.CF.DataType(id='affinity', value=CORBA.Any(CORBA.TypeCode("IDL:CF/Properties:1.0"), 
                       [ ossie.cf.CF.DataType(id='affinity::exec_directive_value', value=CORBA.Any(CORBA.TC_string, '1')), 
                         ossie.cf.CF.DataType(id='affinity::exec_directive_class', value=CORBA.Any(CORBA.TC_string, 'socket')), 
                         ossie.cf.CF.DataType(id='affinity::force_override', value=CORBA.Any(CORBA.TC_boolean, True)), 
                         ossie.cf.CF.DataType(id='affinity::blacklist_cpus', value=CORBA.Any(CORBA.TC_string, '')), 
                         ossie.cf.CF.DataType(id='affinity::deploy_per_socket', value=CORBA.Any(CORBA.TC_boolean, True)), 
                         ossie.cf.CF.DataType(id='affinity::disabled', value=CORBA.Any(CORBA.TC_boolean, False))  ## enable affinity
                       ] ))]

        self.comp_obj.configure(props)

        self.assertEqual(self.comp_obj._get_usageState(), CF.Device.IDLE)
        
        fs_stub = ComponentTests.FileSystemStub()
        fs_stub_var = fs_stub._this()

        ## Run a component with NIC based affinity
        self.comp_obj.load(fs_stub_var, "/component_stub.py", CF.LoadableDevice.EXECUTABLE)
        self.assertEqual(os.path.isfile("component_stub.py"), True) # Technically this is an internal implementation detail that the file is loaded into the CWD of the device
        
        comp_id = "DCE:00000000-0000-0000-0000-000000000000:waveform_1"
        app_id = "waveform_1"
        appReg = ApplicationRegistrarStub(comp_id, app_id)
        appreg_ior = sb.orb.object_to_string(appReg._this())
        pid0 = self.comp_obj.execute("/component_stub.py", [],
                [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id)), 
                 CF.DataType(id="NAME_BINDING", value=any.to_any("component_stub")),CF.DataType(id="PROFILE_NAME", value=any.to_any("/component_stub/component_stub.spd.xml")),
                 CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))])
        self.assertNotEqual(pid0, 0)

        comp_id = "DCE:00000000-0000-0000-0000-000000000001:waveform_1"
        app_id = "waveform_1"
        appReg = ApplicationRegistrarStub(comp_id, app_id)
        appreg_ior = sb.orb.object_to_string(appReg._this())
        pid1 = self.comp_obj.execute("/component_stub.py", [],
                [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id)), 
                 CF.DataType(id="NAME_BINDING", value=any.to_any("component_stub")),CF.DataType(id="PROFILE_NAME", value=any.to_any("/component_stub/component_stub.spd.xml")),
                 CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))])

        self.assertNotEqual(pid1, 0)

        self.check_affinity( 'component_stub.py',get_match("sock1"), False, pid0)
        self.check_affinity( 'component_stub.py',get_match("sock1"), False, pid1)
        
        for pid in [ pid0, pid1 ]:
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

class DomainSupport(ossie.utils.testing.ScaComponentTestCase):
    """Test for all component implementations in test"""
    child_pids = []
    dom = None
    _domainBooter = None
    _domainManager = None
    _deviceBooter = None
    _deviceLock = threading.Lock()
    _deviceBooters = []
    _deviceManagers = []
    sdrroot = ''
    
    def _getDeviceManager(self, domMgr, id):
        for devMgr in domMgr._get_deviceManagers():
            try:
                if id == devMgr._get_identifier():
                    return devMgr
            except CORBA.Exception:
                # The DeviceManager being checked is unreachable.
                pass
        return None
    
    def waitTermination(self, child, timeout=5.0, pause=0.1):
        while child.poll() is None and timeout > 0.0:
            timeout -= pause
            time.sleep(pause)
        return child.poll() != None

    def terminateChild(self, child, signals=(signal.SIGINT, signal.SIGTERM)):
        if child.poll() != None:
           return
        try:
            for sig in signals:
                os.kill(child.pid, sig)
                if self.waitTermination(child):
                    break
            child.wait()
        except OSError, e:
            pass
        finally:
            pass

    def launchDomainManager(self, dmdFile="", domain_name = '', sdrroot=os.getcwd()+'/sdr', *args, **kwargs):
        # Only allow one DomainManager, although this isn't a hard requirement.
        # If it has exited, allow a relaunch.
        if self._domainBooter and self._domainBooter.poll() == None:
            return (self._domainBooter, self._domainManager)
        self.sdrroot = sdrroot

        # Launch the nodebooter.
        self._domainBooter = spawnNodeBooter(dmdFile=dmdFile, domainname = domain_name, sdrroot=sdrroot, *args, **kwargs)
        number_attempts = 0
        while self._domainBooter.poll() == None:
            try:
                self.dom = redhawk.attach(domain_name)
            except:
                number_attempts += 1
                if number_attempts >= 20:
                    raise
                time.sleep(0.1)
                continue
            self._domainManager = self.dom.ref
            if self._domainManager:
                try:
                    self._domainManager._get_identifier()
                    break
                except:
                    pass
        return (self._domainBooter, self._domainManager)

    def _addDeviceBooter(self, devBooter):
        self._deviceLock.acquire()
        try:
            self._deviceBooters.append(devBooter)
        finally:
            self._deviceLock.release()

    def _addDeviceManager(self, devMgr):
        self._deviceLock.acquire()
        try:
            self._deviceManagers.append(devMgr)
        finally:
            self._deviceLock.release()

    def launchDeviceManager(self, dcdFile, domainManager=None, wait=True, sdrroot=os.getcwd()+'/sdr', *args, **kwargs):
        if not os.path.isfile(sdrroot+'/dev'+dcdFile):
            print "ERROR: Invalid DCD path provided to launchDeviceManager ", dcdFile
            return (None, None)
        self.sdrroot = sdrroot

        # Launch the nodebooter.
        if domainManager == None:
            name = None
        else:
            name = domainManager._get_name()
        devBooter = spawnNodeBooter(dcdFile=sdrroot+'/dev'+dcdFile, domainname=name, sdrroot=sdrroot, *args, **kwargs)
        self._addDeviceBooter(devBooter)

        if wait:
            devMgr = self.waitDeviceManager(devBooter, dcdFile, domainManager)
        else:
            devMgr = None

        return (devBooter, devMgr)

    def waitDeviceManager(self, devBooter, dcdFile, domainManager=None):
        try:
            dcdPath = self.sdrroot+'/dev'+dcdFile
        except IOError:
            print "ERROR: Invalid DCD path provided to waitDeviceManager", dcdFile
            return None

        # Parse the DCD file to get the identifier and number of devices, which can be
        # determined from the number of componentplacement elements.
        dcd = DCDParser.parse(dcdPath)
        if dcd.get_partitioning():
            numDevices = len(dcd.get_partitioning().get_componentplacement())
        else:
            numDevices = 0

        # Allow the caller to override the DomainManager (assuming they have a good reason).
        if not domainManager:
            domainManager = self._domainManager

        # As long as the nodebooter process is still alive, keep checking for the
        # DeviceManager.
        devMgr = None
        while devBooter.poll() == None:
            devMgrs = self.dom.devMgrs
            for dM in devMgrs:
                if dcd.get_id() == dM._get_identifier():
                    devMgr = dM.ref
            #devMgr = self._getDeviceManager(domainManager, dcd.get_id())
            if devMgr:
                break
            time.sleep(0.1)

        if devMgr:
            self._waitRegisteredDevices(devMgr, numDevices)
            self._addDeviceManager(devMgr)
        return devMgr

    def _waitRegisteredDevices(self, devMgr, numDevices, timeout=5.0, pause=0.1):
        while timeout > 0.0:
            if (len(devMgr._get_registeredDevices())+len(devMgr._get_registeredServices())) == numDevices:
                return True
            else:
                timeout -= pause
                time.sleep(pause)
        return False

    def _makeLink(self, src, dest):
        if os.path.exists(dest):
            os.unlink(dest)
        os.symlink(src, dest)

    def setUp(self):
        super(DomainSupport,self).setUp()
        self.child_pids=[]
        self._domainBooter = None
        self._domainManager = None
        self._deviceBooter = None
        self.orig_sdrroot=os.getenv('SDRROOT')
        os.putenv('SDRROOT', os.getcwd()+'/sdr')
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
        if self.dom != None:
            time.sleep(1)
            self.dom.terminate()
            self.dom = None
            self.terminateChild(self._domainBooter)
        if self._domainBooter:
            self.terminateChild(self._domainBooter)
        if self._deviceBooter:
            self.terminateChild(self._deviceBooter)
        os.putenv('SDRROOT', self.orig_sdrroot)

class ComponentTests_SystemReservations(DomainSupport):
    def setUp(self):
        super(ComponentTests_SystemReservations,self).setUp()

    def tearDown(self):
        super(ComponentTests_SystemReservations, self).tearDown()

    def runGPP(self, execparam_overrides={}, initialize=True):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        execparams.update(execparam_overrides)
        #execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        #execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        #self.launch(execparams, debugger='valgrind')
        self.launch(execparams, initialize=initialize )
        
        #######################################################################
        # Verify the basic state of the component
        self.assertNotEqual(self.comp_obj, None)
        self.assertEqual(self.comp_obj._non_existent(), False)
        self.assertEqual(self.comp_obj._is_a("IDL:CF/ExecutableDevice:1.0"), True)
        #self.assertEqual(self.spd.get_id(), self.comp_obj._get_identifier())
        
    def close(self, value_1, value_2, margin = 0.01):
        if (value_2 * (1-margin)) < value_1 and (value_2 * (1+margin)) > value_1:
            return True
        return False

    def float_eq(self, a,b,eps=0.0000001):
        return abs(a-b) < eps


    def testMonitorComponents(self):
        self.assertEquals(os.path.isfile('sdr/dom/mgr/DomainManager'),True)
        self.assertEquals(os.path.isfile('sdr/dev/mgr/DeviceManager'),True)
        self._domainBooter, domMgr = self.launchDomainManager(domain_name='REDHAWK_TEST_'+str(os.getpid()))
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml", domainManager=self.dom.ref)
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
        self._domainBooter, domMgr = self.launchDomainManager(domain_name='REDHAWK_TEST_'+str(os.getpid()))
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml", domainManager=self.dom.ref)
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
        self._domainBooter, domMgr = self.launchDomainManager(domain_name='REDHAWK_TEST_'+str(os.getpid()))
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml", domainManager=self.dom.ref)
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
        self._domainBooter, domMgr = self.launchDomainManager(domain_name='REDHAWK_TEST_'+str(os.getpid()))
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml", domainManager=self.dom.ref)
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
        self._domainBooter, domMgr = self.launchDomainManager(domain_name='REDHAWK_TEST_'+str(os.getpid()))
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml", domainManager=self.dom.ref)
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
        self._domainBooter, domMgr = self.launchDomainManager(domain_name='REDHAWK_TEST_'+str(os.getpid()))
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml", domainManager=self.dom.ref)
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
        self._domainBooter, domMgr = self.launchDomainManager(domain_name='REDHAWK_TEST_'+str(os.getpid()))
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml", domainManager=self.dom.ref)
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
        self._domainBooter, domMgr = self.launchDomainManager(domain_name='REDHAWK_TEST_'+str(os.getpid()))
        self.assertNotEquals(domMgr,None)
        self._deviceBooter, devMgr = self.launchDeviceManager("/nodes/DevMgr_sample/DeviceManager.dcd.xml", domainManager=self.dom.ref)
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
        self._domainName = 'REDHAWK_TEST_'+str(os.getpid())
        self._domainBooter, self._domMgr = self.launchDomainManager(domain_name=self._domainName)
        self._testFiles = []
        self._rhDom = redhawk.attach(self._domainName)

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
        self.assertNotEqual(self._domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_VarCache_node/DeviceManager.dcd.xml", domainManager=self.dom.ref)
        self.assertNotEqual(devMgr, None)
        app = self._rhDom.createApplication('/waveforms/check_cwd_w/check_cwd_w.sad.xml')
        self.assertNotEqual(app, None)
        self.assertEquals(app.comps[0].cwd, self.cwd_dir)
        pid = str(app._get_componentProcessIds()[0].processId)
        fp=open('/proc/'+pid+'/cmdline','r')
        cmdline = fp.read()
        fp.close()
        _args = cmdline.split('\x00')
        idx = _args.index('RH::DEPLOYMENT_ROOT')
        deployment_root=_args[idx+1]
        self.assertEquals(deployment_root, self._rhDom.devices[0].cacheDirectory)

    def test_PyCompConfigCacheCWD(self):
        self.assertNotEqual(self._domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_VarCache_node/DeviceManager.dcd.xml", domainManager=self.dom.ref)
        self.assertNotEqual(devMgr, None)
        app = self._rhDom.createApplication('/waveforms/check_cwd_w/check_cwd_w.sad.xml')
        self.assertNotEqual(app, None)
        self.assertEquals(app.comps[0].cwd, self.cwd_dir)
        found_dir = False
        for root, dirs, files in os.walk(self.base_dir):
            if 'check_cwd.py' in files:
                if 'cache/components/check_cwd/python' in root:
                    found_dir = True
                    break
        self.assertEquals(found_dir, True)
        
    def test_CppCompConfigCacheCWD(self):
        self.assertNotEqual(self._domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_VarCache_node/DeviceManager.dcd.xml", domainManager=self.dom.ref)
        self.assertNotEqual(devMgr, None)
        app = self._rhDom.createApplication('/waveforms/check_cwd_cpp_w/check_cwd_cpp_w.sad.xml')
        self.assertNotEqual(app, None)
        self.assertEquals(app.comps[0].cwd, self.cwd_dir)
        found_dir = False
        for root, dirs, files in os.walk(self.base_dir):
            if 'check_cwd_cpp' in files:
                if 'cache/components/check_cwd_cpp/cpp' in root:
                    found_dir = True
                    break
        self.assertEquals(found_dir, True)
        
    def test_JavaCompConfigCacheCWD(self):
        self.assertNotEqual(self._domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_VarCache_node/DeviceManager.dcd.xml", domainManager=self.dom.ref)
        self.assertNotEqual(devMgr, None)
        app = self._rhDom.createApplication('/waveforms/check_cwd_java_w/check_cwd_java_w.sad.xml')
        self.assertNotEqual(app, None)
        self.assertEquals(app.comps[0].cwd[:-1], self.cwd_dir)
        found_dir = False
        for root, dirs, files in os.walk(self.base_dir):
            if 'check_cwd_java.class' in files:
                if 'cache/components/check_cwd_java/java/bin/check_cwd_java/java' in root:
                    found_dir = True
                    break
        self.assertEquals(found_dir, True)

class LoadableDeviceVariableCacheDirTest(DomainSupport):
    def setUp(self):
        super(LoadableDeviceVariableCacheDirTest,self).setUp()
        self._domainName = 'REDHAWK_TEST_'+str(os.getpid())
        self._domainBooter, self._domMgr = self.launchDomainManager(domain_name=self._domainName)
        self._testFiles = []
        self._rhDom = redhawk.attach(self._domainName)
        
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
        self.assertNotEqual(self._domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_VarCacheOnly_node/DeviceManager.dcd.xml", domainManager=self.dom.ref)
        self.assertNotEqual(devMgr, None)
        app = self._rhDom.createApplication('/waveforms/check_cwd_w/check_cwd_w.sad.xml')
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
        self._domainName = 'REDHAWK_TEST_'+str(os.getpid())
        self._domainBooter, self._domMgr = self.launchDomainManager(domain_name=self._domainName)
        self._testFiles = []
        self._rhDom = redhawk.attach(self._domainName)
        
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
        self.assertNotEqual(self._domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_VarCWDOnly_node/DeviceManager.dcd.xml", domainManager=self.dom.ref)
        self.assertNotEqual(devMgr, None)
        app = self._rhDom.createApplication('/waveforms/check_cwd_w/check_cwd_w.sad.xml')
        self.assertNotEqual(app, None)
        self.assertEquals(app.comps[0].cwd, self.cwd_dir)
        found_dir = False
        for root, dirs, files in os.walk(self.base_dir):
            if 'check_cwd.py' in files:
                if 'cwd/components/check_cwd/python' in root:
                    found_dir = True
        self.assertEquals(found_dir, True)


    # TODO Add additional tests here
    #
    # See:
    #   ossie.utils.testing.bulkio_helpers,
    #   ossie.utils.testing.bluefile_helpers
    # for modules that will assist with testing components with BULKIO ports

def get_nonnuma_affinity_ctx( affinity_ctx ):
    # test should run but affinity will be ignored
    import multiprocessing
    maxcpus=multiprocessing.cpu_count()
    maxnodes=1
    all_cpus='0-'+str(maxcpus-1)
    all_cpus_sans0='0-'+str(maxcpus-1)
    if maxcpus == 2:
        all_cpus_sans0='0-1'
    elif maxcpus == 1 :
        all_cpus='0'
        all_cpus_sans0=''

    numa_layout=[ all_cpus ]
    affinity_match={ "all" :  all_cpus,
             "sock0":  all_cpus,
             "sock1": all_cpus,
             "sock0sans0":  all_cpus_sans0,
             "sock1sans0":  all_cpus_sans0,
             "5" : all_cpus,
             "8-10" : all_cpus }

    affinity_ctx['maxcpus']=maxcpus
    affinity_ctx['maxnodes']=maxnodes
    affinity_ctx['all_cpus']=all_cpus
    affinity_ctx['all_cpus_sans0']=all_cpus_sans0
    affinity_ctx['numa_layout']=numa_layout
    affinity_ctx['affinity_match']=affinity_match

def get_numa_affinity_ctx( affinity_ctx ):
    # test numaclt --show .. look for cpu bind of 0,1 and cpu id atleast 31
    maxnode=0
    maxcpu=0
    lines = [line.rstrip() for line in os.popen('numactl --show')]
    for l in lines:
        if l.startswith('nodebind'):
            maxnode=int(l.split()[-1])
        if l.startswith('physcpubind'):
            maxcpu=int(l.split()[-1])

    maxcpus=maxcpu+1
    maxnodes=maxnode+1
    numa_layout=[]
    try:
      for i in range(maxnodes):
          xx = [line.rstrip() for line in open('/sys/devices/system/node/node'+str(i)+'/cpulist')]
          numa_layout.append(xx[0])
    except:
        pass

    all_cpus='0-'+str(maxcpus-1)
    all_cpus_sans0='1-'+str(maxcpus-1)
    if maxcpus == 2:
        all_cpus_sans0='1'
    elif maxcpus == 1 :
        all_cpus="0"
        all_cpus_sans0=''

    affinity_match = { "all":all_cpus,
                       "sock0":  all_cpus,
                       "sock1": all_cpus,
                       "sock0sans0":  all_cpus_sans0,
                       "sock1sans0":  all_cpus_sans0,
                       "5" : all_cpus,
                       "8-10" : all_cpus }

    if len(numa_layout) > 0:
        affinity_match["sock0"]=numa_layout[0]
        aa=numa_layout[0]
        if maxcpus > 2:
            affinity_match["sock0sans0"] = str(int(aa[0])+1)+aa[1:]

    if len(numa_layout) > 1:
        affinity_match["sock1"]=numa_layout[1]
        affinity_match["sock1sans0"]=numa_layout[1]

    if maxcpus > 5:
        affinity_match["5"]="5"

    if maxcpus > 11:
        affinity_match["8-10"]="8-10"

    if maxcpus == 2:
        affinity_match["5"] = all_cpus_sans0
        affinity_match["8-10"]= all_cpus_sans0

    affinity_ctx['maxcpus']=maxcpus
    affinity_ctx['maxnodes']=maxnodes
    affinity_ctx['all_cpus']=all_cpus
    affinity_ctx['all_cpus_sans0']=all_cpus_sans0
    affinity_ctx['numa_layout']=numa_layout
    affinity_ctx['affinity_match']=affinity_match

    
if __name__ == "__main__":
    # figure out numa layout, test numaclt --show ..
    all_cpus="0"
    maxnode=1
    maxcpu=1
    eface="em1"
    #
    # Figure out ethernet interface to use
    #
    lines = [line.rstrip() for line in os.popen('cat /proc/net/dev')]
    import re
    for l in lines[2:]:
        t1=l.split(':')[0].lstrip()
        if re.match('e.*', t1 ) :
            eface=t1
            break

    affinity_test_src['eface']=eface

    nonnuma_affinity_ctx={}
    get_nonnuma_affinity_ctx(nonnuma_affinity_ctx)
    numa_affinity_ctx={}
    get_numa_affinity_ctx(numa_affinity_ctx)

    # figure out if GPP has numa library dependency
    lines = [ line.rstrip() for line in os.popen('ldd ../cpp/GPP') ]
    numa=False
    for l in lines:
        if "libnuma" in l:
            numa=True

    if numa:
        print "NumaSupport ", numa_affinity_ctx
        maxcpus = numa_affinity_ctx['maxcpus']
        maxnodes = numa_affinity_ctx['maxnodes']
        all_cpus = numa_affinity_ctx['all_cpus']
        all_cpus_sans0 = numa_affinity_ctx['all_cpus_sans0']
        numa_layout=numa_affinity_ctx['numa_layout']
        numa_match=numa_affinity_ctx['affinity_match']
    else:
        print "NonNumaSupport ", nonnuma_affinity_ctx
        maxcpus = nonnuma_affinity_ctx['maxcpus']
        maxnodes = nonnuma_affinity_ctx['maxnodes']
        all_cpus = nonnuma_affinity_ctx['all_cpus']
        all_cpus_sans0 = nonnuma_affinity_ctx['all_cpus_sans0']
        numa_layout=nonnuma_affinity_ctx['numa_layout']
        numa_match=nonnuma_affinity_ctx['affinity_match']

    if maxnodes < 2 :
        affinity_test_src["sock1"] = "0"

    if maxcpus == 2:
        affinity_test_src["8-10"] = all_cpus_sans0
        affinity_test_src["5"] = all_cpus_sans0
    else:
        if maxcpus < 9 or maxcpus < 11 :
            affinity_test_src["8-10"] = all_cpus
            affinity_test_src["5"] = all_cpus

    print "numa findings maxnodes:", maxnodes, " maxcpus:", maxcpus, " numa_match:", numa_match, " numa_layout", numa_layout, " map:", affinity_test_src
    ossie.utils.testing.main("../GPP.spd.xml") # By default tests all implementations
