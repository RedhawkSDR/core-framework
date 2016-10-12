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


from PyQt4.QtGui import *
from PyQt4.QtCore import *


class StructDialogBase(QDialog):
    def __init__(self):
        QDialog.__init__(self)
        self.mainlayout = QVBoxLayout(self)
        self.layout = QHBoxLayout()
        self.buttonOk = QPushButton('Ok')
        self.buttonCancel = QPushButton('Cancel')
        self.layout.addWidget(self.buttonOk)
        self.layout.addWidget(self.buttonCancel)
        self.connect(self.buttonOk,SIGNAL("clicked()"),self.accept)
        self.connect(self.buttonCancel,SIGNAL("clicked()"),self.reject)

    def createStructMemberLayout(self, name):
        tmplayout = QHBoxLayout()
        namewidget = QLabel()
        namewidget.setText(name)
        textwidget = QLineEdit()
        tmplayout.addWidget(namewidget)
        tmplayout.addWidget(textwidget)
        return tmplayout, textwidget
