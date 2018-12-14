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

from ossie import parsers
from ossie.utils.log4py import logging
from ossie.utils import weakobj
from ossie.utils.model.connect import ConnectionManager
from ossie.utils.uuid import uuid4

from model import SandboxComponent, SandboxDevice, SandboxService, SandboxEventChannel

log = logging.getLogger(__name__)

class SdrRoot(object):
    def readProfile(self, profile):
        xmlPath = os.path.dirname(profile)
        spd = parsers.spd.parseString(self._readFile(profile))

        # Parse the softpkg SCD, if available.
        if spd.get_descriptor():
            scdFile = os.path.join(xmlPath, str(spd.get_descriptor().get_localfile().get_name()))
            log.trace("Softpkg '%s' has SCD '%s'", spd.get_name(), scdFile)
            scd = parsers.scd.parseString(self._readFile(scdFile))
        else:
            log.trace("Softpkg '%s' has no SCD", spd.get_name())
            scd = None

        # If the softpkg has a PRF, parse that as well.
        if spd.get_propertyfile():
            prfFile = os.path.join(xmlPath, str(spd.get_propertyfile().get_localfile().get_name()))
            log.trace("Softpkg '%s' has PRF '%s'", spd.get_name(), prfFile)
            prf = parsers.prf.parseString(self._readFile(prfFile))
        else:
            log.trace("Softpkg '%s' has no PRF", spd.get_name())
            prf = None

        return spd, scd, prf

    def _getObjectType(self, scd):
        if scd is not None:
            componentType = scd.get_componenttype()
            if componentType in ('device', 'loadabledevice', 'executabledevice'):
                return 'devices'
            elif componentType == 'resource':
                return 'components'
            elif componentType == 'service':
                return 'services'

        return None

    def _getSearchPaths(self, objTypes):
        raise NotImplementedError('Sandbox._getSearchPaths')

    def _getAvailableProfiles(self, searchPath):
        raise NotImplementedError('Sandbox._getAvailableProfiles')

    def _getObjectTypes(self, objType):
        ALL_TYPES = ('components', 'devices', 'services')
        if objType in ALL_TYPES:
            return set((objType,))
        elif objType in (None, 'all'):
            return set(ALL_TYPES)
        else:
            raise ValueError, "'%s' is not a valid object type" % objType

    def findProfile(self, descriptor, objType=None):
        # Try the descriptor as a path to an SPD first
        sdrPath = self._sdrPath(descriptor)
        if self._fileExists(sdrPath):
            try:
                spd = parsers.spd.parseString(self._readFile(sdrPath))
                log.trace("Descriptor '%s' is file name", descriptor)
                return sdrPath
            except:
                pass

        objMatches = []
        for profile in self.readProfiles(objType):
            if descriptor == profile['name']:
                objMatches.append(profile['profile'])
        if len(objMatches) == 1:
            return objMatches[0]
        elif len(objMatches) > 1:
            print "There are multiple object types with the name '%s'" % descriptor
            for type in objMatches:
                print " ", type
            print 'Filter the object type as a "component", "device", or "service".'
            print 'Try sb.launch("<descriptor>", objType="<objectType>")'
            return None
        raise ValueError, "'%s' is not a valid softpkg name or SPD file" % descriptor

    def readProfiles(self, objType=None, searchPath=None):
        # Remap the object type string to a set of object type names
        objTypes = self._getObjectTypes(objType)

        if searchPath is None:
            paths = self._getSearchPaths(objTypes)
        else:
            paths = [searchPath]

        profiles = []
        for path in paths:
            for filename in self._getAvailableProfiles(path):
                # Read and parse the complete profile
                try:
                    spd, scd, prf = self.readProfile(filename)
                except:
                    # Invalid profile, ignore it
                    continue

                # Filter based on the object type in the SCD
                profileType = self._getObjectType(scd)
                if profileType not in objTypes:
                    continue

                profile = { 'name': str(spd.get_name()), 'profile': filename }
                profiles.append(profile)
                    
        return profiles

    def getProfiles(self, objType=None, searchPath=None):
        # Use readProfiles() to handle all of the search and parsing
        return [p['profile'] for p in self.readProfiles(objType, searchPath)]


class Sandbox(object):
    def __init__(self):
        self._eventChannels = {}
        self._started = False

    def createEventChannel(self, name, exclusive=False):
        """
        Create a sandbox event channel 'name'. If the channel already exists,
        return a handle to the existing channel when 'exclusive' is False, or 
        raise a NameError otherwise.
        """
        if name not in self._eventChannels:
            # Channel does not exist and can be safely created
            channel = SandboxEventChannel(name, self)
            # NB: Temporary hack to ensure ref is set
            channel.ref = channel._this()
            self._eventChannels[name] = channel
        elif exclusive:
            raise NameError("Event channel '%s' already exists" % name)

        # Pass on to getEventChannel to ensure that the same type of reference
        # (i.e., weak proxy) is returned.
        return self.getEventChannel(name)

    def getEventChannel(self, name):
        """
        Get the sandbox event channel 'name'. If it does not exist,
        throw a NameError.
        """
        if name not in self._eventChannels:
            raise NameError("No event channel '%s'" % name)
        else:
            # Return a weak proxy so that destroy() can fully delete the
            # object in normal usage and ensure that it stays destroyed.
            return weakobj.objectref(self._eventChannels[name])

    def getEventChannels(self):
        return [weakobj.objectref(c) for c in self._eventChannels.itervalues()]
    
    def _removeEventChannel(self, name):
        del self._eventChannels[name]

    @property
    def started(self):
        return self._started

    def start(self):
        log.debug('start()')
        self._started = True
        for component in self.getComponents():
            if not component:
                continue
            log.debug("Starting component '%s'", component._instanceName)
            component.start()

    def stop(self):
        log.debug('stop()')
        self._started = False
        for component in self.getComponents():
            if not component:
                continue
            log.debug("Stopping component '%s'", component._instanceName)
            try:
                component.stop()
            except Exception, e:
                pass

    def reset(self):
        for component in self.getComponents():
            # Bring down current component process and re-launch it.
            component.reset()

    def launch(self, descriptor, instanceName=None, refid=None, impl=None,
               debugger=None, window=None, properties={}, configure=True,
               initialize=True, timeout=None, objType=None, shared=True, stdout=None):
        sdrRoot = self.getSdrRoot()

        # Parse the component XML profile.
        profile = sdrRoot.findProfile(descriptor, objType)
        if not profile:
            return None
        spd, scd, prf = sdrRoot.readProfile(profile)
        name = spd.get_name()
        if not scd:
            raise RuntimeError, 'Cannot launch softpkg with no SCD'

        # Check that we can launch the component.
        comptype = scd.get_componenttype()
        if comptype == 'resource':
            clazz = SandboxComponent
        elif comptype in ('device', 'loadabledevice', 'executabledevice'):
            clazz = SandboxDevice
        elif comptype == 'service':
            clazz = SandboxService
        else:
            raise NotImplementedError("No support for component type '%s'" % comptype)

        # Generate/check instance name.
        if not instanceName:
            instanceName = self._createInstanceName(name, comptype)
        elif not self._checkInstanceName(instanceName, comptype):
            raise ValueError, "User-specified instance name '%s' already in use" % (instanceName,)

        # Generate/check identifier.
        if not refid:
            refid = 'DCE:'+str(uuid4())
        elif not self._checkInstanceId(refid, comptype):
            raise ValueError, "User-specified identifier '%s' already in use" % (refid,)

        # If possible, determine the correct placement of properties
        execparams, initProps, configProps = self._sortOverrides(prf, properties)
        if not configure:
            configProps = None

        # Determine the class for the component type and create a new instance.
        comp = clazz(self, profile, spd, scd, prf, instanceName, refid, impl)
        launcher = self._createLauncher(comptype, execparams, initProps, initialize, configProps, debugger, window, timeout, shared, stdout)
        if not launcher:
            raise NotImplementedError("No support for component type '%s'" % comptype)
        comp._launcher = launcher

        # Launch the component
        comp._kick()

        return comp

    def shutdown(self):
        # Clean up any event channels created by this sandbox instance.
        for channel in self._eventChannels.values():
            channel.destroy()
        self._eventChannels = {}

    def catalog(self, searchPath=None, objType="components"):
        files = {}
        for profile in self.getSdrRoot().readProfiles(objType, searchPath):
            files[profile['name']] = profile['profile']
        return files

    def _sortOverrides(self, prf, properties):
        if not prf:
            # No PRF file, assume all properties are execparams.
            return properties, {}, {}

        # Classify the PRF properties by which stage of initialization they get
        # set: 'commandline', 'initialize', 'configure' or None (not settable).
        stages = {}
        for prop, stage in self._getInitializationStages(prf):
            stages[str(prop.get_id())] = stage
            if prop.get_name():
                name = str(prop.get_name())
                # Only add name if there isn't already something by the same key
                if not name in stages:
                    stages[name] = stage

        # Add in system-defined execparams that users are allowed to override
        stages['DEBUG_LEVEL'] = 'commandline'
        stages['LOGGING_CONFIG_URI'] = 'commandline'

        # Sort properties into the appropriate stage of initialization
        execparams = {}
        initProps = {}
        configProps = {}
        for key, value in properties.iteritems():
            if not key in stages:
                log.warning("Unknown property '%s'" , key)
                continue
            stage = stages[key]
            if stage == 'commandline':
                execparams[key] = value
            elif stage == 'initialize':
                initProps[key] = value
            elif stage == 'configure':
                configProps[key] = value
            else:
                log.warning("Property '%s' cannot be set at launch", key)

        return execparams, initProps, configProps

    def _getInitializationStage(self, prop, kinds, commandline=False):
        # Helper method to classify the initialization stage for a particular
        # property. The caller provides the type-specific information (kinds,
        # commandline).
        if kinds:
            kinds = set(k.get_kindtype() for k in kinds)
        else:
            kinds = set(('configure',))
        if 'execparam' in kinds:
            return 'commandline'
        elif 'property' in kinds:
            if commandline:
                return 'commandline'
            else:
                return 'initialize'
        elif 'configure' in kinds and prop.get_mode() != 'readonly':
            return 'configure'
        else:
            return None

    def _getInitializationStages(self, prf):
        # Helper method to turn all of the PRF properties into an iterable sequence
        # of (property, stage) pairs.
        for prop in prf.get_simple():
            isCommandline = prop.get_commandline() == 'true'
            yield prop, self._getInitializationStage(prop, prop.get_kind(), isCommandline)
        for prop in prf.get_simplesequence():
            yield prop, self._getInitializationStage(prop, prop.get_kind())
        for prop in prf.get_struct():
            yield prop, self._getInitializationStage(prop, prop.get_configurationkind())
        for prop in prf.get_structsequence():
            yield prop, self._getInitializationStage(prop, prop.get_configurationkind())

    def _breakConnections(self, target):
        # Break any connections involving this object.
        manager = ConnectionManager.instance()
        for _identifier, (identifier, uses, provides) in manager.getConnections().items():
            if uses.hasComponent(target) or provides.hasComponent(target):
                manager.breakConnection(identifier, uses)
                manager.unregisterConnection(identifier, uses)


class SandboxLauncher(object):
    def launch(self, comp):
        raise NotImplementedError('launch')

    def setup(self, comp):
        raise NotImplementedError('setup')

    def terminate(self, comp):
        raise NotImplementedError('terminate')
