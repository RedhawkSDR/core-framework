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
from omniidl import idlast, idlvisitor, idltype
import cppcode
from typeinfo import *

primitiveTypeMap = {
    idltype.tk_void:      'void',
    idltype.tk_boolean:   'CORBA::Boolean',
    idltype.tk_octet:     'CORBA::Octet',
    idltype.tk_char:      'CORBA::Char',
    idltype.tk_short:     'CORBA::Short',
    idltype.tk_ushort:    'CORBA::UShort',
    idltype.tk_long:      'CORBA::Long',
    idltype.tk_ulong:     'CORBA::ULong',
    idltype.tk_longlong:  'CORBA::LongLong', 
    idltype.tk_ulonglong: 'CORBA::ULongLong',
    idltype.tk_float:     'CORBA::Float',
    idltype.tk_double:    'CORBA::Double',
}

descriptorTypeMap = {
    idltype.tk_void:      'V',
    idltype.tk_boolean:   'Z',
    idltype.tk_octet:     'B',
    idltype.tk_char:      'C',
    idltype.tk_short:     'S',
    idltype.tk_ushort:    'S', # No unsigned types in Java
    idltype.tk_long:      'I',
    idltype.tk_ulong:     'I', # No unsigned types in Java
    idltype.tk_longlong:  'J', 
    idltype.tk_ulonglong: 'J', # No unsigned types in Java
    idltype.tk_float:     'F',
    idltype.tk_double:    'D',
}

jniTypeMap = {
    idltype.tk_void:      'void',
    idltype.tk_boolean:   'jboolean',
    idltype.tk_octet:     'jbyte',
    idltype.tk_char:      'jchar',
    idltype.tk_short:     'jshort',
    idltype.tk_ushort:    'jshort',  # No unsigned types in Java
    idltype.tk_long:      'jint',
    idltype.tk_ulong:     'jint',    # No unsigned types in Java
    idltype.tk_longlong:  'jlong', 
    idltype.tk_ulonglong: 'jlong',   # No unsigned types in Java
    idltype.tk_float:     'jfloat',
    idltype.tk_double:    'jdouble',
    idltype.tk_string:    'jstring'
}

prefixes = [
    ('CORBA', 'org.omg'),
]

def nonnull_set (iterable):
    # Returns the set of non-null items in iterable
    return set(i for i in iterable if i is not None)

def qualifiedJavaName (name, separator='.'):
    if not isinstance(name, str):
        name = separator.join(name)
    for pattern, prefix in prefixes:
        if separator != '.':
            pattern = pattern.replace('.', separator)
            prefix = prefix.replace('.', separator)
        if name.startswith(pattern + separator):
            return prefix + separator + name
    return name

def javaDescriptor (itype, isOut=False):
    if isinstance(itype, idlast.Typedef):
        itype = itype.aliasType()
    if isSequence(itype) and not isOut:
        return '[' + javaDescriptor(itype.unalias().seqType())
    elif isPrimitiveType(itype) and not isOut:
        return descriptorTypeMap[itype.unalias().kind()]
    return 'L' + classDescriptor(itype, isOut) + ';'

def classDescriptor (itype, isOut=False):
    # TODO: Basic type holders (including sequences)
    if isString(itype):
        if isOut:
            desc = 'CORBA/String'
        else:
            return 'java/lang/String'
    elif isAny(itype):
        desc = 'CORBA/Any'
    elif isPrimitiveType(itype):
        desc = jniTypeMap[itype.kind()]
        desc = 'CORBA/' + desc[1].upper() + desc[2:]
    else:
        scopedName = itype.scopedName()[:]
        packages = [ pkg+'Package' for pkg in scopedName[1:-1]]
        scopedName[1:-1] = packages
        desc = '/'.join(scopedName)
    if isOut:
        desc += 'Holder'
    return qualifiedJavaName(desc, '/')

def dependencyClass (itype, isOut=False):
    if isinstance(itype, idlast.Typedef):
        itype = itype.aliasType()
    if not isOut:
        if isPrimitiveType(itype) or isString(itype):
            return None
        elif isSequence(itype):
            return dependencyClass(itype.unalias().seqType())
    name = helperName(itype, isOut)
    return name

def methodDescriptor (retType, argList):
    desc = '('
    for param in argList:
        desc += javaDescriptor(param.paramType(), param.is_out())
    desc += ')' + javaDescriptor(retType)
    return desc

def typeString(t):
    if isPrimitiveType(t):
        return primitiveTypeMap[t.unalias().kind()]
    elif isString(t):
        return 'char*'
    elif isAny(t):
        return 'CORBA::Any'
    elif isinstance(t, idltype.Declared):
        return '::'.join(t.scopedName())

def returnType(t):
    if isSequence(t):
        return typeString(t) + '*'
    if isStruct(t):    
        if structAllPrimatives(t) :
            return typeString(t)
        else:
            return typeString(t) + '*'
    elif isInterface(t):
        return typeString(t) + '_ptr'
    elif isAny(t):
        return 'CORBA::Any*'
    return typeString(t)

def parameterType(param):
    paramType = param.paramType()
    decl = typeString(paramType)
    if isPrimitiveType(paramType) or isEnum(paramType):
        byRef = param.is_out()
    elif not param.is_in():
        if isString(paramType):
            decl = 'CORBA::String'
        decl += '_out'
        byRef = False
    elif isInterface(paramType):
        decl += '_ptr'
        byRef = param.is_out()
    else:
        if isString(paramType):
            byRef = param.is_out()
        else:
            byRef = True
        if not param.is_out():
            decl = 'const ' + decl
    if byRef:
        decl += '&'
    return decl

def temporaryType(itype, owner=True):
    if isinstance(itype, idlast.Parameter):
        owner = not itype.is_in()
        itype = itype.paramType()
    if isEnum(itype) or isPrimitiveType(itype):
        # Primitives and enums are by-value, no need to take ownership
        owner = False
    if isString(itype) or isInterface(itype) or owner:
        return varType(itype)
    else:
        return typeString(itype)

def varType (itype):
    if isString(itype):
        return 'CORBA::String_var'
    else:
        return typeString(itype) + '_var'

def jniType (itype):
    itype = itype.unalias()
    if isSequence(itype):
        seqType = itype.seqType()
        if isString(seqType):
            return 'jobjectArray'
        else:
            return jniType(itype.seqType()) + 'Array'
    else:
        return jniTypeMap.get(itype.kind(), 'jobject')

def jniParameterType (param):
    if param.is_out():
        return 'jobject'
    else:
        return jniType(param.paramType())

def methodReturnType (itype):
    if isSequence(itype) or isString(itype):
        return 'jobject'
    return jniType(itype)

def methodNameType (itype):
    jtype = methodReturnType(itype)
    if jtype.startswith('j'):
        jtype = jtype[1:]
    return jtype[0].upper() + jtype[1:]

def throwClause (exList):
    if not exList:
        return ''
    names = ['::'.join(ex.scopedName()) for ex in exList]
    return ' throw (' + ', '.join(names) +')'

def mangleUnderscores (name):
    outname = ''
    count = 0
    for c in name:
        if c == '_':
            count += 1
        else:
            if count > 0:
                outname += '_%d' % count
                count = 0
            outname += c
    return outname
    
def jniFunctionName (name):
    return 'Java_' + '_'.join(map(mangleUnderscores, name.split('.')))

def jniScopedName (name):
    scopedName = name.scopedName()
    return scopedName[:-1] + ['jni'] + scopedName[-1:]

def helperName (itype, isOut=False):
    if isAny(itype):
        scopedName = ['CORBA', 'Any']
    elif isString(itype):
        scopedName = ['CORBA', 'String']
    elif isPrimitiveType(itype):
        scopedName = typeString(itype).split('::')
    elif isinstance(itype, idltype.Sequence) and not isOut:
        # Bare sequence (no typedef); use the functions from the 'omnijni'
        # namespace directly.
        return 'omnijni'
    else:
        if isDeclared(itype):
            itype = removeAlias(itype)
        scopedName = itype.scopedName()[:]
    scopedName.insert(1, 'jni')
    helper = '::'.join(scopedName)
    if isOut:
        helper += 'Holder'
    return helper

def holderName (itype):
    return helperName(itype, True)

def getDependency (itype):
    if isinstance(itype, idltype.Type):
        if isSequence(itype):
            if itype.kind() != idltype.tk_alias:
                itype = itype.seqType()
        if not isinstance(itype, idltype.Declared):
            return None
        decl = itype.decl()
    else:
        decl = itype
    if decl.mainFile() or decl.builtIn():
        return None
    return decl.file()

def dependencyHeader (dependency):
    dirname = os.path.dirname(dependency)
    filename = 'jni_' + os.path.basename(dependency).replace('.idl', '.h')
    if not dirname:
        # No directory path on include file, assume it was a local include
        include = '"%s"'
    else:
        include = '<%s>'
    return include % (os.path.join(dirname, filename))


class HelperBase(object):

    def name (self, node):
        if isinstance(node, idlast.Typedef):
            node = node.declarators()[0]
        return node.identifier()

    def parents (self, node):
        return []

    def isConcreteClass (self):
        return False

    def javaClass (self, node):
        if isinstance(node, idlast.Typedef):
            node = node.declarators()[0]
        return classDescriptor(node).replace('/', '.')

    def qualifiedName (self, node):
        if isinstance(node, idlast.Typedef):
            node = node.declarators()[0]
        return helperName(node)

    def generateDecl (self, node):
        name = self.name(node)
        clazz = cppcode.Class(name, self.parents(node))
        clazz.append('public:')

        self.generateChildDecls(clazz, node)

        if isinstance(self, PeerHelper):
            # Conversion from Java object
            clazz.append('static void fromJObject (%s out, JNIEnv* env, jobject obj);', self.outType(node))

            # Conversion to Java object
            clazz.append('static jobject toJObject (%s in, JNIEnv* env);', self.inType(node))
            clazz.append()

        # Class object
        body = clazz.Function('static jclass getJClass (JNIEnv* env)')
        body.append('return cls_;')
        clazz.append()

        self.generateMethodDecls(clazz, node)

        # Load/Unload
        # The Load method is intended to be the "public" interface, as it
        # serializes access to the omnijni classloader. OnLoad needs to be
        # public so that other JNI helpers can call it, but it should only be
        # called with the classloader mutex held.
        load = clazz.Function('static void Load (JNIEnv* env)')
        load.append('omni_mutex_lock lock_(omnijni::sharedMutex());')
        load.append('OnLoad(env);')
        clazz.append()
        clazz.append('static void OnLoad (JNIEnv* env);')
        clazz.append('static void OnUnload (JNIEnv* env);')
        clazz.append()

        # Private members
        clazz.append('private:')
        if not self.isConcreteClass():
            clazz.append('%s();', name)
            clazz.append('~%s();', name)
        clazz.append('static jclass cls_;');
        self.generateMemberDecls(clazz, node)

        return clazz

    def generateImpl (self, node):
        code = cppcode.Code()
        qualName = self.qualifiedName(node)

        # Declare storage for and initialize members.
        code.append('jclass %s::cls_ = NULL;', qualName)
        self.generateMemberImpls(code, node)
        code.append()

        # JNI OnLoad function code
        body = code.Function('void %s::OnLoad (JNIEnv* env)', qualName)

        # Only need to run once
        body.append('if (cls_) return;')
        body.append()

        # Ensure all dependencies are loaded
        for dep in sorted(self.dependencies(node)):
            body.append('%s::OnLoad(env);', dep)

        # Get class object
        javaName = self.javaClass(node)
        body.append('cls_ = omnijni::loadClass(env, "%s");', javaName)
        self.generateOnLoad(body, node)
        code.append()

        # JNI OnUnload function code
        body = code.Function('void %s::OnUnload (JNIEnv* env)', qualName)
        self.generateOnUnload(body, node)
        code.append()

        # For classes with a Java peer
        if isinstance(self, PeerHelper):
            # Conversion from Java object
            body = code.Function('void %s::fromJObject (%s out, JNIEnv* env, jobject obj)', qualName, self.outType(node))
            self.generateFromJObject(body, node)
            code.append()

            # Conversion to Java object
            body = code.Function('jobject %s::toJObject (%s in, JNIEnv* env)', qualName, self.inType(node))
            self.generateToJObject(body, node)
            code.append()

        # Additional subclass methods
        self.generateMethodImpls(code, node)

        return code

    def dependencies (self, node):
        return []

    def generateChildDecls (self, body, node):
        pass

    def generateMethodDecls (self, body, node):
        pass

    def generateMethodImpls (self, body, node):
        pass

    def generateMemberDecls (self, body, node):
        pass

    def generateMemberImpls (self, body, node):
        pass

    def generateOnLoad (self, body, node):
        pass

    def generateOnUnload (self, body, node):
        pass
    

class PeerHelper (HelperBase):
    def byValue (self, node):
        if isinstance(node, idlast.Typedef):
            node = node.aliasType()
        return isPrimitiveType(node) or isEnum(node)

    def typeName (self, node):
        if isinstance(node, idlast.Typedef):
            node = node.declarators()[0]
        return '::'.join(node.scopedName())

    def outType(self, node):
        return self.typeName(node) + '&'

    def inType (self, node):
        typeName = self.typeName(node)
        if not self.byValue(node):
            typeName = 'const ' + typeName + '&'
        return typeName

    def generateToJObject (self, body, node):
        pass

    def generateFromJObject (self, body, node):
        pass


class EnumHelper(PeerHelper):

    def byValue (self, node):
        return True

    def generateToJObject (self, body, node):
        body.append('return env->CallStaticObjectMethod(cls_, from_int_, (jint)in);')
    
    def generateFromJObject (self, body, node):
        target = '::'.join(node.scopedName())
        body.append('out = (%s)env->CallIntMethod(obj, value_);', target)
        
    def generateMemberDecls (self, body, node):
        body.append('static jmethodID from_int_;');
        body.append('static jmethodID value_;')

    def generateMemberImpls (self, body, node):
        qualName = helperName(node)
        body.append('jmethodID %s::from_int_ = 0;', qualName)
        body.append('jmethodID %s::value_ = 0;', qualName)

    def generateOnLoad (self, body, node):
        # Get method IDs for converting to/from int
        javaDesc = classDescriptor(node)
        body.append('from_int_ = env->GetStaticMethodID(cls_, "from_int", "(I)L%s;");', javaDesc)
        body.append('value_ = env->GetMethodID(cls_, "value", "()I");')
        

class HolderHelper(PeerHelper):

    def dependencies (self, node):
        return nonnull_set([dependencyClass(node)])

    def name (self, node):
        if isPrimitiveType(node) or isAny(node):
            return typeString(node).split('::')[-1] + 'Holder'
        elif isString(node):
            return 'StringHolder'
        else:
            return HelperBase.name(self, node) + 'Holder'

    def typeName (self, node):
        if isPrimitiveType(node) or isAny(node) or isString(node):
            return typeString(node)
        else:
            return super(HolderHelper, self).typeName(node)

    def qualifiedName (self, node):
        return HelperBase.qualifiedName(self, node) + 'Holder'

    def javaClass (self, node):
        if isString(node):
            return qualifiedJavaName('CORBA.StringHolder')
        else:
            return HelperBase.javaClass(self, node) + 'Holder'

    def inType (self, node):
        if isString(node):
            return 'const char*'
        elif isinstance(node, idlast.Interface):
            return super(HolderHelper, self).typeName(node) + '_ptr'
        else:
            return super(HolderHelper, self).inType(node)

    def outType (self, node):
        if isString(node):
            return 'CORBA::String_out'
        elif isinstance(node, idlast.Interface):
            return super(HolderHelper, self).typeName(node) + '_out'
        else:
            return super(HolderHelper, self).outType(node)

    def generateToJObject (self, body, node):
        if isPrimitiveType(node):
            jtype = jniType(node)
            body.append('%s value = (%s)in;', jtype, jtype)
        else:
            body.append('jobject value = ::toJObject(in, env);')
        body.append('return env->NewObject(cls_, ctor_, value);')

    def generateFromJObject (self, body, node):
        if isPrimitiveType(node):
            jtype = jniType(node)
            mtype = jtype[1].upper() + jtype[2:]
            body.append('out = (%s)env->Get%sField(obj, value_);', self.typeName(node), mtype)
        else:
            body.append('jobject value = env->GetObjectField(obj, value_);')
            body.append('::fromJObject(out, env, value);')

    def generateMethodDecls (self, body, node):
        body.append('static void setValue (JNIEnv* env, jobject holder, %s value);', self.inType(node))
        body.append('static jobject newInstance (JNIEnv* env);')
        body.append()

    def generateMethodImpls (self, body, node):
        qualName = self.qualifiedName(node)
        code = body.Function('void %s::setValue (JNIEnv* env, jobject holder, %s value)', qualName, self.inType(node))
        if isPrimitiveType(node):
            jtype = jniType(node)
            mtype = jtype[1].upper() + jtype[2:]
            code.append('env->Set%sField(holder, value_, (%s)value);', mtype, jtype)
        else:
            code.append('jobject obj = ::toJObject(value, env);')
            code.append('env->SetObjectField(holder, value_, obj);')
        body.append()
        code = body.Function('jobject %s::newInstance (JNIEnv* env)', qualName)
        code.append('return env->NewObject(cls_, ctor_, (jobject)NULL);')

    def generateMemberDecls (self, body, node):
        body.append('static jmethodID ctor_;')
        body.append('static jfieldID value_;')

    def generateMemberImpls (self, body, node):
        qualName = self.qualifiedName(node)
        body.append('jmethodID %s::ctor_ = 0;', qualName)
        body.append('jfieldID %s::value_ = 0;', qualName)

    def generateOnLoad (self, body, node):
        desc = javaDescriptor(node)
        body.append('ctor_ = env->GetMethodID(cls_, "<init>", "(%s)V");', desc)
        body.append('value_ = env->GetFieldID(cls_, "value", "%s");', desc)


class SequenceHelper (object):

    def generateDecl (self, node):
        name = node.identifier()
        clazz = cppcode.Class(name)
        clazz.append('public:')

        typeName = '::'.join(node.scopedName())
        # Conversion from Java object
        body = clazz.Function('static inline void fromJObject (%s& out, JNIEnv* env, jobject obj)', typeName)
        body.append('omnijni::fromJObject(out, env, obj);')

        # Conversion to Java object
        body = clazz.Function('static inline jobject toJObject (%s in, JNIEnv* env)', typeName)
        body.append('return omnijni::toJObject(in, env);')

        return clazz


class StructHelper (PeerHelper):

    def dependencies (self, node):
        return nonnull_set(dependencyClass(m.memberType()) for m in node.members())

    def byValue (self, node):
        return False

    def generateToJObject(self, body, node):
        ctorArgs = ['cls_', 'ctor_']
        pre = body.Code()
        body.append()
        post = body.Code()
        for index, m in enumerate(node.members()):
            idlType = m.memberType()
            decl = m.declarators()[0]
            fieldName = decl.identifier()
            if isPrimitiveType(idlType):
                kind = jniType(idlType)
                kind = kind[1].upper() + kind[2:]
                ctorArgs.append('in.'+fieldName)
                continue
            argName = 'arg%d__' % (index,)
            qualName = helperName(idlType)
            pre.append('jobject %s = %s::toJObject(in.%s, env);', argName, qualName, fieldName);
            ctorArgs.append(argName)
            post.append('env->DeleteLocalRef(%s);', argName)

        # Call constructor at end of preamble
        pre.append('jobject retval__ = env->NewObject(%s);', ', '.join(ctorArgs))

        # Add return statement to the end of the postamble
        post.append('return retval__;')

    def generateFromJObject(self, body, node):
        for index, m in enumerate(node.members()):
            idlType = m.memberType()
            decl = m.declarators()[0]
            fieldName = decl.identifier()
            idlType = idlType.unalias()
            if isPrimitiveType(idlType):
                kind = jniType(idlType)
                kind = kind[1].upper() + kind[2:]
                body.append('out.%s = env->Get%sField(obj, fid_[%d]);', fieldName, kind, index)
            else:
                body.append('omnijni::getObjectField(out.%s, env, obj, fid_[%d]);', fieldName, index)

    def generateMemberDecls (self, body, node):
        body.append('static jmethodID ctor_;')
        body.append('static jfieldID fid_[%d];', len(node.members()))

    def generateMemberImpls (self, body, node):
        qualName = self.qualifiedName(node)
        body.append('jmethodID %s::ctor_ = 0;', qualName)
        body.append('jfieldID %s::fid_[%d];', qualName, len(node.members()))

    def generateOnLoad (self, body, node):
        # Get n-argument constructor
        ctorArgs = ''
        for m in node.members():
            ctorArgs += javaDescriptor(m.memberType())
        body.append('ctor_ = env->GetMethodID(cls_, "<init>", "(%s)V");', ctorArgs)

        # Get IDs for all fields
        for index, member in enumerate(node.members()):
            decl = member.declarators()[0]
            fieldName = decl.identifier()
            descriptor = javaDescriptor(member.memberType())
            body.append('fid_[%d] = env->GetFieldID(cls_, "%s", "%s");', index, fieldName, descriptor)


# For our purposes, Exceptions are essentially the same as Structs
class ExceptionHelper(StructHelper):

    def generateMethodDecls (self, body, node):
        StructHelper.generateMethodDecls(self, body, node)
        body.append('static void throwIf (JNIEnv* env, jobject obj);')

    def generateMethodImpls (self, body, node):
        StructHelper.generateMethodImpls(self, body, node)
        code = body.Function('void %s::throwIf (JNIEnv* env, jobject obj)', self.qualifiedName(node))
        inner = code.If('env->IsInstanceOf(obj, cls_)')
        inner.append('%s exc;', self.typeName(node))
        inner.append('fromJObject(exc, env, obj);')
        inner.append('env->DeleteLocalRef(obj);')
        inner.append('throw exc;')
        body.append()


class UnionHelper(PeerHelper):

    def dependencies (self, node):
        depend = [node.switchType()]
        depend.extend(c.caseType() for c in node.cases())
        return nonnull_set(dependencyClass(d) for d in depend)

    def byValue (self, node):
        return False

    def _generateCaseLabels (self, body, node, case):
        # TODO: primitive discriminant typess
        enumscope = '::'.join(node.switchType().scopedName()[:-1])
        for label in case.labels():
            code = body.Case(enumscope + '::' + label.value().identifier())
        return code

    def generateFromJObject (self, body, node):
        body.append('jobject jdisc = env->CallObjectMethod(obj, disc_);')
        body.append('%s disc;', typeString(node.switchType()))
        body.append('%s::fromJObject(disc, env, jdisc);', helperName(node.switchType()))
        body.append('env->DeleteLocalRef(jdisc);')
        body.append('jobject jvalue = NULL;')
        switch = body.Switch('disc')
        for index, case in enumerate(node.cases()):
            code = self._generateCaseLabels(switch, node, case)
            inner = code.Scope()
            caseType = case.caseType()
            inner.append('%s value;', temporaryType(caseType, True))
            if isPrimitiveType(caseType):
                inner.append('value = env->Call%sMethod(obj, get_[%d]);', methodNameType(caseType), index)
            else:
                inner.append('jvalue = env->CallObjectMethod(obj, get_[%d]);', index)
                inner.append('%s::fromJObject(value, env, jvalue);', helperName(caseType))
            inner.append('out.%s(value);', case.declarator().identifier())
            code.append('break;')
        cleanup = body.If('jvalue != NULL')
        cleanup.append('env->DeleteLocalRef(jvalue);')
        body.append()

    def generateToJObject (self, body, node):
        body.append('jobject out = env->NewObject(cls_, ctor_);')
        body.append('jobject value = NULL;')
        switch = body.Switch('in._d()')
        for index, case in enumerate(node.cases()):
            code = self._generateCaseLabels(switch, node, case)
            caseType = case.caseType()
            caseName = case.declarator().identifier()
            if isPrimitiveType(caseType):
                code.append('env->CallVoidMethod(out, set_[%d], (%s)in.%s());', index, jniType(caseType), caseName)
            else:
                helper = helperName(caseType)
                code.append('value = %s::toJObject(in.%s(), env);', helper, caseName)
                code.append('env->CallVoidMethod(out, set_[%d], value);', index)
            code.append('break;')
        switch.Default().append('break;')
        cleanup = body.If('value != NULL')
        cleanup.append('env->DeleteLocalRef(value);')
        body.append('return out;')

    def generateOnLoad (self, body, node):
        body.append('ctor_ = env->GetMethodID(cls_, "<init>", "()V");')
        body.append('disc_ = env->GetMethodID(cls_, "discriminator", "()%s");', javaDescriptor(node.switchType()))
        for index, case in enumerate(node.cases()):
            desc = javaDescriptor(case.caseType())
            name = case.declarator().identifier()
            body.append('get_[%d] = env->GetMethodID(cls_, "%s", "()%s");', index, name, desc)
            body.append('set_[%d] = env->GetMethodID(cls_, "%s", "(%s)V");', index, name, desc)

    def generateMemberDecls (self, body, node):
        body.append('static jmethodID ctor_;')
        body.append('static jmethodID disc_;')
        body.append('static jmethodID get_[%d];', len(node.cases()))
        body.append('static jmethodID set_[%d];', len(node.cases()))

    def generateMemberImpls (self, body, node):
        qualName = self.qualifiedName(node)
        body.append('jmethodID %s::ctor_ = 0;', qualName)
        body.append('jmethodID %s::disc_ = 0;', qualName)
        body.append('jmethodID %s::get_[%d];', qualName, len(node.cases()))
        body.append('jmethodID %s::set_[%d];', qualName, len(node.cases()))

class POAHelper (HelperBase):

    def inherits (self, node):
        return ['::'.join(jniScopedName(inh))+'POA' for inh in node.inherits()]

    def dependencies (self, node):
        deps = set()
        deps.update(self.inherits(node))
        for method in self.__methods:
            deps.update(self.methodDeps(method))
        return deps

    def methodDeps (self, method):
        deps = [dependencyClass(method.returnType())]
        deps.extend(dependencyClass(p.paramType(), p.is_out()) for p in method.parameters())
        deps.extend(dependencyClass(e) for e in method.exceptions())
        return nonnull_set(deps)

    def name (self, node):
        return HelperBase.name(self, node) + 'POA'

    def isConcreteClass (self):
        return True

    def parents (self, node):
        parent = node.scopedName()[:]
        parent[-2] = 'POA_' + parent[-2]
        poaBase = 'public virtual ' + '::'.join(parent)
        classes = ['public virtual omnijni::Servant', poaBase]
        classes.extend('public virtual '+inh for inh in self.inherits(node))
        return classes

    def qualifiedName (self, node):
        return HelperBase.qualifiedName(self, node) + 'POA'

    def javaClass (self, node):
        return HelperBase.javaClass(self, node) + 'Operations'

    def generateMethodDecls (self, body, node):
        for method in self.__methods:
            rtype = returnType(method.returnType())
            args = []
            for param in method.parameters():
                paramType = parameterType(param)
                paramName = param.identifier()
                args.append(paramType + ' ' + paramName)
            argstr = ', '.join(args)
            body.append('%s %s (%s);', rtype, method.name(), argstr)
        body.append()

    def generateMethodImpls (self, body, node):
        qualName = self.qualifiedName(node)
        for index, method in enumerate(self.__methods):
            name = qualName+'::'+method.name()
            self._generateWrapper(body, index, name, method)

        # Get the Java peer class with the correct JNI package path
        javaName = qualifiedJavaName(jniScopedName(node)) + 'POA'

        # JNI ctor
        newServant = jniFunctionName(javaName + '.new_servant')
        code = body.Function('extern "C" JNIEXPORT jlong JNICALL %s (JNIEnv* env, jclass)', newServant)
        # Load all JNI helpers required for this POA. Because methods invoked
        # via CORBA run on non-Java threads, we need to load any class that
        # might be used before any CORBA calls can be received; this function
        # is always invoked from a Java context, which should ensure that all
        # loads are successful.
        code.append('%s::Load(env);', qualName)
        code.append('%s* servant = new %s();', qualName, qualName)
        code.append('CORBA::release(servant->_this());')
        code.append('servant->_remove_ref();')
        code.append('return reinterpret_cast<jlong>(servant);')
        body.append()

        # JNI dtor
        delServant = jniFunctionName(javaName + '.del_servant')
        code = body.Function('extern "C" JNIEXPORT void JNICALL %s (JNIEnv* env, jclass, jlong jservant)', delServant)
        code.append('%s* servant = reinterpret_cast<%s*>(jservant);', qualName, qualName)
        code.append('servant->_set_delegate(env, NULL);')
        code.append('PortableServer::POA_var poa = servant->_default_POA();')
        code.append('PortableServer::ObjectId_var oid = poa->servant_to_id(servant);')
        code.append('poa->deactivate_object(oid);')
        body.append()

        # Create new reference
        newRef = jniFunctionName(javaName + '.new_reference')
        code = body.Function('extern "C" JNIEXPORT jlong JNICALL %s (JNIEnv* env, jclass, jlong jservant)', newRef)
        code.append('%s* servant = reinterpret_cast<%s*>(jservant);', qualName, qualName)
        code.append('return reinterpret_cast<jlong>(servant->_this());')
        body.append()

        # Set delegate
        setDelegate = jniFunctionName(javaName + '.set_delegate')
        code = body.Function('extern "C" JNIEXPORT void JNICALL %s (JNIEnv* env, jclass, jlong jservant, jobject delegate)', setDelegate)
        code.append('%s* servant = reinterpret_cast<%s*>(jservant);', qualName, qualName)
        code.append('servant->_set_delegate(env, delegate);')
        body.append()

    def _generateWrapper (self, body, index, name, method):
        # Generate signature
        retType = method.returnType()
        argList = method.parameters()
        rtype = returnType(retType)
        args = []
        for param in argList:
            paramType = parameterType(param)
            paramName = param.identifier()
            args.append(paramType + ' ' + paramName)
        argstr = ', '.join(args)
        func = body.Function('%s %s (%s)', rtype, name, argstr)

        # Create preamble and postamble, to be filled in per-argument
        pre = func.Code()
        pre.append('JNIEnv* env__ = omnijni::attachThread(jvm_);')
        func.append()
        code = func.Code()
        func.append()
        post = func.Code()
        func.append()

        args = ['delegate_', 'mid_[%d]'%(index,)]
        for argnum, param in enumerate(argList):
            paramType = param.paramType()
            if isPrimitiveType(paramType) and not param.is_out():
                args.append(param.identifier())
                continue

            argname = param.identifier()
            localname = 'arg%d__' % (argnum,)

            helper = helperName(paramType, param.is_out())

            if param.is_in():
                pre.append('jobject %s = %s::toJObject(%s, env__);', localname, helper, argname)
            else:
                pre.append('jobject %s = %s::newInstance(env__);', localname, helper)

            if param.is_out():
                if not param.is_in() and (isSequence(paramType) or isStruct(paramType)):
                    # Sequence and struct out types work a little differently,
                    # requiring us to allocate a new instance and dereference
                    # the pointer when passing to the conversion function
                    post.append('%s = new %s();', argname, typeString(paramType))
                    argname = '*'+argname+'.ptr()'

                post.append('%s::fromJObject(%s, env__, %s);', helper, argname, localname)

            args.append(localname)
            post.append('env__->DeleteLocalRef(%s);', localname)

        argstr = ', '.join(args)
        jtype = methodReturnType(retType)
        if jtype == 'void':
            code.append('env__->CallVoidMethod(%s);', argstr)
        else:
            code.append('%s rv__ = env__->Call%sMethod(%s);', jtype, methodNameType(retType), argstr)
        
        code.append('jthrowable exc__ = env__->ExceptionOccurred();')

        rethrow = post.If('exc__ != NULL')
        rethrow.append('env__->ExceptionClear();')
        for excType in method.exceptions():
            rethrow.append('%s::throwIf(env__, exc__);', helperName(excType))
        rethrow.append('CORBA::jni::SystemException::throwNative(env__, exc__);')

        if jtype != 'void':
            if isPrimitiveType(retType):
                func.append('return rv__;')
            else:
                if isEnum(retType):
                    func.append('%s rvout__ = (%s)0;', rtype, rtype)
                    rval = 'rvout__'
                else:
                    func.append('%s rvout__;', varType(retType))
                    if isSequence(retType) or isStruct(retType):
                        func.append('rvout__ = new %s();', typeString(retType))
                    rval = 'rvout__._retn()'

                func.append('%s::fromJObject(rvout__, env__, rv__);', helperName(retType))
                func.append('env__->DeleteLocalRef(rv__);')
                func.append('return %s;', rval)

        body.append()

    def generateMemberDecls (self, body, node):
        body.append('static JavaVM* jvm_;')
        if self.__methods:
            body.append('static jmethodID mid_[%d];', len(self.__methods))

    def generateMemberImpls (self, body, node):
        qualName = self.qualifiedName(node)
        body.append('JavaVM* %s::jvm_ = NULL;', qualName)
        if self.__methods:
            body.append('jmethodID %s::mid_[%d];', qualName, len(self.__methods))

    def generateOnLoad (self, body, node):
        body.append('env->GetJavaVM(&jvm_);')
        for index, method in enumerate(self.__methods):
            args = methodDescriptor(method.returnType(), method.parameters())
            body.append('mid_[%d] = env->GetMethodID(cls_, "%s", "%s");', index, method.name(), args)

    def generateDecl (self, node):
        self.__methods = MethodVisitor().getMethods(node, False)
        return HelperBase.generateDecl(self, node)

    def generateImpl (self, node):
        self.__methods = MethodVisitor().getMethods(node, False)
        return HelperBase.generateImpl(self, node)


class StubHelper (PeerHelper):

    def dependencies (self, node):
        deps = set()
        for method in MethodVisitor().getMethods(node):
            deps.update(self.methodDeps(method))
        return deps

    def methodDeps (self, method):
        deps = [dependencyClass(method.returnType())]
        deps.extend(dependencyClass(p.paramType(), p.is_out()) for p in method.parameters())
        deps.extend(dependencyClass(e) for e in method.exceptions())
        return nonnull_set(deps)

    def javaClass (self, node):
        name = jniScopedName(node)[:-1]
        name.append('_' + node.identifier() + 'Stub')
        return qualifiedJavaName(name)

    def inType (self, node):
        return self.typeName(node) + '_ptr'

    def outType (self, node):
        return self.typeName(node) + '_out'

    def generateToJObject (self, body, node):
        body.append('jlong ref = reinterpret_cast<jlong>(%s::_duplicate(in));', self.typeName(node))
        body.append('return env->NewObject(cls_, ctor_, ref);')

    def generateFromJObject (self, body, node):
        block = body.If('obj')
        block.append('CORBA::Object_var objref;')
        block.append('CORBA::jni::Object::fromJObject(objref, env, obj);')
        block.append('out = %s::_narrow(objref);', self.typeName(node))
        block = block.Else()
        block.append('out = %s::_nil();', self.typeName(node))

    def generateChildDecls (self, body, node):
        for decl in node.declarations():
            code = DeclHelper().generate(decl)
            body.append(code)

    def generateMemberDecls (self, body, node):
        body.append('static jmethodID ctor_;')

    def generateMemberImpls (self, body, node):
        body.append('jmethodID %s::ctor_ = 0;', self.qualifiedName(node))

    def generateMethodImpls (self, body, node):
        # Generate wrappers for every method the stub supports.
        for method in MethodVisitor().getMethods(node):
            self._generateWrapper(body, node, method)
            body.append()

        # Narrow CORBA::Object pointers to the specific type; the caller must
        # assume ownership.
        stubClass = self.javaClass(node)
        ptrType = self.inType(node)
        jniName = jniFunctionName(stubClass + '._narrow_object_ref')
        code = body.Function('extern "C" JNIEXPORT jlong JNICALL %s (JNIEnv* env, jobject, jlong ref)', jniName)
        # Load all JNI helpers required for this stub; this function must be
        # called before making any native CORBA calls.
        code.append('%s::Load(env);', self.qualifiedName(node))
        code.append('CORBA::Object_ptr obj = reinterpret_cast<CORBA::Object_ptr>(ref);')
        code.append('%s ptr = %s::_narrow(obj);', ptrType, self.typeName(node))
        code.append('return reinterpret_cast<jlong>(ptr);')
        body.append()

        # Widen specific type to a CORBA::Object pointer; no new reference is
        # created.
        jniName = jniFunctionName(stubClass + '._get_object_ref')
        code = body.Function('extern "C" JNIEXPORT jlong JNICALL %s (JNIEnv *, jobject, jlong ref)', jniName)
        code.append('%s ptr = reinterpret_cast<%s>(ref);', ptrType, ptrType)
        code.append('CORBA::Object_ptr obj = ptr;');
        code.append('return reinterpret_cast<jlong>(obj);')
        body.append()

    def generateOnLoad (self, body, node):
        # Get single argument constructor
        body.append('ctor_ = env->GetMethodID(cls_, "<init>", "(J)V");')

    def _generateWrapper (self, body, node, method):
        stubName = self.javaClass(node)
        retType = method.returnType()
        rtype = jniType(retType)
        jniName = jniFunctionName(stubName + '.' + method.stubName())
        args = ['JNIEnv *env__', 'jclass', 'jlong jref__']
        for param in method.parameters():
            args.append(jniParameterType(param) + ' ' + param.identifier())
        argstr = ', '.join(args)
        func = body.Function('extern "C" JNIEXPORT %s JNICALL %s (%s)', rtype, jniName, argstr)
        pre = func.Code()
        body = func.Code()
        post = func.Code()

        # Preamble: convert Java type to C++
        args = []
        for index, param in enumerate(method.parameters()):
            paramType = param.paramType()
            if isPrimitiveType(paramType) and not param.is_out():
                # No special handling required.
                args.append(param.identifier())
                continue

            argname = param.identifier()
            localname = 'arg%d__' % (index,)
            if isString(paramType) and not param.is_out():
                pre.append('omnijni::StringWrapper %s(%s, env__);', localname, argname)
            elif isSequence(paramType) and not param.is_out():
                seqType = paramType.unalias().seqType()

                # For most primitive types, except char (which is 16-bit in Java and 8-bit in CORBA),
                # skip conversion and borrow the underlying buffer.
                borrowBuffer = isPrimitiveType(seqType) and seqType.unalias().kind() != idltype.tk_char
                if borrowBuffer:
                    wrapper = 'wrap%d__' % (index,)
                    primType = jniType(seqType)
                    elemType = primType[1:].title()
                    pre.append('omnijni::%sArrayWrapper %s(%s, env__);', elemType, wrapper, argname)
                    cppType = '::'.join(paramType.scopedName())
                    pre.append('%s %s(%s.size(), %s.size(), (%s*)%s.data(), 0);', cppType, localname, wrapper, wrapper, typeString(seqType), wrapper)
                else:
                    cppType = '::'.join(paramType.scopedName())
                    pre.append('%s %s;', cppType, localname)
                    pre.append('fromJObject(%s, env__, %s);', localname, argname)
            else:
                pre.append('%s %s;', temporaryType(param), localname);
                if param.is_in():
                    helper = helperName(paramType, param.is_out())
                    pre.append('%s::fromJObject(%s, env__, %s);', helper, localname, argname)
            
            # Set value inside of holder for out params
            if param.is_out():
                post.append('%s::setValue(env__, %s, %s);', holderName(paramType), argname, localname)

            args.append(localname)
        argstr = ', '.join(args)

        # Cast back to our reference type and make the call
        interface = self.typeName(node)
        body.append('%s_ptr ptr__ = reinterpret_cast<%s_ptr>(jref__);', interface, interface)

        objcall = 'ptr__->%s(%s);' % (method.name(), argstr)
        if rtype != 'void':
            body.append('%s retval__;', temporaryType(retType, True))
            objcall = 'retval__ = ' + objcall;

        tryscope = body.Try()
        tryscope.append(objcall)

        # Determine default value for return on exception; this avoids warnings
        # about uninitialized variables, but depends on automatic cleanup from
        # the scope-based wrappers (e.g., StringWrapper).
        if rtype == 'void':
            errRet = 'return;'
        elif isPrimitiveType(retType):
            errRet = 'return 0;'
        else:
            errRet = 'return NULL;'

        for excType in method.exceptions():
            excName = '::'.join(excType.scopedName())
            clause = tryscope.Catch('const '+excName+'& ex')
            clause.append('jthrowable jexc = (jthrowable)%s::toJObject(ex, env__);', helperName(excType))
            clause.append('env__->Throw(jexc);')
            clause.append(errRet)
        clause = tryscope.Catch('const CORBA::SystemException& ex')
        clause.append('CORBA::jni::SystemException::throwJava(ex, env__);')
        clause.append(errRet)

        if rtype != 'void':
            retType = retType.unalias()
            if isPrimitiveType(retType):
                func.append('return retval__;')
            elif isSequence(retType):
                seqType = retType.seqType()
                if isPrimitiveType(seqType):
                    func.append('return NULL; // TODO: primitive sequence')
                else:
                    func.append('return (jobjectArray)toJObject(retval__, env__);')
            else:
                helper = helperName(retType)
                func.append('return %s::toJObject(retval__, env__);', helper)

class DeclHelper (idlvisitor.AstVisitor):

    def generate (self, node):
        self.__code = cppcode.Code()
        node.accept(self)
        return self.__code

    def _generateDecl (self, helper, node):
        code = helper().generateDecl(node)
        self.__code.append(code)
        self.__code.append()

    def visitModule (self, node):
        for defn in node.definitions():
            defn.accept(self)

    def visitInterface (self, node):
        self._generateDecl(POAHelper, node)
        self._generateDecl(StubHelper, node)
        self._generateDecl(HolderHelper, node)

    def visitEnum (self, node):
        self._generateDecl(EnumHelper, node)

    def visitStruct (self, node):
        self._generateDecl(StructHelper, node)
        self._generateDecl(HolderHelper, node)

    def visitUnion (self, node):
        self._generateDecl(UnionHelper, node)

    def visitTypedef (self, node):
        aliasType = node.aliasType()
        if not isSequence(aliasType):
            return
        self._generateDecl(SequenceHelper, node.declarators()[0])
        self._generateDecl(HolderHelper, node)

    def visitException (self, node):
        self._generateDecl(ExceptionHelper, node)


class HeaderVisitor (idlvisitor.AstVisitor):

    def __init__ (self, name, **kwargs):
        self.__module = cppcode.Header(name)
        self.__body = self.__module.Code()
        self.__global = self.__module.Code()
        self.__options = kwargs

    def write (self, f):
        self.__module.write(f)

    def visitAST (self, node):
        for n in node.declarations():
            n.accept(self)

    def visitEnum (self, node):
        enumName = '::'.join(node.scopedName())
        helper = helperName(node)
        self.generateFromJObjectWrapper(self.__global, enumName+'&', helper)
        self.generateToJObjectWrapper(self.__global, enumName, helper)
        self.generateGetJClassWrapper(self.__global, enumName, helper)

    def visitInterface (self, node):
        for inherits in node.inherits():
            self.checkDependency(inherits)

        for t in node.declarations():
            t.accept(self)        
 
        interfaceName = '::'.join(node.scopedName())
        outName = interfaceName + '_out'
        ptrName = interfaceName + '_ptr'
        helper = helperName(node)
        self.generateFromJObjectWrapper(self.__global, outName, helper)
        self.generateToJObjectWrapper(self.__global, ptrName, helper)
        self.generateGetJClassWrapper(self.__global, interfaceName+'Ref', helper)

    def visitModule(self, node):
        if not node.mainFile():
            return

        self.__module.include('<jni.h>')
        self.__module.include('<omnijni/omnijni.h>')
        if not self.__options.get('standalone', False):
            # Include the omniidl-generated C++ header
            headerext = self.__options.get('incext', '.h')
            header = os.path.basename(node.file()).replace('.idl', headerext)
            if 'incprefix' in self.__options:
                # Prepend an additional include path that may not match the
                # module's namespace (e.g. CORBA module "Foo" can installed in
                # "/usr/include/b").
                header = os.path.join(self.__options['incprefix'], header)
            self.__module.include('<%s>' % header)

        ns = self.__body.Namespace(node.identifier())
        jni = ns.Namespace('jni')
        code = DeclHelper().generate(node)
        jni.append(code)
        self.__body.append()

        for n in node.definitions():
            n.accept(self)

    def visitStruct (self, node):
        structName = '::'.join(node.scopedName())
        helper = helperName(node)
        self.generateFromJObjectWrapper(self.__global, structName+'&', helper)
        self.generateToJObjectWrapper(self.__global, structName+'&', helper)
        self.generateGetJClassWrapper(self.__global, structName, helper)

    def visitUnion (self, node):
        # TODO: Can unions use the same global wrapper code as structs?
        self.visitStruct(node)

    def generateFromJObjectWrapper (self, code, name, helper):
        func = code.Function('inline void fromJObject (%s out, JNIEnv* env, jobject obj)', name)
        func.append('%s::fromJObject(out, env, obj);', helper)
        code.append()

    def generateToJObjectWrapper (self, code, name, helper):
        func = code.Function('inline jobject toJObject (const %s in, JNIEnv* env)', name)
        func.append('return %s::toJObject(in, env);', helper)
        code.append()

    def generateGetJClassWrapper (self, code, name, helper):
        code.append('template<>')
        func = code.Function('inline jclass getJClass<%s> (JNIEnv* env)', name)
        func.append('return %s::getJClass(env);', helper)
        code.append()

    def visitTypedef (self, node):
        aliasType = node.aliasType()
        if isSequence(aliasType):
            # Ensure that the item type's dependency is checked; because they
            # call a templatized function, the sequence conversion methods
            # implicitly depend on the item conversion methods
            seqType = aliasType.unalias().seqType()
            self.checkDependency(seqType)

            seqName = '::'.join(node.declarators()[0].scopedName())
            self.generateFromJObjectWrapper(self.__global, seqName+'&', 'omnijni')
            self.generateToJObjectWrapper(self.__global, seqName+'&', 'omnijni')

    def checkDependency (self, idlType):
        dep = getDependency(idlType)
        if dep:
            header = dependencyHeader(dep)
            self.__module.include(header)

class CppVisitor (idlvisitor.AstVisitor):

    def __init__ (self, **kwargs):
        self.__module = cppcode.Module()
        self.__options = kwargs

    def write (self, f):
        self.__module.write(f)

    def visitAST(self, node):
        for n in node.declarations():
            n.accept(self)

    def visitModule(self, node):
        if not node.mainFile():
            return

        header = os.path.basename(node.file()).replace('.idl', '.h')
        self.__module.include('"jni_%s"' % (header,))

        for n in node.definitions():
            n.accept(self)

    def _generateImpl (self, helper, node):
        code = helper().generateImpl(node)
        self.__module.append(code)
        self.__module.append()

    def visitEnum (self, node):
        self._generateImpl(EnumHelper, node)

    def visitException (self, node):
        self._generateImpl(ExceptionHelper, node)

    def visitStruct (self, node):
        for member in node.members():
            self.checkDependency(member.memberType())

        self._generateImpl(StructHelper, node)
        self._generateImpl(HolderHelper, node)

    def visitUnion (self, node):
        for case in node.cases():
            self.checkDependency(case.caseType())

        self._generateImpl(UnionHelper, node)

    def visitInterface (self, node):
        for inherits in node.inherits():
            self.checkDependency(inherits)

        for decl in node.declarations():
            decl.accept(self)

        self._generateImpl(POAHelper, node)
        self._generateImpl(StubHelper, node)
        self._generateImpl(HolderHelper, node)

        for call in node.all_callables():
            call.accept(self)

    def visitAttribute (self, node):
        self.checkDependency(node.attrType())

    def visitOperation (self, node):
        for param in node.parameters():
            self.checkDependency(param.paramType())
        for exc in node.raises():
            self.checkDependency(exc)
        self.checkDependency(node.returnType())

    def visitTypedef (self, node):
        aliasType = node.aliasType()
        if not isSequence(aliasType):
            return
        self.checkDependency(aliasType)
        self._generateImpl(HolderHelper, node)
    
    def checkDependency (self, idlType):
        dep = getDependency(idlType)
        if dep:
            header = dependencyHeader(dep)
            self.__module.include(header)


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

    fileName = os.path.basename(tree.file())

    headerFile = 'jni_' + fileName.replace('.idl', '.h')
    visitor = HeaderVisitor(headerFile, **options)
    tree.accept(visitor)
    visitor.write(cppcode.SourceFile(open(headerFile, 'w')))
    
    cppFile = 'jni_' + fileName.replace('.idl', '.cpp')
    visitor = CppVisitor(**options)
    tree.accept(visitor)
    visitor.write(cppcode.SourceFile(open(cppFile, 'w')))
