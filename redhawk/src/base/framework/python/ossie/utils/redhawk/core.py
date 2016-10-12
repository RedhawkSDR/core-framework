#! /usr/local/bin/python
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

import traceback
import atexit as _atexit
import os as _os
from ossie.cf import CF as _CF
from ossie.cf import CF__POA as _CF__POA
from ossie.cf import ExtendedCF as _ExtendedCF
from ossie.cf import StandardEvent
from omniORB import CORBA as _CORBA
import CosNaming as _CosNaming
import sys as _sys
import time as _time
import datetime as _datetime
import weakref
import threading
import logging
from ossie import parsers
from ossie import properties
from ossie.utils import idllib
from ossie.utils.model import _Port
from ossie.utils.model import _readProfile
from ossie.utils.model import _idllib
from ossie.utils.model import ConnectionManager as _ConnectionManager
from ossie.utils.model import *
from ossie.utils.notify import notification
from ossie.utils import weakobj

from channels import IDMListener, ODMListener
from component import Component
from device import Device, createDevice
from service import Service, RogueService
from model import DomainObjectList
from model import IteratorContainer
from ossie.utils.model import QueryableBase

# Limit exported symbols
__all__ = ('App', 'Component', 'Device', 'DeviceManager', 'Domain',
           'getCFType', 'getCurrentDateTimeString', 'getDEBUG',
           'getMemberType', 'setDEBUG', 'setTrackApps', 'getTrackApps')

_launchedApps = []

log = logging.getLogger(__name__)

def getCurrentDateTimeString():
    # return a string representing current day and time
    # format is DDD_HHMMSSmmm where DDD represents day of year (1-365) and mmm represents number of milliseconds
    dt = _datetime.datetime.now()
    time_str = str(dt)
    # If time is on an even second (no microseconds) then don't do split to avoid exception
    if len(time_str.split('.')) == 1:
        microseconds_str = "000000"
    else:
        time_str, microseconds_str = time_str.split('.')
    return _time.strftime("%j_%H%M%S",dt.timetuple()) + microseconds_str[:3]

def _cleanUpLaunchedApps():
    for app in _launchedApps[:]:
        try:
            app.releaseObject()
        except:
            log.warn('Unable to release application object...continuing')

_atexit.register(_cleanUpLaunchedApps)

class App(_CF__POA.Application, Resource):
    """This is the basic descriptor for a waveform (collection of inter-connected Components)
       
       App overview:
       
       A waveform is defined by an XML file (<waveform name>.sad.xml) that resides in a waveform
       directory, usually $SDRROOT/dom/waveforms. This XML file lists a series of
       components, a variety of default values for each of these components, and a set of connections
       between different components' input and output ports. Input ports are referred to as 'Provides'
       and output ports are referred to as 'Uses'.
       
       A waveform can follow any type of design, but may look something like this:
       
                                  _________ 
                                  |        |
       _________    _________   ->| Comp 3 |
       |        |   |        | /  |        |
       | Comp 1 |-->| Comp 2 |/   ----------
       |        |   |        |\   _________ 
       ----------   ---------- \  |        |
                                ->| Comp 4 |
                                  |        |
                                  ----------
       
    """
    def __init__(self, name="", domain=None, sad=None):
        # __initialized needs to be set first to prevent __setattr__ from entering an error state
        self.__initialized = False
        Resource.__init__(self, None)
        self.name = name
        self._instanceName = name
        self.__components = None
        self.ports = []
        self._portsUpdated = False
        self.ns_name = ''
        self._domain = domain
        self._sad = sad
        self._externalProps = self._getExternalProperties()
        self.adhocConnections = []
        self._connectioncount = 0
        self.assemblyController = None
        self._acRef = None
        self._id = None

        if self._domain == None:
            orb = _CORBA.ORB_init(_sys.argv, _CORBA.ORB_ID)
            obj = orb.resolve_initial_references("NameService")
            self.rootContext = obj._narrow(_CosNaming.NamingContext)
        else:
            self.rootContext = self._domain.rootContext

        # Mark that __init__ has completed; after this point, unknown attributes
        # are assumed to be properties
        self.__initialized = True

    ########################################
    # External Application API
    def _get_profile(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_profile()
            except:
                pass
        return retval
    
    def _get_name(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_name()
            except:
                pass
        return retval
    
    def _get_registeredComponents(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_registeredComponents()
            except:
                pass
        return retval
    
    def _get_componentNamingContexts(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_componentNamingContexts()
            except:
                pass
        return retval
    
    def _get_componentProcessIds(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_componentProcessIds()
            except:
                pass
        return retval
    
    def _get_componentDevices(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_componentDevices()
            except:
                pass
        return retval

    def _get_componentImplementations(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_componentImplementations()
            except:
                pass
        return retval
    
    def releaseObject(self):
        if self.ref:
            try:
                self._domain.removeApplication(self)
            except:
                raise
    
    # End external Application API
    ########################################

    @property
    def comps(self):
        if self.__components is None:
            self._populateComponents()
        return self.__components

    def __getattribute__(self, name):
        try:
            if name in ('ports', '_usesPortDict', '_providesPortDict'):
                if not object.__getattribute__(self,'_portsUpdated'):
                    self._populatePorts()
            if name == '_id':
                if object.__getattribute__(self,'_id') == None:
                    tmp_id = object.__getattribute__(self, 'ref')._get_identifier()
                    self.__setattr__('_id', tmp_id)

            return object.__getattribute__(self,name)
        except AttributeError:
            # Check if current request is an external prop
            if self._externalProps.has_key(name):
                propId, compRefId = self._externalProps[name]
                for curr_comp in self.comps:
                    if curr_comp._get_identifier().find(compRefId) != -1:
                        try:
                            return getattr(curr_comp, propId)
                        except AttributeError:
                            continue
            else:
                # If it's not external, check assembly controller
                try:
                    # Force the components to get updated
                    self.comps
                    return getattr(self._acRef, name)
                except AttributeError:
                    pass
            raise AttributeError('App object has no attribute ' + str(name))
    
    def __setattr__(self, name, value):
        # Short-circuit private attributes, if initialization is not complete,
        # or it's an existing attribute
        if name.startswith('_App__') or not self.__initialized or name in self.__dict__:
            return object.__setattr__(self, name, value)

        # Check if current value to be set is an external prop
        if self._externalProps.has_key(name):
            propId, compRefId = self._externalProps[name]
            for curr_comp in self.comps:
                if curr_comp._get_identifier().find(compRefId) != -1:
                    try:
                        return setattr(curr_comp, propId, value)
                    except AttributeError:
                        continue
        else:
            # If it's not external, check assembly controller
            try:
                # Force the components to get updated
                self.comps
                return setattr(self._acRef, name, value)
            except AttributeError:
                pass
           
        return object.__setattr__(self, name, value)
    
    def update(self):
        self._populateComponents()

    def _getExternalProperties(self):
        # Return a map with all external properties
        extProps = {}
        if self._sad.get_externalproperties():
            for prop in self._sad.get_externalproperties().get_property():
                propId = prop.get_externalpropid()
                if not propId:
                    propId = prop.get_propid()
                extProps[propId] = (prop.get_propid(), prop.get_comprefid())
        return extProps

    def _populateComponents(self):
        component_list = self.ref._get_registeredComponents()

        # Set __components to any empty list (it starts as None), to mark that
        # this function has been run
        self.__components = []

        spd_list = {}
        for compfile in self._sad.get_componentfiles().get_componentfile():
            spd_list[str(compfile.get_id())] = str(compfile.get_localfile().get_name())

        if self._sad.get_assemblycontroller():
            self.assemblyController = self._sad.get_assemblycontroller().get_componentinstantiationref().get_refid()

        try:
            impls = dict((c.componentId, c.elementId) for c in self.ref._get_componentImplementations())
        except:
            impls = {}

        componentPids = self.ref._get_componentProcessIds()
        componentDevs = self.ref._get_componentDevices()
        for comp_entry in component_list:
            profile = comp_entry.softwareProfile
            compRef = comp_entry.componentObject
            try:
                refid = compRef._get_identifier()
                implId = impls.get(refid, None)
                instanceName = '%s/%s' % (self.ns_name, refid.split(':')[0])
                pid = 0
                devs = []
                for compPid in componentPids:
                    if compPid.componentId == refid:
                        pid = compPid.processId
                        break
                for compDev in componentDevs:
                    if compDev.componentId == refid:
                        devs.append(compDev.assignedDeviceId)
            except:
                refid = None
                implId = None
                instanceName = None
                pid = 0
                devs = []
            spd, scd, prf = _readProfile(profile, self._domain.fileManager)
            new_comp = Component(profile, spd, scd, prf, compRef, instanceName, refid, implId, pid, devs)

            self.__components.append(new_comp)

            if refid.find(self.assemblyController) >= 0:
                self._acRef = new_comp

    def api(self):
        # Display components, their properties, and external ports
        print "Waveform [" + self.ns_name + "]"
        print "---------------------------------------------------"

        print "External Ports =============="
        PortSupplier.api(self)

        print "Components =============="
        for count, comp_entry in enumerate(self.comps):
            name = comp_entry.name
            if comp_entry._get_identifier().find(self.assemblyController) != -1:
                name += " (Assembly Controller)"
            print "%d. %s" % (count+1, name)
        print "\n"

        # Display AC props
        if self._acRef:
            self._acRef.api(showComponentName=False, showInterfaces=False, showProperties=True)

        # Loops through each external prop looking for a component to use to display the internal prop value
        for extId in self._externalProps.keys():
            propId, compRefId = self._externalProps[extId]
            for comp_entry in self.comps:
                if comp_entry._get_identifier().find(compRefId) != -1:
                    # Pass along external prop info to component api()
                    comp_entry.api(showComponentName=False,showInterfaces=False,showProperties=True, externalPropInfo=(extId, propId))
                    break

        print

    def _populatePorts(self, fs=None):
        """Add all port descriptions to the component instance"""
        object.__setattr__(self, '_portsUpdated', True)

        sad = object.__getattribute__(self,'_sad')
        if not sad:
            print "Unable to create port list for " + object.__getattribute__(self,'name') + " - sad file unavailable"
            return

        ports = object.__getattribute__(self,'ports')
        if len(ports) > 0:
            return
    
        if fs==None:
            fs = object.__getattribute__(self,'_domain').fileManager
    
        interface_modules = ['BULKIO', 'BULKIO__POA']
        
        if not sad.get_externalports():
            # Nothing to do
            return
        
        for externalport in sad.get_externalports().get_port():
            portName = None
            instanceId = None
            if externalport.get_usesidentifier():
                portName = externalport.get_usesidentifier()
            elif externalport.get_providesidentifier():
                portName = externalport.get_providesidentifier()
            else:
                # Invalid XML; this should never occur.
                continue
            instanceId = externalport.get_componentinstantiationref().get_refid()
            placements = sad.get_partitioning().get_componentplacement()
            componentfileref = None
            for placement in placements:
                if componentfileref:
                    break
                for inst in placement.get_componentinstantiation():
                    if inst.get_id() == instanceId:
                        componentfileref = placement.get_componentfileref().get_refid()
                        break
            if not componentfileref: # maybe it's in the host collocation
                collocations = sad.get_partitioning().get_hostcollocation()
                for collocation in collocations:
                    if componentfileref:
                        break
                    placements = collocation.get_componentplacement()
                    for placement in placements:
                        if componentfileref:
                            break
                        for inst in placement.get_componentinstantiation():
                            if inst.get_id() == instanceId:
                                componentfileref = placement.get_componentfileref().get_refid()
                                break
            if not componentfileref:
                continue
            spd_file = None
            for componentfile in sad.get_componentfiles().get_componentfile():
                if componentfile.get_id() == componentfileref:
                    spd_file = componentfile.get_localfile().get_name()
                    break
            if not spd_file:
                continue
            
            # Read and parse the SPD file.
            spdFile = fs.open(spd_file, True)
            spdContents = spdFile.read(spdFile.sizeOf())
            spdFile.close()
            doc_spd = parsers.spd.parseString(spdContents)
            
            # Get the SCD file from the SPD file.
            if not doc_spd.get_descriptor():
                continue
            basedir = _os.path.dirname(spd_file)
            scdpath = _os.path.join(basedir, doc_spd.get_descriptor().get_localfile().get_name())

            # Read and parse the SCD file.
            scdFile = fs.open(scdpath, True)
            scdContents = scdFile.read(scdFile.sizeOf())
            scdFile.close()
            doc_scd = parsers.scd.parseString(scdContents)
            
            scd_ports = doc_scd.get_componentfeatures().get_ports()
            for uses in scd_ports.get_uses():
                usesName = str(uses.get_usesname())
                if usesName != portName:
                    continue
                idl_repid = str(uses.get_repid())

                #Checks if this external port was renamed
                if externalport.get_externalname():
                    usesName = externalport.get_externalname()

                # Add the port to the uses port list
                self._usesPortDict[usesName] = {'Port Name': usesName, 'Port Interface':idl_repid}

                try:
                    int_entry = _idllib.getInterface(idl_repid)
                except idllib.IDLError:
                    print "Invalid port descriptor in scd for " + self.name + " for " + idl_repid
                    continue
                new_port = _Port(usesName, interface=None, direction="Uses", using=int_entry)
                new_port.generic_ref = self.ref.getPort(str(new_port._name))
                new_port.ref = new_port.generic_ref._narrow(_ExtendedCF.QueryablePort)
                if new_port.ref == None:
                    new_port.ref = new_port.generic_ref._narrow(_CF.Port)
                new_port.extendPort()
    
                idl_repid = new_port.ref._NP_RepositoryId
                try:
                    int_entry = _idllib.getInterface(idl_repid)
                except idllib.IDLError:
                    print "Unable to find port description for " + self.name + " for " + idl_repid
                    continue
                new_port._interface = int_entry
    
                ports.append(new_port)
            
            for provides in scd_ports.get_provides():
                providesName = str(provides.get_providesname())
                if providesName != portName:
                    continue
                idl_repid = str(provides.get_repid())

                #checks if this external port was renamed
                if externalport.get_externalname():
                    providesName = externalport.get_externalname()

                # Add the port to the provides port list
                self._providesPortDict[providesName] = {'Port Name': providesName, 'Port Interface':idl_repid}

                try:
                    int_entry = _idllib.getInterface(idl_repid)
                except idllib.IDLError:
                    print "Invalid port descriptor in scd for " + self.name + " for " + idl_repid
                    continue
                new_port = _Port(providesName, interface=int_entry, direction="Provides")
                new_port.generic_ref = self.ref.getPort(str(new_port._name))

                # See if interface python module has been loaded, if not then try to import it
                if str(int_entry.nameSpace) not in interface_modules:
                    success = False
                try:
                    pkg_name = (int_entry.nameSpace.lower())+'.'+(int_entry.nameSpace.lower())+'Interfaces'
                    _to = str(int_entry.nameSpace)
                    mod = __import__(pkg_name,globals(),locals(),[_to])
                    globals()[_to] = mod.__dict__[_to]
                    success = True
                except ImportError, msg:
                    pass
                if not success:
                    std_idl_path = _os.path.join(_os.environ.get('OSSIEHOME', ''), 'lib/python')
                    for dirpath, dirs, files in _os.walk(std_idl_path):
                        if len(dirs) == 0:
                            continue
                        for directory in dirs:
                            try:
                                _from = directory+'.'+(int_entry.nameSpace.lower())+'Interfaces'
                                _to = str(int_entry.nameSpace)
                                mod = __import__(_from,globals(),locals(),[_to])
                                globals()[_to] = mod.__dict__[_to]
                                success = True
                            except:
                                continue
                            break
                        if success:
                            break
                if not success:
                    continue
        
                interface_modules.append(str(int_entry.nameSpace))
                
                exec_string = 'new_port.ref = new_port.generic_ref._narrow('+int_entry.nameSpace+'.'+int_entry.name+')'
        
                try:
                    exec(exec_string)
                    ports.append(new_port)
                except:
                    continue
    
    def connect(self, provides, usesPortName=None, providesPortName=None, usesName=None, providesName=None):
        """usesName and providesName are deprecated. Use usesPortName or providesPortName
        """
        uses_name = usesName
        provides_name = providesName
        if usesPortName != None:
            uses_name = usesPortName
        if providesPortName != None:
            provides_name = providesPortName
        if providesPortName != None and providesName != None:
            log.warn('providesPortName and providesName provided; using only providesName')
        if usesPortName != None and usesName != None:
            log.warn('usesPortName and usesName provided; using only usesPortName')
        connections = _ConnectionManager.instance().getConnections()
        name_mangling = '%s/%s' % (self._instanceName, uses_name)
        while True:
            connectionId = 'adhoc_connection_%d' % (self._connectioncount)
            if not name_mangling+connectionId in connections:
                break
            self._connectioncount = self._connectioncount+1
        PortSupplier.connect(self, provides, usesPortName=uses_name, providesPortName=provides_name, connectionId=connectionId)
    
    def __getitem__(self,i):
        """Return the component with the given index (obsolete)"""
        return self.comps[i]


class DeviceManager(_CF__POA.DeviceManager, QueryableBase, PropertyEmitter, PortSet):
    """The DeviceManager is a descriptor for an logical grouping of devices.

       Relevant member data:
       name - Node's name

    """
    @notification
    def deviceRegistered(self, device):
        """
        A new device registered with this device manager.
        """
        pass

    @notification
    def deviceUnregistered(self, identifier):
        """
        A device with the given identifier unregistered from this device manager.
        """
        pass

    @notification
    def serviceRegistered(self, service):
        """
        A new service registered with this device manager.
        """
        pass

    @notification
    def serviceUnregistered(self, identifier):
        """
        A service with the given identifier unregistered from this device manager.
        """
        pass

    def __init__(self, name="", devMgr=None, dcd=None, domain=None, idmListener=None, odmListener=None):
        self.name = name
        self.ref = devMgr
        self.id = self.ref._get_identifier()
        self._domain = domain
        self._dcd = dcd
        self.fs = self.ref._get_fileSys()
        try:
            spd, scd, prf = _readProfile("/mgr/DeviceManager.spd.xml", self.fs)
            super(DeviceManager, self).__init__(prf, self.id)
        except:
            pass
        self._buildAPI()
        
        self.__odmListener = odmListener
        self.__idmListener = idmListener

        self.__devices = DomainObjectList(weakobj.boundmethod(self._get_registeredDevices),
                                          weakobj.boundmethod(self.__newDevice),
                                          lambda x: x._get_identifier())
        self.__services = DomainObjectList(weakobj.boundmethod(self._get_registeredServices),
                                           weakobj.boundmethod(self.__newService),
                                           lambda x: x.serviceName)

        # Connect notification points to device lists.
        self.__devices.itemAdded.addListener(weakobj.boundmethod(self.deviceRegistered))
        self.__devices.itemRemoved.addListener(weakobj.boundmethod(self.deviceUnregistered))
        self.__services.itemAdded.addListener(weakobj.boundmethod(self.serviceRegistered))
        self.__services.itemRemoved.addListener(weakobj.boundmethod(self.serviceUnregistered))

        if self._domain == None:
            orb = _CORBA.ORB_init(_sys.argv, _CORBA.ORB_ID)
            obj = orb.resolve_initial_references("NameService")
            self.rootContext = obj._narrow(_CosNaming.NamingContext)
        else:
            self.rootContext = self._domain.rootContext

        # If connected to the ODM channel, listen for device added and removed
        # events. With the current device manager implemenatation, it's unlikely
        # that a new device will register after startup, but we handle it anyway.
        # Unregistration, on the other hand, may happen at any time.
        if self.__odmListener:
            weakobj.addListener(self.__odmListener.deviceAdded, self.__deviceAddedEvent)
            weakobj.addListener(self.__odmListener.deviceRemoved, self.__deviceRemovedEvent)
            weakobj.addListener(self.__odmListener.serviceAdded, self.__serviceAddedEvent)
            weakobj.addListener(self.__odmListener.serviceRemoved, self.__serviceRemovedEvent)

    ########################################
    # Begin external Device Manager API
    
    def _get_deviceConfigurationProfile(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_deviceConfigurationProfile()
            except:
                pass
        return retval
    
    def _get_fileSys(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_fileSys()
            except:
                pass
        return retval
    
    def _get_identifier(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_identifier()
            except:
                pass
        return retval
    
    def _get_label(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_label()
            except:
                pass
        return retval
    
    def _get_registeredDevices(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_registeredDevices()
            except:
                pass
        return retval
    
    def _get_registeredServices(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_registeredServices()
            except:
                pass
        return retval
    
    def registerDevice(self, registeringDevice):
        if self.ref:
            try:
                self.ref.registerDevice(registeringDevice)
            except:
                raise
    
    def unregisterDevice(self, registeredDevice):
        if self.ref:
            try:
                self.ref.unregisterDevice(registeredDevice)
            except:
                raise
    
    def shutdown(self):
        if self.ref:
            try:
                self.ref.shutdown()
            except:
                raise
    
    def registerService(self, registeringService):
        if self.ref:
            try:
                self.ref.registerService(registeringService)
            except:
                raise
    
    def unregisterService(self, registeredService):
        if self.ref:
            try:
                self.ref.unregisterService(registeredService)
            except:
                raise
    
    def getComponentImplementationId(self, componentInstantiationId):
        retval = None
        if self.ref:
            try:
                retval = self.ref.getComponentImplementationId(componentInstantiationId)
            except:
                raise
        return retval
    
    
    def configure(self, props):
        if self.ref:
            try:
                self.ref.configure(props)
            except:
                raise
    
    def query(self, props):
        retval = None
        if self.ref:
            try:
                retval = self.ref.query(props)
            except:
                raise
        return retval

    @property
    def devs(self):
        """
        The devices currently registered with this device manager.
        """
        if not self.__odmListener:
            self.__devices.sync()
        return self.__devices.values()

    @property
    def services(self):
        """
        The services currently registered with this device manager.
        """
        if not self.__odmListener:
            self.__services.sync()
        return self.__services.values()

    # End external Device Manager API
    ########################################
        
    def __newDevice(self, device):
        try:
            profile = device._get_softwareProfile()
            deviceRef = device._narrow(_CF.Device)
            instanceName = '%s/%s' % (self.name, device._get_label())
            refid = device._get_identifier()
            implId = self.ref.getComponentImplementationId(refid)
            spd, scd, prf = _readProfile(profile, self.fs)
            return createDevice(profile, spd, scd, prf, deviceRef, instanceName, refid, implId, self.__idmListener)
        except _CORBA.Exception:
            log.warn('Ignoring inaccessible device')

    def __deviceAddedEvent(self, event):
        if not self.ref:
            return
        # The device added event does not include the device manager to which
        # the new device belongs, so check it against the CORBA object's list
        # of current devices.
        # NB: The way the current device manager implementation works, this is
        #     an unlikely occurrance, anyway.
        try:
            devices = self.ref._get_registeredDevices()
        except:
            # The device manager is not reachable.
            return
        for device in devices:
            try:
                deviceId = device._get_identifier()
            except:
                # Skip unreachable device.
                continue
            if deviceId == event.sourceId:
                self.__devices.add(event.sourceId, event.sourceIOR)
                break

    def __deviceRemovedEvent(self, event):
        # If the complete device set is not cached, there is no way of checking
        # whether the device was registered with this particular device manager
        # (it wouldn't be in the 'registeredDevices' attribute anymore), so try
        # to remove the device and ignore the error.
        try:
            self.__devices.remove(event.sourceId)
        except KeyError:
            pass

    def __newService(self, service):
        try:
            refid=''
            profile=None
            serviceRef = service.serviceObject
            instanceName = service.serviceName
            for placement in self._dcd.partitioning.componentplacement:
                if instanceName == placement.componentinstantiation[0].usagename:
                    spd_id = placement.componentfileref.get_refid()
                    refid = placement.componentinstantiation[0].get_id()
                    for componentfile in self._dcd.componentfiles.componentfile:
                        if componentfile.get_id() == spd_id:
                            profile = componentfile.localfile.name
                            break
                    break
            implId = self.ref.getComponentImplementationId(refid)
            if profile:
                spd, scd, prf = _readProfile(profile, self.fs)
                return Service(profile, spd, scd, prf, serviceRef, instanceName, refid, implId)
            else:
                return RogueService(serviceRef, instanceName )
        except _CORBA.Exception:
            log.warn('Ignoring inaccessible service')

    def __serviceAddedEvent(self, event):
        if not self.ref:
            return
        # The service added event does not include the device manager to which
        # the new service belongs, so check it against the CORBA object's list
        # of current services.
        # NB: The way the current device manager implementation works, this is
        #     an unlikely occurrance, anyway.
        try:
            services = self.ref._get_registeredServices()
        except:
            # The device manager is not reachable.
            return
        for service in services:
            if service.serviceName == event.sourceName:
                # Pass the CF.DeviceManager.ServiceType object, not the service
                # reference. The DomainObjectList expects the argument to an
                # individual add to be the same as the type of one item in the
                # CORBA list, which in this case is ServiceType.
                self.__services.add(event.sourceName, service)
                break

    def __serviceRemovedEvent(self, event):
        # Remove the service from the list, ignoring the error raised if it
        # doesn't belong to this DeviceManager (see also __deviceRemovedEvent).
        try:
            self.__services.remove(event.sourceName)
        except KeyError:
            pass


class AllocationManager(CorbaObject):
    LOCAL_DEVICES=_CF.AllocationManager.LOCAL_DEVICES
    ALL_DEVICES=_CF.AllocationManager.ALL_DEVICES
    AUTHORIZED_DEVICES=_CF.AllocationManager.AUTHORIZED_DEVICES
    LOCAL_ALLOCATIONS=_CF.AllocationManager.LOCAL_ALLOCATIONS
    ALL_ALLOCATIONS=_CF.AllocationManager.ALL_ALLOCATIONS

    def __init__(self, ref=None):
        CorbaObject.__init__(self, ref)


    @property
    def allDevices(self):
        if self.ref:
            try:
               return self.ref._get_allDevices()
            except: 
               pass
        return []

    @property
    def authorizedDevices(self):
        if self.ref:
            try:
               return self.ref._get_authorizedDevices()
            except: 
               pass
        return []

    @property
    def localDevices(self):
        if self.ref:
            try:
                return self.ref._get_localDevices()
            except: 
               pass
        return []

    @property
    def getDomainMgr(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_domainMgr()
            except:
                pass
        return retval

    def createRequest(self, requestId, props, pools=[], devices=[], sourceId=''):
        if type(props) == dict:
            props=properties.props_from_dict(props)

        return _CF.AllocationManager.AllocationRequestType(requestId, props, pools, devices, sourceId)

    def allocate( self, allocationRequests=[], local=False ):
        retval=[]
        if self.ref:
            try:
                if local:
                   retval = self.ref.allocateLocal( allocationRequests )
                else:
                   retval = self.ref.allocate( allocationRequests )
            except:
                raise
        return retval

    def deallocate( self, allocationIds=[] ):
        if self.ref:
            try:
                self.ref.deallocate( allocationIds )
            except:
                raise

    def allocations( self, allocationIds=[], local=False ):
        retval=[]
        if self.ref:
            try:
                if local:
                   retval = self.ref.localAllocations( allocationIds )
                else:
                   retval = self.ref.allocations( allocationIds )
            except:
                raise
        return retval


    def listDevices( self, deviceScope=ALL_DEVICES, count=0 ):
        retval = ([],None)
        if self.ref:
            try:
                retval = self.ref.listDevices( deviceScope, count )
                if len(retval) > 1 and retval[1]:
                   return ( retval[0], IteratorContainer("DeviceLocationList", retval[1]) )
            except:
                raise
        return retval

    def listAllocations( self, allocationScope=ALL_DEVICES, count=0 ):
        retval = ([],None)
        if self.ref:
            try:
                retval = self.ref.listAllocations( allocationScope, count )
                if len(retval) > 1 and retval[1]:
                   return ( retval[0], IteratorContainer("AllocationsList", retval[1]) )
            except:
                raise
        return retval

class ConnectionManager(CorbaObject):

    ENDPOINT_APPLICATION=_CF.ConnectionManager.ENDPOINT_APPLICATION 
    ENDPOINT_DEVICE=_CF.ConnectionManager.ENDPOINT_DEVICE
    ENDPOINT_SERVICE=_CF.ConnectionManager.ENDPOINT_SERVICE
    ENDPOINT_EVENTCHANNEL=_CF.ConnectionManager.ENDPOINT_EVENTCHANNEL
    ENDPOINT_COMPONENT=_CF.ConnectionManager.ENDPOINT_COMPONENT
    ENDPOINT_DOMAINMANAGER=_CF.ConnectionManager.ENDPOINT_DOMAINMANAGER
    ENDPOINT_DEVICEMANAGER=_CF.ConnectionManager.ENDPOINT_DEVICEMANAGER
    ENDPOINT_OBJECTREF=_CF.ConnectionManager.ENDPOINT_OBJECTREF
    def __init__(self, ref=None):
        CorbaObject.__init__(self, ref)

    @property
    def connections(self):
        if self.ref:
            try:
               return self.ref._get_connections()
            except: 
               pass
        return []

    def endPointRequest( self, rsc_id, port_name, endpoint_kind ):
        restype = _CF.ConnectionManager.EndpointResolutionType(componentId=rsc_id)
        if endpoint_kind == ENDPOINT_APPLICATION:
            restype = _CF.ConnectionManager.EndpointResolutionType(applicationId=rsc_id)
        if endpoint_kind == ENDPOINT_DEVICE:
            restype = _CF.ConnectionManager.EndpointResolutionType(deviceId=rsc_id)
        if endpoint_kind == ENDPOINT_SERVICE:
            restype = _CF.ConnectionManager.EndpointResolutionType(serviceName=rsc_id)
        if endpoint_kind == ENDPOINT_EVENTCHANNEL:
            restype = _CF.ConnectionManager.EndpointResolutionType(channelName=rsc_id)
        if endpoint_kind == ENDPOINT_DEVICEMANAGER:
            restype = _CF.ConnectionManager.EndpointResolutionType(deviceMgr=rsc_id)
        if endpoint_kind == ENDPOINT_OBJECTREF:
            restype = _CF.ConnectionManager.EndpointResolutionType(objectRef=rsc_id)
        return _CF.ConnectionManager.EndpointRequest(restype, port_name)

    def applicationEndPoint( self, app_id, port_name ) :
        restype = _CF.ConnectionManager.EndpointResolutionType(applicationId=app_id)
        return _CF.ConnectionManager.EndpointRequest(restype, port_name)

    def deviceEndPoint( self, dev_id, port_name ) :
        restype = _CF.ConnectionManager.EndpointResolutionType(deviceId=dev_id)
        return _CF.ConnectionManager.EndpointRequest(restype, port_name)

    def componentEndPoint( self, comp_id, port_name ) :
        restype = _CF.ConnectionManager.EndpointResolutionType(componentId=comp_id)
        return _CF.ConnectionManager.EndpointRequest(restype, port_name)

    def serviceEndPoint( self, svc_name, port_name ) :
        restype = _CF.ConnectionManager.EndpointResolutionType(serviceName=svc_name)
        return _CF.ConnectionManager.EndpointRequest(restype, port_name)

    def eventChannelEndPoint( self, channel_name, port_name ) :
        restype = _CF.ConnectionManager.EndpointResolutionType(channelName=channel_name)
        return _CF.ConnectionManager.EndpointRequest(restype, port_name)

    def deviceMgrEndPoint( self, devMgr_name, port_name ) :
        restype = _CF.ConnectionManager.EndpointResolutionType(deviceMgrId=devMgr_name)
        return _CF.ConnectionManager.EndpointRequest(restype, port_name)

    def objectRefEndPoint( self, obj_ref, port_name ) :
        restype = _CF.ConnectionManager.EndpointResolutionType(objectRef=obj_ref)
        return _CF.ConnectionManager.EndpointRequest(restype, port_name)


    def connect( self, usesEndPoint, providesEndPoint, requesterId, connectionId ):
        retval=None
        if self.ref:
            try:
                retval = self.ref.connect(usesEndPoint, providesEndPoint, requesterId, connectionId )
            except:
                raise
        return retval

    def disconnect( self, connectionRecordId ):
        if self.ref:
            try:
                self.ref.disconnect( connectionRecordId )
            except:
                raise

    def listConnections( self, count=0 ):
        retval = ([],None)
        if self.ref:
            try:
                retval = self.ref.listConnections(count )
                if len(retval) > 1 and retval[1]:
                   return ( retval[0], IteratorContainer("ConnectionsList", retval[1]) )
            except:
                raise
        return retval


class EventChannelManager(CorbaObject):

    def __init__(self, ref=None):
        CorbaObject.__init__(self, ref)

    def release( self, channelName ):
        if self.ref:
            try:
                retval = self.ref.release(channelName)
            except:
                raise

    def create( self, channelName ):
        retval=None
        if self.ref:
            try:
                retval = self.ref.create(channelName)
            except:
                raise
        return retval

    def createForRegistrations( self, channelName ):
        retval=None
        if self.ref:
            try:
                retval=self.ref.createForRegistrations( channelName )
            except:
                raise
        return retval

    def markForRegistrations( self, channelName ):
        if self.ref:
            try:
                self.ref.markForRegistrations( channelName )
            except:
                raise

    def registerResource( self, channelName, reg_id="" ):
        retval=None
        if self.ref:
            try:
                reg = _CF.EventChannelManager.EventRegistration( channelName, reg_id ) 
                retval = self.ref.registerResource( reg )
            except:
                raise
        return retval

    def unregister( self, channelName, reg_id ):
        if self.ref:
            try:
                reg = _CF.EventChannelManager.EventRegistration( channelName, reg_id ) 
                self.ref.unregister( reg )
            except:
                raise

    def listChannels( self, count=0 ):
        retval = ([],None)
        if self.ref:
            try:
                retval = self.ref.listChannels(count)
                if len(retval) > 1 and retval[1]:
                   return ( retval[0], IteratorContainer("ChannelList", retval[1]) )
            except:
                raise
        return retval

    def listRegistrants( self, channelName="", count=0 ):
        retval = ([],None)
        if self.ref:
            try:
                retval = self.ref.listRegistrants(channelName, count )
                if len(retval) > 1 and retval[1]:
                   return ( retval[0], IteratorContainer("ChannelRegistrationList", retval[1]) )
            except:
                raise
        return retval



class Domain(_CF__POA.DomainManager, QueryableBase, PropertyEmitter):
    """The Domain is a descriptor for a Domain Manager.
    
        The main functionality that can be exercised by this class is:
        - terminate - uninstalls all running waveforms and terminates the node
        - waveform management:
            - createApplication - install/create a particular waveform application
            - removeApplication - release a particular waveform application
    """
    @notification
    def deviceManagerAdded(self, deviceManager):
        """
        The device manager 'deviceManager' was added to the system.
        """
        log.trace('deviceManagerAdded %s', deviceManager.id)

    @notification
    def deviceManagerRemoved(self, identifier):
        """
        A device manager with the given identifier was removed from the system.
        """
        log.trace('deviceManagerRemoved %s', identifier)

    @notification
    def applicationAdded(self, application):
        """
        The application object 'application' was added to the system.
        """
        log.trace('applicationAdded %s', application.name)

    @notification
    def applicationRemoved(self, identifier):
        """
        An application with the given identifier was removed from the system.
        """
        log.trace('applicationRemoved %s', identifier)

    @notification
    def deviceRegistered(self, device):
        """
        The device object 'device' registered with this DomainManager.
        """
        log.trace('deviceRegistered %s', device)

    @notification
    def deviceUnregistered(self, identifier):
        """
        The device with the given identifier unregistered from this
        DomainManager.
        """
        log.trace('deviceUnregistered %s', identifier)

    @notification
    def serviceRegistered(self, service):
        """
        The service object 'service' registered with this DomainManager.
        """
        log.trace('serviceRegistered %s', service.serviceName)

    @notification
    def serviceUnregistered(self, name):
        """
        The service with the given name unregistered from this DomainManager.
        """
        log.trace('serviceUnregistered %s', name)

    def __init__(self, name="DomainName1", location=None, connectDomainEvents=True):
        self.name = name
        self._sads = []
        self._sadFullPath = []
        self.ref = None
        self.NodeAlive = True
        self._waveformsUpdated = False
        self.location = location
        self.__odmListener = None
        self.__idmListener = None
        self.__allocationMgr = None
        self.__connectionMgr = None
        self.__eventChannelMgr = None
        
        # create orb reference
        self.orb = _CORBA.ORB_init(_sys.argv, _CORBA.ORB_ID)
        if location:
            obj = self.orb.string_to_object('corbaname::'+location)
        else:
            obj = self.orb.resolve_initial_references("NameService")
        try:
            self.rootContext = obj._narrow(_CosNaming.NamingContext)
        except:
            raise RuntimeError('NameService not found')

        # get DomainManager reference
        dm_name = [_CosNaming.NameComponent(self.name,""),_CosNaming.NameComponent(self.name,"")]
        found_domain = False
        
        domain_find_attempts = 0
        
        self.poa = self.orb.resolve_initial_references("RootPOA")
        self.poaManager = self.poa._get_the_POAManager()
        self.poaManager.activate()
        
        while not found_domain and domain_find_attempts < 30:
            try:
                obj = self.rootContext.resolve(dm_name)
                found_domain = True
            except:
                _time.sleep(0.1)
                domain_find_attempts += 1
        
        if domain_find_attempts == 30:
            raise StandardError, "Did not find domain "+name
                
        self.ref = obj._narrow(_CF.DomainManager)
        self.fileManager = self.ref._get_fileMgr()
        self.id = self.ref._get_identifier()
        try:
            spd, scd, prf = _readProfile("/mgr/DomainManager.spd.xml", self.fileManager)
            super(Domain, self).__init__(prf, self.id)
        except Exception, e:
            pass

        self._buildAPI()

        self.__deviceManagers = DomainObjectList(weakobj.boundmethod(self._get_deviceManagers),
                                                 weakobj.boundmethod(self.__newDeviceManager),
                                                 lambda x: x._get_identifier())
        self.__applications = DomainObjectList(weakobj.boundmethod(self._get_applications),
                                               weakobj.boundmethod(self.__newApplication),
                                               lambda x: x._get_identifier())

        # Connect notification points to domain object lists.
        self.__deviceManagers.itemAdded.addListener(weakobj.boundmethod(self.deviceManagerAdded))
        self.__deviceManagers.itemRemoved.addListener(weakobj.boundmethod(self.deviceManagerRemoved))
        self.__applications.itemAdded.addListener(weakobj.boundmethod(self.applicationAdded))
        self.__applications.itemRemoved.addListener(weakobj.boundmethod(self.applicationRemoved))

        # Connect the the IDM/ODM channels unless disabled.
        self.__idmListener = None
        self.__odmListener = None
        if connectDomainEvents:
            self.__connectIDMChannel()
            self.__connectODMChannel()

    def _populateApps(self):
        self.__setattr__('_waveformsUpdated', True)
        self._updateListAvailableSads()

    def __newDeviceManager(self, deviceManager):
        try:
            label = deviceManager._get_label()
            dcdPath = deviceManager._get_deviceConfigurationProfile()
            devMgrFileSys = deviceManager._get_fileSys()
            dcdFile = devMgrFileSys.open(dcdPath, True)
        except:
            raise RuntimeError, "Unable to open $SDRROOT/dev"+dcdPath+". Unable to create proxy for Device Manager"
        dcdContents = dcdFile.read(dcdFile.sizeOf())
        dcdFile.close()

        parsed_dcd=parsers.dcd.parseString(dcdContents)
        return DeviceManager(name=label, devMgr=deviceManager, dcd=parsed_dcd, domain=weakref.proxy(self), idmListener=self.__idmListener, odmListener=self.__odmListener)
    
    @property
    def devMgrs(self):
        # If the ODM channel is not connected, force an update to the list.
        if not self.__odmListener:
            self.__deviceManagers.sync()
        return self.__deviceManagers.values()

    def __newApplication(self, app):
        prof_path = app._get_profile()

        sadFile = self.fileManager.open(prof_path, True)
        sadContents = sadFile.read(sadFile.sizeOf())
        sadFile.close()
        doc_sad = parsers.sad.parseString(sadContents)
        comp_list = app._get_componentNamingContexts()
        waveform_ns_name = ''
        if len(comp_list) > 0:
            comp_ns_name = comp_list[0].elementId
            waveform_ns_name = comp_ns_name.split('/')[1]

        app_name = app._get_name()
        if app_name[:7]=='OSSIE::':
            waveform_name = app_name[7:]
        else:
            waveform_name = app_name
        waveform_entry = App(name=waveform_name, domain=weakref.proxy(self), sad=doc_sad)
        waveform_entry.ref = app
        waveform_entry.ns_name = waveform_ns_name
        return waveform_entry

    @property
    def apps(self):
        # If the ODM channel is not connected, force an update to the list.
        if not self.__odmListener:
            self.__applications.sync()
        return self.__applications.values()

    @property
    def devices(self):
        devs = []
        for devMgr in self.devMgrs:
            devs.extend(devMgr.devs)
        return devs

    @property
    def services(self):
        svcs = []
        for devMgr in self.devMgrs:
            svcs.extend(devMgr.services)
        return svcs

    # this function is a bit of overkill. The reason why an evaluation
    # function is passed is that because at first, the retrieve function
    # did an exact match and then a regular expression, so it was simpler
    # to pass the evaluation criteria rather than hard-code the behavior
    def __execute_search(self, element=None, search='', func=None):
        if element == None:
            return None
        if func == None:
            return None
        devices = self.devices
        for device in devices:
            if func(device.__dict__[element], search):
                return device
         # search services
        services = self.services
        for service in services:
            if func(service.__dict__[element], search):
                return service
         # search apps
        apps = self.apps
        for app in apps:
            if func(app.__dict__[element], search):
                return app
         # search for components on apps
        for app in apps:
            for comp in app.comps:
                if func(comp.__dict__[element], search):
                    return comp

    def retrieve(self, search=''):
        """
        find a component, device, service, application, or event channel based on
        the given search string. The search pattern is
        for (1) the identifier. If there are no hits on the
        identifier, then (2) for the name
        """
        if search == '':
            raise Exception('A valid search pattern must be provided')
        # search by id
         # search devices, services, apps, and components on apps
        def __equals(a,b):
            return a==b
        retval = self.__execute_search('_id', search, __equals)
        if retval:
            return retval
        # search by name
        retval = self.__execute_search('_instanceName', search, __equals)
        if retval:
            return retval
         # search for event channels
        evtMgr = self.ref._get_eventChannelMgr()
        get_number = 100
        evt_channels = evtMgr.listChannels(get_number)
        if len(evt_channels[0]) == get_number:
            done = False
            while not done:
                ret_ch = evt_channels[1].next_n(get_number)
                if len(ret_ch[1]) != get_number:
                    done = True
                evt_channels[0] = evt_channels[0] + ret_ch[1]
        for evt_channel in evt_channels[0]:
            if evt_channel.channel_name == search:
                return evt_channel

        raise Exception('No element found with the given criteria')

    def __connectIDMChannel(self):
        """
        Connect the the IDM channel for updates to device states.
        """
        idmListener = IDMListener()
        try:
            idmListener.connect(self.ref)
        except Exception, e:
            # No device events will be received
            log.warning('Unable to connect to IDM channel: %s', e)
            return

        self.__idmListener = idmListener

    def __connectODMChannel(self):
        """
        Connect to the ODM channel for updates to domain objects.
        """
        odmListener = ODMListener()
        try:
            odmListener.connect(self.ref)
        except Exception, e:
            # No domain object events will be received
            log.warning('Unable to connect to ODM channel: %s', e)
            return

        self.__odmListener = odmListener

        # Object lists directly managed by DomainManager
        # NB: In contrast to other languages, this object is deleted before
        #     the objects it references, so use weak listeners to prevent
        #     race conditions that lead to annoying error messages.
        weakobj.addListener(odmListener.deviceManagerAdded, self.__deviceManagerAddedEvent)
        weakobj.addListener(odmListener.deviceManagerRemoved, self.__deviceManagerRemovedEvent)
        weakobj.addListener(odmListener.applicationAdded, self.__applicationAddedEvent)
        weakobj.addListener(odmListener.applicationRemoved, self.__applicationRemovedEvent)
        weakobj.addListener(odmListener.applicationFactoryAdded, self.__appFactoryAddedEvent)
        weakobj.addListener(odmListener.applicationFactoryRemoved, self.__appFactoryRemovedEvent)

        # Object lists managed by DeviceManagers
        # NB: See above.
        weakobj.addListener(odmListener.deviceAdded, self.__deviceAddedEvent)
        weakobj.addListener(odmListener.deviceRemoved, self.__deviceRemovedEvent)
        weakobj.addListener(odmListener.serviceAdded, self.__serviceAddedEvent)
        weakobj.addListener(odmListener.serviceRemoved, self.__serviceRemovedEvent)

    def __del__(self):
        """
            Destructor
        """
        # Explicitly disconnect ODM listener to avoid warnings on shutdown
        if self.__odmListener:
            try:
                self.__odmListener.disconnect()
            except Exception, e:
                pass

        # Explictly disconnect IDM Listener to avoid warnings on shutdown
        if self.__idmListener:
            try:
                self.__idmListener.disconnect()
            except:
                pass

    ########################################
    # External Domain Manager API
    
    def _get_identifier(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_identifier()
            except:
                pass
        return retval
    
    def _get_deviceManagers(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_deviceManagers()
            except:
                pass
        return retval
    
    def _get_applications(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_applications()
            except:
                pass
        return retval
    
    def _get_applicationFactories(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_applicationFactories()
            except:
                pass
        return retval
    
    def _get_fileMgr(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_fileMgr()
            except:
                pass
        return retval
    
    def _get_domainManagerProfile(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_domainManagerProfile()
            except:
                pass
        return retval
    
    def configure(self, props):
        if self.ref:
            try:
                self.ref.configure(props)
            except:
                raise
    
    def query(self, props):
        retval = None
        if self.ref:
            try:
                retval = self.ref.query(props)
            except:
                raise
        return retval
    
    def registerDevice(self, device, deviceManager):
        if self.ref:
            try:
                self.ref.registerDevice(device, deviceManager)
            except:
                raise
    
    def registerDeviceManager(self, deviceManager):
        if self.ref:
            try:
                self.ref.registerDeviceManager(deviceManager)
            except:
                raise
    
    def unregisterDevice(self, device):
        if self.ref:
            try:
                self.ref.unregisterDevice(device)
            except:
                raise
    
    def unregisterDeviceManager(self, deviceManager):
        if self.ref:
            try:
                self.ref.unregisterDeviceManager(deviceManager)
            except:
                raise
    
    def installApplication(self, profile):
        if self.ref:
            try:
                self.ref.installApplication(profile)
            except:
                raise
    
    def uninstallApplication(self, appid):
        if self.ref:
            try:
                self.ref.uninstallApplication(appid)
            except:
                raise
    
    def registerService(self, service, deviceManager, name):
        if self.ref:
            try:
                self.ref.registerService(service, deviceManager, name)
            except:
                raise
    
    def unregisterService(self, service, name):
        if self.ref:
            try:
                self.ref.unregisterService(service, name)
            except:
                raise
    
    def registerWithEventChannel(self, registeringObject, registeringId, eventChannelName):
        if self.ref:
            try:
                self.ref.registerWithEventChannel(registeringObject, registeringId, eventChannelName)
            except:
                raise
    
    def unregisterFromEventChannel(self, unregisteringId, eventChannelName):
        if self.ref:
            try:
                self.ref.unregisterFromEventChannel(unregisteringId, eventChannelName)
            except:
                raise
    
    def getAllocationMgr(self):
        if self.ref and self.__allocationMgr == None :
            try:
                self.__allocationMgr = AllocationManager(self.ref._get_allocationMgr())
            except:
                raise
        return self.__allocationMgr

    def getConnectionMgr(self):
        if self.ref and self.__connectionMgr == None :
            try:
                self.__connectionMgr = ConnectionManager(self.ref._get_connectionMgr())
            except:
                raise
        return self.__connectionMgr

    def getEventChannelMgr(self):
        if self.ref and self.__eventChannelMgr == None :
            try:
                self.__eventChannelMgr = EventChannelManager(self.ref._get_eventChannelMgr())
            except:
                raise
        return self.__eventChannelMgr

    # End external Domain Manager API
    ########################################
    
    def _searchFilePattern(self, starting_point, pattern):
        file_list = self.fileManager.list(starting_point+'/*')
        filesFound = []
        for entry in file_list:
            if entry.kind == _CF.FileSystem.DIRECTORY:
                if starting_point == '/':
                    filesFound.extend(self._searchFilePattern(starting_point+entry.name, pattern))
                else:
                    filesFound.extend(self._searchFilePattern(starting_point+'/'+entry.name, pattern))
            else:
                if pattern in entry.name:
                    filesFound.append(starting_point+'/'+entry.name)
        return filesFound
    
    def catalogSads(self):
        self._updateListAvailableSads()
        return self._sadFullPath

    def _updateListAvailableSads(self):
        """
            Update available waveforms list.
        """
        sadList = self._searchFilePattern('/waveforms', 'sad.xml')
        for entry in range(len(sadList)):
            sad_filename = sadList[entry].split('/')[-1]
            sad_entry = sad_filename.split('.')[-3]
            if not (sad_entry in self._sads):
                self._sads.append(sad_entry)
                self._sadFullPath.append(sadList[entry])
    
    def terminate(self):
        # Kills waveforms (in reverse order since removeApplication() pops items from list) 
        for app in reversed(self.apps):
            self.removeApplication(app)
        
        # Kills nodes
        for node in self.devMgrs:
            node.shutdown()
    
    def removeApplication(self, app_obj=None):
        if app_obj == None:
            return

        # Remove the application from the internal list.
        appId = app_obj._get_identifier()
        self.__applications.remove(appId)
        
        for app in _launchedApps:
            if app._get_identifier() == appId:
                _launchedApps.remove(app)
                break
            
        app_obj.ref.releaseObject()

    def createApplication(self, application_sad='', name=None, initConfiguration={}):
        """Install and create a particular waveform. This function returns
            a pointer to the instantiated waveform"""
        uninstallAppWhenDone = True
        # If only an application name is given, format it properly
        if application_sad[0] != "/" and not ".sad.xml" in application_sad:
            application_sad = "/waveforms/" + application_sad + "/" + application_sad + ".sad.xml"
        
        try:
            self.ref.installApplication(application_sad)
        except _CF.DomainManager.ApplicationAlreadyInstalled:
            uninstallAppWhenDone = False
    
        sadFile = self.fileManager.open(application_sad, True)
        try:
            sadContents = sadFile.read(sadFile.sizeOf())
        finally:
            sadFile.close()
    
        doc_sad = parsers.SADParser.parseString(sadContents)
        app_id = str(doc_sad.get_id())
    
        # Find the application factory that is needed
        app_factory = None
        for factory in self.ref._get_applicationFactories():
            if factory._get_identifier() == app_id:
                app_factory = factory
                break
    
        if app_factory is None:
            raise AssertionError("Application factory not found")

        # If no name is given, generate a unique one from the clock
        if name is None:
            name = '%s_%s' % (app_factory._get_name(), getCurrentDateTimeString())

        # Convert dictionary to properties, otherwise assume that initial
        # configuration is already a list of properties.
        if isinstance(initConfiguration, dict):
            initConfiguration = properties.props_from_dict(initConfiguration)

        try:
            app = app_factory.create(name, initConfiguration, [])
        finally:
            if uninstallAppWhenDone:
                self.ref.uninstallApplication(app_id)
        
        appId = app._get_identifier()
        # Add the app to the application list, or get the existing object if
        # the ODM event was processed first.
        waveform_entry = self.__applications.add(appId, app)
        if getTrackApps():
            _launchedApps.append(waveform_entry)
        
        return waveform_entry
    
    def _updateRunningApps(self):
        """Makes sure that the dictionary of waveforms is up-to-date"""
        print "WARNING: _updateRunningApps() is deprecated.  Running apps are automatically updated on access."

    ########################################
    # Internal event channel management
    def __deviceManagerAddedEvent(self, event):
        try:
            self.__deviceManagers.add(event.sourceId, event.sourceIOR)
        except:
            # The device manager is already gone or otherwise unavailable.
            pass

    def __deviceManagerRemovedEvent(self, event):
        self.__deviceManagers.remove(event.sourceId)

    def __deviceAddedEvent(self, event):
        pass

    def __deviceRemovedEvent(self, event):
        self.deviceUnregistered(event.sourceId)

    def __appFactoryAddedEvent(self, event):
        log.trace('Installed app factory %s', event.sourceName)

    def __appFactoryRemovedEvent(self, event):
        log.trace('Uninstalled app factory %s', event.sourceName)

    def __applicationAddedEvent(self, event):
        try:
            self.__applications.add(event.sourceId, event.sourceIOR)
        except:
            # The application is already gone or otherwise unavailable.
            pass

    def __applicationRemovedEvent(self, event):
        try:
            self.__applications.remove(event.sourceId)
        except KeyError:
            # Ignore applications already removed via App.releaseObject
            pass

    def __serviceAddedEvent(self, event):
        pass

    def __serviceRemovedEvent(self, event):
        self.serviceUnregistered(event.sourceName)




