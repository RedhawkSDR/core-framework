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

import logging
from omniORB.any import to_any

# Ensure that the entire ExtendedCF module is loaded, otherwise the Sandbox
# interface may be missing. Due to the way omniORB handles dependencies on
# import, if the bulkio interfaces are loaded first, the ExtendedCF module will
# only have the QueryablePort IDL.
from ossie.cf import ExtendedCF
if not hasattr(ExtendedCF, 'Sandbox'):
    import sys
    del sys.modules['ossie.cf'].ExtendedCF
    from ossie.cf import ExtendedCF
from ossie.cf import CF

from ossie.utils.model import Resource, Device, CorbaObject

from base import SdrRoot, Sandbox, SandboxComponent

log = logging.getLogger(__name__)

class IDESdrRoot(SdrRoot):
    def __init__(self, ideRef):
        self.__ide = ideRef
        self.__fileSystem = self.__ide._get_fileManager()

    def _fileExists(self, filename):
        try:
            return self.__fileSystem.exists(filename)
        except CF.InvalidFileName:
            return False

    def _readFile(self, filename):
        fileObj = self.__fileSystem.open(filename, True)
        try:
            return fileObj.read(fileObj.sizeOf())
        finally:
            fileObj.close()

    def _sdrPath(self, filename):
        # Assume the correct path was given by the IDE
        return filename

    def _getSearchPaths(self, objTypes):
        # Search everything
        return ['']

    def _getAvailableProfiles(self, searchPath):
        profiles = self.__ide._get_availableProfiles()
        if searchPath:
            # Treat the search path as a prefix, and filter out any profiles
            # that are not in a subdirectory thereof
            if not searchPath.endswith('/'):
                # Avoid partial matches (e.g., '/dev' matching '/devices/')
                searchPath += '/'
            return [p for p in profiles if p.startswith(searchPath)]
        else:
            return profiles

    def getLocation(self):
        return 'REDHAWK IDE virtual SDR'


class IDEMixin(object):
    def __init__(self, execparams, initProps, configProps):
        self._execparams = execparams
        self._initProps = initProps
        self._configProps = configProps

    def _launch(self):
        # Pack the execparams into an array of string-valued properties
        properties = [CF.DataType(k, to_any(str(v))) for k, v in self._execparams.iteritems()]
        # Pack the remaining props by having the component do the conversion
        properties.extend(self._itemToDataType(k,v) for k,v in self._initProps.iteritems())
        properties.extend(self._itemToDataType(k,v) for k,v in self._configProps.iteritems())

        # Tell the IDE to launch a specific implementation, if given
        if self._impl is not None:
            properties.append(CF.DataType('__implementationID', to_any(self._impl)))

        ref = self._sandbox._createResource(self._profile, self._instanceName, properties)

        # The IDE sandbox API only allows us to specify the instance name, not
        # the identifier, so update by querying the component itself
        self._refid = ref._get_identifier()

        return ref

    def _getExecparams(self):
        return {}

    def _terminate(self):
        pass


class IDESandboxComponent(SandboxComponent, IDEMixin):
    def __init__(self, sandbox, profile, spd, scd, prf, instanceName, refid, impl, execparams,
                 initProps, configProps):
        SandboxComponent.__init__(self, sandbox, profile, spd, scd, prf, instanceName, refid, impl)
        IDEMixin.__init__(self, execparams, initProps, configProps)
        self._parseComponentXMLFiles()
        self._buildAPI()

    def _getExecparams(self):
        return dict((str(ep.id), ep.defValue) for ep in self._getPropertySet(kinds=('execparam',), includeNil=False))


class IDEComponent(IDESandboxComponent, Resource):
    def __init__(self, *args, **kwargs):
        Resource.__init__(self)
        IDESandboxComponent.__init__(self, *args, **kwargs)

    def __repr__(self):
        return "<IDE component '%s' at 0x%x>" % (self._instanceName, id(self))
    

class IDEDevice(IDESandboxComponent, Device):
    def __init__(self, *args, **kwargs):
        Device.__init__(self)
        IDESandboxComponent.__init__(self, *args, **kwargs)
        Device._buildAPI(self)

    def __repr__(self):
        return "<IDE device '%s' at 0x%x>" % (self._instanceName, id(self))

    def api(self):
        IDESandboxComponent.api(self)
        print
        Device.api(self)


class IDEService(CorbaObject):
    def __init__(self, ref, instanceName):
        CorbaObject.__init__(self, ref)
        self._instanceName = instanceName

    def __repr__(self):
        return "<IDE service '%s' at 0x%x>" % (self._instanceName, id(self))


class IDESandbox(Sandbox):
    __comptypes__ = {
        'resource': IDEComponent,
        'device':   IDEDevice,
        }

    def __init__(self, ideRef):
        super(IDESandbox, self).__init__()
        self.__ide = ideRef
        self.__components = {}
        self.__services = {}

    def getSdrRoot(self):
        return IDESdrRoot(self.__ide)

    def _createInstanceName(self, softpkgName, componentType='resource'):
        # Sync with IDE
        self._scanChalkboard()

        # Use one-up counter to make component instance name unique.
        counter = len(self.__components) + 1
        while True:
            name = '%s_%d' % (softpkgName.replace('.','_'), counter)
            if name not in self.__components:
                return name
            counter += 1

    def _checkInstanceId(self, refid, componentType='resource'):
        # The identifier will get overwritten in launch, so the refid is always
        # "valid"
        return True

    def _launch(self, profile, spd, scd, prf, instanceName, refid, impl, execparams,
                initProps, initialize, configProps, debugger, window, timeout):
        # Determine the class for the component type and create a new instance.
        clazz = self.__comptypes__[scd.get_componenttype()]
        comp = clazz(self, profile, spd, scd, prf, instanceName, refid, impl, execparams, initProps, configProps)
        comp._kick()
        return comp

    def _createResource(self, profile, name, qualifiers=[]):
        log.debug("Creating resource '%s' with profile '%s'", name, profile)

        rescFactory = self.__ide.getResourceFactoryByProfile(profile)
        return rescFactory.createResource(name, qualifiers)

    def _registerComponent(self, component):
        self.__components[component._instanceName] = component

    def _unregisterComponent(self, component):
        name = component._instanceName
        if name in self.__components:
            del self.__components[name]

    def _isRegistered(self, name):
        componentAlive = False
        if name in self.__components:
            componentAlive = True
            try:
                self.__component[name]._get_identifier()
            except:
                componentAlive = False
        return componentAlive

    def _scanChalkboard(self):
        # Remember the names of known components, so that any that are no longer
        # registered can be removed
        stale = set(self.__components.keys())
        sdrroot = self.getSdrRoot()
        for desc in self.__ide._get_registeredResources():
            try:
                resource = desc.resource
                if resource._is_a('IDL:CF/Device:1.0'):
                    clazz = IDEDevice
                    refid = resource._get_identifier()
                    instanceName = resource._get_label()
                    impl = self.__ide._get_deviceManager().getComponentImplementationId(refid)
                else:
                    clazz = IDEComponent
                    refid = resource._get_identifier()
                    if ':DCE:' in refid:
                        # Components launched from the Python console have the
                        # name embedded in the identifier
                        instanceName = refid.split(':')[0]
                    else:
                        # Components launched directly from the IDE do not have
                        # distinct names and identifiers
                        instanceName = refid
                    impl = None

                # Mark the component name as current
                stale.discard(instanceName)

                if self._isRegistered(instanceName):
                    continue

                # The profile returned in the descriptor is often incorrect;
                # ask the resource itself.
                profile = resource._get_softwareProfile()

                # Create the component/device sandbox wrapper, disabling the
                # automatic launch since it is already running
                spd, scd, prf = self.getSdrRoot().readProfile(profile)
                comp = clazz(self, profile, spd, scd, prf, instanceName, refid, impl, {}, {}, {})
                comp.ref = resource
                self.__components[instanceName] = comp
            except Exception, e:
                log.error("Could not wrap resource with profile %s': %s", desc.profile, e)
        
        # Clean up stale components
        for name in stale:
            del self.__components[name]

    def _scanServices(self):
        # Remember the names of known services, so that any that are no longer
        # registered can be removed
        stale = set(self.__services.keys())

        devmgr = self.__ide._get_deviceManager()
        for desc in devmgr._get_registeredServices():
            ref = desc.serviceObject
            instanceName = desc.serviceName

            # Mark as seen
            stale.discard(instanceName)

            # Check if it already exists
            if instanceName in self.__services:
                continue

            # Create a new IDEService proxy for the service
            service = IDEService(ref, instanceName)
            self.__services[instanceName] = service

        # Clean up stale services
        for name in stale:
            del self.__services[name]

    def catalog(self, searchPath=None, objType=None):
        if searchPath:
            log.warn("IDE sandbox does not support alternate paths")

        return super(IDESandbox,self).catalog(searchPath, objType)

    def browse(self, searchPath=None, objType=None, withDescription=False):
        if searchPath:
            log.warn("IDE sandbox does not support alternate paths")

        sdrroot = self.getSdrRoot()

        output_text = ""
        rsrcDict = {}
        for profile in sdrroot.getProfiles():
            rsrcType = "components"
            if profile.find("/components") != -1:
                rsrcType = "components"
            elif profile.find("/devices") != -1:
                rsrcType = "devices"
            elif profile.find("/services") != -1:
                rsrcType = "services"
            spd, scd, prf = sdrroot.readProfile(profile)
            if rsrcDict.has_key(rsrcType) == False:
                rsrcDict[rsrcType] = []
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
                rsrcDict[rsrcType].append(new_item)
            else:
                rsrcDict[rsrcType].append(spd.get_name())

        for key in sorted(rsrcDict.iterkeys()):
            output_text += "************************ " + str(key) + " ***************************\n"

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
        print output_text

    def getComponent(self, name):
        self._scanChalkboard()
        return self.__components.get(name, None)

    def retrieve(self, name):
        self._scanChalkboard()
        return self.__components.get(name, None)

    def getComponents(self):
        self._scanChalkboard()
        return self.__components.values()

    def getService(self, name):
        self._scanServices()
        return self.__services.get(name, None)

    def getServices(self):
        self._scanServices()
        return self.__services.values()
