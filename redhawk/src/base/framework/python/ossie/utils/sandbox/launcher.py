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
import threading
import tempfile
import subprocess
import platform
import zipfile
from ossie.utils.sandbox.cluster import cluster
from .process import LocalProcess

from ossie.utils import log4py
from ossie import parsers
from ossie.utils.popen import Popen
from ossie.utils import uuid as _uuid

__all__ = ('VirtualDevice')

log = logging.getLogger(__name__)

class VirtualDevice(object):
    def __init__(self):
        self._processor = platform.machine()
        if self._processor == 'i686':
            # Map from Linux standard machine name to REDHAWK
            self._processor = 'x86'
        self._osName = platform.system()

        log.debug("VirtualDevice processor '%s' OS '%s'", self._processor, self._osName)

    def _matchProcessor(self, implementation):
        if not implementation.get_processor():
            # Implementation specifies no processor dependency
            return True

        for proc in implementation.get_processor():
            if proc.get_name() == self._processor:
                return True
        return False

    def _matchOS(self, implementation):
        if not implementation.get_os():
            # Implementation specifies no OS dependency
            return True

        for operating_system in implementation.get_os():
            if operating_system.get_name() == self._osName:
                return True
        return False

    def _checkImplementation(self, sdrroot, profile, impl):
        # Match device properties
        log.trace("Checking processor and OS for implementation '%s'", impl.get_id())
        if not self._matchProcessor(impl) or not self._matchOS(impl):
            return False

        # Check that localfile points to a real location
        localfile = impl.get_code().get_localfile().get_name()
        filename = sdrroot.relativePath(profile, localfile)
        log.trace("Checking localfile '%s' ('%s')", localfile, filename)
        if impl.get_code().get_type() not in "Container":
            if not os.path.exists(filename):
                return False

        # If the implementation has an entry point, make sure it exists too
        if impl.get_code().get_entrypoint():
            entry_point = impl.get_code().get_entrypoint().split('::')[0]

            if impl.get_code().get_type() not in "Container":
                filename = sdrroot.relativePath(profile, entry_point)
                if not os.path.exists(filename):
                    return False
            else:
                # no way to check the content of the container without running it
                pass
            log.trace("Checking entrypoint '%s' ('%s')", entry_point, filename)

        return True

    def matchImplementation(self, sdrroot, profile, spd):
        for impl in spd.get_implementation():
            if self._checkImplementation(sdrroot, profile, impl):
                return impl
        raise RuntimeError("Softpkg '%s' has no usable implementation" % spd.get_name())

    def getExecArgs(self, entryPoint, deps, execparams, debugger, window, stdout=None, isContainer=False):        
        # Make sure the entry point exists and can be run.
        if not isContainer:
            if not os.path.exists(entryPoint):
                raise RuntimeError("Entry point '%s' does not exist" % entryPoint)
            elif not os.access(entryPoint, os.X_OK|os.R_OK):
                raise RuntimeError("Entry point '%s' is not executable" % entryPoint)
        log.trace("Using entry point '%s'", entryPoint)

        # Process softpkg dependencies and modify the child environment.
        environment = dict(list(os.environ.items()))
        for dependency in deps:
            self._processDependency(environment, dependency)

        for varname in ('LD_LIBRARY_PATH', 'PYTHONPATH', 'CLASSPATH'):
            log.trace('%s=%s', varname, environment.get(varname, '').split(':'))

        # Convert execparams into arguments.
        arguments = []
        for name, value in execparams.items():
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
        if debugger:
            environment.update(debugger.envUpdate())

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
            except IOError as e:
                pass
        elif window_mode == 'direct':
            # Run the command directly in a window (typically, in the debugger).
            command, arguments = window.command(command, arguments)

        return command, arguments, environment, stdout

    def execute(self, entryPoint, deps, execparams, debugger, window, stdout=None):

        command, arguments, environment, stdout = self.getExecArgs(entryPoint, deps, execparams, debugger, window, stdout)

        process = LocalProcess(command, arguments, environment, stdout)

        return process

    def executeContainer(self, entryPoint_image, deps, execparams, debugger, window, stdout=None, orchestrationType=None, componenthostid=None):
        image = ""
        if "::" in entryPoint_image:
            entryPoint = entryPoint_image.split("::")[0]
            image      = entryPoint_image.split("::")[1]
        else:
            raise RuntimeError("No docker image was found in the spd file")

        command, arguments, environment, stdout = self.getExecArgs(entryPoint, deps, execparams, debugger, window, stdout, True)

        if command[-3:] == '.so':
            c_command, c_arguments, c_environment, c_stdout = command, arguments, environment, stdout
            command = "/var/redhawk/sdr/dom/mgr/rh/ComponentHost/ComponentHost"
            arguments[1] = 'DCE:'+str(_uuid.uuid4())
            arguments[5] = "/var/redhawk/sdr/dom/mgr/rh/ComponentHost/ComponentHost.spd.xml"
            arguments[7] = componenthostid

        process = cluster.executeCluster(command, arguments, image, environment, stdout, orchestrationType)
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
        try:
            with open(filename, 'rb') as f:
                return f.read(4) == '\x7fELF'
        except:
            return False

    def _isJarfile(self, filename):
        return filename.endswith('.jar') and zipfile.is_zipfile(filename)

    def _isPythonLibrary(self, filename):
        PYTHON_EXTS = ('.py', '.pyc', '.pyo')
        if os.path.splitext(filename)[1] in PYTHON_EXTS:
            # File is a Python module
            return True
        elif os.path.isdir(filename):
            # Check for Python package
            initpath = os.path.join(filename, '__init__')
            for ext in PYTHON_EXTS:
                if os.path.exists(initpath + ext):
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
