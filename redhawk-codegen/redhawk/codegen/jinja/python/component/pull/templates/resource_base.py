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
#{% if component.children|length != 0 %}
import ossie
from ossie import resource
import sys, os
#{% endif %}
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

#{% if component.children %}
#{%   for childname, childcomponent in component.children.items() %}
import ${childname}
#{%   endfor %}
#{% endif %}

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
#{% if component.children|length != 0 %}
            self.childDevices = []
#{% endif %}

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

#{% if component.children|length != 0 %}
        def addChild(self, name):
            device_object = None
            try:
                self._cmdLock.acquire()
                with ossie.device.envState():
                    if type(name) == str:
                        device_name = name
                    else:
                        device_name = name.__name__
                    parameters = []
                    parameters.append(CF.DataType('IDM_CHANNEL_IOR', _any.to_any(resource.__orb__.object_to_string(self._idm_publisher.channel))))
                    parameters.append(CF.DataType('DEVICE_LABEL', _any.to_any(device_name)))
                    parameters.append(CF.DataType('PROFILE_NAME', _any.to_any('none')))
                    parameters.append(CF.DataType('DEVICE_MGR_IOR', _any.to_any(resource.__orb__.object_to_string(self._devMgr.ref))))
                    parameters.append(CF.DataType('DEVICE_ID', _any.to_any('DCE:123')))

                    execparams = {}
                    for param in parameters:
                        if param.value.value() != None:
                            # SR:453 indicates that an InvalidParameters exception should be
                            # raised if the input parameter is not of a string type; however,
                            # version 2.2.2 of the SCA spec is less strict in its wording. For
                            # our part, as long as the value can be stringized, it is accepted,
                            # to allow component developers to use more specific types.
                            try:
                                execparams[str(param.id)] = str(param.value.value())
                            except:
                                raise CF.ExecutableDevice.InvalidParameters([param])

                    mod = __import__(device_name)
                    kclass = getattr(mod, device_name+'_i')
                    device_object = local_start_device(kclass, execparams)
                    self.childDevices.append(device_object)
            finally:
                self._cmdLock.release()
            return device_object
#{% endif %}

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

#{% if component.children|length != 0 %}
def local_start_device(deviceclass, execparams, interactive_callback=None, thread_policy=None,loggerName=None, skip_run=False):
    interactive = False

    ossie.device._checkForRequiredParameters(execparams)

    try:
        try:
            orb = ossie.resource.createOrb()
            ossie.resource.__orb__ = orb

            ## sets up backwards compat logging 
            ossie.resource.configureLogging(execparams, loggerName, orb)

            devicePOA = ossie.resource.getPOA(orb, thread_policy, "devicePOA")

            devMgr = ossie.device._getDevMgr(execparams, orb)

            parentdev_ref = ossie.device._getParentAggregateDevice(execparams, orb)

            # Configure logging (defaulting to INFO level).
            label = execparams.get("DEVICE_LABEL", "")
            id = execparams.get("DEVICE_ID", "")
            log_config_uri = execparams.get("LOGGING_CONFIG_URI", None)
            debug_level = execparams.get("DEBUG_LEVEL", None)
            if debug_level != None: debug_level = int(debug_level)
            dpath=execparams.get("DOM_PATH", "")
            category=loggerName
            try:
              if not category and label != "": category=label.rsplit("_", 1)[0]
            except:
                pass 
            ctx = ossie.logger.DeviceCtx( label, id, dpath )
            ossie.logger.Configure( log_config_uri, debug_level, ctx, category )

            component_Obj = deviceclass(devMgr, 
                                        execparams["DEVICE_ID"], 
                                        execparams["DEVICE_LABEL"], 
                                        execparams["PROFILE_NAME"],
                                        parentdev_ref,
                                        execparams)
            devicePOA.activate_object(component_Obj)

            idm_channel_ior=execparams.get("IDM_CHANNEL_IOR",None)
            registrar_ior=execparams.get("DEVICE_MGR_IOR",None)
            component_Obj.postConstruction( registrar_ior, idm_channel_ior )

            # set logging context for resource to supoprt CF::Logging
            component_Obj.saveLoggingContext( log_config_uri, debug_level, ctx )

            objectActivated = True
            obj = devicePOA.servant_to_id(component_Obj)
            component_Obj.initialize()
            return component_Obj

        except SystemExit:
            pass
        except KeyboardInterrupt:
            pass
        except:
            traceback.print_exc()
    finally:
        pass
#{% endif %}
