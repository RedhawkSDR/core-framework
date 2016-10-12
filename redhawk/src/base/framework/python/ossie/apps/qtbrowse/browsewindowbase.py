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


# Form implementation generated from reading ui file 'browsewindowbase.ui'
#
# Created: Thu Feb 4 17:22:26 2010
#      by: The PyQt User Interface Compiler (pyuic) 3.13
#
# WARNING! All changes made in this file will be lost!


from PyQt4.QtGui import *
from PyQt4.QtCore import *
from ossie.utils import sb
import copy
from ossie.utils import redhawk,prop_helpers,type_helpers
import structdialog

def createPlotMenu(parent):
    plotMenu = parent.addMenu('Plot')
    lineAction = plotMenu.addAction('Line')
    lineAction = plotMenu.addAction('Line (PSD)')
    lineAction = plotMenu.addAction('Raster')
    lineAction = plotMenu.addAction('Raster (PSD)')

def createBulkioMenu(parent):
    createPlotMenu(parent)
    soundAction = parent.addAction('Sound')

def createPlot(resp):
    if resp == 'Line':
        return sb.LinePlot()
    elif resp == 'Line (PSD)':
        return sb.LinePSD()
    elif resp == 'Raster':
        return sb.RasterPlot()
    elif resp == 'Raster (PSD)':
        return sb.RasterPSD()

def hasBulkio(ports):
    for port in ports:
        if port._using != None:
            if port._using.nameSpace == 'BULKIO':
                return True
    return False

def plotResponse(resp):
    return resp == 'Line' or resp == 'Line (PSD)' or resp == 'Raster' or resp == 'Raster (PSD)'

def setPropValue(prop, itemtext):
    if itemtext == 'None':
        prop.configureValue(None)
    else:
        if prop.type == 'string':
            prop.configureValue(itemtext)
        elif prop.type == 'boolean':
            if itemtext == 'false':
                prop.configureValue(False)
            elif itemtext == 'true':
                prop.configureValue(True)
        elif prop.type == 'char':
            try:
                prop.configureValue(itemtext)
            except:
                pass
        else:
            value = type_helpers._SIStringToNumeric(itemtext)
            if isinstance(value, str):
                value = float(itemtext)
            try:
                prop.configureValue(value)
            except:
                pass

def typeStructDict(structDef, structDict):
    retval = {}
    badConversion = False
    for member in structDef.members:
        if structDict.has_key(member):
            _type = structDef.members[member].type
            if _type == 'string' or _type == 'char':
                retval[member] = structDict[member]
            elif _type == 'boolean':
                bval = structDict[member]
                try:
                    bval = bval.lower()
                except:
                    pass
                if bval == 'false':
                    retval[member] = False
                elif bval == 'true':
                    retval[member] = True
                else:
                    badConversion = True
                    retval = [member,_type]
                    break
            else:
                try:
                    retval[member] = float(structDict[member])
                except:
                    badConversion = True
                    retval = [member,_type]
                    break
    return retval

def setPropSeqValue(prop, itemtext):
    item_str_list = [y.strip() for y in itemtext[1:-1].split(',')]
    if itemtext == 'None':
        prop.configureValue(None)
    elif itemtext == '[]':
        prop.configureValue([])
    else:
        if prop.type == 'string':
            prop.configureValue(item_str_list)
        elif prop.type == 'boolean':
            val_list = [y.lower() for y in item_str_list]
            new_val = []
            for val in val_list:
                if val == 'false':
                    new_val.append(False)
                elif val == 'true':
                    new_val.append(True)
            prop.configureValue(new_val)
        elif prop.type == 'char':
            try:
                prop.configureValue(item_str_list)
            except:
                pass
        else:
            try:
                val_list = [float(y) for y in item_str_list]
                prop.configureValue(val_list)
            except:
                pass

class TreeWidget(QTreeWidget):
    def __init__(self, parent=None):
        QTreeWidget.__init__(self, parent)
        self.setContextMenuPolicy(Qt.CustomContextMenu)
        self.customContextMenuRequested.connect(self.contextMenuEvent)
        self.itemDoubleClicked.connect(self.onTreeWidgetItemDoubleClicked)
        self.itemChanged.connect(self.onTreeWidgetItemChanged)
        self.appObject = None
        self.devMgrsObject = None
        self.domMgrObject = None
        self.callbackObject = None

        # Disable edit triggers so we can control what fields are editable
        self.setEditTriggers(QTreeView.NoEditTriggers)

    def setCallbackObject(self, obj):
        self.callbackObject = obj
        self.appObject = obj.appsItem
        self.devMgrsObject = obj.devMgrItem
        self.domMgrObject = obj.domMgrItem

    def getPos(self, item, parent):
        pos = None
        if item is not None:
            parent = item.parent()
            while parent is not None:
                parent.setExpanded(True)
                parent = parent.parent()
            itemrect = self.visualItemRect(item)
            pos = self.mapToGlobal(itemrect.center())
        return pos

    def _isProperty(self, item):
        if item.parent() is None:
            return False
        elif item.parent().text(0) == 'Properties':
            return True
        else:
            return self._isProperty(item.parent())

    def _isEditable(self, item, column):
        if not item.flags() & Qt.ItemIsEditable:
            return False
        elif self._isProperty(item):
            # Only allow editing of value
            return column == 1
        else:
            return False

    def onTreeWidgetItemDoubleClicked(self, item, column):
        if self._isEditable(item, column):
            self.editItem(item, column)

    def findPropUnderApp(self, propitem): # propitem is the item level where the property id is the text content of column 0
        contname = propitem.parent().parent().parent().parent().text(0)
        compname = ''
        propname = ''
        prop = None
        # search for a matching app
        for app in self.callbackObject.apps:
            if app['app_ref'].name == contname:
                compname = propitem.parent().parent().text(0)
                for comp in app['app_ref'].comps:
                    if comp.name == compname:
                        for prop in comp._properties:
                            if propitem.text(0) == prop.clean_name:
                                return contname, compname, prop.clean_name, prop

        # search for a matching devMgr
        for devMgr in self.callbackObject.devMgrs:
            if devMgr['devMgr_ref'].name == contname:
                compname = propitem.parent().parent().text(0)
                for comp in devMgr['devMgr_ref'].devs:
                    if comp.name == compname:
                        for prop in comp._properties:
                            if propitem.text(0) == prop.clean_name:
                                return contname, compname, prop.clean_name, prop

        return contname, compname, propname, prop

    def findPropUnderContainer(self, propitem, containers): # propitem is the item level where the property id is the text content of column 0
        contname = propitem.parent().parent().parent().parent().text(0)
        compname = propname = ''
        prop = None
        if containers[0].has_key('application'):
            ref_key = 'app_ref'
        elif containers[0].has_key('devMgr'):
            ref_key = 'devMgr_ref'
        for cont in containers:
            if cont[ref_key].name == contname:
                compname = propitem.parent().parent().text(0)
                if ref_key == 'app_ref':
                    compset = cont[ref_key].comps
                else:
                    compset = cont[ref_key].devs
                for comp in compset:
                    if comp.name == compname:
                        for prop in comp._properties:
                            if propitem.text(0) == prop.clean_name:
                                return contname, compname, prop.clean_name, prop

        return contname, compname, propname, prop

    def checkPropertiesComponents(self, propertyItem):
        if propertyItem != None and propertyItem.parent() != None and propertyItem.parent().text(0) == "Properties" and \
            propertyItem.parent().parent() != None and propertyItem.parent().parent().parent() != None and \
            (propertyItem.parent().parent().parent().text(0) == "Components" or propertyItem.parent().parent().parent().text(0) == "Devices"):
                return True
        return False

    def onTreeWidgetItemChanged(self, item, column):
        # simple, simple sequence
        if self.checkPropertiesComponents(item):
            if column == 1:
                appname, compname, propname, prop = self.findPropUnderApp(item)
                if prop.__class__ == prop_helpers.sequenceProperty:
                    init_value = str(prop.queryValue())
                    if init_value == 'None' or init_value == '[]':
                        value = init_value
                    else:
                        value = [y.strip() for y in init_value[1:-1].split(',')]
                    if str(item.text(1)) == 'None' or str(item.text(1)) == '[]':
                        itemtext = str(item.text(1))
                    else:
                        itemtext = [y.strip() for y in str(item.text(1))[1:-1].split(',')]
                    if value != itemtext:
                        if prop.mode != 'readonly':
                            setPropSeqValue(prop, str(item.text(1)))
                    item.setText(1,str(prop.queryValue()))
                else:
                    value = str(prop.queryValue())
                    itemtext = str(item.text(1))
                    if prop.type == 'boolean':
                        try:
                            value = value.lower()
                            itemtext = itemtext.lower()
                        except:
                            pass
                    if value != itemtext:
                        if prop.mode != 'readonly':
                            setPropValue(prop, str(item.text(1)))
                        item.setText(1,str(prop.queryValue()))
        # struct
        if self.checkPropertiesComponents(item.parent()):
            if column == 1:
                appname, compname, propname, prop = self.findPropUnderApp(item.parent())
                for member in prop.members:
                    if item.text(0) == member:
                        value = str(prop.members[member].queryValue())
                        itemtext = str(item.text(1))
                        if prop.members[member].type == 'boolean':
                            try:
                                value = value.lower()
                                itemtext = itemtext.lower()
                            except:
                                pass
                        if value != item.text(1):
                            if prop.mode != 'readonly':
                                setPropValue(prop.members[member], str(item.text(1)))
                            item.setText(1,str(prop.members[member].queryValue()))
        # struct sequence
        if item.parent() != None and self.checkPropertiesComponents(item.parent().parent()):
            if column == 1:
                appname, compname, propname, prop = self.findPropUnderApp(item.parent().parent())
                idx = int(item.parent().text(0)[1:-1])
                for member in prop.structDef.members:
                    if item.text(0) == member:
                        value = str(prop[idx].members[member].queryValue())
                        itemtext = str(item.text(1))
                        if prop[idx].members[member].type == 'boolean':
                            try:
                                value = value.lower()
                                itemtext = itemtext.lower()
                            except:
                                pass
                        if value != item.text(1):
                            if prop.mode != 'readonly':
                                setPropValue(prop[idx].members[member], str(item.text(1)))
                            item.setText(1,str(prop[idx].members[member].queryValue()))

    def getRefs(self, containers, container_name, compname=None):
        contref = contTrack = comp = None
        foundCont = foundComp = False
        if len(containers) == 0:
            return contref, contTrack, comp
        if containers[0].has_key('application'):
            ref_key = 'app_ref'
        elif containers[0].has_key('devMgr'):
            ref_key = 'devMgr_ref'
        for cont in containers:
            if cont[ref_key].name == container_name:
                contref = cont[ref_key]
                contTrack = cont
                foundCont = True
                break
        if not foundCont or compname == None:
            return contref, contTrack, comp
        if ref_key == 'app_ref':
            compset = contref.comps
        else:
            compset = contref.devs
        for comp in compset:
            if comp.name == compname:
                foundComp = True
                break
        return contref, contTrack, comp

    def contextMenuEvent(self, event):
        pos = None
        selection = self.selectedItems()
        if selection:
            item = selection[0]
        else:
            item = self.currentItem()
            if item is None:
                item = self.invisibleRootItem().child(0)

        if item.parent() != None and (item.parent().text(0) == "Components" or item.parent().text(0) == "Devices"):
            if item.parent().text(0) == "Components":
                containers = self.callbackObject.apps
            else:
                containers = self.callbackObject.devMgrs
            container_name = item.parent().parent().text(0)
            contref = contTrack = None
            compname = item.text(0)
            foundApp = False
            contref, contTrack, comp = self.getRefs(containers, container_name, compname)
            if contref == None:
                if item.parent().text(0) == "Components":
                    self.callbackObject.log.warn("Unable to find Application: "+container_name)
                else:
                    self.callbackObject.log.warn("Unable to find Device Manager: "+container_name)
                return
            if comp == None:
                if item.parent().text(0) == "Components":
                    self.callbackObject.log.warn("Unable to find Component ("+compname+") on Application ("+container_name+")")
                else:
                    self.callbackObject.log.warn("Unable to find Device ("+compname+") on Device Manager ("+container_name+")")
                return
            pos = self.getPos(item, item.parent())
            if pos is not None:
                menu = QMenu(self)
                startAction = menu.addAction('Start')
                if comp._get_started():
                    startAction.setEnabled(False)
                stopAction = menu.addAction('Stop')
                if not comp._get_started():
                    stopAction.setEnabled(False)
                if hasBulkio(comp.ports):
                    createBulkioMenu(menu)
                menu.popup(pos)
                retval = menu.exec_()
            if retval != None:
                resp = retval.text()
                if resp == 'Start':
                    comp.start()
                elif resp == 'Stop':
                    comp.stop()
                elif plotResponse(resp):
                    plot = createPlot(resp)
                    plot.start()
                    try:
                        comp.connect(plot)
                        contTrack['widgets'].append((comp, plot))
                    except Exception, e:
                        plot.close()
                        if 'must specify providesPortName or usesPortName' in e.__str__():
                            QMessageBox.critical(self, 'Connection failed.', 'Cannot find a matching port. Please select a specific port in the Component port list to plot', QMessageBox.Ok)
                        else:
                            QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)
                elif resp == 'Sound':
                    sound=sb.SoundSink()
                    sound.start()
                    try:
                        comp.connect(sound)
                        contTrack['widgets'].append((comp, sound))
                    except Exception, e:
                        sound.releaseObject()
                        if 'must specify providesPortName or usesPortName' in e.__str__():
                            QMessageBox.critical(self, 'Connection failed.', 'Cannot find a matching port. Please select a specific port in the Component port list to plot', QMessageBox.Ok)
                        else:
                            QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)
        elif item.parent() == self.appObject:
            appname = item.text(0)
            contref, contTrack, comp = self.getRefs(self.callbackObject.apps, appname)
            if contref == None:
                self.callbackObject.log.warn("Unable to find Application: "+appname)
                return
            pos = self.getPos(item, item.parent())
            retval = None
            if pos is not None:
                menu = QMenu(self)
                relAction = menu.addAction('Release')
                startAction = menu.addAction('Start')
                if contref._get_started():
                    startAction.setEnabled(False)
                stopAction = menu.addAction('Stop')
                if not contref._get_started():
                    stopAction.setEnabled(False)
                if hasBulkio(contref.ports):
                    createBulkioMenu(menu)
                menu.popup(pos)
                retval = menu.exec_()
            if retval != None:
                resp = retval.text()
                if resp == 'Release':
                    self.callbackObject.addRequest(('releaseApplication(QString)',appname))
                elif resp == 'Start':
                    contref.start()
                elif resp == 'Stop':
                    contref.stop()
                elif plotResponse(resp):
                    plot = createPlot(resp)
                    plot.start()
                    try:
                        contref.connect(plot)
                        contTrack['widgets'].append((contref, plot))
                    except Exception, e:
                        plot.close()
                        if 'must specify providesPortName or usesPortName' in e.__str__():
                            QMessageBox.critical(self, 'Connection failed.', 'Cannot find a matching port. Please select a specific Port in the Application Port list to plot', QMessageBox.Ok)
                        else:
                            QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)
                elif resp == 'Sound':
                    sound=sb.SoundSink()
                    sound.start()
                    try:
                        contref.connect(sound)
                        contTrack['widgets'].append((contref, sound))
                    except Exception, e:
                        sound.releaseObject()
                        if 'must specify providesPortName or usesPortName' in e.__str__():
                            QMessageBox.critical(self, 'Connection failed.', 'Cannot find a matching port. Please select a specific port in the Component port list to plot', QMessageBox.Ok)
                        else:
                            QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)
        elif item.parent() != None and item.parent().text(0) == "Ports" and item.parent().parent() != None and \
          (item.parent().parent().parent().text(0) == "Components" or item.parent().parent().parent().text(0) == "Devices"):
            compname = item.parent().parent().text(0)
            container_name = item.parent().parent().parent().parent().text(0)
            portname = item.text(0)[:-7]
            contref = contTrack = None
            if item.parent().parent().parent().text(0) == "Components":
                containers = self.callbackObject.apps
            else:
                containers = self.callbackObject.devMgrs
            contref, contTrack, comp = self.getRefs(containers, container_name, compname)
            if contref == None:
                if item.parent().parent().parent().text(0) == "Components":
                    self.callbackObject.log.warn("Unable to find Application: "+container_name)
                else:
                    self.callbackObject.log.warn("Unable to find Device Manager: "+container_name)
                return
            if comp == None:
                if item.parent().parent().parent().text(0) == "Components":
                    self.callbackObject.log.warn("Unable to find Component ("+compname+") on Application ("+container_name+")")
                else:
                    self.callbackObject.log.warn("Unable to find Device ("+compname+") on Device Manager ("+container_name+")")
                return
            foundPort = False
            for port in comp.ports:
                if port.name == portname:
                    foundPort = True
                    break
            if not foundPort:
                if item.parent().parent().parent().text(0) == "Components":
                    self.callbackObject.log.warn("Unable to find port "+portname+" in component "+compname+" on application "+container_name)
                else:
                    self.callbackObject.log.warn("Unable to find port "+portname+" in device "+compname+" on device manager "+container_name)
                return
            pos = self.getPos(item, item.parent())
            retval = None
            if pos is not None:
                if port._using != None and port._using.nameSpace == 'BULKIO':
                    menu = QMenu(self)
                    createBulkioMenu(menu)
                    menu.popup(pos)
                    retval = menu.exec_()
            if retval != None:
                resp = retval.text()
                if plotResponse(resp):
                    plot = createPlot(resp)
                    plot.start()
                    try:
                        if port._using != None:
                            comp.connect(plot,usesPortName=str(portname))
                        contTrack['widgets'].append((contref, plot))
                    except Exception, e:
                        plot.close()
                        QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)
                elif resp == 'Sound':
                    sound=sb.SoundSink()
                    sound.start()
                    try:
                        comp.connect(sound)
                        contTrack['widgets'].append((contref, sound))
                    except Exception, e:
                        sound.releaseObject()
                        if 'must specify providesPortName or usesPortName' in e.__str__():
                            QMessageBox.critical(self, 'Connection failed.', 'Cannot find a matching port. Please select a specific port in the Component port list to plot', QMessageBox.Ok)
                        else:
                            QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)
        elif item.parent() != None and item.parent().text(0) == "Ports" and item.parent().parent() != None and item.parent().parent().parent() == self.appObject:
            appname = item.parent().parent().text(0)
            portname = item.text(0)[:-7]
            appref, appTrack, comp = self.getRefs(self.callbackObject.apps, appname)
            if appref == None:
                self.callbackObject.log.warn("Unable to find Application: "+appname)
                return
            foundPort = False
            for port in appref.ports:
                if port.name == portname:
                    foundPort = True
                    break
            if not foundPort:
                self.callbackObject.log.warn("Unable to find port "+portname+" on application "+appname)
                return
            pos = self.getPos(item, item.parent())
            retval = None
            if pos is not None:
                if port._using != None and port._using.nameSpace == 'BULKIO':
                    menu = QMenu(self)
                    createBulkioMenu(menu)
                    menu.popup(pos)
                    retval = menu.exec_()
            if retval != None:
                resp = retval.text()
                if plotResponse(resp):
                    plot = createPlot(resp)
                    plot.start()
                    try:
                        if port._using != None:
                            appref.connect(plot,usesPortName=str(portname))
                        appTrack['widgets'].append((appref, plot))
                    except Exception, e:
                        plot.close()
                        QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)
                elif resp == 'Sound':
                    sound=sb.SoundSink()
                    sound.start()
                    try:
                        appref.connect(sound)
                        appTrack['widgets'].append((appref, sound))
                    except Exception, e:
                        sound.releaseObject()
                        if 'must specify providesPortName or usesPortName' in e.__str__():
                            QMessageBox.critical(self, 'Connection failed.', 'Cannot find a matching port. Please select a specific port in the Component port list to plot', QMessageBox.Ok)
                        else:
                            QMessageBox.critical(self, 'Connection failed.', e.__str__(), QMessageBox.Ok)

        elif item == self.appObject:
            itemrect = self.visualItemRect(item)
            pos = self.mapToGlobal(itemrect.center())
            menu = QMenu(self)
            menu.addAction('Create Application')
            menu.addAction('Release All Applications')
            menu.popup(pos)
            retval = menu.exec_()
            if retval != None:
                resp = retval.text()
                if resp == 'Create Application':
                    self.callbackObject.createSelected()
                elif resp == 'Release All Applications':
                    applist = []
                    for app in self.callbackObject.apps:
                        applist.append(app['app_ref'].name)
                    for appname in applist:
                        self.callbackObject.addRequest(('releaseApplication(QString)',appname))
        elif item.parent() == self.devMgrsObject:
            devMgrname = item.text(0)
            foundDevMgr = False
            for devMgr in self.callbackObject.devMgrs:
                if devMgr['devMgr_ref'].name == devMgrname:
                    dmref = devMgr['devMgr_ref']
                    foundDevMgr = True
                    break
            if not foundDevMgr:
                self.callbackObject.log.warn("Unable to find Device Manager: "+devMgrname)
                return
            itemrect = self.visualItemRect(item)
            pos = self.mapToGlobal(itemrect.center())
            menu = QMenu(self)
            menu.addAction('Shutdown')
            menu.popup(pos)
            retval = menu.exec_()
            if retval != None:
                resp = retval.text()
                if resp == 'Shutdown':
                    self.callbackObject.addRequest(('shutdownDeviceManager(QString)',devMgrname))
        elif self.checkPropertiesComponents(item.parent()):
            itemrect = self.visualItemRect(item)
            appname, compname, propname, prop = self.findPropUnderApp(item.parent())
            pos = self.mapToGlobal(itemrect.center())
            menu = QMenu(self)
            del_action = menu.addAction('Delete')
            up_action = menu.addAction('Up')
            down_action = menu.addAction('Down')
            menu.popup(pos)
            if not ('write' in prop.mode):
                del_action.setEnabled(False)
                up_action.setEnabled(False)
                down_action.setEnabled(False)
            retval = menu.exec_()
            if retval != None:
                resp = retval.text()
                idx = int(item.text(0)[1:-1])
                data = prop.queryValue()
                if resp == 'Delete':
                    data.pop(idx)
                    prop.configureValue(data)
                    self.callbackObject.addRequest(('refreshProperty',appname,compname,prop.clean_name))
                elif resp == 'Up':
                    if idx != 0:
                        new_data = data[:idx-1]
                        new_data.append(data[idx])
                        new_data.append(data[idx-1])
                        if len(data) > idx+1:
                            new_data[idx+1:] = data[idx+1:]
                        prop.configureValue(new_data)
                        self.callbackObject.addRequest(('refreshProperty',appname,compname,prop.clean_name))
                elif resp == 'Down':
                    if idx != len(data)-1:
                        new_data = data[:idx]
                        new_data.append(data[idx+1])
                        new_data.append(data[idx])
                        if len(data) > idx+2:
                            new_data[idx+2:] = data[idx+2:]
                        prop.configureValue(new_data)
                        self.callbackObject.addRequest(('refreshProperty',appname,compname,prop.clean_name))
        elif self.checkPropertiesComponents(item):
            appname, compname, propname, prop = self.findPropUnderApp(item)
            foundProp = False
            if prop.__class__ == prop_helpers.structSequenceProperty:
                foundProp = True
                data = prop.queryValue()
            if foundProp:
                itemrect = self.visualItemRect(item)
                pos = self.mapToGlobal(itemrect.center())
                menu = QMenu(self)
                action = menu.addAction('Add')
                menu.popup(pos)
                if not ('write' in prop.mode):
                    action.setEnabled(False)
                retval = menu.exec_()
                if retval != None:
                    resp = retval.text()
                    structdef = prop.structDef
                    retval = structdialog.structInstance(prop.clean_name, prop.structDef.members)
                    if retval != None:
                        origvalue = prop.queryValue()
                        newvalue = typeStructDict(prop.structDef, retval)
                        if type(newvalue) == dict:
                            origvalue.append(newvalue)
                            prop.configureValue(origvalue)
                            self.callbackObject.addRequest(('refreshProperty',appname,compname,prop.clean_name))
                        else:
                            QMessageBox.critical(self, 'Bad type', "Element '"+newvalue[0]+"'. Expected type '"+newvalue[1]+"'", QMessageBox.Ok)

class BrowseWindowBase(QMainWindow):
    def __init__(self,parent = None,name = None,fl = 0,domainName=None):
        QMainWindow.__init__(self)
        base = self.statusBar()

        self.setCentralWidget(QWidget(self))
        BrowseWindowBaseLayout = QVBoxLayout(self.centralWidget())
        windowWidth = 720
        windowHeight = 569

        layout1 = QHBoxLayout(None)

        self.textLabel1 = QLabel("textLabel1", self.centralWidget())
        layout1.addWidget(self.textLabel1)
        self.textLabel2 = QLabel("", self.centralWidget())
        base.addWidget(self.textLabel2)

        self.domainLabel = QLabel("domainLabel", self.centralWidget())
        layout1.addWidget(self.domainLabel)
        spacer1 = QSpacerItem(361,20,QSizePolicy.Expanding,QSizePolicy.Minimum)
        layout1.addItem(spacer1)
        BrowseWindowBaseLayout.addLayout(layout1)

        self.objectListView = TreeWidget(self.centralWidget())
        self.objectListView.setColumnCount(2)
        self.objectListView.setHeaderLabels(['item', 'value'])
        self.objectListView.header().resizeSection(0, windowWidth/2-20)
        BrowseWindowBaseLayout.addWidget(self.objectListView)

        layout12 = QGridLayout(None)
        self.refreshButton = QPushButton(self.centralWidget())
        layout12.addWidget(self.refreshButton,0,0)
        BrowseWindowBaseLayout.addLayout(layout12)

        self.languageChange(domainName)

        self.resize(QSize(windowWidth,windowHeight).expandedTo(self.minimumSizeHint()))
        self.connect(self.refreshButton,SIGNAL("clicked()"),self.refreshView)
        self.connect(self.objectListView,SIGNAL("itemRenamed(QListViewItem*,int)"),self.propertyChanged)

    def closeEvent(self, event):
        self.cleanupOnExit()
        event.accept()

    def languageChange(self, domainName):
        if domainName == None:
            self.setWindowTitle(self.__tr("REDHAWK Domain Browser"))
            try:
                redhawk.scan()
            except RuntimeError:
                self.setStatusBar()
        else:
            self.setWindowTitle(self.__tr("REDHAWK Domain Browser: "+domainName))
        self.textLabel1.setText(self.__tr("Domain:"))
        self.domainLabel.setText(QString())
        self.refreshButton.setText(self.__tr("&Refresh"))

    def setStatusBar(self):
        self.textLabel2.setText(self.__tr("NameService not found"))

    def refreshView(self):
        print "BrowseWindowBase.refreshView(): Not implemented yet"

    def propertyChanged(self):
        print "BrowseWindowBase.propertyChanged(): Not implemented yet"

    def __tr(self,s,c = None):
        return qApp.translate("BrowseWindowBase",s,c)
