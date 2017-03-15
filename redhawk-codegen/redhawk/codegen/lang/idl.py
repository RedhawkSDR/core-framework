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
from ossie.utils.sca import importIDL
from ossie.utils.idllib import IDLLibrary

from redhawk.codegen.utils import strenum

CorbaTypes = strenum('octet','boolean','char','short','ushort','long','ulong',
                     'longlong','ulonglong','float','double','string','objref', 'utctime')

idlRepo = IDLLibrary()
idlRepo.addSearchPath(os.path.join(os.environ['OSSIEHOME'], 'share/idl'))

class IDLInterface(object):
    def __init__(self, repid):
        self.__repid = repid
        interface = self.__repid.split(':')[1]
        if '/' in interface:
            self.__namespace, self.__interface = interface.rsplit('/', 1)
        else:
            self.__namespace = ''
            self.__interface = interface
        self.__idl = None

    def repid(self):
        return self.__repid

    def namespace(self):
        return self.__namespace

    def interface(self):
        return self.__interface

    def idl(self):
        if not self.__idl:
            # NB: This may be a costly operation, as it can parse most of the
            #     IDL files known to REDHAWK if the source file is not obvious;
            #     it's not strictly necessary unless looking at the operations
            #     or attributes.
            self.__idl = idlRepo.getInterface(self.repid())
        return self.__idl

    def operations(self):
        return self.idl().operations

    def attributes(self):
        return self.idl().attributes

    def filename(self):
        return self.idl().filename

class IDLStruct(object):
    def __init__(self, repid):
        self.__repid = repid
        interface = self.__repid.split(':')[1]
        if '/' in interface:
            self.__namespace, self.__interface = interface.rsplit('/', 1)
        else:
            self.__namespace = ''
            self.__interface = interface
        self.__idl = None

    def repid(self):
        return self.__repid

    def namespace(self):
        return self.__namespace

    def idl(self):
        if not self.__idl:
            # NB: This may be a costly operation, as it can parse most of the
            #     IDL files known to REDHAWK if the source file is not obvious;
            #     it's not strictly necessary unless looking at the operations
            #     or attributes.
            self.__idl = idlRepo.getIdlStruct(self.repid())
        return self.__idl

    def members(self):
        return self.idl().members
