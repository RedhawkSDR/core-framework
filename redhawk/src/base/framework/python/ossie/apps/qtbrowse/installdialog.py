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


from PyQt4.QtGui import *
from .installdialogbase import InstallDialogBase


class InstallDialog(InstallDialogBase):

    def __init__(self, fileList, parent = None,name = None,modal = 0,fl = 0):
        InstallDialogBase.__init__(self,parent,name,modal,fl)
        for appname, filename in fileList:
            QTreeWidgetItem(self.appListBox, [appname, filename])

        # Resize both columns to show everything
        self.appListBox.resizeColumnToContents(0)
        self.appListBox.resizeColumnToContents(1)

        # Turn on column sorting and default to alpha order on names
        self.appListBox.setSortingEnabled(True)
        self.appListBox.sortByColumn(0, 0)

        # Ensure that the window is large enough to display the entire name and
        # a reasonable portion of the SAD filename
        minWidth = 2*self.appListBox.columnWidth(0)
        self.appListBox.setMinimumWidth(minWidth)
        self.appListBox.setMinimumHeight(250)

    def selectedApp (self):
        tmp = self.appListBox.currentItem()
        return str(tmp.text(1))

def getApplicationFile (appList, parent):
    d = InstallDialog(appList, parent)
    d.setModal(True)
    d.show()
    d.exec_()
    if d.result() != QDialog.Accepted:
        return None
    app = d.selectedApp()
    d.hide()
    del d

    return app
