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
from redhawk.codegen.jinja.ports import PortFactory
from generator import PythonPortGenerator
from generic import GenericPortGenerator
from redhawk.codegen.lang import python 

class FrontendPortFactory(PortFactory):
    NAMESPACE = 'FRONTEND'

    def match(self, port):
        return IDLInterface(port.repid()).namespace() == self.NAMESPACE

    def generator(self, port):
        interface = IDLInterface(port.repid()).interface()
        if port.isUses():
            return GenericPortGenerator('generic.uses.py', port)
        return FrontendPortGenerator(port)

class FrontendProvidesPortFactory(PortFactory):
    NAMESPACE = 'FRONTEND'

    def match(self, port):
        return IDLInterface(port.repid()).namespace() == self.NAMESPACE

    def generator(self, port):
        interface = IDLInterface(port.repid()).interface()
        if port.isUses():
            return GenericPortGenerator('generic.uses.py', port)
        return FrontendPortGenerator(port)

class FrontendPortGenerator(PythonPortGenerator):
    def className(self):
        return "frontend." + self.templateClass()

    def templateClass(self):
        if self.direction == 'uses':
            porttype = 'Out'
        else:
            porttype = 'In'
        porttype += self.interface + 'Port'
        return porttype

    def _ctorArgs(self, port):
        return [python.stringLiteral(port.name())]

    def constructor(self, name):
        fei_ports = ['InDigitalTunerPort','InDigitalScanningTunerPort','InFrontendTunerPort','InAnalogTunerPort','InGPSPort','InRFInfoPort','InRFSourcePort','InNavDataPort']
        for _port in fei_ports:
            if _port in self.className():
                return '%s(%s, self)' % (self.className(), ', '.join(self._ctorArgs(name)))
        return '%s(%s)' % (self.className(), ', '.join(self._ctorArgs(name)))

    def loader(self):
        return jinja2.PackageLoader(__package__)
