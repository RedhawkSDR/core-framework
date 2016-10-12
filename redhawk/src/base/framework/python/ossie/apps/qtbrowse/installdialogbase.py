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


# Form implementation generated from reading ui file 'installdialogbase.ui'
#
# Created: Thu Feb 4 17:22:26 2010
#      by: The PyQt User Interface Compiler (pyuic) 3.13
#
# WARNING! All changes made in this file will be lost!


from qt import *


class InstallDialogBase(QDialog):
    def __init__(self,parent = None,name = None,modal = 0,fl = 0):
        QDialog.__init__(self,parent,name,modal,fl)

        if not name:
            self.setName("InstallDialogBase")


        InstallDialogBaseLayout = QHBoxLayout(self,11,6,"InstallDialogBaseLayout")

        self.appListBox = QListBox(self,"appListBox")
        InstallDialogBaseLayout.addWidget(self.appListBox)

        layout6 = QVBoxLayout(None,0,6,"layout6")

        layout5 = QGridLayout(None,1,1,0,6,"layout5")

        self.cancelButton = QPushButton(self,"cancelButton")

        layout5.addWidget(self.cancelButton,1,0)

        self.installButton = QPushButton(self,"installButton")
        self.installButton.setDefault(1)

        layout5.addWidget(self.installButton,0,0)
        layout6.addLayout(layout5)
        spacer3 = QSpacerItem(20,101,QSizePolicy.Minimum,QSizePolicy.Expanding)
        layout6.addItem(spacer3)
        InstallDialogBaseLayout.addLayout(layout6)

        self.languageChange()

        self.resize(QSize(576,256).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        self.connect(self.cancelButton,SIGNAL("clicked()"),self.reject)
        self.connect(self.installButton,SIGNAL("clicked()"),self.accept)


    def languageChange(self):
        self.setCaption(self.__tr("Install Application"))
        self.cancelButton.setText(self.__tr("&Cancel"))
        self.cancelButton.setAccel(self.__tr("Alt+C"))
        self.installButton.setText(self.__tr("&Install"))
        self.installButton.setAccel(self.__tr("Alt+I"))


    def __tr(self,s,c = None):
        return qApp.translate("InstallDialogBase",s,c)
