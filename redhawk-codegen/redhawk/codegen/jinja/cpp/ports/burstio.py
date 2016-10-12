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

from redhawk.codegen.lang.idl import IDLInterface
from redhawk.codegen.lang import cpp
from redhawk.codegen.jinja.ports import PortFactory

from generator import CppPortGenerator

class BurstioPortFactory(PortFactory):
    NAMESPACE = 'BURSTIO'

    def match(self, port):
        return IDLInterface(port.repid()).namespace() == self.NAMESPACE

    def generator(self, port):
        return BurstioPortGenerator(port)

class BurstioPortGenerator(CppPortGenerator):
    def header(self):
        return '<burstio/burstio.h>'

    def className(self):
        # The port class is the interface name (first character capitalized),
        # plus the direction, e.g., "BurstByteIn" for "burstByte" provides port
        porttype = self.interface[0].upper() + self.interface[1:]
        if self.direction == 'uses':
            porttype += 'Out'
        else:
            porttype += 'In'
        return 'burstio::' + porttype

    def supportsMultiOut(self):
        return (self.direction == 'uses')

    def _ctorArgs(self, name):
        return (cpp.stringLiteral(name),)
