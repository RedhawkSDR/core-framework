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

from redhawk.codegen.model.softwarecomponent import ComponentTypes

def is_always(obj, value):
    return False not in (o == value for o in obj)

def is_sometimes(obj, value):
    return True in (o == value for o in obj)

def is_never(obj, value):
    return True not in (o == value for o in obj)

def is_resource(component):
    """
    Returns True if the component is a REDHAWK resource.
    """
    return component['type'] == ComponentTypes.RESOURCE

def is_device(component):
    """
    Returns True if the component is a REDHAWK device.
    """
    return component['type'] in (ComponentTypes.DEVICE, ComponentTypes.LOADABLEDEVICE, ComponentTypes.EXECUTABLEDEVICE)

def is_loadabledevice(component):
    """
    Returns True if the component is a REDHAWK loadable device.
    """
    return component['type'] in (ComponentTypes.LOADABLEDEVICE, ComponentTypes.EXECUTABLEDEVICE)

def is_executabledevice(component):
    """
    Returns True if the component is a REDHAWK executable device.
    """
    return component['type'] == ComponentTypes.EXECUTABLEDEVICE

def is_aggregatedevice(component):
    """
    Returns True if the component is a REDHAWK aggregate device.
    """
    return component.get('aggregate', False)

def is_programmabledevice(component):
    """
    Returns True if the component is a programmable device (host for persona)
    """
    return component.get('programmable', False)

def is_service(component):
    """
    Returns True if the component is a REDHAWK service.
    """
    return component['type'] == ComponentTypes.SERVICE

def is_simple(prop):
    """
    Returns True if the property is a simple property.
    """
    return prop['class'] == 'simple'

def is_simplesequence(prop):
    """
    Returns True if the property is a simplesequence property.
    """
    return prop['class'] == 'simplesequence'

def is_struct(prop):
    """
    Returns True if the property is a struct property.
    """
    return prop['class'] == 'struct'

def is_structsequence(prop):
    """
    Returns True if the property is a structsequence property.
    """
    return prop['class'] == 'structsequence'

def is_enumerated(prop):
    """
    Returns True if the property, or one of its nested properties, has
    enumerated values.
    """
    if is_simple(prop):
        return 'enums' in prop
    elif is_struct(prop):
        # If any field is enumerated, return True
        return any(is_enumerated(f) for f in prop['fields'])
    elif is_structsequence(prop):
        # Check the struct defintion
        return is_enumerated(prop['structdef'])
    else:
        # Simple sequence properties do not support enumerations
        return False

def _getvalue(obj, name):
    """
    Looks up the item or attribute 'name' in obj. If the item is callable,
    return the result of a no-argument call, otherwise return the item.
    """
    try:
        value = obj[name]
    except TypeError:
        value = getattr(obj, name)
    if callable(value):
        return value()
    else:
        return value

def is_provides(port):
    """
    Returns True if 'port' is a provides port.
    """
    direction = _getvalue(port, 'direction')
    return direction in ('provides','bidir')

def is_uses(port):
    """
    Returns True if 'port' is a uses port.
    """
    direction = _getvalue(port, 'direction')
    return direction in ('uses','bidir')

def is_bidir(port):
    """
    Returns True if 'port' is both a provides and uses port.
    """
    direction = _getvalue(port, 'direction')
    return direction == 'bidir'
