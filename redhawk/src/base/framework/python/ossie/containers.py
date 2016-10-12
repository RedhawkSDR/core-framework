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

class Container(object):
    def __init__(self, ref=None):
        self.ref = ref
    def getRef(self):
        return self.ref
        
class ApplicationContainer(Container):
    def __init__(self, app=None):
        super(ApplicationContainer,self).__init__(app)

class DomainManagerContainer(Container):
    def __init__(self, domMgr=None):
        super(DomainManagerContainer,self).__init__(domMgr)

class DeviceManagerContainer(Container):
    def __init__(self, devMgr=None):
        super(DeviceManagerContainer,self).__init__(devMgr)

class NetworkContainer(object):
    def __init__(self, nic=None):
        self._nic = nic
    def getNic(self):
        return self._nic
