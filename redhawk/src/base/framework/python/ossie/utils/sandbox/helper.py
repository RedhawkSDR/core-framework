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

from ossie.utils.log4py import logging
from ossie.utils.model import PortSupplier
from ossie.utils.model.connect import ConnectionManager
from ossie.utils.uuid import uuid4

from ossie.threadedcomponent import *

def default_sandbox():
    # Use a deferred import to avoid a circular dependency with the 'sb' module
    from ossie.utils.sb import domainless
    return domainless._getSandbox()

class SandboxMeta(type):
    def __call__(self, *args, **kwargs):
        # Pick off sandbox-specific arguments, which are not given to the
        # helper class __init__ method
        sandbox = kwargs.pop('sandbox', None)
        if sandbox is None:
            sandbox = default_sandbox()
        auto_start = kwargs.pop('autoStart', True)

        # Create/initialize the helper
        obj = super(SandboxMeta, self).__call__(*args, **kwargs)

        # Create a unique instance name, and register with the sandbox
        name = sandbox._createInstanceName(obj.__class__.__name__)
        obj._registerWithSandbox(sandbox, name)

        # Set a sensible default logger based on the module and instance name
        obj.log = logging.getLogger(obj.__module__).getChild(name)

        # Perform any post-registration initialization
        obj._initializeHelper()

        # Auto-start helpers
        if auto_start and sandbox.started:
            obj.start()

        return obj

class SandboxHelper(PortSupplier):
    __metaclass__ = SandboxMeta

    def __init__(self):
        PortSupplier.__init__(self)

        self._refid = str(uuid4())
        self._port = None
        self._started = False

    def _registerWithSandbox(self, sandbox, instanceName):
        self._sandbox = sandbox
        self._instanceName = instanceName
        self._sandbox._registerComponent(self)

    def _addUsesPort(self, name, repoID, portClass, custom={}):
        port_dict = {
            'Port Name': name,
            'Port Interface': repoID,
            'Port Class': portClass
        }
        port_dict.update(custom)
        self._usesPortDict[name] = port_dict

    def _addProvidesPort(self, name, repoID, portClass, custom={}):
        port_dict = {
            'Port Name': name,
            'Port Interface': repoID,
            'Port Class': portClass
        }
        port_dict.update(custom)
        self._providesPortDict[name] = port_dict

    def _createPort(self, portDict, name=None):
        clazz = portDict['Port Class']
        if not name:
            name = portDict['Port Name']
        port = clazz(name)

        # Automatically start the port if the helper has already been started
        if self.started:
            self._startPort(port)

        # Allow subclasses to perform additional post-creation logic
        self._portCreated(port, portDict)

        return port

    @property
    def started(self):
        return self._started

    def start(self):
        if self._started:
            return
        self._startHelper()
        self._started = True

    def stop(self):
        if not self._started:
            return
        self._stopHelper()
        self._started = False

    def _startPort(self, port):
        if hasattr(port, 'startPort'):
            port.startPort()

    def _stopPort(self, port):
        if hasattr(port, 'stopPort'):
            port.stopPort()

    def _startPorts(self):
        if self._port:
            self._startPort(self._port)

    def _stopPorts(self):
        if self._port:
            self._stopPort(self._port)

    def releaseObject(self):
        # Break any connections involving this helper
        manager = ConnectionManager.instance()
        for identifier, uses, provides in manager.getConnections().itervalues():
            if uses.hasComponent(self) or provides.hasComponent(self):
                usesRef = uses.getReference()
                usesRef.disconnectPort(identifier)
                manager.unregisterConnection(identifier, uses)
        self._sandbox._unregisterComponent(self)

    def reset(self):
        pass

    def getPort(self, portName):
        if self._port:
            if portName != self._port.name:
                raise RuntimeError(self.__class__.__name__ + ' only supports 1 port type at a time')
        else:
            port_dict = self._usesPortDict.get(portName, None)
            if port_dict is None:
                port_dict = self._providesPortDict.get(portName, None)
            if port_dict is None:
                raise RuntimeError("Unknown port '%s'" % portName)
            self._port = self._createPort(port_dict)

        return self._port._this()

    # Extension points for subclasses
    def _initializeHelper(self):
        # Equivalent to component constructor() method; override to perform any
        # initialization that requires that the helper has registered with the
        # sandbox (e.g., any setup that requires knowing the instance name)
        pass

    def _portCreated(self, port, portDict):
        # Extension point for subclasses to perform any post port-creation
        # tasks (e.g., setting a mode based on the port type)
        pass

    def _startHelper(self):
        # Subclasses should override this method rather than start() to provide
        # additional start behavior
        self._startPorts()

    def _stopHelper(self):
        # Subclasses should override this method rather than stop() to provide
        # additional stop behavior
        self._stopPorts()


class ThreadedSandboxHelper(SandboxHelper, ThreadedComponent):
    def __init__(self):
        SandboxHelper.__init__(self)
        ThreadedComponent.__init__(self)
    
    def _startHelper(self):
        super(ThreadedSandboxHelper,self)._startHelper()
        self.startThread()

    def _stopHelper(self):
        super(ThreadedSandboxHelper,self)._stopHelper()
        self.stopThread()

    def process(self):
        try:
            return self._threadFunc()
        except Exception:
            import traceback
            traceback.print_exc()
            return FINISH

    def _threadFunc(self):
        return FINISH

class ThreadStatus(object):
    NOOP = NOOP
    NORMAL = NORMAL
    FINISH = FINISH
