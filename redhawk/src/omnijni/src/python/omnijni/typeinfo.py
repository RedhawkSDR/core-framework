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

from omniidl import idltype, idlast

_primitiveTypes = (
    idltype.tk_void,
    idltype.tk_boolean,
    idltype.tk_octet,
    idltype.tk_char,
    idltype.tk_short,
    idltype.tk_ushort,
    idltype.tk_long,
    idltype.tk_ulong,
    idltype.tk_longlong,
    idltype.tk_ulonglong,
    idltype.tk_float,
    idltype.tk_double,
)

def isEnum (itype):
    if not isinstance(itype, idltype.Type):
        return False
    itype = itype.unalias()
    return isinstance(itype, idltype.Declared) and isinstance(itype.decl(), idlast.Enum)

def isPrimitiveType (itype):
    if not isinstance(itype, idltype.Type):
        return False
    return itype.unalias().kind() in _primitiveTypes

def isString (itype):
    if not isinstance(itype, idltype.Type):
        return False
    return isinstance(itype.unalias(), idltype.String)

def isSequence (itype):
    if not isinstance(itype, idltype.Type):
        return False
    return isinstance(itype.unalias(), idltype.Sequence)

def isStruct (itype):
    itype = itype.unalias()
    return isinstance(itype, idltype.Declared) and isinstance(itype.decl(), idlast.Struct)

def structAllPrimatives (itype):
    if isStruct(itype):
        itype = itype.unalias()
        for m in itype.decl().members():
            if isPrimitiveType(m.memberType()) == False:
                return False
        return True
    return False


def isInterface (itype):
    itype = itype.unalias()
    if isinstance(itype, idltype.Declared):
        return isinstance(itype.decl(), (idlast.Interface, idlast.Forward))
    return False

def isDeclared (itype):
    if isinstance(itype, idlast.Typedef):
        itype = itype.aliasType()
    if isinstance(itype, idltype.Type):
        return isinstance(itype.unalias(), idltype.Declared)
    else:
        return isinstance(itype, idlast.DeclRepoId)

def isAny (itype):
    if not isinstance(itype, idltype.Type):
        return False
    itype = itype.unalias()
    return isinstance(itype, idltype.Base) and itype.kind() == idltype.tk_any

def isBuiltIn (itype):
    if isAny(itype):
        return True
    elif not isDeclared(itype):
        return False
    return itype.unalias().decl().builtIn()

def removeAlias (itype):
    if isinstance(itype, idlast.Typedef):
        itype = itype.aliasType()
    if isinstance(itype, idltype.Type):
        return itype.unalias()
    else:
        return itype

class Method:

    def __init__ (self, name, returnType, parameters, exceptions=() ):
        self.__name = name
        self.__returnType = returnType
        self.__parameters = parameters
        self.__exceptions = exceptions

    def name (self):
        return self.__name

    def stubName (self):
        return self.__name

    def returnType (self):
        return self.__returnType

    def parameters (self):
        return self.__parameters

    def exceptions (self):
        return self.__exceptions

class Getter (Method):

    def __init__ (self, name, attrType ):
        Method.__init__(self, name, attrType, [], [] )

    def stubName (self):
        return '_get_' + self.name()

class Setter (Method):
    
    def __init__ (self, name, attrType):
        returnType = idltype.Base(idltype.tk_void)
        param = idlast.Parameter(None, None, None, [], [], 0, attrType, "value")
        Method.__init__(self, name, returnType, [param], [])

    def stubName (self):
        return '_set_' + self.name()


class MethodVisitor:

    def getMethods (self, node, inherited=True):
        self.__inherited = inherited
        self.__methods = []
        node.accept(self)
        return self.__methods
    
    def visitInterface (self, node):
        if self.__inherited:
            callables = node.all_callables()
        else:
            callables = node.callables()
        for call in callables:
            call.accept(self)

    def visitAttribute (self, node):
        name = node.identifiers()[0]
        self.__methods.append(Getter(name, node.attrType())) 
        if not node.readonly():
            self.__methods.append(Setter(name, node.attrType()))
    
    def visitOperation (self, node):
        self.__methods.append(Method(node.identifier(), node.returnType(), node.parameters(), node.raises()))
