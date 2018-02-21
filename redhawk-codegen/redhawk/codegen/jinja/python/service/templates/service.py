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

#!/usr/bin/env python
#
# AUTO-GENERATED
#
# Source: ${component.profile.spd}

import sys, signal, copy, os
import logging

from ossie.cf import CF, CF__POA #@UnusedImport
from ossie.service import start_service
from omniORB import CORBA, URI, PortableServer

#{% for module in component.imports %}
${python.importModule(module)}
#{% endfor %}

class ${className}(${component.baseclass}):

    def __init__(self, name="${className}", execparams={}):
        self.name = name
        self._baseLog = logging.getLogger(self.name)
        self._log = logging.getLogger(self.name)

    def terminateService(self):
        pass

#{% for function in component.operations %}
    def ${function.name}(self
#{%- for param in function.params %}
, ${param.name} 
#{%- endfor %}
):
        # TODO
        pass

#{% endfor %}
#{% for attribute in component.attributes %}
    def _get_${attribute.name}(self):
        # TODO
        pass
#{% if not attribute.readonly %}

    def _set_${attribute.name}(self, data):
        # TODO
        pass
#{% endif %}

#{% endfor %}

if __name__ == '__main__':
    start_service(${className}, thread_policy=PortableServer.SINGLE_THREAD_MODEL)
