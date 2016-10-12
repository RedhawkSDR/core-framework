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

class BulkioPortFactory(PortFactory):
    NAMESPACE = 'BULKIO'

    def match(self, port):
        interface = IDLInterface(port.repid())
        if interface.namespace() != self.NAMESPACE:
            return False
        return interface.interface().startswith('data')

    def generator(self, port):
        return BulkioPortGenerator(port)

class BulkioPortGenerator(CppPortGenerator):
    def header(self):
        return '<bulkio/bulkio.h>'

    def className(self):
        # Trim 'data' from front of interface to get data type
        datatype = self.interface.lstrip('data')
        # If interface is unsigned need to make sure next character is upper
        # case to conform with bulkio base classes
        if datatype.startswith('U'):
            datatype = datatype[0] + datatype[1].upper() + datatype[2:]
        if self.direction == 'provides':
            direction = 'In'
        else:
            direction = 'Out'
        return 'bulkio::' + direction + datatype + 'Port'

    def supportsMultiOut(self):
        return (self.direction == 'uses')

    def _ctorArgs(self, name):
        return (cpp.stringLiteral(name),)
