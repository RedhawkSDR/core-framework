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


# Form implementation generated from reading ui file 'propertydialogbase.ui'
#
# Created: Thu Feb 4 17:22:26 2010
#      by: The PyQt User Interface Compiler (pyuic) 3.13
#
# WARNING! All changes made in this file will be lost!


from qt import *


class PropertyDialogBase(QDialog):
    def __init__(self,parent = None,name = None,modal = 0,fl = 0):
        QDialog.__init__(self,parent,name,modal,fl)

        if not name:
            self.setName("PropertyDialogBase")

        self.setSizeGripEnabled(1)

        PropertyDialogBaseLayout = QVBoxLayout(self,11,6,"PropertyDialogBaseLayout")

        self.propertyListView = QListView(self,"propertyListView")
        self.propertyListView.addColumn(self.__tr("Property"))
        self.propertyListView.header().setClickEnabled(0,self.propertyListView.header().count() - 1)
        self.propertyListView.addColumn(self.__tr("Value"))
        self.propertyListView.header().setClickEnabled(0,self.propertyListView.header().count() - 1)
        PropertyDialogBaseLayout.addWidget(self.propertyListView)

        Layout1 = QHBoxLayout(None,0,6,"Layout1")
        Horizontal_Spacing2 = QSpacerItem(20,20,QSizePolicy.Expanding,QSizePolicy.Minimum)
        Layout1.addItem(Horizontal_Spacing2)

        self.buttonOk = QPushButton(self,"buttonOk")
        self.buttonOk.setAutoDefault(1)
        self.buttonOk.setDefault(1)
        Layout1.addWidget(self.buttonOk)

        self.buttonCancel = QPushButton(self,"buttonCancel")
        self.buttonCancel.setAutoDefault(1)
        Layout1.addWidget(self.buttonCancel)
        PropertyDialogBaseLayout.addLayout(Layout1)

        self.languageChange()

        self.resize(QSize(363,267).expandedTo(self.minimumSizeHint()))
        self.clearWState(Qt.WState_Polished)

        self.connect(self.buttonOk,SIGNAL("clicked()"),self.accept)
        self.connect(self.buttonCancel,SIGNAL("clicked()"),self.reject)
        self.connect(self.propertyListView,SIGNAL("itemRenamed(QListViewItem*,int)"),self.valueChanged)


    def languageChange(self):
        self.setCaption(self.__tr("Application Properties"))
        self.propertyListView.header().setLabel(0,self.__tr("Property"))
        self.propertyListView.header().setLabel(1,self.__tr("Value"))
        self.buttonOk.setText(self.__tr("&OK"))
        self.buttonOk.setAccel(QString.null)
        self.buttonCancel.setText(self.__tr("&Cancel"))
        self.buttonCancel.setAccel(QString.null)


    def valueChanged(self):
        print "PropertyDialogBase.valueChanged(): Not implemented yet"

    def __tr(self,s,c = None):
        return qApp.translate("PropertyDialogBase",s,c)
