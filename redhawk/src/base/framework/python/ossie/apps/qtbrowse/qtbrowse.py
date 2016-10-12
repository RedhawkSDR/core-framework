#!/usr/bin/env python
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
import getopt
from PyQt4.QtGui import *
from PyQt4.QtCore import *
import logging

from browsewindow import BrowseWindow

def main():
    # Set up a console logger.
    console = logging.StreamHandler()
    formatter = logging.Formatter("%(asctime)s %(name)-12s:%(levelname)-8s: %(message)s")
    console.setFormatter(formatter)
    logging.getLogger().addHandler(console)
    logging.getLogger().setLevel(logging.INFO)

    kw = {}
    longopts = ['domainname=', 'verbose']
    opts, args = getopt.getopt(sys.argv[1:], 'v', longopts)
    for opt, val in opts:
        if opt == '--domainname':
            kw['domainName'] = val
        if opt in ['-v', '--verbose']:
            kw['verbose'] = True

    a = QApplication(sys.argv)
    QObject.connect(a,SIGNAL("lastWindowClosed()"),a,SLOT("quit()"))
    w = BrowseWindow(**kw)
    w.show()
    a.exec_()

if __name__ == "__main__":
    main()
