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
from ossie import utils
from ossie.utils import sca

availableTypes = ["boolean", "char", "double", "float", "short", "long",
                    "objref", "octet", "string", "ulong","ushort"]
availableKinds = ["allocation", "property", "configure", "test", "execparam", "factoryparam"]
availableActions = ["eq", "ne", "gt", "lt", "ge", "le", "external"]
availableModes = ["readonly", "readwrite", "writeonly"]

def getNode(inpath,Nname,parent=None,sdrroot='/sdr/sca', in_interface_list=None):
    
    scdPath = inpath+"/DeviceManager.scd.xml"
    spdPath = inpath+"/DeviceManager.spd.xml"
    prfPath = inpath+"/DeviceManager.prf.xml"
    dcdPath = inpath+"/DeviceManager.dcd.xml"
    
    component_interfaces = in_interface_list
    
    newNode = sca.Node(Nname, inpath, generate=False)
    
    #
    # Build the description of the node from the DCD    
    #
    doc_dcd = xml.dom.minidom.parse(dcdPath)
    if len(doc_dcd.getElementsByTagName('deviceconfiguration')[0].getElementsByTagName('partitioning')) == 0:
#         utils.errorMsg(parent,"Invalid file: " + dcdPath)
        print "Invalid file: " + dcdPath
        return None
    
    newNode.id = doc_dcd.getElementsByTagName('deviceconfiguration')[0].getAttribute('id')
    
    for placement in doc_dcd.getElementsByTagName('deviceconfiguration')[0].getElementsByTagName('partitioning')[0].getElementsByTagName('componentplacement'):
        newComp = sca.Component(type='executabledevice', int_list=component_interfaces)
        if not component_interfaces:
            component_interfaces = newComp.interface_list
        comp_instance = placement.getElementsByTagName('componentinstantiation')[0]
        newComp.name = str(comp_instance.getElementsByTagName('usagename')[0].childNodes[0].data)
        # strip off the DCE: part of the id becuase it will get added back in later
        newComp.uuid = str(comp_instance.getAttribute('id')).strip("DCE:")
        newComp.file_uuid = str(placement.getElementsByTagName('componentfileref')[0].getAttribute('refid'))
        
        # Find the SPD file associated with the device
        local_SPD = ""
        for component_file in doc_dcd.getElementsByTagName('deviceconfiguration')[0].getElementsByTagName('componentfiles')[0].getElementsByTagName('componentfile'):
            if component_file.getAttribute('id') == newComp.file_uuid:
                local_SPD = component_file.getElementsByTagName('localfile')[0].getAttribute('name')
                break
        pathSPD = os.path.join(sdrroot, local_SPD)
        if not os.path.exists(pathSPD):
            print "Cannot locate SPD file: " + pathSPD
            continue
        doc_spd = xml.dom.minidom.parse(pathSPD)
        
        # Find the SCD associated with the device
        newComp.baseName = doc_spd.getElementsByTagName('softpkg')[0].getAttribute('name')
        local_SCD = doc_spd.getElementsByTagName('softpkg')[0].getElementsByTagName('descriptor')[0].getElementsByTagName('localfile')[0].getAttribute('name')
        pathSCD = os.path.join(sdrroot, local_SCD)
        if not os.path.exists(pathSCD):
            print "Cannot locate SCD file: " + pathSCD
            continue
        doc_scd = xml.dom.minidom.parse(pathSCD)
        
        # Get the Ports
        if len(doc_scd.getElementsByTagName('softwarecomponent')[0].getElementsByTagName('componentfeatures')[0].getElementsByTagName('ports'))!=0:
            ports = doc_scd.getElementsByTagName('softwarecomponent')[0].getElementsByTagName('componentfeatures')[0].getElementsByTagName('ports')[0]
            if len(ports.getElementsByTagName('provides')) != 0:
                for p in ports.getElementsByTagName('provides'):
                    tmpName = p.getAttribute('providesname')
                    tmpInt = getInterface(p.getAttribute('repid'),tmpName)
                    if tmpInt == None:
                        return None
                    tmpType = p.getElementsByTagName('porttype').getAttribute('type')
                    newPort = sca.Port(tmpName,tmpInt,type='Provides',portType=tmpType)
                    newComp.ports.append(newPort)
                
            if len(ports.getElementsByTagName('uses')) != 0:
                for p in ports.getElementsByTagName('uses'):
                    tmpName = p.getAttribute('usesname')
                    tmpInt = getInterface(p.getAttribute('repid'),tmpName)
                    if tmpInt == None:
                        return None
                    tmpType = p.getElementsByTagName('porttype').getAttribute('type')
                    newPort = sca.Port(tmpName,tmpInt,type='Uses',portType=tmpType)
                    newComp.ports.append(newPort)
                    
        # Make sure that xml and code are not generated for this resource
        newComp.generate = False        
        
        # Store the name of the file without the suffix (.scd.xml)
        i = pathSCD.rfind("/")
        if i != -1:
            newComp.xmlName = pathSCD[i+1:-8]
        else:
            newComp.xmlName = pathSCD[:-8]
        
        newNode.Devices.append(newComp)
    
        #
        # Import the properties from the PRF file
        #
        # If there are no properties, just return the component as is
        #if pathPRF == None:
        #    return newComp
        #

    return newNode
        
        

def getInterface(repid,name):
    try:
        repid = repid.strip('IDL:')
        repid = repid[:repid.rfind(':')]
        tmpNS = repid[:repid.find('/')]
        tmpName = repid[repid.find('/')+1:]
        newInt = sca.Interface(tmpName,nameSpace=tmpNS)
        return newInt
        
    except:
#         utils.errorMsg(parent,"Can't read the Interface information for port: " + name)
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
    if not hasattr(s,"name"):
        return None
    tmpName = s.name
    if not hasattr(s,"id"):
        return None
    tmpId = str(s.id)
    if not hasattr(s,"type"):
        return None
    tmpType = s.type    
    if not hasattr(s,"mode"):
        return None
    tmpMode = s.mode    
    if hasattr(s,"description"):
        tmpDes = str(s.description)
    else:
        tmpDes = ''
    
    if tmpMode not in availableModes:
        return None
    if tmpType not in availableTypes:
        return None
    
    newProp = sca.SimpleProperty(tmpName,tmpMode,tmpType,description=tmpDes,id=tmpId)
    
    if hasattr(s,"value"):
        newProp.value = newProp.defaultValue = str(s.value)
    
    if hasattr(s,"units"):
        newProp.units = str(s.units)
        
    if hasattr(s,"range"):
        newProp.range = (str(s.range.min),str(s.range.max))
        
    if hasattr(s,"enum"):
        newProp.enum = str(s.enum.label)
        
    if not hasattr(s, "kind"):
        return None    
    newProp.kind = str(s.kind.kindtype)
    
    if hasattr(s,"action"):
        newProp.action = str(s.action.type)
        
    return newProp
    
def getSimpleSequenceProperty(s):   
    if not hasattr(s,"name"):
        return None
    tmpName = s.name
    if not hasattr(s,"id"):
        return None
    tmpId = str(s.id)
    if not hasattr(s,"type"):
        return None
    tmpType = s.type    
    if not hasattr(s,"mode"):
        return None
    tmpMode = s.mode    
    if hasattr(s,"description"):
        tmpDes = str(s.description)
    else:
        tmpDes = ''
    
    if tmpMode not in availableModes:
        return None
    if tmpType not in availableTypes:
        return None
    
    newProp = sca.SimpleSequenceProperty(tmpName,tmpMode,tmpType,description=tmpDes,values=[],id=tmpId)
    
    if hasattr(s,"value"):
        for val in s.value:
            newProp.values.append((str(val), str(val)))
    
    if hasattr(s,"units"):
        newProp.units = str(s.units)
        
    if hasattr(s,"range"):
        newProp.range = (str(s.range.min),str(s.range.max))
        
    if not hasattr(s, "kind"):
        return None    
    newProp.kind = str(s.kind.kindtype)
    
    if hasattr(s,"action"):
        newProp.action = str(s.action.type)
        
    return newProp
