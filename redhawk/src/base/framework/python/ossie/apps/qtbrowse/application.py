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

from xml.dom import minidom

import properties


class ApplicationFactory:
    def __init__ (self, name, id, components, partitioning, assemblycontroller, fs):
        self.name = name
        self.id = id
        self.components = components
        self.partitioning = partitioning
        self.assemblycontroller = assemblycontroller
        self.fs = fs

    def getComponentById (self, id):
        for component in self.components:
            if component['id'] == id:
                return component
        return None

    def getComponentPlacementById (self, id):
        for placement in self.partitioning:
            if placement['id'] == id:
                return placement

    def getPropertyFile (self):
        placement = self.getComponentPlacementById(self.assemblycontroller)
        component = self.getComponentById(placement['refid'])
        return properties.getPropertyFile(component['localfile'], self.fs)

    def getProperties (self):
        proplist = properties.getPropertiesFromPRF(self.getPropertyFile(), self.fs)
        placement = self.getComponentPlacementById(self.assemblycontroller)
        for overrideProp in placement['properties']:
            for prop in proplist:
                if overrideProp['refid'] == prop['id']:
                    if prop['elementType'] == 'simple':
                        prop['value'] = overrideProp['value']
                    elif prop['elementType'] == 'simplesequence':
                        prop['values'] = overrideProp['values']
        return proplist


def getApplicationFactoryFromSAD(xmlfile, fs):
    appDom = minidom.parseString(properties.xmlFile(xmlfile, fs))
    assemblyNode = appDom.getElementsByTagName('softwareassembly')[0]
    name = assemblyNode.getAttribute('name')
    id = assemblyNode.getAttribute('id')

    # Get the component files.
    componentList = []
    for componentNode in assemblyNode.getElementsByTagName('componentfile'):
        component = { 'id': componentNode.getAttribute('id'),
                      'type': componentNode.getAttribute('type') }
        localfile = componentNode.getElementsByTagName('localfile')[0]
        component['localfile'] = localfile.getAttribute('name')
        componentList.append(component)

    # Read the partitioning section.
    partitioning = []
    partitionNode = assemblyNode.getElementsByTagName('partitioning')[0]
    for placementNode in partitionNode.getElementsByTagName('componentplacement'):
        placement = {}
        filerefNode = placementNode.getElementsByTagName('componentfileref')[0]
        placement['refid'] = filerefNode.getAttribute('refid')

        # TODO: Handle the rest of the 'componentinstantiation' nodes.
        instNode = placementNode.getElementsByTagName('componentinstantiation')[0]
        placement['id'] = instNode.getAttribute('id')
        partitioning.append(placement)

        # Parse the 'componentproperties' for this instantiation.
        placement['properties'] = properties.getPropertiesFromComponentInstantiation(instNode)

    # Find the assembly controller id.
    controllerNode = assemblyNode.getElementsByTagName('assemblycontroller')[0]
    assemblyNode = controllerNode.getElementsByTagName('componentinstantiationref')[0]
    assemblyController = assemblyNode.getAttribute('refid')
    return ApplicationFactory(name, id, componentList, partitioning, assemblyController, fs)
