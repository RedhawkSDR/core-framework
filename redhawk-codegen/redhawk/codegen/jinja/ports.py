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

from environment import CodegenEnvironment

class PortFactory(object):
    def match(self, port):
        raise NotImplementedError, 'PortFactory.match'

    def generator(self, port):
        raise NotImplementedError, 'PortFactory.generator'


class PortFactoryList(object):
    def __init__(self, *factories):
        self.__factories = factories

    def match(self, port):
        for factory in self.__factories:
            if factory.match(port):
                return True
        return False
    
    def generator(self, port):
        for factory in self.__factories:
            if factory.match(port):
                return factory.generator(port)
        raise KeyError, 'Unsupported port type '+port.repid()


class PortGenerator(object):
    @classmethod
    def match(cls, port):
        return False

    @classmethod
    def generator(cls, port):
        return cls(port)

    def __init__(self, port):
        self.idl = IDLInterface(port.repid())
        self.namespace = self.idl.namespace()
        self.interface = self.idl.interface()
        if port.isProvides():
            self.direction = 'provides'
        else:
            self.direction = 'uses'

    def get_template(self, template):
        loader = self.loader()
        env = CodegenEnvironment(loader=loader, **template.options())
        env.filters.update(template.filters())
        return env.get_template(template.template)

    def loader(self):
        raise NotImplementedError, self.__class__.__name__+'.loader'

    def hasImplementation(self):
        return self._implementation() is not None
    
    def implementation(self):
        template = self._implementation()
        return self.get_template(template)

    def _implementation(self):
        return None
