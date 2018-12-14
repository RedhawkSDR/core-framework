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
import pydoc
import warnings

from omniORB import CORBA
from omniORB.any import to_any

from ossie import parsers
from ossie.cf import CF
from ossie.utils.model.connect import ConnectionManager
from ossie.utils.uuid import uuid4

from base import SdrRoot, Sandbox, SandboxLauncher, SandboxComponent
from devmgr import DeviceManagerStub
from naming import ApplicationRegistrarStub
import launcher
from debugger import GDB, JDB, PDB, Valgrind, Debugger
import terminal

warnings.filterwarnings('once',category=DeprecationWarning)

# Prepare the ORB
orb = CORBA.ORB_init()
poa = orb.resolve_initial_references("RootPOA")
poa._get_the_POAManager().activate()

log = logging.getLogger(__name__)

class LocalSdrRoot(SdrRoot):
    def __init__(self, sdrroot):
        self.__sdrroot = sdrroot

    def _sdrPath(self, filename):
        # Give precedence to filenames that are valid as-is
        if os.path.isfile(filename):
            # Convert to an absolute path, to avoid any problems with relative
            # paths when passed to other contexts
            return os.path.abspath(filename)
        # Assume the filename points to somewhere in SDRROOT
        return os.path.join(self.__sdrroot, filename)

    def domPath(self, filename):
        return os.path.join(self.__sdrroot, 'dom' + filename)

    def relativePath(self, base, path):
        if path.startswith('/'):
            return self.domPath(path)
        else:
            return os.path.join(os.path.dirname(base), path)

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


class LocalLauncher(SandboxLauncher):
    def __init__(self, execparams, initProps, initialize, configProps, debugger, window, timeout, shared, stdout=None):
        self._execparams = execparams
        self._debugger = debugger
        self._window = window
        self._initProps = initProps
        self._initialize = initialize
        self._configProps = configProps
        self._timeout = timeout
        self._shared = shared
        self._stdout = stdout

    def _getImplementation(self, spd, identifier):
        for implementation in spd.get_implementation():
            if implementation.get_id() == identifier:
                return implementation
        raise KeyError, "Softpkg '%s' has no implementation '%s'" % (spd.get_name(), identifier)

    def _resolveDependencies(self, sdrRoot, device, implementation):
        dep_files = []
        for dependency in implementation.get_dependency():
            softpkg = dependency.get_softpkgref()
            if not softpkg:
                continue
            filename = softpkg.get_localfile().get_name()
            log.trace("Resolving softpkg dependency '%s'", filename)
            local_filename = sdrRoot.domPath(filename)
            dep_spd = parsers.spd.parse(local_filename)
            dep_impl = softpkg.get_implref()
            if dep_impl:
                impl = self._getImplementation(dep_spd, dep_impl.get_refid())
            else:
                # No implementation requested, find one that matches the device
                impl = device.matchImplementation(sdrRoot, local_filename, dep_spd)

            log.trace("Using implementation '%s'", impl.get_id())
            dep_localfile = impl.get_code().get_localfile().get_name()
            dep_files.append(sdrRoot.relativePath(local_filename, dep_localfile))

            # Resolve nested dependencies.
            dep_files.extend(self._resolveDependencies(sdrRoot, device, impl))

        return dep_files

    def _cleanHeap(self, pid):
        filename = '/dev/shm/heap-'+str(pid)
        if (os.path.isfile(filename)):
            os.remove(filename)

    def launch(self, comp):
        # Build up the full set of command line arguments
        execparams = comp._getExecparams()
        execparams.update(self._execparams)
        execparams.update(self._getRequiredExecparams(comp))

        # Set up the debugger if requested
        debugger = self._debugger
        try:
            if isinstance(debugger, basestring):
                if debugger == 'pdb':
                    debugger = PDB()
                elif debugger == 'jdb':
                    debugger = JDB()
                elif debugger == 'gdb':
                    debugger = GDB()
                elif debugger == 'valgrind':
                    debugger = Valgrind()
            elif isinstance(debugger, Debugger):
                # check for PDB, JDB, Valgrind, or GDB
                pass
            elif debugger is None:
                pass
            else:
                raise RuntimeError, 'not supported'
        except Exception, e:
            log.warning('Cannot run debugger %s (%s)', debugger, e)
            debugger = None

        # If using an interactive debugger that directly runs the command, put
        # it in a window so it doesn't compete for the terminal.
        window = self._window
        if debugger and debugger.modifiesCommand():
            if debugger.isInteractive() and not debugger.canAttach():
                if not window:
                    window = 'xterm'

        # Allow symbolic names for windows
        if isinstance(window, basestring):
            try:
                if window == 'xterm':
                    window = terminal.XTerm(comp._instanceName)
                elif window == 'gnome-terminal':
                    window = terminal.GnomeTerm(comp._instanceName)
                else:
                    raise RuntimeError, 'not supported'
            except Exception, e:
                log.warning('Cannot run terminal %s (%s)', window, e)
                debugger = None

        # Find a suitable implementation
        device = launcher.VirtualDevice()
        sdrroot = comp._sandbox.getSdrRoot()
        if comp._impl:
            impl = self._getImplementation(comp._spd, comp._impl)
        else:
            impl = device.matchImplementation(sdrroot, comp._profile, comp._spd)
        log.trace("Using implementation '%s'", impl.get_id())

        # Resolve all dependency localfiles
        deps = self._resolveDependencies(sdrroot, device, impl)

        # Execute the entry point, either on the virtual device or the Sandbox
        # component host
        entry_point = sdrroot.relativePath(comp._profile, impl.get_code().get_entrypoint())
        if impl.get_code().get_type() == 'SharedLibrary':
            if self._shared:
                container = comp._sandbox._getComponentHost(_debugger = debugger)
            else:
                container = comp._sandbox._launchComponentHost(comp._instanceName, _debugger = debugger)
            container.executeLinked(entry_point, [], execparams, deps)
            process = container._process
        else:
            process = device.execute(entry_point, deps, execparams, debugger, window, self._stdout)

            # Set up a callback to notify when the component exits abnormally.
            name = comp._instanceName
            def terminate_callback(pid, status):
                self._cleanHeap(pid)
                if status > 0:
                    print 'Component %s (pid=%d) exited with status %d' % (name, pid, status)
                elif status < 0:
                    print 'Component %s (pid=%d) terminated with signal %d' % (name, pid, -status)
            process.setTerminationCallback(terminate_callback)

        # Wait for the component to register with the virtual naming service or
        # DeviceManager.
        if self._timeout is None:
            # Default timeout depends on whether the debugger might increase
            # the startup time
            if debugger and debugger.modifiesCommand():
                timeout = 60.0
            else:
                timeout = 10.0
        else:
            timeout = self._timeout
        sleepIncrement = 0.1
        while self.getReference(comp) is None:
            if not process.isAlive():
                raise RuntimeError, "%s '%s' terminated before registering with virtual environment" % (self._getType(), comp._instanceName)
            time.sleep(sleepIncrement)
            timeout -= sleepIncrement
            if timeout < 0:
                process.terminate()
                raise RuntimeError, "%s '%s' did not register with virtual environment"  % (self._getType(), comp._instanceName)

        # Attach a debugger to the process.
        if debugger and debugger.canAttach():
            if not window:
                window = terminal.XTerm('%s (%s)' % (debugger.name(), comp._instanceName))
            debug_command, debug_args = debugger.attach(process)
            debug_command, debug_args = window.command(debug_command, debug_args)
            debug_process = launcher.LocalProcess(debug_command, debug_args)
            process.addChild(debug_process)

        # Store the process on the component proxy.
        if impl.get_code().get_type() == 'SharedLibrary' and self._shared:
            comp._process = None
            comp._pid = None
        else:
            comp._process = process
            comp._pid = process.pid()

        # Return the now-resolved CORBA reference.
        ref = self.getReference(comp)
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
            ref._non_existent()
        except:
            pass
        return ref

    def setup(self, comp):
        # Initialize the component unless asked not to.
        if self._initialize:
            # Set initial property values for 'property' kind properties
            initvals = comp._getInitializeProperties()
            initvals.update(self._initProps)
            try:
                comp.initializeProperties(initvals)
            except:
                log.exception('Failure in component property initialization')

            # Actually initialize the component
            comp.initialize()

        # Configure component with default values unless requested not to (e.g.,
        # when launched from a SAD file).
        if self._configProps is not None:
            # Set initial configuration properties (pre-2.0 components)
            initvals = comp._getInitialConfigureProperties()
            initvals.update(self._configProps)
            try:
                comp.configure(initvals)
            except:
                log.exception('Failure in component configuration')

    def terminate(self, comp):
        if comp._process:
            # Give the process a little time (50ms) to exit after releaseObject()
            # returns before sending termination signals
            timeout = 50e-3
            end = time.time() + timeout
            while comp._process.isAlive() and time.time() < end:
                time.sleep(1e-3)

            # Kill child process (may be multiple processes in the case of a debugger)
            comp._process.terminate()
            comp._process = None


class LocalComponentLauncher(LocalLauncher):
    def launch(self, *args, **kwargs):
        self.__registrar = ApplicationRegistrarStub()
        log.trace('Activating virtual ApplicationRegistrar')
        namingContextId = poa.activate_object(self.__registrar)
        try:
            return LocalLauncher.launch(self, *args, **kwargs)
        finally:
            log.trace('Deactivating virtual ApplicationRegistrar')
            poa.deactivate_object(namingContextId)
            del self.__registrar

    def getReference(self, component):
        return self.__registrar.getObject(component._instanceName)

    def _getRequiredExecparams(self, component):
        return {'COMPONENT_IDENTIFIER': component._refid,
                'NAMING_CONTEXT_IOR': orb.object_to_string(self.__registrar._this()),
                'PROFILE_NAME': component._profile,
                'NAME_BINDING': component._instanceName}

    def _getType(self):
        return 'resource'


class LocalDeviceLauncher(LocalLauncher):
    def getReference(self, device):
        return DeviceManagerStub.instance().getDevice(device._refid)

    def _getRequiredExecparams(self, device):
        devmgr_stub = DeviceManagerStub.instance()
        devmgr_ior = orb.object_to_string(devmgr_stub._this())
        # Create (or reuse) IDM channel.
        idm_channel = device._sandbox.createEventChannel('IDM_Channel')
        idm_ior = orb.object_to_string(idm_channel.ref)

        return {'DEVICE_ID': device._refid,
                'DEVICE_LABEL': device._instanceName,
                'DEVICE_MGR_IOR': devmgr_ior,
                'IDM_CHANNEL_IOR': idm_ior,
                'PROFILE_NAME': device._profile}

    def _getType(self):
        return 'device'


class LocalServiceLauncher(LocalLauncher):
    def setup(self, service):
        # Services don't get initialized or configured
        return

    def getReference(self, service):
        return DeviceManagerStub.instance().getService(service._instanceName)

    def _getRequiredExecparams(self, service):
        devmgr_stub = DeviceManagerStub.instance()
        devmgr_ior = orb.object_to_string(devmgr_stub._this())

        return {'DEVICE_MGR_IOR': devmgr_ior,
                'SERVICE_NAME': service._instanceName}

    def _getType(self):
        return 'service'


class ComponentHost(SandboxComponent):
    def __init__(self, *args, **kwargs):
        SandboxComponent.__init__(self, *args, **kwargs)

    def _register(self):
        pass

    def _unregister(self):
        pass

    def executeLinked(self, entryPoint, options, parameters, deps):
        log.debug('Executing shared library %s %s', entryPoint, ' '.join('%s=%s' % (k,v) for k,v in parameters.iteritems()))
        params = [CF.DataType(k, to_any(str(v))) for k, v in parameters.iteritems()]
        self.ref.executeLinked(entryPoint, options, params, deps)


class LocalSandbox(Sandbox):
    def __init__(self, sdrroot=None):
        super(LocalSandbox, self).__init__()
        self.__components = {}
        self.__services = {}
        if not sdrroot:
            sdrroot = os.environ['SDRROOT']
        self._sdrroot = LocalSdrRoot(sdrroot)
        self.__container = None

    def _getComponentHost(self, _debugger=None):
        if self.__container is None:
            self.__container = self._launchComponentHost(_debugger=_debugger)
        return self.__container

    def _launchComponentHost(self, instanceName=None, _debugger=None):
        # Directly create the sandbox object instead of going through launch()
        profile = self._sdrroot.domPath('/mgr/rh/ComponentHost/ComponentHost.spd.xml')
        spd, scd, prf = self._sdrroot.readProfile(profile)
        if instanceName is None:
            instanceName = self._createInstanceName('ComponentHost', 'resource')
        refid = str(uuid4())
        comp = ComponentHost(self, profile, spd, scd, prf, instanceName, refid, None)

        # Likewise, since the specific component type is known, create the
        # launcher directly. The deployment root is overridden to point to the
        # root of the local filesystem; all component paths provided to the
        # component host will be absolute.
        execparams = {'RH::DEPLOYMENT_ROOT':'/'}
        if not isinstance(_debugger, Valgrind):
            _debugger = None
        comp._launcher = LocalComponentLauncher(execparams, {}, True, {}, _debugger, None, None, False)
        comp._kick()
        return comp

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

    def _createLauncher(self, comptype, execparams, initProps, initialize, configProps, debugger,
                        window, timeout, shared, stdout):
        if comptype == 'resource':
            clazz = LocalComponentLauncher
        elif comptype in ('device', 'loadabledevice', 'executabledevice'):
            clazz = LocalDeviceLauncher
        elif comptype == 'service':
            clazz = LocalServiceLauncher 
        else:
            return None
        return clazz(execparams, initProps, initialize, configProps, debugger, window, timeout, shared, stdout)

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

        # Clean up all components
        for name, component in self.__components.items():
            log.debug("Releasing component '%s'", name)
            try:
                component.releaseObject()
            except:
                log.debug("Component '%s' raised an exception while exiting", name)
        self.__components = {}

        # Terminate all services
        for name, service in self.__services.items():
            log.debug("Terminating service '%s'", name)
            try:
                service._terminate()
            except:
                log.debug("Service '%s' raised an exception while terminating", name)
        self.__services = {}

        # Clean up the component host
        if self.__container:
            log.debug('Releasing component host')
            try:
                self.__container.releaseObject()
            except:
                log.debug('Component host raised an exception while terminating')
        self.__container = None

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

    def getType(self):
        return "local"
