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

class JavaFactory(code.Factory):
    def __init__(self):
        classes = (Code, Scope, If, Try, Class, Function)
        super(JavaFactory, self).__init__(*classes)

Code = code.factoryclass(code.Code, JavaFactory)
Scope = code.factoryclass(code.Scope, JavaFactory)
If = code.factoryclass(code.If, JavaFactory)
Try = code.factoryclass(code.Try, JavaFactory)
Function = code.factoryclass(code.Function, JavaFactory)

class Class(Scope):
    def __init__ (self, name, parent=None, visibility='public', abstract=False):
        super(Class, self).__init__()
        self.__name = name
        self.__modifiers = []
        if visibility:
            self.__modifiers.append('public')
        if abstract:
            self.__modifiers.append('abstract')
        self.__parent = parent
        self.__interfaces = []

    def implements (self, name):
        self.__interfaces.append(name)

    def write (self, f):
        decl = ' '.join(self.__modifiers + ['class', self.__name])
        if self.__parent:
            decl += ' extends ' + self.__parent
        if len(self.__interfaces) > 0:
            decl += ' implements ' + ','.join(self.__interfaces)
        f.write(decl)
        super(Class, self).write(f)

class Classfile(Code):
    def __init__ (self, package=None):
        super(Classfile, self).__init__()
        self.__package = package
        self.__imports = []

    def addImport (self, hfile):
        self.__imports.append(hfile)

    def write (self, f):
        if self.__package:
            f.write('package %s;', self.__package)
            f.write()

        if len(self.__imports) > 0:
            for target in self.__imports:
                f.write('import %s;', target)
            f.write()

        super(Classfile, self).write(f)
