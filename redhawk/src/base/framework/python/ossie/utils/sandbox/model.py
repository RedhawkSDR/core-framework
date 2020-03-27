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

import warnings
import cStringIO, pydoc

from ossie.utils.model import CorbaObject
from ossie.utils.model import PortSupplier, PropertySet, ComponentBase
from ossie.utils.model import Resource, Device, Service

from ossie.utils.sandbox.events import EventChannel

warnings.filterwarnings('once',category=DeprecationWarning)

class SandboxMixin(object):
    def __init__(self, sandbox):
        self._sandbox = sandbox

    def _kick(self):
        self.ref = self._launcher.launch(self)
        self._launcher.setup(self)
        self._register()

    def _terminate(self):
        if self._launcher:
            self._launcher.terminate(self)

    def _register(self):
        raise NotImplemented('_register')

    def _getExecparams(self):
        raise NotImplemented('_getExecparams')


class SandboxResource(ComponentBase, SandboxMixin):
    def __init__(self, sandbox, profile, spd, scd, prf, instanceName, refid, impl):
        ComponentBase.__init__(self, spd, scd, prf, instanceName, refid, impl)
        SandboxMixin.__init__(self, sandbox)
        self._profile = profile
        self._componentName = spd.get_name()

        self.__ports = None
        self._msgSupplierHelper = None
        self._parseComponentXMLFiles()
        self._buildAPI()

    def _getInitializeProperties(self):
        properties = {}
        for prop in self._getPropertySet(kinds=('property',), includeNil=False, commandline=False):
            if prop.defValue is None:
                continue
            properties[prop.id] = prop.defValue
        return properties

    def _getInitialConfigureProperties(self):
        properties = {}
        for prop in self._getPropertySet(kinds=('configure',), modes=('readwrite', 'writeonly'), includeNil=False):
            if prop.defValue is None:
                continue
            properties[prop.id] = prop.defValue
        return properties

    def _getExecparams(self):
        execparams = dict((str(ep.id), ep.defValue) for ep in self._getPropertySet(kinds=('execparam',), includeNil=False))
        for prop in self._getPropertySet(kinds=('property',), includeNil=False, commandline=True):
            execparams[prop.id] = prop.defValue
        return execparams

    def _readProfile(self):
        sdrRoot = self._sandbox.getSdrRoot()
        self._spd, self._scd, self._prf = sdrRoot.readProfile(self._profile)

    def _register(self):
        self._sandbox._registerComponent(self)

    def _unregister(self):
        self._sandbox._unregisterComponent(self)

    @property
    def _ports(self):
        #DEPRECATED: replaced with ports
        warnings.warn("'_ports' is deprecated", DeprecationWarning)
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
        self._sandbox._breakConnections(self)

        # Unregister from the sandbox
        self._unregister()

        try:
            # Call superclass release, which calls the CORBA method.
            super(SandboxResource,self).releaseObject()
        except:
            # Tolerate exceptions (e.g., the object has already been released)
            # and continue on to ensure that the process still gets terminated.
            pass

        # Allow the launcher to peform any follow-up cleanup.
        SandboxMixin._terminate(self)

    def api(self, destfile=None):
        '''
        Inspect interfaces and properties for the component
        '''
        localdef_dest = False
        if destfile == None:
            localdef_dest = True
            destfile = cStringIO.StringIO()

        print >>destfile, "Component [" + str(self._componentName) + "]:\n"
        PortSupplier.api(self, destfile=destfile)
        PropertySet.api(self, destfile=destfile)

        if localdef_dest:
            pydoc.pager(destfile.getvalue())
            destfile.close()

    def sendMessage(self, msg, msgId=None, msgPort=None, restrict=True ):
        """
        send a message out a component's message event port
        
        msg : dictionary of information to send or an any object
        msgId : select a specific message structure property from the component, if None will 
                choose first available message property structure for the component
        msgPort : select a specified message event port to use, if None will try to autoselect
        restrict : if True, will restrict msgId to only those message ids defined by the component
                   if False, will allow for ad-hoc message to be sent
        """
        if self._msgSupplierHelper == None:
            import ossie.utils
            self._msgSupplierHelper = ossie.utils.sb.io_helpers.MsgSupplierHelper(self)
        if self.ref and self.ref._get_started() == True and self._msgSupplierHelper: 
            return self._msgSupplierHelper.sendMessage( msg, msgId, msgPort, restrict )
        return False

class SandboxComponent(SandboxResource, Resource):
    def __init__(self, *args, **kwargs):
        Resource.__init__(self)
        SandboxResource.__init__(self, *args, **kwargs)

    def __repr__(self):
        return "<%s component '%s' at 0x%x>" % (self._sandbox.getType(), self._instanceName, id(self))


class SandboxDevice(SandboxResource, Device):
    def __init__(self, *args, **kwargs):
        Device.__init__(self)
        SandboxResource.__init__(self, *args, **kwargs)

        Device._buildAPI(self)

    def __repr__(self):
        return "<%s device '%s' at 0x%x>" % (self._sandbox.getType(), self._instanceName, id(self))

    def api(self, destfile=None):
        localdef_dest = False
        if destfile == None:
            localdef_dest = True
            destfile = cStringIO.StringIO()

        SandboxResource.api(self, destfile=destfile)
        print >>destfile, '\n'
        Device.api(self, destfile=destfile)

        if localdef_dest:
            pydoc.pager(destfile.getvalue())
            destfile.close()


class SandboxService(Service, SandboxMixin):
    def __init__(self, sandbox, profile, spd, scd, prf, instanceName, refid, impl):
        Service.__init__(self, None, profile, spd, scd, prf, instanceName, refid, impl)
        SandboxMixin.__init__(self, sandbox)

    def _register(self):
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
                if prop.get_commandline() != 'true':
                    continue
            # Only include properties with values
            value = prop.get_value()
            if value is not None:
                execparams[prop.get_id()] = value
        return execparams

    def __repr__(self):
        return "<%s service '%s' at 0x%x>" % (self._instanceName, self._sandbox.getType(), id(self))


class SandboxEventChannel(EventChannel, CorbaObject):
    def __init__(self, name, sandbox):
        EventChannel.__init__(self, name)
        CorbaObject.__init__(self)
        self._sandbox = sandbox
        self._instanceName = name

    def destroy(self):
        # Break any connections involving this event channel.
        self._sandbox._breakConnections(self)
        self._sandbox._removeEventChannel(self._instanceName)
        EventChannel.destroy(self)
