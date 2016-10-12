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
import copy
import warnings

from ossie import parsers
from ossie.utils.model import Service, Resource, Device
from ossie.utils.model.connect import ConnectionManager

from base import SdrRoot, Sandbox, SandboxComponent
import launcher
import pydoc

log = logging.getLogger(__name__)

class LocalSdrRoot(SdrRoot):
    def __init__(self, sdrroot):
        self.__sdrroot = sdrroot

    def _sdrPath(self, filename):
        # Give precedence to filenames that are valid as-is
        if os.path.isfile(filename):
            return filename
        # Assume the filename points to somewhere in SDRROOT
        return os.path.join(self.__sdrroot, filename)

    def _fileExists(self, filename):
        return os.path.isfile(filename)

    def _readFile(self, filename):
        path = open(self._sdrPath(filename), 'r')
        return path.read()

    def _getSearchPaths(self, objTypes):
        paths = []
        if 'components' in objTypes:
            paths.append('dom/components')
        if 'devices' in objTypes:
            paths.append('dev/devices')
        if 'services' in objTypes:
            paths.append('dev/services')
        return [self._sdrPath(p) for p in paths]

    def _getAvailableProfiles(self, path):
        files = []
        for root, dirs, fnames in os.walk(path):
            for filename in fnmatch.filter(fnames, '*.spd.xml'):
                files.append(os.path.join(root, filename))
        return files

    def _getObjectTypes(self, objType):
        if objType == 'component':
            warnings.warn("objType='component' is deprecated; use 'components'", DeprecationWarning)
            objType = 'components'
        elif objType == 'device':
            warnings.warn("objType='device' is deprecated; use 'devices'", DeprecationWarning)
            objType = 'devices'
        elif objType == 'service':
            warnings.warn("objType='service' is deprecated; use 'services'", DeprecationWarning)
            objType = 'services'
        return super(LocalSdrRoot,self)._getObjectTypes(objType)

    def getLocation(self):
        return self.__sdrroot

    def findProfile(self, descriptor, objType=None):
        if not descriptor:
            raise RuntimeError, 'No component descriptor given'
        # Handle user home directory paths
        if descriptor.startswith('~'):
            descriptor = os.path.expanduser(descriptor)
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
        self._pid = self._process.pid()
        return ref
    
    def _requestTermination(self):
        self._process.requestTermination()

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
        execparams = dict((str(ep.id), ep.defValue) for ep in self._getPropertySet(kinds=('execparam',), includeNil=False))
        commandline_property = dict((str(ep.id), ep.defValue) for ep in self._getPropertySet(kinds=('property',), includeNil=False,commandline=True))
        execparams.update(commandline_property)
        return execparams

    def releaseObject(self):
        try:
            self._requestTermination()
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
            if ('execparam' not in kinds) and ('property' not in kinds):
                continue
            if 'property' in kinds:
                if prop.get_commandline() == 'false':
                    continue
            # Only include properties with values
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
        super(LocalSandbox, self).__init__()
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

    def _launch(self, profile, spd, scd, prf, instanceName, refid, impl, execparams,
                initProps, initialize, configProps, debugger, window, timeout):
        # Determine the class for the component type and create a new instance.
        comptype = scd.get_componenttype()
        clazz = self.__comptypes__[comptype]
        comp = clazz(self, profile, spd, scd, prf, instanceName, refid, impl, execparams, debugger, window, timeout)

        try:
            # Occasionally, when a lot of components are launched from the
            # sandbox, omniORB may have a cached connection where the other end
            # has terminated (this is particularly a problem with Java, because
            # the Sun ORB never closes connections on shutdown). If the new
            # component just happens to have the same TCP/IP address and port,
            # the first time we try to reach the component, it will get a
            # CORBA.COMM_FAILURE exception even though the reference is valid.
            # In this case, a call to _non_existent() should cause omniORB to
            # clean up the stale socket, and subsequent calls behave normally.
            comp.ref._non_existent()
        except:
            pass

        # Services don't get initialized or configured
        if comptype == 'service':
            return comp

        # Initialize the component unless asked not to.
        if initialize:
            # Set initial property values for 'property' kind properties
            initvals = copy.deepcopy(comp._propRef)
            initvals.update(initProps)
            try:
                comp.initializeProperties(initvals)
            except:
                log.exception('Failure in component property initialization')

            # Actually initialize the component
            comp.initialize()

        # Configure component with default values unless requested not to (e.g.,
        # when launched from a SAD file).
        if configProps is not None:
            # Make a copy of the default properties, and update with any passed-in
            # properties that were not already passed to initializeProperties()
            initvals = copy.deepcopy(comp._configRef)
            initvals.update(configProps)
            try:
                comp.configure(initvals)
            except:
                log.exception('Failure in component configuration')

        return comp

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

    def retrieve(self, name):
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

    def browse(self, searchPath=None, objType=None,withDescription=False):
        if not searchPath:
            if objType == None or objType == "all":
                pathsToSearch = [os.path.join(self.getSdrRoot().getLocation(), 'dom', 'components'), \
                               os.path.join(self.getSdrRoot().getLocation(), 'dev', 'devices'), \
                               os.path.join(self.getSdrRoot().getLocation(), 'dev', 'services')]
            elif objType == "components":
                pathsToSearch = [os.path.join(self.getSdrRoot().getLocation(), 'dom', 'components')]
            elif objType == "devices":
                pathsToSearch = [os.path.join(self.getSdrRoot().getLocation(), 'dev', 'devices')]
            elif objType == "services":
                pathsToSearch = [os.path.join(self.getSdrRoot().getLocation(), 'dev', 'services')]
            else:
                raise ValueError, "'%s' is not a valid object type" % objType
        else:
            pathsToSearch = [searchPath]

        output_text = ""
        for path in pathsToSearch:
            rsrcDict = {}
            path.rstrip("/")
            objType = path.split("/")[-1]
            if objType == "components":
                pathPrefix = "$SDRROOT/dom/components"
            elif objType == "devices":
                pathPrefix = "$SDRROOT/dev/devices"
            elif objType == "services":
                pathPrefix = "$SDRROOT/dev/services"
            else:
                pathPrefix = path

            for root, dirs, fnames in os.walk(path):
                for filename in fnmatch.filter(fnames, "*spd.xml"):
                    filename = os.path.join(root, filename)
                    try:
                        spd = parsers.spd.parse(filename)
                        full_namespace = root[root.find(path)+len(path)+1:]
                        namespace = full_namespace[:full_namespace.find(spd.get_name())]
                        if namespace == '':
                            namespace = pathPrefix
                        if rsrcDict.has_key(namespace) == False:
                            rsrcDict[namespace] = []
                        if withDescription == True:
                            new_item = {}
                            new_item['name'] = spd.get_name()
                            if spd.description == None:
                                if spd.get_implementation()[0].description == None or \
                                   spd.get_implementation()[0].description == "The implementation contains descriptive information about the template for a software component.":
                                    new_item['description'] = None
                                else:
                                    new_item['description'] = spd.get_implementation()[0].description.encode("utf-8")
                            else:
                                new_item['description'] = spd.description
                            rsrcDict[namespace].append(new_item)
                        else:
                            rsrcDict[namespace].append(spd.get_name())
                    except Exception, e:
                        print str(e)
                        print 'Could not parse %s', filename

            for key in sorted(rsrcDict.iterkeys()):
                if key == pathPrefix:
                    output_text += "************************ " + str(key) + " ***************************\n"
                else:
                    output_text += "************************ " + str(pathPrefix + "/" + key) + " ***************************\n"

                value = rsrcDict[key]
                value.sort()
                if withDescription == True:
                    for item in value:
                        if item['description']:
                            output_text += str(item['name']) + " - " + str(item['description']) + "\n"
                        else:
                            output_text += str(item['name']) + "\n"
                        output_text += "--------------------------------------\n"
                else:
                    l = value
                    while len(l) % 4 != 0:
                        l.append(" ")

                    split = len(l)/4

                    l1 = l[0:split]
                    l2 = l[split:2*split]
                    l3 = l[2*split:3*split]
                    l4 = l[3*split:4*split]
                    for v1,v2,v3,v4 in zip(l1,l2,l3,l4):
                        output_text += '%-30s%-30s%-30s%-30s\n' % (v1,v2,v3,v4)
                    output_text += "\n"
        pydoc.pager(output_text)
