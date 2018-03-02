#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

from ossie.utils.sb.io_helpers import helperBase
from ossie.utils.model import PortSupplier

class SandboxHelper(helperBase):
    def __init__(self):
        helperBase.__init__(self)

    def start(self):
        pass

    def stop(self):
        pass

class SandboxPortHelper(SandboxHelper, PortSupplier):
    def __init__(self):
        SandboxHelper.__init__(self)
        PortSupplier.__init__(self)
        self._port = None

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
            self._port = port_dict['Port Class'](portName)

        return self._port._this()

    def start(self):
        if self._port and hasattr(self._port, 'startPort'):
            self._port.startPort()

    def stop(self):
        if self._port and hasattr(self._port, 'stopPort'):
            self._port.stopPort()
