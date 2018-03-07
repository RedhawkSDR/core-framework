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

from ossie.utils.model import PortSupplier
from ossie.utils.model.connect import ConnectionManager
from ossie.utils.uuid import uuid4

from ossie.threadedcomponent import *

def default_sandbox():
    from ossie.utils.sb import domainless
    return domainless._getSandbox()

class SandboxMeta(type):
    def __call__(self, *args, **kwargs):
        sandbox = kwargs.pop('sandbox', None)
        if sandbox is None:
            sandbox = default_sandbox()
        auto_start = kwargs.pop('autoStart', True)

        obj = super(SandboxMeta, self).__call__(*args, **kwargs)
        obj._registerWithSandbox(sandbox)

        # Auto-start helpers
        if auto_start and sandbox._get_started():
            obj.start()

        return obj

class SandboxHelper(PortSupplier):
    __metaclass__ = SandboxMeta

    def __init__(self):
        PortSupplier.__init__(self)

        self._refid = str(uuid4())
        self._port = None
        self._started = False

    def _registerWithSandbox(self, sandbox):
        self._sandbox = sandbox

        # Create a unique instance name and register with the sandbox.
        self._instanceName = self._sandbox._createInstanceName(self.__class__.__name__)
        self._sandbox._registerComponent(self)

    def _addUsesPort(self, name, repoID, portClass):
        self._usesPortDict[name] = {
            'Port Name': name,
            'Port Interface': repoID,
            'Port Class': portClass
        }

    def _addProvidesPort(self, name, repoID, portClass):
        self._providesPortDict[name] = {
            'Port Name': name,
            'Port Interface': repoID,
            'Port Class': portClass
        }

    def _createPort(self, cls, name):
        return cls(name)

    @property
    def started(self):
        return self._started

    def start(self):
        if self._started:
            return
        self._startHelper()
        self._started = True

    def _startHelper(self):
        if self._port and hasattr(self._port, 'startPort'):
            self._port.startPort()

    def stop(self):
        if not self._started:
            return
        self._stopHelper()
        self._started = False

    def _stopHelper(self):
        if self._port and hasattr(self._port, 'stopPort'):
            self._port.stopPort()

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
            cls = port_dict['Port Class']
            self._port = self._createPort(cls, portName)

        return self._port._this()

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
