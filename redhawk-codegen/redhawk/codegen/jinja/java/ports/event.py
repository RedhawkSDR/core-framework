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

from redhawk.codegen.lang import java

from generator import BuiltinJavaPort

class PropertyEventPortGenerator(BuiltinJavaPort):
    REPID = 'IDL:omg.org/CosEventChannelAdmin/EventChannel:1.0'
    NAME = 'propEvent'

    def __init__(self, port):
        BuiltinJavaPort.__init__(self, 'org.ossie.events.PropertyEventSupplier', port)

    @classmethod
    def match(cls, port):
        return (port.repid() == cls.REPID) and (port.name() == cls.NAME)

    def _ctorArgs(self, name):
        return (java.stringLiteral(name),)
