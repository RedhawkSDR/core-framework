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
import fnmatch
import time

from ossie import parsers
from ossie.utils.model import Service, Resource, Device
from ossie.utils.model.connect import ConnectionManager

from base import SdrRoot, Sandbox, SandboxComponent
import launcher

log = logging.getLogger(__name__)

class LocalSdrRoot(SdrRoot):
    def __init__(self, sdrroot):
        self.__sdrroot = sdrroot

    def _sdrPath(self, filename):
        return os.path.join(self.__sdrroot, filename)

    def _fileExists(self, filename):
        path = os.path.isfile(self._sdrPath(filename))
        # Try relative path if sdrPath fails
        if not path:
            path = os.path.isfile(filename)
        
        return path

    def _readFile(self, filename):
        try:
            path = open(self._sdrPath(filename), 'r')
        except:
            # Try relative path if sdrPath fails
            path = open(filename, 'r')
        return path.read()


    def getProfiles(self, objType=None):
        files = []
        searchPath = []
        if objType == "component":
            searchPath.append(os.path.join(self.__sdrroot, 'dom', 'components'))
        elif objType == "device":
            searchPath.append(os.path.join(self.__sdrroot, 'dev', 'devices'))
        elif objType == "service":
            searchPath.append(os.path.join(self.__sdrroot, 'dev', 'services'))
        elif objType == None:
            searchPath.append(os.path.join(self.__sdrroot, 'dom', 'components'))
            searchPath.append(os.path.join(self.__sdrroot, 'dev', 'devices'))
            searchPath.append(os.path.join(self.__sdrroot, 'dev', 'services'))
        else:
            raise ValueError, "'%s' is not a valid object Type" % objType
        for path in searchPath:
            for root, dirs, fnames in os.walk(path):
                for filename in fnmatch.filter(fnames, '*.spd.xml'):
                    files.append(os.path.join(root, filename))
        return files

    def getLocation(self):
        return self.__sdrroot

    def findProfile(self, descriptor, objType=None):
        if not descriptor:
            raise RuntimeError, 'No component descriptor given'
        # Override base class behavior if descriptor points to a file regardless
        # of SDRROOT setting.
        if os.path.isfile(descriptor):
            try:
                spd = parsers.spd.parseString(self._readFile(descriptor))
                log.trace("Descriptor '%s' is file name", descriptor)
                return os.path.abspath(descriptor)
            except:
                pass
        return super(LocalSdrRoot,self).findProfile(descriptor, objType=objType)


class LocalMixin(object):
    def __init__(self, execparams, debugger, window, timeout):
        self._process = None
        self._execparams = execparams
        self._debugger = debugger
        self._window = window
        self._timeout = timeout

    def _launch(self):
        launchFactory = self.__launcher__(self._profile, self._refid, self._instanceName, self._sandbox)
        execparams = self._getExecparams()
        execparams.update(self._execparams)
        proc, ref = launchFactory.execute(self._spd, self._impl, execparams, self._debugger, self._window, self._timeout)
        self._process = proc
        return ref

    def _getExecparams(self):
        return {}

    def _terminate(self):
        if self._process:
            # Kill child process (may be multiple processes in the case of a debugger)
            self._process.terminate()
            self._process = None

    def _processAlive(self):
        if self._process:
            return self._process.isAlive()
        else:
            return False

class LocalSandboxComponent(SandboxComponent, LocalMixin):
    def __init__(self, sdrroot, profile, spd, scd, prf, instanceName, refid, impl,
                 execparams, debugger, window, timeout):
        SandboxComponent.__init__(self, sdrroot, profile, spd, scd, prf, instanceName, refid, impl)
        LocalMixin.__init__(self, execparams, debugger, window, timeout)
        
        self._kick()

        self._parseComponentXMLFiles()
        self._buildAPI()

    def _getExecparams(self):
        return dict((str(ep.id), ep.defValue) for ep in self._getPropertySet(kinds=('execparam',), includeNil=False))

    def releaseObject(self):
        try:
            super(LocalSandboxComponent,self).releaseObject()
        except:
            # Tolerate exceptions (e.g., the object has already been released)
            # and continue on to ensure that the process still gets terminated.
            pass

        # Give the process a little time (50ms) to exit after releaseObject()
        # returns before sending termination signals
        timeout = 50e-3
        end = time.time() + timeout
        while self._processAlive() and time.time() < end:
            time.sleep(1e-3)

        self._terminate()


class LocalComponent(LocalSandboxComponent, Resource):
    __launcher__ = launcher.ResourceLauncher

    def __init__(self, *args, **kwargs):
        Resource.__init__(self)
        LocalSandboxComponent.__init__(self, *args, **kwargs)
    def __repr__(self):
        return "<local component '%s' at 0x%x>" % (self._instanceName, id(self))


class LocalDevice(LocalSandboxComponent, Device):
    __launcher__ = launcher.DeviceLauncher

    def __init__(self, *args, **kwargs):
        Device.__init__(self)
        LocalSandboxComponent.__init__(self, *args, **kwargs)

        Device._buildAPI(self)

    def __repr__(self):
        return "<local device '%s' at 0x%x>" % (self._instanceName, id(self))

    def api(self):
        LocalSandboxComponent.api(self)
        print
        Device.api(self)


class LocalService(Service, LocalMixin):
    __launcher__ = launcher.ServiceLauncher
    
    def __init__(self, sdrroot, profile, spd, scd, prf, instanceName, refid, impl,
                 execparams, debugger, window, timeout):
        self._sandbox = sdrroot
        Service.__init__(self, None, profile, spd, scd, prf, instanceName, refid, impl)
        LocalMixin.__init__(self, execparams, debugger, window, timeout)
        self.ref = self._launch()
        self.populateMemberFunctions()
        
        self._sandbox._addService(self)

    def _getExecparams(self):
        if not self._prf:
            return {}
        execparams = {}
        for prop in self._prf.get_simple():
            # Skip non-execparam properties
            kinds = set(k.get_kindtype() for k in prop.get_kind())
            if 'execparam' not in kinds:
                continue
            # Only include execparam properties with values
            value = prop.get_value()
            if value is not None:
                execparams[prop.get_id()] = value
        return execparams

    def __repr__(self):
        return "<local service '%s' at 0x%x>" % (self._instanceName, id(self))


class LocalSandbox(Sandbox):
    __comptypes__ = {
        'resource':         LocalComponent,
        'device':           LocalDevice,
        'loadabledevice':   LocalDevice,
        'executabledevice': LocalDevice,
        'service':          LocalService
        }
    
    def __init__(self, sdrroot):
        super(LocalSandbox, self).__init__(autoInit=True)
        self.__components = {}
        self.__services = {}
        self._sdrroot = LocalSdrRoot(sdrroot)

    def _getComponentContainer(self, componentType):
        if componentType == 'service':
            return self.__services
        else:
            return self.__components

    def _createInstanceName(self, softpkgName, componentType='resource'):
        # Use one-up counter to make instance name unique.
        container = self._getComponentContainer(componentType)
        counter = len(container) + 1
        while True:
            name = '%s_%d' % (softpkgName.replace('.','_'), counter)
            if name not in container:
                return name
            counter += 1

    def _checkInstanceName(self, instanceName, componentType='resource'):
        # Ensure instance name is unique.
        container = self._getComponentContainer(componentType)
        return instanceName not in container

    def _checkInstanceId(self, refid, componentType):
        # Ensure refid is unique.
        container = self._getComponentContainer(componentType)
        for component in container.values():
            if refid == component._refid:
                return False
        return True

    def getComponents(self):
        return self.__components.values()

    def getServices(self):
        return self.__services.values()

    def _registerComponent(self, component):
        # Add the component to the sandbox state.
        self.__components[component._instanceName] = component
    
    def _unregisterComponent(self, component):
        name = component._instanceName
        if name in self.__components:
            del self.__components[name]
    
    def _addService(self, service):
        self.__services[service._instanceName] = service
        
    def getComponent(self, name):
        return self.__components.get(name, None)

    def getComponentByRefid(self, refid):
        for component in self.__components.itervalues():
            if refid == component._refid:
                return component
        return None

    def getService(self, name):
        return self.__services.get(name, None)

    def getSdrRoot(self):
        return self._sdrroot

    def setSdrRoot(self, path):
        # Validate new root.
        if not os.path.isdir(path):
            raise RuntimeError, 'invalid SDRROOT, directory does not exist'
        if not os.path.isdir(os.path.join(path, 'dom')):
            raise RuntimeError, 'invalid SDRROOT, dom directory does not exist'
        if not os.path.isdir(os.path.join(path, 'dev')):
            raise RuntimeError, 'invalid SDRROOT, dev directory does not exist'
        self._sdrroot = LocalSdrRoot(path)

    def shutdown(self):
        ConnectionManager.instance().cleanup()
        self.stop()
        for name, component in self.__components.items():
            log.debug("Releasing component '%s'", name)
            try:
                component.releaseObject()
            except:
                log.debug("Component '%s' raised an exception while exiting", name)
        for name, service in self.__services.items():
            log.debug("Terminating service '%s'", name)
            try:
                service._terminate()
            except:
                log.debug("Service '%s' raised an exception while terminating", name)
        self.__components = {}
        self.__services = {}
        super(LocalSandbox,self).shutdown()

    def catalog(self, searchPath=None, objType="components"):
        files = {}
        if not searchPath:
            if objType == "components":
                searchPath = os.path.join(self.getSdrRoot().getLocation(), 'dom', 'components')
            elif objType == "devices":
                searchPath = os.path.join(self.getSdrRoot().getLocation(), 'dev', 'devices')
            elif objType == "services":
                searchPath = os.path.join(self.getSdrRoot().getLocation(), 'dev', 'services')
            else:
                raise ValueError, "'%s' is not a valid object type" % objType
        for root, dirs, fnames in os.walk(searchPath):
            for filename in fnmatch.filter(fnames, "*spd.xml"):
                filename = os.path.join(root, filename)
                try:
                    spd = parsers.spd.parse(filename)
                    files[str(spd.get_name())] = filename
                except:
                    log.exception('Could not parse %s', filename)

        return files
