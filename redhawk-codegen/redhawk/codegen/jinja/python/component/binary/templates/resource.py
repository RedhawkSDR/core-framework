#{#
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
#}
#% set className = component.userclass.name
#% set baseClass = component.baseclass.name
#% set artifactType = component.artifacttype
#!/usr/bin/env python
#
#% block license
#% endblock
#
# AUTO-GENERATED
#
# Source: ${component.profile.spd}
#{% if component is device %}
from ossie.device import start_device
#{% else %}
from ossie.resource import Resource, start_component
#{% endif %}
import logging
from ossie.utils import uuid

from ${baseClass} import *

class ${className}(${baseClass}):
#{% if component.description %}
    """${component.description()}"""
#{% else %}
    """<DESCRIPTION GOES HERE>"""
#{% endif %}
    def initialize(self):
        """
        This is called by the framework immediately after your ${artifactType} registers with the NameService.
        
        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()
        """
        ${baseClass}.initialize(self)
        # TODO add customization here.
        
#{% if component is device %}
    def updateUsageState(self):
        """
        This is called automatically after allocateCapacity or deallocateCapacity are called.
        Your implementation should determine the current state of the device:
           self._usageState = CF.Device.IDLE   # not in use
           self._usageState = CF.Device.ACTIVE # in use, with capacity remaining for allocation
           self._usageState = CF.Device.BUSY   # in use, with no capacity remaining for allocation
        """
        return NOOP

#{% endif %}

    def process(self):
        retval = self.checkInputs()
        if not retval and self.bufferingEnabled:
            return NOOP
        retval = self.run_binary()
        if not self.bufferingEnabled:
            self.data = []
        if retval != None:
            self.sendOutput(retval)
        return NORMAL

  
if __name__ == '__main__':
    logging.getLogger().setLevel(logging.WARN)
#{% if component is device %}
    logging.debug("Starting Device")
    start_device(${className})
#{% else %}
    logging.debug("Starting Component")
    start_component(${className})
#{% endif %}
