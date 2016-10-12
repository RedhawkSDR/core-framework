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

import code
from code import SourceFile

class CppFactory(code.Factory):
    def __init__ (self):
        classes = (Code, Scope, If, Try, Class, Function, Switch, Namespace)
        super(CppFactory, self).__init__(*classes)

Code = code.factoryclass(code.Code, CppFactory)
Scope = code.factoryclass(code.Scope, CppFactory)
If = code.factoryclass(code.If, CppFactory)
Try = code.factoryclass(code.Try, CppFactory)
Function = code.factoryclass(code.Function, CppFactory)
Switch = code.factoryclass(code.Switch, CppFactory)

class Class(Scope):
    def __init__ (self, name, inherits=[]):
        super(Class, self).__init__('{', '};')
        self.__name = name
        self.__parents = inherits

    def write (self, f):
        decl = 'class ' + self.__name
        if self.__parents:
            decl += ' : ' + ', '.join(self.__parents)
        f.write(decl)
        super(Class, self).write(f)

class Namespace(Scope):
    def __init__ (self, name):
        super(Namespace, self).__init__('namespace '+name+' {')
        self.__name = name

class Module(Code):
    def __init__ (self):
        super(Module, self).__init__()
        self.__includes = []

    def include (self, hfile):
        if hfile in self.__includes:
            return
        self.__includes.append(hfile)
        
    def write (self, f):
        for hfile in self.__includes:
            f.write('#include %s', hfile)
        f.write()

        super(Module, self).write(f)

class Header(Module):
    def __init__ (self, name):
        super(Header, self).__init__()
        self.__guard = '__%s__' % (name.upper().replace('.', '_'))

    def write(self, f):
        f.write('#ifndef %s', self.__guard)
        f.write('#define %s', self.__guard)
        f.write()

        super(Header, self).write(f)

        f.write()
        f.write('#endif // %s', self.__guard)
