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
from structdialogbase import StructDialogBase


class StructDialog(StructDialogBase):

    def __init__(self, prop_name, members):
        StructDialogBase.__init__(self)
        self.setWindowTitle('new '+prop_name+' element')
        self.members = {}
        for member in members:
            tmplayout,text = self.createStructMemberLayout(member)
            self.members[member] = text
            self.mainlayout.addLayout(tmplayout)
        self.mainlayout.addLayout(self.layout)

    def structValues (self):
        tmp = self.appListBox.currentItem()
        return str(tmp.text())


def structInstance (prop_name, members):
    d = StructDialog(prop_name, members)
    d.setModal(True)
    d.show()
    d.exec_()
    if d.result() != QDialog.Accepted:
        return None
    retval = {}
    for member in d.members:
        retval[member] = str(d.members[member].text())
    d.hide()
    del d

    return retval
