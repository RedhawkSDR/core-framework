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

from redhawk.codegen.lang import cpp

from generator import CppPortGenerator

class PropertyEventPortGenerator(CppPortGenerator):
    REPID = 'IDL:omg.org/CosEventChannelAdmin/EventChannel:1.0'
    NAME = 'propEvent'

    @classmethod
    def match(cls, port):
        return (port.repid() == cls.REPID) and (port.name() == cls.NAME)

    def _ctorArgs(self, name):
        return [cpp.stringLiteral(name)]

    def className(self):
        return 'PropertyEventSupplier'

    def header(self):
        return '<ossie/PropertyInterface.h>'
