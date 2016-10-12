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

#
from ossie.cf import CF, CF__POA
from ossie.device import start_device
from ossie.properties import rebind
import os, commands, tempfile
import resource
import socket
import stat
import subprocess
import threading
import errno
import select #@UnresolvedImport
import sys

from GPP_base import *

MY_DIR = os.path.dirname(__file__)

class GPP(GPP_base):
    """GPP Device implementation"""

    def initialize(self):
        # initialize the base class
        GPP_base.initialize(self)
        self._popen_lock = threading.Lock()

        # Setup the TMPDIR environment variable for use in diskCapacity requests
        if not os.environ.has_key("TMPDIR"):
            os.environ["TMPDIR"] = tempfile.gettempdir()

        nproc = resource.getrlimit(resource.RLIMIT_NPROC)
        if nproc[0] < nproc[1]:
            #Max the softlimit out
            resource.setrlimit(resource.RLIMIT_NPROC, (nproc[1], nproc[1]))
        if nproc[1] < 1024:
            self._log.warning("Your system nproc hard limit [%s} is set abnormally low", nproc[1])

        ######################
        # Set initial capacities
        self.memCapacity = int(self.memTotal * self.memThresholdDecimal)    # starts out as the same value
        self.loadCapacity = self.loadTotal * self.loadThresholdDecimal
        self.mcastnicIngressCapacity = int(self.mcastnicIngressTotal * self.mcastNicThresholdDecimal)
        self.mcastnicEgressCapacity = int(self.mcastnicEgressTotal * self.mcastNicThresholdDecimal)
        self.init_processor_flags()

        self.next_property_event = None

        self.start()

    ###########################################
    # CF::ExecutableDevice
    def execute(self, name, options, parameters):
        self._log.debug("execute(%s, %s, %s)", name, options, parameters)
        if not name.startswith("/"):
            raise CF.InvalidFileName(CF.CF_EINVAL, "Filename must be absolute")

        if self.isLocked(): raise CF.Device.InvalidState("System is locked down")
        if self.isDisabled(): raise CF.Device.InvalidState("System is disabled")

        # TODO SR:448
        priority = 0
        stack_size = 4096
        invalidOptions = []
        for option in options:
            val = option.value.value()
            if option.id == CF.ExecutableDevice.PRIORITY_ID:
                if ((not isinstance(val, int)) and (not isinstance(val, long))):
                    invalidOptions.append(option)
                else:
                    priority = val
            elif option.id == CF.ExecutableDevice.STACK_SIZE_ID:
                if ((not isinstance(val, int)) and (not isinstance(val, long))):
                    invalidOptions.append(option)
                else:
                    stack_size = val
        if len(invalidOptions) > 0:
            self._log.error("execute() received invalid options %s", invalidOptions)
            raise CF.ExecutableDevice.InvalidOptions(invalidOptions)

        command = os.path.abspath(name[1:]) # This is relative to our CWD
        self._log.debug("Running %s %s", command, os.getcwd())

        # SR:452
        # TODO should we also check the load file reference count?
        # Workaround
        if not os.path.isfile(command):
            raise CF.InvalidFileName(CF.CF_EINVAL, "File could not be found %s" % command)
        os.chmod(command, stat.S_IEXEC | stat.S_IREAD | stat.S_IWRITE)

        args = []
        if self.useScreen:
            os.environ["GPP_LD_LIBRARY_PATH"] = os.environ.get("LD_LIBRARY_PATH", "")
            # From the man page:
            #   -D -m   This starts screen in "detached" mode,
            #           but doesn't fork a new process.
            #           The command exits if the session terminates.
            args.extend(["screen", "-D", "-m", "-c", os.path.join(MY_DIR, "gpp.screenrc")])
            #args.extend(["screen", "-D", "-m"])

            # Attempt to figure out a useful screen session name
            component_id = None
            component_name = None
            for param in parameters:
                if param.id == "COMPONENT_IDENTIFIER" and param.value.value() != None:
                    component_id = param.value.value()
                elif param.id == "NAME_BINDING" and param.value.value() != None:
                    component_name = param.value.value()

            if component_id != None and component_name != None:
                # component_id is in the form <componentinstantiationid>:<waveformname>
                try:
                    if component_id.startswith("DCE:"):
                        component_id = component_id[4:]
                    component_inst_id, waveform_name = component_id.split(":", 1)
                    args.extend(["-S", "%s.%s" % (waveform_name, component_name)])
                    args.extend(["-t", "%s.%s" % (waveform_name, component_name)])
                except ValueError:
                    pass

        args.append(command)

        # SR:446, SR:447
        for param in parameters:
            if param.value.value() != None:
                args.append(str(param.id))
                # SR:453 indicates that an InvalidParameters exception should be
                # raised if the input parameter is not of a string type; however,
                # version 2.2.2 of the SCA spec is less strict in its wording. For
                # our part, as long as the value can be stringized, it is accepted,
                # to allow component developers to use more specific types.
                try:
                    args.append(str(param.value.value()))
                except:
                    raise CF.ExecutableDevice.InvalidParameters([param])
        self._log.debug("Popen %s %s", command, args)


        # SR:445
        self._popen_lock.acquire()
        try:
            # Python's subprocess module has a bug where it propagates the exception to
            # the caller when it gets interrupted trying to read the status back from the
            # child process, leaving the child process effectively orphaned and registering
            # a false failure. To work around it, we temporarily replace os.read with a
            # retrying version that allows Popen to succeed in this case.
            class RetryFunc(object):
                def __init__ (self, func):
                    import os
                    self.func = func

                def __call__ (self, *args, **kwargs):
                    while True:
                        try:
                            return self.func(*args, **kwargs)
                        except OSError, e:
                            if e.errno != errno.EINTR:
                                raise

            reader = RetryFunc(os.read)
            os.read = reader

            if self.componentOutputLog in (None, "") or self.useScreen:
                stdoutanderr = None
            else:
                realOutputLog = os.path.expandvars(self.componentOutputLog)
                realOutputLog = self.expandproperties(realOutputLog, parameters)
                stdoutanderr = subprocess.PIPE

            # TODO in 1.7.2 switch to ossie.utils.Popen
            try:
                sp = subprocess.Popen(args, executable=args[0], cwd=os.getcwd(), env=os.environ, close_fds=True, stdin=self._devnull, stdout=stdoutanderr, stderr=subprocess.STDOUT, preexec_fn=os.setpgrp)
            except OSError, e:
                # CF error codes do not map directly to errno codes, so omit the enumerated value.
                self._log.error("subprocess.Popen: %s", e.strerror)
                raise CF.ExecutableDevice.ExecuteFail(CF.CF_NOTSET, e.strerror)

            # Launch a thread to handle re-directing stdout/stderr to a file
            if stdoutanderr != None:
                outputThread = threading.Thread(target=self._handlestdoutanderr, args=(sp, realOutputLog))
                outputThread.setDaemon(True)
                outputThread.start()
        finally:
            os.read = reader.func
            self._popen_lock.release()

        # TODO: SR:455
        # We need to detect failures to execute

        pid = sp.pid
        self._applications[pid] = sp
        # SR:449
        self._log.debug("execute() --> %s", pid)
        self._log.debug("APPLICATIONS %s", self._applications)
        return pid

    def _handlestdoutanderr(self, proc, outputLog):
        self._log.debug("Redirecting stdout/stderr for component (pid %s) to %s", proc.pid, outputLog)
        outputLogFile = open(outputLog, 'a')
        lastFlush = time.time()
        COMPONENT_OUTPUT_LOG_FLUSH_TIME = 2.0 # TODO: Make this a property
        while True:
            # Wait for output from process; wait time based on time to next
            # flush (but no less than 0.1 sec)
            waitTime = max(0.1, time.time() - lastFlush)
            try:
                rlist, wlist, xlist = select.select([proc.stdout], [], [], waitTime)
            except select.error:
                if sys.exc_info()[1][0] == 4:
                    continue
                else:
                    self._log.exception("Error watching process output")
            if proc.stdout in rlist:
                data = os.read(proc.stdout.fileno(), 1024)
                if data == "":
                    # No data is an EOS indication
                    proc.stdout.close()
                    outputLogFile.close()
                    break
                elif os.fstat(outputLogFile.fileno()).st_nlink < 1:
                    # Output log was deleted; re-open it
                    self._log.debug("Detected log file deletion - reopening log %s for component (pid %s)", outputLog, proc.pid)
                    outputLogFile.close()
                    outputLogFile = open(outputLog, 'a')
                elif time.time() - lastFlush >= COMPONENT_OUTPUT_LOG_FLUSH_TIME:
                    # Time to flush output log to disk
                    outputLogFile.flush()
                    lastFlush = time.time()
                outputLogFile.write(data)
            else:
                # Time exceeded waiting for output from component
                # Flush output log to disk
                outputLogFile.flush()
                lastFlush = time.time()
                continue
        self._log.debug("Detected stdout/stderr EOS for component (pid %s)", proc.pid)

    def expandproperties(self, path, parameters):
        """Expand properties in the form @var@.  Unknown variables
        are left unchanged."""
        import re
        _varprog = re.compile(r'\@(\w+)\@')
        i = 0
	_parameters = {}
	for param in parameters:
            _parameters[param.id] = param.value.value()

        while True:
            m = _varprog.search(path, i)
            if not m:
                break
            i, j = m.span(0)
            name = m.group(1)
	    if _parameters.has_key(name):
		tail = path[j:]
		path = path[:i] + str(_parameters[name])
		i = len(path)
		path += tail
	    else:
		i = j
        return path

    ###########################################
    # CF::TestableObject
    ###########################################
    def runTest(self, properties, testid):
        if not isinstance(testid, int) or testid < 1 or testid > 1:
           raise CF.Device.UnknownTest("unknown test: %s" % str(testid))
        if not isinstance(properties, list):
            raise CF.Device.UnknownProperties("properties is not a list")
        if testid == 1:
            return "NOT IMPLEMENTED"

    ###########################################
    # CF::Device
    ###########################################
    def allocateCapacity(self, properties):
        result = GPP_base.allocateCapacity(self, properties)
        # After the allocation send an event for all changed properties
        try:
            self.port_propEvent.sendChangedPropertiesEvent()
        except:
            self._log.exception("Error sending properties event")
        return result

    def deallocateCapacity(self, properties):
        GPP_base.deallocateCapacity(self, properties)
        # After the deallocation send an event for all changed properties
        try:
            self.port_propEvent.sendChangedPropertiesEvent()
        except:
            self._log.exception("Error sending properties event")

    # overrides allocateCapacity for memCapacity
    def allocate_memCapacity(self, value):
        self._log.debug("allocate_memCapacity(%s) Capacity(%s) Free(%s)", value, self.memCapacity, self.memFree)

        # see if calculated capacity and measured capacity is avaliable
        if value > self.memCapacity or value > self.memFree:
            return False

        self.memCapacity = self.memCapacity - value
        self._log.debug("Remaining capacity %s", self.memCapacity)
        return True

    # overrides deallocateCapacity for memCapacity
    def deallocate_memCapacity(self, value):
        self._log.debug("deallocate_memCapacity(%s)", value)
        # make sure this isn't over the actual capacity (due to an update of the memThreshold)
        if self.memCapacity + value > self.memThresholdValue:
            self.memCapacity = self.memThresholdValue
        else:
            self.memCapacity = self.memCapacity + value
        self._log.debug("Remaining capacity %s", self.memCapacity)

    def allocate_processor_cores(self, value):
        self._log.debug("allocate_processor_cores(%s)", value)
        if value <= self.processor_cores:
            return True
        return False

    def allocate_processor_flags(self, values):
        self._log.debug("allocate_processor_flags(%s)", values)
        for value in values:
            if value not in self.processor_flags:
                return False
        return True

    def allocate_loadCapacityPerCore(self, value):
        self._log.debug("allocate_loadCapacityPerCore(%s)", value)
        if value <= self.loadCapacityPerCore:
            return True
        return False

    # overrides allocateCapacity for bogomipsCapacity
    def allocate_loadCapacity(self, value):
        self._log.debug("allocate_loadCapacity(%s)", value)

        # see if the system has enough capacity
        currentLoad = self._loadavg()[0]
        self._log.debug("Current load %s", currentLoad)
        if currentLoad + value > self.loadThresholdValue:
            self._log.warn("allocate load capacity would exceed measured system load %s", currentLoad)

        # see if calculated capacity and measured capacity is avaliable
        if value > self.loadCapacity:
            self._log.debug("allocate load capacity failed due to insufficent capacity %s", self.loadCapacity)
            return False

        self.loadCapacity = self.loadCapacity - value
        return True

    # overrides deallocateCapacity for bogomipsCapacity
    def deallocate_loadCapacity(self, value):
        self._log.debug("deallocate_loadCapacity(%s)", value)
        # make sure this isn't over the actual capacity (due to an update of the bogomipsThreshold)
        if self.loadCapacity + value > self.loadThresholdValue:
            self.loadCapacity = self.loadThresholdValue
        else:
            self.loadCapacity = self.loadCapacity + value


    def allocate_diskCapacity(self, values):
        """
        Allocating disk capacity is simply a check that there is enough disk
        space at the specified paths.  The GPP doesn't actually "reserve"
        capacity because we doing so would assume that the component deletes
        all of it's files on release.
        """
        self._log.debug("allocate_diskCapacity(%s)", values)
        for v in values:
            path = os.path.abspath(os.path.expandvars(v.diskCapacityPath))
            if not os.path.isdir(path):
                self._log.debug(" diskCapacity request %s failed because path is not an existing directory", v)
                return False

            if not os.access(path, os.W_OK):
                self._log.debug(" diskCapacity request %s failed because path is not writable", v)
                return False

            fs = self._findMountFileSystem(path)
            if fs == None:
                self._log.debug(" diskCapacity request %s failed because path is not on a mount point", v)
                return False

            if (fs.available * self.diskThresholdDecimal) < v.diskCapacity:
                self._log.debug(" diskCapacity request %s failed because mount %s only has %s available", v, fs.mount, fs.available)
                return False
            self._log.debug(" diskCapacity request %s succeeded on mount %s", v, fs.mount)

        return True

    def deallocate_diskCapacity(self, value):
        """Deallocate has nothing to do because the capacity check is
        just a comparison against free space."""
        self._log.debug("deallocate_diskCapacity(%s)", value)

    # overrides allocateCapacity for mcastnicCapacity
    def allocate_mcastnicIngressCapacity(self, value):
        self._log.debug("allocate_mcastnicIngressCapacity(%s)", value)

        if self.mcastnicInterface == None:
            self._log.debug(" mcastnicIngressCapacity request failed because no mcastnicInterface has been configured")
            return False

        # see if calculated capacity and measured capacity is avaliable
        if value > self.mcastnicIngressCapacity:
            self._log.debug(" mcastnicIngressCapacity request failed because of insufficent capacity, only %s available", self.mcastnicIngressCapacity)
            return False

        self.mcastnicIngressCapacity = self.mcastnicIngressCapacity - value
        return True

    def allocate_mcastnicEgressCapacity(self, value):
        self._log.debug("allocate_mcastnicEgressCapacity(%s)", value)

        if self.mcastnicInterface == None:
            self._log.debug(" mcastnicEgressCapacity request failed because no mcastnicInterface has been configured")
            return False

        # see if calculated capacity and measured capacity is avaliable
        if value > self.mcastnicEgressCapacity:
            self._log.debug(" mcastnicEgressCapacity request failed because of insufficent capacity, only %s available", self.mcastnicEgressCapacity)
            return False

        self.mcastnicEgressCapacity = self.mcastnicEgressCapacity - value
        return True

    # overrides deallocateCapacity for mcastnicCapacity
    def deallocate_mcastnicIngressCapacity(self, value):
        self._log.debug("deallocate_mcastnicIngressCapacity(%s)", value)
        # make sure this isn't over the actual capacity (due to an update of the mcastnicThreshold)
        if self.mcastnicIngressCapacity + value > self.mcastNicIngressThresholdValue:
            self.mcastnicIngressCapacity = self.mcastNicIngressValue
        else:
            self.mcastnicIngressCapacity = self.mcastnicIngressCapacity + value

    # overrides deallocateCapacity for mcastnicCapacity
    def deallocate_mcastnicEgressCapacity(self, value):
        self._log.debug("deallocate_mcastnicEgressCapacity(%s)", value)
        # make sure this isn't over the actual capacity (due to an update of the mcastnicThreshold)
        if self.mcastnicEgressCapacity + value > self.mcastNicEgressThresholdValue:
            self.mcastnicEgressCapacity = self.mcastNicEgressValue
        else:
            self.mcastnicEgressCapacity = self.mcastnicEgressCapacity + value

    #
    # takes care of usage state
    #
    def updateUsageState(self):
        # Update usage state
        if self.memCapacity == 0 and self.loadCapacity == 0 and self.diskCapacity == 0 and self.mcastnicCapacity == 0:
            self._usageState = CF.Device.BUSY
        elif self.memCapacity == self.memThresholdValue and self.loadCapacity == self.loadThresholdValue: # and self.diskCapacity == self._actualDiskThreshold and self.mcastnicCapacity == self._actualMcastnicThreshold:
            self._usageState = CF.Device.IDLE
        else:
            self._usageState = CF.Device.ACTIVE


    ###########################################
    # system hardware functions
    ###########################################
    def _cpuinfo(self):
        cpuinfo = open("/proc/cpuinfo")
        result = []

        for line in cpuinfo:
            fields = [f.strip() for f in line.split(":", 1)]
            if len(fields) == 1:  # Empty line
                continue

            try:
                name = fields[0]
            except IndexError:
                continue
            try:
                value = fields[1]
            except IndexError:
                continue

            try:
                value = int(value)
            except ValueError:
                try:
                    value = float(value)
                except ValueError:
                    pass

            if name == "processor":
                result.append({})
            result[-1][name] = value

        assert len(result) > 0
        return result

    def _meminfo(self):
        meminfo = open("/proc/meminfo")
        result = {}
        for line in meminfo:
            fields = [f.strip() for f in line.split()]
            try:
                name = fields[0]
            except IndexError:
                continue
            try:
                value = fields[1]
            except IndexError:
                continue
            try:
                units = fields[2]
            except IndexError:
                units = None

            if name[-1] == ":":
                name = name[:-1]

            try:
                value = int(value)
            except ValueError:
                pass

            result[name] = (value, units)
        return result

    def _netstat(self):
        """Identical to netstat -i"""
        netinfo = open("/proc/net/dev").readlines()
        # Skip the first two header lines
        netinfo = netinfo[2:]
        result = {}
        for line in netinfo:
            iface, fields = [f.strip() for f in line.split(":", 1)]
            fields = [f.strip() for f in line.split()]

            result[iface] = fields
        return result

    def _loadavg(self):
        loadavg = open("/proc/loadavg").readlines()[0]
        loadavg = loadavg.split()
        loadavg[0] = float(loadavg[0])
        loadavg[1] = float(loadavg[1])
        loadavg[2] = float(loadavg[2])
        return loadavg

    def _findMountFileSystem(self, path):
        """Given a path, found the mount that includes it"""
        for fs in self.fileSystems:
            try:
                s1 = os.stat(path)
                s2 = os.stat(fs.mount)
            except os.error:
                return None
            dev1 = s1.st_dev
            dev2 = s2.st_dev
            if dev1 == dev2:
                return fs
        return None

    def init_processor_flags(self):
        allCpuFlags = None
        for cpuProps in self._cpuinfo():
            if cpuProps.has_key('flags'):
                currentCpuFlags = cpuProps['flags'].split(' ')
                if allCpuFlags is None:
                    allCpuFlags = currentCpuFlags
                else:
                    allCpuFlags = [x for x in allCpuFlags if x in currentCpuFlags]
            else:
                self.processor_flags = []
        self.processor_flags = allCpuFlags

    def updateMcastVlanTable(self):
        (exitstatus, ifconfig_info) = commands.getstatusoutput('/sbin/ifconfig')
        if exitstatus != 0:
            self._log.debug("Proplem running '/sbin/ifconfig'")
            return

        # clear out old vlan table
        self.vlan_table = []
        # add vlans
        for i in ifconfig_info.splitlines():
            infoline = i.strip()
            if infoline[:3] != 'eth' or infoline.find('Link encap') < 0:
                continue

            ethaddr = infoline[:infoline.find('Link encap')].strip()
            if '.' not in ethaddr:
                # doesn't have a vlan
                continue
            baseinterface, vlan = ethaddr.split('.')
            if baseinterface == self.mcastnicInterface:
                if not vlan.isdigit():
                    self._log.debug("Found invalid vlan: ", vlan)
                    continue
                vlan_num = int(vlan)
                if vlan_num not in self.vlan_table:
                    self.vlan_table.append(vlan_num)

    def checkMountPoint(self, mountPoint):
        if not mountPoint or ":" not in mountPoint:
            self._log.debug("Invalid mount point queried.")
            return False

        (exitstatus, mount_info) = commands.getstatusoutput('mount')
        if exitstatus != 0:
            self._log.debug("Proplem running 'mount'")
            return False

        for i in mount_info.splitlines():
            if i.split()[0] == mountPoint:
                return True

        return False

    #################################
    # process thread
    #################################
    def process(self):
        #TODO: put in code to check on the status of running processes and update device values
        if self.next_property_event == None or self.next_property_event <= time.time():
            self.next_property_event = time.time() + self.propertyEventRate
            try:
                self.port_propEvent.sendChangedPropertiesEvent()
            except:
                self._log.exception("Error sending properties event")
        return NOOP

    # Helper properties to map thresholds to decimal percents
    def get_diskThresholdDecimal(self):
        if self.diskThreshold:
            return self.diskThreshold / 100.0
        else:
            return 1.0
    diskThresholdDecimal = property(fget=get_diskThresholdDecimal)

    def get_loadThresholdDecimal(self):
        if self.loadThreshold:
            return self.loadThreshold / 100.0
        else:
            return 1.0
    loadThresholdDecimal = property(fget=get_loadThresholdDecimal)

    def get_loadThresholdValue(self):
        return (self.loadTotal * self.loadThresholdDecimal)
    loadThresholdValue = property(fget=get_loadThresholdValue)


    def get_memThresholdDecimal(self):
        if self.memThreshold:
            return self.memThreshold / 100.0
        else:
            return 1.0
    memThresholdDecimal = property(fget=get_memThresholdDecimal)

    def get_memThresholdValue(self):
        return int(self.memTotal * self.memThresholdDecimal)
    memThresholdValue = property(fget=get_memThresholdValue)

    def get_mcastNicThresholdDecimal(self):
        if self.mcastnicThreshold:
            return self.mcastnicThreshold / 100.0
        else:
            return 1.0
    mcastNicThresholdDecimal = property(fget=get_mcastNicThresholdDecimal)

    def get_mcastNicIngressThresholdValue(self):
        return (self.mcastnicIngressTotal * self.mcastNicThresholdDecimal)
    mcastNicIngressThresholdValue = property(fget=get_mcastNicIngressThresholdValue)

    def get_mcastNicEgressThresholdValue(self):
        return (self.mcastnicEgressTotal * self.mcastNicThresholdDecimal)
    mcastNicEgressThresholdValue = property(fget=get_mcastNicEgressThresholdValue)

    #################################
    # SCA Property getter/setters
    #################################
    def get_processor_cores(self):
        cpuinfo = self._cpuinfo()
        logical_processors = len(cpuinfo)
        try:
            ht_per_core = (cpuinfo[0]['siblings'] / cpuinfo[0]['cpu cores'])
        except (IndexError, KeyError):
            ht_per_core = 1
        return logical_processors / ht_per_core
    processor_cores = rebind(GPP_base.processor_cores, fget=get_processor_cores)

    def get_memTotal(self):
        meminfo = self._meminfo()
        try:
            memTotal = meminfo['MemTotal'][0]
            units = meminfo['MemTotal'][1]
            assert units == "kB" # We don't expect the Linux kernel to change this

            swapTotal = meminfo['SwapTotal'][0]
            units = meminfo['SwapTotal'][1]
            assert units == "kB" # We don't expect the Linux kernel to change this

            # The kernel reports KiB values but uses the old notation of kB
            value = (memTotal + swapTotal)
            value = value / 1024
            return value
        except KeyError:
            return 0
    memTotal = rebind(GPP_base.memTotal, fget=get_memTotal)

    def get_memFree(self):
        """Don't report MemFree because it includes allocatations that the kernel
        would free up if necessary.  Instead, report MemTotal - Committed_AS
        """
        meminfo = self._meminfo()
        try:
            total = self.memTotal

            try:
                commited_as = meminfo['Committed_AS'][0]
                units = meminfo['Committed_AS'][1]
                assert units == "kB" # We don't expect the Linux kernel to change this
                # The kernel reports KiB values but uses the old notation of kB
                commited_as = commited_as / 1024
                return total - commited_as
            except KeyError:
                # Fall back to MemFree
                value = meminfo['MemFree'][0]
                units = meminfo['MemFree'][1]
                assert units == "kB" # We don't expect the Linux kernel to change this
                # The kernel reports KiB values but uses the old notation of kB
                value = value / 1024
                return value
        except KeyError:
            return 0
    memFree = rebind(GPP_base.memFree, fget=get_memFree)

    def get_fileSystems(self):
        """Use df to provide the current status for all file systems on this machine."""
        # Use the POSIX definition of df for maximum portability; ignore stderr
        # for some systems, such as Ubuntu 14.04, that may print an otherwise
        # harmless error message
        status, output = commands.getstatusoutput("/bin/df -P -k 2>/dev/null")
        output = output.split("\n")
        # Validate the first line looks as expected
        fields = output[0].split()
        if fields != ["Filesystem", "1024-blocks", "Used", "Available", "Capacity", "Mounted", "on"]:
            raise OSError, "Unexpected output from /bin/df...check for POSIX compatibility"

        result = []
        for line in output[1:]:
            if 'Permission denied' in line or \
               'Stale NFS file handle' in line:
                continue
            fields = line.split()
            filesystem = fields[0]
            mounted_on = fields[-1]
            total_mbytes = int(fields[1]) / 1024
            used_mbytes = int(fields[2]) / 1024
            available_mbytes = int(fields[3]) / 1024
            result.append(GPP_base.Filesystem(filesystem, mounted_on, total_mbytes, used_mbytes, available_mbytes))
        return result

    fileSystems = rebind(GPP_base.fileSystems, fget=get_fileSystems)

    def get_mcastnicVLANs(self):
        devices = self._netstat().keys()
        result = []
        for device in devices:
            if not "." in device:
                continue
            else:
                device, vlan = device.split(".", 1)
                if device == self.mcastnicInterface:
                    result.append(vlan)
        return result
    mcastnicVLANs = rebind(GPP_base.mcastnicVLANs, fget=get_mcastnicVLANs)

    def get_mcastnicIngressFree(self):
        return self.mcastnicIngressCapacity
    mcastnicIngressFree = rebind(GPP_base.mcastnicIngressFree, fget=get_mcastnicIngressFree)

    def get_mcastnicEgressFree(self):
        return self.mcastnicEgressCapacity
    mcastnicEgressFree = rebind(GPP_base.mcastnicEgressFree, fget=get_mcastnicEgressFree)

    def get_loadTotal(self):
        return self.processor_cores * self.loadCapacityPerCore
    loadTotal = rebind(GPP_base.loadTotal, fget=get_loadTotal)

    def get_loadAverage(self):
        loadavg = self._loadavg()
        result = GPP_base.LoadAverage()
        result.onemin = loadavg[0]
        result.fivemin = loadavg[1]
        result.fifteenmin = loadavg[2]
        return result
    loadAverage = rebind(GPP_base.loadAverage, fget=get_loadAverage)

    def get_loadFree(self):
        return self.loadCapacity
    loadFree = rebind(GPP_base.loadFree, fget=get_loadFree)

    def get_hostName(self):
        return socket.gethostname()
    hostName = rebind(GPP_base.hostName, fget=get_hostName)

###########################################
# program execution
###########################################
if __name__ == "__main__":
    start_device(GPP)
