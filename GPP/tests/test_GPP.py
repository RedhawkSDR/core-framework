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

import commands
import multiprocessing
import os
import Queue
import resource
import shlex
import shutil
import socket
import subprocess
import sys
import time

from omniORB import any, CORBA

import CosEventChannelAdmin, CosEventChannelAdmin__POA
from ossie.utils.sandbox.registrar import ApplicationRegistrarStub
from ossie.utils.sandbox import naming
from ossie.utils import sb, redhawk
from ossie.cf import CF, CF__POA
import ossie.utils.testing
import ossie.properties
from redhawk import numa

from _unitTestHelpers import scatest, runtestHelpers

def hasNumaSupport():
    return runtestHelpers.haveDefine('../cpp/Makefile', 'HAVE_LIBNUMA')

topology = numa.NumaTopology()

skipUnless = scatest._skipUnless


def wait_predicate(pred, timeout):
    end = time.time() + timeout
    while time.time() < end:
        if pred():
            return
        time.sleep(0.1)

def nolaunch(obj):
    """
    Decorator to disable automatic launch of the GPP from the setUp() method.
    This is for use by tests that must override properties that can only be set
    via the command line or initializeProperties().
    """
    obj.nolaunch = True
    return obj

# Base unit testing class for new-style GPP tests, based off of the default
# generated unit test. Adds simplified management and cleanup of programs
# launched by the GPP.
class GPPSandboxTest(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the device.
    SPD_FILE = '../GPP.spd.xml'

    def setUp(self):
        print "\n-----------------------"
        print "Running: ", self.id().split('.')[-1]
        print "-----------------------\n"

        if self._shouldLaunch():
            self.launchGPP(properties={'DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b':'x86_64'})
        else:
            self.comp = None

        self._pids = []
        self._testDirs = []
        self._testFiles = []
        self._busyProcs = []

    def _shouldLaunch(self):
        method = getattr(self, self._testMethodName, None)
        if not method:
            return True
        # Check for the 'nolaunch' attribute (its value is irrelevant); unless
        # it's present, launch the GPP
        return not hasattr(method, 'nolaunch')

    def launchGPP(self, properties={}):
        # Launch the device, using the selected implementation
        self.comp = sb.launch(self.spd_file,
                              impl=self.impl,
                              #debugger='gdb',
                              properties=properties)
        return self.comp

    def tearDown(self):
        # Clean up any leftover busy subprocesses
        for proc in self._busyProcs:
            if proc.poll() is None:
                proc.kill()

        # Terminate all launched executables, ignoring errors
        remaining_pids = []
        for pid in self._pids:
            try:
                self.comp.ref.terminate(pid)
            except:
                remaining_pids.append(pid)

        # In case the GPP really failed badly, manually kill the processes
        for pid in remaining_pids:
            try:
                os.killpg(pid, 9)
            except OSError:
                pass

        # Clean up all sandbox artifacts created during test
        sb.release()

        for filename in self._testFiles:
            try:
                os.unlink(filename)
            except OSError:
                pass

        for path in self._testDirs:
            shutil.rmtree(path)

    def addTestDirectory(self, path):
        self._testDirs.append(path)

    def addTestFile(self, path):
        self._testFiles.append(path)

    def removeTestFile(self, path):
        self._testFiles.remove(path)

    def addBusyTasks(self, count):
        self._busyProcs += [subprocess.Popen('bin/busy.py') for _ in xrange(count)]

    def clearBusyTasks(self):
        for proc in self._busyProcs:
            proc.kill()
        self._busyProcs = []

    def waitUsageState(self, state, timeout):
        wait_predicate(lambda: self.comp._get_usageState() == state, timeout)
        self.assertEqual(self.comp._get_usageState(), state)

    def _execute(self, executable, options, parameters):
        if isinstance(options, dict):
            options = [CF.DataType(k, any.to_any(v)) for k, v in options.items()]
        if isinstance(parameters, dict):
            parameters = [CF.DataType(k, any.to_any(v)) for k, v in parameters.items()]
        pid = self.comp.ref.execute(executable, options, parameters)
        if pid != 0:
            self._pids.append(pid)
        return pid

    def _launchComponent(self, executable, name, profile, options={}, parameters={}):
        # Using the stub from the naming module allows fetching the component
        # object, which the other version does not support; these should be
        # consolidated at some point
        appReg = naming.ApplicationRegistrarStub()
        appreg_ior = sb.orb.object_to_string(appReg._this())

        params = {}
        params.update(parameters)
        params['COMPONENT_IDENTIFIER'] = name
        params['NAME_BINDING'] =  name
        params['PROFILE_NAME'] = profile
        params['NAMING_CONTEXT_IOR'] = appreg_ior

        pid = self._execute(executable, options, params)
        self.assertNotEqual(pid, 0)

        wait_predicate(lambda: appReg.getObject(name) is not None, 2.0)
        comp = appReg.getObject(name)
        self.failIf(comp is None, "component '" + name + "' never registered")

        return (pid, comp)

    def _launchComponentStub(self, name, options={}, parameters={}):
        executable = '/dat/component_stub/python/component_stub.py'
        profile = '/component_stub/component_stub.spd.xml'
        return self._launchComponent(executable, name, profile, options, parameters)


class GPPTests(GPPSandboxTest):
    def testPropertyEvents(self):
        event_queue = Queue.Queue()
        event_channel = sb.createEventChannel('properties')
        event_channel.eventReceived.addListener(event_queue.put)

        self.comp.connect(event_channel)

        self.comp.loadThreshold = 81
        
        # Make sure the background status events are emitted
        try:
            event = event_queue.get(timeout=1.0)
        except Queue.Empty:
            self.fail('Property change event not received')
        event = any.from_any(event, keep_structs=True)
        event_dict = ossie.properties.props_to_dict(event.properties)
        self.assertEqual(self.comp._id, event.sourceId)
        self.assertEqual(self.comp.loadThreshold.id, event.properties[0].id)
        self.assertEqual(81, any.from_any(event.properties[0].value))

    def testLimits(self):
        limits = resource.getrlimit(resource.RLIMIT_NPROC)

        # Check that the system limits are sane
        self.assertTrue(self.comp.sys_limits.current_threads > 0)
        if limits[1] == -1:
            # system limit is set to unlimited, can only check that the component is reporting a positive value
            self.assertTrue(self.comp.sys_limits.current_threads > 0)
        else:
            self.assertTrue(self.comp.sys_limits.max_threads > self.comp.sys_limits.current_threads)
        self.assertTrue(self.comp.sys_limits.current_open_files > 0)
        self.assertTrue(self.comp.sys_limits.max_open_files > self.comp.sys_limits.current_open_files)

        # Check that the GPP's process limits are also sane
        self.assertTrue(self.comp.gpp_limits.current_threads > 0)
        if limits[0] == -1:
            # process limit is set to unlimited, can only check that the component is reporting a positive value
            self.assertTrue(self.comp.gpp_limits.current_threads > 0)
        else:
            self.assertTrue(self.comp.gpp_limits.max_threads > self.comp.gpp_limits.current_threads)
        self.assertTrue(self.comp.gpp_limits.current_open_files > 0)
        self.assertTrue(self.comp.gpp_limits.max_open_files > self.comp.gpp_limits.current_open_files)

    def testReservation(self):
        # Set the idle threshold to 30% (i.e., can use up to 70%) and the
        # reserved capacity per component to 25%; this gives plenty of headroom
        # with two components (50% utilization leaves a 20% margin), but a
        # third component unambiguously crosses into the busy threshold
        self.comp.thresholds.cpu_idle = 30
        self.comp.reserved_capacity_per_component = 0.25 * self.comp.processor_cores
        self.assertEquals(self.comp._get_usageState(),CF.Device.IDLE)

        self._launchComponentStub('reservation_1')
        self._launchComponentStub('reservation_2')

        # Give the GPP a couple of measurement cycles to make sure it doesn't
        # go busy; the CPU utilization (always the first entry) should report
        # 50% subscribed
        time.sleep(2)
        self.assertEquals(self.comp._get_usageState(), CF.Device.ACTIVE)
        expected = 0.5 * self.comp.processor_cores
        self.assertEquals(expected, self.comp.utilization[0].subscribed)

        # Launch the third component and give up to 2 seconds for the GPP to go
        # busy; CPU utilization should now be 75% subscribed
        self._launchComponentStub('reservation_3')
        self.waitUsageState(CF.Device.BUSY, 2.0)
        expected = 0.75 * self.comp.processor_cores
        self.assertEquals(expected, self.comp.utilization[0].subscribed)

        # Reduce the reserved capacity such that it consumes less than the idle
        # threshold (10% x 3 = 30% active = 70% idle)
        self.comp.reserved_capacity_per_component = 0.1 * self.comp.processor_cores
        self.waitUsageState(CF.Device.ACTIVE, 2.0)
        # 30% is an inexact fraction, so allow a little tolerance
        expected = 0.3 * self.comp.processor_cores
        self.assertAlmostEquals(expected, self.comp.utilization[0].subscribed, 1)

    def testFloorReservation(self):
        # Reserve an absurdly large amount of cores, which should drive the GPP
        # to a busy state immediately
        self.assertEquals(self.comp._get_usageState(),CF.Device.IDLE)
        params = {"RH::GPP::MODIFIED_CPU_RESERVATION_VALUE": 1000.0}
        self._launchComponentStub('floor_reservation_1', parameters=params)

        self.waitUsageState(CF.Device.BUSY, 2.0)

    def _unpackThresholdEvents(self, message):
        for dt in any.from_any(message, keep_structs=True):
            if dt.id != 'threshold_event':
                continue
            props = any.from_any(dt.value, keep_structs=True)
            yield ossie.properties.props_to_dict(props)

    def _checkThresholdEvent(self, resourceId, thresholdClass, exceeded):
        try:
            event = self.queue.get(timeout=2.0)
        except Queue.Empty:
            self.fail('Threshold event not received')
        self.assertEqual(self.comp._refid, event['threshold_event::source_id'])
        self.assertEqual(thresholdClass, event['threshold_event::threshold_class'])
        self.assertEqual(resourceId, event['threshold_event::resource_id'])
        self._assertThresholdState(event, exceeded)

    def _assertThresholdState(self, event, exceeded):
        if exceeded:
            event_type = 'THRESHOLD_EXCEEDED'
        else:
            event_type = 'THRESHOLD_NOT_EXCEEDED'
        self.assertEqual(event_type, event['threshold_event::type'])

    def _testThresholdEventType(self, name, resourceId, thresholdClass, value):
        # Save the original value and set the test value to trigger an
        # "exceeded" event
        orig_value = self.comp.thresholds[name]
        self.comp.thresholds[name] = value
        self._checkThresholdEvent(resourceId, thresholdClass, True)

        # Turning off the threshold should trigger a "not exceeded" event
        self.comp.thresholds.ignore = True
        self._checkThresholdEvent(resourceId, thresholdClass, False)

        # Turning it on again should trigger another "exceeded" event
        self.comp.thresholds.ignore = False
        self._checkThresholdEvent(resourceId, thresholdClass, True)

        # Restore the original value, trigger "not exceeded" event
        self.comp.thresholds[name] = orig_value
        self._checkThresholdEvent(resourceId, thresholdClass, False)

    def _checkNicEvents(self, nics, exceeded):
        expected = set(nics)
        end = time.time() + 2.0
        while expected and (time.time() < end):
            try:
                event = self.queue.get_nowait()
            except Queue.Empty:
                time.sleep(0.1)
                continue

            # Only 1 device connected to the event channel, the source ID had
            # better be correct
            self.assertEqual(self.comp._refid, event['threshold_event::source_id'])

            # Should only be receiving one NIC message from each configured
            # interface
            nic_name = event['threshold_event::resource_id']
            self.failUnless(nic_name in nics, 'Received message from unexpected NIC %s' % nic_name)
            self.failUnless(nic_name in expected, 'Received too many messages from NIC %s' % nic_name)
            threshold_class = event['threshold_event::threshold_class']
            self.assertEqual('NIC_THROUGHPUT', threshold_class, 'Received unexpected threshold class %s' % threshold_class)
            self._assertThresholdState(event, exceeded)
            expected.remove(nic_name)

        self.assertEqual(set(), expected, 'Did not receive message from NIC(s): ' + ' '.join(expected))

    def testThresholdEvents(self):
        # Cut down the threshold cycle time to trigger events faster (we're not
        # worried about the extra processing time here)
        self.comp.threshold_cycle_time = 0.1

        # Add a file for the shm_free threshold test.  1 MiB is sufficient.
        fpath = '/dev/shm/junk'
        self._testFiles.append(fpath)
        cmd = 'dd if=/dev/zero of={0} count=1024 bs=1024'.format(fpath)
        cmd = shlex.split(cmd)
        with open(os.devnull, 'w') as devnull:
            subprocess.call(cmd, stdout=devnull, stderr=devnull)

        # Create a virtual event channel to queue the GPP's messages
        event_channel = sb.createEventChannel('thresholds')
        self.queue = Queue.Queue()
        def queue_message(message):
            # Unpack and queue up threshold event messages
            for event in self._unpackThresholdEvents(message):
                self.queue.put(event)

        event_channel.eventReceived.addListener(queue_message)
        self.comp.connect(event_channel, usesPortName="MessageEvent_out")

        time.sleep(1)
        queue_empty = False
        while not queue_empty:
            try:
                event = self.queue.get(timeout=2.0)
            except Queue.Empty:
                queue_empty = True

        # Test all thresholds except NIC, which is a little more complex
        self._testThresholdEventType('cpu_idle', 'cpu', 'CPU_IDLE', 100)
        self._testThresholdEventType('mem_free', 'physical_ram', 'MEMORY_FREE', self.comp.memFree + 100)
        self._testThresholdEventType('load_avg', 'cpu', 'LOAD_AVG', 0)
        self._testThresholdEventType('shm_free', 'shm', 'SHM_FREE', self.comp.shmCapacity)
        self._testThresholdEventType('files_available', 'ulimit', 'OPEN_FILES', 100.0)
        self._testThresholdEventType('threads', 'ulimit', 'THREADS', 100.0)

        # If there is more than one NIC (real or virtual), each one will emit
        # an event; we only really care about the "available" NICs
        nics = list(self.comp.available_nic_interfaces)
        nic_usage = int(self.comp.thresholds.nic_usage)
        self.comp.thresholds.nic_usage = 0
        self._checkNicEvents(nics, True)

        # Turning off the threshold should trigger "not exceeded" event(s)
        self.comp.thresholds.ignore = True
        self._checkNicEvents(nics, False)

        # Turning it on again should trigger "exceeded" event(s)
        self.comp.thresholds.ignore = False
        self._checkNicEvents(nics, True)

        # Restore the original value, trigger "not exceeded" event(s)
        self.comp.thresholds.nic_usage = nic_usage
        self._checkNicEvents(nics, False)

    def testDefaultDirectories(self):
        # Test that when cache and working directory are not given, the
        # properties still have meaningful values
        cwd = os.getcwd()
        self.assertEquals(cwd, self.comp.cacheDirectory)
        self.assertEquals(cwd, self.comp.workingDirectory)

    @nolaunch
    def testCacheDirectory(self):
        # Create an alternate directory for the cache
        cache_dir = os.path.join(os.getcwd(), 'testCacheDirectory')
        os.mkdir(cache_dir)
        self.addTestDirectory(cache_dir)
        self.launchGPP({'cacheDirectory':cache_dir, 'DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b':'x86_64'})

        # Make sure the property is correct
        self.assertEqual(cache_dir, self.comp.cacheDirectory)

        # Load a file and check that it was copied to the right place
        expected = os.path.join(cache_dir, 'bin/echo_pid.py')
        self.failIf(os.path.exists(expected))
        fs_stub = ComponentTests.FileSystemStub()
        self.comp.ref.load(fs_stub._this(), "/bin/echo_pid.py", CF.LoadableDevice.EXECUTABLE)
        self.failUnless(os.path.isfile(expected))

    @nolaunch
    def testWorkingDirectory(self):
        # Create an alternate directory for the working directory
        working_dir = os.path.join(os.getcwd(), 'testWorkingDirectory')
        os.mkdir(working_dir)
        self.addTestDirectory(working_dir)
        self.launchGPP({'workingDirectory':working_dir, 'DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b':'x86_64'})

        # Make sure the property is correct
        self.assertEqual(working_dir, self.comp.workingDirectory)

        # Run a test executable that writes to its current directory
        expected = os.path.join(working_dir, 'pid.out')
        self.failIf(os.path.exists(expected))
        pid = self._execute("/bin/echo_pid.py", {}, {})
        wait_predicate(lambda: os.path.exists(expected), 1.0)
        self.failUnless(os.path.exists(expected))

        # Read the output file and make sure that the right PID was written
        with open(expected, 'r') as fp:
            echo_pid = int(fp.read().strip())
        self.assertEqual(pid, echo_pid)

    @nolaunch
    def testCacheAndWorkingDirectory(self):
        # Test the interaction of the cache and working directories; create an
        # alternate directory for both
        base_dir = os.path.join(os.getcwd(), 'testCacheAndWorkingDirectory')
        os.mkdir(base_dir)
        self.addTestDirectory(base_dir)
        cache_dir  = os.path.join(base_dir, 'cache')
        os.mkdir(cache_dir)
        working_dir = os.path.join(base_dir, 'cwd')
        os.mkdir(working_dir)
        self.launchGPP({'cacheDirectory':cache_dir, 'workingDirectory':working_dir, 'DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b':'x86_64'})

        # Make sure the properties are correct
        self.assertEqual(cache_dir, self.comp.cacheDirectory)
        self.assertEqual(working_dir, self.comp.workingDirectory)

        # Load a file and check that it was copied to the right place
        expected = os.path.join(cache_dir, 'bin/echo_pid.py')
        self.failIf(os.path.exists(expected))
        fs_stub = ComponentTests.FileSystemStub()
        self.comp.ref.load(fs_stub._this(), "/bin/echo_pid.py", CF.LoadableDevice.EXECUTABLE)
        self.failUnless(os.path.isfile(expected))

        # Run a test executable that writes to its current directory
        expected = os.path.join(working_dir, 'pid.out')
        self.failIf(os.path.exists(expected))
        pid = self._execute("/bin/echo_pid.py", {}, {})
        wait_predicate(lambda: os.path.exists(expected), 1.0)
        self.failUnless(os.path.exists(expected))

        # Read the output file and make sure that the right PID was written
        with open(expected, 'r') as fp:
            echo_pid = int(fp.read().strip())
        self.assertEqual(pid, echo_pid)

    def testSharedMemoryProperties(self):
        status = os.statvfs('/dev/shm')

        # The total shouldn't change in normal operation, so using the same
        # expected integer math should give the same value
        total = (status.f_blocks * status.f_frsize) / 1024 / 1024
        self.assertEqual(total, self.comp.shmCapacity)

        # Free could vary slightly if something else is happening on the
        # system, so give it a little bit of slack (1 MB)
        free = (status.f_bfree * status.f_frsize) / 1024 / 1024
        self.failIf(abs(free - self.comp.shmFree) > 1)

    def testBusyCpuIdle(self):
        # Disable load average threshold
        self.comp.thresholds.load_avg = -1

        self.assertEqual(self.comp._get_usageState(), CF.Device.IDLE)

        # Task all of the CPUs to be busy (more or less) and wait for the idle
        # threshold to be exceeded
        self.addBusyTasks(self.comp.processor_cores)
        self.waitUsageState(CF.Device.BUSY, 5.0)
        self.failUnless("CPU IDLE" in self.comp.busy_reason.upper())

        # Clear all busy tasks and wait for the device to go back to idle
        self.clearBusyTasks()
        self.waitUsageState(CF.Device.IDLE, 5.0)
        self.assertEqual(self.comp._get_usageState(), CF.Device.IDLE)
        self.assertEqual(self.comp.busy_reason, "")

    def testBusyLoadAvg(self):
        # Disable CPU idle threshold and lower the load average threshold so
        # that it's easier to exceed
        self.comp.thresholds.cpu_idle = -1
        self.comp.thresholds.load_avg = 25

        # The load average may exceed the threshold to begin with, depending on
        # what the system was doing before this test
        print 'Waiting for load average to fall below threshold, may take a while'
        self.waitUsageState(CF.Device.IDLE, 30.0)

        # Occupy all of the CPUs with busy tasks and wait for the load average
        # to exceed the threshold; this may take a while, since it's based on a
        # 1 minute window
        self.addBusyTasks(self.comp.processor_cores)
        print 'Waiting for load average to exceed threshold, may take a while'
        self.waitUsageState(CF.Device.BUSY, 30.0)
        self.failUnless("LOAD AVG" in self.comp.busy_reason.upper())

        # Clear all of the busy tasks; again, due to the 1 minute window, it
        # may take a little while for the load average to drop back below the
        # threshold
        self.clearBusyTasks()
        print 'Waiting for load average to fall below threshold, may take a while'
        self.waitUsageState(CF.Device.IDLE, 90.0)
        self.assertEqual(self.comp.busy_reason, "")

    def testBusySharedMemory(self):
        # Cut down the update time for testing
        self.comp.threshold_cycle_time = 0.1

        self.assertEqual(self.comp._get_usageState(), CF.Device.IDLE)

        # Set the shared memory threshold a little below the current free, so
        # that a relative small uptick in usage will cross the threshold
        current_shm = int(self.comp.shmFree)
        self.comp.thresholds.shm_free = current_shm - 2

        # Create a temporary file that consumes a few MB, enough to cross the
        # threshold and a little further just to be sure
        shm_file = '/dev/shm/test-%d' % os.getpid()
        fill_size = 4*1024*1024
        self.addTestFile(shm_file)
        with open(shm_file, 'w') as fp: 
            # Resize the file and write one byte every page to ensure that
            # shared memory is consumed
            fp.truncate(fill_size)
            for pos in xrange(0, fill_size, 4096):
                fp.seek(pos)
                fp.write('\x00')

        self.waitUsageState(CF.Device.BUSY, 1.0)

        # Remove the file, which should push the free shared memory back over
        # the threshold
        os.unlink(shm_file)
        self.waitUsageState(CF.Device.IDLE, 1.0)


class ComponentTests(GPPSandboxTest):
    """Test for all component implementations in test"""
    def tearDown(self):
        super(ComponentTests, self).tearDown()
        sproc="./spacely sprockets"
        try:
            os.remove(sproc)
        except:
            pass

        try:
            os.system('pkill -9 -f busy.py')
        except OSError:
            pass

        try:
            os.system('pkill -9 -f "spacely sprockets"')
        except OSError:
            pass


    def get_single_nic_interface(self):
        self.nic_list = []
        cmd = '/sbin/ifconfig -a'
        (exitstatus, ifconfig_info) = commands.getstatusoutput(cmd)
        if exitstatus != 0:
            print "Problem running '{0}'".format(cmd)
            return

        # add vlans
        for i in ifconfig_info.splitlines():
            i = i.strip()
            if i.startswith('e') == False or i.find('Link encap') < 0:
                continue

            if len(i.split()) > 0  :
               self.nic_list.append( i.split()[0] )

    # Create a test file system
    class FileStub(CF__POA.File):
        def __init__(self, path):
            self.path = path
            self.fobj = open(self.path)
        
        def sizeOf(self):
            return os.path.getsize(self.path)
        
        def read(self, bytes):
            return self.fobj.read(bytes)
        
        def close(self):
            return self.fobj.close()
            
    class FileSystemStub(CF__POA.FileSystem):
        def __init__(self, path='.'):
            self.path = os.path.abspath(path)
            
        def list(self, path):
            path = os.path.basename(path)
            return [CF.FileSystem.FileInformationType(path, CF.FileSystem.PLAIN, 100, [])]
        
        def exists(self, fileName):
            tmp_fileName = self.path + fileName
            return os.access(tmp_fileName, os.F_OK)
            
        def open(self, path, readonly):
            tmp_fileName = self.path + path
            file = ComponentTests.FileStub(tmp_fileName)
            return file._this()

    def testScaBasicBehavior(self):
        # Get expectedProps from .prf file.
        expectedProps = []
        expectedProps.extend(self.comp._getPropertySet(kinds=("configure", "execparam"),
                                                       modes=("readwrite", "readonly"),
                                                       includeNil=True))
        expectedProps.extend(self.comp._getPropertySet(kinds=("allocate",), action="external", includeNil=True))
        # Get props via CORBA from the component reference.
        props = self.comp.ref.query([])
        props = dict((x.id, any.from_any(x.value)) for x in props)
        # Query may return more properties, but not fewer.
        for expectedProp in expectedProps:
            self.assertEquals(expectedProp.id in props, True)

        qr = [CF.DataType(id="DCE:9190eb70-bd1e-4556-87ee-5a259dcfee39", value=any.to_any(None)),  # hostName
              CF.DataType(id="DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d", value=any.to_any(None))]  # DeviceKind
        qr = self.comp.ref.query(qr)
        self.assertEqual(qr[0].value.value(), socket.gethostname())
        self.assertEqual(qr[1].value.value(), "GPP")

        # Verify that all expected ports are available
        for port in self.comp._scd.get_componentfeatures().get_ports().get_uses():
            port_obj = self.comp.ref.getPort(str(port.get_usesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a("IDL:CF/Port:1.0"),  True)

        for port in self.comp._scd.get_componentfeatures().get_ports().get_provides():
            port_obj = self.comp.ref.getPort(str(port.get_providesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a(port.get_repid()),  True)

        # Make sure start and stop can be called without throwing exceptions
        self.comp.ref.start()
        self.comp.ref.stop()

        # Simulate regular component shutdown
        self.comp.ref.releaseObject()

    def testExecute(self):
        fs_stub = ComponentTests.FileSystemStub('./dat')
        fs_stub_var = fs_stub._this()

        self.comp.ref.load(fs_stub_var, "/component_stub.py", CF.LoadableDevice.EXECUTABLE)
        # Technically this is an internal implementation detail that the file is loaded into the CWD of the device
        self.assertEqual(os.path.isfile("component_stub.py"), True)

        comp_id = "DCE:00000000-0000-0000-0000-000000000000:waveform_1"
        app_id = "waveform_1"
        appReg = ApplicationRegistrarStub(comp_id, app_id)
        appreg_ior = sb.orb.object_to_string(appReg._this())
        params = [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id)),
                  CF.DataType(id="NAME_BINDING", value=any.to_any("component_stub")),
                  CF.DataType(id="PROFILE_NAME", value=any.to_any("/component_stub/component_stub.spd.xml")),
                  CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))]
        pid = self.comp.ref.execute("/component_stub.py", [], params)
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
        self.comp.ref.terminate(pid)
        try:
            os.kill(pid, 0)
        except OSError:
            pass
        else:
            self.fail("Process failed to terminate")

    def visual_testBusy(self):
        self.assertEqual(self.comp.ref._get_usageState(), CF.Device.IDLE)
        cores = multiprocessing.cpu_count()
        sleep_time = 3+cores/10.0
        if sleep_time < 7:
            sleep_time = 7
        procs = []
        for core in range(cores*2):
            procs.append(subprocess.Popen('bin/busy.py'))
        end_time = time.time() + sleep_time
        while end_time > time.time():
            print str(time.time()) + " busy reason: " + str(self.comp.busy_reason)
            time.sleep(.4)
        self.assertEqual(self.comp.ref._get_usageState(), CF.Device.BUSY)
        br=self.comp.busy_reason
        br_cpu="CPU IDLE" in br.upper() or "LOAD AVG" in br.upper()
        self.assertEqual(br_cpu, True)
        for proc in procs:
            proc.kill()
        time.sleep(sleep_time)
        self.assertEqual(self.comp.ref._get_usageState(), CF.Device.IDLE)
        self.assertEqual(self.comp.busy_reason, "")

        fs_stub = ComponentTests.FileSystemStub('./dat')
        fs_stub_var = fs_stub._this()

        self.comp.ref.load(fs_stub_var, "/component_stub.py", CF.LoadableDevice.EXECUTABLE)
        # Technically this is an internal implementation detail that the file is loaded into the CWD of the device
        self.assertEqual(os.path.isfile("component_stub.py"), True)

        comp_id = "DCE:00000000-0000-0000-0000-000000000000:waveform_1"
        app_id = "waveform_1"
        appReg = ApplicationRegistrarStub(comp_id, app_id)
        appreg_ior = sb.orb.object_to_string(appReg._this())
        pid = self.comp.ref.execute("/component_stub.py",
                                    [],
                                    [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id)),
                                     CF.DataType(id="NAME_BINDING", value=any.to_any("component_stub")),
                                     CF.DataType(id="PROFILE_NAME", value=any.to_any("/component_stub/component_stub.spd.xml")),
                                     CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))])
        self.assertNotEqual(pid, 0)
        time.sleep(1)
        self.assertEqual(self.comp.ref._get_usageState(), CF.Device.ACTIVE)
        cores = multiprocessing.cpu_count()
        procs = []
        for core in range(cores*2):
            procs.append(subprocess.Popen('bin/busy.py'))
        end_time = time.time() + sleep_time
        while end_time > time.time():
            print str(time.time()) + " busy reason: " + str(self.comp.busy_reason)
            time.sleep(.4)

        self.assertEqual(self.comp.ref._get_usageState(), CF.Device.BUSY)
        br_cpu="CPU IDLE" in br.upper() or "LOAD AVG" in br.upper()
        for proc in procs:
            proc.kill()
        time.sleep(sleep_time)
        self.assertEqual(self.comp.ref._get_usageState(), CF.Device.ACTIVE)
        self.assertEqual(self.comp.busy_reason, "")

        try:
            os.kill(pid, 0)
        except OSError:
            self.fail("Process failed to execute")
        time.sleep(1)
        self.comp.ref.terminate(pid)
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
        self.comp.ref.configure(configureProps)

        qr = self.comp.ref.query([CF.DataType(id="DCE:218e612c-71a7-4a73-92b6-bf70959aec45", value=any.to_any(None))])
        useScreen = qr[0].value.value()
        self.assertEqual(useScreen, True)

        fs_stub = ComponentTests.FileSystemStub('./dat')
        fs_stub_var = fs_stub._this()

        self.comp.ref.load(fs_stub_var, "/component_stub.py", CF.LoadableDevice.EXECUTABLE)
        # Technically this is an internal implementation detail that the file is loaded into the CWD of the device
        self.assertEqual(os.path.isfile("component_stub.py"), True)

        comp_id = "DCE:00000000-0000-0000-0000-000000000000:waveform_1"
        app_id = "waveform_1"
        appReg = ApplicationRegistrarStub(comp_id, app_id)
        appreg_ior = sb.orb.object_to_string(appReg._this())
        pid = self.comp.ref.execute("/component_stub.py",
                                    [],
                                    [CF.DataType(id="COMPONENT_IDENTIFIER", value=any.to_any(comp_id)),
                                     CF.DataType(id="NAME_BINDING", value=any.to_any("MyComponent")),
                                     CF.DataType(id="PROFILE_NAME", value=any.to_any("empty")),
                                     CF.DataType(id="NAMING_CONTEXT_IOR", value=any.to_any(appreg_ior))])
        self.assertNotEqual(pid, 0)

        try:
            os.kill(pid, 0)
        except OSError:
            self.fail("Process failed to execute")
        time.sleep(1)

        if "SCREENDIR" in os.environ:
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

        self.comp.ref.terminate(pid)
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
        proc="bin/busy.py"
        sproc="./spacely sprockets"
        shutil.copy(proc,sproc)
        procs = subprocess.Popen(sproc)
	self._busyProcs += [procs]
        self.assertEqual(procs.poll(), None )

        self.assertEqual(self.comp._get_usageState(), CF.Device.IDLE)
        # wait for an update to occur
        time.sleep(4)

        # basically if we get past here.. and did not crash we are goodx
        self.assertEqual(self.comp._process.isAlive(), True )
        try:
            os.system('pkill -9 -f "'+sproc+'"')
            procs.kill()
        except:
            pass

    @nolaunch
    def test_mcastNicThreshold(self):
        # pass additional mcast params for the test
        params = { "DCE:4e416acc-3144-47eb-9e38-97f1d24f7700": 'eth0',
                   'DCE:5a41c2d3-5b68-4530-b0c4-ae98c26c77ec': 100,
                   'DCE:442d5014-2284-4f46-86ae-ce17e0749da0': 100,
                   'DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b':'x86_64' }
        self.launchGPP(params)

        # fire off change listener for setting threshold
        cprops = [CF.DataType(id='DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1',value=any.to_any(1))]
        self.comp.configure(cprops)

        # check that values were changed
        cprops = [CF.DataType(id='DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1',value=any.to_any(None)),
                  CF.DataType(id='DCE:506102d6-04a9-4532-9420-a323d818ddec',value=any.to_any(None)) ]
        cprops = self.comp.ref.query(cprops)
        self.assertEquals( cprops[0].value.value(), 1)
        self.assertEquals( cprops[1].value.value(), 1)

        # try failed allocation
        allocProps = [CF.DataType(id='DCE:506102d6-04a9-4532-9420-a323d818ddec',value=any.to_any(200))]
        self.assertRaises( CF.Device.InvalidCapacity, self.comp.ref.allocateCapacity, allocProps)

        # fire off change listener for setting threshold
        cprops = [CF.DataType(id='DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1',value=any.to_any(90))]
        self.comp.ref.configure(cprops)

        # check that values were changed
        cprops = [CF.DataType(id='DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1',value=any.to_any(None)),
                  CF.DataType(id='DCE:506102d6-04a9-4532-9420-a323d818ddec',value=any.to_any(None)) ]
        cprops = self.comp.ref.query(cprops)
        self.assertEquals( cprops[0].value.value(), 90)
        self.assertEquals( cprops[1].value.value(), 90)

        # try good allocation
        allocProps = [CF.DataType(id='DCE:506102d6-04a9-4532-9420-a323d818ddec',value=any.to_any(50))]
        self.comp.ref.allocateCapacity( allocProps)

        cprops = [CF.DataType(id='DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1',value=any.to_any(None)),
                  CF.DataType(id='DCE:506102d6-04a9-4532-9420-a323d818ddec',value=any.to_any(None)) ]
        cprops = self.comp.ref.query(cprops)
        self.assertEquals( cprops[0].value.value(), 90)
        self.assertEquals( cprops[1].value.value(), 40)

        self.comp.ref.deallocateCapacity( allocProps)

        cprops = [CF.DataType(id='DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1',value=any.to_any(None)),
                  CF.DataType(id='DCE:506102d6-04a9-4532-9420-a323d818ddec',value=any.to_any(None)) ]
        cprops = self.comp.ref.query(cprops)
        self.assertEquals( cprops[0].value.value(), 90)
        self.assertEquals( cprops[1].value.value(), 90)

    def test_loadCapacity(self):

        # wait for load avg to settle 
        time.sleep(5)

        # query current capacity
        cprops = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(None))]
        cprops = self.comp.ref.query(cprops)

        capacity=cprops[0].value.value()
        req_cap = capacity/2;

        # try allocation
        allocProps = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(req_cap))]
        self.comp.ref.allocateCapacity(allocProps)

        cprops = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(None))]
        cprops=self.comp.ref.query(cprops)

        # make sure capacity was affected
        remaining_cap = capacity-req_cap
        self.assertEquals( cprops[0].value.value(), remaining_cap)

        # now request more... result will be be  0..
        allocProps = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(capacity))]
        self.comp.ref.allocateCapacity(allocProps)

        cprops = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(None))]
        cprops=self.comp.ref.query(cprops)

        # make sure capacity is zero..
        self.assertEquals( cprops[0].value.value(), 0.0)

        # now dealloacte more than requests.. should go back to max.
        allocProps = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(capacity*2))]
        self.comp.ref.deallocateCapacity(allocProps)

        cprops = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(None))]
        cprops=self.comp.ref.query(cprops)

        # make sure capacity is original max.
        self.assertEquals( cprops[0].value.value(), capacity)   

        # now set reservation mode off 
        self.comp.reserved_capacity_per_component=0.0

        # now try to allocate capacity, should fail
        allocProps = [CF.DataType(id='DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056',value=any.to_any(capacity*2))]
        self.assertRaises( CF.Device.InsufficientCapacity, self.comp.ref.allocateCapacity, allocProps)

    @nolaunch
    def test_threshold_usagestate(self):
        self.get_single_nic_interface()
        if len(self.nic_list)> 1:
            params = {'nic_interfaces': [self.nic_list[0]]}
            self.launchGPP(parameters=params, properties={'DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b':'x86_64'})
        else:
            self.launchGPP(properties={'DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b':'x86_64'})

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
            params = {'nic_interfaces': [self.nic_list[0]]}
            self.launchGPP(parameters=params, properties={'DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b':'x86_64'})
        else:
            self.launchGPP(properties={'DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b':'x86_64'})

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


@skipUnless(topology.available() and hasNumaSupport(), 'Affinity control is disabled')
class AffinityTests(GPPSandboxTest):
    def setUp(self):
        super(AffinityTests,self).setUp()

        # Always enable affinity handling for these tests
        self.comp.affinity.disabled = False

    def _getAllowedCpuList(self, pid):
        filename = '/proc/%d/status' % pid
        with open(filename, 'r') as fp:
            for line in fp:
                if not 'Cpus_allowed_list' in line:
                    continue
                cpu_list = line.split()[1]
                return numa.parseValues(cpu_list, ",")
        return []

    def _getNumCpus(self):
        output=2
        try:
            status,output=commands.getstatusoutput("ls -d /sys/devices/system/cpu/cpu[0-9]*  | wc -l")
            if status == 0:
                return int(output)
            else:
                status,output=commands.getstatusoutput("lscpu  | egrep '^CPU\('  | awk  '{ print  $2 }'")
                return int(output)
        except:
            pass
        return output

    def _deployWithAffinityOptions(self, name, affinity={}):
        options = {}
        if affinity:
            options['AFFINITY'] = [CF.DataType(k, any.to_any(v)) for k,v in affinity.items()]
        pid, comp = self._launchComponentStub(name, options=options)
        return pid

    def _getIrqs(self, nic):
        """Get IRQ numbers for a nic."""
        # Location 1:  the filenames in this dir listing.
        dpath = '/sys/class/net/{0}/device/msi_irqs'.format(nic)
        irqs = []
        try:
            irqs = os.listdir(dpath)
        except OSError:
            # If Message Signaled Interrupts is not installed, location 1 does not exist.
            pass
        non_number_found = False
        for irq in irqs:
            if not irq.isdigit():
                non_number_found = True
        if irqs and not non_number_found:
            return irqs

        # If location 1 does not exist,
        # use location 2:  the contents of this file.
        fpath = '/sys/class/net/{0}/device/irq'.format(nic)
        content = ''
        irqs = []
        try:
            content = open(fpath).read().strip()
        except IOError:
            pass
        if not content:
            return []
        non_number_found = False
        irqs = content.split()
        for irq in irqs:
            if not irq.isdigit():
                non_number_found = True
        if irqs and not non_number_found:
            return irqs
        return []

    def _getNicAffinityViaIrqs(self, nic, irqs):
        cpus = set()
        with open('/proc/interrupts') as fp:
            for line in fp.readlines():
                words = line.split()
                irq = words[0][:-1]  # remove ':' from end
                if irq in irqs:
                    # Discard the first column (the IRQ number) and the last two
                    # (type and name) to get the CPU IRQ service totals
                    for cpu, count in enumerate(words[1:-2]):
                        try:
                          if count.isalnum() and int(count) > 0:
                              cpus.add(cpu)
                        except:
                            pass
        return sorted(cpus)

    def _getNicAffinityViaParse(self, nic):
        cpus = set()
        with open('/proc/interrupts') as fp:
            for line in fp.readlines():
                # Deselect lines that end with neither of these:
                # <NIC name>         eg eth0
                # <NIC name>-        eg eth0-rxtx-0
                line = line.strip()
                words = line.split()
                word = words[-1]
                if len(word) > len(nic):
                    suffix = word[len(nic):]
                    if not suffix.startswith('-') or 'event' in suffix:
                        continue
                elif word != nic:
                    continue
                # Discard the first column (the IRQ number) and the last two
                # (type and name) to get the CPU IRQ service totals
                cpu_irqs = line.split()[1:-2]
                for cpu, count in enumerate(cpu_irqs):
                    try:
                       if count.isalnum() and int(count) > 0:
                           cpus.add(cpu)
                    except:
                        pass
        return sorted(cpus)

    def _getNicAffinity(self, nic):
        irqs = self._getIrqs(nic)
        if irqs:
            return self._getNicAffinityViaIrqs(nic, irqs)
        else:
            return self._getNicAffinityViaParse(nic)

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
        # find all the cpus for each node
        nodes = set(topology.getNodeForCpu(cpu) for cpu in self._getNicAffinity(nic))
        cpulist = sum((node.cpus for node in nodes), [])
        if len(nic_cpus) > 1:
            # There's more than one CPU assigned to service NIC interrupts,
            # just blacklist the first one
            blacklist_cpu = nic_cpus.pop(0)
        else:
            # Only one CPU for NIC interrupts, figure out its node and
            # blacklist one of the other CPUs
            cpu = nic_cpus[0]
            node = topology.getNodeForCpu(cpu)
            cpulist = node.cpus[:]
            # Find the CPU in the list and select the next one (wrapping around
            # as necessary) to ensure that we don't blacklist the wrong CPU
            index = node.cpus.index(cpu)
            index = (index + 1) % len(node.cpus)
            blacklist_cpu = node.cpus[index]

        
        # default nic deployment is per socket, use cpulist per
        # node to check results
        nic_cpus=[ x for x in cpulist if x not in [blacklist_cpu]]

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
        os.environ['SDRROOT'] = self.orig_sdrroot

class ComponentTests_SystemReservations(DomainSupport):
    def close(self, value_1, value_2, margin = 0.01):
        if (value_2 * (1-margin)) < value_1 and (value_2 * (1+margin)) > value_1:
            return True
        return False

    def assertClose(self, value_1, value_2, margin = 0.01, msg=None):
        if self.close(value_1, value_2, margin):
            return
        if msg is None:
            msg = '{0} != {1} within {2}%'.format(value_1, value_2, margin*100)
        self.fail(msg)

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
        self.assertClose(upper_capacity, self.dom.devMgrs[0].devs[0].utilization[0]['maximum'])
        
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
        self.assertClose(sub_now, extra_reservation)

        app_2=self.dom.createApplication('/waveforms/busy_w/busy_w.sad.xml','busy_w',[])
        time.sleep(wait_amount)
        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        system_load_now = base_util['system_load']
        sub_now = base_util['subscribed']
        sub_now_pre = base_util['subscribed']
        comp_load = base_util['component_load']
        #print "After App2 Create subnow(sub) " , sub_now, " sys_load", system_load_now, " sys_load_base ", system_load_base, " comp_load ", comp_load, " subscribed(base) ", subscribed, " extra ", extra_reservation, " res per", res_per_comp, " idle cap mod ", idle_cap_mod 
        self.assertClose(sub_now, extra_reservation+res_per_comp)

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
            self.assertClose(sub_now-comp_load,res_per_comp)
        else:
            self.assertClose(sub_now, extra_reservation+res_per_comp)

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
        self.assertClose(sub_now, extra_reservation+res_per_comp )
        self.assertAlmostEquals(sub_now_pre, sub_now, 2)

    def _verifyReservations(self, extra, application, wait_amount):
        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        system_load_now = base_util['system_load']
        sub_now = base_util['subscribed']
        comp_load = base_util['component_load']
        self.assertClose(sub_now, extra)
        self.assertEquals(comp_load, 0)

        application.start()
        time.sleep(wait_amount)
        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        system_load_now = base_util['system_load']
        sub_now = base_util['subscribed']
        comp_load = base_util['component_load']
        self.assertClose(sub_now, extra)
        self.assertClose(comp_load, 2, margin=0.1)

        application.stop()
        time.sleep(wait_amount)
        base_util = self.dom.devMgrs[0].devs[0].utilization[0]
        system_load_now = base_util['system_load']
        sub_now = base_util['subscribed']
        comp_load = base_util['component_load']
        self.assertClose(sub_now, extra)
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
        self.assertClose(upper_capacity, self.dom.devMgrs[0].devs[0].utilization[0]['maximum'])

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
        self.assertClose(upper_capacity, self.dom.devMgrs[0].devs[0].utilization[0]['maximum'])

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
        self.assertClose(upper_capacity, self.dom.devMgrs[0].devs[0].utilization[0]['maximum'])

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
        self.assertClose(upper_capacity, self.dom.devMgrs[0].devs[0].utilization[0]['maximum'])

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
        self.assertClose(upper_capacity, self.dom.devMgrs[0].devs[0].utilization[0]['maximum'])

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

        dcd_file = 'sdr/dev/nodes/test_VarCache_node/DeviceManager.dcd.xml'
        with open(dcd_file + '.in', 'r') as fp:
            original = fp.read()

        cwd = os.getcwd()
        self.base_dir = cwd + '/LoadableDeviceVariableDirectoriesTest'
        self.cache_dir = self.base_dir+'/cache'
        self.cwd_dir = self.base_dir+'/cwd'
        modified = original.replace('@@@CACHE_DIRECTORY@@@', self.cache_dir)
        modified = modified.replace('@@@CURRENT_WORKING_DIRECTORY@@@', self.cwd_dir)

        with open(dcd_file, 'w') as fp:
            fp.write(modified)
        self._testFiles.append(dcd_file)

    def tearDown(self):
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


if __name__ == "__main__":
    if False:
        # Debugging support: enable this conditional to dump NUMA topology
        print "NumaSupport %d nodes %d CPUs" % (len(topology.nodes), len(topology.cpus))
        for node in topology.nodes:
            print 'Node', node.node, 'CPUs:', node.cpus
    ossie.utils.testing.main()  # By default tests all implementations
