#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

from ossie.cf import CF
import ossie.utils.redhawk

class CannotResolve(Exception):
    pass

class NotSandboxObject(Exception):
    pass

class CannotResolveRef(Exception):
    pass

def makeEndPoint(obj, port_name='', rsc_id=None, force=False):
    '''
      obj: a sandbox or CORBA-accessible object
      port_name: the port name (where applicable). Use an empty string for base supported interface
      rsc_id: the unique domain id for the Domain object/resource. This string is optional (the function tries to figure out what it should be)
      force: return an endpoint (objref) if nothing else matches
    '''
    try:
        return makeEndPointFromPy(obj, port_name, rsc_id)
    except NotSandboxObject:
        pass
    except Exception as e:
        raise
    
    try:
        return makeEndPointFromRef(obj, port_name, rsc_id)
    except CannotResolveRef:
        pass
    except Exception as e:
        raise
    
    if not force:
        raise CannotResolve('Object '+str(obj)+' could not be resolved to a sandbox object or CORBA-accessible Domain object')
    
    restype = CF.ConnectionManager.EndpointResolutionType(objectRef='')
    return CF.ConnectionManager.EndpointRequest(restype, port_name)

def makeEndPointFromPy(obj, port_name='', rsc_id=None):
    
    if isinstance(obj, ossie.utils.redhawk.device.DomainDevice):
        _id = rsc_id
        if not _id:
            _id = obj._id
        restype = CF.ConnectionManager.EndpointResolutionType(deviceId=_id)
        return CF.ConnectionManager.EndpointRequest(restype, port_name)
    elif isinstance(obj, ossie.utils.redhawk.core.App):
        _id = rsc_id
        if not _id:
            _id = obj._id
        restype = CF.ConnectionManager.EndpointResolutionType(applicationId=_id)
        return CF.ConnectionManager.EndpointRequest(restype, port_name)
    elif isinstance(obj, ossie.utils.redhawk.component.Component):
        _id = rsc_id
        if not _id:
            _id = obj._id
        restype = CF.ConnectionManager.EndpointResolutionType(componentId=_id)
        return CF.ConnectionManager.EndpointRequest(restype, port_name)
    elif isinstance(obj, ossie.utils.redhawk.core.EventChannel):
        _id = rsc_id
        if not _id:
            _id = obj.name
        restype = CF.ConnectionManager.EndpointResolutionType(channelName=_id)
        return CF.ConnectionManager.EndpointRequest(restype, port_name)
    elif isinstance(obj, ossie.utils.redhawk.core.Service):
        _id = rsc_id
        if not _id:
            _id = obj.name
        restype = CF.ConnectionManager.EndpointResolutionType(serviceName=_id)
        return CF.ConnectionManager.EndpointRequest(restype, port_name)
    elif isinstance(obj, ossie.utils.redhawk.core.DeviceManager):
        _id = rsc_id
        if not _id:
            _id = obj._id
        restype = CF.ConnectionManager.EndpointResolutionType(deviceMgrId=_id)
        return CF.ConnectionManager.EndpointRequest(restype, port_name)
    raise NotSandboxObject('Object '+str(obj)+' is not a sandbox object')

def makeEndPointFromRef(obj, port_name='', rsc_id=None):
    _id = rsc_id
    try:
        if hasattr(obj, '_this'):
            repid = obj._this()._NP_RepositoryId
        else:
            repid = obj._NP_RepositoryId
    except:
        raise Exception('Object must have repository id')

    if not _id: # could just be object
        try:
            _id = obj._get_identifier()
        except:
            pass

    if repid == 'IDL:omni/omniEvents/EventChannel:1.0' and not _id:
        raise CannotResolveRef('Object '+str(obj)+' is an Event Channel. An Event Channel name must be provided (rsc_id)')
    
    if 'Device' in repid and _id != None: # executable, loadable, base, or any of the aggregate devices
        restype = CF.ConnectionManager.EndpointResolutionType(deviceId=_id)
        return CF.ConnectionManager.EndpointRequest(restype, port_name)
    elif repid == 'IDL:CF/Application:1.0' and _id != None:
        restype = CF.ConnectionManager.EndpointResolutionType(applicationId=_id)
        return CF.ConnectionManager.EndpointRequest(restype, port_name)
    elif repid == 'IDL:CF/Resource:1.0' and _id != None:
        restype = CF.ConnectionManager.EndpointResolutionType(componentId=_id)
        return CF.ConnectionManager.EndpointRequest(restype, port_name)
    elif repid == 'IDL:CF/DeviceManager:1.0' and name != None:
        restype = CF.ConnectionManager.EndpointResolutionType(channelName=name)
        return CF.ConnectionManager.EndpointRequest(restype, port_name)
    raise CannotResolveRef('Object '+str(obj)+' cannot be resolved to a Device, Application, Component, Event Channel, or Device Manager. A reference is insufficient to resolve the object further')
