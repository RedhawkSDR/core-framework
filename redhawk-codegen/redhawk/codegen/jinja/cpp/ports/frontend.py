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

import jinja2

from redhawk.codegen.lang.idl import IDLInterface
from redhawk.codegen.lang import cpp
from redhawk.codegen.jinja.ports import PortFactory
from redhawk.codegen.jinja.cpp import CppTemplate

from generator import CppPortGenerator

if not '__package__' in locals():
    # Python 2.4 compatibility
    __package__ = __name__.rsplit('.', 1)[0]

_headerMap = {
    'AnalogTuner':     '<frontend/fe_tuner_port_impl.h>',
    'DigitalTuner':    '<frontend/fe_tuner_port_impl.h>',
    'FrontendTuner':   '<frontend/fe_tuner_port_impl.h>',
    'GPS':             '<frontend/fe_gps_port_impl.h>',
    'NavData':         '<frontend/fe_navdata_port_impl.h>',
    'RFInfo':          '<frontend/fe_rfinfo_port_impl.h>',
    'RFSource':        '<frontend/fe_rfsource_port_impl.h>'
}

class FrontendPortFactory(PortFactory):
    NAMESPACE = 'FRONTEND'

    def match(self, port):
        return IDLInterface(port.repid()).namespace() == self.NAMESPACE

    def generator(self, port):
        interface = IDLInterface(port.repid()).interface()
        return FrontendPortGenerator(port)

class FrontendPortGenerator(CppPortGenerator):
    def header(self):
        return '<frontend/frontend.h>'

    def headers(self):
        return [_headerMap[self.interface]]

    def className(self):
        return "frontend::" + self.templateClass()

    def templateClass(self):
        if self.direction == 'uses':
            porttype = 'Out'
        else:
            porttype = 'In'
        porttype += self.interface + 'Port'
        return porttype

    def _ctorArgs(self, name):
        if self.direction == 'uses':
            return (cpp.stringLiteral(name),)
        else:
            return [cpp.stringLiteral(name),"this"]

    def loader(self):
        return jinja2.PackageLoader(__package__)

class FrontendOutputPortFactory(PortFactory):
    NAMESPACE = 'FRONTEND'

    def match(self, port):
        return IDLInterface(port.repid()).namespace() == self.NAMESPACE and (not port.isProvides())

    def generator(self, port):
        interface = IDLInterface(port.repid()).interface()
        return FrontendPortGenerator(port)

class FrontendOutputPortGenerator(CppPortGenerator):
    def header(self):
        return '<frontend/frontend.h>'

    def headers(self):
        return [_headerMap[self.interface]]

    def className(self):
        return "frontend::" + self.templateClass()

    def templateClass(self):
        if self.direction == 'uses':
            porttype = 'Out'
        else:
            porttype = 'In'
        porttype += self.interface + 'Port'
        return porttype

    def _ctorArgs(self, name):
        if self.direction == 'uses':
            return (cpp.stringLiteral(name),)
        else:
            return [cpp.stringLiteral(name),"this"]

    def loader(self):
        return jinja2.PackageLoader(__package__)

