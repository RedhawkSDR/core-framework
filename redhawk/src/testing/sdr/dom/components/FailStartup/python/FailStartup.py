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
#
# AUTO-GENERATED
#
# Source: FailStartup.spd.xml
from ossie.resource import start_component
import logging

from FailStartup_base import *

class FailStartup_i(FailStartup_base):
    def __init__(self, identifier, execparams):
        if execparams['FAIL_AT'] == 'constructor':
            raise StandardError
        FailStartup_base.__init__(self, identifier, execparams)

    def initializeProperties(self, initProperties):
        if self.FAIL_AT == "initializeProperties":
            raise CF.PropertySet.InvalidConfiguration('Failure requested', initProperties)
        FailStartup_base.initializeProperties(self, initProperties)
        
    def initialize(self):
        if self.FAIL_AT == "initialize":
            raise CF.LifeCycle.InitializeError(['Failure requested'])
        FailStartup_base.initialize(self)
        
    def start(self):
        if self.FAIL_AT == "start":
            raise CF.Resource.StartError()
        FailStartup_base.start(self)

    def configure(self, configProperties):
        if self.FAIL_AT == "configure":
            raise CF.PropertySet.InvalidConfiguration('Failure requested', configProperties)
        return FailStartup_base.configure(self, configProperties)

  
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.INFO)
    logging.debug("Starting Component")
    start_component(FailStartup_i)

