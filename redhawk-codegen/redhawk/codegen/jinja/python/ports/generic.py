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
from omniORB import CORBA

from redhawk.codegen.lang import python
from redhawk.codegen.jinja.ports import PortFactory
from redhawk.codegen.jinja.python import PythonTemplate

from generator import PythonPortGenerator

if not '__package__' in locals():
    # Python 2.4 compatibility
    __package__ = __name__.rsplit('.', 1)[0]

class GenericPortFactory(PortFactory):
    def match(cls, port):
        return True

    def generator(cls, port):
        if port.isProvides():
            return GenericPortGenerator('generic.provides.py', port)
        else:
            return GenericPortGenerator('generic.uses.py', port)

class GenericPortGenerator(PythonPortGenerator):
    def __init__(self, template, port):
        super(GenericPortGenerator,self).__init__(port)
        self.__template = PythonTemplate(template)

    def _ctorArgs(self, port):
        return ('self', python.stringLiteral(port.name()))

    def imports(self):
        if self.direction == 'provides':
            modules = [python.poaModule(self.namespace)]
        else:
            modules = [
                'ossie.cf.ExtendedCF',
                'ossie.cf.ExtendedCF__POA',
                python.idlModule(self.namespace)
            ]
        return [python.importModule(m) for m in modules]

    def poaClass(self):
        if self.direction == 'uses':
            return 'ExtendedCF__POA.QueryablePort'
        else:
            return super(GenericPortGenerator,self).poaClass()

    def loader(self):
        return jinja2.PackageLoader(__package__)

    def operations(self):
        for op in self.idl.operations():
            args = []
            returns = []
            if op.returnType.kind() != CORBA.tk_void:
                returns.append(str(op.returnType))
            for param in op.params:
                if param.direction in ('in', 'inout'):
                    args.append(param.name)
                if param.direction in ('inout', 'out'):
                    returns.append(str(param.paramType))
            _out = False
            for p in op.params:
                if p.direction == 'out':
                    _out = True
                    break
            _inout = False
            for p in op.params:
                if p.direction == 'inout':
                    _inout = True
                    break
            yield {'name': op.name,
                   'hasout': _out,
                   'hasinout': _inout,
                   'hasreturnType': str(op.returnType),
                   'args': args,
                   'returns': returns}
        for attr in self.idl.attributes():
            yield {'name': '_get_'+attr.name,
                   'args': [],
                   'is_attribute': True,
                   'base_attribute': attr.name,
                   'returns': [str(attr.attrType)]}
            if not attr.readonly:
                yield {'name': '_set_'+attr.name,
                       'args': ['data'],
                       'hasreturnType': 'void',
                       'returns': []}

    def _implementation(self):
        return self.__template
