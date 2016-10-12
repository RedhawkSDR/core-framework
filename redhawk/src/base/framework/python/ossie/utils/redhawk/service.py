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

from ossie.utils import model

class Service(model.Service):
    def __init__(self, profile, spd, scd, prf, serviceRef, instanceName, refid, impl):
        model.Service.__init__(self, serviceRef, profile, spd, scd, prf, instanceName, refid, impl)

class RogueService(model.RogueService):
    def __init__(self, serviceRef, instanceName ):
        model.RogueService.__init__(self, serviceRef, instanceName )
