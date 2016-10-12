# -*- coding: utf-8 -*-
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


import sys
import os
from qt import *
import logging

from xml.dom import minidom

from omniORB import CORBA, any
from ossie.cf import CF

from ossie.utils import redhawk

from properties import *
import application as application
from browsewindowbase import BrowseWindowBase
import installdialog as installdialog
import propertydialog as propertydialog

def buildDevSeq(dasXML, fs):
    _xmlFile = fs.open(str(dasXML), True)
    buff = _xmlFile.read(_xmlFile.sizeOf())
    _xmlFile.close()

    das = minidom.parseString(buff)
    ds = []
    deviceAssignmentTypeNodeList = das.getElementsByTagName("deviceassignmenttype")
    for node in deviceAssignmentTypeNodeList:
        componentid = node.getElementsByTagName("componentid")[0].firstChild.data
        assigndeviceid = node.getElementsByTagName("assigndeviceid")[0].firstChild.data
        ds.append( CF.DeviceAssignmentType(str(componentid),str(assigndeviceid)) )
    return ds


def clearChildren (item):
    while item.firstChild():
        item.takeItem(item.firstChild())


TYPE_MAP = { "boolean"  : CORBA.TC_boolean,
             "char"     : CORBA.TC_char,
             "double"   : CORBA.TC_double,
             "float"    : CORBA.TC_float,
             "short"    : CORBA.TC_short,
             "long"     : CORBA.TC_long,
             "longlong" : CORBA.TC_longlong,
             "objref"   : CORBA.TC_Object,
             "octet"    : CORBA.TC_octet,
             "string"   : CORBA.TC_string,
             "ulong"    : CORBA.TC_ulong,
             "ushort"   : CORBA.TC_ushort }

def toAny (value, type):
    if type in ( "short", "long", "octet", "ulong", "ushort" ):
        value = int(value)
    elif type in ( "float", "double" ):
        value = float(value)
    elif type == "boolean":
        value = str(value.lower()) in ( "true", "1" )
    elif type == "longlong":
        value = long(value)
    elif type == "char":
        value = value[0]
    return CORBA.Any(TYPE_MAP[type], value)


class BrowseWindow(BrowseWindowBase):

    def __init__(self, domainName=None, verbose=False,
                 parent = None,name = None,fl = 0):
        self.verbose = verbose
        self.currentDomain = ""
        BrowseWindowBase.__init__(self,parent,name,fl)
        
        self.log = logging.getLogger(BrowseWindow.__name__)

        if not domainName:
            domainName = self.findDomains()
            if not domainName:
                return
        self.debug('Domain:', domainName)
        self.updateDomain(domainName)
        
    def findDomains(self):
        domainList = redhawk.scan()
        stringlist = QStringList()
        for entry in domainList:
            stringlist.append(QString(entry))
        retval = QInputDialog.getItem(QString("pick a domain"), QString("domains"), stringlist, 0, False, self)
        if retval[1] == False:
            return None
        return str(retval[0])
        
    def updateDomain(self, domainName):
        try:
            self.currentDomain = domainName
            self.domManager = redhawk.attach(domainName)

            if self.domManager is None:
                raise StandardError('Could not narrow Domain Manager')

            # this is here just to make sure that the pointer is accessible
            if self.domManager._non_existent():
                raise StandardError("Unable to access the Domain Manager in naming context "+domainName)
        except:
            raise StandardError("Unable to access the Domain Manager in naming context "+domainName)

        try:
            self.fileMgr = self.domManager._get_fileMgr()
        except:
            raise StandardError("Unable to access the File Manager in Domain Manager "+domainName)

        self.domainLabel.setText(self.currentDomain + ' ' + self.domManager._get_identifier())

        self.objectListView.setSortColumn(2)

        self.domMgrItem = QListViewItem(self.objectListView, 'Domain Manager', "", '0')
        self.appFactoryItem = QListViewItem(self.objectListView, 'Application Factories', "", '1')
        self.appsItem = QListViewItem(self.objectListView, 'Applications', "", '2')
        self.devManItem = QListViewItem(self.objectListView, 'Device Managers', "", '3')

        self.parseDomain()

    def refreshView (self):
        if self.currentDomain == "":
            domainName = self.findDomains()
            if not domainName:
                return
            self.updateDomain(domainName)

        self.refreshDomainManager()
        self.refreshApplicationFactories()
        self.refreshApplications()
        self.refreshDeviceManagers()


    def refreshApplications (self):
        clearChildren(self.appsItem)
        self.parseApplications()


    def refreshApplicationFactories (self):
        clearChildren(self.appFactoryItem)
        self.parseApplicationFactories()


    def refreshDeviceManagers (self):
        clearChildren(self.devManItem)
        self.parseAllDeviceManagers()


    def getAttribute (self, item, attribute):
        child = item.firstChild()
        while child:
            if child.text(0) == attribute:
                return str(child.text(1))
            child = child.nextSibling()
        return None


    def propertyChanged (self, item):
        # First, make sure a property changed, then find the owning application
        # (note the this wouldn't work for device properties).
        parent = item.parent()
        if not parent or parent.text(0) != 'Properties':
            return
        parent = parent.parent()
        if not parent:
            return

        # NB: Using the hidden property id to set the property; may want a better
        #     lookup method.
        id = str(item.text(3))
        value = str(item.text(1))
        try:
            value = eval(value)
        except:
            pass
        
        if parent.text(0) == 'Domain Manager':
            propset = self.domManager
            value = toAny(value, self.domMgrProps[id]['type'])
        else:
            element = self.getAttribute(parent, 'Naming Element')
            if not element:
                return
            propset = lookupPropertySet(self.rootContext, element)
            value = any.to_any(value)
        
        try:
            propset.configure([CF.DataType(id, value)])
        except:
            pass


    def selectionChanged (self, item):
        parent = item.parent()
        for button in [ self.uninstallButton, self.createButton ]:
            button.setEnabled(parent == self.appFactoryItem)

        for button in [ self.releaseButton, self.startButton, self.stopButton ]:
            button.setEnabled(parent == self.appsItem)


    def uninstallSelected (self):
        item = self.objectListView.currentItem()
        if not item or item.parent() != self.appFactoryItem:
            return
        id = self.getAttribute(item, 'Identifier')
        if id:
            self.domManager.uninstallApplication(id)
            self.refreshApplicationFactories()


    def installApplication (self):
        try:
            appList = [ app.name for app in self.fileMgr.list('/waveforms/*') ]
        except CORBA.TRANSIENT:
            QMessageBox.critical(self, 'File Error', 'Unable to get list of SAD XML files for this domain.', QMessageBox.Ok)
            return

        # Present the app list in alphabetical order.
        appList.sort()
        app = installdialog.getApplicationFile(appList, self)
        if app:
            self.domManager.installApplication(app)
            self.refreshApplicationFactories()


    def findApplication (self, id, name):
        for app in self.domManager._get_applications():
            if app._get_identifier() == id:
                return app
        return None
        

    def findApplicationFactory (self, id):
        for appFactory in self.domManager._get_applicationFactories():
            if appFactory._get_identifier() == id:
                return appFactory
        return None
        

    def createSelected (self):
        item = self.objectListView.currentItem()
        if not item or item.parent() != self.appFactoryItem:
            return

        appFactory = self.findApplicationFactory(self.getAttribute(item, 'Identifier'))
        if not appFactory:
            return
            
        profile = appFactory._get_softwareProfile()
        dasFile = profile[:-8]+"_DAS.xml"
        if not self.fileMgr.exists(dasFile):
            dasFile = None

        #startDir = os.path.join(XML_DIR, 'waveforms')
        #dasFile = QFileDialog.getOpenFileName(startDir, 'DAS files (*_DAS.xml)', self, 'open file dialog', 'Open a DAS file')
        if dasFile:
            retval = QMessageBox.question(self, "DAS File Found", "Found DAS file matching requested Application.\nShould this file be used?", QMessageBox.Yes, QMessageBox.No)
            if retval == QMessageBox.No:
                dasFile = None

        if dasFile:
            # Parse the DAS file; failure prevents application creation.
            try:
                devSeq = buildDevSeq(str(dasFile), self.fileMgr)
            except:
                QMessageBox.critical(self, 'Create Failed', 'Could not read DAS file.', QMessageBox.Ok)
                return
        else:
            devSeq = []

        # Parse the software profile to get the property file for the
        # assembly controller. These will be the default properties.
        profile = self.getAttribute(item, 'Software Profile')
        af = application.getApplicationFactoryFromSAD(profile, self.fileMgr)
        properties = af.getProperties()

        def filterProp(prop):
            if "configure" in prop["kinds"]:
                return True
            if "execparam" in prop["kinds"]:
                return True
            return False

        properties = filter(filterProp, properties)

        if properties:
            ok, properties = propertydialog.showPropertyEditor(properties, self)
            if not ok:
                return

            # Convert the properties-as-dictionaries to OSSIE Properties.
            def convertProperty (property):
                if not property.has_key('value'):
                    return None
                if property['elementType'] == 'simple':
                    value = toAny(property['value'], property['type'])
                else:
                    value = any.to_any(property['values'])
                return CF.DataType(id=property['id'], value=value)

            try:
                properties = []#map(convertProperty, properties)
            except:
                QMessageBox.critical(self, "Invalid property", "%s:\n%s" % sys.exc_info()[0:2], QMessageBox.Ok)
                return
            
            pop_list = []
            for idx in range(len(properties)):
                if properties[idx] == None:
                    pop_list.append(idx)
            
            while (len(pop_list)>0):
                properties.pop(pop_list.pop())

        
        try:
            self.debug('Properties:', properties)
            self.debug('DAS:', devSeq)
            app = appFactory.create(appFactory._get_name(), properties, devSeq)
        except CF.ApplicationFactory.CreateApplicationError, e:
            QMessageBox.critical(self, 'Creation of waveform failed.', e.msg, QMessageBox.Ok)
            return

        self.refreshApplications()


    def releaseSelected (self):
        item = self.objectListView.currentItem()
        if not item or item.parent() != self.appsItem:
            return
        app = self.findApplication(self.getAttribute(item, 'Identifier'), self.getAttribute(item, 'Name'))
        if app:
            try:
                self.debug('Release', app._get_identifier())
                app.releaseObject()
            except:
                QMessageBox.critical(self, 'Release Failed', 'Release of waveform failed.', QMessageBox.Ok)
            else:
                self.refreshApplications()


    def startSelected (self):
        item = self.objectListView.currentItem()
        if not item or item.parent() != self.appsItem:
            return
        app = self.findApplication(self.getAttribute(item, 'Identifier'), self.getAttribute(item, 'Name'))
        if app:
            try:
                app.start()
            except:
                pass


    def stopSelected (self):
        item = self.objectListView.currentItem()
        if not item or item.parent() != self.appsItem:
            return
        app = self.findApplication(self.getAttribute(item, 'Identifier'), self.getAttribute(item, 'Name'))
        if app:
            try:
                app.stop()
            except:
                pass


    def parseApplicationFactories (self):
        # Build the list of application factories.
        for appFactory in self.domManager._get_applicationFactories():
            id = appFactory._get_identifier()
            name = appFactory._get_name()
            item = QListViewItem(self.appFactoryItem, name)
            QListViewItem(item, 'Identifier', id)
            QListViewItem(item, 'Software Profile', appFactory._get_softwareProfile())


    def parseApplications (self):
        # Build the list of installed applications.
        fs = self.domManager._get_fileMgr()
        for app in self.domManager._get_applications():
            id = app._get_identifier()
            name = app._get_name()
            for namingContext in app._get_componentNamingContexts():
                nc = namingContext.elementId
                ncSet = nc.split("/")
                if len(ncSet) == 3:
                    name = ncSet[1]
                    break
            appItem = QListViewItem(self.appsItem, name)
            QListViewItem(appItem, 'Identifier', id, '1')
            QListViewItem(appItem, 'Profile', app._get_profile(), '2')
            QListViewItem(appItem, 'Name', name, '3')

            # Coalesce all of the component information into one data structure
            components = {}
            for comp in app._get_registeredComponents():
                components[comp.identifier] = {}
                components[comp.identifier]['name'] = comp.softwareProfile.split('/')[-1].split('.spd.xml')[0]
                components[comp.identifier]['namingElement'] = app._get_name()
                components[comp.identifier]['ref'] = comp.componentObject
                components[comp.identifier]['propertyDefs'] = getPropertiesForApplication(app._get_profile())
                try:
                    components[comp.identifier]['properties'] = comp.componentObject.query([])
                except:
                    components[comp.identifier]['properties'] = []

            for componentPid in app._get_componentProcessIds():
                components[componentPid.componentId]['pid'] = componentPid.processId

            for device in app._get_componentDevices():
                component = components[device.componentId]
                if 'devices' not in component:
                    component['devices'] = []
                component['devices'].append(device.assignedDeviceId)

            for impl in app._get_componentImplementations():
                components[impl.componentId]['implementation'] = impl.elementId

            componentItem = QListViewItem(appItem, 'Components')
            for id, component in components.items():
                item = QListViewItem(componentItem, component['name'])
                QListViewItem(item, 'Identifier', id, '1')
                QListViewItem(item, 'PID', str(component['pid']), '2')
                if len(component['devices']) > 1:
                    devItem = QListViewItem(item, 'Devices', '', '3')
                    for index, dev in enumerate(component['devices']):
                        QListViewItem(devItem, str(index), dev)
                else:
                    QListViewItem(item, 'Device', component['devices'][0], '3')
                QListViewItem(item, 'Naming Element', component['namingElement'], '4')
                if len(component['properties']) == 0:
                    continue
                propItem = QListViewItem(item, 'Properties', '', '5')
                self.buildPropertiesListView(propItem, component['properties'], component['propertyDefs'])

    def parseAllDeviceManagers (self):
        # Build the list of Device Managers.
        for devMan in self.domManager._get_deviceManagers():
            try:
                self.parseDeviceManager(devMan)
            except CORBA.TRANSIENT:
                self.log.warning('Failure querying DeviceManager')
                continue

    def parseDeviceManager (self, devMan):
        id = devMan._get_identifier()
        label = devMan._get_label()

        # Create a node for the Device Manager, with its attributes and devices as children.
        item = QListViewItem(self.devManItem, label)
        QListViewItem(item, 'Identifier', id, '1')
        QListViewItem(item, 'Configuration', devMan._get_deviceConfigurationProfile(), '2')
        propItem = QListViewItem(item, 'Properties', '', '3')
        deviceItem = QListViewItem(item, 'Devices', '', '4')
        serviceItem = QListViewItem(item, 'Services', '', '5')

        devmanFS = devMan._get_fileSys()

        # Read the DCD file to get the SPD file, which can then be used to get the properties.
        _xmlFile = devmanFS.open(str(devMan._get_deviceConfigurationProfile()), True)
        dcd = minidom.parseString(_xmlFile.read(_xmlFile.sizeOf()))      
        _xmlFile.close()
        spdFile = dcd.getElementsByTagName('localfile')[0].getAttribute('name')
        if not spdFile.startswith("/"):
            spdFile = os.path.join(os.path.dirname(xmlfile), spdFile)

        # Get the property name mapping.
        prfFile = getPropertyFile(spdFile, devmanFS)
        devManProps = parsePropertyFile(prfFile, devmanFS)
        try:
            props = devMan.query([])
        except:
            props = []
        self.buildPropertiesListView(propItem, props, devManProps)

        # Create a node for each Device under the Device Manager.
        for service in devMan._get_registeredServices():
            id = service.serviceName
            item = QListViewItem(serviceItem, id)

        # Create a node for each Device under the Device Manager.
        for device in devMan._get_registeredDevices():
            id = device._get_identifier()
            label = device._get_label()
            item = QListViewItem(deviceItem, device._get_label())
            QListViewItem(item, 'Identifier', device._get_identifier(), '1')
            QListViewItem(item, 'Admin State', str(device._get_adminState()), '2')
            QListViewItem(item, 'Operational State', str(device._get_operationalState()), '3')
            QListViewItem(item, 'Usage State', str(device._get_usageState()), '4')
            QListViewItem(item, 'Software Profile', device._get_softwareProfile(), '5')

            # Build a list of the device's properties in a sub-list
            propItem = QListViewItem(item, 'Properties')
            try:
                propfile = getPropertyFile(device._get_softwareProfile(), devmanFS)
                propertyDefs = parsePropertyFile(propfile, devmanFS)
            except:
                continue
            try:
                props = device.query([])
            except:
                props = []
            self.buildPropertiesListView(propItem, props, propertyDefs)

    def parseDomainManager (self):
        QListViewItem(self.domMgrItem, 'Identifier', self.domManager._get_identifier())
        QListViewItem(self.domMgrItem, 'Profile', self.domManager._get_domainManagerProfile())
        self.domMgrPropsItem = QListViewItem(self.domMgrItem, 'Properties')

        # Read the DMD file to get the SPD file, which can then be used to get the properties.
        _xmlFile = self.fileMgr.open(str(self.domManager._get_domainManagerProfile()), True)
        dmd = minidom.parseString(_xmlFile.read(_xmlFile.sizeOf()))      
        _xmlFile.close()
        spdFile = dmd.getElementsByTagName('localfile')[0].getAttribute('name')
        if not spdFile.startswith("/"):
            spdFile = os.path.join(os.path.dirname(xmlfile), spdFile)

        # Get the property name mapping.
        prfFile = getPropertyFile(spdFile, self.fileMgr)
        self.domMgrProps = parsePropertyFile(prfFile, self.fileMgr)

        # Create entries for all of the properties.
        try:
            props = self.domManager.query([])
        except:
            props = []
        self.buildPropertiesListView(self.domMgrPropsItem, props, self.domMgrProps)

    def refreshDomainManager (self):
        properties = {}
        try:
            props = self.domManager.query([])
        except:
            props = []
        for prop in props:
            id = self.domMgrProps[prop.id].get('name', prop.id)
            properties[id] = str(any.from_any(prop.value))

        child = self.domMgrPropsItem.firstChild()
        while child:
            child.setText(1, properties[str(child.text(0))])
            child = child.nextSibling()

    def parseDomain (self):
        self.parseDomainManager()
        self.parseApplicationFactories()
        self.parseApplications()
        self.parseAllDeviceManagers()

    def buildPropertiesListView (self, parent, properties, propertyDefs):
        for prop in properties:
            id = prop.id
            value = any.from_any(prop.value)
            if id not in propertyDefs:
                valueItem = QListViewItem(parent, id, str(value), '', id)
                continue
            property = propertyDefs[id]
            name = property.get('name', id)
            if property['elementType'] in ('simple', 'simplesequence'):
                # NB: As a temporary hack for mapping back to property id, store the id in
                #     the hidden fourth field.
                valueItem = QListViewItem(parent, name, str(value), '', id)
                if property['mode'] in ('readwrite', 'writeonly'):
                    valueItem.setRenameEnabled(1, True)
            elif property['elementType'] == 'struct':
                valueItem = QListViewItem(parent, name, '', '', id)
                for field in value:
                    QListViewItem(valueItem, field['id'], str(field['value']))
            elif property['elementType'] == 'structsequence':
                valueItem = QListViewItem(parent, name, '', '', id)
                if value == None:
                    continue
                for index, structval in enumerate(value):
                    subitem = QListViewItem(valueItem, str(index))
                    for field in structval:
                        QListViewItem(subitem, field['id'], str(field['value']))

    def debug (self, *args):
        if not self.verbose:
            return
        for arg in args:
            print arg,
        print
