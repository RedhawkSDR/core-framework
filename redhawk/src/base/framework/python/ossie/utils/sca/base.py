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

import commands
import os
import xml.dom.minidom
from ossie.cf import CF, CF__POA
from omniORB import CORBA
import CosNaming
import sys
import time

from xml.parsers.expat import ExpatError

from ossie.utils.sca import importIDL

# UUID Generator
def uuidgen():
   return commands.getoutput('uuidgen')

# Finds SDR root directory
def findSdrRoot():
    if 'SDRROOT' in os.environ and os.path.exists(os.environ['SDRROOT']):
        sdrroot = os.path.normpath(os.environ['SDRROOT'])
    elif os.path.exists('/sdr/sca'):
        sdrroot = '/sdr/sca'
    elif os.path.exists('/sdr'):
        sdrroot = '/sdr'
    else:
        print "Cannot find SDR root directory"
        return False
        
    return sdrroot

# Component Class
class Component(object):
  """This is a class used to access a Component in a deployed waveform
       Relevant member data:
         name - component's instance name (same as the key from the waveform dictionary)
         Ports - dictionary with all the ports in this component
         profile - name of the SPD XML that describes this component
         API - dictionary with the API for this component
         
       A developer interfaces with the component through its API. A call to getAPI()
        returns a dictionary where the name of the port is the key, and the corresponding
        value is:
          - type - Uses (output) or Provides (input)
          - namespace:interface - The IDL interface that the port implements
          - function list - list of all functions that the Port supports
  
  """
  def __init__(self, name="",AC=False,type="resource",generate=True, int_list=None):
    self.name = name        # this refers to the instance name
    self.baseName = name    # this refers to the component that the instance is based on
    self.connections = []
    self.ports = []
    self.Ports = {}
    self.mutable_params = []
    self.device = None
    self.node = None
    self.uuid = uuidgen()
    self.file_uuid = uuidgen()
    #self.ace = False
    self.timing = False
    self.logging = False
    self.AssemblyController = AC
    self.type = type
    self.generate = generate
    self.xmlName = name     #if imported from component library - this may change
    self.properties = []
    self.reference = None
    self.ref = None
    self.profile = ''
    self.spd_path = ''
    self.prf_path = ''
    self.scd_path = ''
    if os.path.exists('/sdr/sca'):
        self.root = '/sdr/sca'
    elif os.path.exists('/sdr'):
        self.root = '/sdr'
    else:
        self.root = ''
    if int_list == None:
        #print "importing idl"
        self.interface_list = importIDL.importStandardIdl()
    else:
        self.interface_list = int_list
    self.API = {}
    
  def getAPI(self):
      """Returns a dictionary with the API for this component
      """
      return self.API
  
  def printAPI(self):
      """Prints a user-friendly version of the API for this component onto the screen
      """
      for port in self.API:
          print "Port name: " + port
          print "  direction: " + self.API[port][0]
          print "  interface: " + self.API[port][1]
          for funcs in self.API[port][2]:
              print "    " + funcs
          print ""

  def populatePorts(self):
      """Add all port descriptions to the component instance"""
      if self.profile == '':
          print "Unable to create port list for " + self.name + " - profile unavailable"
          return
      if len(self.ports) != 0:
          return
      
      os.chdir(self.root)
      doc_spd = xml.dom.minidom.parse(self.profile)
      localfiles = doc_spd.getElementsByTagName('localfile')
      for localfile in localfiles:
          if localfile.parentNode.tagName == 'propertyfile':
              self.prf_path = os.path.join(self.root,localfile.getAttribute('name'))
          if localfile.parentNode.tagName == 'descriptor':
              self.scd_path = os.path.join(self.root,localfile.getAttribute('name'))
      doc_prf = xml.dom.minidom.parse(self.prf_path)
      doc_scd = xml.dom.minidom.parse(self.scd_path)

      interface_modules = []
      
      int_list = {}
      for entry in doc_scd.getElementsByTagName('interface'):
          for int_entry in self.interface_list:
              if int_entry.name == entry.getAttribute('name'):
                  int_list[entry.getAttribute('repid')]=int_entry
      
      for uses in doc_scd.getElementsByTagName('uses'):
          idl_repid = uses.getAttribute('repid')
          if not int_list.has_key(idl_repid):
              print "Invalid port descriptor in scd for " + self.name + " for " + idl_repid
              continue
          int_entry = int_list[idl_repid]
          new_port = Port(uses.getAttribute('usesname'), int_entry, type="Uses")
          new_port.generic_ref = self.reference.getPort(str(new_port.name))
          new_port.ref = new_port.generic_ref._narrow(CF.Port)
          self.ports.append(new_port)
          self.Ports[uses.getAttribute('usesname')]=new_port
      
      for provides in doc_scd.getElementsByTagName('provides'):
          idl_repid = provides.getAttribute('repid')
          if not int_list.has_key(idl_repid):
              print "Invalid port descriptor in scd for " + self.name + " for " + idl_repid
              continue
          int_entry = int_list[idl_repid]
          new_port = Port(provides.getAttribute('providesname'), int_entry, type="Provides")
          new_port.generic_ref = self.reference.getPort(str(new_port.name))
          
          # See if interface python module has been loaded, if not then try to import it
          if str(int_entry.nameSpace) not in interface_modules:
            try:
                exec_string = 'import ' + str(int_entry.nameSpace)
                exec exec_string
            except ImportError, msg:
                print msg
                continue
            else:
                interface_modules.append(str(int_entry.nameSpace))
          
          exec_string = 'new_port.ref = new_port.generic_ref._narrow('+int_entry.nameSpace+'.'+int_entry.name+')'

          exec(exec_string)
          self.ports.append(new_port)
          self.Ports[provides.getAttribute('providesname')]=new_port
      
      self.buildAPI()

  def buildAPI(self):
      """Build a dictionary with the API for the component"""
      if self.API != {}:
          return
          
      for port in self.Ports:
          if self.Ports[port].type == 'Uses':
              name = self.Ports[port].name
              int_type = self.Ports[port].interface.nameSpace+':'+self.Ports[port].interface.name
              op_list = []
              operation_str = 'void connectPort(in CORBA::ObjPtr Ref, in string name)'
              op_list.append(operation_str)
              operation_str = 'void disconnectPort(in string name)'
              op_list.append(operation_str)
              port_list = []
              port_list.append(self.Ports[port].type)
              port_list.append(int_type)
              port_list.append(op_list)
              self.API[name]=port_list
          elif self.Ports[port].type == 'Provides':
              name = self.Ports[port].name
              int_type = self.Ports[port].interface.nameSpace+':'+self.Ports[port].interface.name
              op_list = []
              for op in self.Ports[port].interface.operations:
                  operation_str = op.returnType + ' ' + op.name+'('
                  param_str = ''
                  for param in op.params:
                      param_str += param.direction + ' ' + param.dataType + ' ' + param.name + ', '
                  operation_str += param_str[:-2] + ')'
                  op_list.append(operation_str)
              port_list = []
              port_list.append(self.Ports[port].type)
              port_list.append(int_type)
              port_list.append(op_list)
              self.API[name]=port_list
          else:
              print "Invalid port direction descriptor in " + self.name
              continue
  
  def __getitem__(self,i):
      """Return a list of the connections the component has (obsolete)"""
      return self.connections[i]
  
  def setUUID(self):
      """Modify the UUID for the component (use of this function is likely to cause negative effects)"""
      self.uuid = uuidgen()
      
  def changeName(self,newname):
      """Modify the component's name (use of this function is likely to cause negative effects)"""
      self.name = newname
      if self.generate == True:
          self.baseName = newname
          self.xmlName = newname
  
  def getPorts(self):
      """Return a dictionary of all Component Ports"""
      return self.Ports

  def queryProperties(self):
      """Return a dictionary of the properties and values."""
      
      if self.profile == '':
          print "Unable to query properties for " + self.name + " - profile unavailable"
          return None

      if self.ref == None:
          print 'No reference to component <' + self.name + '>'
          return None

      os.chdir(self.root)

      # Get prf_path if necessary
      if self.prf_path == '':
          doc_spd = xml.dom.minidom.parse(self.profile)
          localfiles = doc_spd.getElementsByTagName('localfile')
          for localfile in localfiles:
              if localfile.parentNode.tagName == 'propertyfile':
                  self.prf_path = os.path.join(self.root,localfile.getAttribute('name'))
              if localfile.parentNode.tagName == 'descriptor':
                  self.scd_path = os.path.join(self.root,localfile.getAttribute('name'))

      # Parse the properties file
      try:
          doc_prf = xml.dom.minidom.parse(self.prf_path)
      except ExpatError, msg:
          print "Error reading <" + self.prf_path + ">",
          print msg
          return None

      props_tag = doc_prf.documentElement

      properties_data = {}
            
      for prop_tag in props_tag.childNodes:
          if prop_tag.nodeType != prop_tag.ELEMENT_NODE: continue
         
          if prop_tag.tagName in ['simple', 'simplesequence'] and prop_tag.attributes != None and prop_tag.attributes.length > 0:
              tmp_data = {'id':'','name':'','mode':'','type':'','value':None,'kindtype':'','description':''}
              for i in range(prop_tag.attributes.length):
                  tmpname = str(prop_tag.attributes.item(i).nodeName)
                  tmpval = str(prop_tag.attributes.item(i).nodeValue)
                  if tmpname in tmp_data: tmp_data[tmpname] = tmpval

              if not prop_tag.hasChildNodes():
                  continue

              for prop_data in prop_tag.childNodes:
                  if prop_data.nodeType != prop_data.ELEMENT_NODE: continue

                  if prop_data.tagName == 'description' and prop_data.hasChildNodes():
                      tmp_data['description'] = str(prop_data.firstChild.data)
                  elif prop_data.tagName == 'kind':
                      if prop_data.attributes != None and prop_data.attributes.length > 0:
                          for i in range(prop_data.attributes.length):
                              tmpname = str(prop_data.attributes.item(i).nodeName)
                              tmpval = str(prop_data.attributes.item(i).nodeValue)
                              if tmpname == 'kindtype': tmp_data[tmpname] = tmpval
              
              if tmp_data['id']:
                  tmp_id = tmp_data.pop('id')
                  properties_data[tmp_id] = tmp_data

      # clean up doc
      doc_prf.unlink()

      # Now preform the query and fill in value data
      query_props = self.ref.query([])

      for qprop in query_props:
          if qprop.id in properties_data:
              properties_data[qprop.id]['value'] = qprop.value.value()

      return properties_data


class Waveform(object):
    """This is the basic descriptor for a waveform (collection of inter-connected Components)

       Relevant member data:
       app - Pointer to the Application (SCA core framework) object
       Components - dictionary for the different Component instances making up the waveform
       ns_name - the unique naming service name for this waveform
       name - the (non-unique) name for this waveform
       
       Waveform overview:
       
       A waveform is defined by an XML file (<waveform name>.sad.xml) that resides in a waveform
       directory, usually /sdr/sca/waveforms or /sdr/waveforms. This XML file lists a series of
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
       
       To access a specific component, you need its name. To get a list of the components in
        the waveform, type
        
        obj.Components
        
       This member returns a dictionary of all components in the waveform. To access the specific
        component, just access the specific entry where the component's name is the key
        in the returned dictionary.
       
    """
    def __init__(self, name="", int_list=None, domain=None):
        self.name = name
        self.components = []
        self.Components = {}
        self.devices = []
        self.ace = False
        self.ns_name = ''
        self.app = None
        self.interface_list = int_list
        self.domain = domain

        if self.domain == None:
            orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
            obj = orb.resolve_initial_references("NameService")
            self.rootContext = obj._narrow(CosNaming.NamingContext)
            if os.path.exists('/sdr/sca'):
                self.sdrroot = '/sdr/sca'
            elif os.path.exists('/sdr'):
                self.sdrroot = '/sdr'
            else:
                self.sdrroot = ''
        else:
            self.rootContext = self.domain.rootContext
            self.sdrroot = self.domain.root

    def update(self):

        prof_path = self.app._get_profile()
        if os.path.isabs(prof_path):
            prof_path = prof_path[1:]
        abs_prof_path = os.path.join(self.sdrroot, prof_path)
          
        doc_sad = xml.dom.minidom.parse(abs_prof_path)
        comp_list = self.app._get_componentNamingContexts()
        waveform_ns_name = ''
        if len(comp_list) > 0:
            comp_ns_name = comp_list[0].elementId
            waveform_ns_name = comp_ns_name.split('/')[1]

        app_name = self.app._get_name()

        self.populateComponents(comp_list, doc_sad)

    def populateComponents(self, component_list, in_doc_sad):
        """component_list is a list of component names
           in_doc_sad is a parsed version of the SAD (using xml.dom.minidom)"""
        if in_doc_sad == None:
            return

        spd_list = {}
        for compfile in in_doc_sad.getElementsByTagName('componentfile'):
            spd_list[compfile.getAttribute('id')] = compfile.getElementsByTagName('localfile')[0].getAttribute('name')

        dce_list = {}
        for compplac in in_doc_sad.getElementsByTagName('componentplacement'):
            dce_list[compplac.getElementsByTagName('componentinstantiation')[0].getAttribute('id')] = compplac.getElementsByTagName('componentfileref')[0].getAttribute('refid')

        for comp_entry in component_list:
            new_comp = Component(int_list=self.interface_list)
            new_comp.name = comp_entry.elementId.split('/')[2]
            new_comp.pointer = comp_entry
            new_comp.uuid = comp_entry.componentId
            new_comp.file_uuid = ""
            ns_name = comp_entry.elementId.split('/')

            full_name = [CosNaming.NameComponent(ns_name[0],""),
                CosNaming.NameComponent(ns_name[1],""),
                CosNaming.NameComponent(ns_name[2],"")]

            found_object = False
            find_object_attempts = 0
            while not found_object and find_object_attempts < 50:
                try:
                    obj = self.rootContext.resolve(full_name)
                    found_object = True
                except:
                    time.sleep(0.1)
                    find_object_attempts += 1
            
            if find_object_attempts == 50:
                print "I was unable to get the pointer to Component "+ns_name[0]+"/"+ns_name[1]+"/"+ns_name[2]+", it is probably not running"
            else:
                new_comp.reference = obj._narrow(CF.Resource)
                new_comp.ref = new_comp.reference

                new_comp.root = self.sdrroot

                if not dce_list.has_key(new_comp.uuid):
                    print "Component descriptor error - unmatched Component DCE"
                    continue

                new_comp.profile = spd_list[dce_list[new_comp.uuid]]

                new_comp.populatePorts()

            self.components.append(new_comp)
            self.Components[new_comp.name]=new_comp
    
    def __getitem__(self,i):
        """Return the component with the given index (obsolete)"""
        return self.components[i]
    
    def getComponents(self):
        """Return a dictionary of all Components in the Waveform"""
        return self.Components

        
class Node(object):
  """The Node is a descriptor for an logical grouping of devices.

       Relevant member data:
       name - Node's name

  """
  def __init__(self, name="", path="", generate=True, int_list=None):
    self.name = name
    self.path = path
    self.Devices = []
    self.type = "node"
    self.generate = generate
    self.id = ""
    
  def addDevice(self, in_dev=None):
    if in_dev != None:
      self.Devices.append(in_dev)

        
class Domain(object):
  """The Domain is a descriptor for an already-running nodeBooter process (to start
      a nodeBooter, either run it from a command window or call start_node from the
      ossie module).

       Relevant member data:
       installed_waveforms - Dictionary of all waveforms installed on this node
       name - Domain name

     The main functionality that can be exercised by this class is:
      - terminate - uninstalls all running waveforms and terminates the node
      - waveform management:
          - getAvailableWaveforms - returns a list of all waveforms that this node can install
          - getInstalledWaveforms - returns a dictionary of all waveforms that are currently running. Calling this function
              automatically updated the list of installed waveforms
          - installWaveform - install a particular waveform
          - uninstallWaveform - uninstall a particular waveform
          - update_waveform_list - because nodeBooter is a separate service, other programs
              might install/uninstall waveforms on the node. This function updates the node descriptor's
              internal list
  """
  def __init__(self, name="DomainName1", int_list=None, location=None):
    self.name = name
    self.Devices = []
    self.Waveforms = {}
    self.DomainManager = None
    self.NodeAlive = True
    self.Nodes = []
    self.location = location
    
    # create orb reference
    input_arguments = sys.argv
    if location != None:
        if len(sys.argv) == 1:
            if sys.argv[0] == '':
                input_arguments = ['-ORBInitRef','NameService=corbaname::'+location]
            else:
                input_arguments.append('-ORBInitRef','NameService=corbaname::'+location)
        else:
            input_arguments.append('-ORBInitRef','NameService=corbaname::'+location)
        
    self.orb = CORBA.ORB_init(input_arguments, CORBA.ORB_ID)
    obj = self.orb.resolve_initial_references("NameService")
    self.rootContext = obj._narrow(CosNaming.NamingContext)
    # get DomainManager reference
    dm_name = [CosNaming.NameComponent(self.name,""),CosNaming.NameComponent(self.name,"")]
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
            time.sleep(0.1)
            domain_find_attempts += 1

    if domain_find_attempts == 30:
        print "Did not find the domain"
        return
    #else:
    #    print "found the domain"
            
    self.DomainManager = obj._narrow(CF.DomainManager)
    # get poa and poaManager reference
    
    # check to make sure that all the devices are installed
    devmgr_find_attempts = 0;
    found_devMgrs = False
    second_try = False
    while not found_devMgrs and devmgr_find_attempts < 30:
       try:
           if second_try:
               obj = self.rootContext.resolve(dm_name)
               self.DomainManager = obj._narrow(CF.DomainManager)
           devMgrs = self.DomainManager._get_deviceManagers()
           found_devMgrs = True
       except:
           devmgr_find_attempts += 1
           time.sleep(0.1)
           second_try = True
    
    if devmgr_find_attempts == 30:
        return

    for devmgr in range(len(devMgrs)):
        devMgr = devMgrs[devmgr]
        curr_devSeq = devMgr._get_registeredDevices()
        for dev in range(len(curr_devSeq)):
            curr_dev = curr_devSeq[dev]
            dev_label = str(curr_dev._get_label())
            dev_name = [CosNaming.NameComponent(self.name,""),CosNaming.NameComponent(dev_label,"")]
            found_device = False
            while not found_device:
                try:
                    obj = self.rootContext.resolve(dev_name)
                    found_device = True
                except:
                    time.sleep(0.1)
            
    
    if int_list == None:
        self.interface_list = importIDL.importStandardIdl()
    else:
        self.interface_list = int_list
        
    # find and store the sdr root
    self.root = findSdrRoot()
    if not self.root: self.root = None
      
    # update information
    self.updateListAvailableWaveforms()
    self.updateInstalledWaveforms(shallow=True)
    
  
  def __del__(self):
    """
        Destructor
    """
    pass
  
  def terminate(self):
    """
        Terminate the Domain including the Node(s).
    """
    if not self.NodeAlive:
        return

    # uninstall waveforms
    if self.Waveforms != {}:
        for waveform_name in self.Waveforms:
            #waveform = self.Waveforms.pop(waveform_name)
            waveform = self.Waveforms[waveform_name]
            waveform.app.releaseObject()
            
        self.Waveforms = {}
            
    # uninstall devices
    for device_entry in self.Devices:
        if device_entry.reference != None:
            device_entry.reference.releaseObject()

    # clean up the node
    os.system('pkill nodeBooter')
    self.NodeAlive = False
    dmn_ctx = [CosNaming.NameComponent(self.name,"")]
    self.rootContext.unbind(dmn_ctx)

    
  def addDevice(self, in_dev=None):
    """
        Add a device to the Domain.
    """
    if in_dev != None:
      self.Devices.append(in_dev)
  
  def addNode(self, in_node=None):
      if in_node != None:
          self.Nodes.append(in_node)
          for device in in_node.Devices:
              self.Devices.append(device)

  def updateListAvailableWaveforms(self):
      """
          Update available waveforms list.
      """
      waveroot = os.path.join(self.root, 'waveforms')    
      if not os.path.exists(waveroot):
          print "Cannot find SDR waveforms directory"
          #return {}
          return

      self.waveforms = {}            
      for wave_dir in os.listdir(waveroot):
          wave_dir_path = os.path.join(waveroot,wave_dir)
          if not os.path.isdir(wave_dir_path):
              continue

          for wave_file in os.listdir(wave_dir_path):
              if ".sad.xml" in wave_file.lower():
                  f_path = os.path.join('waveforms', wave_dir)
                  f_path = os.path.join(f_path, wave_file)
                  if wave_dir not in self.waveforms:
                      self.waveforms[wave_dir] = f_path

  def getAvailableWaveforms(self, domain_name="DomainName1"):
      """List the waveforms that are available to install"""
      return self.waveforms.keys()

  def getInstalledWaveforms(self, domain_name="DomainName1"):
      """Dictionary of the waveforms that are currently installed"""
      self.updateInstalledWaveforms()
      return self.Waveforms
  
  def uninstallWaveform(self, waveform_name=''):
      """Uninstall a running waveform"""
      if not self.Waveforms.has_key(waveform_name):
          print "The waveform described for uninstall does not exist"
          return
      waveform = self.Waveforms.pop(waveform_name)
      waveform.app.releaseObject()

  def installWaveform(self, waveform_name='', domain_name="DomainName1"):
      """Install and create a particular waveform. This function returns
         a pointer to the instantiated waveform"""
      waveform_list = self.waveforms
      if not waveform_list.has_key(waveform_name):
          print "Requested waveform does not exist"
          return
      self.DomainManager.installApplication(waveform_list[waveform_name])

      self.doc_sad = xml.dom.minidom.parse(os.path.join(self.root,waveform_list[waveform_name]))

      # get a list of all the component instance id's
      component_list = []
      for component_placement in self.doc_sad.getElementsByTagName('componentinstantiation'):
          component_list.append(str(component_placement.getAttribute('id')))

      # Specify what component should be deployed on particular devices
      _devSeq = []
      sad_file = os.path.join(self.root, waveform_list[waveform_name])
      DAS_file = sad_file.replace('.sad','_DAS')
      tmp_devSeq = buildDevSeq(DAS_file)
      for x in range(len(tmp_devSeq)):
          _devSeq.append(tmp_devSeq[x])

      number_device_matches = len(_devSeq)
      matches_found = 0

      devMgrSeq = self.DomainManager._get_deviceManagers()
      available_dev_seq = []
      for devmgr in range(len(devMgrSeq)):
          devMgr = devMgrSeq[devmgr]
          curr_devSeq = devMgr._get_registeredDevices()
          for dev in range(len(curr_devSeq)):
              curr_dev = curr_devSeq[dev]
              available_dev_seq.append(curr_dev._get_identifier())

      _available_devSeq = []
      for x in range(len(component_list)):
          found_dev_in_devSeq = False
          for search_dev in range(len(_devSeq)):
              if _devSeq[search_dev].componentId == component_list[x]:
                  for check_list in range(len(available_dev_seq)):
                      if available_dev_seq[check_list]==_devSeq[search_dev].assignedDeviceId:
                          matches_found = matches_found + 1
                          _available_devSeq.append(_devSeq[search_dev])
                          break

      if matches_found != len(component_list):
          print "At least one device required for this waveform is missing - aborting install"
          return

      
      # get a list of the application factories in the Domain
      _applicationFactories = self.DomainManager._get_applicationFactories()

      # find the application factory that is needed
      app_name = str(self.doc_sad.getElementsByTagName('softwareassembly')[0].getAttribute('name'))
      app_factory_num = -1
      for app_num in range(len(_applicationFactories)):
          if _applicationFactories[app_num]._get_name()==app_name:
              app_factory_num = app_num
              break

      if app_factory_num == -1:
          print "Application factory not found"
          sys.exit(-1)

      _appFacProps = []

      try:
          app = _applicationFactories[app_factory_num].create(_applicationFactories[app_factory_num]._get_name(),_appFacProps,_available_devSeq)
      except:
          print "Unable to create application - make sure that all appropriate nodes are installed"
          return
      
      comp_list = app._get_componentNamingContexts()
      waveform_ns_name = ''
      if len(comp_list) > 0:
          comp_ns_name = comp_list[0].elementId
          waveform_ns_name = comp_ns_name.split('/')[1]
      
      waveform_entry = Waveform(name=waveform_name, int_list=self.interface_list, domain=self)
      waveform_entry.app = app
      waveform_entry.ns_name = waveform_ns_name
      waveform_entry.populateComponents(comp_list, self.doc_sad)
      
      self.Waveforms[waveform_ns_name]=waveform_entry

      return waveform_entry

  def updateInstalledWaveforms(self, shallow=False):
      """Makes sure that the dictionary of waveforms is up-to-date"""
      try:
          app_list = self.DomainManager._get_applications()
      except:
          return

      app_name_list = []

      for app in app_list:
          prof_path = app._get_profile()
          if os.path.isabs(prof_path):
            prof_path = prof_path[1:]
          abs_prof_path = os.path.join(self.root, prof_path)
          
          doc_sad = xml.dom.minidom.parse(abs_prof_path)
          comp_list = app._get_componentNamingContexts()
          waveform_ns_name = ''
          if len(comp_list) > 0:
              comp_ns_name = comp_list[0].elementId
              waveform_ns_name = comp_ns_name.split('/')[1]

          app_name_list.append(waveform_ns_name)
          
          if self.Waveforms.has_key(waveform_ns_name):
              self.Waveforms[waveform_ns_name].app = app
              continue

          app_name = app._get_name()
          if app_name[:7]=='OSSIE::':
              waveform_name = app_name[7:]
          else:
              waveform_name = app_name
          waveform_entry = Waveform(name=waveform_name, int_list=self.interface_list, domain=self)
          waveform_entry.app = app
          waveform_entry.ns_name = waveform_ns_name
          if not shallow:
              waveform_entry.populateComponents(comp_list, doc_sad)

          self.Waveforms[waveform_ns_name]=waveform_entry
          self.Waveforms[waveform_ns_name].update()
      
      for waveform_key in self.Waveforms.keys():
          if waveform_key not in app_name_list:
              tmp = self.Waveforms.pop(waveform_key)

class Platform:
  def __init__(self, name=""):
    self.name = name
    self.nodes = []      
      
class Port(object):
  """The Port is the gateway into and out of a particular component. A Port has a string name that is unique
      to that port within the context of a particular component.
     
     There are two types of Ports: Uses (output) and Provides (input).
     
     To access a Provides Port, make the appropriate function call on the Port's ref member. The Port is a
      server and will service this new request.
      
     To access a Uses Port, the port must be informed of the location and name of a server object. The server
      object must match the interface type that the port supports. The Uses Port is then given a reference
      to the server object and a name for the new connection - this name must be unique. We recommend that you
      use uuidgen for that name.
     
      When the server object supporting the Uses Port is destroyed, the Uses Port must be informed that the
      server no longer exists. You do this by calling the Uses Port's disconnectPort function. The argument
      for disconnectPort is the unique string used to create the connection.

       Relevant member data:
       name - Port's name
       interface - IDL interface for this port
       type - Uses (output) or Provides (input)
       generic_ref - CORBA generic pointer
       ref - Interface-specific CORBA pointer to port (useful in most cases)
  """
  def __init__(self, name, interface, type="Uses",portType="data"):
    self.name = name
    self.interface = interface
    self.portType = portType    #control or data
    self.type = type            #Uses or Provides
    self.u_cname = "dataOut_" + interface.name + "_i"
    self.p_cname = "dataIn_" + interface.name + "_i"
    if type == "Uses":
        self.cname = self.u_cname
    if type == "Provides":
        self.cname = self.p_cname
    self.generic_ref = None
    self.ref = None

class Connection:
    def __init__(self, LP, RP, RC):
        self.localPort = LP
        self.remotePort = RP 
        self.remoteComp = RC
        
class Interface:
    def __init__(self,name,nameSpace="",operations=[],filename="",fullpath=""):
        self.name = name
        self.nameSpace = nameSpace
        self.operations = []
        self.filename = filename    #does not include the '.idl' suffix
        self.fullpath = fullpath
        self.inherited_names = []
        self.inherited = []
        
    def __eq__(self,other):
        if isinstance(other, Interface):        
            return (other.nameSpace == self.nameSpace ) and (other.name == self.name)
        else:
            return False
        
    def __ne__(self,other):
        if isinstance(other, Interface):
            return (other.nameSpace != self.nameSpace ) and (other.name != self.name)
        else:
            return True

        
class Operation:
    def __init__(self,name,returnType,params=[]):
        self.name = name
        self.returnType = returnType
        self.cxxReturnType = ''
        self.params = []
        
class Param:
    def __init__(self,name,dataType='',direction=''):
        """ 
        Exampleinterface complexShort {
            void pushPacket(in PortTypes::ShortSequence I, in PortTypes::ShortSequence Q);
        };
        """

        self.name = name            # The actual argument name: 'I'
        self.dataType = dataType    # The type of the argument: 'PortTypes::ShortSequence'
        self.cxxType = ""
        self.direction = direction  # Flow of data: 'in'
        
class Property:
    def __init__(self,elementType,name,mode,description='',id=''):
        self.elementType = elementType
        self.name = name
        self.mode = mode
        self.force_overload = False
        if len(id) == 0:
            self.id = 'DCE:' + uuidgen()
        else:
            self.id = id
            
        
class SimpleProperty(Property):
    def __init__(self,name,mode,type,description='',value='',units=None,
                range=(-1,-1),enum='',kind='configure',action=None,id=''):
        Property.__init__(self,"Simple",name,mode,description,id)
        self.type = type
        self.description = description   
        self.value = value
        self.defaultValue = value
        self.units = units
        self.range = range
        self.enum = enum
        self.kind = kind
        self.action = action

class SimpleSequenceProperty(Property):
    def __init__(self,name,mode,type,description='',values=[],units=None,
                range=(-1,-1),kind='configure',action=None,id=''):
        Property.__init__(self,"SimpleSequence",name,mode,description,id)
        self.type = type
        self.description = description
        self.values = values # list of tuples of the form (value, defaultValue)
        self.units = units
        self.range = range
        self.kind = kind
        self.action = action
    

#########################################################################
#builds the device assigment sequence from the DAS xml file
def buildDevSeq(dasXML):
    das = xml.dom.minidom.parse(dasXML)
    ds = []
    for x in das.getElementsByTagName('deploymentenforcement')[0].getElementsByTagName('deviceassignmentsequence')[0].getElementsByTagName('deviceassignmenttype'):
        #print "component id:", x.componentid
        #print "assignmentdevid:", x.assigndeviceid
        compid = x.getElementsByTagName('componentid')[0].childNodes[0].data
        assgnid = x.getElementsByTagName('assigndeviceid')[0].childNodes[0].data
        ds.append(CF.DeviceAssignmentType(str(compid),str(assgnid)))
    return ds
