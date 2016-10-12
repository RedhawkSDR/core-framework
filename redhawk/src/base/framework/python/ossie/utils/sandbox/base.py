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
import copy
import warnings as _warnings

from ossie import parsers
from ossie.cf import CF
from ossie.utils import log4py
from ossie.utils import weakobj
from ossie.utils.model import PortSupplier, PropertySet, ComponentBase, CorbaObject
from ossie.utils.model.connect import ConnectionManager
from ossie.utils.uuid import uuid4

from ossie.utils.sandbox.events import EventChannel

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

    def findProfile(self, descriptor, objType=None):
        objMatches = []
        if self._fileExists(descriptor):
            try:
                spd = parsers.spd.parseString(self._readFile(descriptor))
                log.trace("Descriptor '%s' is file name", descriptor)
                return self._sdrPath(descriptor)
            except:
                pass
        for profile in self.getProfiles(objType):
            try:
                spd = parsers.spd.parseString(self._readFile(profile))
                if spd.get_name() == descriptor:
                    log.trace("Softpkg name '%s' found in '%s'", descriptor, profile)
                    objMatches.append(profile)
            except:
                log.warning('Could not parse %s', profile)
                continue
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


class Sandbox(object):
    def __init__(self, autoInit=True):
        self._autoInit = autoInit
        self._eventChannels = {}

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

    def start(self):
        log.debug('start()')
        for component in self.getComponents():
            if not component:
                continue
            log.debug("Starting component '%s'", component._instanceName)
            component.start()

    def stop(self):
        log.debug('stop()')
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
               debugger=None, window=None, execparams={}, configure={},
               initialize=True, timeout=None, objType=None):
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
        if comptype not in self.__comptypes__:
            raise NotImplementedError, "No support for component type '%s'" % comptype

        if not instanceName:
            instanceName = self._createInstanceName(name, comptype)
        elif not self._checkInstanceName(instanceName, comptype):
            raise ValueError, "User-specified instance name '%s' already in use" % (instanceName,)

        if not refid:
            refid = str(uuid4())
        elif not self._checkInstanceId(refid, comptype):
            raise ValueError, "User-specified identifier '%s' already in use" % (refid,)

        # Determine the class for the component type and create a new instance.
        clazz = self.__comptypes__[comptype]
        comp = clazz(self, profile, spd, scd, prf, instanceName, refid, impl, execparams, debugger, window, timeout)
        # Services don't get initialized or configured
        if comptype == 'service':
            return comp

        # Initialize the component unless asked not to (if the subclass has not
        # disabled automatic initialization).
        if initialize and self._autoInit:
            comp.initialize()
        
        # Configure component with default values unless requested not to (e.g.,
        # when launched from a SAD file).
        if configure is not None:
            # Make a copy of the default properties, and update with the passed-in
            # properties
            initvals = copy.deepcopy(comp._configRef)
            initvals.update(configure)
            try:
                comp.configure(initvals)
            except:
                log.exception('Failure in component configuration')
        return comp

    def shutdown(self):
        # Clean up any event channels created by this sandbox instance.
        for channel in self._eventChannels.values():
            channel.destroy()
        self._eventChannels = {}


class SandboxComponent(ComponentBase):
    def __init__(self, sandbox, profile, spd, scd, prf, instanceName, refid, impl):
        super(SandboxComponent,self).__init__(spd, scd, prf, instanceName, refid, impl)
        self._sandbox = sandbox
        self._profile = profile
        self._componentName = spd.get_name()
        self._configRef = {}
        for prop in self._getPropertySet(kinds=('configure',), modes=('readwrite', 'writeonly'), includeNil=False):
            if prop.defValue is None:
                continue
            self._configRef[str(prop.id)] = prop.defValue

        self.__ports = None
        
    def _readProfile(self):
        sdrRoot = self._sandbox.getSdrRoot()
        self._spd, self._scd, self._prf = sdrRoot.readProfile(self._profile)

    def _kick(self):
        self.ref = self._launch()
        self._sandbox._registerComponent(self)

    @property
    def _ports(self):
        #DEPRECATED: replaced with ports
        _warnings.warn("'_ports' is deprecated", DeprecationWarning)
        return self.ports
 
    @property
    def ports(self):
        if self.__ports == None:
            self.__ports = self._populatePorts()
        return self.__ports
        
    def reset(self):
        self.releaseObject()
        self._readProfile()
        self._kick()
        self.initialize()
        self._parseComponentXMLFiles()
        self._buildAPI()
        # Clear cached ports list
        self.__ports = None

    def releaseObject(self):
        # Break any connections involving this component.
        manager = ConnectionManager.instance()
        for identifier, (uses, provides) in manager.getConnections().items():
            if uses.hasComponent(self) or provides.hasComponent(self):
                manager.breakConnection(identifier)
                manager.unregisterConnection(identifier)
        self._sandbox._unregisterComponent(self)
        super(SandboxComponent,self).releaseObject()

    def api(self):
        '''
        Inspect interfaces and properties for the component
        '''
        print "Component [" + str(self._componentName) + "]:"
        PortSupplier.api(self)
        PropertySet.api(self)


class SandboxEventChannel(EventChannel, CorbaObject):
    def __init__(self, name, sandbox):
        EventChannel.__init__(self, name)
        CorbaObject.__init__(self)
        self._sandbox = sandbox
        self._instanceName = name

    def destroy(self):
        # Break any connections involving this event channel.
        manager = ConnectionManager.instance()
        for identifier, (uses, provides) in manager.getConnections().items():
            if provides.hasComponent(self):
                manager.breakConnection(identifier)
                manager.unregisterConnection(identifier)
        self._sandbox._removeEventChannel(self._instanceName)
        EventChannel.destroy(self)
