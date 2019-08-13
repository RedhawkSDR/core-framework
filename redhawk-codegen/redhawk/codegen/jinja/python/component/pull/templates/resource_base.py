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
#% set className = component.baseclass.name
#% set superclass = component.superclasses[0].name
#% set artifactType = component.artifacttype
#!/usr/bin/env python
#
# AUTO-GENERATED CODE.  DO NOT MODIFY!
#
# Source: ${component.profile.spd}
#{% filter lines|unique(keep_blank_lines=true)|join('\n') %}
from ossie.cf import CF
from ossie.cf import CF__POA
from ossie.utils import uuid

#{% for parent in component.superclasses %}
from ${parent.package} import ${parent.name}
#{% endfor %}
from ossie.threadedcomponent import *
#{% if component.properties|test('simple') is sometimes(true) %}
from ossie.properties import simple_property
#{% endif %}
#{% if component.structdefs %}
from ossie.properties import simple_property
from ossie.properties import simpleseq_property
from ossie.properties import struct_property
#{% endif %}
#{% if component.properties|test('simplesequence') is sometimes(true) %}
from ossie.properties import simpleseq_property
#{% endif %}
#{% if component.properties|test('structsequence') is sometimes(true) %}
from ossie.properties import structseq_property
#{% endif %}

import Queue, copy, time, threading
#{% for portgen in component.portgenerators %}
#{%   if loop.first %}
from ossie.resource import usesport, providesport, PortCallError
#{%   endif %}
#{%   for statement in portgen.imports() %}
${statement}
#{%   endfor %}
#{% endfor %}
#{% endfilter %}
#{% block basefeiimports %}
#{% if ('FrontendTuner' in component.implements) or ('GPS' in component.implements) or ('NavData' in component.implements) or ('RFInfo' in component.implements) or ('RFSource' in component.implements) %}
import frontend, bulkio
from frontend import FRONTEND
#{% endif %}
#{% endblock %}
#{% block baseadditionalimports %}
#{# Allow additional child class imports #}
#{% endblock %}

#{% import "base/properties.py" as properties with context %}
#{% for prop in component.properties if prop is enumerated %}
#{%   if loop.first %}
class enums:
#{%   endif %}
    ${properties.enumvalues(prop)|indent(4)}

#{% endfor %}
class ${className}(${component.poaclass}, ${component.superclasses|join(', ', attribute='name')}, ThreadedComponent):
        # These values can be altered in the __init__ of your derived class

        PAUSE = 0.0125 # The amount of time to sleep if process return NOOP
        TIMEOUT = 5.0 # The amount of time to wait for the process thread to die when stop() is called
        DEFAULT_QUEUE_SIZE = 100 # The number of BulkIO packets that can be in the queue before pushPacket will block

#{% if component is device %}
        def __init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams):
            ${superclass}.__init__(self, devmgr, uuid, label, softwareProfile, compositeDevice, execparams)
#{%   if component is aggregatedevice %}
            AggregateDevice.__init__(self)
#{%   endif %}
#{% else %}
        def __init__(self, identifier, execparams):
            loggerName = (execparams['NAME_BINDING'].replace('/', '.')).rsplit("_", 1)[0]
            Component.__init__(self, identifier, execparams, loggerName=loggerName)
#{% endif %}
            ThreadedComponent.__init__(self)

#{% if 'FrontendTuner' in component.implements %}
            self.listeners={}
#{% endif %}
            # self.auto_start is deprecated and is only kept for API compatibility
            # with 1.7.X and 1.8.0 ${artifactType}s.  This variable may be removed
            # in future releases
            self.auto_start = False
            # Instantiate the default implementations for all ports on this ${artifactType}
#{% for port in component.ports %}
            self.${port.pyname} = ${port.constructor}
            self.${port.pyname}._portLog = self._baseLog.getChildLogger('${port.name}', 'ports')
#{% endfor %}
#{% if component.hasmultioutport %}
            self.addPropertyChangeListener('connectionTable',self.updated_connectionTable)
#{% endif %}
#{% for prop in component.properties if prop.inherited and prop.pyvalue %}
            self.${prop.pyname} = ${prop.pyvalue}
#{% endfor %}

        def start(self):
            ${superclass}.start(self)
            ThreadedComponent.startThread(self, pause=self.PAUSE)

        def stop(self):
            ${superclass}.stop(self)
            if not ThreadedComponent.stopThread(self, self.TIMEOUT):
                raise CF.Resource.StopError(CF.CF_NOTSET, "Processing thread did not die")

#{% if component.hasmultioutport %}
        def updated_connectionTable(self, id, oldval, newval):
#{% for port in component.ports if port.multiout %}
            self.${port.pyname}.updateConnectionFilter(newval)
#{% endfor %}

#{% endif %}
        def releaseObject(self):
            try:
                self.stop()
            except Exception:
                self._baseLog.exception("Error stopping")
            ${superclass}.releaseObject(self)

        ######################################################################
        # PORTS
        # 
        # DO NOT ADD NEW PORTS HERE.  You can add ports in your derived class, in the SCD xml file, 
        # or via the IDE.

#{% for portgen in component.portgenerators if portgen.hasImplementation() %}
        # '${portgen.namespace}/${portgen.interface}' port
        class ${portgen.templateClass()}(${portgen.poaClass()}):
            """This class is a port template for the ${portgen.className()} port and
            should not be instantiated nor modified.
            
            The expectation is that the specific port implementation will extend
            from this class instead of the base CORBA class ${portgen.poaClass()}.
            """
            pass

#{% endfor %}
#{% for port in component.ports %}
#{%   filter codealign %}
        ${port.pyname} = ${port.generator.direction}port(name="${port.name}",
                                                         repid="${port.repid}",
                                                         type_="${port.types[0]}"
#%-   if port.hasDescription
,
                                                         description="""${port.description}"""
#%   endif
)
#{%   endfilter %}

#{% endfor %}
        ######################################################################
        # PROPERTIES
        # 
        # DO NOT ADD NEW PROPERTIES HERE.  You can add properties in your derived class, in the PRF xml file
        # or by using the IDE.
#{% filter codealign %}
#{% for prop in component.properties %}
#{%   if prop is struct and not prop.builtin %}
        ${properties.structdef(prop)|indent(8)}

#{%   elif prop is structsequence and not prop.structdef.builtin %}
        ${properties.structdef(prop.structdef,False)|indent(8)}

#{%   endif %}
#{%   if not prop.inherited %}
        ${properties.create(prop)}
#{%   endif %}
#{% endfor %}
#{% endfilter %}
#{% for portgen in component.portgenerators if portgen is provides and portgen.hasImplementation() %}

#{%   if loop.first %}
'''provides port(s). Send logging to _portLog '''

#{%   endif %}
#{% include portgen.implementation() %}
#{% endfor %}
#{% for portgen in component.portgenerators if portgen is uses and portgen.hasImplementation() %}

#{%   if loop.first %}
'''uses port(s). Send logging to _portLog '''

#{%   endif %}
#{% include portgen.implementation() %}
#{% endfor %}

#{% block extensions %}
#{# Allow for child class extensions #}
#{% endblock %}
