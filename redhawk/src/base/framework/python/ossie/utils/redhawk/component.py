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

import cStringIO, pydoc

from ossie.utils.model import ComponentBase, Resource, PropertySet, PortSupplier

class DomainComponent(ComponentBase):
    def __init__(self, profile, spd, scd, prf, instanceName, refid, impl, pid=0, devs=[]):
        # Remove waveform from the instance name for ComponentBase so that the
        # short name is visible as 'instanceName'
        baseName = instanceName.split('/')[-1]
        super(DomainComponent,self).__init__(spd, scd, prf, baseName, refid, impl, pid, devs)
        # Retain the "waveformId/instantiationId" in _instanceName for backward
        # compatibility
        self._instanceName = instanceName
        self._profile = profile
        self.ports = []

        try:
            self.name = str(spd.get_name())
            self._parseComponentXMLFiles()
            self._buildAPI()
            if self.ref != None:
                self.ports = self._populatePorts()
        except Exception, e:
            print "Component:__init__() ERROR - Failed to instantiate component " + str(self.name) + " with exception " + str(e)

    #####################################

    def api(self, showComponentName=True, showInterfaces=True, showProperties=True, externalPropInfo=None, destfile=None):
        '''
        Inspect interfaces and properties for the component
        '''
        localdef_dest = False
        if destfile == None:
            localdef_dest = True
            destfile = cStringIO.StringIO()

        className = self.__class__.__name__
        if showComponentName == True:
            print >>destfile, className+" [" + str(self.name) + "]:"
        if showInterfaces == True:
            PortSupplier.api(self, destfile=destfile)
        if showProperties == True and self._properties != None:
            PropertySet.api(self, externalPropInfo, destfile=destfile)

        if localdef_dest:
            pydoc.pager(destfile.getvalue())
            destfile.close()


class Component(DomainComponent, Resource):
    """
    This representation provides a proxy to a running component. The CF::Resource api can be accessed
    directly by accessing the members of the class
    
    A simplified access to properties is available through:
        Component.<property id> provides read/write access to component properties
    
    """
    def __init__(self, profile, spd, scd, prf, objref, instanceName, refid, impl=None, pid=0, devs=[]):
        Resource.__init__(self, objref)
        DomainComponent.__init__(self, profile, spd, scd, prf, instanceName, refid, impl, pid, devs)
