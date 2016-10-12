#!/usr/bin/env python
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

#
from ossie.cf import CF, CF__POA
from ossie.resource import Resource, start_component
from ossie.properties import simple_property, simpleseq_property


class CapacityUser_i(CF__POA.Resource, Resource):
    """SCA component for capacity allocation testing"""

    prop1 = simple_property(id_="DCE:b8f43ac8-26b5-40b3-9102-d127b84f9e4b",
                            type_="string",
                            action="external",
                            kinds=("configure",))

    prop2 = simpleseq_property(id_="DCE:10b3364d-f035-4639-8e7f-02ac4706f5c7",
                               type_="string",
                               action="external",
                               kinds=("configure",))

    def __init__(self, identifier, execparams):
        Resource.__init__(self, identifier, execparams)

if __name__ == '__main__':
    start_component(CapacityUser_i)
