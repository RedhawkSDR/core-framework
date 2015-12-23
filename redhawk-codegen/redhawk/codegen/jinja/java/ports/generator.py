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
from redhawk.codegen.jinja.ports import PortGenerator

class JavaPortGenerator(PortGenerator):
    def __eq__(self, other):
        return self.className() == other.className()

    def className(self):
        porttype = '%s_%s' % (self.namespace, self.interface)
        # If IDL starts with omg.org/, don't include that in port name
        porttype = porttype.replace('omg.org/','')
        if self.direction == 'uses':
            porttype += 'OutPort'
        else:
            porttype += 'InPort'
        return porttype

    def _basename(self):
        name =  '.'.join((self.namespace, self.interface))
        # If IDL starts with omg.org/, java package actually starts with org.omg.
        name = name.replace('omg.org/','org.omg.')
        return name

    def interfaceClass(self):
        return self._basename() + 'Operations'

    def helperClass(self):
        return self._basename() + 'Helper'

    def poaClass(self):
        return self._basename() + 'POA'

    def _ctorArgs(self, name):
        return tuple()

    def constructor(self, name):
        return '%s(%s)' % (self.className(), ', '.join(self._ctorArgs(name)))

    def supportsMultiOut(self):
        return False

class BuiltinJavaPort(JavaPortGenerator):
    def __init__(self, javaclass, port):
        JavaPortGenerator.__init__(self, port)
        self.package, self.__name = javaclass.rsplit('.', 1)

    def className(self):
        return self.__name
