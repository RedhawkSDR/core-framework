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

from redhawk.codegen.lang import python
from redhawk.codegen.lang.idl import IDLInterface
from redhawk.codegen.jinja.ports import PortGenerator

class PythonPortGenerator(PortGenerator):
    def __eq__(self, other):
        return self.className() == other.className()

    def className(self):
        return self.templateClass() + '_i'

    def templateClass(self):
        interface = self.interface[0].upper() + self.interface[1:]
        porttype = 'Port' + self.pyNamespace() + interface
        if self.direction == 'uses':
            porttype += 'Out'
        else:
            porttype += 'In'
        return porttype

    def attributeClass(self):
        return self.direction+'port'

    def imports(self):
        return []

    def pyNamespace(self):
        return self.namespace.replace('omg.org/', '')

    def corbaClass(self):
        return self.pyNamespace() + '.' + self.interface

    def poaClass(self):
        if self.direction == 'uses':
            return 'CF__POA.Port'
        else:
            return self.pyNamespace() + '__POA.' + self.interface

    def _ctorArgs(self, port):
        return []

    def constructor(self, port):
        return '%s(%s)' % (self.className(), ', '.join(self._ctorArgs(port)))

    def supportsMultiOut(self):
        return False

class BuiltinPythonPort(PythonPortGenerator):
    def __init__(self, pyclass, port):
        PythonPortGenerator.__init__(self, port)
        self.__imports = (python.importModule(pyclass),)
        self.__name = pyclass.split('.')[-1]

    def imports(self):
        return self.__imports

    def className(self):
        return self.__name
