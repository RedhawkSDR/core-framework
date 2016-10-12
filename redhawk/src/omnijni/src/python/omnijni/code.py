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

import new

def factoryclass (base, factory):
    def init (self, *args, **kwargs):
        base.__init__(self, *args, **kwargs)
        factory.__init__(self)
    return new.classobj(base.__name__, (base, factory), { '__init__': init })

class Factory(object):
    def __init__ (self, *classes):
        self.__classes = dict([(cls.__name__, cls) for cls in classes])

    def __getattr__ (self, name):
        if not name in self.__classes:
            raise KeyError, name
        return lambda *args, **kwargs: self._append(name, *args, **kwargs)
    
    def _create (self, name, *args, **kwargs):
        return self.__classes[name](*args, **kwargs)

    def _append (self, name, *args, **kwargs):
        code = self._create(name, *args, **kwargs)
        self.append(code)
        return code


class Code(object):
    def __init__ (self):
        self.__code = []

    def append (self, code="", *args):
        if code and isinstance(code, str):
            code = code % args
        self.__code.append(code)

    def write (self, f):
        for code in self.__code:
            if isinstance(code, str):
                f.write(code)
            else:
                code.write(f)

class Scope(Code):
    def __init__ (self, pre='{', post='}'):
        super(Scope, self).__init__()
        self.__pre = pre
        self.__post = post

    def write (self, f):
        f.write(self.__pre)
        f.indent()

        super(Scope, self).write(f)

        f.outdent()
        f.write(self.__post)

class _MultiClause(Scope):
    def __init__(self, *args, **kwargs):
        super(_MultiClause, self).__init__(*args, **kwargs)
        self.__clauses = []

    def _addClause (self, clause):
        code = self._create('Scope', clause)
        self.__clauses.append(code)
        return code

    def write (self, f):
        super(_MultiClause, self).write(f)
        for clause in self.__clauses:
            clause.write(f)

class If(_MultiClause):
    def __init__ (self, condition, *args):
        condition = condition % args
        super(If, self).__init__('if ('+condition+') {')

    def ElseIf (self, condition, *args):
        condition = condition % args
        return self._addClause('else if ('+condition+') {')

    def Else (self):
        return self._addClause('else {')

class Try(_MultiClause):
    def __init__ (self):
        super(Try, self).__init__('try {')

    def Catch (self, exception):
        return self._addClause('catch ('+exception+') {')

class Function(Scope):
    def __init__ (self, decl, *args):
        super(Function, self).__init__()
        self.__decl = decl
        self.__args = args

    def write (self, f):
        f.write(self.__decl, *self.__args)
        super(Function, self).write(f)

class Switch(Scope):
    def __init__(self, disc):
        super(Switch,self).__init__('switch ('+disc+') {')

    def Case(self, label):
        return self._append('Scope', 'case '+label+':', '')

    def Default(self):
        return self._append('Scope', 'default:', '')

class SourceFile:
    def __init__ (self, f, indent='  '):
        self.__file = f
        self.__indent_level = 0
        self.__indent_string = indent

    def write (self, line='', *args, **kwargs):
        if line:
            line = line % args
            line = self.__indent_string*self.__indent_level + line
        self.__file.write(line + '\n')

    def indent (self):
        self.__indent_level += 1

    def outdent (self):
        if (self.__indent_level == 0):
            raise IndentationError, 'Indentation level went below zero'
        self.__indent_level -= 1
