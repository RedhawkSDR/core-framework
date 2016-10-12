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

from omniORB import CORBA, PortableServer
from omniORB import URI, any
from ossie.cf import CF, CF__POA
import CosNaming
from xml.dom import minidom
import sys
import os

DeviceManagerFile = sys.argv[1]

devPro = file(DeviceManagerFile, "r")
devProContent = devPro.read()
devDCD = minidom.parseString(devProContent)
componentplacements = devDCD.getElementsByTagName('componentplacement')
devIds = []
for placement in componentplacements:
    instance = placement.getElementsByTagName('componentinstantiation')[0]
    instanceId = instance.getAttribute('id')
    devIds.append(str(instanceId))
domainmanager = devDCD.getElementsByTagName('domainmanager')[0]
namingservice = domainmanager.getElementsByTagName('namingservice')[0]
name = str(namingservice.getAttribute('name'))
domainname = name.split('/')[0]

if len(sys.argv) == 3:
    domainName = sys.argv[2]

orb = CORBA.ORB_init([''], CORBA.ORB_ID)
obj_poa = orb.resolve_initial_references("RootPOA")
poaManager = obj_poa._get_the_POAManager()
poaManager.activate()
ns_obj = orb.resolve_initial_references("NameService")
rootContext = ns_obj._narrow(CosNaming.NamingContext)

if rootContext is None:
    sys.exit()
    
domainMgrURI = URI.stringToName("%s/%s" % (domainName, domainName))
domManager = rootContext.resolve(domainMgrURI)

if domManager is None:
    sys.exit()
    
devManObjs = domManager._get_deviceManagers()

applications = domManager._get_applications()
dass = []
for app in applications:
    das = app._get_componentDevices()
    dass.append((das, app))

for das in dass:
    for entry in das[0]:
        if entry.assignedDeviceId in devIds:
#            try:
                print "Releasing an application"
                das[1].releaseObject()
            #except:
            #    print "some bad stuff happened while releasing the application"
            #    pass
