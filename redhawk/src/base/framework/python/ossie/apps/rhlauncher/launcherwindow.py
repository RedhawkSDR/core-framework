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
import os

from PyQt4 import QtGui

from ossie.parsers import dmd

from commandwidget import CommandWidget
from domaindialog import DomainDialog
from devicedialog import DeviceDialog
import ui

class LauncherWindow(QtGui.QMainWindow):
    def __init__(self, sdrroot, *args, **kwargs):
        super(LauncherWindow,self).__init__(*args, **kwargs)
        ui.load('launcherwindow.ui', self)

        self._sdrroot = sdrroot
        dmdfile = os.path.join(self._sdrroot, 'dom/domain/DomainManager.dmd.xml')
        if os.path.exists(dmdfile):
            dom = dmd.parse(dmdfile)
            self._defaultDomain = dom.get_name()
        else:
            self._defaultDomain = 'REDHAWK_DEV'

    def closeEvent(self, event):
        if self.childrenActive():
            buttons = QtGui.QMessageBox.Yes | QtGui.QMessageBox.No | QtGui.QMessageBox.Cancel
            status = QtGui.QMessageBox.question(self, 'Processes active', """There are active processes. Do you want to terminate them?""", buttons)
            if status == QtGui.QMessageBox.Cancel:
                event.ignore()
                return
            elif status == QtGui.QMessageBox.Yes:
                self.terminateChildren()
        QtGui.QMainWindow.closeEvent(self, event)

    def processWidgets(self):
        return [self.nodeTabs.widget(index) for index in xrange(self.nodeTabs.count())]

    def terminateChildren(self):
        for widget in self.processWidgets():
            widget.terminateProcess()
        for widget in self.processWidgets():
            widget.waitTermination()

    def childrenActive(self):
        for widget in self.processWidgets():
            if widget.processActive():
                return True
        return False

    def closeTab(self, index):
        tab = self.nodeTabs.widget(index)
        tab.terminateProcess()
        self.nodeTabs.removeTab(index)

    def launchDomain(self):
        dialog = DomainDialog(self)
        dialog.setDomainName(self._defaultDomain)
        if not dialog.exec_():
            return

        args = []
        domain_name = dialog.domainName()
        if domain_name:
            args.extend(['--domainname', domain_name])
        else:
            domain_name = self._defaultDomain
        args.extend(['-debug', str(dialog.debugLevel())])
        if not dialog.persistence():
            args.append('--nopersist')
        args.append('-D')

        domainTab = CommandWidget(self.nodeTabs)
        index = self.nodeTabs.addTab(domainTab, 'Domain ' + domain_name)
        self.nodeTabs.setCurrentIndex(index)

        domainTab.executeCommand('nodeBooter', args)

    def launchDevice(self):
        dialog = DeviceDialog(self)
        dialog.setSdrRoot(self._sdrroot)
        
        if not dialog.exec_():
            return

        args = []
        args.extend(['--domainname', dialog.domainName()])
        args.extend(['-debug', str(dialog.debugLevel())])
        args.extend(['-d', dialog.dcdFile()])
        deviceTab = CommandWidget(self.nodeTabs)
        tab_text = 'Node %s [%s]' % (dialog.nodeName(), dialog.domainName())
        index = self.nodeTabs.addTab(deviceTab, tab_text)
        self.nodeTabs.setCurrentIndex(index)

        deviceTab.executeCommand('nodeBooter', args)
