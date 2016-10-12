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


from ossie.cf import CF, CF__POA
from ossie.service import start_service
from ossie.properties import *
import os, sys, stat
from omniORB import URI, any
import logging
import signal
import shutil
import threading

# A service can implement/provide any IDL, we implement 
# a rudamentary CF::PropertySet as a test
class BasicService(CF__POA.PropertySet):

    def __init__(self, name, params):
        self.name = name
        self.params = params
        # Convert to the types specified in
        self.params['PARAM2'] = int(self.params['PARAM2'])
        self.params['PARAM3'] = float(self.params['PARAM3'])
        self.params['PARAM4'] = (self.params['PARAM4'].lower() == "true")

    def configure(self, configProperties):
        for prop in configProperties:
            self.execparam[prop.id] = any.from_any(prop.value)

    def query(self, configProperties):
        # If the list is empty, get all props
        if configProperties == []:
            return [CF.DataType(id=i, value=any.to_any(v)) for i,v in self.params.items()]
        else:
            result = []
            for p in configProperties:
                result.append(CF.DataType(id=p.id, value=any.to_any(self.parms[p.id])))
            return result

if __name__ == "__main__":
    # THESE ARE FOR UNITTESTING, REGULAR SERVICES SHOULD NOT USE THEM
    if not "SERVICE_NAME" in sys.argv:
        raise StandardError, "Missing SERVICE_NAME"
    if not "DEVICE_MGR_IOR" in sys.argv:
        raise StandardError, "Missing DEVICE_MGR_IOR"

    # THIS IS THE ONLY CODE THAT A REGULAR SERVICE SHOULD HAVE IN IT'S MAIN
    from omniORB import PortableServer
    start_service(BasicService, thread_policy=PortableServer.SINGLE_THREAD_MODEL)
