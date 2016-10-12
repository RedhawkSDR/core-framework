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
from xml.dom import minidom
import CosNaming
from ossie.cf import CF

XML_DIR = '/midas/sca/sdr'

def xmlFile (file, fs=None):
    if fs != None:
        try:
            _xmlFile = fs.open(str(file), True)
            buff = _xmlFile.read(_xmlFile.sizeOf())
            _xmlFile.close()
        except:
            buff = ''
    else:
        try:
            _xmlFile = open(str(file), 'r')
            buff = _xmlFile.read()
            _xmlFile.close()
        except:
            buff = ''
    return buff



def getPropertiesForApplication (profile, fs=None):
    lookup = {}
    try:
        profDom = minidom.parseString(xmlFile(profile, fs))
        for component in getComponentFiles(profile, fs):
            propfile = getPropertyFile(component, fs)
            lookup.update(parsePropertyFile(propfile, fs))
    except:
        pass
    return lookup


def getComponentFiles (xmlfile, fs):
    componentFiles = []
    try:
        fileDom = minidom.parseString(xmlFile(xmlfile, fs))
        for component in fileDom.getElementsByTagName('componentfile'):
            localfile = component.getElementsByTagName('localfile')[0]
            componentFiles.append(localfile.getAttribute('name'))
    except:
        pass
    return componentFiles


def getPropertyFile (xmlfile, fs=None):
    try:
        fileDom = minidom.parseString(xmlFile(xmlfile, fs))
        propfile = fileDom.getElementsByTagName('propertyfile')[0]
        localfile = propfile.getElementsByTagName('localfile')[0].getAttribute("name")
        if not localfile.startswith("/"):
            localfile = os.path.join(os.path.dirname(xmlfile), localfile)
            return localfile
    except:
        return None

def parsePropertyFile (xmlfile, fs):
    # Build a dictionary of uuid-to-name mappings for all of the properties.
    lookup = {}
    for prop in getPropertiesFromPRF(xmlfile, fs):
        lookup[prop['id']] = prop

    return lookup


def lookupPropertySet (rootContext, elementId):
    prpRef = [CosNaming.NameComponent(x, '') for x in elementId.split("/")]

    prpRsrcRef = rootContext.resolve(prpRef)
    if prpRsrcRef is None:
        raise StandardError("Unable to find rootContext for %s" % (elementId))

    prpRsrcHandle = prpRsrcRef._narrow(CF.Resource)
    if prpRsrcHandle is None:
        raise StandardError("Unable to get Resource reference for %s" % (elementId))

    prpSetHandle = prpRsrcRef._narrow(CF.PropertySet)
    if prpSetHandle is None:
        raise StandardError("Unable to get PropertySet reference for %s" % (elementId))

    return prpSetHandle


def getPropertiesFromPRF (xmlfile, fs):
    if not xmlfile:
        return []
    
    propDom = minidom.parseString(xmlFile(xmlfile, fs))

    properties = []
    propNode = propDom.getElementsByTagName('properties')[0]
    for node in propNode.childNodes:
        if node.nodeType != node.ELEMENT_NODE:
            continue
        kinds = []
        for kindNode in node.getElementsByTagName('kind'):
            kinds.append(str(kindNode.getAttribute('kindtype')))
        if len(kinds) == 0:
            kinds = ['configure']
        property = { 'id': str(node.getAttribute('id')),
                     'name': str(node.getAttribute('name')),
                     'mode': str(node.getAttribute('mode')),
                     'type': str(node.getAttribute('type')),
                     'elementType': node.tagName,
                     'kinds': kinds }

        if not property['name']:
            property['name'] = property['id']
        if not property['mode']:
            property['mode'] = 'readwrite'
        if node.tagName == 'simple':
            valueNode = node.getElementsByTagName('value')
            if valueNode:
                if valueNode[0].hasChildNodes():
                    property['value'] = str(valueNode[0].childNodes[0].data)
        elif node.tagName == 'simplesequence':
            valuesNode = node.getElementsByTagName('values')
            if valuesNode:
                values = valuesNode[0].getElementsByTagName('value')
                property['values'] = [str(v.childNodes[0].data) for v in values]
        elif node.tagName == 'struct':
            # TODO: struct values
            pass
        elif node.tagName == 'structsequence':
            # TODO: structsequence values
            pass
                
        properties.append(property)

    return properties


def getPropertiesFromComponentInstantiation(instantiationNode):
    propertiesNode = instantiationNode.getElementsByTagName('componentproperties')
    if len(propertiesNode) == 0:
        return []

    properties = []
    for node in propertiesNode[0].childNodes:
        # TODO: Add support for structref and structsequenceref
        if node.nodeName not in ['simpleref', 'simplesequenceref']:
            continue

        try:
            property = {}
            property['refid'] = node.getAttribute('refid')
            if node.nodeName == 'simpleref':
                property['value'] = str(node.getAttribute('value'))
            elif node.nodeName == 'simplesequenceref':
                valuesNode = node.getElementsByTagName('values')[0]
                values = valuesNode.getElementsByTagName('value')
                property['values'] = [str(v.childNodes[0].data) for v in values]

            properties.append(property)
        except:
            pass

    return properties
