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

import ossie.parsers.prf

from redhawk.codegen.utils import strenum

Kinds = strenum('execparam', 'configure', 'allocation', 'factoryparam', 'test', 'event', 'message')
Modes = strenum('writeonly', 'readonly', 'readwrite')
Actions = strenum('external', 'eq', 'ge', 'gt', 'le', 'lt', 'ne')

class Property(object):

    def __init__(self, xml):
        self.xml = xml

    def kinds(self):
        kinds = self.kinds_()
        if len(kinds) > 0:
            return set(str(k.kindtype) for k in kinds)
        else:
            return set((Kinds.CONFIGURE,))

    def identifier(self):
        return self.xml.id_

    def hasName(self):
        return self.xml.name is not None

    def name(self):
        if self.xml.name:
            return self.xml.name
        else:
            return self.xml.id_

    def mode(self):
        if not self.xml.mode:
            return Modes.READWRITE
        else:
            return str(self.xml.mode)

    def hasDescription(self):
        return self.xml.description is not None

    def description(self):
        return self.xml.description

    def isConfigure(self):
        return Kinds.CONFIGURE in self.kinds()

    def isAllocation(self):
        return Kinds.ALLOCATION in self.kinds()

    def isFactoryParam(self):
        return Kinds.FACTORYPARAM in self.kinds()

    def isTest(self):
        return Kinds.TEST in self.kinds()

    def isEvent(self):
        return Kinds.EVENT in self.kinds()

    def isMessage(self):
        return Kinds.MESSAGE in self.kinds()

class _Simple(object):
    def kinds_(self):
        return self.xml.kind

    def isStruct(self):
        return False

    def type(self):
       return self.xml.type_

    def isComplex(self):
        if self.xml.complex.lower() == "true":
            return True
        else:
            return False

    def isOptional(self):
        if self.xml.optional.lower() == "true":
            return True
        else:
            return False

    def action(self):
        if not self.xml.action or not self.xml.action.type_:
            return Actions.EXTERNAL
        else:
            return str(self.xml.action.type_)

    def range(self):
        if self.xml.range_:
            return self.xml.range_.min, self.xml.range_.max
        else:
            return None

    def hasUnits(self):
        return self.xml.units is not None

    def units(self):
        if self.xml.units:
            return self.xml.units
        else:
            return ""

class _Struct(object):
    def isStruct(self):
        return True

    def kinds_(self):
        return self.xml.configurationkind    

    def hasUnits(self):
        return False

    def units(self):
        return ""

    def action(self):
        return Actions.EXTERNAL

class _Single(object):
    def isSequence(self):
        return False

class _Sequence(object):
    def isSequence(self):
        return True

class SimpleProperty(Property, _Simple, _Single):
    def hasValue(self):
        return self.xml.value is not None

    def value(self):
        return self.xml.value

    def hasEnumerations(self):
        return bool(self.xml.enumerations)

    def enumerations(self):
        if self.xml.enumerations:
            return [(e.label, e.value) for e in self.xml.enumerations.enumeration]
        else:
            return None

class SimpleSequenceProperty(Property, _Simple, _Sequence):
    def hasValue(self):
        return self.xml.values is not None

    def value(self):
        if self.hasValue():
            return self.xml.values.value
        return None

class StructProperty(Property, _Struct, _Single):
    def type(self):
        return 'struct'

    def fields(self):
        f = [SimpleProperty(s) for s in self.xml.simple]
        f += [SimpleSequenceProperty(s) for s in self.xml.simplesequence]
        return f

    def hasValue(self):
        for field in self.fields():
            if field.value is not None:
                return True
        return False

    def value(self):
        return dict((s.name(), s.value()) for s in self.fields())

class StructSequenceProperty(Property, _Struct, _Sequence):
    def type(self):
        return 'structsequence'

    def struct(self):
        return StructProperty(self.xml.struct)

    def hasValue(self):
        return len(self.xml.structvalue) > 0

    def value(self):
        structref = self.struct()
        base = structref.value()
        idmap = dict((s.identifier(), s.name()) for s in structref.fields())
        return [self.mapvalue_(base, idmap, v) for v in self.xml.structvalue]

    def mapvalue_(self, base, mapping, structval):
        value = base.copy()
        value.update((v.refid, v.value) for v in structval.simpleref)
        value.update((v.refid, v.values.value) for v in structval.simplesequenceref)
        return value

def parse(prfFile):
    prf = ossie.parsers.prf.parse(prfFile)
    props = [SimpleProperty(p) for p in prf.simple]
    props += [SimpleSequenceProperty(p) for p in prf.simplesequence]
    props += [StructProperty(p) for p in prf.struct]
    props += [StructSequenceProperty(p) for p in prf.structsequence]
    return props
