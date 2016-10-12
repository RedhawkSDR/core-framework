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
import glob

from PyQt4 import QtGui

from ossie.parsers import dcd
from ossie.utils import redhawk

import ui

class DeviceDialog(QtGui.QDialog):
    def __init__(self, *args, **kwargs):
        super(DeviceDialog,self).__init__(*args, **kwargs)
        ui.load('devicedialog.ui', self)
        for domain in redhawk.scan():
            self.domainNameEdit.addItem(domain)
        self.nodeTreeWidget.itemDoubleClicked.connect(self.onTreeWidgetItemDoubleClicked)

    def setSdrRoot(self, sdrroot):
        self.nodeTreeWidget.clear()
        nodepath = os.path.join(sdrroot, 'dev/nodes/*/DeviceManager.dcd.xml')
        for dcdfile in glob.glob(nodepath):
            try:
                node = dcd.parse(dcdfile)
                name = node.get_name()
                domain = node.get_domainmanager().get_namingservice().get_name()
                domain = domain.split('/')[-1]
                dcdfile = dcdfile.replace(os.path.join(sdrroot,'dev'), '')
                # Add the node to the tree widget, including the default domain
                # as a hidden column
                QtGui.QTreeWidgetItem(self.nodeTreeWidget, [name, dcdfile, domain])
            except:
                pass
        # Readjust the column widths to ensure that the entire name is shown
        # and that the scollbar allows viewing the entire DCD filename
        self.nodeTreeWidget.resizeColumnToContents(0)
        self.nodeTreeWidget.resizeColumnToContents(1)

        # Sort alphabetically by name
        self.nodeTreeWidget.sortByColumn(0, 0)

    def onTreeWidgetItemDoubleClicked(self, item):
        self.accept()

    def selectionChanged(self):
        domain = str(self.nodeTreeWidget.currentItem().text(2))
        self.setDomainName(domain)

    def domainName(self):
        return str(self.domainNameEdit.currentText())

    def setDomainName(self, name):
        index = self.domainNameEdit.findText(name)
        if index >= 0:
            self.domainNameEdit.setCurrentIndex(index)

    def debugLevel(self):
        return self.logLevelComboBox.currentIndex()

    def nodeName(self):
        return str(self.nodeTreeWidget.currentItem().text(0))

    def dcdFile(self):
        return str(self.nodeTreeWidget.currentItem().text(1))
