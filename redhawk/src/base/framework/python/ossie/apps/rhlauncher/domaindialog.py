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
from PyQt4 import QtGui

import ui

class DomainDialog(QtGui.QDialog):
    def __init__(self, *args, **kwargs):
        super(DomainDialog,self).__init__(*args, **kwargs)
        ui.load('domaindialog.ui', self)

    def persistence(self):
        return self.persistCheckBox.isChecked()

    def setPersistence(self, enabled):
        self.persistCheckBox.setChecked(enabled)

    def domainName(self):
        return str(self.domainNameEdit.text())

    def setDomainName(self, name):
        self.domainNameEdit.setText(name)

    def debugLevel(self):
        return self.logLevelComboBox.currentIndex()
