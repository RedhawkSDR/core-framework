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
import os, sys, stat
from omniORB import URI, any
import commands, copy, time, signal, pprint, subprocess
import logging
import signal
import shutil

PROPERTIES = (
  ("DCE:6b298d70-6735-43f2-944d-06f754cd4eb9", "no_default_prop"),
  ("DCE:456310b2-7d2f-40f5-bfef-9fdf4f3560ea", "default_prop"),
  ("DCE:4a23ad60-0b25-4121-a630-68803a498f75", "os_name"),
  ("DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b", "processor_name"),
  ("DCE:7f36cdfb-f828-4e4f-b84f-446e17f1a85b", "DeviceKind"),
  ("DCE:64303822-4c67-4c04-9a5c-bf670f27cf39", "RunsAs"),
  ("DCE:8cad8ca5-c155-4d1d-ae40-e194aa1d855f", "bandwidthCapacity"),
  ("number_connections", "number_connections"),
  ("prop3", "prop3"),
)

_ID_MAP = dict([(prop[0], pos) for pos, prop in enumerate(PROPERTIES)])
_NAME_MAP = dict([(prop[1], pos) for pos, prop in enumerate(PROPERTIES)])

def getName(propid):
    return PROPERTIES[_ID_MAP[propid]][1]

def getId(name):
    return PROPERTIES[_NAME_MAP[name]][0]


class toOther_i(CF__POA.Port):
    def __init__(self, parent, name):
        self.parent = parent
        self.name = name
        self.outPorts = {}
    
    def connectPort(self, connection, connectionId):
        port = connection._narrow(CF__POA.Resource)
        self.outPorts[str(connectionId)] = port
        number_connections = self.parent.props[getId("number_connections")].value._v
        self.parent.props[getId("number_connections")] = CF.DataType(id=getId("number_connections"), value=any.to_any(number_connections+1))
    
    def disconnectPort(self, connectionId):
        if self.outPorts.has_key(str(connectionId)):
            self.outPorts.pop(str(connectionId), None)   
            number_connections = self.parent.props[getId("number_connections")].value._v
            self.parent.props[getId("number_connections")] = CF.DataType(id=getId("number_connections"), value=any.to_any(number_connections-1))

    def getIdentifiers(self):
        props = []
        for id in self.outPorts:
            value = any.to_any(self.outPorts[id]._get_identifier())
            props.append(CF.DataType(id=id, value=value))
        return props

class BasicUsesDevice(CF__POA.Device):

  STOP_SIGNALS = ((signal.SIGINT, 1),
                  (signal.SIGTERM, 5),
                  (signal.SIGKILL, None))

  def __init__(self, devmgr=None, uuid=None, label=None, softwareProfile=None):
    self.props = {}
    self.uuid = uuid
    self._devmgr = devmgr
    self._label = label
    self._usageState = CF.Device.IDLE
    self._adminState = CF.Device.UNLOCKED
    self._operationalState = CF.Device.ENABLED
    self._softwareProfile = softwareProfile
    self._compositeDevice = None
    self._log = logging.getLogger(label)

    if self._devmgr:
        self._devmgr.registerDevice(self._this())

  # Helper Methods
  def isLocked(self):
    if self._adminState == CF.Device.LOCKED: return True
    return False
  
  def isUnLocked(self):
    if self._adminState == CF.Device.UNLOCKED: return True
    return False
  
  def isDisabled(self):
    if self._operationalState == CF.Device.DISABLED: return True
    return False

  def isBusy(self):
    if self._usageState == CF.Device.BUSY: return True
    return False

  def isIdle(self):
    if self._usageState == CF.Device.IDLE: return True
    return False

  #########################################
  # CF::Resource
  def start(self):
    self._log.debug("BasicUsesDevice.start()")
    pass

  def stop(self):
    self._log.debug("BasicUsesDevice.stop()")
    pass

  def _get_identifier(self):
    return self.uuid
  
  # CF::LifeCycle
  def initialize(self):
    self._log.debug("BasicUsesDevice.initialize()")
    # Initialize capacity properties
    self.props[getId("bandwidthCapacity")] = CF.DataType(id=getId("bandwidthCapacity"), value=any.to_any(100000000))
    self.props[getId("number_connections")] = CF.DataType(id=getId("number_connections"), value=any.to_any(0))
    self.props[getId("prop3")] = CF.DataType(id=getId("prop3"), value=any.to_any("yes"))
    self.toOther = toOther_i(self, "resource_out")

  def releaseObject(self):
    self._log.debug("BasicUsesDevice.releaseObject()")
    self.props = {}
    try:
        if self._devmgr:
            self._devmgr.unregisterDevice(self._this())
    except:
       raise CF.LifeCycle.ReleaseError()

  # CF::PropertySet
  def initializeProperties(self, ctorProperties):
    self._log.debug("BasicUsesDevice.initializeProperties(%s)", ctorProperties)
    for prop in ctorProperties:
        if not self.props.has_key(prop.id):
            self.props[prop.id] = CF.DataType(id=prop.id, value=prop.value)
        else:
            self.props[prop.id].value = prop.value
    self._log.debug("BasicUsesDevice initializeProperties %s", self.props)
  
  # CF::PropertySet
  def query(self, configProperties):
    self._log.debug("BasicUsesDevice.query(%s)", configProperties)
    if configProperties == []:
      rv = []
      for key in self.props.keys():
        val = self.props[key].value
        d = CF.DataType(id=key, value=val)
        rv.append(d)
    else:
      unknownProperties = []
      for prop in configProperties:
        try:
            prop.value = self.props[prop.id].value
        except KeyError:
            unknownProperties.append(prop)

      if len(unknownProperties) > 0:
        raise CF.UnknownProperties(unknownProperties)

      rv = configProperties
    self._log.debug("BasicUsesDevice.query() -> %s", rv)
    return rv

  def configure(self, configProperties):
    self._log.debug("BasicUsesDevice.configure(%s)", configProperties)
    for prop in configProperties:
        if not self.props.has_key(prop.id):
            self.props[prop.id] = CF.DataType(id=prop.id, value=prop.value)
        else:
            self.props[prop.id].value = prop.value
    self._log.debug("BasicUsesDevice Properties %s", self.props)

  # CF::PortSupplier
  def getPort(self, name):
    self._log.debug("BasicUsesDevice.getPort(%s)", name)
    if name == "resource_out":
        return self.toOther._this()
    return None

  # CF::TestableObject
  def runTest(self, properties, testid):
    self._log.debug("BasicUsesDevice.runTest()")
    if not isinstance(testid, int) or testid < 1 or testid > 1:
      raise CF.Device.UnknownTest("unknown test: %s" % str(testid))
    if not isinstance(properties, list):
      raise CF.Device.UnknownProperties("properties is not a list")
    if testid == 1:
      return "NOT IMPLEMENTED"
       
  ###########################################       
  # CF::Device
  def allocateCapacity(self, properties):
    self._log.debug("BasicUsesDevice.allocateCapacity(%s)", properties)
    result = True
    # Validate
    for prop in properties:
        if not prop.id in (getId("bandwidthCapacity"), getId("prop3")):
            raise CF.Device.InvalidCapacity(CF.CFNOTSET, "Invalid capacity %s" % prop.id)

    # Consume
    for prop in properties:
        if prop.id == getId("bandwidthCapacity"):
            print type(self.props[prop.id].value._v)
            print type(prop.value._v)
            self.props[prop.id].value._v = self.props[prop.id].value._v - prop.value._v
            if self.props[prop.id].value._v < 0:
                result = False

    # If we couldn't allocate enough capacity, add it back
    if result == False:
        for prop in properties:
            if prop.id == getId("bandwidthCapacity"):
                self.props[prop.id].value._v = self.props[prop.id].value._v + prop.value._v

    # Update usage state
    if self.props[getId("bandwidthCapacity")].value._v == 0 and self.props[getId("bandwidthCapacity")].value._v == 0:
        self._usageState = CF.Device.BUSY
    elif self.props[getId("bandwidthCapacity")].value._v == 100000000 and self.props[getId("bandwidthCapacity")].value._v == 100000000: 
        self._usageState = CF.Device.IDLE
    else:
        self._usageState = CF.Device.ACTIVE

    self._log.debug("BasicUsesDevice.allocateCapacity() --> %s", result)
    return result

  def deallocateCapacity(self, properties):
    self._log.debug("BasicUsesDevice.deallocateCapacity(%s)", properties)
    # Validate
    for prop in properties:
        if not prop.id in (getId("bandwidthCapacity"), getId("prop3")):
            raise CF.Device.InvalidCapacity(CF.CFNOTSET, "Invalid capacity %s" % prop.id)

    # Unconsume
    for prop in properties:
        if prop.id == getId("bandwidthCapacity"):
            self.props[prop.id].value._v = self.props[prop.id].value._v + prop.value._v
        elif prop.id == getId("prop3"):
            pass
        else:
            raise CF.Device.InvalidCapacity(CF.CFNOTSET, "Invalid capacity %s" % prop.id)

    # Update usage state
    if self.props[getId("bandwidthCapacity")].value._v == 0 and self.props[getId("bandwidthCapacity")].value._v == 0:
        self._usageState = CF.Device.BUSY
    elif self.props[getId("bandwidthCapacity")].value._v == 100000000 and self.props[getId("bandwidthCapacity")].value._v == 100000000: 
        self._usageState = CF.Device.IDLE
    else:
        self._usageState = CF.Device.ACTIVE

    self._log.debug("BasicUsesDevice.deallocateCapacity() -->")
  
  # Attributes
  def _set_usageState(self, state):
    self._usageState = state
  
  def _get_usageState(self):
    return self._usageState

  def _set_adminState(self, state):
    self._adminState = state
  
  def _get_adminState(self):
    return self._adminState
   
  def _set_operationalState(self, state):
    self._operationalState = state
  
  def _get_operationalState(self):
    return self._operationalState
     
  def _get_softwareProfile(self):
    return self._softwareProfile
     
  def _get_label(self):
    return self._label
   
  def _set_compositeDevice(self, device):
    self._compositeDevice = device
  
  def _get_compositeDevice(self):
    return self._compositeDevice

def exit_handler(signum, frame):
    raise SystemExit

if __name__ == "__main__":
    # Grab command line arguments and options
    import sys, getopt
    # Import the CORBA stuff
    from omniORB import CORBA
    from omniORB import URI
    import CosNaming

    logging.basicConfig()
    logging.getLogger().setLevel(logging.DEBUG)

    args = sys.argv[1:]
    devmgr_ior = None
    label = None
    softwareProfile = None
    uuid = None

    logging.debug("Commandline args %s", args)
    while len(args) > 0:
        arg = args.pop(0)
        if arg == "DEVICE_MGR_IOR":
            devmgr_ior = str(args.pop(0))
        elif arg == "PROFILE_NAME":
            softwareProfile = str(args.pop(0))
        elif arg == "DEVICE_ID":
            uuid = str(args.pop(0))
        elif arg == "DEVICE_LABEL":
            label = args.pop(0)

    if uuid == None or label == None or softwareProfile == None:
        print "Missing arguments to properly start the GPPpy device"

    orb = None
    signal.signal(signal.SIGINT, exit_handler)
    signal.signal(signal.SIGTERM, exit_handler)
    try:
        try:
            orb = CORBA.ORB_init()
        
            # get the POA
            obj_poa = orb.resolve_initial_references("RootPOA")
            poaManager = obj_poa._get_the_POAManager()
            poaManager.activate()
            
            ns_obj = orb.resolve_initial_references("NameService")
            rootContext = ns_obj._narrow(CosNaming.NamingContext)

            # get the device manager
            if devmgr_ior != None:
                devMgr = orb.string_to_object(devmgr_ior)
                devMgr = devMgr._narrow(CF.DeviceManager)
            else:
                devMgr = None
            # create the main component object
            # TODO: Somehow we should create a CommandWrapperProperties.py file
            # based on the CommandWrapper.prf.xml as part of make
            component_Obj = BasicUsesDevice(devMgr, uuid, label, softwareProfile)
            component_Var = component_Obj._this()
            
            #logging.debug("Binding to name %s", label)
            #name = URI.stringToName(label)
            #rootContext.rebind(name, component_Var)
            logging.debug("Starting ORB event loop")
            orb.run()
        except SystemExit:
            pass
        except KeyboardInterrupt:
            pass
    finally:
        orb.destroy()
