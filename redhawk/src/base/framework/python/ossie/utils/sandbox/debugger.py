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
import socket

class Debugger(object):
    def __init__(self, command, option_value_join, **opts):
        self.command = command
        self.arguments = []
        for name, value in opts.iteritems():
            if value is True:
                value = 'yes'
            elif value is False:
                value = 'no'
            if value == '':
                self.arguments.append(name)
            else:
                if option_value_join:
                    self.arguments.append(name + option_value_join + str(value))
                else:
                    self.arguments.append(name)
                    self.arguments.append(str(value))

    def isInteractive(self):
        return True

    def modifiesCommand(self):
        return False

    def canAttach(self):
        return False

    def envUpdate(self):
        return {}

class GDB(Debugger):
    def __init__(self, attach=True, **opts):
        status, gdb = commands.getstatusoutput('which gdb')
        if status:
            raise RuntimeError, 'gdb cannot be found'
        pass_opts = {}
        for name, value in opts.iteritems():
            if len(name) == 1:
                name = '-'+name
                pass_opts[name] = value
            elif name[0] == '-' and name[1] != '-':
                pass_opts[name] = value
            elif name[:2] != '--':
                name = '--'+name
                name = name.replace('_','-')
                pass_opts[name] = value
        super(GDB,self).__init__(gdb, '=', **pass_opts)
        self._attach = attach

    def modifiesCommand(self):
        return not self._attach

    def canAttach(self):
        return self._attach

    def attach(self, process):
        return self.command, ['-p', str(process.pid())] + self.arguments

    def wrap(self, command, arguments):
        return self.command, ['--args', command] + arguments + self.arguments

    def name(self):
        return 'gdb'

class PDB(Debugger):
    def __init__(self, **opts):
        super(PDB,self).__init__(PDB.findPDB(), None, **opts)

    def modifiesCommand(self):
        return True

    def wrap(self, command, arguments):
        return self.command, [command] + arguments + self.arguments

    @staticmethod
    def findPDB():
        for path in sys.path:
            filename = os.path.join(path, 'pdb.py')
            if os.path.isfile(filename):
                return filename
        raise RuntimeError, 'pdb cannot be found'

    def name(self):
        return 'pdb'

class JDB(Debugger):
    def __init__(self, attach=True, **opts):
        status, jdb = commands.getstatusoutput('which jdb')
        if status:
            raise RuntimeError, 'jdb cannot be found'
        pass_opts = {}
        for name, value in opts.iteritems():
            if name[0] != '-':
                name = '-'+name
                name = name.replace('_','-')
                pass_opts[name] = value
        super(JDB,self).__init__(jdb, None, **opts)
        self._lastport = 5680
        self._attach = attach

    def modifiesCommand(self):
        return False

    def canAttach(self):
        return self._attach

    def attach(self, process):
        return self.command, ['-attach', str(self._lastport)] + self.arguments

    def wrap(self, command, arguments):
        return command, arguments + self.arguments

    def envUpdate(self):
        _open = False
        s=socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        while not _open:
            try:
                s.bind((socket.gethostbyname(socket.gethostname()), self._lastport))
                _open = True
                s.close()
            except:
                self._lastport += 1
        return {'JAVA_TOOL_OPTIONS':'-agentlib:jdwp=transport=dt_socket,server=y,suspend=n,address='+str(self._lastport)}

    def name(self):
        return 'jdb'

class Valgrind(Debugger):
    def __init__(self, **opts):
        status, valgrind = commands.getstatusoutput('which valgrind')
        if status:
            raise RuntimeError, 'valgrind cannot be found'
        pass_opts = {}
        for name, value in opts.iteritems():
            if len(name) == 1:
                name = '-'+name
                pass_opts[name] = value
            elif name[0] == '-' and name[1] != '-':
                pass_opts[name] = value
            elif name[:2] != '--':
                name = '--'+name
                name = name.replace('_','-')
                pass_opts[name] = value
        super(Valgrind,self).__init__(valgrind, '=', **pass_opts)

    def modifiesCommand(self):
        return True

    def isInteractive(self):
        return False

    def wrap(self, command, arguments):
        return self.command, self.arguments + [command] + arguments

    def name(self):
        return 'valgrind'
