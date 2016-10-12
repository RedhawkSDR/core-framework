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

import os
import errno
from omniidl import idlast, idlvisitor, idltype
import javacode
from typeinfo import *

# Map of singleton Base Type objects
baseTypeMap = {
    idltype.tk_void:       "void",
    idltype.tk_short:      "short",
    idltype.tk_long:       "int",
    idltype.tk_ushort:     "short",
    idltype.tk_ulong:      "int",
    idltype.tk_float:      "float",
    idltype.tk_double:     "double",
    idltype.tk_boolean:    "boolean",
    idltype.tk_char:       "char",
    idltype.tk_octet:      "byte",
    idltype.tk_any:        "CORBA.Any",
    idltype.tk_longlong:   "long",
    idltype.tk_ulonglong:  "long",
    idltype.tk_longdouble: "longdouble",
    idltype.tk_wchar:      "wchar"
}

prefixes = [
    ('CORBA', 'org.omg'),
]

def qualifiedName(name):
    if not isinstance(name, str):
        name = '.'.join(name)
    for pattern, prefix in prefixes:
        if name.startswith(pattern + '.'):
            return prefix + '.' + name
    return name

def typeString(t):
    if isinstance(t, idltype.Base):
        name = baseTypeMap[t.kind()]
    elif isString(t):
        name = "String"
    elif isinstance(t, idltype.Declared):
        mod =  t.scopedName()[0]
        # This is somewhat of a hack, but assume that everything after the
        # module is an interface, with a package for any contained type.
        packages = [p+'Package' for p in t.scopedName()[1:-1]]
        name = [mod] + packages + [t.scopedName()[-1]]
    return qualifiedName(name)

def javaType (itype):
    itype = itype.unalias()
    if isinstance(itype, idltype.Sequence):
        return typeString(itype.seqType()) + '[]'
    else:
        return typeString(itype)

def holderType (itype):
    name = typeString(itype)
    if isPrimitiveType(itype):
        name = 'CORBA.' + name[0].upper() + name[1:]
    return qualifiedName(name + 'Holder')

def paramType (param):
    if param.is_out():
        return holderType(param.paramType())
    return javaType(param.paramType())

def poaName (node):
    return node.identifier() + 'POA'

def stubName (node):
    return '_' + node.identifier() + 'Stub'

def helperName (node):
    return node.identifier() + 'Helper'


class StubClass:

    def __init__ (self, package, libname):
        self.__package = package
        self.__libname = libname

    def generateJavaMethod (self, code, method):
        name = method.name()
        retType = method.returnType()
        argList = method.parameters()
        nativeName = method.stubName()
        rtype = javaType(retType)

        # Public Java instance method
        args = []
        for param in argList:
            args.append(paramType(param) + ' ' + param.identifier())
        argstr = ', '.join(args)
        body = code.Function('public %s %s (%s)', rtype, name, argstr)

        # Call the static native method
        args = [ 'this.ref_' ]
        for param in argList:
            args.append(param.identifier())
        argstr = ', '.join(args)
        if not nativeName:
            nativeName = name
        jnicall = '%s(%s);' % (nativeName, argstr)
        if rtype == 'void':
            body.append(jnicall)
        else:
            body.append('return %s', jnicall)

        # Signature for native method
        args = [ 'long __ref__' ]
        for param in argList:
            args.append(paramType(param) + ' ' + param.identifier())
        argstr = ', '.join(args)
        code.append('private static native %s %s (%s);', rtype, nativeName, argstr)
        code.append()

    def generate (self, node):
        name = stubName(node)
        classfile = javacode.Classfile(self.__package)

        interface = qualifiedName(node.scopedName())

        classdef = classfile.Class(name, 'omnijni.ObjectImpl')
        classdef.implements(interface)

        ctor = classdef.Function('public %s ()', name)
        classdef.append()

        ctor = classdef.Function('protected %s (long ref)', name)
        ctor.append('super(ref);')
        classdef.append()

        for method in MethodVisitor().getMethods(node):
            self.generateJavaMethod(classdef, method)

        # Generate list of supported CORBA interfaces.
        ids = classdef.Scope('private static String __ids[] = {', '};')
        for interface in [node] + node.inherits():
            ids.append('"%s",', interface.repoId())
        classdef.append()

        # Return CORBA interfaces
        body = classdef.Function('public String[] _ids ()')
        body.append('return (String[])__ids.clone();')
        classdef.append()

        # Static initializer, must load library
        initializer = classdef.Scope('static {')
        initializer.append('System.loadLibrary("%s");', self.__libname)
        classdef.append()

        classdef.append('protected native long _get_object_ref(long ref);')
        classdef.append('protected native long _narrow_object_ref(long ref);')

        return classfile


class HelperClass:

    def __init__ (self, package, interface):
        self.__package = package
        self.__interface = interface

    def generate (self, node):
        classfile = javacode.Classfile(self.__package)

        # Create the helper class
        parent = self.__interface + 'Helper'
        name = helperName(node)
        stub = self.__package + '.' + stubName(node)

        helperClass = classfile.Class(name, parent, abstract=True)
        body = helperClass.Function('public static %s narrow (org.omg.CORBA.Object obj)', self.__interface)
        cond = body.If('obj == null')
        cond.append('return null;')
        instanceof = cond.ElseIf('obj instanceof %s', stub)
        instanceof.append('return (%s)obj;', self.__interface)
        wrongtype = cond.ElseIf('!obj._is_a(id())')
        wrongtype.append('throw new org.omg.CORBA.BAD_PARAM();')
        native = cond.ElseIf('obj instanceof omnijni.ObjectImpl')
        native.append('%s stub = new %s();', stub, stub)
        native.append('long ref = ((omnijni.ObjectImpl)obj)._get_object_ref();')
        native.append('stub._set_object_ref(ref);')
        native.append('return (%s)stub;', self.__interface)
        narrow = cond.Else()
        narrow.append('org.omg.CORBA.ORB orb = ((org.omg.CORBA.portable.ObjectImpl)obj)._orb();')
        narrow.append('String ior = orb.object_to_string(obj);')
        narrow.append('return narrow(omnijni.ORB.string_to_object(ior));')
        return classfile


class POAClass:

    def __init__ (self, package, libname):
        self.__package = package
        self.__libname = libname

    def generate (self, node):
        name = poaName(node)
        classfile = javacode.Classfile(self.__package)
        interface = qualifiedName(node.scopedName()) + 'Operations'
        classdef = classfile.Class(name, 'omnijni.Servant', abstract=True)
        classdef.implements(interface)

        ctor = classdef.Function('public %s ()', name)
        classdef.append()

        body = classdef.Function('public org.omg.CORBA.Object _this_object (org.omg.CORBA.ORB orb)')
        body.append('this._activate();')
        body.append('long ref = %s.new_reference(this.servant_);', name)
        stubClass = self.__package + '.' + stubName(node)
        body.append('%s stub = new %s(ref);', stubClass, stubClass)
        body.append('String ior = omnijni.ORB.object_to_string(stub);')
        body.append('return orb.string_to_object(ior);')
        classdef.append()

        body = classdef.Function('public synchronized void _activate ()')
        clause = body.If('this.servant_ == 0')
        clause.append('this.servant_ = %s.new_servant();', name)
        clause.append('set_delegate(this.servant_, this);')
        classdef.append()

        body = classdef.Function('public synchronized void _deactivate ()')
        clause = body.If('this.servant_ != 0')
        clause.append('%s.del_servant(this.servant_);', name)
        clause.append('this.servant_ = 0;')
        classdef.append()

        # Static initializer, must load library
        initializer = classdef.Scope('static {')
        initializer.append('System.loadLibrary("%s");', self.__libname)
        classdef.append()

        classdef.append('private static native long new_servant();')
        classdef.append('private static native long del_servant(long servant);')
        classdef.append('private static native long new_reference(long servant);')
        classdef.append('private static native void set_delegate (long servant, %s delegate);', interface)
        classdef.append('private long servant_;')

        return classfile


class JavaVisitor(idlvisitor.AstVisitor):

    def __init__ (self, **options):
        self.__options = options

    def visitAST (self, node):
        for n in node.declarations():
            n.accept(self)

    def visitModule (self, node):
        if not node.mainFile():
            return

        for n in node.definitions():
            n.accept(self)        

    def visitInterface (self, node):
        package = qualifiedName(node.scopedName()[:-1] + ['jni'])

        # Ensure the directory structure is there
        path = os.path.join(*package.split('.'))
        try:
            os.makedirs(path)
        except OSError, e:
            # If the leaf directory already existed (or was created in the
            # interim), ignore the error
            if e.errno != errno.EEXIST:
                raise

        # Override library name with argument
        libname = self.__options.get('libname', None)
        if not libname:
            libname = node.scopedName()[-2].lower() + 'jni'

        # Generate the stub
        stubCode = StubClass(package, libname).generate(node)
        stubFile = os.path.join(path, stubName(node) + '.java')
        stubCode.write(javacode.SourceFile(open(stubFile, 'w')))

        # Generate the helper
        interface = qualifiedName(node.scopedName())
        helperCode = HelperClass(package, interface).generate(node)
        helperFile = os.path.join(path, helperName(node) + '.java')
        helperCode.write(javacode.SourceFile(open(helperFile, 'w')))

        # Generate the POA
        poaCode = POAClass(package, libname).generate(node)
        poaFile = os.path.join(path, poaName(node) + '.java')
        poaCode.write(javacode.SourceFile(open(poaFile, 'w')))


def run(tree, args):
    options = {}
    for arg in args:
        if arg.find('=') >= 0:
            key, value = arg.split('=', 1)
            if key == 'pkgprefix':
                pattern, prefix = value.split(':')
                prefixes.append((pattern, prefix))
            else:
                options[key] = value
        else:
            options[arg] = True

    visitor = JavaVisitor(**options)
    tree.accept(visitor)
