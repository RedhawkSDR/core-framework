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
import copy
from qt import *
from propertydialogbase import PropertyDialogBase


class PropertyDialog(PropertyDialogBase):

    def __init__(self, properties, parent=None, name=None, modal=0, fl=0):
        PropertyDialogBase.__init__(self,parent,name,modal,fl)
        self.properties = copy.deepcopy(properties)
        for property in properties:
            name = property['name']
            if property.has_key('value'):
                value = str(property['value'])
            elif property.has_key('values'):
                value = str(property['values'])
            else:
                value = ''
            id = property['id']
            item = QListViewItem(self.propertyListView, name, value, id)
            if property['mode'] != 'readonly':
                item.setRenameEnabled(1, True)


    def values (self):
        return self.properties


    def valueChanged (self, item):
        name = str(item.text(0))
        value = str(item.text(1))
        for property in self.properties:
            if property['name'] == name:
                if property['elementType'] == 'simple':
                    property['value'] = value
                elif property['elementType'] == 'simplesequence':
                    # Evaluate the property here to make sure it's a
                    # proper list, so that the receiver doesn't have to.
                    try:
                        property['values'] = list(eval(value))
                    except:
                        item.setText(1, str(property['values']))
                else:
                    pass
                return


def showPropertyEditor (defaults, parent):
    d = PropertyDialog(defaults, parent)
    d.setModal(True)
    d.show()
    d.exec_loop()
    if d.result() == QDialog.Accepted:
        results = (True, d.values())
    else:
        results = (False, defaults)
    d.hide()
    del d
    return results
