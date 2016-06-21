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
import logging
import signal
import time
import commands
import threading
import tempfile
import subprocess
import platform

from ossie.utils import log4py
from ossie import parsers
from ossie.utils.popen import Popen

__all__ = ('LocalProcess', 'VirtualDevice')

log = logging.getLogger(__name__)


class LocalProcess(object):
    STOP_SIGNALS = ((signal.SIGINT, 1),
                    (signal.SIGTERM, 5),
                    (signal.SIGKILL, 0))

    def __init__(self, command, arguments, environment=None, stdout=None):
        self.__terminateRequested = False
        self.__command = command
        self.__arguments = arguments
        log.debug('%s %s', command, ' '.join(arguments))
        self.__process = Popen([command]+arguments, executable=command,
                               cwd=os.getcwd(), env=environment,
                               stdout=stdout, stderr=subprocess.STDOUT,
                               preexec_fn=os.setpgrp)
        self.__tracker = None
        self.__callback = None
        self.__children = []
    
    def setTerminationCallback(self, callback):
        if not self.__tracker:
            # Nothing is currently waiting for notification, start monitor.
            name = 'process-%d-tracker' % self.pid()
            self.__tracker = threading.Thread(name=name, target=self._monitorProcess)
            self.__tracker.daemon = True
            self.__tracker.start()
        self.__callback = callback

    def _monitorProcess(self):
        try:
            status = self.__process.wait()
        except:
            # If wait fails, don't bother with notification.
            return
        if self.__callback:
            self.__callback(self.pid(), status)

    def terminate(self):
        for child in self.__children:
            child.terminate()
        self.__children = []

        for sig, timeout in self.STOP_SIGNALS:
            try:
                log.debug('Killing process group %s with signal %s', self.__process.pid, sig)
                os.killpg(self.__process.pid, sig)
            except OSError:
                pass
            giveup_time = time.time() + timeout
            while self.__process.poll() is None:
                if time.time() > giveup_time:
                    break
                time.sleep(0.1)
            if self.__process.poll() is not None:
                break
        self.__process.wait()
        self.__process = None
    
    def requestTermination(self):
        self.__terminateRequested = True
    
    def command(self):
        return self.__command

    def pid(self):
        if self.__process:
            return self.__process.pid
        else:
            return None

    def isAlive(self):
        return self.__process and self.__process.poll() is None

    def addChild(self, process):
        self.__children.append(process)


class VirtualDevice(object):
    def __init__(self):
        self._processor = platform.machine()
        self._osName = platform.system()

    def getEntryPoint(self, spd, implementation):
        entry_point = implementation.get_code().get_entrypoint()
        if not entry_point.startswith('/'):
            entry_point = os.path.join(os.path.dirname(spd), entry_point)
        return entry_point

    def _matchProcessor(self, implementation):
        for proc in implementation.get_processor():
            if proc.get_name() == self._processor:
                return True
        return False

    def _matchOS(self, implementation):
        for operating_system in implementation.get_os():
            if operating_system.get_name() == self._osName:
                return True
        return False

    def matchImplementation(self, profile, spd):
        for impl in spd.get_implementation():
            if self._matchProcessor(impl) and self._matchOS(impl):
                entry_point = self.getEntryPoint(profile, impl)
                if os.path.exists(entry_point):
                    return impl
        raise RuntimeError, "Softpkg '%s' has no usable implementation" % spd.get_name()

    def execute(self, entryPoint, deps, execparams, debugger, window):
        # Make sure the entry point exists and can be run.
        if not os.path.exists(entryPoint):
            raise RuntimeError, "Entry point '%s' does not exist" % entryPoint
        elif not os.access(entryPoint, os.X_OK|os.R_OK):
            raise RuntimeError, "Entry point '%s' is not executable" % entryPoint
        log.trace("Using entry point '%s'", entryPoint)

        # Process softpkg dependencies and modify the child environment.
        environment = dict(os.environ.items())
        for dependency in deps:
            self._processDependency(environment, dependency)

        for varname in ('LD_LIBRARY_PATH', 'PYTHONPATH', 'CLASSPATH'):
            log.trace('%s=%s', varname, environment.get(varname, '').split(':'))

        # Convert execparams into arguments.
        arguments = []
        for name, value in execparams.iteritems():
            arguments += [name, str(value)]

        if window:
            window_mode = 'monitor'
        else:
            window_mode = None

        if debugger and debugger.modifiesCommand():
            # Run the command in the debugger.
            command, arguments = debugger.wrap(entryPoint, arguments)
            if debugger.isInteractive() and not debugger.canAttach():
                window_mode = 'direct'
        else:
            # Run the command directly.
            command = entryPoint

        stdout = None
        if window_mode == 'monitor':
            # Open up a window for component output.
            try:
                tempdir = tempfile.mkdtemp()
                fifoname = os.path.join(tempdir, 'fifo')
                os.mkfifo(fifoname)
                window_command, window_args = window.command('/usr/bin/tail', ['-n', '+0', '-f', fifoname])
                window_proc = LocalProcess(window_command, window_args)
                stdout = open(fifoname, 'w')
                os.unlink(fifoname)
            except IOError, e:
                pass
        elif window_mode == 'direct':
            # Run the command directly in a window (typically, in the debugger).
            command, arguments = window.command(command, arguments)
        process = LocalProcess(command, arguments, environment, stdout)

        return process

    def _processDependency(self, environment, filename):
        if self._isSharedLibrary(filename):
            self._extendEnvironment(environment, "LD_LIBRARY_PATH", os.path.dirname(filename))
        elif self._isPythonLibrary(filename):
            self._extendEnvironment(environment, "PYTHONPATH", os.path.dirname(filename))
        elif self._isJarfile(filename):
            self._extendEnvironment(environment, "CLASSPATH", filename)
        else:
            self._extendEnvironment(environment, "LD_LIBRARY_PATH", filename)
            self._extendEnvironment(environment, "OCTAVE_PATH", filename)

    def _isSharedLibrary(self, filename):
        status, output = commands.getstatusoutput('nm ' + filename)
        return status == 0

    def _isJarfile(self, filename):
        return filename.endswith('.jar')

    def _isPythonLibrary(self, filename):
        if os.path.splitext(filename)[1] in ('.py', '.pyc', '.pyo'):
            # File is a Python module
            return True
        elif os.path.isdir(filename):
            # Check for Python package
            initpath = os.path.join(filename, '__init__.py')
            for initfile in (initpath, initpath+'c', initpath+'o'):
                if os.path.exists(initfile):
                    return True
        return False

    def _extendEnvironment(self, env, keyname, value):
        if keyname not in env:
            env[keyname] = value
        else:
            oldvalue = env[keyname].split(':')
            if value in oldvalue:
                # Path is already in list.
                return
            oldvalue.insert(0,value)
            env[keyname] = ':'.join(oldvalue)
