#!/usr/bin/env python
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

#
from omniORB import any
from ossie.cf import CF, CF__POA
from ossie.resource import Resource, usesport, providesport, start_component
from ossie.events import GenericEventConsumer
try:
    import CosEventChannelAdmin
    from ossie.cf import StandardEvent
    hasEvents = True
except:
    hasEvents = False


class UsesPort(CF__POA.Port):
    def __init__(self, repid):
        self._connections = {}
        self.__repid = repid

    def connectPort(self, connection, connectionId):
        try:
            port = connection._narrow(self.__repid)
        except:
            port = None
        if port is None:
            # InvalidPort includes a numeric error code, instead of a CF::ErrorNumberType,
            # though it's not particularly useful in this case, anyway.
            raise CF.Port.InvalidPort(0, "connection could not be narrowed to '%s'" % (self.__repid._NP_RepositoryId,))

        port = self._connectPort(port, connectionId)
        if port:
            self._connections[str(connectionId)] = port

    def disconnectPort(self, connectionId):
        try:
            del self._connections[str(connectionId)]
        except KeyError:
            raise CF.Port.InvalidPort(CF.CF_EINVAL, "No such connection '%s'" % (connectionId,))            

    def _connectPort(self, port, connectionId):
        return port


class TestUsesPort(UsesPort):
    def __init__(self):
        UsesPort.__init__(self, CF.FileManager)

    def getListing(self, pattern):
        props = []
        for id in self._connections:
            listing = self._connections[id].list(pattern)
            props.append(CF.DataType(id=id, value=any.to_any(listing)))
        return props


class FileManagerPortTest(CF__POA.Resource, Resource):
    """Simple Python component for basic port testing"""

    toTest = usesport("filemanager_out", "IDL:CF/FileManager:1.0", type_="test")

    def __init__(self, identifier, execparams):
        Resource.__init__(self, identifier, execparams)

    def initialize(self):
        Resource.initialize(self)
        self.toTest = TestUsesPort()

    def runTest(self, testid, properties):
        if testid == 0:
            return self.toTest.getListing('/')
        else:
            raise CF.TestableObject.UnknownTest()
        return []

if __name__ == '__main__':
    start_component(FileManagerPortTest)
