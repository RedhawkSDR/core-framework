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

from omniORB import CORBA

from ossie import parsers
from ossie.utils.model.connect import ConnectionManager

from base import SdrRoot, Sandbox, SandboxLauncher
from devmgr import DeviceManagerStub
from naming import NamingContextStub
import launcher
from debugger import GDB, PDB, Valgrind
import terminal

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
            return filename
        # Assume the filename points to somewhere in SDRROOT
        return os.path.join(self.__sdrroot, filename)

    def _fileExists(self, filename):
        return os.path.isfile(filename)

    def _readFile(self, filename):
        path = open(self._sdrPath(filename), 'r')
        return path.read()

    def getProfiles(self, objType=None):
        files = []
        searchPath = []
        if objType == "component" or objType is None:
            searchPath.append('dom/components')
        if objType == "device" or objType is None:
            searchPath.append('dev/devices')
        if objType == "service" or objType is None:
            searchPath.append('dev/services')
        if not searchPath:
            raise ValueError, "'%s' is not a valid object Type" % objType
        for path in searchPath:
            path = self._sdrPath(path)
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
    def __init__(self, execparams, initProps, initialize, configProps, debugger, window, timeout):
        self._execparams = execparams
        self._debugger = debugger
        self._window = window
        self._initProps = initProps
        self._initialize = initialize
        self._configProps = configProps
        self._timeout = timeout

    def launch(self, comp):
        # Build up the full set of command line arguments
        execparams = comp._getExecparams()
        execparams.update(self._execparams)
        execparams.update(self._getRequiredExecparams(comp))

        # Set up the debugger if requested
        debugger = self._debugger
        if isinstance(debugger, basestring):
            try:
                if debugger == 'pdb':
                    debugger = PDB()
                elif debugger == 'gdb':
                    debugger = GDB()
                elif debugger == 'valgrind':
                    debugger = Valgrind()
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

        process = launcher.launchSoftpkg(comp._profile, comp._sandbox, comp._spd,
                                         comp._impl, execparams, debugger, window)

        # Set up a callback to notify when the component exits abnormally.
        name = comp._instanceName
        def terminate_callback(pid, status):
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
        comp._process = process

        # Return the now-resolved CORBA reference.
        return self.getReference(comp)

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
        self.__namingContext = NamingContextStub()
        log.trace('Activating virtual NamingContext')
        namingContextId = poa.activate_object(self.__namingContext)
        try:
            return LocalLauncher.launch(self, *args, **kwargs)
        finally:
            log.trace('Deactivating virtual NamingContext')
            poa.deactivate_object(namingContextId)
            del self.__namingContext

    def getReference(self, component):
        return self.__namingContext.getObject(component._instanceName)

    def _getRequiredExecparams(self, component):
        return {'COMPONENT_IDENTIFIER': component._refid,
                'NAMING_CONTEXT_IOR': orb.object_to_string(self.__namingContext._this()),
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


class LocalSandbox(Sandbox):
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

    def _createLauncher(self, comptype, execparams, initProps, initialize, configProps, debugger, window, timeout):
        if comptype == 'resource':
            clazz = LocalComponentLauncher
        elif comptype in ('device', 'loadabledevice', 'executabledevice'):
            clazz = LocalDeviceLauncher
        elif comptype == 'service':
            clazz = LocalServiceLauncher
        else:
            return None
        return clazz(execparams, initProps, initialize, configProps, debugger, window, timeout)

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

    def getType(self):
        return "local"
