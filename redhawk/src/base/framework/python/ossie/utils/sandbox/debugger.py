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

import commands
import os
import sys

class Debugger(object):
    def __init__(self, command):
        self.command = command

    def isInteractive(self):
        return True

    def modifiesCommand(self):
        return False

    def canAttach(self):
        return False

class GDB(Debugger):
    def __init__(self, attach=True):
        status, gdb = commands.getstatusoutput('which gdb')
        if status:
            raise RuntimeError, 'gdb cannot be found'
        super(GDB,self).__init__(gdb)
        self._attach = attach

    def modifiesCommand(self):
        return not self._attach

    def canAttach(self):
        return self._attach

    def attach(self, process):
        return self.command, [process.command(), str(process.pid())]

    def wrap(self, command, arguments):
        return self.command, ['--args', command] + arguments

class PDB(Debugger):
    def __init__(self):
        super(PDB,self).__init__(PDB.findPDB())

    def modifiesCommand(self):
        return True

    def wrap(self, command, arguments):
        return self.command, [command] + arguments

    @staticmethod
    def findPDB():
        for path in sys.path:
            filename = os.path.join(path, 'pdb.py')
            if os.path.isfile(filename):
                return filename
        raise RuntimeError, 'pdb cannot be found'


class Valgrind(Debugger):
    def __init__(self, quiet=False, verbose=False, **opts):
        status, valgrind = commands.getstatusoutput('which valgrind')
        if status:
            raise RuntimeError, 'valgrind cannot be found'
        super(Valgrind,self).__init__(valgrind)
        self.arguments = []
        if quiet:
            self.arguments.append('-q')
        if verbose:
            self.arguments.append('-v')
        for name, value in opts.iteritems():
            optname = '--' + name.replace('_','-')
            if value is True:
                value = 'yes'
            elif value is False:
                value = 'no'
            self.arguments.append(optname + '=' + str(value))

    def modifiesCommand(self):
        return True

    def isInteractive(self):
        return False

    def wrap(self, command, arguments):
        return self.command, self.arguments + [command] + arguments

