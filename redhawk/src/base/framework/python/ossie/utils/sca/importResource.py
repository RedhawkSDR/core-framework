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

import os, sys
import xml.dom.minidom
import base
# from errorMsg import *

availableTypes = ["boolean", "char", "double", "float", "short", "long",
                    "objref", "octet", "string", "ulong","ushort"]
availableKinds = ["allocation", "configure", "test", "execparam", "factoryparam"]
availableActions = ["eq", "ne", "gt", "lt", "ge", "le", "external"]
availableModes = ["readonly", "readwrite", "writeonly"]

def getResource(path,Rname,parent=None, idl_cache = None):
    
    _idl_cache = idl_cache
    scdPath = findFile(path,Rname,".scd.xml")                
    if scdPath == None:         
#         errorMsg(parent,"No scd file found for: " + Rname)
        print "No scd file found for: " + Rname
        return None
    
    spdPath = findFile(path,Rname,".spd.xml")
    prfPath = findFile(path,Rname,".prf.xml")
    
    #
    # Build the main component or device from the SCD file    
    #
    doc_scd = xml.dom.minidom.parse(scdPath)
    doc_scd.normalize()
    if len(doc_scd.getElementsByTagName('softwarecomponent')[0].getElementsByTagName('componenttype'))==0:
#         errorMsg(parent,"Invalid file: " + scdPath)
        print "Invalid file: " + scdPath
        return None

    component_type = doc_scd.getElementsByTagName('softwarecomponent')[0].getElementsByTagName('componenttype')[0].childNodes[0].data
    #Instantiate a new component of the appropriate type
    if component_type == u'resource':
        newComp = base.Component(name=Rname,type='resource', int_list = _idl_cache)
    elif component_type == u'executabledevice':
        newComp = base.Component(name=Rname,type='executabledevice', int_list = _idl_cache)
    elif component_type == u'loadabledevice':
        newComp = base.Component(name=Rname,type='loadabledevice', int_list = _idl_cache)
    elif component_type == u'device':
        newComp = base.Component(name=Rname,type='device', int_list = _idl_cache)
    else:
#         errorMsg(parent,"Can't identify resource type for: " + Rname)
        print "Can't identify resource type for: " + Rname
        return None

    # Get the Ports
    ports = doc_scd.getElementsByTagName('softwarecomponent')[0].getElementsByTagName('componentfeatures')[0].getElementsByTagName('ports')
    if len(ports) != 0:
        provides_ports = ports[0].getElementsByTagName('provides')
        uses_ports = ports[0].getElementsByTagName('uses')
        if len(provides_ports)!=0:
            for p in provides_ports:
                tmpName = p.getAttribute('providesname')
                tmpInt = getInterface(p.getAttribute('repid'),tmpName)
                if tmpInt == None:
                    return None
                tmpType = p.getElementsByTagName('porttype')[0].getAttribute('type')
                newPort = base.Port(tmpName,tmpInt,type='Provides',portType=tmpType)
                newComp.ports.append(newPort)
                
        if len(uses_ports) != 0:
            for p in uses_ports:
                tmpName = p.getAttribute('usesname')
                tmpInt = getInterface(p.getAttribute('repid'),tmpName)
                if tmpInt == None:
                    return None
                tmpType = p.getElementsByTagName('porttype')[0].getAttribute('type')
                newPort = base.Port(tmpName,tmpInt,type='Uses',portType=tmpType)
                newComp.ports.append(newPort)
    
    # Make sure that xml and code are not generated for this resource
    newComp.generate = False        
    
    # Store the name of the file without the suffix (.scd.xml)
    i = scdPath.rfind("/")
    if i != -1:
        newComp.xmlName = scdPath[i+1:-8]
    else:
        newComp.xmlName = scdPath[:-8]
    
    #
    # Import the properties from the PRF file
    #
    # If there are no properties, just return the component as is
    if prfPath == None:
        return newComp
    
    doc_prf = xml.dom.minidom.parse(prfPath)
    doc_prf.normalize()
    
    properties_tag = doc_prf.getElementsByTagName('properties')
    if len(properties_tag)==0:
#         errorMsg(parent,"Invalid file: " + prfPath)
        print "Invalid file: " + prfPath
        return None

    simple_properties = properties_tag[0].getElementsByTagName('simple')
    if len(simple_properties)!=0:
        for s in simple_properties:
            p = getSimpleProperty(s)
            if p == None:
                #errorMsg(parent,"Invalid file: " + prfPath)
                continue            
            newComp.properties.append(p)
    
    simple_sequence_properties = properties_tag[0].getElementsByTagName('simplesequence')
    if len(simple_sequence_properties)!=0:
        for s in simple_sequence_properties:
            p = getSimpleSequenceProperty(s)
            if p == None:
                #errorMsg(parent,"Invalid file: " + prfPath)
                continue            
            newComp.properties.append(p)
    
    return newComp

def getInterface(repid,name):
    try:
        repid = repid.strip('IDL:')
        repid = repid[:repid.rfind(':')]
        tmpNS = repid[:repid.find('/')]
        tmpName = repid[repid.find('/')+1:]
        newInt = base.Interface(tmpName,nameSpace=tmpNS)
        return newInt
        
    except:
#         errorMsg(parent,"Can't read the Interface information for port: " + name)
        print "Can't read the Interface information for port: " + name
        return None
    
    
def findFile(path,Rname,suf):
    tmpf = None
    if os.path.isfile(path + '/' + Rname +'Resource'+suf):
        tmpf = path + '/' + Rname +'Resource' + suf
    elif os.path.isfile(path + '/' + Rname + suf):
        tmpf = path + '/' + Rname + suf     
    else:
        tmpFiles = os.listdir(path)
        for f in tmpFiles:
            if len(f)>=8 and f[-8:] == suf:
                tmpf = path + '/' + f
                break
    return tmpf

def getSimpleProperty(s):   
    tmpName = s.getAttribute("name")
    tmpId = str(s.getAttribute("id"))
    tmpType = s.getAttribute("type")
    tmpMode = s.getAttribute("mode")
    tmpDes_source = s.getElementsByTagName("description")
    if len(tmpDes_source)!=0 and tmpDes_source[0].hasChildNodes():
        tmpDes = str(tmpDes_source[0].childNodes[0].data)
    else:
        tmpDes = ''
    
    if tmpMode not in availableModes:
        return None
    if tmpType not in availableTypes:
        return None
    
    newProp = base.SimpleProperty(tmpName,tmpMode,tmpType,description=tmpDes, id=tmpId)
    
    value_element = s.getElementsByTagName("value")
    if len(value_element)!=0 and value_element[0].hasChildNodes():
        newProp.value = newProp.defaultValue = str(value_element[0].childNodes[0].data)
    
    units_element = s.getElementsByTagName("units")
    if len(units_element)!=0 and units_element[0].hasChildNodes():
        newProp.units = str(units_element[0].childNodes[0].data)
        
    range_element = s.getElementsByTagName("range")
    if len(range_element)!=0 and range_element[0].hasChildNodes():
        newProp.range = (str(range_element[0].getAttribute('min')),str(range_element[0].getAttribute('max')))
        
    enum_element = s.getElementsByTagName("enum")
    if len(enum_element)!=0 and enum_element[0].hasChildNodes():
        newProp.enum = str(enum_element[0].getAttribute('label'))
        
    kind_element = s.getElementsByTagName("kind")
    vaking_element = s.getElementsByTagName("vakind")
    if len(kind_element)==0 and vaking_element[0].hasChildNodes():
        return None
    newProp.kind = str(kind_element[0].getAttribute('kindtype'))
    
    action_element = s.getElementsByTagName("action")
    if len(action_element)!=0 and action_element[0].hasChildNodes():
        newProp.action = str(action_element[0].getAttribute('type'))
        
    return newProp
    
def getSimpleSequenceProperty(s):   
    tmpName = s.getAttribute("name")
    tmpId = str(s.getAttribute("id"))
    tmpType = s.getAttribute("type")
    tmpMode = s.getAttribute("mode")
    description_tag = s.getElementsByTagName("description")
    if len(description_tag)!=0:
        tmpDes = str(description_tag[0].childNodes[0].data)
    else:
        tmpDes = ''
    
    if tmpMode not in availableModes:
        return None
    if tmpType not in availableTypes:
        return None
    
    newProp = base.SimpleSequenceProperty(tmpName,tmpMode,tmpType,description=tmpDes,values=[],id=tmpId)
    
    value_element = s.getElementsByTagName("value")
    if len(value_element)!=0:
        for val in value_element:
            newProp.values.append((str(val.childNodes[0].data), str(val.childNodes[0].data)))
    
    units_element = s.getElementsByTagName("units")
    if len(units_element)!=0:
        newProp.units = str(units_element[0].childNodes[0].data)
        
    range_element = s.getElementsByTagName("range")
    if len(range_element)!=0:
        newProp.range = (str(range_element[0].getAttribute('min')),str(range_element[0].getAttribute('max')))
        
    kind_element = s.getElementsByTagName("kind")
    if len(kind_element)==0:
        return None
    newProp.kind = str(kind_element[0].getAttribute('kindtype'))
    
    action_element = s.getElementsByTagName("action")
    if len(action_element)!=0:
        newProp.action = str(action_element[0].getAttribute('type'))
        
    return newProp
